// Initial tuning from the design brief (docs/superpowers/specs/porthole-racing/gameplay-brief.md): every number is a
// playtest default, not a validated value. Speeds are normalized (1.0 = dry-road maximum = V_MAX m/s); distances in
// track meters; lateral position in road half-widths (the road edges are -1 and +1).
#pragma once
#include <stdint.h>

namespace racing {
constexpr uint32_t STEP_MS = 20, MAX_FRAME_MS = 100;
constexpr float DT = STEP_MS / 1000.0f;
constexpr int LAPS = 3, CARS = 4, PLAYER = 0, MAX_ROCKETS = 4, MAX_PUFFS = 6, MAX_INVENTORY = 3;
constexpr float V_MAX = 30;                        // m/s at speed 1.0
constexpr float ACCEL = 1 / 2.5f;                  // 0 -> 1.0 in 2.5 s
constexpr float DECEL = 1.5f;                      // falling to a lower cap
constexpr float BOOST_CAP = 1.35f;
constexpr uint32_t BOOST_MS = 1500;
constexpr float CONTACT_LOSS = 0.65f, OBSTACLE_LOSS = 0.55f, ROCKET_LOSS = 0.60f, HIT_FLOOR = 0.25f;
constexpr float SLOW_CAP = 0.65f, OFFROAD_CAP = 0.55f;
constexpr uint32_t PROTECT_MS = 800, AIR_MS = 700, ROCKET_COOLDOWN_MS = 450, ROCKET_LIFE_MS = 1200;
constexpr float ROCKET_SPEED = 2.5f;               // times V_MAX
constexpr uint32_t COUNTDOWN_MS = 3000;
// Lateral handling: full lock moves STEER_RATE half-widths a second at full speed; a full-strength bend pushes the
// car outward at DRIFT, so a bend never needs more than about 60% lock.
constexpr float STEER_RATE = 1.5f, DRIFT = 0.85f, GRIP_TAU = 0.06f, ICE_TAU = 0.25f, ICE_RESPONSE = 0.75f;
constexpr float RAIL = 1.35f;                      // guardrail: the car stops sliding here, no penalty
constexpr float CAR_HALF_W = 0.16f, CAR_LEN = 4.5f;
constexpr float PICKUP_HALF_W = 0.26f;
constexpr int GRID_SPACING = 7;                    // m between grid slots
// Adaptive rivals (a user decision of 2026-10-31): each kid has a level per track, +1 after a win (+2 when no rival
// was within CLEAR_LEAD m), -2 after any other finish (halved instead above 0, so a losing streak from the top is
// back to 0 in about three races). Below 0 the rivals' top speed drops, gently at first and faster deeper down
// (slowdown), and from LEVEL_COURTESY on a rival with the kid close behind moves out of its lane.
// Above 0, levels 1-9 each give one rival one more Boost charge (the slower two first, the pace-setter last, three at
// most, like the kid's), fired only while chasing (the kid CHASE_MIN..CHASE_MAX m ahead) and never within
// RIVAL_BOOST_GAP_MS of another rival's boost. Levels 10-15 raise a slower rival's top by LEVEL_TOP_STEP; levels
// 16-18 let a rival take its three charges back each lap, as the kid does from road pickups. Ordered by a simulated
// sweep so win rates fall with level on every track, roughly (one track wobbles by up to 15 points between
// neighbours): raising rival pace first made races easier (less traffic to hit). No rival's dry-road top exceeds
// the kid's, and the slower two stay below 1.0: rival 2 level with the pace-setter forms a two-abreast wall.
constexpr uint8_t LADDER_CHARGE[] = {1, 2, 1, 2, 1, 2, 3, 3, 3};   // the rival given a charge at levels 1..9
constexpr uint8_t LADDER_TOP[] = {2, 2, 1, 1, 1, 1};               // the rival a step faster at levels 10..15
constexpr uint8_t LADDER_REFILL[] = {3, 1, 2};                     // the rival recharged each lap from 16..18
constexpr int LEVEL_MIN = -16, LEVEL_MAX = sizeof LADDER_CHARGE + sizeof LADDER_TOP + sizeof LADDER_REFILL;
constexpr int LEVEL_COURTESY = -7, LEVEL_SEED_MAX = 3;   // a never-raced track starts one below the best, at most 3
constexpr float LEVEL_SLOW_STEP = 0.0035f, LEVEL_SLOW_CURVE = 0.0003f, LEVEL_TOP_STEP = 0.01f;
constexpr float CHASE_MIN = 3, CHASE_MAX = 40, CLEAR_LEAD = 100;
constexpr uint32_t RIVAL_BOOST_GAP_MS = 2500;
constexpr float slowdown(int level) { return 1 + LEVEL_SLOW_STEP * level - LEVEL_SLOW_CURVE * level * level; }   // < 0
}  // namespace racing
