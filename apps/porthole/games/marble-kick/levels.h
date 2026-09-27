// The levels: a constant table, in play order. The save stores a level index (and its best stars), so levels are only
// ever appended; what a level holds may change, its index may not. Coordinates are panel px from the tray's center,
// +y down; the goal is the gap in the rim at the top, `goalHalf` px either side of the middle.
//
// A level is pegs (still, or moving: a defender on a line, the keeper across the goal's mouth), rails (straight wooden
// bars, the maze), holes (the ball drops in and comes back at the start), three stars to roll over, and how its goal
// moves round the rim (still, spinning, or swinging between two angles). Every level carries its solution: a tilt sequence (milli-g from
// neutral, each held `ms`, whole 40 ms frames so the sim's playtests replay it frame for frame) that
// host/test_marble.cpp replays to a goal; later the same recordings seed ghosts.
#pragma once
#include <stdint.h>

namespace marble {
// How a peg looks and moves. A moving peg goes from (x, y) to (bx, by) and back once every periodMs, easing at the
// ends (a sine), on the level's own clock (physics.h). The keeper's line is in the goal's own frame (its mouth at the
// top), so it rides round with a moving goal.
enum Kind : uint8_t { STILL, DEFENDER, KEEPER };
struct Peg { int16_t x, y, r; uint8_t kind; int16_t bx, by; uint16_t periodMs; };
struct Rail { int16_t x0, y0, x1, y1; };   // a straight bar, RAIL_R thick either side (physics.h)
struct Hole { int16_t x, y, r; };
// Where the goal is on the rim: degrees clockwise from the top. FIXED at a0; SPIN from a0 at a1 degrees a second,
// turning back every periodMs (0: never); SWING between a0 and a1, easing at the ends, once there and back every
// periodMs.
enum GoalMode : uint8_t { FIXED, SPIN, SWING };
struct GoalMove { uint8_t mode; int16_t a0, a1; uint16_t periodMs; };
constexpr GoalMove TOP = {FIXED, 0, 0, 0};
struct Star { int16_t x, y; };
struct Step { int16_t tx, ty; uint16_t ms; };
constexpr int MAX_PEGS = 6, MAX_RAILS = 10, MAX_HOLES = 4, STARS = 3;
struct Level {
  int16_t startX, startY, goalHalf;
  GoalMove goal;
  uint8_t pegCount;
  Peg pegs[MAX_PEGS];
  uint8_t railCount;
  Rail rails[MAX_RAILS];
  uint8_t holeCount;
  Hole holes[MAX_HOLES];
  Star stars[STARS];
  const Step* solution;
  uint8_t steps;
};
constexpr Peg peg(int x, int y, int r) { return {(int16_t)x, (int16_t)y, (int16_t)r, STILL, (int16_t)x, (int16_t)y, 0}; }
constexpr Peg defender(int x, int y, int bx, int by, int periodMs) {
  return {(int16_t)x, (int16_t)y, 20, DEFENDER, (int16_t)bx, (int16_t)by, (uint16_t)periodMs};
}
// The keeper paces the goal line: `span` px either side of the middle, `y` px below the center (negative: up).
constexpr Peg keeper(int y, int span, int periodMs) {
  return {(int16_t)-span, (int16_t)y, 18, KEEPER, (int16_t)span, (int16_t)y, (uint16_t)periodMs};
}

// ---- solutions (recorded) ----
inline constexpr Step SOLVE_1[] = {{-310, -270, 240}, {-170, -150, 240}, {-160, -150, 240}, {-90, -90, 240}, {430, -30, 240}, {260, -60, 240}, {210, -70, 240}, {200, -70, 480}, {170, -60, 240}, {0, 0, 240}, {-400, -150, 240}, {-250, -100, 240}, {-190, -100, 240}, {-110, -60, 240}, {130, -330, 240}, {50, -220, 240}, {20, -190, 240}, {0, 0, 240}};
inline constexpr Step SOLVE_2[] = {{-220, -350, 240}, {-120, -190, 240}, {-110, -180, 240}, {-110, -170, 240}, {0, 0, 240}, {300, -230, 240}, {160, -160, 240}, {130, -170, 240}, {110, -140, 240}, {0, 0, 240}, {-90, -320, 240}, {-60, -190, 240}, {0, 0, 240}};
inline constexpr Step SOLVE_3[] = {{-370, -170, 240}, {-210, -100, 240}, {-190, -90, 240}, {-190, -100, 240}, {-150, -70, 240}, {100, -320, 240}, {90, -150, 240}, {240, -170, 240}, {150, -160, 240}, {110, -90, 240}, {300, 50, 240}, {220, -10, 240}, {180, -10, 240}, {0, 0, 240}, {-130, -390, 240}, {-60, -220, 240}, {-50, -150, 240}};
inline constexpr Step SOLVE_4[] = {{400, -70, 240}, {190, -40, 240}, {0, 0, 240}, {260, -160, 240}, {160, -90, 240}, {0, 0, 240}, {-120, -310, 240}, {-110, -110, 240}, {-150, -240, 240}, {-90, -90, 240}, {-340, 140, 240}, {-220, 40, 240}, {-210, 20, 240}, {-120, 0, 240}, {-270, -10, 240}, {-220, 0, 240}, {-210, 0, 240}, {-190, 0, 240}, {0, 0, 240}, {140, -340, 240}, {120, -130, 240}, {220, -190, 240}, {130, -130, 240}, {0, 0, 240}, {250, -170, 240}, {170, -140, 240}, {100, -240, 240}, {-20, -260, 240}, {-110, -100, 240}};
inline constexpr Step SOLVE_5[] = {{380, -150, 240}, {210, -90, 240}, {200, -80, 480}, {140, -50, 240}, {-120, -340, 240}, {-50, -220, 240}, {-90, -70, 240}, {-150, -250, 240}, {-90, -200, 240}, {-80, -200, 240}, {0, 0, 240}, {-230, -80, 240}, {-120, 0, 240}, {-200, -220, 240}, {-70, -280, 240}, {80, -180, 240}};
inline constexpr Step SOLVE_6[] = {{-420, -100, 240}, {-250, -70, 240}, {-220, -50, 240}, {0, 0, 240}, {170, -390, 240}, {40, -250, 240}, {10, -220, 240}, {0, 0, 240}, {130, -310, 240}, {50, -150, 240}, {300, -310, 240}, {230, -210, 240}, {370, 60, 240}, {180, -30, 240}, {-120, 50, 240}, {-150, -400, 240}, {-130, -70, 240}};
inline constexpr Step SOLVE_7[] = {{-370, -180, 240}, {-200, -110, 240}, {-190, -90, 240}, {-190, -100, 240}, {-120, -60, 240}, {100, -330, 240}, {100, -140, 240}, {270, -140, 240}, {170, -150, 240}, {110, -100, 240}, {290, 60, 240}, {220, 10, 240}, {110, 40, 240}, {-140, -390, 240}, {-60, -220, 240}, {-30, -190, 240}, {0, 0, 240}};
inline constexpr Step SOLVE_8[] = {{-410, -50, 240}, {-220, -30, 240}, {0, 0, 240}, {-190, -250, 240}, {-130, -160, 240}, {0, 0, 240}, {110, -330, 240}, {60, -200, 240}, {0, 0, 240}, {310, -130, 240}, {190, -120, 240}, {150, -100, 240}, {0, 0, 240}, {310, 20, 240}, {130, 50, 240}, {180, -230, 240}, {200, -90, 240}, {220, -30, 240}, {220, 20, 240}, {110, 100, 240}};
inline constexpr Step SOLVE_9[] = {{400, -70, 240}, {190, -40, 240}, {0, 0, 240}, {260, -160, 240}, {160, -90, 240}, {0, 0, 240}, {-120, -310, 240}, {-110, -110, 240}, {-150, -240, 240}, {-90, -90, 240}, {-340, 140, 240}, {-220, 40, 240}, {-210, 20, 240}, {-120, 0, 240}, {-270, -10, 240}, {-220, 0, 240}, {-210, 0, 240}, {-190, 0, 240}, {0, 0, 240}, {70, -340, 240}, {90, -140, 240}, {30, -250, 240}, {-160, -120, 240}, {320, -280, 240}, {260, -230, 240}, {170, -150, 240}, {120, -110, 240}, {-30, -280, 240}, {-300, -70, 240}, {-240, 130, 240}};
inline constexpr Step SOLVE_10[] = {{-360, -200, 240}, {-200, -110, 240}, {-190, -100, 240}, {-130, -60, 240}, {140, -330, 240}, {40, -220, 240}, {20, -210, 240}, {0, -130, 240}, {230, -180, 240}, {10, -300, 240}, {-80, -230, 240}, {-140, -70, 240}, {80, -320, 240}, {400, 90, 240}, {180, -130, 240}, {20, -130, 240}, {-130, -370, 240}, {0, -270, 240}, {10, -160, 240}, {0, 0, 240}};
inline constexpr Step SOLVE_11[] = {{410, -40, 240}, {230, -20, 240}, {200, -20, 240}, {0, 0, 240}, {-130, -390, 240}, {-60, -220, 240}, {-50, -130, 240}, {-380, 80, 240}, {-220, 0, 240}, {-210, -20, 240}, {0, 0, 240}, {40, -300, 240}, {-220, -360, 240}, {-190, -350, 240}, {-80, -160, 240}, {60, -290, 240}, {20, -220, 240}, {10, -140, 240}, {-200, -190, 240}, {370, -40, 240}, {220, -40, 240}, {210, -60, 240}, {200, -80, 240}, {170, -120, 240}, {-20, -320, 240}, {-250, -130, 240}};
inline constexpr Step SOLVE_12[] = {{400, -70, 240}, {190, -40, 240}, {0, 0, 240}, {260, -160, 240}, {160, -90, 240}, {0, 0, 240}, {-120, -310, 240}, {-110, -110, 240}, {-150, -240, 240}, {-90, -90, 240}, {-340, 140, 240}, {-220, 40, 240}, {-210, 20, 240}, {-120, 0, 240}, {-270, -10, 240}, {-220, 0, 240}, {-210, 0, 240}, {-190, 0, 240}, {0, 0, 240}, {140, -340, 240}, {120, -130, 240}, {220, -190, 240}, {130, -130, 240}, {-50, -170, 240}, {-430, 10, 240}, {-430, -30, 240}, {-420, -70, 240}, {-400, -100, 240}, {-300, -100, 240}, {-230, -80, 240}, {-120, -50, 240}, {420, -60, 240}, {280, -80, 240}, {200, -90, 240}, {180, -110, 240}, {410, -130, 240}, {260, -120, 240}, {260, -90, 240}, {70, -250, 240}, {-170, -100, 240}};
template <int N> constexpr uint8_t count(const Step (&)[N]) { return (uint8_t)N; }

// Each level: start x, y; the goal's half width and how it moves; pegs; rails; holes; stars; its solution.
inline constexpr Level LEVELS[] = {
  // 1. an empty pitch: tilt it and the ball rolls in
  {0, 110, 72, TOP, 0, {}, 0, {}, 0, {}, {{-90, 30}, {90, -40}, {0, -100}}, SOLVE_1, count(SOLVE_1)},
  // 2. one peg in the way
  {0, 110, 66, TOP, 1, {peg(0, -10, 30)}, 0, {}, 0, {}, {{-75, -10}, {75, -10}, {0, -125}}, SOLVE_2, count(SOLVE_2)},
  // 3. a wall across the middle: straight up gets stuck, go round an end
  {0, 110, 64, TOP, 0, {}, 1, {{-110, -20, 110, -20}}, 0, {}, {{-148, -20}, {148, -20}, {0, -110}}, SOLVE_3,
   count(SOLVE_3)},
  // 4. a zig-zag of two walls, and a hole on the straight way up
  {0, 125, 62, TOP, 0, {}, 2, {{-200, 30, 90, 30}, {-90, -65, 200, -65}}, 1, {{0, 75, 24}},
   {{140, 30}, {0, -18}, {-140, -65}}, SOLVE_4, count(SOLVE_4)},
  // 5. a defender pacing past the way round the wall
  {0, 110, 62, TOP, 1, {defender(40, -110, 150, -110, 3000)}, 1, {{-200, -30, 110, -30}}, 0, {},
   {{150, -30}, {-120, -100}, {60, -150}}, SOLVE_5, count(SOLVE_5)},
  // 6. a cup that catches the ball, two defenders, and a hole on the right-hand way round
  {0, 120, 62, TOP, 2, {defender(-130, 40, 130, 40, 3600), defender(-120, -130, -20, -130, 2800)}, 3,
   {{-60, -80, 60, -80}, {-60, -80, -60, -20}, {60, -80, 60, -20}}, 1, {{125, -100, 24}},
   {{-120, -40}, {120, -40}, {0, -150}}, SOLVE_6, count(SOLVE_6)},
  // 7. the keeper: go round the wall, then wait for a way past him
  {0, 110, 66, TOP, 1, {keeper(-145, 65, 3200)}, 1, {{-100, -20, 100, -20}}, 0, {},
   {{-140, -20}, {140, -20}, {0, -100}}, SOLVE_7, count(SOLVE_7)},
  // 8. the goal swings side to side, the keeper with it, past a cup with a hole below it
  {0, 120, 66, {SWING, -45, 45, 10000}, 1, {keeper(-150, 55, 2600)}, 3,
   {{-90, -30, 90, -30}, {-90, -30, -90, 40}, {90, -30, 90, 40}}, 1, {{0, 75, 24}}, {{-140, -20}, {140, -20}, {0, -100}},
   SOLVE_8, count(SOLVE_8)},
  // 9. the zig-zag again, a defender pacing above it and a swinging goal
  {0, 125, 62, {SWING, -30, 30, 8000}, 1, {defender(-150, -125, -10, -125, 3000)}, 2,
   {{-200, 30, 90, 30}, {-90, -65, 200, -65}}, 1, {{0, 75, 24}}, {{140, 30}, {0, -18}, {-150, -65}}, SOLVE_9,
   count(SOLVE_9)},
  // 10. pockets at the rim and a cup: the rim is no road to the goal
  {0, 120, 64, TOP, 1, {keeper(-155, 55, 3000)}, 5,
   {{-190, -50, -95, -110}, {190, -50, 95, -110}, {-45, -60, 45, -60}, {-45, -60, -45, -10}, {45, -60, 45, -10}}, 0,
   {}, {{-130, 20}, {130, 20}, {0, -100}}, SOLVE_10, count(SOLVE_10)},
  // 11. a slalom of short walls, two defenders and a hole, the goal swinging
  {-100, 100, 62, {SWING, -35, 35, 9000}, 2, {defender(20, 60, 140, 60, 3200), defender(-140, -60, -20, -60, 2600)},
   3, {{-200, 20, -40, 20}, {40, -20, 200, -20}, {-60, -110, 60, -110}}, 1, {{-40, 60, 22}},
   {{100, 60}, {-100, -60}, {0, -140}}, SOLVE_11, count(SOLVE_11)},
  // 12. everything: the zig-zag, a defender above it, and the keeper on a goal that turns and turns back
  {0, 125, 62, {SPIN, 0, 12, 5000}, 2, {defender(-150, -120, -40, -120, 2600), keeper(-150, 50, 2800)}, 2,
   {{-200, 30, 90, 30}, {-90, -65, 200, -65}}, 1, {{0, 75, 24}}, {{140, 30}, {0, -18}, {-140, -65}}, SOLVE_12,
   count(SOLVE_12)},
};
constexpr int NUM_LEVELS = sizeof LEVELS / sizeof LEVELS[0];
}  // namespace marble
