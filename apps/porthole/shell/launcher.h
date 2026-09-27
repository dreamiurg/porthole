// Launcher geometry for any number of games up to MAX_APPS (8), pure so host/test_shell.cpp can hold every layout to
// the round glass. Up to three games share one row. More split into balanced pages of up to three (4: 2+2, 5: 3+2,
// 6: 3+3, 7: 3+2+2, 8: 3+3+2) behind a pair of arrows beside the mute button: two rows do not fit, since between the
// header (y 56) and the mute button there is room for one row of 44 px tiles and their names, and a second row's names
// would fall where the glass is narrower than "Pets Club".
#pragma once
#include "ui.h"

namespace launcher {
constexpr ui::Box HEADER = {30, 30, 100, 24};   // who is playing: 2 px below the home button's hit box (rows -4..27)
constexpr int MUTE_X = 80, MUTE_HIT = 15;       // ui::iconButton: radius 11, hit circle 4 px wider
constexpr int NAME_Y = 105, LINE_H = 9;         // a name starts 4 px under its tile; a line is 8 px and its shadow

// Two games keep the launcher's first look: their names leave a gap over the mute button. Any other count has a name
// right above it, so names may take a second line and the grass and the mute button sit lower, the mute's hit circle
// under a two-line name.
inline bool twoLines(int n) { return n != 2; }
inline int muteY(int n) { return twoLines(n) ? 138 : 128; }
inline int grassY(int n) { return twoLines(n) ? 123 : 116; }

inline int pages(int n) { return n > 3 ? (n + 2) / 3 : 1; }   // no games is one empty page
inline int count(int n, int page) {                              // the first pages get the odd one
  return page < pages(n) ? n / pages(n) + (page < n % pages(n) ? 1 : 0) : 0;
}
inline int first(int n, int page) { const int r = n % pages(n); return page * (n / pages(n)) + (page < r ? page : r); }
inline int pageOf(int n, int k) { int p = 0; while (p + 1 < pages(n) && k >= first(n, p + 1)) p++; return p; }

// Tile i of m on a page: two are 48 wide and 64 apart; three, 44 wide and 50 apart, the only row of three that fits.
inline int gap(int m) { return m <= 2 ? 64 : 50; }
inline ui::Box tile(int i, int m) { const int w = m <= 2 ? 48 : 44; return {80 - w / 2 + (2 * i - (m - 1)) * gap(m) / 2, 57, w, 44}; }
// Text w px wide centered on cx, an odd pixel toward the middle of the glass: a name on the right gets the same room
// as its mirror on the left.
inline int centerX(int cx, int w) { return cx - (w + (cx > gfx::CX ? 1 : 0)) / 2; }
// Line `line` of tile i's name: as wide as the gap to the next name allows, narrowed until its lower corners stay a
// pixel off the bezel (the UI audit's rule). Under a side tile of three that is 43 px, then 33 on the second line.
inline ui::Box nameLine(int i, int m, int line) {
  const ui::Box t = tile(i, m);
  const int cx = t.x + t.w / 2, y = NAME_Y + line * LINE_H;
  int w = gap(m) - 4;
  while (!gfx::inCircle(centerX(cx, w), y + 8, 1) || !gfx::inCircle(centerX(cx, w) + w, y + 8, 1)) w--;
  return {centerX(cx, w), y, w, 8};
}
// A game's name under tile i of m, for n games: wrapped to the narrower of its lines (a side name's second), each line
// where it is drawn. `fits`: every letter kept (wrap drops what does not fit its lines) and no line wider than its room.
struct Name { int lines; char text[2][40]; ui::Box at[2]; bool fits; };
inline int letters(const char* s) { int k = 0; for (; *s; s++) k += *s != ' '; return k; }
inline Name name(const char* s, int i, int m, int n) {
  const ui::Box room[2] = {nameLine(i, m, 0), nameLine(i, m, 1)};
  const int cx = tile(i, m).x + tile(i, m).w / 2, maxLines = twoLines(n) ? 2 : 1;
  Name r = {};
  r.lines = gfx::wrap(s, maxLines == 2 && room[1].w < room[0].w ? room[1].w : room[0].w, r.text, maxLines);
  int kept = 0; r.fits = true;
  for (int l = 0; l < r.lines; l++) {
    const int w = gfx::textWidth(r.text[l]);
    r.at[l] = {centerX(cx, w), room[l].y, w, 8};
    r.fits = r.fits && w <= room[l].w;
    kept += letters(r.text[l]);
  }
  r.fits = r.fits && kept == letters(s);
  return r;
}
// Page arrows: on the grass beside the mute button, 2 px clear of its hit circle; any lower and a corner leaves the glass.
inline ui::Box arrow(bool right) { return {right ? 97 : 39, 124, 24, 22}; }
}  // namespace launcher
