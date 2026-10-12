// Marble Kick's look, drawn in code on the RGB565 surface: a walnut rim, a maple tray with turned grain, striped green
// felt with chalk lines, red lacquered pegs, brass goalposts and knob, a cream ball, wooden letter blocks. Its own flat
// colors (render.cpp), shared with no other game.
//
// Everything goes through the runtime's clipped span writer (os/canvas.h), so any page can repaint just a rectangle: the tray is computed
// from the distance to the center row by row (no cached image), and each thing is drawn whole but clipped. That is
// what lets the game redraw only the ball's old and new spots in each of the panel's two buffers (game.cpp, render()).
// Panel px, absolute (the tray's center is 240,240).
#pragma once
#include <math.h>
#include <stdint.h>
#include "canvas.h"
#include "gfx565.h"
#include "physics.h"

namespace marble::paint {
// The clipped painter is the runtime's (os/canvas.h): every draw below touches only canvas::clip's box.
using canvas::Box; using canvas::FULL; using canvas::NONE; using canvas::empty; using canvas::same; using canvas::unite;
using canvas::clip;

constexpr int SHADOW_DX = 5, SHADOW_DY = 6;   // where every shadow falls: light from the upper left
// The box a ball at (x, y) covers, shadow included.
inline Box ballBox(int x, int y) {
  return {x - BALL_R - 1, y - BALL_R - 1, x + BALL_R + SHADOW_DX + 2, y + BALL_R + SHADOW_DY + 2};
}

// Where the goal is, for the tray's net: drawn at the nearest quarter degree (`key`, which names what the pixels show),
// and the screen box its net and posts can reach.
struct Mouth { int half; float c, s; int key; Box box; };
inline int angleKey(float radians) { return (int)lroundf(radians * 720 / 3.14159265f); }
Mouth mouth(int goalHalf, float angle);
void tray(const Mouth& m);                   // the whole tray, the goal's net cut into the rim where the goal is
void shadow(int x, int y, int r, const Mouth& m);   // the tray in shade under a round thing of radius r at (x, y)
void railShadow(const Rail& r, const Mouth& m);
void rail(const Rail& r);                    // a wooden bar (panel px)
void groove(const Rail& line, const Mouth& m);   // a defender's track in the felt
void hole(int x, int y, int r);
void star(int x, int y, int r, bool lit);
void peg(int x, int y, int r);
void keeper(int x, int y);
void post(int x, int y);
void ball(int x, int y, int r, int dark);    // the cream ball with its dark pentagon; dark 1-2: sinking into a hole
void knob(int x, int y, bool pressed, float hold);   // the way home; hold 0..1 fills a ring around it (Play)
void coin(int x, int y, int number, int r);  // a level's number, carved in a brass coin
void playButton(int x, int y, bool pressed); // a red lacquer button with a cream arrow
void flag(int x, int y, float angle, int dir);   // a pennant on a goalpost, flying outward (dir -1 left, +1 right)
// The Calibrate page's dish: a small ball that rolls the way the game's will, settling in the middle (ready: the
// middle ring turns brass).
constexpr int DISH_R = 64, DISH_BALL_R = 12, DISH_TRAVEL = DISH_R - DISH_BALL_R - 4;
void dish(int x, int y, int ballDx, int ballDy, bool ready);
constexpr uint32_t CONFETTI_MS = 3000;
void confetti(uint32_t ms);                  // the Done page's celebration, ms since it began; nothing after CONFETTI_MS
// Wooden letter blocks, one glyph each ("0-9 G O A L !"), centered on x; each glyph's ink box goes to the UI audit.
void blocks(const char* s, int x, int y, int cell);
}  // namespace marble::paint
