#include "render.h"
#include <math.h>
#include <stdio.h>
#include <string.h>
#include "crc32.h"
#include "canvas.h"
#include "font.h"
#include "generated/fonts.h"
#include "palette.h"
#include "scope.h"

namespace radar::paint {
using namespace canvas;
namespace {
constexpr uint16_t c565(uint32_t c) { return rgb888_to_565(c); }
constexpr uint16_t BG = c565(0x071B2E), RING_C = c565(0x2F8F72), FAINT = c565(0x1A4F4A), LETTER = c565(0xCFE8E0);
constexpr uint16_t RANGE_C = c565(0x7FD1B9), CALL = c565(0xF4F1DE), ALT = c565(0x8EC5FF), PANEL = c565(0x0E2E4A);
constexpr uint16_t ROUTE = c565(0xFFB86B);
constexpr uint16_t HOME_C = c565(0xFFD166), HOME_DOWN = c565(0xC9A24D);
constexpr uint16_t KIND_C[] = {c565(0xFFD166), c565(0x7BDFF2), c565(0xFF7B54)};   // airliner, small, military
constexpr int RANGE_X = CX + 10, RANGE_Y = CY + RING - 44;   // the range label, under the middle on the right

// The launcher icon, in the shell's indexed palette: the scope from above, an amber plane crossing it.
// k rim, n night, g rings, y the plane.
constexpr char ICON_ART[] =
  "................................" "...........kkkkkkkkkk..........." "........kkknnnggggnnnkkk........"
  ".......kknnggggggggggnnkk......." "......kknggnnnnggnnnnggnkk......" ".....knggnnnnnnggnnnnnnggnk....."
  "....knggnnnnnnnggnnnnnnnggnk...." "...kkggnnnnngggggggyynnnnggkk..." "..kkngnnnnnggnnggnnyynnnnngnkk.."
  "..kngnnnnggnnnnggnyyyygnnnngnk.." "..kngnnnngnnnnnggnyyyygnnnngnk.." ".kngnnnngnnnnnggggyyyyngnnnngnk."
  ".kngnnnggnnngggggyyyyyyggnnngnk." ".kngnnngnnnngnnggyygnyyngnnngnk." ".kggnnngnnnggnngynnggnnygnnnggk."
  ".kggggggggggggggggggggggggggggk." ".kggggggggggggggggggggggggggggk." ".kggnnngnnnggnnggnnggnnngnnnggk."
  ".kngnnngnnnngnnggnngnnnngnnngnk." ".kngnnnggnnnggggggggnnnggnnngnk." ".kngnnnngnnnnnggggnnnnngnnnngnk."
  "..kngnnnngnnnnnggnnnnngnnnngnk.." "..kngnnnnggnnnnggnnnnggnnnngnk.." "..kkngnnnnnggnnggnnggnnnnngnkk.."
  "...kkggnnnnnggggggggnnnnnggkk..." "....knggnnnnnnnggnnnnnnnggnk...." ".....knggnnnnnnggnnnnnnggnk....."
  "......kknggnnnnggnnnnggnkk......" ".......kknnggggggggggnnkk......." "........kkknnnggggnnnkkk........"
  "...........kkkkkkkkkk..........." "................................";
static_assert(sizeof ICON_ART == 32 * 32 + 1, "the launcher slot fits a 32x32 icon");
constexpr uint8_t iconColor(char c) {
  return c == 'k' ? (uint8_t)C_BLACK : c == 'n' ? (uint8_t)C_NIGHT : c == 'g' ? (uint8_t)C_DKGREEN
       : c == 'y' ? (uint8_t)C_YELLOW : C_T;
}
struct IconPx { uint8_t px[32 * 32]; };
constexpr IconPx makeIcon() {
  IconPx ic{};
  for (int i = 0; i < 32 * 32; i++) ic.px[i] = iconColor(ICON_ART[i]);
  return ic;
}
constexpr IconPx ICON_PX = makeIcon();
const gfx::Sprite ICON = {32, 32, ICON_PX.px};

struct Pt { int x, y; };
Pt along(Pt from, float screenDeg, float len) {
  return {from.x + (int)lroundf(sinf(screenDeg * DEG) * len), from.y - (int)lroundf(cosf(screenDeg * DEG) * len)};
}
void bar(Pt a, Pt b, int w, uint16_t c) {   // a thick line, w either side
  const float ux = (float)(b.x - a.x), uy = (float)(b.y - a.y), l = fmaxf(1, sqrtf(ux * ux + uy * uy));
  const int nx = (int)lroundf(-uy / l * w), ny = (int)lroundf(ux / l * w);
  const int p[] = {a.x + nx, a.y + ny, b.x + nx, b.y + ny, b.x - nx, b.y - ny, a.x - nx, a.y - ny};
  poly(p, 4, c);
}
void textAt(const font::Font& f, const char* s, int cx, int cy, uint16_t c) {   // centered on (cx, cy)
  font::text(f, s, cx - font::textWidth(f, s) / 2, cy - f.lineHeight / 2, c);
}

void clear() { roundBox(FULL, 0, BG); }

// N, E, S, W around the ring, turned so `up` is at the top.
Pt cardinal(int i, int up) { return along({CX, CY}, screenDeg(i * 90.0f, up), CARD_R); }

void scope(int up, int range) {
  clear();
  for (int k = 4; k >= 1; k--) {   // outer ring bright, inner ones faint; each a ring, outside in
    disc(CX, CY, RING * k / 4, k == 4 ? RING_C : FAINT);
    disc(CX, CY, RING * k / 4 - 2, BG);
  }
  span(CY - 1, CX - RING, CX + RING, FAINT);
  span(CY, CX - RING, CX + RING, FAINT);
  for (int y = CY - RING; y < CY + RING; y++) span(y, CX - 1, CX + 1, FAINT);
  static const char* const CARD[] = {"N", "E", "S", "W"};
  for (int i = 0; i < 4; i++) {
    const Pt p = cardinal(i, up);
    textAt(radar::FONT24, CARD[i], p.x, p.y, LETTER);
  }
  char buf[12];
  rangeText(range, buf, sizeof buf);
  font::text(radar::FONT24, buf, RANGE_X, RANGE_Y, RANGE_C);
}

// Drawn smaller than its touch area (HOME, the full 8 mm box): a small key on the scope, easy to hit.
void home(bool down) {
  const int cx = (paint::HOME.x + paint::HOME.w / 2) * 3, cy = (paint::HOME.y + paint::HOME.h / 2) * 3;
  roundBox({cx - 24, cy - 22, cx + 24, cy + 22}, 12, down ? HOME_DOWN : HOME_C);
  const int roof[] = {cx, cy - 14, cx + 14, cy - 1, cx - 14, cy - 1};   // a house, in the scope's night
  poly(roof, 3, BG);
  roundBox({cx - 9, cy - 2, cx + 9, cy + 12}, 0, BG);
  roundBox({cx - 3, cy + 4, cx + 3, cy + 12}, 0, down ? HOME_DOWN : HOME_C);
}

// A non-live scope's words, in a panel above the middle (the menu button keeps the middle, home sits low).
constexpr int NOTE_Y = CY - 92;
void note(const char* s) {
  const font::Box b = {&radar::FONT24, 300, 0, 2, font::Align::CENTER};
  const int h = font::textHeight(radar::FONT24, s, b.w, b.spacing);
  roundBox({CX - 166, NOTE_Y - h / 2 - 18, CX + 166, NOTE_Y + h / 2 + 18}, 20, PANEL);
  font::textBox(b, s, CX - b.w / 2, NOTE_Y - h / 2, CALL);
}

void menuButton() {   // the scope's middle: three bars on a small amber disc; it answers taps out to MENU_HIT
  disc(MENU_CX * 3, MENU_CY * 3, MENU_R * 3, HOME_C);
  for (int k = -1; k <= 1; k++) roundBox({CX - 9, CY + k * 6 - 1, CX + 9, CY + k * 6 + 2}, 0, BG);
}

void plane(const air::Plane& p, Placed at, int up) {
  const uint16_t c = KIND_C[p.kind <= air::K_MILITARY ? p.kind : 0];
  if (!at.inside) { disc(at.x, at.y, 6, c); return; }
  const float heading = screenDeg(p.trackDeg, up), k = p.kind == air::K_SMALL ? 0.65f : 1.0f;
  const Pt mid = {at.x, at.y}, nose = along(mid, heading, 20 * k), back = along(mid, heading, -10 * k);
  // The speed line: about a minute's flight, cut where it leaves the ring.
  Pt end = along(nose, heading, fminf(48, p.knots / 8.0f));
  for (int i = 0; i < 12 && (end.x - CX) * (end.x - CX) + (end.y - CY) * (end.y - CY) > RING * RING; i++)
    end = {(end.x + nose.x) / 2, (end.y + nose.y) / 2};
  if (p.knots > 0) bar(nose, end, 2, c);
  const Pt l = along(back, heading - 90, 8 * k), r = along(back, heading + 90, 8 * k);   // slim: the nose reads at once
  const int tri[] = {nose.x, nose.y, r.x, r.y, l.x, l.y};
  poly(tri, 3, c);
}

// Each plane's label side last repaint, by callsign, so a label stays put while its plane moves: sides[cur] is being
// written, sides[!cur] read. A plane with no flight number has no side to keep.
struct Side { uint32_t key; int8_t side; };
Side g_sides[2][air::MAX_PLANES];
int g_cur = 0, g_count[2] = {0, 0};
int lastSide(uint32_t key) {
  const int prev = !g_cur;
  for (int i = 0; i < g_count[prev]; i++) if (g_sides[prev][i].key == key) return g_sides[prev][i].side;
  return -1;
}
// Taken: what a tag must not cover. Each tag placed joins it.
struct Taken { Box r[air::MAX_PLANES * 2 + 8];   // 3 fixed, 4 letters, a symbol and a label per plane
  int n = 0; void add(Box b) { if (n < (int)(sizeof r / sizeof r[0])) r[n++] = b; } };

// A plane's label: the lines picked in the menu that it has (flight, route, altitude), one under the other.
void tag(const air::Plane& p, Placed at, const bool* show, Taken& taken) {
  char alt[12];
  altText(p.altFt, alt, sizeof alt);
  const char* line[SHOWS] = {p.call, p.route, alt};
  const uint16_t ink[SHOWS] = {CALL, ROUTE, ALT};
  const font::Font& f = radar::FONT18;
  int w = 0, n = 0;
  for (int i = 0; i < SHOWS; i++)
    if (show[i] && line[i][0]) { n++; const int lw = font::textWidth(f, line[i]); w = lw > w ? lw : w; }
  if (!n) return;
  const uint32_t key = p.call[0] ? os::crc32(p.call, strlen(p.call)) : 0;
  const Tag t = tagFor({at.x, at.y, w, n * f.lineHeight, key ? lastSide(key) : -1}, {taken.r, taken.n});
  if (!t.ok) return;
  if (key && g_count[g_cur] < air::MAX_PLANES) g_sides[g_cur][g_count[g_cur]++] = {key, (int8_t)t.side};
  taken.add({t.x - 3, t.y - 2, t.x + w + 3, t.y + n * f.lineHeight + 2});
  roundBox({t.x - 3, t.y - 1, t.x + w + 3, t.y + n * f.lineHeight + 1}, 4, BG);   // a backing: no ring or line through it
  for (int i = 0, y = t.y; i < SHOWS; i++)
    if (show[i] && line[i][0]) { font::text(f, line[i], t.x, y, ink[i]); y += f.lineHeight; }
}

void live(const View& v) {
  const air::Snapshot& s = *v.snap;
  static Placed at[air::MAX_PLANES];
  for (int i = 0; i < s.n; i++) {   // each plane flown on from its last fetch, so they glide instead of jumping
    const air::Plane& p = s.planes[i];
    const float km = p.knots * 1.852f / 3600 * (float)v.ageSec;
    at[i] = place(p.eastKm + sinf(p.trackDeg * DEG) * km, p.northKm + cosf(p.trackDeg * DEG) * km, v.up, v.range);
  }
  menuButton();   // under the planes: the one right over home stays in sight
  for (int i = 0; i < s.n; i++) plane(s.planes[i], at[i], v.up);
  if (!s.n) { note("No planes nearby right now."); return; }
  if (!v.show[SHOW_CALL] && !v.show[SHOW_ROUTE] && !v.show[SHOW_ALT]) return;
  static Taken taken;
  taken.n = 0;
  const int mb = MENU_R * 3 + 4, hx = HOME.x * 3, hy = HOME.y * 3;
  taken.add({CX - mb, CY - mb, CX + mb, CY + mb});
  taken.add({hx, hy, hx + HOME.w * 3, hy + HOME.h * 3});
  char buf[12];
  rangeText(v.range, buf, sizeof buf);
  taken.add({RANGE_X, RANGE_Y, RANGE_X + font::textWidth(radar::FONT24, buf), RANGE_Y + radar::FONT24.lineHeight});
  for (int i = 0; i < 4; i++) {   // N, E, S, W
    const Pt p = cardinal(i, v.up);
    taken.add({p.x - 16, p.y - 18, p.x + 16, p.y + 18});
  }
  for (int i = 0; i < s.n; i++) if (at[i].inside) taken.add({at[i].x - 12, at[i].y - 12, at[i].x + 12, at[i].y + 12});
  g_cur = !g_cur;
  g_count[g_cur] = 0;
  for (int i = 0; i < s.n; i++) if (at[i].inside) tag(s.planes[i], at[i], v.show, taken);
}

// A logical box as a button: amber when picked, night panel otherwise, its word in the middle.
void button(const ui::Box& b, const char* word, bool picked, const font::Font& f = radar::FONT24) {
  const int x = b.x * 3, y = b.y * 3, w = b.w * 3, h = b.h * 3;
  roundBox({x, y, x + w, y + h}, 14, picked ? HOME_C : PANEL);
  textAt(f, word, x + w / 2, y + h / 2, picked ? BG : CALL);
}
void menu(const View& v) {
  clear();
  button(DONE, "OK", true);
  textAt(radar::FONT18, "Which way is up", CX, 32 * 3, LETTER);
  static const char* const UP[] = {"N", "E", "S", "W"};
  for (int i = 0; i < 4; i++) button(upBox(i), UP[i], v.up == i);
  textAt(radar::FONT18, "Range in miles", CX, 66 * 3, LETTER);
  char buf[8];
  for (int i = 0; i < RANGES; i++) { snprintf(buf, sizeof buf, "%d", RANGE_MI[i]); button(rangeBox(i), buf, v.range == i); }
  textAt(radar::FONT18, "Show on planes", CX, 100 * 3, LETTER);
  static const char* const SHOW[SHOWS] = {"Flight", "Route", "Alt"};
  for (int i = 0; i < SHOWS; i++) button(showBox(i), SHOW[i], v.show[i], radar::FONT18);
  textAt(radar::FONT18, v.spot, CX, 138 * 3, RANGE_C);
}

}  // namespace

void draw(const View& v) {
  if (v.page == P_MENU) { menu(v); return; }
  scope(v.up, v.range);
  if (v.snap->state == air::S_LIVE) live(v);
  else { menuButton(); note(stateText(v.snap->state)); }
  home(v.homeDown);
}

const gfx::Sprite& icon() { return ICON; }

}  // namespace radar::paint
