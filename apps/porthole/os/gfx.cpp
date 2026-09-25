#include "gfx.h"
#include <string.h>
#include <stdio.h>
#include "font8x8_basic.h"

namespace gfx {
uint8_t fb[W * H];

static uint8_t g_left[128], g_right[128];  // glyph ink bounds (inclusive), computed once
static bool g_fontReady = false;
static void fontInit() {
  if (g_fontReady) return;
  for (int ch = 0; ch < 128; ch++) {
    int l = 8, r = -1;
    for (int row = 0; row < 8; row++) {
      uint8_t bits = (uint8_t)font8x8_basic[ch][row];
      for (int x = 0; x < 8; x++) if (bits & (1 << x)) { if (x < l) l = x; if (x > r) r = x; }
    }
    if (r < 0) { l = 0; r = 2; }  // space & blanks: 3px wide
    g_left[ch] = (uint8_t)l; g_right[ch] = (uint8_t)r;
  }
  g_fontReady = true;
}

void clear(uint8_t c) { memset(fb, c, sizeof fb); }

void rect(int x, int y, int w, int h, uint8_t c) {
  int x0 = x < 0 ? 0 : x, y0 = y < 0 ? 0 : y;
  int x1 = x + w > W ? W : x + w, y1 = y + h > H ? H : y + h;
  for (int yy = y0; yy < y1; yy++) if (x1 > x0) memset(&fb[yy * W + x0], c, (size_t)(x1 - x0));
}
void hline(int x, int y, int w, uint8_t c) { rect(x, y, w, 1, c); }
void frame(int x, int y, int w, int h, uint8_t c) {
  rect(x, y, w, 1, c); rect(x, y + h - 1, w, 1, c); rect(x, y, 1, h, c); rect(x + w - 1, y, 1, h, c);
}
void roundRect(int x, int y, int w, int h, uint8_t c) {
  rect(x + 1, y, w - 2, h, c); rect(x, y + 1, 1, h - 2, c); rect(x + w - 1, y + 1, 1, h - 2, c);
}
void circle(int cx, int cy, int r, uint8_t c) {
  for (int dy = -r; dy <= r; dy++) {
    int dx = 0; while ((dx + 1) * (dx + 1) + dy * dy <= r * r + r) dx++;
    rect(cx - dx, cy + dy, 2 * dx + 1, 1, c);
  }
}
void ring(int cx, int cy, int r, uint8_t c) {
  int x = r, y = 0, err = 1 - r;
  while (x >= y) {
    pixel(cx + x, cy + y, c); pixel(cx - x, cy + y, c); pixel(cx + x, cy - y, c); pixel(cx - x, cy - y, c);
    pixel(cx + y, cy + x, c); pixel(cx - y, cy + x, c); pixel(cx + y, cy - x, c); pixel(cx - y, cy - x, c);
    y++; if (err < 0) err += 2 * y + 1; else { x--; err += 2 * (y - x) + 1; }
  }
}
void line(int x0, int y0, int x1, int y1, uint8_t c) {
  int dx = x1 > x0 ? x1 - x0 : x0 - x1, dy = y1 > y0 ? y1 - y0 : y0 - y1;
  int sx = x0 < x1 ? 1 : -1, sy = y0 < y1 ? 1 : -1, err = dx - dy;
  for (;;) {
    pixel(x0, y0, c);
    if (x0 == x1 && y0 == y1) break;
    int e2 = 2 * err;
    if (e2 > -dy) { err -= dy; x0 += sx; }
    if (e2 < dx) { err += dx; y0 += sy; }
  }
}
void checker(int x, int y, int w, int h, uint8_t a, uint8_t b) {
  for (int yy = y; yy < y + h; yy++) for (int xx = x; xx < x + w; xx++) pixel(xx, yy, ((xx + yy) & 1) ? a : b);
}
void blit(const Sprite& s, int x, int y, bool flipx, bool flipy) {
  for (int sy = 0; sy < s.h; sy++) {
    int dy = y + (flipy ? s.h - 1 - sy : sy);
    if ((unsigned)dy >= H) continue;
    const uint8_t* row = s.px + sy * s.w;
    for (int sx = 0; sx < s.w; sx++) {
      uint8_t c = row[sx];
      if (c == C_T) continue;
      int dx = x + (flipx ? s.w - 1 - sx : sx);
      if ((unsigned)dx < W) fb[dy * W + dx] = c;
    }
  }
}
void blitTint(const Sprite& s, int x, int y, uint8_t col, bool flipx) {
  for (int sy = 0; sy < s.h; sy++) for (int sx = 0; sx < s.w; sx++) {
    if (s.px[sy * s.w + sx] == C_T) continue;
    pixel(x + (flipx ? s.w - 1 - sx : sx), y + sy, col);
  }
}
void blitScaled(const Sprite& s, int x, int y, int scale, bool flipx) {
  for (int sy = 0; sy < s.h; sy++) for (int sx = 0; sx < s.w; sx++) {
    uint8_t c = s.px[sy * s.w + sx];
    if (c == C_T) continue;
    int ox = flipx ? s.w - 1 - sx : sx;
    rect(x + ox * scale, y + sy * scale, scale, scale, c);
  }
}

int glyphWidth(char ch) {
  fontInit();
  unsigned char u = (unsigned char)ch; if (u >= 128) u = '?';
  return g_right[u] - g_left[u] + 1;
}
int textWidth(const char* s, int scale) {
  int w = 0;
  for (; *s; s++) w += (glyphWidth(*s) + 1) * scale;
  return w > 0 ? w - scale : 0;
}
TextBox textLog[64]; int textLogCount = 0; bool textLogEnabled = false;
static void logText(int x, int y, const char* s, uint8_t c, int scale) {
  if (!textLogEnabled || textLogCount >= 64 || !*s) return;
  int w = textWidth(s, scale), h = 8 * scale;
  // dominant background color under the text box, sampled before the glyphs are drawn
  int hist[256] = {0};
  for (int yy = y; yy < y + h; yy++) for (int xx = x; xx < x + w; xx++) if ((unsigned)xx < W && (unsigned)yy < H) hist[fb[yy * W + xx]]++;
  int bg = 0; for (int i = 0; i < 256; i++) if (hist[i] > hist[bg]) bg = i;
  textLog[textLogCount++] = {(int16_t)x, (int16_t)y, (int16_t)w, (int16_t)h, c, (uint8_t)bg};
}
int text(int x, int y, const char* s, uint8_t c, int scale) {
  fontInit();
  logText(x, y, s, c, scale);
  for (; *s; s++) {
    unsigned char u = (unsigned char)*s; if (u >= 128) u = '?';
    int l = g_left[u], r = g_right[u];
    for (int row = 0; row < 8; row++) {
      uint8_t bits = (uint8_t)font8x8_basic[u][row];
      for (int gx = l; gx <= r; gx++) if (bits & (1 << gx)) {
        if (scale == 1) pixel(x + gx - l, y + row, c);
        else rect(x + (gx - l) * scale, y + row * scale, scale, scale, c);
      }
    }
    x += (r - l + 2) * scale;
  }
  return x;
}
void textCentered(int cx, int y, const char* s, uint8_t c, int scale) { text(cx - textWidth(s, scale) / 2, y, s, c, scale); }
void textShadow(int x, int y, const char* s, uint8_t c, uint8_t shadow, int scale) { text(x + scale, y + scale, s, shadow, scale); text(x, y, s, c, scale); }
void textCenteredShadow(int cx, int y, const char* s, uint8_t c, uint8_t shadow, int scale) {
  int x = cx - textWidth(s, scale) / 2; textShadow(x, y, s, c, shadow, scale);
}

int wrap(const char* s, int maxw, char lines[][40], int maxLines, int scale) {
  int n = 0; char cur[40]; int curLen = 0; cur[0] = 0;
  const char* p = s;
  while (*p && n < maxLines) {
    if (*p == '\n') { strcpy(lines[n++], cur); curLen = 0; cur[0] = 0; p++; continue; }
    const char* e = p; while (*e && *e != ' ' && *e != '\n') e++;
    char word[40]; int wl = (int)(e - p); if (wl > 39) wl = 39; memcpy(word, p, (size_t)wl); word[wl] = 0;
    char trial[80]; if (curLen) snprintf(trial, sizeof trial, "%s %s", cur, word); else snprintf(trial, sizeof trial, "%s", word);
    if (textWidth(trial, scale) <= maxw || curLen == 0) { strncpy(cur, trial, 39); cur[39] = 0; curLen = (int)strlen(cur); }
    else { strcpy(lines[n++], cur); strncpy(cur, word, 39); cur[39] = 0; curLen = (int)strlen(cur); }
    p = e; while (*p == ' ') p++;
  }
  if (curLen && n < maxLines) strcpy(lines[n++], cur);
  return n;
}
}  // namespace gfx
