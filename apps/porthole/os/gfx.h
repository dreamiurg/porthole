// Tiny indexed-color 2D renderer for a 160x160 logical screen.
#pragma once
#include <stdint.h>
#include "palette.h"

namespace gfx {
constexpr int W = 160, H = 160, CX = 80, CY = 80, R = 80;
extern uint8_t fb[W * H];

struct Sprite { uint8_t w, h; const uint8_t* px; };  // px: palette index or C_T

void clear(uint8_t c);
inline void pixel(int x, int y, uint8_t c) { if ((unsigned)x < W && (unsigned)y < H) fb[y * W + x] = c; }
void rect(int x, int y, int w, int h, uint8_t c);
void frame(int x, int y, int w, int h, uint8_t c);
void roundRect(int x, int y, int w, int h, uint8_t c);     // filled, 1px corner cut
void circle(int cx, int cy, int r, uint8_t c);
void ring(int cx, int cy, int r, uint8_t c);
void line(int x0, int y0, int x1, int y1, uint8_t c);
void hline(int x, int y, int w, uint8_t c);
void checker(int x, int y, int w, int h, uint8_t a, uint8_t b);
void blit(const Sprite& s, int x, int y, bool flipx = false, bool flipy = false);
void blitTint(const Sprite& s, int x, int y, uint8_t col, bool flipx = false);
void blitScaled(const Sprite& s, int x, int y, int scale, bool flipx = false);

// Proportional 8px font (glyphs trimmed to their ink width). scale 1 or 2.
int  glyphWidth(char ch);
int  textWidth(const char* s, int scale = 1);
int  text(int x, int y, const char* s, uint8_t c, int scale = 1);       // returns end x
void textCentered(int cx, int y, const char* s, uint8_t c, int scale = 1);
void textShadow(int x, int y, const char* s, uint8_t c, uint8_t shadow, int scale = 1);
void textCenteredShadow(int cx, int y, const char* s, uint8_t c, uint8_t shadow, int scale = 1);
// Word-wrap into lines of at most maxw pixels. Returns line count; fills lines[] with copies.
int  wrap(const char* s, int maxw, char lines[][40], int maxLines, int scale = 1);

// Audit log of drawn text (bounding boxes + color) so a checker can spot clipping and poor contrast. Boxes are logical
// px. color/bg are palette indices, or RGB888 when rgb (RGB565 text, os/font.h, logs its covering box).
struct TextBox { int16_t x, y, w, h; uint32_t color, bg; bool rgb; };
extern TextBox textLog[64]; extern int textLogCount; extern bool textLogEnabled;
inline void textLogReset() { textLogCount = 0; }

// Is (x,y) inside the round visible area?
inline bool inCircle(int x, int y, int margin = 0) {
  int dx = x - CX, dy = y - CY; int r = R - margin; return dx * dx + dy * dy <= r * r;
}
}  // namespace gfx
