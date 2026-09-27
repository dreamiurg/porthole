#include "render.h"
#include <math.h>
#include <stdlib.h>
#include "font.h"
#include "generated/fonts.h"

namespace fc::paint {
using namespace canvas;   // the painter: span, disc, poly, roundBox, isqrt, rows, discRun, clipBox
namespace {
constexpr uint16_t c565(uint32_t c) { return (uint16_t)((c >> 8 & 0xF800) | (c >> 5 & 0x07E0) | (c >> 3 & 0x001F)); }
constexpr uint32_t dim(uint32_t c) {   // the same surface in the sun's shadow: 62% of each channel, baked, not blended
  return ((c >> 16 & 255) * 62 / 100) << 16 | ((c >> 8 & 255) * 62 / 100) << 8 | (c & 255) * 62 / 100;
}
// The ground: what shadows fall on, lit and in shade.
enum Mat : uint8_t { PAVE, JOINT, CURB, ASPHALT, CHALK, NET_BG, NET_LINE, POST, MAT_COUNT };
constexpr uint32_t MAT_RGB[MAT_COUNT] = {0xE8C791, 0xCDA66C, 0xD9D3C5, 0x4F5B67, 0xF1EDE2, 0x353E47, 0xC7CDD2, 0xFBFBF8};
struct Shades { uint16_t c[MAT_COUNT][2]; };
constexpr Shades makeShades() {
  Shades s{};
  for (int i = 0; i < MAT_COUNT; i++) { s.c[i][0] = c565(MAT_RGB[i]); s.c[i][1] = c565(dim(MAT_RGB[i])); }
  return s;
}
constexpr Shades SH = makeShades();
constexpr uint32_t SUN_RGB = 0xFFD23F, INK_RGB = 0x2A1A18, WHITE_RGB = 0xFFFFFF;
constexpr uint16_t SUN = c565(SUN_RGB), SUN_EDGE = c565(0x9C7412), INK = c565(INK_RGB), WHITE = c565(WHITE_RGB);
// The kits: shirt, its edge, and the number's ink (4.5:1 or more on the shirt).
struct KitRgb { uint32_t shirt, edge, ink; };
constexpr KitRgb KITS[4] = {{0x10857F, 0x074A47, WHITE_RGB}, {0x0A5552, 0x042B29, WHITE_RGB},
                            {0xF0735E, 0x9E3D2C, INK_RGB}, {0xA8412F, 0x5E2016, WHITE_RGB}};
constexpr uint16_t HAIR[4] = {c565(0x3B2A20), c565(0x1C1816), c565(0xD9A650), c565(0x7E3F1F)};
constexpr uint16_t HEAD_EDGE = c565(0x14100E), GLOVE = c565(0xF4F1E8);
constexpr uint16_t BALL_WHITE = c565(0xFBFBF7), BALL_EDGE = c565(0x3C3C38), BALL_PATCH = c565(0x2B2B28);
constexpr uint16_t SIGN_FACE = c565(0x2C4A5E), CUP = c565(0xF2B632), CUP_DARK = c565(0x9E6E0E);
constexpr int CURB_W = 6, LINE_W = 3, BOX_W = 80, BOX_Y = HALF_H - 58;

// A thick line from a to b, w either side.
struct Pt { int x, y; };
void bar(Pt a, Pt b, int w, uint16_t c) {
  const float ux = (float)(b.x - a.x), uy = (float)(b.y - a.y), l = fmaxf(1, sqrtf(ux * ux + uy * uy));
  const int nx = (int)lroundf(-uy / l * w), ny = (int)lroundf(ux / l * w);
  const int q[8] = {a.x + nx, a.y + ny, b.x + nx, b.y + ny, b.x - nx, b.y - ny, a.x - nx, a.y - ny};
  poly(q, 4, c);
}
// Text through font::clip, which game.cpp keeps equal to this painter's clip box. Each line logs itself for the UI
// audit (os/font.cpp).
void label(const font::Font& f, const char* s, int cx, int top, uint16_t c) {
  font::text(f, s, cx - font::textWidth(f, s) / 2, top, c);
}
// The line top that puts a digit's ink (Rubik's digits and capitals are all one height) centered on row cy.
int middled(const font::Font& f, int cy) {
  const font::Glyph& g = f.glyphs['0' - 31];
  return cy - (f.lineHeight - f.baseLine - g.ofsY - g.boxH / 2);
}

// ---- the court, one row at a time
// Half the width of a round-cornered rectangle (half sizes hw x hh, corner radius r, centered on the glass) at row
// offset dy; -1 when the row misses it.
int roundHalf(int dy, int hw, int hh, int r) {
  const int a = abs(dy);
  if (a > hh) return -1;
  if (a <= hh - r) return hw;
  const int k = a - (hh - r);
  return hw - r + isqrt(r * r - k * k);
}
struct Row { int y, x0, x1; bool shade; };   // one row being painted, [x0, x1) of it, in the sun or in shadow
void seg(const Row& row, int a, int b, int mat) {
  span(row.y, a > row.x0 ? a : row.x0, b < row.x1 ? b : row.x1, SH.c[mat][row.shade]);
}
// The row is painted in layers that do not overlap (paving, curb, the chalk edge, asphalt), so a full repaint writes
// each pixel about once: on the board every pixel written is PSRAM bandwidth.
struct Span { int a, b; };   // [a, b) of a row; empty when b <= a
constexpr Span NOTHING = {CX, CX};
Span centered(int half) { return half < 0 ? NOTHING : Span{CX - half, CX + half + 1}; }
bool has(const Span& s) { return s.b > s.a; }
void around(const Row& row, const Span& s, const Span& hole, int mat) {   // s less a hole inside it (or none)
  if (!has(hole)) { seg(row, s.a, s.b, mat); return; }
  seg(row, s.a, hole.a < s.b ? hole.a : s.b, mat);
  seg(row, hole.b > s.a ? hole.b : s.a, s.b, mat);
}
// Sandstone slabs in a running bond: 36 px courses, 48 px slabs with 2 px joints, each course shifted half a slab.
void paving(const Row& row, int a, int b) {
  if (a < row.x0) a = row.x0;
  if (b > row.x1) b = row.x1;
  const int yy = row.y + 360;
  if (b <= a || yy % 36 < 2) { seg(row, a, b, JOINT); return; }
  const int off = (yy / 36) % 2 * 24;
  int x = a;
  for (int j = a - ((a - off) % 48 + 48) % 48; x < b; j += 48) {   // slab, joint, slab...
    if (j > x) { seg(row, x, j < b ? j : b, PAVE); x = j; }
    if (x < b && x < j + 2) { seg(row, x, j + 2 < b ? j + 2 : b, JOINT); x = j + 2; }
  }
}
void lines(const Row& row, int dy, const Span& inner) {
  const int a = abs(dy);
  if (a >= BOX_Y - 1 && a <= BOX_Y + 1) seg(row, CX - BOX_W - 1, CX + BOX_W + 2, CHALK);   // the boxes by the goals
  else if (a > BOX_Y) { seg(row, CX - BOX_W - 1, CX - BOX_W + 2, CHALK); seg(row, CX + BOX_W - 1, CX + BOX_W + 2, CHALK); }
  const int ro = (int)CIRCLE_R + 1, ri = (int)CIRCLE_R - 2;   // the centre circle, the halfway line, the spot
  if (a <= ro) around(row, centered(isqrt(ro * ro - dy * dy)), a <= ri ? centered(isqrt(ri * ri - dy * dy)) : NOTHING, CHALK);
  if (a <= 1) seg(row, inner.a, inner.b, CHALK);
  if (a <= 3) seg(row, CX - isqrt(12 - dy * dy), CX + isqrt(12 - dy * dy) + 1, CHALK);
}
// A goal behind the end line: a white frame round a dark mouth with a diamond net, row by row.
Span netSpan(int dy) {
  const int a = abs(dy);
  return a > HALF_H && a <= HALF_H + NET_DEPTH + 3 ? Span{CX - GOAL_HALF - 3, CX + GOAL_HALF + 4} : NOTHING;
}
void net(const Row& row, int dy) {
  const int a = abs(dy), back = HALF_H + NET_DEPTH;
  if (!has(netSpan(dy))) return;
  if (a > back) { seg(row, CX - GOAL_HALF - 3, CX + GOAL_HALF + 4, POST); return; }
  seg(row, CX - GOAL_HALF - 3, CX - GOAL_HALF + 1, POST);
  seg(row, CX + GOAL_HALF, CX + GOAL_HALF + 4, POST);
  const Box& c = clipBox();
  if (row.y < c.y0 || row.y >= c.y1) return;
  const int x0 = row.x0 > CX - GOAL_HALF + 1 ? row.x0 : CX - GOAL_HALF + 1, x1 = row.x1 < CX + GOAL_HALF ? row.x1 : CX + GOAL_HALF;
  for (int x = x0 > c.x0 ? x0 : c.x0; x < x1 && x < c.x1; x++) {
    const int u = x - CX + 800;
    gfx565::pixel(x, row.y, SH.c[(u + a) % 8 == 0 || (u - a) % 8 == 0 ? NET_LINE : NET_BG][row.shade]);
  }
}
void courtRow(const Row& row) {
  const int dy = row.y - CY;
  const Span curb = centered(roundHalf(dy, HALF_W + CURB_W, HALF_H + CURB_W, CORNER_R + CURB_W));
  const Span court = centered(roundHalf(dy, HALF_W, HALF_H, CORNER_R)), goal = netSpan(dy);
  const Span inner = centered(roundHalf(dy, HALF_W - LINE_W, HALF_H - LINE_W, CORNER_R - LINE_W));
  const Span pave = has(curb) ? curb : goal;
  if (has(pave)) { paving(row, row.x0, pave.a); paving(row, pave.b, row.x1); }
  else paving(row, row.x0, row.x1);
  around(row, curb, has(court) ? court : goal, CURB);
  around(row, court, inner, CHALK);   // the court's edge
  if (has(inner)) { seg(row, inner.a, inner.b, ASPHALT); lines(row, dy, inner); }
  net(row, dy);
}
void shade(int y, int l, int r) {   // the court in the sun's shadow along [l, r) of row y
  const Box& c = clipBox();
  if (y < c.y0 || y >= c.y1) return;
  if (l < c.x0) l = c.x0;
  if (r > c.x1) r = c.x1;
  if (r > l) courtRow({y, l, r, true});
}
void shadeDisc(int cx, int cy, int r) {
  int y0 = cy - r, y1 = cy + r + 1;
  rows(y0, y1);
  for (int y = y0; y < y1; y++) { const Run d = discRun(cx, cy, r, y); shade(y, d.l, d.r); }
}
void shadePoly(const int* p, int n) {
  int y0, y1, l, r;
  polyRows(p, n, &y0, &y1);
  for (int y = y0; y < y1; y++) if (polyRun(p, n, y, &l, &r)) shade(y, l, r);
}

// Every glyph is Rubik Mono One's (generated/fonts.h), limited to GLYPHS; game.cpp's strings are checked against it.
constexpr const char* DIGITS[] = {"0", "1", "2", "3", "4", "5", "6", "7", "8", "9"};
const char* digit(int n) { return DIGITS[n < 0 ? 0 : n > 9 ? 9 : n]; }
void tile(const Box& b, int r, const KitRgb& k, const char* s, const font::Font& f) {   // a team's score
  roundBox({b.x0, b.y0 + 3, b.x1, b.y1 + 3}, r, c565(dim(k.shirt)));   // its edge, below it
  roundBox(b, r, c565(k.shirt));
  label(f, s, (b.x0 + b.x1) / 2, middled(f, (b.y0 + b.y1) / 2), c565(k.ink));
}
}  // namespace

void court() {
  const Box& c = clipBox();
  for (int y = c.y0; y < c.y1; y++) courtRow({y, c.x0, c.x1, false});
}
// The sun is low in the upper left: a standing player's shadow reaches well past his feet to the lower right, a
// capsule from under him to about a body and a half away.
void playerShadow(int x, int y) {
  const int ax = x + 1, ay = y + 2, bx = x + 15, by = y + 20, r = 13;
  const float ux = (float)(bx - ax), uy = (float)(by - ay), l = sqrtf(ux * ux + uy * uy);
  const int nx = (int)lroundf(-uy / l * r), ny = (int)lroundf(ux / l * r);
  const int q[8] = {ax + nx, ay + ny, bx + nx, by + ny, bx - nx, by - ny, ax - nx, ay - ny};
  int y0 = ay - r, y1 = by + r + 1;
  rows(y0, y1);
  for (int yy = y0; yy < y1; yy++) {   // convex: the run is from the leftmost to the rightmost of its three parts
    const Run da = discRun(ax, ay, r, yy), db = discRun(bx, by, r, yy);
    int l0 = 100000, r0 = -100000, a, b;
    if (da.r > da.l) { l0 = da.l; r0 = da.r; }
    if (db.r > db.l) { l0 = db.l < l0 ? db.l : l0; r0 = db.r > r0 ? db.r : r0; }
    if (polyRun(q, 4, yy, &a, &b)) { l0 = a < l0 ? a : l0; r0 = b > r0 ? b : r0; }
    if (r0 > l0) shade(yy, l0, r0);
  }
}
void ballShadow(int x, int y) { shadeDisc(x + 3, y + 4, BALL_R - 1); }
void ring(int x, int y) {
  disc(x, y, 23, SUN_EDGE);
  disc(x, y, 21, SUN);
}
// A player from above: shoulders in the kit with the number big on them, his head a bump at the edge he faces, a
// keeper's gloves at his sides; the pass target gets a sun-yellow marker over him.
void player(int x, int y, Vec face, const Look& l) {
  const KitRgb& k = KITS[l.kit];
  const int hx = x + (int)lroundf(face.x * 17), hy = y + (int)lroundf(face.y * 17);
  disc(hx, hy, 9, HEAD_EDGE);
  disc(hx, hy, 7, HAIR[l.hair & 3]);
  if (l.keeper)
    for (int s = -1; s <= 1; s += 2) {
      const int gx = x + (int)lroundf(-face.y * 17 * s), gy = y + (int)lroundf(face.x * 17 * s);
      disc(gx, gy, 6, HEAD_EDGE);
      disc(gx, gy, 4, GLOVE);
    }
  disc(x, y, 17, c565(k.edge));
  disc(x, y, 15, c565(k.shirt));
  label(FONT18, l.number, x, middled(FONT18, y), c565(k.ink));
  if (l.target) {
    const int t[6] = {x - 8, y - 31, x + 8, y - 31, x, y - 21}, in[6] = {x - 5, y - 29, x + 5, y - 29, x, y - 23};
    poly(t, 3, SUN_EDGE);
    poly(in, 3, SUN);
  }
}
void ball(int x, int y) {
  disc(x, y, BALL_R, BALL_EDGE);
  disc(x, y, BALL_R - 1, BALL_WHITE);
  disc(x - 2, y - 1, 2, BALL_PATCH);
  disc(x + 3, y + 2, 1, BALL_PATCH);
}
void aimSpot(int x, int y) {
  disc(x, y, 8, SUN_EDGE);
  disc(x, y, 6, SUN);
  disc(x, y, 2, SUN_EDGE);
}
void arrow(int x, int y, Vec dir, int length) {
  const int ax = x + (int)lroundf(dir.x * 26), ay = y + (int)lroundf(dir.y * 26);
  const int bx = x + (int)lroundf(dir.x * (26 + length)), by = y + (int)lroundf(dir.y * (26 + length));
  const int tx = x + (int)lroundf(dir.x * (40 + length)), ty = y + (int)lroundf(dir.y * (40 + length));
  const int px = (int)lroundf(-dir.y * 13), py = (int)lroundf(dir.x * 13);
  bar({ax, ay}, {bx, by}, 4, SH.c[CHALK][0]);
  const int head[6] = {bx + px, by + py, tx, ty, bx - px, by - py};
  poly(head, 3, SH.c[CHALK][0]);
}
void readyRing(int x, int y) {
  for (int i = 0; i < 16; i++) {
    const float a = i * 0.3927f;
    disc(x + (int)lroundf(cosf(a) * 30), y + (int)lroundf(sinf(a) * 30), 4, SUN);
  }
}
void scoreboard(int teal, int coral) {
  tile({BOARD_X - TILE / 2, CY - 21 - TILE, BOARD_X + TILE / 2, CY - 21}, 9, KITS[KIT_CORAL], digit(coral), FONT36);
  tile({BOARD_X - TILE / 2, CY + 21, BOARD_X + TILE / 2, CY + 21 + TILE}, 9, KITS[KIT_TEAL], digit(teal), FONT36);
}
// A chalk dial with the time played filling it clockwise from the top in dark asphalt.
void clock(int steps) {
  disc(BOARD_X, CY, CLOCK_R, INK);
  disc(BOARD_X, CY, CLOCK_R - 2, SH.c[CHALK][0]);
  for (int i = 0; i < steps && i < CLOCK_STEPS; i++) {
    const float a0 = -1.5708f + i * 0.2618f, a1 = a0 + 0.2618f, r = CLOCK_R - 2;
    const int t[6] = {BOARD_X, CY, BOARD_X + (int)lroundf(cosf(a0) * r), CY + (int)lroundf(sinf(a0) * r),
                      BOARD_X + (int)lroundf(cosf(a1) * r), CY + (int)lroundf(sinf(a1) * r)};
    poly(t, 3, SH.c[ASPHALT][0]);
  }
}
// A round street sign, white rim, dark blue face, a white arrow pointing out; pressed, it sinks 2 px; held during a
// match its rim fills with sun yellow, clockwise from the top.
void leaveSign(bool pressed, float hold) {
  const int x = SIGN_X, y = SIGN_Y + (pressed ? 2 : 0);
  shadeDisc(x + 4, y + 5, SIGN_R);
  disc(x, y, SIGN_R, WHITE);
  disc(x, y, SIGN_R - 5, SIGN_FACE);
  for (int i = 0; hold > 0 && i < (int)(hold * 24); i++) {   // game.cpp's look() rounds the same way
    const float a = -1.5708f + i * 0.2618f;
    disc(x + (int)lroundf(cosf(a) * (SIGN_R - 2.5f)), y + (int)lroundf(sinf(a) * (SIGN_R - 2.5f)), 3, SUN);
  }
  const int head[6] = {x - 15, y, x - 2, y - 12, x - 2, y + 12};
  poly(head, 3, WHITE);
  roundBox({x - 3, y - 4, x + 14, y + 5}, 0, WHITE);
}
void goSign(int x, int y, bool pressed, bool ready) {
  if (pressed) y += 2;
  const int shadow[8] = {x + 5, y - 38, x + 49, y + 6, x + 5, y + 50, x - 39, y + 6};
  const int edge[8] = {x, y - 44, x + 44, y, x, y + 44, x - 44, y};
  const int face[8] = {x, y - 38, x + 38, y, x, y + 38, x - 38, y};
  const int go[6] = {x - 10, y - 16, x + 17, y, x - 10, y + 16};
  shadePoly(shadow, 4);
  poly(edge, 4, ready ? INK : SH.c[CURB][1]);
  poly(face, 4, ready ? SUN : SH.c[CURB][0]);
  poly(go, 3, ready ? INK : SH.c[CURB][1]);
}
// The plate: a white rim round the leave sign's dark blue, its shadow on the court, sun-yellow words (6.5:1).
void banner(const char* s, int top) {
  constexpr int BANG_GAP = 7, PAD_X = 18, PAD_Y = 13, RIM = 5;
  const font::Font& f = FONT64;
  char w[8] = {};
  int n = 0;
  for (; s[n] && n < 7; n++) w[n] = s[n];
  const bool bang = n > 1 && w[n - 1] == '!';
  if (bang) w[--n] = 0;
  const font::Glyph &last = f.glyphs[(uint8_t)w[n - 1] - 31], &b = f.glyphs['!' - 31], &d = f.glyphs['0' - 31];
  const int wordW = font::textWidth(f, w), inkEnd = wordW - ((last.advW + 8) >> 4) + last.ofsX + last.boxW;
  const int width = bang ? inkEnd + BANG_GAP + b.boxW : wordW, x = CX - width / 2;
  const int inkTop = top + f.lineHeight - f.baseLine - d.ofsY - d.boxH;   // digits and capitals: one height
  const Box plate = {x - PAD_X, inkTop - PAD_Y, x + width + PAD_X, inkTop + d.boxH + PAD_Y};
  for (int y = plate.y0 + 7; y < plate.y1 + 7; y++) {   // its shadow: the same round box, 6 right and 7 down
    const int k = y - plate.y0 - 7 < 16 ? 16 - (y - plate.y0 - 7) : y - (plate.y1 + 6 - 16) > 0 ? y - (plate.y1 + 6 - 16) : 0;
    const int in = k ? 16 - isqrt(256 - k * k) : 0;
    shade(y, plate.x0 + 6 + in, plate.x1 + 6 - in);
  }
  roundBox(plate, 16, WHITE);
  roundBox({plate.x0 + RIM, plate.y0 + RIM, plate.x1 - RIM, plate.y1 - RIM}, 16 - RIM, SIGN_FACE);
  font::text(f, w, x, top, SUN);
  if (bang) font::text(f, "!", x + inkEnd + BANG_GAP - b.ofsX, top, SUN);
}
void result(int teal, int coral, bool won) {
  tile({CX - 110, CY - 48, CX - 14, CY + 48}, 16, KITS[KIT_TEAL], digit(teal), FONT64);
  tile({CX + 14, CY - 48, CX + 110, CY + 48}, 16, KITS[KIT_CORAL], digit(coral), FONT64);
  if (!won) return;
  const int x = CX, y = CY - 112;   // a cup: bowl, handles, stem, base
  disc(x - 30, y - 8, 13, CUP_DARK);
  disc(x + 30, y - 8, 13, CUP_DARK);
  disc(x - 30, y - 8, 7, SH.c[ASPHALT][0]);
  disc(x + 30, y - 8, 7, SH.c[ASPHALT][0]);
  const int bowl[8] = {x - 30, y - 30, x + 30, y - 30, x + 16, y + 8, x - 16, y + 8};
  poly(bowl, 4, CUP);
  disc(x, y + 8, 16, CUP);
  roundBox({x - 5, y + 20, x + 6, y + 34}, 0, CUP_DARK);
  roundBox({x - 20, y + 32, x + 21, y + 42}, 3, CUP);
}
}  // namespace fc::paint
