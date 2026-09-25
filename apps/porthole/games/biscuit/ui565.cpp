#include "ui565.h"
#include <stdio.h>
#include "generated/discovery_art.h"
#include "gfx565.h"

namespace biscuit::ui565 {
namespace {
const char* const ICONS[7][12] = {
  {"...##.......", "...##..##...", "..####.##...", "............", "############", ".##########.", "..########..", "...######...",
   "...######...", "............", "............", "............"},
  {"............", "..###..###..", ".##########.", "############", "############", ".##########.", "..########..", "...######...",
   "....####....", ".....##.....", "............", "............"},
  {"......####..", "....######..", "...#####....", "..####......", "..###.......", "..###.......", "..####......", "...#####....",
   "....######..", "......####..", "............", "............"},
  {"............", ".####..####.", ".####..####.", ".#..#..#..#.", ".#..#..#..#.", ".#..#..#..#.", ".#..#..#..#.", ".####..####.",
   "..###..###..", "...##..##...", "............", "............"},
  {"....####....", "..##....##..", ".##......##.", "##..##....##", "##...##...##", "##....##..##", ".##....##.#.", "..##....##..",
   "....####....", "............", "............", "............"},
  {"..##....##..", ".####..####.", ".####..####.", "..##....##..", "............", "##..####..##", "###########.", ".##########.",
   "..########..", "...######...", "............", "............"},
  {".....##.....", ".....##.....", "..########..", "...######...", "....####....", "..########..", ".###.##.###.", "##...##...##",
   ".....##.....", ".....##.....", "............", "............"},
};
constexpr int RADIUS = 7;
uint16_t pressed(uint16_t c) {   // 7/8 of each channel: the finger is on it
  unsigned r = c >> 11, g = (c >> 5) & 63, b = c & 31;
  return (uint16_t)((r * 7 / 8) << 11 | (g * 7 / 8) << 5 | b * 7 / 8);
}
void box(const Box& b, uint16_t fill, uint16_t border) {
  gfx565::roundRect({b.x * 3, b.y * 3, b.w * 3, b.h * 3}, RADIUS, border);
  gfx565::roundRect({b.x * 3 + 1, b.y * 3 + 1, b.w * 3 - 2, b.h * 3 - 2}, RADIUS - 1, fill);
}
void frame(const Input& in, const Button& b) {
  const bool down = b.enabled && in.down && in.hit(b.box.x, b.box.y, b.box.w, b.box.h);
  box(b.box, !b.enabled ? DISABLED : down ? pressed(b.fill) : b.fill, LINE);
}
}  // namespace

void icon(Icon which, int x, int y, int scale, uint16_t color) {
  const char* const* rows = ICONS[(int)which];
  for (int r = 0; r < 12; r++)
    for (int c = 0; c < 12; c++) if (rows[r][c] == '#') gfx565::rect(x + c * scale, y + r * scale, scale, scale, color);
}
void tennisBall(int x, int y) {
  static const char* const BALL[10] = {"...oooo...", "..ollllo..", ".olhhhllo.", "olhhhlmmlo", "olhhlmmlmo",
                                       "olhlmmlmmo", "olmmlllmo.", ".omllllmo.", "..ommmmo..", "...oooo..."};
  for (int r = 0; r < 10; r++) for (int c = 0; c < 10; c++) {
    char k = BALL[r][c];
    if (k == '.') continue;
    uint16_t col = k == 'o' ? rgb(0x73824a) : k == 'l' ? rgb(0xdeeb84) : k == 'm' ? rgb(0xcedb6b) : rgb(0xffffce);
    gfx565::rect(x + c * 3, y + r * 3, 3, 3, col);
  }
}
void text(const Label& l, const char* s, uint16_t color) {
  const font::Box& b = l.box;
  font::textBox(b, s, l.x, l.middle ? l.y + (b.h - font::textHeight(*b.font, s, b.w, b.spacing)) / 2 : l.y, color);
}

bool tapped(const Input& in, const Button& b) { return b.enabled && in.tapIn(b.box.x, b.box.y, b.box.w, b.box.h); }
void button(const Input& in, const Button& b) {
  frame(in, b);
  text(buttonLabel(b.box, b.font), b.label, INK);
}
void actionButton(const Input& in, const Button& b, Icon glyph) {
  frame(in, b);
  const int x = b.box.x * 3, y = b.box.y * 3, w = b.box.w * 3;
  icon(glyph, x + (w - 24) / 2, y + 7, 2, INK);
  font::textBox({b.font, (int16_t)w, 0, 0, Align::CENTER}, b.label, x, y + b.box.h * 3 - 5 - b.font->lineHeight, INK);
}
void tile(const Input& in, const Tile& t) {
  frame(in, {t.box, "", &FONT20, t.fill, true});
  const int x = t.box.x * 3, y = t.box.y * 3;
  icon(t.glyph, x + 10, y + 13, 2, INK);
  font::textBox(TILE_TITLE.box, t.title, x + TILE_TITLE.x, y + TILE_TITLE.y, INK);
  font::textBox(TILE_DETAIL.box, t.detail, x + TILE_DETAIL.x, y + TILE_DETAIL.y, DETAIL);
}
void hotspot(const Input& in, const Box& b) {
  if (in.down && in.hit(b.x, b.y, b.w, b.h)) {   // LVGL's pressed hotspot: warm white at 48/255 over the room
    for (int y = b.y * 3; y < (b.y + b.h) * 3; y++) for (int x = b.x * 3; x < (b.x + b.w) * 3; x++) {
      uint16_t& p = gfx565::fb[y * gfx565::W + x];
      unsigned r = p >> 11, g = (p >> 5) & 63, bl = p & 31;
      p = (uint16_t)((r * 207 + 31 * 48) / 255 << 11 | (g * 207 + 60 * 48) / 255 << 5 | (bl * 207 + 26 * 48) / 255);
    }
  }
}

void roundButton(const Input& in, const Box& b, const char* label) {
  const bool down = in.down && in.hit(b.x, b.y, b.w, b.h);
  const int cx = b.x * 3 + b.w * 3 / 2, cy = b.y * 3 + b.h * 3 / 2, r = (b.h < b.w ? b.h : b.w) * 3 / 2;
  gfx565::circle(cx, cy, r, LINE);
  gfx565::circle(cx, cy, r - 1, down ? pressed(PURPLE) : PURPLE);
  font::text(FONT28, label, cx - font::textWidth(FONT28, label) / 2, cy - FONT28.lineHeight / 2, INK);
}

bool homeTapped(const Input& in) { return in.tapInCircle(HOME_CX, HOME_CY, HOME_HIT_R); }
void home(const Input& in) {   // ui::drawBack at 3x: a dark rim, the orange disc (sunk while pressed), the white house
  const int dx = in.x - HOME_CX, dh = in.y - HOME_CY;   // pressed exactly where homeTapped() hits
  const int dy = in.down && dx * dx + dh * dh <= HOME_HIT_R * HOME_HIT_R ? 3 : 0;
  const uint16_t orange = rgb(PALETTE_RGB[C_ORANGE]);
  gfx565::circle(HOME_CX * 3 + 1, HOME_CY * 3 + 1, 34, rgb(PALETTE_RGB[C_DKBROWN]));
  gfx565::circle(HOME_CX * 3 + 1, HOME_CY * 3 - 2 + dy, 31, dy ? pressed(orange) : orange);
  const gfx::Sprite& g = ui::HOME_GLYPH;
  for (int y = 0; y < g.h; y++) for (int x = 0; x < g.w; x++)
    if (g.px[y * g.w + x] != C_T) gfx565::rect((HOME_CX - 4 + x) * 3, (HOME_CY - 5 + y) * 3 + dy, 3, 3, rgb(PALETTE_RGB[C_WHITE]));
}

void top(const Input& in, const char* title, int stars) {
  button(in, BACK_BUTTON);
  char s[24]; snprintf(s, sizeof s, "%d stars", stars);
  text(STARS, s, INK);
  text(HEADER, title, INK);
}
static Button prev(const Nav& n) { return {PREV, n.prev, &FONT20, PURPLE, n.prevOn}; }
static Button next(const Nav& n) { return {NEXT, n.next, &FONT20, PURPLE, n.nextOn}; }
static Nav list(int page, int count) { return {"Previous", "Next", page > 0, page + 1 < count}; }
int navTapped(const Input& in, const Nav& n) {
  const bool back = tapped(in, prev(n)), on = tapped(in, next(n));   // both asked: the audit sees both
  return back ? -1 : on ? 1 : 0;
}
void nav(const Input& in, const Nav& n, int page, int count) {
  char s[16]; pageNumber(s, page, count);
  text(PAGE_NUMBER, s, INK);
  button(in, prev(n));
  button(in, next(n));
}
int navTapped(const Input& in, int page, int count) { return navTapped(in, list(page, count)); }
void nav(const Input& in, int page, int count) { nav(in, list(page, count), page, count); }
void picture(int id, int x, int y, int scale) { gfx565::blit(DISCOVERY_ART[id], x, y, scale); }
void pictureRow(const Input& in, const Box& b, int id, const char* label) {
  frame(in, {b, label, &FONT20, PURPLE, true});
  picture(id, b.x * 3 + ROW_PICTURE_X, b.y * 3 + ROW_PICTURE_Y, 1);
  Label l = FACT_BUTTON;   // placed like the lowest row's: moved to this one
  l.x = (int16_t)(l.x - ROW[ROWS - 1].x * 3 + b.x * 3); l.y = (int16_t)(l.y - ROW[ROWS - 1].y * 3 + b.y * 3);
  if (font::textHeight(FONT20, label, l.box.w, 0) <= 2 * FONT20.lineHeight) l.box.font = &FONT20;
  text(l, label, INK);
}
void need(int slot, Icon glyph, int value, uint16_t color) {
  const int x = NEED_X[slot], y = NEED_Y;
  gfx565::roundRect({x, y, NEED_W, NEED_H}, 5, LINE);
  gfx565::roundRect({x + 1, y + 1, NEED_W - 2, NEED_H - 2}, 4, PAPER);
  icon(glyph, x + 6, y + 10, 1, color);
  gfx565::rect(x + 23, y + 14, 28, 7, PEACH);
  gfx565::rect(x + 23, y + 14, value * 28 / 100 > 1 ? value * 28 / 100 : 1, 7, color);
  char s[12]; snprintf(s, sizeof s, "%d", value);
  font::textBox(NEED_VALUE.box, s, x + NEED_VALUE.x, y + NEED_VALUE.y, INK);
}
void bubble(const char* s) {
  gfx565::roundRect({BUBBLE_X, BUBBLE_Y, BUBBLE_W, BUBBLE_H}, RADIUS, BUBBLE_EDGE);
  gfx565::roundRect({BUBBLE_X + 1, BUBBLE_Y + 1, BUBBLE_W - 2, BUBBLE_H - 2}, RADIUS - 1, PAPER);
  text(BUBBLE, s, INK);
}
}  // namespace biscuit::ui565
