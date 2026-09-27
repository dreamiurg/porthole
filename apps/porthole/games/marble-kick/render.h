// Marble Kick's look, drawn in code on the RGB565 surface: a walnut rim, a maple tray with turned grain, striped green
// felt with chalk lines, red lacquered pegs, brass goalposts and knob, a cream ball, wooden letter blocks. Its own flat
// colors (render.cpp), shared with no other game.
//
// Everything goes through one clipped span writer, so any page can repaint just a rectangle: the tray is computed
// from the distance to the center row by row (no cached image), and each thing is drawn whole but clipped. That is
// what lets the game redraw only the ball's old and new spots in each of the panel's two buffers (game.cpp, render()).
// Panel px, absolute (the tray's center is 240,240).
#pragma once
#include <stdint.h>
#include "gfx565.h"
#include "physics.h"

namespace marble::paint {
struct Box { int x0, y0, x1, y1; };   // x1, y1 exclusive; empty when x1 <= x0
constexpr Box FULL = {0, 0, gfx565::W, gfx565::H};
constexpr Box NONE = {0, 0, 0, 0};
inline bool empty(const Box& b) { return b.x1 <= b.x0 || b.y1 <= b.y0; }
inline bool same(const Box& a, const Box& b) { return a.x0 == b.x0 && a.y0 == b.y0 && a.x1 == b.x1 && a.y1 == b.y1; }
Box unite(const Box& a, const Box& b);
void clip(const Box& b);   // every draw below touches only this box

constexpr int SHADOW_DX = 5, SHADOW_DY = 6;   // where every shadow falls: light from the upper left
// The box a ball at (x, y) covers, shadow included.
inline Box ballBox(int x, int y) {
  return {x - BALL_R - 1, y - BALL_R - 1, x + BALL_R + SHADOW_DX + 2, y + BALL_R + SHADOW_DY + 2};
}

void tray(int goalHalf);                     // the whole tray, the goal's net cut into the rim above the middle
void shadow(int x, int y, int r, int goalHalf);   // the tray in shade under a round thing of radius r at (x, y)
void peg(int x, int y, int r);
void post(int x, int y);
void ball(int x, int y, int r);             // the cream ball with its dark pentagon (BALL_R on the tray)
void knob(int x, int y, bool pressed, float hold);   // the way home; hold 0..1 fills a ring around it (Play)
void coin(int x, int y, int number, int r);  // a level's number, carved in a brass coin
void playButton(int x, int y, bool pressed); // a red lacquer button with a cream arrow
void flag(int x, int y, int dir);            // a pennant on a goalpost, flying outward (dir -1 left, +1 right)
// The Calibrate page's dish: a small ball that rolls the way the game's will, settling in the middle (ready: the
// middle ring turns brass).
constexpr int DISH_R = 64, DISH_BALL_R = 12, DISH_TRAVEL = DISH_R - DISH_BALL_R - 4;
void dish(int x, int y, int ballDx, int ballDy, bool ready);
constexpr uint32_t CONFETTI_MS = 3000;
void confetti(uint32_t ms);                  // the Done page's celebration, ms since it began; nothing after CONFETTI_MS
// Wooden letter blocks, one glyph each ("0-9 G O A L !"), centered on x; each glyph's ink box goes to the UI audit.
void blocks(const char* s, int x, int y, int cell);
}  // namespace marble::paint
