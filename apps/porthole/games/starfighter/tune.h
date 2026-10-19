// Native panel pixels, pixels/second and milliseconds. Tune on the physical puck after host replay.
#pragma once
#include <stdint.h>
namespace starfighter {
constexpr uint32_t STEP_MS = 20, MAX_FRAME_MS = 100;
constexpr float DT = STEP_MS / 1000.0f;
constexpr float PLAYER_Y = 360, PLAYER_MIN_X = 132, PLAYER_MAX_X = 348;
constexpr float PLAYER_SPEED = 190, PLAYER_RADIUS = 10;
constexpr float BANK_DEAD = 0.08f;
constexpr uint32_t INVULNERABLE_MS = 1600;
constexpr float PLAYER_BOLT_SPEED = 440, ENEMY_BOLT_SPEED = 96;
constexpr float CANNON_X = 34, CANNON_Y = 22, LASER_CONVERGE_SPEED = 280;
constexpr float ENEMY_MIN_Y = 156;  // Heavy sprite top clears the compact rim HUD.
constexpr float ENEMY_HALF_W = 26, ENEMY_HALF_H = 20;
constexpr float HEAVY_HALF_W = 34, HEAVY_HALF_H = 24;
constexpr float TURRET_HALF_W = 20, TURRET_HALF_H = 21;
constexpr float ENEMY_RAM_RADIUS = 18, TURRET_RAM_RADIUS = 24;
constexpr float WAVE_SPEED_GAIN = 0.04f;
constexpr uint32_t WAVE_FIRE_GAIN_MS = 350;
constexpr int MAX_ENEMIES = 16, MAX_BOLTS = 64, MAX_EFFECTS = 16, MAX_OBSTACLES = 8;
constexpr int WAVES = 4, GROUPS_PER_WAVE = 7, SHIPS_PER_GROUP = 4;
constexpr uint32_t GROUP_GAP_MS = 1200, WAVE_CLEAR_MS = 1600;
constexpr uint32_t TRENCH_ENTRY_MS = 2400, TRENCH_MS = 45000, OBSTACLE_MS = 3200;
constexpr uint32_t TARGET_PASS_MS = 7000, TORPEDO_MS = 1800, EXPLOSION_MS = 2600;
constexpr float TARGET_TOLERANCE = 36;
}  // namespace starfighter
