#include "ui.h"
#include "gfx565.h"
#include "utf8.h"
#include <stdio.h>
#include <string.h>

using namespace gfx;

namespace ui {
uint8_t inkOn(uint8_t fill) {
  uint32_t c = PALETTE_RGB[fill & 31];
  int lum = (((c >> 16) & 255) * 299 + ((c >> 8) & 255) * 587 + (c & 255) * 114) / 1000;
  return lum > 120 ? C_DKBROWN : C_WHITE;
}
void newDot(const Box& b) { circle(b.x + b.w - 4, b.y + 4, 5, C_WHITE); circle(b.x + b.w - 4, b.y + 4, 4, C_RED); }
void newDot565(const Box& b) {
  const int cx = (b.x + b.w - 4) * 3 + 1, cy = (b.y + 4) * 3 + 1;
  gfx565::circle(cx, cy, 15, rgb888_to_565(PALETTE_RGB[C_WHITE])); gfx565::circle(cx, cy, 12, rgb888_to_565(PALETTE_RGB[C_RED]));
}
void panel(const Box& b, uint8_t fill, uint8_t border) {
  roundRect(b.x, b.y, b.w, b.h, border); roundRect(b.x + 1, b.y + 1, b.w - 2, b.h - 2, fill);
}
void drawButton(const Button& bt, bool pressed) {
  const Box& b = bt.box; int dy = pressed ? 1 : 0;
  if (!pressed) roundRect(b.x, b.y + 2, b.w, b.h, C_DKBROWN);   // shadow
  roundRect(b.x, b.y + dy, b.w, b.h, C_DKBROWN);
  roundRect(b.x + 1, b.y + 1 + dy, b.w - 2, b.h - 2, bt.col);
  hline(b.x + 2, b.y + 1 + dy, b.w - 4, C_WHITE);
  if (bt.icon) blit(*bt.icon, b.x + (b.w - bt.icon->w) / 2, b.y + dy + 2);
  if (bt.label) textCentered(b.x + b.w / 2, bt.icon ? b.y + dy + b.h - 9 : b.y + dy + (b.h - 7) / 2, bt.label, inkOn(bt.col));
}
bool button(const Input& in, const Button& b) {
  drawButton(b, pressing(in, b.box));
  return in.tapIn(b.box.x, b.box.y, b.box.w, b.box.h);
}
bool iconButton(const Input& in, int cx, int cy, const Sprite& icon, uint8_t col) {
  const int r = 11;
  circle(cx, cy + 1, r, C_DKBROWN); circle(cx, cy, r, C_DKBROWN); circle(cx, cy, r - 1, col);
  blitTint(icon, cx - icon.w / 2, cy - icon.h / 2, C_WHITE);
  return in.tapInCircle(cx, cy, r + 4);
}

void toast(const char* text) {
  int w = textWidth(text) + 10;
  panel({80 - w / 2, 62, w, 14}, C_WHITE, C_DKBROWN);
  textCentered(80, 65, text, C_DKBROWN);
}

// The home glyph: the back button belongs to os/, not to one game.
static const uint8_t HOME_PX[] = {
  255, 255, 255, 255, 0, 255, 255, 255, 255,  255, 255, 255, 0, 0, 0, 255, 255, 255,
  255, 255, 0, 0, 0, 0, 0, 255, 255,          255, 0, 0, 0, 0, 0, 0, 0, 255,
  0, 0, 0, 0, 0, 0, 0, 0, 0,                  255, 0, 0, 0, 0, 0, 0, 0, 255,
  255, 0, 0, 0, 255, 0, 0, 0, 255,            255, 0, 0, 0, 255, 0, 0, 0, 255};
const Sprite HOME_GLYPH = {9, 8, HOME_PX};
static const uint8_t BACK_PX[] = {
  255, 255, 255, 0, 255, 255, 255, 255, 255,  255, 255, 0, 0, 255, 255, 255, 255, 255,
  255, 0, 0, 0, 255, 255, 255, 255, 255,      0, 0, 0, 0, 0, 0, 0, 0, 0,
  0, 0, 0, 0, 0, 0, 0, 0, 0,                  255, 0, 0, 0, 255, 255, 255, 255, 255,
  255, 255, 0, 0, 255, 255, 255, 255, 255,    255, 255, 255, 0, 255, 255, 255, 255, 255};
const Sprite BACK_GLYPH = {9, 8, BACK_PX};
static void topButton(const Sprite& glyph) { circle(80, 12, 11, C_DKBROWN); circle(80, 11, 10, C_ORANGE); blitTint(glyph, 76, 7, C_WHITE); }
void drawBack() { topButton(BACK_GLYPH); }
bool back(const Input& in) { drawBack(); return in.tapInCircle(80, 12, 16); }
void drawHome() { topButton(HOME_GLYPH); }
bool home(const Input& in) { drawHome(); return in.tapInCircle(80, 12, 16); }

// ---- name keyboard: pages of 13 letters, each a 4x4 grid of 24x22 keys 2 px apart (26 letters at that size do
// not fit the round glass at once). The last row is the page's last letter, the page switch, backspace and OK. Pages 0-1
// are English (A-M, N-Z), 2-4 Ukrainian (33 letters and the apostrophe); the key under the grid switches language.
static const uint16_t PAGES[5][13] = {
  {'A', 'B', 'C', 'D', 'E', 'F', 'G', 'H', 'I', 'J', 'K', 'L', 'M'},
  {'N', 'O', 'P', 'Q', 'R', 'S', 'T', 'U', 'V', 'W', 'X', 'Y', 'Z'},
  {0x410, 0x411, 0x412, 0x413, 0x490, 0x414, 0x415, 0x404, 0x416, 0x417, 0x418, 0x406, 0x407},   // А Б В Г Ґ Д Е Є Ж З И І Ї
  {0x419, 0x41A, 0x41B, 0x41C, 0x41D, 0x41E, 0x41F, 0x420, 0x421, 0x422, 0x423, 0x424, 0x425},   // Й К Л М Н О П Р С Т У Ф Х
  {0x426, 0x427, 0x428, 0x429, 0x42C, 0x42E, 0x42F, '\''},                                       // Ц Ч Ш Щ Ь Ю Я '
};
static const char* const FLIP[5] = {"N-Z", "A-M", "Й-Х", "Ц-Я", "А-Ї"};
static int nextPage(int p) { return p == 1 ? 0 : p == 4 ? 2 : p + 1; }
static uint16_t lower(uint16_t c) {   // the small letter of a capital on the keys
  if (c < 0x80) return c >= 'A' && c <= 'Z' ? (uint16_t)(c + 32) : c;
  if (c >= 0x410 && c <= 0x42F) return (uint16_t)(c + 0x20);
  return c == 0x490 ? 0x491 : (uint16_t)(c + 0x50);   // Ґ, and Є І Ї (U+0404..0407 -> U+0454..0457)
}
static const int KEY_W = 24, KEY_H = 22, GAP = 2, LEFT = 29, TOP = 42;   // rows 42..136 stay inside the chord
static Box key(int i) { return {LEFT + (i % 4) * (KEY_W + GAP), TOP + (i / 4) * (KEY_H + GAP), KEY_W, KEY_H}; }
static const Box LANG = {68, 138, 24, 22};   // a key under the grid, 2 px below it like the keys
enum { K_FLIP = 13, K_DEL, K_OK };
static bool tapped(const Input& in, const Box& b) { return in.tapIn(b.x, b.y, b.w, b.h); }

bool keyboard(const Input& in, char* buf, int& len, uint8_t& page, size_t cap) {
  if (page > 4) page = 0;
  for (int i = 0; i < 13; i++) {
    const uint16_t c = PAGES[page][i];
    // an apostrophe only joins two letters: never first, never twice ("Мар'яна")
    if (!c || !tapped(in, key(i)) || utf8::count(buf) >= NAME_LEN || (c == '\'' && (!len || buf[len - 1] == '\''))) continue;
    utf8::append(buf, len, cap, len ? lower(c) : c);   // a letter that would not fit the save is not typed
  }
  if (tapped(in, key(K_FLIP))) page = (uint8_t)nextPage(page);
  if (tapped(in, LANG)) page = page < 2 ? 2 : 0;
  if (tapped(in, key(K_DEL))) utf8::popLetter(buf, len);
  return tapped(in, key(K_OK)) && len > 0 && buf[len - 1] != '\'';   // nor last
}
static void drawKey(const Input& in, const Box& b, const char* label, uint8_t col) {
  int dy = pressing(in, b) ? 1 : 0;
  roundRect(b.x, b.y + 1, b.w, b.h, C_DKBROWN);   // shadow
  roundRect(b.x, b.y + dy, b.w, b.h, C_DKBROWN);
  roundRect(b.x + 1, b.y + 1 + dy, b.w - 2, b.h - 2, dy ? (uint8_t)C_YELLOW : col);
  textCentered(b.x + b.w / 2, b.y + (b.h - 8) / 2 + dy, label, C_DKBROWN);
}
void drawKeyboard(const Input& in, const char* buf, uint8_t page, const char* hint, size_t cap) {
  if (page > 4) page = 0;
  panel({42, 23, 74, 12}, C_WHITE, C_DKBROWN);
  // Terminal style: the name left-aligned and a solid block after it, so nothing moves while it waits for a letter.
  // The hint fills the field alone (70 px of 72), so it shows centered, with no block, until the first letter. No block
  // once another letter of this page would not fit: 8 letters, or the save's bytes (a Ukrainian letter takes 2).
  const size_t room = cap - 1 - strlen(buf);
  const bool full = utf8::count(buf) >= NAME_LEN || room < (page < 2 ? 1u : 2u);
  if (!buf[0]) textCentered(79, 25, hint, C_DKGRAY);
  else if (const int x = text(45, 25, buf, C_NAVY); !full) rect(x + 1, 25, 5, 8, C_NAVY);
  for (int i = 0; i < 13; i++) {
    char l[3] = {0}; int n = 0;
    if (!PAGES[page][i]) continue;
    utf8::append(l, n, sizeof l, PAGES[page][i]);
    drawKey(in, key(i), l, C_WHITE);
  }
  drawKey(in, key(K_FLIP), FLIP[page], C_SKY);
  drawKey(in, key(K_DEL), "<", C_LTGRAY);
  const size_t n = strlen(buf);
  drawKey(in, key(K_OK), "OK", n && buf[n - 1] != '\'' ? (uint8_t)C_GREEN : (uint8_t)C_LTGRAY);
  drawKey(in, LANG, page < 2 ? "UA" : "EN", C_PEACH);
}

bool clicked(const Input& in, const Box& b) {
  UiAudit::add(b.x, b.y, b.w, b.h);
  auto on = [&](int x, int y) { return x >= b.x && y >= b.y && x < b.x + b.w && y < b.y + b.h; };
  return in.released && on(in.downX, in.downY) && on(in.x, in.y);
}
bool clicked(const Input& in, int cx, int cy, int r) {
  UiAudit::add(cx - r, cy - r, 2 * r, 2 * r);
  auto on = [&](int x, int y) { return (x - cx) * (x - cx) + (y - cy) * (y - cy) <= r * r; };
  return in.released && on(in.downX, in.downY) && on(in.x, in.y);
}
bool pressing(const Input& in, int cx, int cy, int r) {
  const int dx = in.downX - cx, dy = in.downY - cy, ex = in.x - cx, ey = in.y - cy;
  return in.down && dx * dx + dy * dy <= r * r && ex * ex + ey * ey <= r * r;
}
bool pressing(const Input& in, const Box& b) {
  return in.hit(b.x, b.y, b.w, b.h) && in.down && in.downX >= b.x && in.downX < b.x + b.w && in.downY >= b.y && in.downY < b.y + b.h;
}
void FreshGate::filter(Input& in, uint32_t now) {
  if (!closed) return;
  const int dx = in.x - spotX, dy = in.y - spotY;
  const bool away = spot && dx * dx + dy * dy > SPOT_R * SPOT_R;
  if (in.pressed && (away || now - shownMs >= FRESH_MS)) { closed = false; return; }
  in.down = in.pressed = in.released = in.tap = in.longPress = false;   // a held finger stays invisible until lifted
}
}  // namespace ui
