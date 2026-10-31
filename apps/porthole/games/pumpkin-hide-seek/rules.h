#pragma once
#include <stdint.h>

namespace pumpkin {
constexpr uint32_t STEP_MS = 10, MAX_FRAME_MS = 100, CATCH_MS = 900, PHEW_MS = 1200, FIRST_REST_MS = 3000;
// A hidden pumpkin tilted past RUSTLE_MG for RUSTLE_MS rustles its spot; Grandma's next move checks it.
// 400 mg is 0.8x the full rolling force (a lean is fine, a shove is not); retune them together.
// 1 s gives a 6-year-old time to read SHH! and level the device before she hears.
constexpr uint32_t RUSTLE_MS = 1000;
constexpr float RUSTLE_MG = 400;
// The ghost, once a round: a search or warning stops for GHOST_REST_MS; a rest gets GHOST_EXTRA_MS longer.
constexpr uint32_t GHOST_REST_MS = 5000, GHOST_EXTRA_MS = 4000, DISTRACT_MS = 1500;
constexpr float SPOTS[] = {36, 80, 124}, DOOR_X = 24;
constexpr int CANDIES = 3;
constexpr uint8_t ALL_CANDY = (1 << CANDIES) - 1;
// Grandma loops Rest -> Warn -> Search. Each Warn picks the next move.
enum class Phase : uint8_t { Rest, Warn, Search };
enum class Move : uint8_t { Sweep, Fast, TurnBack, Check };
struct Round {
  float x = 80, velocity = 0;
  int hidden = -1;
  uint32_t elapsed = 0, exposure = 0;
  bool caught = false, won = false;
  uint32_t seed = 1;
  float candy[CANDIES] = {58, 102, 134};
  uint8_t got = 0;
  uint32_t sinceCandy = UINT32_MAX;  // ms since the last pickup (for the sparkle)
  Phase phase = Phase::Rest;
  Move move = Move::Sweep;
  bool fromRight = true;  // the sweep starts at Grandma's side
  int spot = -1;          // Check: the spot she looks behind
  uint32_t phaseMs = 0, phaseLen = FIRST_REST_MS;
  bool covered = false;  // the beam is over the hidden pumpkin
  uint32_t phew = 0;     // ms of "PHEW!" left
  uint32_t moves = 0;    // Grandma's moves so far: the first is always a plain sweep
  uint32_t rustle = 0;   // ms of hard tilt while hidden
  int heard = -1;        // spot she heard rustle; her next move checks it
  bool fromHeard = false;  // the current move is that heard check (a ghost can hand it back)
  bool ghostUsed = false;
  uint32_t distract = 0;  // ms of "WHAT WAS THAT?" left
};
Round newRound(uint32_t seed);
int nearby(const Round& round);
bool warning(const Round& round);
bool searching(const Round& round);
int checking(const Round& round);  // spot Grandma announces or checks, else -1
float beamX(const Round& round);
bool doorOpen(const Round& round);
void step(Round& round, float roll, bool action, bool ghost = false);
}  // namespace pumpkin
