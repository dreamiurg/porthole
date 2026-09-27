// Tilt FC's look, drawn in code on the RGB565 surface: a sunny street court seen from above, blue-grey asphalt with
// chalk lines inside a concrete curb, warm sandstone paving round it, white goal frames with a diamond net, teal (the
// kid's team) against coral, and a low sun in the upper left throwing long shadows to the lower right. Its own flat
// colors (render.cpp) and Rubik Mono One (generated/fonts.h), shared with no other game.
//
// Everything goes through one clipped span writer (and font::clip for the few words), so any rectangle can be
// repainted alone: the court is computed row by row from the geometry in match.h, no cached image. That is what lets
// game.cpp repaint only what moved in each of the panel's two buffers. Panel px, absolute (the court's middle is
// 240,240; match.h's positions are relative to it).
#pragma once
#include <stdint.h>
#include "gfx565.h"
#include "match.h"

namespace fc::paint {
struct Box { int x0, y0, x1, y1; };   // x1, y1 exclusive; empty when x1 <= x0
constexpr Box FULL = {0, 0, gfx565::W, gfx565::H};
constexpr Box NONE = {0, 0, 0, 0};
inline bool empty(const Box& b) { return b.x1 <= b.x0 || b.y1 <= b.y0; }
inline bool same(const Box& a, const Box& b) { return a.x0 == b.x0 && a.y0 == b.y0 && a.x1 == b.x1 && a.y1 == b.y1; }
Box unite(const Box& a, const Box& b);
void clip(const Box& b);   // every draw below, text included, touches only this box

constexpr int CX = gfx565::CX, CY = gfx565::CY;
// What a player covers, his long shadow and the marker over his head included; a ball, its shadow included.
inline Box playerBox(int x, int y) { return {x - 27, y - 32, x + 32, y + 36}; }
inline Box ballBox(int x, int y) { return {x - BALL_R - 1, y - BALL_R - 1, x + BALL_R + 5, y + BALL_R + 6}; }
inline Box aimBox(int x, int y) { return {x - 10, y - 10, x + 11, y + 11}; }

// Every string the game draws: the shirt numbers (the kid's favourite is 10), GOAL!, and single digits (scores, the
// countdown). host/test_tiltfc.cpp holds them to the font's GLYPHS: a glyph outside it would draw nothing.
inline constexpr const char* NUMBERS[PLAYERS] = {"10", "9", "1", "4", "8", "1"};
inline constexpr const char* GOAL_WORD = "GOAL!";
enum Kit : uint8_t { KIT_TEAL, KIT_TEAL_KEEPER, KIT_CORAL, KIT_CORAL_KEEPER };
struct Look { Kit kit; uint8_t hair; const char* number; bool keeper, target; };
void court();                                    // pavement, curb, asphalt, chalk and both goals: the whole clip box
void playerShadow(int x, int y);                 // a long shadow from the feet toward the lower right
void ballShadow(int x, int y);
void ring(int x, int y);                         // the sun-yellow ring round the kid's player, on the ground
void player(int x, int y, Vec face, const Look& l);
void ball(int x, int y);
void aimSpot(int x, int y);                      // where a shot will cross the goal line: a sun-yellow arrowhead
void arrow(int x, int y, Vec dir, int length);   // Calibrate: a chalk arrow from the player, the way he will run
void readyRing(int x, int y);                    // Calibrate: held steady
// The right-hand scoreboard: coral's goals above (their end), teal's below, the match clock between.
void scoreboard(int teal, int coral);
void clock(float used);                          // 0..1 of the match played: a chalk dial filling with asphalt
void leaveSign(bool pressed, float hold);        // the way out, on the left: hold 0..1 fills its rim (in the match)
void goSign(int x, int y, bool pressed);         // a sun-yellow diamond road sign with a go arrow
void banner(const char* s, int y);               // big words across the court (GOAL!, the countdown), centered
void result(int teal, int coral, bool won);      // the Full time page: two big score tiles, a cup for a win
constexpr int SIGN_X = 52, SIGN_Y = CY, SIGN_R = 30;          // the leave sign's middle and radius
constexpr int BOARD_X = 428, TILE = 46, CLOCK_R = 17;         // the scoreboard's column and sizes
inline Box clockBox() { return {BOARD_X - CLOCK_R - 1, CY - CLOCK_R - 1, BOARD_X + CLOCK_R + 2, CY + CLOCK_R + 2}; }
}  // namespace fc::paint
