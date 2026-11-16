// Reference-model checks for the shared painters (os/gfx565, os/canvas, os/gfx): each primitive against a brute-force
// model of the pixels it must set, with clips, off-screen and degenerate arguments. A game's golden only says that
// something moved; this says which painter. Run: make test
#include "host/check.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "canvas.h"
#include "gfx.h"
#include "gfx565.h"

namespace {
using gfx565::H;
using gfx565::W;
int checks = 0;
uint32_t seed = 0x51ED270Bu;
int rnd(int lo, int hi) {   // inclusive
  seed ^= seed << 13; seed ^= seed >> 17; seed ^= seed << 5;
  return lo + (int)(seed % (uint32_t)(hi - lo + 1));
}
uint16_t ink() { return (uint16_t)rnd(1, 0xFFFF); }

// ---- the RGB565 panel: what the painter drew and what the model says, each between guard rows
struct Panel { uint16_t before[W], px[W * H], after[W]; };
Panel got, want;
canvas::Box cut = canvas::FULL;   // the model's clip
void model(long long x, long long y, uint16_t c) {
  if (x >= cut.x0 && x < cut.x1 && y >= cut.y0 && y < cut.y1) want.px[y * W + x] = c;
}
void same() { CHECK(!memcmp(&got, &want, sizeof got)); }   // the guards too
void modelRect(long long x, long long y, long long w, long long h, uint16_t c) {
  for (long long yy = y < 0 ? 0 : y; yy < y + h && yy < H; yy++)
    for (long long xx = x < 0 ? 0 : x; xx < x + w && xx < W; xx++) model(xx, yy, c);
}
void modelDisc(int cx, int cy, int r, uint16_t c) {   // gfx::circle's and canvas::disc's rule
  for (int y = cy - r; y <= cy + r; y++)
    for (int x = cx - r; x <= cx + r; x++)
      if ((x - cx) * (x - cx) + (y - cy) * (y - cy) <= r * r + r) model(x, y, c);
}

void rects() {
  for (int n = 0; n <= 40; n++)   // every short length at each alignment of the first pixel, one row and several
    for (int x = 0; x < 4; x++) {
      const uint16_t c = ink();
      gfx565::hline(x + 7 * n, n, n, c); modelRect(x + 7 * n, n, n, 1, c);
      gfx565::rect(x + 3 * n, 100 + n, n, 3, c); modelRect(x + 3 * n, 100 + n, n, 3, c);
      same();
    }
  for (int i = 0; i < 3000; i++) {
    const int x = rnd(-60, 540), y = rnd(-60, 540), w = rnd(-4, i % 7 ? 130 : 700), h = rnd(-4, i % 5 ? 40 : 700);
    const uint16_t c = ink();
    gfx565::rect(x, y, w, h, c); modelRect(x, y, w, h, c);
    same();
  }
  gfx565::clear(0x1234); modelRect(0, 0, W, H, 0x1234);
  same();
}

void frames() {
  for (int i = 0; i < 1000; i++) {
    const int x = rnd(-30, 500), y = rnd(-30, 500), w = rnd(1, 90), h = rnd(1, 90);
    const uint16_t c = ink();
    gfx565::frame(x, y, w, h, c);
    modelRect(x, y, w, 1, c); modelRect(x, y + h - 1, w, 1, c); modelRect(x, y, 1, h, c); modelRect(x + w - 1, y, 1, h, c);
    same();
  }
}

void discs() {
  for (int r = -2; r <= 245; r++) {   // every radius a game can ask for, on the glass's center
    const uint16_t c = ink();
    gfx565::circle(240, 240, r, c); modelDisc(240, 240, r, c);
    same();
  }
  for (int i = 0; i < 300; i++) {
    const int cx = rnd(-40, 520), cy = rnd(-40, 520), r = rnd(0, 70);
    const uint16_t c = ink();
    gfx565::circle(cx, cy, r, c); modelDisc(cx, cy, r, c);
    same();
  }
}

// A corner pixel is in when its center is within r of the corner circle's center (doubled, to stay in integers).
bool inCorner(int i, int k, int r) { return i >= r || k >= r || (2 * (r - i) - 1) * (2 * (r - i) - 1) + (2 * (r - k) - 1) * (2 * (r - k) - 1) <= 4 * r * r; }
void roundRects() {
  for (int i = 0; i < 600; i++) {
    const int x = rnd(-30, 460), y = rnd(-30, 460), w = rnd(1, 120), h = rnd(1, 120);
    int r = rnd(0, 70);
    const uint16_t c = ink();
    gfx565::roundRect({x, y, w, h}, r, c);
    if (r > w / 2) r = w / 2;
    if (r > h / 2) r = h / 2;
    for (int yy = 0; yy < h; yy++)
      for (int xx = 0; xx < w; xx++) {
        const int col = xx < w - 1 - xx ? xx : w - 1 - xx, row = yy < h - 1 - yy ? yy : h - 1 - yy;
        if (inCorner(col, row, r)) model(x + xx, y + yy, c);
      }
    same();
  }
}

// ---- os/canvas: the same shapes cut to a clip box (which canvas::clip itself cuts to the panel)
void clipTo(const canvas::Box& b) {
  canvas::clip(b);
  cut = {b.x0 < 0 ? 0 : b.x0, b.y0 < 0 ? 0 : b.y0, b.x1 > W ? W : b.x1, b.y1 > H ? H : b.y1};
  CHECK(canvas::same(canvas::clipBox(), cut));
}
void randomClip(int i) {
  if (i % 4 == 0) { clipTo(canvas::FULL); return; }
  const int x = rnd(-50, 470), y = rnd(-50, 470);
  clipTo({x, y, x + rnd(-5, 300), y + rnd(-5, 300)});
}
// roundBox: `in` columns are cut from each end of a row k rows into a corner of radius r.
int cutColumns(int k, int r) { int in = 0; while ((r - in) * (r - in) + k * k > r * r) in++; return in; }
void modelRoundBox(const canvas::Box& b, int r, uint16_t c) {
  for (int y = b.y0; y < b.y1; y++) {
    const int top = y - b.y0, bottom = b.y1 - 1 - y;
    const int k = top < r ? r - top : bottom < r ? r - bottom : 0;
    const int in = k ? cutColumns(k, r) : 0;
    for (int x = b.x0 + in; x < b.x1 - in; x++) model(x, y, c);
  }
}
void clipped() {
  for (int i = 0; i < 1500; i++) {
    randomClip(i);
    const int x = rnd(-40, 500), y = rnd(-40, 500), w = rnd(-3, 200), h = rnd(1, 120), r = rnd(-1, 90);
    const uint16_t c = ink();
    canvas::span(y, x, x + w, c); modelRect(x, y, w, 1, c);
    same();
    canvas::disc(x, y, r, c); modelDisc(x, y, r, c);
    same();
    const canvas::Box box = {x, y, x + w, y + h};
    const int corner = w > 1 ? rnd(0, (w < h ? w : h) / 2) : 0;
    canvas::roundBox(box, corner, c); modelRoundBox(box, corner, c);
    same();
    const int quad[8] = {x, y, x + w, y, x + w, y + h, x, y + h};   // an upright box as a polygon: both ends inclusive
    if (w >= 0) { canvas::poly(quad, 4, c); modelRect(x, y, w + 1, h + 1, c); }
    same();
  }
  clipTo(canvas::FULL);
  const int roots[][2] = {{-1, -1}, {0, 0}, {15, 3}, {16, 4}, {57840, 240}};
  for (const auto& q : roots) CHECK(canvas::isqrt(q[0]) == q[1]);
}
// Any triangle stays inside the clip and its own bounding box, and paints the corners the clip lets through.
bool inCut(int x, int y) { return x >= cut.x0 && x < cut.x1 && y >= cut.y0 && y < cut.y1; }
void triangles() {
  for (int i = 0; i < 300; i++) {
    randomClip(i);
    int p[6];
    for (int& v : p) v = rnd(-40, 520);
    canvas::Box bounds = {p[0], p[1], p[0] + 1, p[1] + 1};
    for (int k = 2; k < 6; k += 2) bounds = canvas::unite(bounds, {p[k], p[k + 1], p[k] + 1, p[k + 1] + 1});
    memset(got.px, 0, sizeof got.px);
    canvas::poly(p, 3, 0xFFFF);
    for (int y = 0; y < H; y++)
      for (int x = 0; x < W; x++) {
        if (!got.px[y * W + x]) continue;
        assert(inCut(x, y));
        assert(x >= bounds.x0 && x < bounds.x1 && y >= bounds.y0 && y < bounds.y1);
      }
    for (int k = 0; k < 6; k += 2) if (inCut(p[k], p[k + 1])) CHECK(got.px[p[k + 1] * W + p[k]]);
    memcpy(got.px, want.px, sizeof got.px);
  }
  clipTo(canvas::FULL);
}

// ---- os/gfx: the 160x160 indexed surface
uint8_t ref[gfx::W * gfx::H];
void set(int x, int y, uint8_t c) { if (x >= 0 && x < gfx::W && y >= 0 && y < gfx::H) ref[y * gfx::W + x] = c; }
void box(int x, int y, int w, int h, uint8_t c) { for (int yy = y; yy < y + h; yy++) for (int xx = x; xx < x + w; xx++) set(xx, yy, c); }
void sameIndexed() { CHECK(!memcmp(gfx::fb, ref, sizeof ref)); }
void indexedShapes() {
  gfx::clear(3); memset(ref, 3, sizeof ref);
  for (int i = 0; i < 4000; i++) {
    const int x = rnd(-20, 180), y = rnd(-20, 180), w = rnd(1, 60), h = rnd(1, 60), r = rnd(-1, 40);
    const uint8_t c = (uint8_t)rnd(0, 31), d = (uint8_t)rnd(0, 31);
    switch (i % 6) {
      case 0: gfx::rect(x, y, w - 3, h - 3, c); box(x, y, w - 3, h - 3, c); break;   // empty and negative sizes too
      case 1: gfx::hline(x, y, w, c); box(x, y, w, 1, c); break;
      case 2: gfx::frame(x, y, w, h, c); box(x, y, w, 1, c); box(x, y + h - 1, w, 1, c); box(x, y, 1, h, c); box(x + w - 1, y, 1, h, c); break;
      case 3: gfx::roundRect(x, y, w, h, c); box(x + 1, y, w - 2, h, c); box(x, y + 1, w, h - 2, c); break;   // the box less its corners
      case 4:
        gfx::circle(x, y, r, c);
        for (int yy = y - r; yy <= y + r; yy++) for (int xx = x - r; xx <= x + r; xx++) if ((xx - x) * (xx - x) + (yy - y) * (yy - y) <= r * r + r) set(xx, yy, c);
        break;
      default:
        gfx::checker(x, y, w, h, c, d);
        for (int yy = y; yy < y + h; yy++) for (int xx = x; xx < x + w; xx++) set(xx, yy, ((xx + yy) & 1) ? c : d);
    }
    sameIndexed();
  }
}

int painted(uint8_t c) { int n = 0; for (uint8_t p : gfx::fb) n += p == c; return n; }
void lines() {
  for (int i = 0; i < 500; i++) {   // on the screen: both ends, one pixel a step of the longer axis, inside its box
    const int x0 = rnd(0, 159), y0 = rnd(0, 159), x1 = i % 9 ? rnd(0, 159) : x0, y1 = i % 7 ? rnd(0, 159) : y0;
    gfx::clear(0);
    gfx::line(x0, y0, x1, y1, 9);
    const int dx = abs(x1 - x0), dy = abs(y1 - y0);
    CHECK(gfx::fb[y0 * gfx::W + x0] == 9 && gfx::fb[y1 * gfx::W + x1] == 9 && painted(9) == (dx > dy ? dx : dy) + 1);
    gfx::rect(x0 < x1 ? x0 : x1, y0 < y1 ? y0 : y1, dx + 1, dy + 1, 0);
    CHECK(painted(9) == 0);
  }
  gfx::line(-500, 80, 700, 90, 9); gfx::line(80, -300, 70, 900, 9);   // off the screen at both ends: clipped per pixel
  CHECK(painted(9) > 300);
}
void rings() {
  for (int r = 0; r <= 78; r++) {   // a closed 1 px outline at distance r of the center, the same in all eight octants
    gfx::clear(0);
    gfx::ring(80, 80, r, 5);
    for (int y = 0; y < gfx::H; y++)
      for (int x = 0; x < gfx::W; x++) {
        if (!gfx::fb[y * gfx::W + x]) continue;
        const int dx = x - 80, dy = y - 80, d2 = dx * dx + dy * dy;
        assert(d2 < (r + 1) * (r + 1) && (r == 0 || d2 > (r - 1) * (r - 1)));
        assert(gfx::fb[(80 - dy) * gfx::W + 80 + dx] && gfx::fb[(80 + dy) * gfx::W + 80 - dx] && gfx::fb[(80 + dx) * gfx::W + 80 + dy]);
      }
    CHECK(gfx::fb[80 * gfx::W + 80 + r] && gfx::fb[(80 - r) * gfx::W + 80] && painted(5) >= (r ? 4 : 1));
  }
  gfx::ring(0, 0, 300, 5); gfx::ring(159, 159, 20, 5);   // mostly off the screen: clipped per pixel
}

void blits() {
  static const uint8_t PX[3 * 2] = {1, C_T, 2, 3, 4, C_T};
  const gfx::Sprite s = {3, 2, PX};
  gfx::clear(0); memset(ref, 0, sizeof ref);
  for (int i = 0; i < 600; i++) {
    const int x = rnd(-8, 165), y = rnd(-8, 165), scale = rnd(1, 4);
    const bool fx = rnd(0, 1), fy = rnd(0, 1);
    const uint8_t tint = (uint8_t)rnd(5, 31);
    for (int sy = 0; sy < 2; sy++)
      for (int sx = 0; sx < 3; sx++) {
        const uint8_t c = PX[sy * 3 + sx];
        if (c == C_T) continue;
        const int ox = fx ? 2 - sx : sx;
        if (i % 3 == 0) set(x + ox, y + (fy ? 1 - sy : sy), c);
        else if (i % 3 == 1) set(x + ox, y + sy, tint);
        else box(x + ox * scale, y + sy * scale, scale, scale, c);
      }
    if (i % 3 == 0) gfx::blit(s, x, y, fx, fy);
    else if (i % 3 == 1) gfx::blitTint(s, x, y, tint, fx);
    else gfx::blitScaled(s, x, y, scale, fx);
    sameIndexed();
  }
}

void textAndWrap() {
  static uint8_t small[gfx::W * gfx::H];
  gfx::clear(0);
  const int end = gfx::text(4, 6, "Wag it! (9)", 7);
  memcpy(small, gfx::fb, sizeof small);
  gfx::clear(0);
  CHECK(gfx::text(8, 12, "Wag it! (9)", 7, 2) == 2 * end);   // scale 2: every pixel of scale 1, doubled
  for (int y = 0; y < gfx::H; y++)
    for (int x = 0; x < gfx::W; x++) assert(gfx::fb[y * gfx::W + x] == small[(y / 2) * gfx::W + x / 2]);
  gfx::text(-20, -3, "off both edges of the round screen, top", 7); gfx::text(150, 156, "and bottom", 7, 2);   // clipped per pixel
  char l[4][40];
  CHECK(gfx::wrap("The quick brown fox jumps over the lazy dog", 80, l, 4) == 4);
  CHECK(!strcmp(l[0], "The quick") && !strcmp(l[1], "brown fox") && !strcmp(l[2], "jumps over") && !strcmp(l[3], "the lazy dog"));
  for (auto& line : l) CHECK(gfx::textWidth(line) <= 80);
  CHECK(gfx::wrap("one\ntwo  three\n\nfour", 200, l, 4) == 4 && !strcmp(l[1], "two three") && !l[2][0] && !strcmp(l[3], "four"));
  CHECK(gfx::wrap("a b c d e f", 5, l, 3) == 3 && !strcmp(l[2], "c"));          // no more than maxLines
  CHECK(gfx::wrap("Supercalifragilistic day", 30, l, 4) == 2 && !strcmp(l[0], "Supercalifragilistic"));   // a wide word keeps its line
  CHECK(gfx::wrap("", 100, l, 4) == 0 && gfx::wrap("big words", 60, l, 4, 2) == 2);
}
}  // namespace

int main() {
  memset(&got, 0, sizeof got); memset(&want, 0, sizeof want);
  for (int i = 0; i < W; i++) got.before[i] = got.after[i] = want.before[i] = want.after[i] = 0xDEAD;
  gfx565::target(got.px);
  rects();
  frames();
  discs();
  roundRects();
  clipped();
  triangles();
  indexedShapes();
  lines();
  rings();
  blits();
  textAndWrap();
  printf("primitives OK: %d checks\n", checks);
  return 0;
}
