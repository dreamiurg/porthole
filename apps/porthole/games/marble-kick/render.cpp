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
constexpr uint32_t BRASS_RGB = 0xD9A441;
constexpr uint16_t BRASS = c565(BRASS_RGB), BRASS_DARK = c565(0xA67A24), BRASS_HI = c565(0xF2CF7A);
constexpr uint16_t PEG = c565(0xB5452F), PEG_HI = c565(0xD8664B), PEG_DARK = c565(0x86301F), SHINE = c565(0xF6E3C8);
constexpr uint16_t BALL = c565(0xF7F2E6), BALL_EDGE = c565(0xC9BFA8), PENT = c565(0x2A2420), WHITE = c565(0xFFFFFF);
constexpr int CX = gfx565::CX, CY = gfx565::CY;   // the tray's center
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
void roundBox(const Box& b, int r, uint16_t c) {
  int y0 = b.y0, y1 = b.y1;
  rows(y0, y1);
  for (int y = y0; y < y1; y++) {
    const int k = y - b.y0 < r ? r - (y - b.y0) : y - (b.y1 - 1 - r) > 0 ? y - (b.y1 - 1 - r) : 0;   // rows into a corner
    const int in = k ? r - isqrt(r * r - k * k) : 0;
    span(y, b.x0 + in, b.x1 - in, c);
  }
}

uint16_t felt(int y, int ring, bool shade) {
  const bool line = ring == FELT && y >= CY - 1 && y <= CY + 1;   // the halfway line, inside the touchline only
  const int m = line ? CHALK : (y / 40) % 2 ? FELT_B : FELT_A;
  return SHADES.c[m][shade || ring == WALL_SHADE];
}
// A row of the tray being painted: lit or in shade, and how wide the goal's mouth is.
struct Row { int y; bool shade; int goalHalf; };
void net(const Row& row, int x0, int x1) {   // the goal's net: a 9 px mesh
  if (x0 < clip_.x0) x0 = clip_.x0;
  if (x1 > clip_.x1) x1 = clip_.x1;
  const bool line = row.y % 9 == 0;
  for (int x = x0; x < x1; x++) gfx565::pixel(x, row.y, SHADES.c[line || (x - CX + 900) % 9 == 0 ? NET_LINE : NET][row.shade]);
}
// One run of one ring's material, x1 exclusive. In the rim above the middle, the goal's mouth shows the net instead.
void fill(const Row& row, int x0, int x1, int ring) {
  const Ring& g = RINGS[ring];
  if (g.mat < 0) { span(row.y, x0, x1, felt(row.y, g.mat, row.shade)); return; }
  const uint16_t c = SHADES.c[g.mat][row.shade];
  const int gl = CX - row.goalHalf, gr = CX + row.goalHalf;
  if (g.r <= PITCH_R || row.y >= CY || x1 <= gl || x0 >= gr) { span(row.y, x0, x1, c); return; }
  span(row.y, x0, gl, c);
  span(row.y, gr, x1, c);
  net(row, x0 > gl ? x0 : gl, x1 < gr ? x1 : gr);
}
// The tray along a row from x0 to x1: each ring's part of the row, left and right of the next ring in, never twice.
void trayRow(const Row& row, int x0, int x1) {
  const int dy = row.y - CY;
  int h = 1000;
  for (int i = 0; i < NRINGS; i++) {
    const int in = i + 1 < NRINGS ? isqrt(RINGS[i + 1].r * RINGS[i + 1].r - dy * dy - 1) : -1;
    if (in < 0) { fill(row, x0 > CX - h ? x0 : CX - h, x1 < CX + 1 + h ? x1 : CX + 1 + h, i); return; }
    fill(row, x0 > CX - h ? x0 : CX - h, x1 < CX - in ? x1 : CX - in, i);
    fill(row, x0 > CX + 1 + in ? x0 : CX + 1 + in, x1 < CX + 1 + h ? x1 : CX + 1 + h, i);
    h = in;
  }
}

// 5x7 glyphs for the letter blocks, a row per byte, bit 4 the leftmost column.
struct Glyph { char c; uint8_t rows[7]; };
constexpr Glyph GLYPHS[] = {
  {' ', {0, 0, 0, 0, 0, 0, 0}},   // blank: what any other character draws
  {'0', {14, 17, 19, 21, 25, 17, 14}}, {'1', {4, 12, 4, 4, 4, 4, 14}},   {'2', {14, 17, 1, 2, 4, 8, 31}},
  {'3', {30, 1, 1, 14, 1, 1, 30}},     {'4', {2, 6, 10, 18, 31, 2, 2}},  {'5', {31, 16, 30, 1, 1, 17, 14}},
  {'6', {6, 8, 16, 30, 17, 17, 14}},   {'7', {31, 1, 2, 4, 8, 8, 8}},    {'8', {14, 17, 17, 14, 17, 17, 14}},
  {'9', {14, 17, 17, 15, 1, 2, 12}},   {'G', {14, 17, 16, 23, 17, 17, 15}}, {'O', {14, 17, 17, 17, 17, 17, 14}},
  {'A', {14, 17, 17, 31, 17, 17, 17}}, {'L', {16, 16, 16, 16, 16, 16, 31}}, {'!', {4, 4, 4, 4, 4, 0, 4}},
};
const Glyph& glyph(char c) {
  for (const Glyph& g : GLYPHS) if (g.c == c) return g;
  return GLYPHS[0];
}
// How a glyph is drawn: its cell size in px, its ink, and the face under it (both RGB888, for the UI audit).
struct Pen { int cell; uint32_t ink, bg; };
// The audit's view of a glyph (gfx::textLog, logical px, rounded outward): its ink box and colors.
void logText(const Box& b, const Pen& p) {
  if (!gfx::textLogEnabled || gfx::textLogCount >= (int)(sizeof gfx::textLog / sizeof *gfx::textLog)) return;
  gfx::textLog[gfx::textLogCount++] = {(int16_t)(b.x0 / 3), (int16_t)(b.y0 / 3), (int16_t)((b.x1 + 2) / 3 - b.x0 / 3),
                                       (int16_t)((b.y1 + 2) / 3 - b.y0 / 3), p.ink, p.bg, true};
}
void drawGlyph(const Glyph& g, int x, int y, const Pen& p) {
  const int s = p.cell;
  for (int r = 0; r < 7; r++)
    for (int c = 0; c < 5; c++)
      if (g.rows[r] >> (4 - c) & 1) roundBox({x + c * s, y + r * s, x + c * s + s, y + r * s + s}, 0, c565(p.ink));
  logText({x, y, x + 5 * s, y + 7 * s}, p);
}
}  // namespace

Box unite(const Box& a, const Box& b) {
  if (empty(a)) return b;
  if (empty(b)) return a;
  return {a.x0 < b.x0 ? a.x0 : b.x0, a.y0 < b.y0 ? a.y0 : b.y0, a.x1 > b.x1 ? a.x1 : b.x1, a.y1 > b.y1 ? a.y1 : b.y1};
}
void clip(const Box& b) {
  clip_ = {b.x0 < 0 ? 0 : b.x0, b.y0 < 0 ? 0 : b.y0, b.x1 > gfx565::W ? gfx565::W : b.x1, b.y1 > gfx565::H ? gfx565::H : b.y1};
}

void tray(int goalHalf) {
  for (int y = clip_.y0; y < clip_.y1; y++) trayRow({y, false, goalHalf}, clip_.x0, clip_.x1);
}
void shadow(int x, int y, int r, int goalHalf) {
  const int cx = x + SHADOW_DX, cy = y + SHADOW_DY;
  int y0 = cy - r, y1 = cy + r + 1;
  rows(y0, y1);
  for (int yy = y0; yy < y1; yy++) {
    const int h = isqrt(r * r + r - (yy - cy) * (yy - cy));
    trayRow({yy, true, goalHalf}, cx - h < clip_.x0 ? clip_.x0 : cx - h, cx + h + 1 > clip_.x1 ? clip_.x1 : cx + h + 1);
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
void ball(int x, int y, int r) {
  disc(x, y, r, BALL_EDGE);
  disc(x - 1, y - 1, r - 2, BALL);
  int p[10];
  for (int i = 0; i < 5; i++) {   // the dark pentagon, point up
    const float a = -1.5708f + i * 1.2566f;
    p[2 * i] = x + (int)lroundf(cosf(a) * r * 0.4f); p[2 * i + 1] = y + (int)lroundf(sinf(a) * r * 0.4f);
  }
  poly(p, 5, PENT);
  disc(x - r * 2 / 5, y - r * 9 / 20, r / 7 + 1, WHITE);
}
// A house (the device's own sign for home) carved in walnut into a brass knob. Pressed, it sinks 2 px into its shadow;
// held on the Play page, a ring of chalk dots fills around it clockwise from the top.
void knob(int x, int y, bool pressed, float hold) {
  if (pressed) y += 2;
  disc(x, y, KNOB_R, BRASS_DARK);
  disc(x - 1, y - 1, KNOB_R - 3, BRASS);
  disc(x - 13, y - 13, 4, BRASS_HI);
  const int roof[] = {x - 16, y, x, y - 15, x + 16, y};
  poly(roof, 3, c565(INK_RGB[0]));
  roundBox({x - 10, y, x + 11, y + 14}, 0, c565(INK_RGB[0]));
  roundBox({x - 3, y + 5, x + 4, y + 14}, 0, BRASS_DARK);   // the door
  for (int i = 0; hold > 0 && i < 24; i++) {
    const float a = -1.5708f + i * 0.2618f;
    disc(x + (int)lroundf(cosf(a) * (KNOB_R + 7)), y + (int)lroundf(sinf(a) * (KNOB_R + 7)), 3,
         i < hold * 24 ? SHADES.c[CHALK][0] : SHADES.c[WALNUT_EDGE][0]);
  }
}
void coin(int x, int y, int number, int r) {
  disc(x, y, r, BRASS_DARK);
  disc(x - 1, y - 1, r - 3, BRASS);
  const int cell = r >= 30 ? 5 : 4, digits = number >= 10 ? 2 : 1, w = digits * 5 * cell + (digits - 1) * cell;
  if (digits == 2) drawGlyph(glyph((char)('0' + number / 10 % 10)), x - w / 2, y - 7 * cell / 2, {cell, INK_RGB[0], BRASS_RGB});
  drawGlyph(glyph((char)('0' + number % 10)), x + w / 2 - 5 * cell, y - 7 * cell / 2, {cell, INK_RGB[0], BRASS_RGB});
}
void playButton(int x, int y, bool pressed) {
  if (pressed) y += 2;
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
// A turned maple dish with a small ball in it; the ring in the middle turns brass when the ball has settled there.
void dish(int x, int y, int ballDx, int ballDy, bool ready) {
  disc(x, y, DISH_R + 9, SHADES.c[WALNUT][0]);
  disc(x, y, DISH_R + 4, SHADES.c[GRAIN][0]);
  disc(x, y, DISH_R, SHADES.c[MAPLE][0]);
  disc(x, y, DISH_BALL_R + 10, ready ? BRASS_HI : SHADES.c[GRAIN][0]);
  disc(x, y, DISH_BALL_R + (ready ? 5 : 7), SHADES.c[MAPLE][0]);
  ball(x + ballDx, y + ballDy, DISH_BALL_R);
}
// Confetti falling through the whole glass for CONFETTI_MS after `ms` = 0, in the tray's own colors; then gone.
void confetti(uint32_t ms) {
  static const uint16_t COLORS[4] = {PEG, BRASS, SHADES.c[CHALK][0], PEG_HI};
  if (ms >= CONFETTI_MS) return;
  const float t = ms / 1000.0f;
  for (int i = 0; i < 28; i++) {
    const uint32_t h = (uint32_t)(i + 1) * 2654435761u;
    const int x = 60 + (int)(h % 360) + (int)lroundf(sinf(t * 4 + i) * (8 + (h >> 5) % 10));
    const int y = -20 - (int)((h >> 9) % 220) + (int)lroundf(t * (240 + (h >> 17) % 90));
    const int w = i & 1 ? 10 : 6, hgt = i & 1 ? 6 : 10;
    roundBox({x, y, x + w, y + hgt}, 0, COLORS[i % 4]);
  }
}
void blocks(const char* s, int x, int y, int cell) {
  int n = 0;
  while (s[n]) n++;
  const int w = 5 * cell + 2 * cell, gap = cell;
  int bx = x - (n * w + (n - 1) * gap) / 2;
  for (int i = 0; i < n; i++, bx += w + gap) {
    roundBox({bx, y + cell, bx + w, y + 10 * cell}, cell * 2, SIDE);   // the block's side, below its face
    roundBox({bx, y, bx + w, y + 9 * cell}, cell * 2, FACE);
    drawGlyph(glyph(s[i]), bx + cell, y + cell, {cell, INK_RGB[i % 3], FACE_RGB});
  }
}
}  // namespace marble::paint
