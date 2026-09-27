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
constexpr Step SOLVE_1[] = {{0, -430, 240}, {0, -260, 240}, {0, -220, 240}, {0, -230, 240}, {0, -220, 480}, {0, -230, 240}, {0, -220, 240}, {0, -180, 240}};
constexpr Step SOLVE_2[] = {{-290, -310, 240}, {-180, -190, 240}, {-130, -140, 240}, {90, 100, 240}, {200, -380, 240}, {60, -210, 240}, {60, -220, 240}, {40, -180, 240}, {160, -210, 240}, {90, -200, 240}, {80, -160, 240}};
constexpr Step SOLVE_3[] = {{100, -420, 240}, {70, -250, 240}, {-30, 120, 240}, {-140, -350, 240}, {-30, -210, 240}, {-40, -200, 240}, {30, 130, 240}, {-60, -350, 240}, {0, 0, 240}, {110, -330, 240}, {60, -120, 240}};
constexpr Step SOLVE_4[] = {{240, -360, 240}, {120, -180, 240}, {-70, 100, 240}, {-90, -400, 240}, {-10, -200, 240}, {-10, -230, 240}, {0, 130, 240}, {-210, -320, 240}, {-70, -110, 240}, {30, -300, 240}, {-10, -180, 240}};
constexpr Step SOLVE_5[] = {{-190, -380, 240}, {-120, -240, 240}, {0, 0, 240}, {50, -340, 240}, {-10, -210, 240}, {-20, -230, 240}, {-20, -120, 240}, {300, -210, 240}, {120, -110, 240}, {-10, -290, 240}, {-90, 100, 240}};
constexpr Step SOLVE_6[] = {{290, 290, 240}, {110, -410, 240}, {-70, -230, 240}, {420, -50, 240}, {420, -90, 240}, {430, 30, 240}, {390, 190, 240}, {-40, 220, 240}, {-340, -260, 240}, {-330, -270, 240}, {-100, -90, 240}, {-280, -110, 240}, {-170, -130, 240}, {100, 60, 240}, {410, -110, 240}, {360, -110, 240}, {270, -340, 240}, {160, -390, 240}, {-120, -290, 240}, {-140, -340, 240}, {30, -200, 240}, {-10, -130, 240}};
template <int N> constexpr uint8_t count(const Step (&)[N]) { return (uint8_t)N; }

constexpr Level LEVELS[] = {
  // 1. an empty pitch: just roll it in
  {0, 110, 72, 0, {}, SOLVE_1, count(SOLVE_1)},
  // 2. one peg in the way
  {0, 110, 66, 1, {{0, -10, 30}}, SOLVE_2, count(SOLVE_2)},
  // 3. two pegs
  {0, 120, 62, 2, {{-55, 20, 26}, {60, -70, 26}}, SOLVE_3, count(SOLVE_3)},
  // 4. a wall of three pegs across the pitch, closed at the rim, with one gap
  {0, 120, 60, 3, {{-146, -10, 42}, {-62, -10, 42}, {146, -10, 42}}, SOLVE_4, count(SOLVE_4)},
  // 5. pegs guarding the goal mouth: two against the rim either side, one in front splitting the way in two
  {0, 110, 70, 3, {{-115, -105, 33}, {115, -105, 33}, {0, -105, 18}}, SOLVE_5, count(SOLVE_5)},
  // 6. a slalom
  {-60, 120, 56, 4, {{40, 90, 24}, {-50, 25, 24}, {50, -40, 24}, {-30, -110, 22}}, SOLVE_6, count(SOLVE_6)},
};
constexpr int NUM_LEVELS = sizeof LEVELS / sizeof LEVELS[0];
}  // namespace marble
