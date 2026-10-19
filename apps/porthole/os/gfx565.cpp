#include "gfx565.h"
#include <string.h>

namespace gfx565 {
uint16_t* fb = nullptr;
void target(uint16_t* pixels) { fb = pixels; }

// ponytail: a 32-bit hash per row, so a changed row that collides with the old one stays stale for that frame
// (about 1 in 4 billion); keep a copy of the logical rows instead if that ever shows.
static uint32_t rowHash(const uint8_t* row) {
  uint32_t h = 0x811C9DC5u, w;
  for (int i = 0; i < W / 3; i += 4) { memcpy(&w, row + i, 4); h = (h ^ w) * 0x9E3779B1u; h ^= h >> 15; }
  return h;
}
void expandIndexed3(const uint8_t* logical160, const uint16_t* palette, Expanded* held) {
  uint16_t row[W];
  const bool same = held && held->palette == palette;
  if (held) held->palette = palette;
  for (int y = 0; y < H / 3; y++) {
    if (held) {
      const uint32_t h = rowHash(logical160 + y * (W / 3));
      if (same && held->rows[y] == h) continue;
      held->rows[y] = h;
    }
    for (int x = 0; x < W / 3; x++) {
      const uint16_t c = palette[logical160[y * (W / 3) + x]];
      row[x * 3] = row[x * 3 + 1] = row[x * 3 + 2] = c;
    }
    for (int dy = 0; dy < 3; dy++) memcpy(fb + (y * 3 + dy) * W, row, sizeof row);
  }
}

void rect(int x, int y, int w, int h, uint16_t c) {
  int x0 = x < 0 ? 0 : x, y0 = y < 0 ? 0 : y;
  int x1 = x + w > W ? W : x + w, y1 = y + h > H ? H : y + h;
  if (x1 <= x0 || y1 <= y0) return;
  uint16_t* first = fb + y0 * W + x0;
  for (int xx = 0; xx < x1 - x0; xx++) first[xx] = c;
  for (int yy = y0 + 1; yy < y1; yy++) memcpy(fb + yy * W + x0, first, (size_t)(x1 - x0) * 2);
}
void clear(uint16_t c) { rect(0, 0, W, H, c); }
void hline(int x, int y, int w, uint16_t c) { rect(x, y, w, 1, c); }
void frame(int x, int y, int w, int h, uint16_t c) {
  rect(x, y, w, 1, c); rect(x, y + h - 1, w, 1, c); rect(x, y, 1, h, c); rect(x + w - 1, y, 1, h, c);
}
void roundRect(const Rect& b, int r, uint16_t c) {
  const int x = b.x, y = b.y, w = b.w, h = b.h;
  if (r > w / 2) r = w / 2;
  if (r > h / 2) r = h / 2;
  if (r < 0) r = 0;
  rect(x, y + r, w, h - 2 * r, c);
  for (int k = 0; k < r; k++) {   // corner rows: a pixel is in when its center is within r of the corner circle's center
    int d = 2 * (r - k) - 1, in = 0;   // doubled distances keep it in integers
    while (in < r && (2 * (r - in) - 1) * (2 * (r - in) - 1) + d * d > 4 * r * r) in++;
    rect(x + in, y + k, w - 2 * in, 1, c);
    rect(x + in, y + h - 1 - k, w - 2 * in, 1, c);
  }
}
void circle(int cx, int cy, int r, uint16_t c) {   // same disc as gfx::circle, at panel resolution
  for (int dy = -r; dy <= r; dy++) {
    int dx = 0; while ((dx + 1) * (dx + 1) + dy * dy <= r * r + r) dx++;
    rect(cx - dx, cy + dy, 2 * dx + 1, 1, c);
  }
}
// One scaled source row at a time: its pixels land in a line in SRAM, then the line is copied `scale` times, so the
// panel buffer gets whole-row writes instead of a small rect per pixel or run.
namespace {
struct Line {
  uint16_t px[W];
  int lo = W, hi = 0;   // the part filled since the last flush
  void fill(int x0, int x1, uint16_t c) {
    if (x0 < 0) x0 = 0;
    if (x1 > W) x1 = W;
    for (int i = x0; i < x1; i++) px[i] = c;
    if (x0 < x1) { lo = x0 < lo ? x0 : lo; hi = x1 > hi ? x1 : hi; }
  }
  void flush(int y, int scale) {
    for (int dy = 0; dy < scale && hi > lo; dy++)
      if ((unsigned)(y + dy) < (unsigned)H) memcpy(fb + (y + dy) * W + lo, px + lo, (size_t)(hi - lo) * 2);
    lo = W; hi = 0;
  }
};
}  // namespace
void blit(const Image565& im, int x, int y, int scale) {
  if (scale == 1) {   // row copies: full-screen images are drawn every frame
    int x0 = x < 0 ? -x : 0, x1 = x + im.width > W ? W - x : im.width;
    for (int sy = y < 0 ? -y : 0; sy < im.height && y + sy < H && x1 > x0; sy++)
      memcpy(fb + (y + sy) * W + x + x0, im.pixels + sy * im.width + x0, (size_t)(x1 - x0) * 2);
    return;
  }
  Line line;
  for (int sy = 0; sy < im.height; sy++) {
    const int top = y + sy * scale;
    if (top >= H || top + scale <= 0) continue;
    const uint16_t* row = im.pixels + sy * im.width;
    for (int sx = 0; sx < im.width; sx++) line.fill(x + sx * scale, x + (sx + 1) * scale, row[sx]);
    line.flush(top, scale);
  }
}
void blitRle(const RleImage& im, int x, int y, int scale) {
  const uint32_t total = (uint32_t)im.width * im.height;
  uint32_t at = 0;
  Line line;
  for (uint32_t i = 0; i < im.runCount && at < total; i++) {
    uint32_t left = im.runs[2 * i];
    const uint16_t c = im.runs[2 * i + 1];
    if (left > total - at) left = total - at;
    while (left) {   // a run carries on into the next rows
      uint32_t col = at % im.width, row = at / im.width, n = im.width - col < left ? im.width - col : left;
      line.fill(x + (int)col * scale, x + (int)(col + n) * scale, c);
      at += n; left -= n;
      if (col + n == im.width) line.flush(y + (int)row * scale, scale);
    }
  }
  if (at % im.width) line.flush(y + (int)(at / im.width) * scale, scale);   // runs that stop partway through a row
}
}  // namespace gfx565
