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
static const Sprite HOME = {9, 8, HOME_PX};
void drawBack() { circle(80, 12, 11, C_DKBROWN); circle(80, 11, 10, C_ORANGE); blitTint(HOME, 76, 7, C_WHITE); }
bool back(const Input& in) { drawBack(); return in.tapInCircle(80, 12, 16); }

// ---- name keyboard
static const char* const ROWS[4] = {"ABCDEF", "GHIJKLM", "NOPQRST", "UVWXYZ"};
// TODO: keys are 20 wide, under the 24x22 floor (the exception recorded in apps/porthole/CLAUDE.md constraint 1).
static const int KEY_W = 20, KEY_H = 24, TOP = 36, BOTTOM = TOP + 4 * KEY_H;
static const Box DEL = {57, BOTTOM, 22, KEY_H}, OK = {81, BOTTOM, 22, KEY_H};
static Box key(int r, int c) { return {80 - (int)strlen(ROWS[r]) * KEY_W / 2 + c * KEY_W, TOP + r * KEY_H, KEY_W, KEY_H}; }
static bool tapped(const Input& in, const Box& b) { return in.tapIn(b.x, b.y, b.w, b.h); }

bool keyboard(const Input& in, char* buf, int& len) {
  for (int r = 0; r < 4; r++)
    for (int c = 0; ROWS[r][c]; c++)
      if (tapped(in, key(r, c)) && len < NAME_LEN) { buf[len] = len ? (char)(ROWS[r][c] + 32) : ROWS[r][c]; buf[++len] = 0; }
  if (tapped(in, DEL) && len > 0) buf[--len] = 0;
  return tapped(in, OK) && len > 0;
}
static void drawKey(const Input& in, const Box& b, const char* label, uint8_t col) {
  int dy = in.down && in.hit(b.x, b.y, b.w, b.h) ? 1 : 0;
  roundRect(b.x + 1, b.y + 1 + dy, b.w - 2, b.h - 2, C_DKBROWN);
  roundRect(b.x + 2, b.y + 2 + dy, b.w - 4, b.h - 4, dy ? (uint8_t)C_YELLOW : col);
  textCentered(b.x + b.w / 2, b.y + 8 + dy, label, C_DKBROWN);
}
void drawKeyboard(const Input& in, const char* buf, const char* hint, uint32_t ms) {
  panel({42, 23, 74, 12}, C_WHITE, C_DKBROWN);
  char shown[NAME_LEN + 2]; snprintf(shown, sizeof shown, "%s%s", buf, (ms / 400) % 2 ? "_" : "");
  if (buf[0]) textCentered(79, 25, shown, C_NAVY); else textCentered(79, 25, hint, C_DKGRAY);
  for (int r = 0; r < 4; r++)
    for (int c = 0; ROWS[r][c]; c++) { char l[2] = {ROWS[r][c], 0}; drawKey(in, key(r, c), l, C_WHITE); }
  drawKey(in, DEL, "<", C_LTGRAY);
  drawKey(in, OK, "OK", buf[0] ? (uint8_t)C_GREEN : (uint8_t)C_LTGRAY);
}

void TapGuard::filter(Input& in, uint32_t now, int cur) {
  if (!in.tap) return;
  int dx = in.x - x, dy = in.y - y;
  if (dx > -12 && dx < 12 && dy > -12 && dy < 12 && now - ms < 300 && screen != cur) { in.tap = false; return; }
  ms = now; x = in.x; y = in.y; screen = cur;
}
}  // namespace ui
