// Shared widgets: the look every Porthole screen shares (shell and games). Stateless: each call draws and/or
// hit-tests against the Input it is given, so the UI audit sees every region through Input::hit/tapIn.
#pragma once
#include <stdint.h>
#include "gfx.h"
#include "input.h"

namespace ui {
struct Box { int x, y, w, h; };
struct Button { Box box; const char* label; const gfx::Sprite* icon; uint8_t col; };

uint8_t inkOn(uint8_t fill);   // readable text color for a fill: dark on bright, white on dark
void panel(const Box& b, uint8_t fill, uint8_t border);
void drawButton(const Button& b, bool pressed);
bool button(const Input& in, const Button& b);   // draws it (sunk while the finger is on it); true on a tap
// Round button of radius 11 with a one-color icon mask drawn in white; the hit circle is 4 px wider.
bool iconButton(const Input& in, int cx, int cy, const gfx::Sprite& icon, uint8_t col);
void toast(const char* text);   // a one-line note in a white panel across the middle of the glass
// The orange home button at the top of the glass: every screen's way back.
void drawBack();
bool back(const Input& in);

// Name keyboard: a field on top, then two pages of letters (A-M, N-Z) as a 4x4 grid of 24x22 keys with the page
// switch, backspace and OK on the last row. `page` is the caller's (0 or 1; reset it with the name). Names are up to
// 8 letters, first one capital.
constexpr int NAME_LEN = 8;
bool keyboard(const Input& in, char* buf, int& len, uint8_t& page);   // true when OK is tapped with at least one letter
void drawKeyboard(const Input& in, const char* buf, uint8_t page, const char* hint, uint32_t ms);   // hint shows while empty

// A fresh screen ignores every touch until a press begins FRESH_MS after it appeared: the finger that changed it
// (still down after a long press) and the second tap of a kid's slow double tap (300-400 ms apart) would otherwise
// act on whatever now sits under them. Call shown() on every screen change and filter() on every frame's input.
struct FreshGate {
  static constexpr uint32_t FRESH_MS = 450;
  uint32_t shownMs = 0; bool closed = false;
  void shown(uint32_t now) { shownMs = now; closed = true; }
  void filter(Input& in, uint32_t now);
};
}  // namespace ui
