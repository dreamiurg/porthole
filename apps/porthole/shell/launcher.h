// Launcher geometry for any number of games up to MAX_APPS (12), pure so host/test_shell.cpp can hold every layout to
// the round glass. Six games share a page: two rows of three picture tiles between the header (y 56) and the page
// arrows (from y 124), a short last row centered. Ten games are two pages. The pictures carry each game's
// identity; there is no room for a name under a tile, so pressing a tile shows its game's name over the header row.
#pragma once
#include "ui.h"

namespace launcher {
// Who is playing, 2 px below the home button's hit box (rows -4..27), then the brightness (sun) button: the one row with
// room for another target. The header holds the face and the widest 8-letter name (63 px, from x + 26); both outer
// corners are 2 px off the bezel.
constexpr ui::Box HEADER = {21, 30, 91, 24};
constexpr ui::Box LIGHT = {114, 30, 24, 24};
constexpr ui::Box CAPTION = {HEADER.x, HEADER.y, LIGHT.x + LIGHT.w - HEADER.x, HEADER.h};   // a pressed game's name
constexpr ui::Box LIGHT_AUTO = {50, 108, 60, 30};   // the light screen's Auto, under its row of three suns
constexpr int GRASS_Y = 123;
constexpr int PER_PAGE = 6, PER_ROW = 3;
constexpr int TILE_W = 40, TILE_H = 30, GAP_X = 4, GAP_Y = 2, TOP = 57;   // a picture is TILE_W - 2 by TILE_H - 2

inline int pages(int n) { return n > PER_PAGE ? (n + PER_PAGE - 1) / PER_PAGE : 1; }   // no games is one empty page
inline int count(int n, int page) { const int left = n - page * PER_PAGE; return left <= 0 || page >= pages(n) ? 0 : left < PER_PAGE ? left : PER_PAGE; }
inline int first(int, int page) { return page * PER_PAGE; }
inline int pageOf(int, int k) { return k / PER_PAGE; }

// Tile i of the m on a page: rows of three, the last row centered under the one above.
inline ui::Box tile(int i, int m) {
  const int row = i / PER_ROW, inRow = m - row * PER_ROW < PER_ROW ? m - row * PER_ROW : PER_ROW;
  const int width = inRow * TILE_W + (inRow - 1) * GAP_X;
  return {80 - width / 2 + (i % PER_ROW) * (TILE_W + GAP_X), TOP + row * (TILE_H + GAP_Y), TILE_W, TILE_H};
}
// The light screen's three suns, big to small: 44 px tiles 50 apart in the row under the header.
inline ui::Box sun(int i) { return {8 + i * 50, 57, 44, 44}; }
// Page arrows: a pair on the grass under the grid, 8 px apart; any lower and a corner leaves the glass.
inline ui::Box arrow(bool right) { return {right ? 84 : 52, 124, 24, 22}; }
// A name fits the caption with a 2 px margin each side (the sim asserts this for every game at boot).
inline bool nameFits(const char* name) { return gfx::textWidth(name) <= CAPTION.w - 4; }
}  // namespace launcher
