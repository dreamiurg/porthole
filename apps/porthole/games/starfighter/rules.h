// Pure bounded simulation. Coordinates are absolute native 480x480 screen pixels; +y points down.
#pragma once
#include "tune.h"
namespace starfighter {
enum class Stage : uint8_t { Squadron, WaveClear, TrenchEntry, Trench, Target, Torpedo, Explosion, Victory, Defeat };
enum class Formation : uint8_t { V, Swoop, Heavy };
struct Player { float x = 240, y = PLAYER_Y, bank = 0; uint32_t invulnerableMs = 0; };
struct Enemy {
  float x = 0, y = 0, homeX = 0, phase = 0;
  uint32_t fireMs = 0, lastVolley = 0;
  uint8_t hp = 0, group = 0;
  Formation formation = Formation::V;
  bool active = false, heavy = false;
};
struct Bolt {
  float x = 0, y = 0, vx = 0, vy = 0;
  uint32_t volley = 0;
  bool enemy = false, active = false;
  float aimX = 0;  // Player beams converge from wide cannon tips, then continue straight.
};
struct Effect { float x = 0, y = 0; uint32_t ms = 0; bool large = false, active = false; };
struct Obstacle { float x = 0, y = 0; bool active = false; };
struct Run {
  Stage stage = Stage::Squadron;
  Player player{};
  Enemy enemies[MAX_ENEMIES]{};
  Bolt bolts[MAX_BOLTS]{};
  Effect effects[MAX_EFFECTS]{};
  Obstacle obstacles[MAX_OBSTACLES]{};
  uint32_t ms = 0, stageMs = 0, score = 0, seed = 1, volleys = 0;
  uint32_t obstacleMs = 0, targetPasses = 0, groupClearMs = 0;
  uint8_t lives = 3, wave = 0, groupsSpawned = 0;
  float targetX = 240;
};
// Restart is a complete reset with deterministic, local PRNG; no persisted progress or wall clock.
void restart(Run& run, uint32_t seed);
// bank is normalized after grip calibration; tap is a single press edge, never a held finger.
void step(Run& run, float bank, bool tap);
bool playing(Stage stage);
bool aligned(const Run& run);
const char* stageName(Stage stage);
}  // namespace starfighter
