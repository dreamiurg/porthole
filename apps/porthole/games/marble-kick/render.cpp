#include "render.h"
#include <math.h>
#include "gfx.h"
#include "gfx565.h"
#include "physics.h"

namespace marble::paint {
namespace {
constexpr uint16_t c565(uint32_t c) { return (uint16_t)((c >> 8 & 0xF800) | (c >> 5 & 0x07E0) | (c >> 3 & 0x001F)); }
constexpr uint32_t dim(uint32_t c) {   // the same wood, felt or chalk in shade: 70% of each channel, baked, not blended
  return ((c >> 16 & 255) * 7 / 10) << 16 | ((c >> 8 & 255) * 7 / 10) << 8 | (c & 255) * 7 / 10;
}
// The tray's materials, lit and in shade.
enum Mat : uint8_t { WALNUT_EDGE, WALNUT, MAPLE, GRAIN, LIP, FELT_A, FELT_B, CHALK, NET, NET_LINE, MAT_COUNT };
constexpr uint32_t MAT_RGB[MAT_COUNT] = {0x3E2615, 0x5A3A22, 0xE3BD83, 0xC99E62, 0xB8915C,
                                         0x2F7D4F, 0x3A8E5C, 0xEFE7D2, 0x1C2E22, 0x8FA08E};
struct Shades { uint16_t c[MAT_COUNT][2]; };
constexpr Shades makeShades() {
  Shades s{};
  for (int i = 0; i < MAT_COUNT; i++) { s.c[i][0] = c565(MAT_RGB[i]); s.c[i][1] = c565(dim(MAT_RGB[i])); }
  return s;
}
constexpr Shades SHADES = makeShades();
constexpr uint16_t BRASS = c565(0xD9A441), BRASS_DARK = c565(0xA67A24), BRASS_HI = c565(0xF2CF7A);
constexpr uint16_t PEG = c565(0xB5452F), PEG_HI = c565(0xD8664B), PEG_DARK = c565(0x86301F), SHINE = c565(0xF6E3C8);
constexpr uint16_t BALL = c565(0xF7F2E6), BALL_EDGE = c565(0xC9BFA8), PENT = c565(0x2A2420), WHITE = c565(0xFFFFFF);
constexpr uint16_t LIQUID = c565(0xD4E2A0), LIQUID_DARK = c565(0xA7BA72), BUBBLE = c565(0xFBFBF1), BUBBLE_EDGE = c565(0x9FB36A);
constexpr uint32_t FACE_RGB = 0xF0D5A6;
constexpr uint16_t FACE = c565(FACE_RGB), SIDE = c565(0xB8915C);
constexpr uint32_t INK_RGB[3] = {0x3E2615, 0x7A2418, 0x1E5234};   // walnut, dark lacquer, dark felt: 6.5:1 or more on FACE

// Rings of the tray from the outside in: ring i covers radii from RINGS[i + 1].r up to RINGS[i].r. The first also
// covers the panel's corners; the ones outside PITCH_R are the rim, where the goal's net is cut in.
struct Ring { int16_t r; int8_t mat; };   // mat < 0: the felt (striped, and the halfway line inside the touchline)
constexpr int FELT = -1, FELT_OUT = -2, WALL_SHADE = -3;
constexpr Ring RINGS[] = {{2000, WALNUT_EDGE}, {236, WALNUT}, {217, WALNUT_EDGE}, {214, MAPLE}, {209, GRAIN}, {207, MAPLE},
                          {201, GRAIN}, {199, MAPLE}, {195, GRAIN}, {194, MAPLE}, {191, LIP}, {PITCH_R, WALL_SHADE},
                          {181, FELT_OUT}, {174, CHALK}, {171, FELT}, {60, CHALK}, {57, FELT}, {6, CHALK}};
constexpr int NRINGS = sizeof RINGS / sizeof RINGS[0];

Box clip_ = FULL;
int isqrt(int n) {   // floor(sqrt(n)), -1 for a negative n
  if (n < 0) return -1;
  int r = (int)sqrtf((float)n);
  while (r * r > n) r--;
  while ((r + 1) * (r + 1) <= n) r++;
  return r;
}
void span(int y, int x0, int x1, uint16_t c) {   // x1 exclusive
  if (y < clip_.y0 || y >= clip_.y1) return;
  if (x0 < clip_.x0) x0 = clip_.x0;
  if (x1 > clip_.x1) x1 = clip_.x1;
  if (x1 > x0) gfx565::hline(x0, y, x1 - x0, c);
}
void rows(int& y0, int& y1) { if (y0 < clip_.y0) y0 = clip_.y0; if (y1 > clip_.y1) y1 = clip_.y1; }   // y1 exclusive
void disc(int cx, int cy, int r, uint16_t c) {
  int y0 = cy - r, y1 = cy + r + 1;
  rows(y0, y1);
  for (int y = y0; y < y1; y++) { const int h = isqrt(r * r + r - (y - cy) * (y - cy)); span(y, cx - h, cx + h + 1, c); }
}
// A convex polygon, vertices in order (x, y pairs).
void poly(const int* p, int n, uint16_t c) {
  int y0 = p[1], y1 = p[1];
  for (int i = 1; i < n; i++) { if (p[2 * i + 1] < y0) y0 = p[2 * i + 1]; if (p[2 * i + 1] > y1) y1 = p[2 * i + 1]; }
  y1++;
  rows(y0, y1);
  for (int y = y0; y < y1; y++) {
    float l = 1e9f, r = -1e9f;
    for (int i = 0; i < n; i++) {
      const int* a = p + 2 * i; const int* b = p + 2 * ((i + 1) % n);
      if (a[1] == b[1] || y < (a[1] < b[1] ? a[1] : b[1]) || y > (a[1] > b[1] ? a[1] : b[1])) continue;
      const float x = a[0] + (float)(y - a[1]) * (b[0] - a[0]) / (b[1] - a[1]);
      if (x < l) l = x;
      if (x > r) r = x;
    }
    if (r >= l) span(y, (int)lroundf(l), (int)lroundf(r) + 1, c);
  }
}
void roundBox(int x, int y, int w, int h, int r, uint16_t c) {
  int y0 = y, y1 = y + h;
  rows(y0, y1);
  for (int yy = y0; yy < y1; yy++) {
    const int k = yy - y < r ? r - (yy - y) : yy - (y + h - 1 - r) > 0 ? yy - (y + h - 1 - r) : 0;   // rows into a corner
    const int in = k ? r - isqrt(r * r - k * k) : 0;
    span(yy, x + in, x + w - in, c);
  }
}

uint16_t felt(int y, int ring, bool shade) {
  const bool line = ring == FELT && y >= 239 && y <= 241;   // the halfway line, inside the touchline only
  const int m = line ? CHALK : (y / 40) % 2 ? FELT_B : FELT_A;
  return SHADES.c[m][shade || ring == WALL_SHADE];
}
// One run of one ring's material. In the rim above the middle, the goal's mouth shows the net instead.
void fill(int y, int x0, int x1, int ring, bool shade, int goalHalf) {
  if (x0 < clip_.x0) x0 = clip_.x0;
  if (x1 > clip_.x1) x1 = clip_.x1;
  if (x1 <= x0) return;
  const Ring& g = RINGS[ring];
  if (g.mat < 0) { span(y, x0, x1, felt(y, g.mat, shade)); return; }
  const int gl = 240 - goalHalf, gr = 240 + goalHalf;
  if (g.r <= PITCH_R || y >= 240 || x1 <= gl || x0 >= gr) { span(y, x0, x1, SHADES.c[g.mat][shade]); return; }
  if (x0 < gl) span(y, x0, gl, SHADES.c[g.mat][shade]);
  if (x1 > gr) span(y, gr, x1, SHADES.c[g.mat][shade]);
  const bool rowLine = y % 9 == 0;
  for (int x = x0 > gl ? x0 : gl; x < (x1 < gr ? x1 : gr); x++)   // the net: a 9 px mesh
    gfx565::pixel(x, y, SHADES.c[rowLine || (x - 240 + 900) % 9 == 0 ? NET_LINE : NET][shade]);
}
// The tray along row y from x0 to x1: each ring's part of the row, left and right of the next ring in, never twice.
void trayRow(int y, int x0, int x1, bool shade, int goalHalf) {
  const int dy = y - 240;
  int h = 1000;
  for (int i = 0; i < NRINGS; i++) {
    const int in = i + 1 < NRINGS ? isqrt(RINGS[i + 1].r * RINGS[i + 1].r - dy * dy - 1) : -1;
    if (in < 0) { fill(y, x0 > 240 - h ? x0 : 240 - h, x1 < 241 + h ? x1 : 241 + h, i, shade, goalHalf); return; }
    fill(y, x0 > 240 - h ? x0 : 240 - h, x1 < 240 - in ? x1 : 240 - in, i, shade, goalHalf);
    fill(y, x0 > 241 + in ? x0 : 241 + in, x1 < 241 + h ? x1 : 241 + h, i, shade, goalHalf);
    h = in;
  }
}

// 5x7 glyphs for the letter blocks, a row per byte, bit 4 the leftmost column.
struct Glyph { char c; uint8_t rows[7]; };
constexpr Glyph GLYPHS[] = {
  {'0', {14, 17, 19, 21, 25, 17, 14}}, {'1', {4, 12, 4, 4, 4, 4, 14}},   {'2', {14, 17, 1, 2, 4, 8, 31}},
  {'3', {30, 1, 1, 14, 1, 1, 30}},     {'4', {2, 6, 10, 18, 31, 2, 2}},  {'5', {31, 16, 30, 1, 1, 17, 14}},
  {'6', {6, 8, 16, 30, 17, 17, 14}},   {'7', {31, 1, 2, 4, 8, 8, 8}},    {'8', {14, 17, 17, 14, 17, 17, 14}},
  {'9', {14, 17, 17, 15, 1, 2, 12}},   {'G', {14, 17, 16, 23, 17, 17, 15}}, {'O', {14, 17, 17, 17, 17, 17, 14}},
  {'A', {14, 17, 17, 31, 17, 17, 17}}, {'L', {16, 16, 16, 16, 16, 16, 31}}, {'!', {4, 4, 4, 4, 4, 0, 4}},
};
const Glyph* glyph(char c) {
  for (const Glyph& g : GLYPHS) if (g.c == c) return &g;
  return nullptr;
}
// The audit's view of a glyph (gfx::textLog, logical px): its ink box, the ink and the face it sits on.
void logText(int x, int y, int w, int h, uint32_t ink, uint32_t bg) {
  if (!gfx::textLogEnabled || gfx::textLogCount >= (int)(sizeof gfx::textLog / sizeof *gfx::textLog)) return;
  gfx::textLog[gfx::textLogCount++] = {(int16_t)(x / 3), (int16_t)(y / 3), (int16_t)((x + w + 2) / 3 - x / 3),
                                       (int16_t)((y + h + 2) / 3 - y / 3), ink, bg, true};
}
void drawGlyph(const Glyph& g, int x, int y, int cell, uint32_t ink, uint32_t bg) {
  for (int r = 0; r < 7; r++)
    for (int c = 0; c < 5; c++)
      if (g.rows[r] >> (4 - c) & 1) roundBox(x + c * cell, y + r * cell, cell, cell, 0, c565(ink));
  logText(x, y, 5 * cell, 7 * cell, ink, bg);
}
}  // namespace

Box unite(const Box& a, const Box& b) {
  if (empty(a)) return b;
  if (empty(b)) return a;
  return {a.x0 < b.x0 ? a.x0 : b.x0, a.y0 < b.y0 ? a.y0 : b.y0, a.x1 > b.x1 ? a.x1 : b.x1, a.y1 > b.y1 ? a.y1 : b.y1};
}
void clip(const Box& b) {
  clip_ = {b.x0 < 0 ? 0 : b.x0, b.y0 < 0 ? 0 : b.y0, b.x1 > 480 ? 480 : b.x1, b.y1 > 480 ? 480 : b.y1};
}

void tray(int goalHalf) {
  for (int y = clip_.y0; y < clip_.y1; y++) trayRow(y, clip_.x0, clip_.x1, false, goalHalf);
}
void shadow(int x, int y, int r, int goalHalf) {
  const int cx = x + SHADOW_DX, cy = y + SHADOW_DY;
  int y0 = cy - r, y1 = cy + r + 1;
  rows(y0, y1);
  for (int yy = y0; yy < y1; yy++) {
    const int h = isqrt(r * r + r - (yy - cy) * (yy - cy));
    trayRow(yy, cx - h < clip_.x0 ? clip_.x0 : cx - h, cx + h + 1 > clip_.x1 ? clip_.x1 : cx + h + 1, true, goalHalf);
  }
}
void peg(int x, int y, int r) {
  disc(x, y, r, PEG_DARK);
  disc(x - 1, y - 1, r - 3, PEG);
  disc(x - r / 4, y - r / 4, r / 2, PEG_HI);
  disc(x - r * 2 / 5, y - r * 2 / 5, r / 7 + 1, SHINE);
}
void post(int x, int y) {
  disc(x, y, POST_R, BRASS_DARK);
  disc(x - 1, y - 1, POST_R - 2, BRASS);
  disc(x - 2, y - 2, 2, BRASS_HI);
}
void ball(int x, int y) {
  disc(x, y, BALL_R, BALL_EDGE);
  disc(x - 1, y - 1, BALL_R - 2, BALL);
  int p[10];
  for (int i = 0; i < 5; i++) {
    const float a = -1.5708f + i * 1.2566f;
    p[2 * i] = x + (int)lroundf(cosf(a) * 8); p[2 * i + 1] = y + (int)lroundf(sinf(a) * 8);
  }
  poly(p, 5, PENT);
  disc(x - 8, y - 9, 3, WHITE);
}
void knob(int x, int y) {
  disc(x, y, KNOB_R, BRASS_DARK);
  disc(x - 1, y - 1, KNOB_R - 3, BRASS);
  disc(x - 10, y - 11, 5, BRASS_HI);
  const int arrow[] = {x - 15, y + 1, x + 8, y - 13, x + 8, y + 15};
  poly(arrow, 3, c565(INK_RGB[0]));
}
void coin(int x, int y, int number) {
  disc(x, y, 22, BRASS_DARK);
  disc(x - 1, y - 1, 19, BRASS);
  const int cell = 4, digits = number >= 10 ? 2 : 1, w = digits * 5 * cell + (digits - 1) * cell;
  const Glyph* tens = glyph((char)('0' + number / 10 % 10));
  const Glyph* ones = glyph((char)('0' + number % 10));
  if (digits == 2) drawGlyph(*tens, x - w / 2, y - 14, cell, INK_RGB[0], 0xD9A441);
  drawGlyph(*ones, x + w / 2 - 5 * cell, y - 14, cell, INK_RGB[0], 0xD9A441);
}
void playButton(int x, int y) {
  disc(x, y, 40, PEG_DARK);
  disc(x - 1, y - 1, 37, PEG);
  disc(x - 13, y - 14, 8, PEG_HI);
  const int arrow[] = {x - 10, y - 19, x + 21, y, x - 10, y + 19};
  poly(arrow, 3, BALL);
}
void flag(int x, int y, int dir) {
  const int cloth[] = {x, y - 2, x + dir * 56, y - 22, x, y - 42};
  poly(cloth, 3, PEG);
  const int stripe[] = {x, y - 15, x + dir * 22, y - 22, x, y - 29};
  poly(stripe, 3, BALL);
  post(x, y);
}
void vial(int x, int y, int bubbleDx, int bubbleDy) {
  disc(x, y, VIAL_R, BRASS_DARK);
  disc(x - 1, y - 1, VIAL_R - 4, BRASS);
  disc(x, y, 64, LIQUID);
  disc(x, y, BUBBLE_R + 6, LIQUID_DARK);
  disc(x, y, BUBBLE_R + 3, LIQUID);
  for (int d = -1; d <= 1; d++) { span(y + d, x - 56, x - BUBBLE_R - 6, LIQUID_DARK); span(y + d, x + BUBBLE_R + 7, x + 57, LIQUID_DARK); }
  int y0 = y - 56, y1 = y + 57;
  rows(y0, y1);
  for (int yy = y0; yy < y1; yy++) if (yy < y - BUBBLE_R - 6 || yy > y + BUBBLE_R + 6) span(yy, x - 1, x + 2, LIQUID_DARK);
  disc(x + bubbleDx, y + bubbleDy, BUBBLE_R + 2, BUBBLE_EDGE);
  disc(x + bubbleDx, y + bubbleDy, BUBBLE_R, BUBBLE);
  disc(x + bubbleDx - 6, y + bubbleDy - 6, 4, WHITE);
}
void blocks(const char* s, int x, int y, int cell) {
  int n = 0;
  while (s[n]) n++;
  const int w = 5 * cell + 2 * cell, gap = cell;
  int bx = x - (n * w + (n - 1) * gap) / 2;
  for (int i = 0; i < n; i++, bx += w + gap) {
    const Glyph* g = glyph(s[i]);
    if (!g) continue;
    roundBox(bx, y + cell, w, 7 * cell + 2 * cell, cell * 2, SIDE);   // the block's side, below its face
    roundBox(bx, y, w, 7 * cell + 2 * cell, cell * 2, FACE);
    drawGlyph(*g, bx + cell, y + cell, cell, INK_RGB[i % 3], FACE_RGB);
  }
}
}  // namespace marble::paint
