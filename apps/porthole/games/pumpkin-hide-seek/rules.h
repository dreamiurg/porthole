#pragma once
#include <stdint.h>

namespace pumpkin {
constexpr uint32_t STEP_MS = 10, MAX_FRAME_MS = 100, CATCH_MS = 900, PHEW_MS = 1200, FIRST_REST_MS = 3000;
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
};
Round newRound(uint32_t seed);
int nearby(const Round& round);
bool warning(const Round& round);
bool searching(const Round& round);
int checking(const Round& round);  // spot Grandma announces or checks, else -1
float beamX(const Round& round);
bool doorOpen(const Round& round);
void step(Round& round, float roll, bool action);
}  // namespace pumpkin
