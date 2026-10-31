// One race: four cars on one circuit, a fixed 20 ms step, no input or drawing dependencies. Cars only move forward
// along the course, so the unwrapped distance is the validated progress: a lap counts only once the whole circuit
// has been driven, and every gate/ramp/hazard is found by sweeping from the last step's distance to this one's.
#pragma once
#include <stdint.h>
#include "track.h"
#include "tune.h"

namespace racing {
enum class Phase : uint8_t { Countdown, Racing, Finished };
struct Car {
  float dist = 0, x = 0, vx = 0, speed = 0, top = 1;   // top: dry-road pace (rivals below 1)
  uint32_t boostMs = 0, protectMs = 0, airMs = 0, wobbleMs = 0, lineMs = 0, finishMs = 0;
  uint16_t solid = 0;                                 // obstacles this car is inside (bit per feature)
  uint8_t grid = 0;
  uint8_t boosts = 0;                                 // a rival's Boost charges (the player's are Race::inventory)
  bool refill = false;                                // a rival taking its charges back at every lap line
  bool finished = false;
};
struct Rocket { float dist = 0, x = 0; uint32_t ageMs = 0; bool live = false; };
struct Puff { float dist = 0, x = 0; uint32_t ms = 0; };
struct Controls { float steer = 0; bool boost = false, rocket = false; };   // steer -1..1; presses are fresh edges
struct Race {
  const Track* track = nullptr;
  Phase phase = Phase::Countdown;
  Car car[CARS];
  Rocket rocket[MAX_ROCKETS];
  Puff puff[MAX_PUFFS];
  uint32_t ms = 0, raceMs = 0, cooldownMs = 0, calloutMs = 0, fullMs[2] = {0, 0}, seed = 1;   // calloutMs: a new lap's banner
  uint32_t hitMs = 0;               // "Hit!" after the player's rocket finds a rival
  uint32_t firstMs = 0;             // race time the player has led, no rival ahead
  uint32_t rivalBoostMs = 0;        // a rival boosted lately: the next one waits, so boosts never gang up on the kid
  uint32_t lapStartMs = 0, lapMs[LAPS] = {};   // the player's laps, line to line (0 until driven)
  uint16_t taken = 0;               // pickup gates collected on the player's current lap (bit per feature)
  uint8_t contact = 0;              // car pairs touching last step (bit per pair)
  uint8_t inventory[2] = {1, 1};    // BOOST, ROCKET
  uint8_t place = CARS, finishedCount = 0, playerLap = 0;
  int8_t level = 0;                 // the kid's level on this track (clampLevel): below 0 the rivals are slower
  int8_t rung = 0;                  // the charge/pace ladder rung it puts the rivals on (tune.h LEVEL_SHIFT)
  uint16_t rocketsFired = 0, rocketHits = 0, pickups = 0, jumps = 0, impacts = 0;
};

// level: how hard the rivals push (tune.h; clampLevel'd, 0 = the brief's pace except on a shifted track). The race is
// a pure function of the track (one of TRACK[]), seed, level and inputs.
void start(Race& r, const Track& t, uint32_t seed, int level);
int clampLevel(int level, int track);   // LEVEL_MIN..the track's top (LEVEL_MAX less its LEVEL_SHIFT)
// After a finished race (tune.h): a win steps up one, two after a runaway; a loss at 0 or below steps down two, from
// 1..LEVEL_GENTLE to 0, from there to LEVEL_EASY straight to -2, and above that steps down LEVEL_HIGH_LOSS.
int nextLevel(int level, int place, bool runaway);
bool runaway(const Race& r);   // the player led for at least RUNAWAY_LED / RUNAWAY_OF of the race
void step(Race& r, const Controls& in);
int lapOf(const Race& r, int car);       // 1..LAPS, the lap being driven (LAPS once finished)
int placeOf(const Race& r, int car);     // 1..CARS
void order(const Race& r, uint8_t out[CARS]);   // car ids, first place first
uint32_t bestLap(const Race& r);       // the player's fastest finished lap, 0 if none
float hitSpeed(float before, float factor);    // the impact rule: min(before, max(HIT_FLOOR, before * factor))
bool airborne(const Car& c);
// What is under a grounded car at (dist, x): the strongest speed cap (1 = none) and whether it is on ice.
float terrainCap(const Track& t, float dist, float x, bool* onIce);
}  // namespace racing
