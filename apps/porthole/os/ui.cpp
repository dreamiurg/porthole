#include "ui.h"
#include <stdio.h>
#include <string.h>

using namespace gfx;

namespace ui {
uint8_t inkOn(uint8_t fill) {
  uint32_t c = PALETTE_RGB[fill & 31];
  int lum = (((c >> 16) & 255) * 299 + ((c >> 8) & 255) * 587 + (c & 255) * 114) / 1000;
  return lum > 120 ? C_DKBROWN : C_WHITE;
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
  drawButton(b, in.down && in.hit(b.box.x, b.box.y, b.box.w, b.box.h));
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
void drawBack() { circle(80, 12, 11, C_DKBROWN); circle(80, 11, 10, C_ORANGE); blitTint(HOME_GLYPH, 76, 7, C_WHITE); }
bool back(const Input& in) { drawBack(); return in.tapInCircle(80, 12, 16); }

// ---- name keyboard: two pages of 13 letters, each a 4x4 grid of 24x22 keys 2 px apart (26 letters at that size do
// not fit the round glass at once). The last row is the page's last letter, the page switch, backspace and OK.
static const char* const PAGES[2] = {"ABCDEFGHIJKLM", "NOPQRSTUVWXYZ"};
static const char* const FLIP[2] = {"N-Z", "A-M"};
static const int KEY_W = 24, KEY_H = 22, GAP = 2, LEFT = 29, TOP = 42;   // rows 42..136 stay inside the chord
static Box key(int i) { return {LEFT + (i % 4) * (KEY_W + GAP), TOP + (i / 4) * (KEY_H + GAP), KEY_W, KEY_H}; }
enum { K_FLIP = 13, K_DEL, K_OK };
static bool tapped(const Input& in, const Box& b) { return in.tapIn(b.x, b.y, b.w, b.h); }

bool keyboard(const Input& in, char* buf, int& len, uint8_t& page) {
  page &= 1;
  for (int i = 0; i < 13; i++)
    if (tapped(in, key(i)) && len < NAME_LEN) { char c = PAGES[page][i]; buf[len] = len ? (char)(c + 32) : c; buf[++len] = 0; }
  if (tapped(in, key(K_FLIP))) page ^= 1;
  if (tapped(in, key(K_DEL)) && len > 0) buf[--len] = 0;
  return tapped(in, key(K_OK)) && len > 0;
}
static void drawKey(const Input& in, const Box& b, const char* label, uint8_t col) {
  int dy = in.down && in.hit(b.x, b.y, b.w, b.h) ? 1 : 0;
  roundRect(b.x, b.y + 1, b.w, b.h, C_DKBROWN);   // shadow
  roundRect(b.x, b.y + dy, b.w, b.h, C_DKBROWN);
  roundRect(b.x + 1, b.y + 1 + dy, b.w - 2, b.h - 2, dy ? (uint8_t)C_YELLOW : col);
  textCentered(b.x + b.w / 2, b.y + 7 + dy, label, C_DKBROWN);
}
void drawKeyboard(const Input& in, const char* buf, uint8_t page, const char* hint, uint32_t ms) {
  page &= 1;
  panel({42, 23, 74, 12}, C_WHITE, C_DKBROWN);
  bool cursor = (int)strlen(buf) < NAME_LEN && (ms / 400) % 2;   // no cursor once the name is full
  char shown[NAME_LEN + 2]; snprintf(shown, sizeof shown, "%s%s", buf, cursor ? "_" : "");
  if (buf[0]) textCentered(79, 25, shown, C_NAVY); else textCentered(79, 25, hint, C_DKGRAY);
  for (int i = 0; i < 13; i++) { char l[2] = {PAGES[page][i], 0}; drawKey(in, key(i), l, C_WHITE); }
  drawKey(in, key(K_FLIP), FLIP[page], C_SKY);
  drawKey(in, key(K_DEL), "<", C_LTGRAY);
  drawKey(in, key(K_OK), "OK", buf[0] ? (uint8_t)C_GREEN : (uint8_t)C_LTGRAY);
}

void FreshGate::filter(Input& in, uint32_t now) {
  if (!closed) return;
  if (in.pressed && now - shownMs >= FRESH_MS) { closed = false; return; }
  in.down = in.pressed = in.released = in.tap = in.longPress = false;   // a held finger stays invisible until lifted
}
}  // namespace ui
