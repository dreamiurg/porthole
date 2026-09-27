// Launcher geometry for any number of games, pure so host/test_shell.cpp can hold every layout to the round glass.
// Up to three games share one row. More split into balanced pages of up to three (4: 2+2, 5: 3+2, 6: 3+3) behind a
// pair of arrows beside the mute button: two rows do not fit, since between the header (y 56) and the mute button
// there is room for one row of 44 px tiles and their names, and a second row's names would fall where the glass is
// narrower than "Pets Club".
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

inline int pages(int n) { return (n + 2) / 3; }
inline int count(int n, int page) { return n / pages(n) + (page < n % pages(n) ? 1 : 0); }   // the first pages get the odd one
inline int first(int n, int page) { const int r = n % pages(n); return page * (n / pages(n)) + (page < r ? page : r); }
inline int pageOf(int n, int k) { int p = 0; while (k >= first(n, p) + count(n, p)) p++; return p; }

// Tile i of m on a page: two are 48 wide and 64 apart; three, 44 wide and 50 apart, the only row of three that fits.
inline int gap(int m) { return m <= 2 ? 64 : 50; }
inline ui::Box tile(int i, int m) { const int w = m <= 2 ? 48 : 44; return {80 - w / 2 + (2 * i - (m - 1)) * gap(m) / 2, 57, w, 44}; }
// Line `line` of tile i's name: as wide as the gap to the next name allows, narrowed until its lower corners stay a
// pixel off the bezel (the UI audit's rule). Under a side tile of three that is 43 px, then 33 on the second line.
inline ui::Box nameLine(int i, int m, int line) {
  const ui::Box t = tile(i, m);
  const int cx = t.x + t.w / 2, y = NAME_Y + line * LINE_H;
  int w = gap(m) - 4;
  while (!gfx::inCircle(cx - w / 2, y + 8, 1) || !gfx::inCircle(cx - w / 2 + w, y + 8, 1)) w--;
  return {cx - w / 2, y, w, 8};
}
// Page arrows: on the grass beside the mute button, 2 px clear of its hit circle; any lower and a corner leaves the glass.
inline ui::Box arrow(bool right) { return {right ? 97 : 39, 124, 24, 22}; }
}  // namespace launcher
