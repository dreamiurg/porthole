// The levels: a constant table, in play order. The save stores a level index, so levels are only ever appended, never
// reordered or removed (like Biscuit's story ids). Coordinates are panel px from the tray's center, +y down; the goal
// is the gap in the rim at the top, `goalHalf` px either side of the middle.
//
// Every level carries its solution: a tilt sequence (milli-g from neutral, each held `ms`, whole 40 ms frames so the
// sim's playtests replay it frame for frame) that host/test_marble.cpp replays to a goal. That is the proof no level
// traps the ball; later the same recordings seed ghosts.
#pragma once
#include <stdint.h>

namespace marble {
struct Peg { int16_t x, y, r; };
struct Step { int16_t tx, ty; uint16_t ms; };
struct Level {
  int16_t startX, startY, goalHalf;
  uint8_t pegCount;
  Peg pegs[8];
  const Step* solution;
  uint8_t steps;
};

// ---- solutions (recorded) ----
constexpr Step SOLVE_1[] = {{0, -430, 240}, {0, -340, 240}, {0, -230, 240}, {0, -250, 480}, {0, -240, 240}, {0, -250, 240}, {0, -120, 240}};
constexpr Step SOLVE_2[] = {{-260, -340, 240}, {-210, -270, 240}, {-90, -90, 240}, {250, -340, 240}, {70, -240, 240}, {50, -240, 240}, {-30, 140, 240}, {200, -370, 240}, {80, -180, 240}, {-60, 130, 240}};
constexpr Step SOLVE_3[] = {{100, -420, 240}, {70, -260, 240}, {-30, 130, 240}, {-130, -410, 240}, {-60, -240, 240}, {0, 0, 240}, {-60, -350, 240}, {30, 130, 240}, {110, -400, 240}, {0, 0, 240}};
constexpr Step SOLVE_4[] = {{240, -360, 240}, {120, -180, 240}, {-70, 100, 240}, {-80, -420, 240}, {-30, -270, 240}, {0, 0, 240}, {-190, -330, 240}, {-70, -130, 240}, {40, 130, 240}, {-10, -430, 240}, {0, 0, 240}};
constexpr Step SOLVE_5[] = {{-160, -400, 240}, {-130, -320, 240}, {-80, -220, 240}, {-70, -140, 240}, {60, 130, 240}, {370, -210, 240}, {220, -90, 240}, {-80, 100, 240}, {-120, -410, 240}, {-50, -290, 240}, {0, 0, 240}};
constexpr Step SOLVE_6[] = {{240, 240, 240}, {190, -390, 240}, {-50, -150, 240}, {400, -160, 240}, {420, -80, 240}, {420, 50, 240}, {250, 210, 240}, {-60, 150, 240}, {-340, -260, 480}, {0, 0, 240}, {-320, -170, 240}, {-180, -110, 240}, {120, 80, 240}, {410, -120, 240}, {380, -120, 240}, {300, -300, 240}, {190, -380, 240}, {-70, -290, 240}, {-130, -390, 240}, {-30, -230, 240}, {0, 0, 240}};
template <int N> constexpr uint8_t count(const Step (&)[N]) { return (uint8_t)N; }

constexpr Level LEVELS[] = {
  // 1. an empty pitch: just roll it in
  {0, 110, 72, 0, {}, SOLVE_1, count(SOLVE_1)},
  // 2. one peg in the way
  {0, 110, 66, 1, {{0, -10, 30}}, SOLVE_2, count(SOLVE_2)},
  // 3. two pegs
  {0, 120, 62, 2, {{-55, 20, 26}, {60, -70, 26}}, SOLVE_3, count(SOLVE_3)},
  // 4. a wall of three pegs with a gap
  {0, 120, 60, 3, {{-130, -10, 38}, {-50, -10, 38}, {130, -10, 38}}, SOLVE_4, count(SOLVE_4)},
  // 5. pegs guarding the goal mouth
  {0, 110, 56, 3, {{-58, -120, 24}, {58, -120, 24}, {0, -40, 22}}, SOLVE_5, count(SOLVE_5)},
  // 6. a slalom
  {-60, 120, 56, 4, {{40, 90, 24}, {-50, 25, 24}, {50, -40, 24}, {-30, -110, 22}}, SOLVE_6, count(SOLVE_6)},
};
constexpr int NUM_LEVELS = sizeof LEVELS / sizeof LEVELS[0];
}  // namespace marble
