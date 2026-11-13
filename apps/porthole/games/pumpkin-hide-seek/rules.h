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
// A Check naming the spot it hides in startles it out: it looks scared under a "!" this long (0.4 s read as nothing,
// kid playtest 5, m4), hopping for the first STARTLE_HOP_MS.
constexpr uint32_t STARTLE_MS = 1000, STARTLE_HOP_MS = 400;
constexpr float SPOTS[] = {36, 80, 124}, DOOR_X = 24;
constexpr int CANDIES = 3;
constexpr uint8_t ALL_CANDY = (1 << CANDIES) - 1;
// Grandma loops Rest -> Warn -> Search. Each Warn picks the next move.
enum class Phase : uint8_t { Rest, Warn, Search };
enum class Move : uint8_t { Sweep, Fast, TurnBack, Check };
struct Round {
  float x = 80, velocity = 0;
  int heading = 0;  // the way it last rolled, -1 or 1 (0: not yet this round)
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
  bool close = false;    // the beam has been on the pumpkin this search: PHEW! when the search ends
  uint32_t phew = 0;     // ms of "PHEW!" left
  uint32_t moves = 0;    // Grandma's moves so far: the first is always a plain sweep
  uint32_t rustle = 0;   // ms of hard tilt while hidden
  bool settled = false;  // the tilt has eased below RUSTLE_MG since this hide began: only then can it rustle
  int heard = -1;        // spot she heard rustle; her next move checks it
  bool fromHeard = false;  // the current move is that heard check (a ghost can hand it back)
  bool ghostUsed = false;
  uint32_t distract = 0;  // ms of "WHAT WAS THAT?" left
  uint32_t startle = 0;   // ms of the startle left: a Check named its spot, so it jumped out
};
Round newRound(uint32_t seed);
int nearby(const Round& round);  // the spot in reach, else -1
int cover(const Round& round);   // the spot in reach it can hide in (not the one a Check names), else -1
int arriving(const Round& round, float roll, uint32_t ms);  // the cover it rolls into within `ms` at this tilt, else -1
int coverDir(const Round& round);  // the way to cover, -1 or 1: FIND COVER's arrow
bool warning(const Round& round);
bool searching(const Round& round);
bool covered(const Round& round);  // the beam is over the hidden pumpkin
bool named(const Round& round);    // out in the open within the beam's reach of the spot a Check names
int checking(const Round& round);  // spot Grandma announces or checks, else -1
float beamX(const Round& round);
bool doorOpen(const Round& round);
// hold: a finger that touched the action sign (pressed there, or slid on within reach of cover) is still on the glass.
// The pumpkin hides the moment it is within reach of cover while held and comes out when the finger lifts: no toggle,
// so a double tap can't pop it out. The spot a Check names is not cover: one hidden there jumps out (`startle`).
void step(Round& round, float roll, bool hold, bool ghost = false);
}  // namespace pumpkin
