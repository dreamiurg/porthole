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
// Adaptive rivals (a user decision of 2026-10-31): each kid has a level per track (race.h nextLevel). A win adds one,
// two after a runaway (the kid led for RUNAWAY_LED/RUNAWAY_OF of the race). A loss at 0 or below takes two; from
// 1..LEVEL_GENTLE it drops to 0; from there to LEVEL_EASY straight to -2 (there the rivals only boost to pass a kid
// who leads, so a kid stuck behind would get the same race again; at -2 they are slower); above that it takes
// LEVEL_HIGH_LOSS, so from the top (18) a losing streak is below 0 after four losses (15, 12, 9, -2). A competent
// driver settles near two wins in three on every track (55-72% in simulation).
// Below 0, on every track, the rivals' top speed drops, gently first and faster deeper down (slowdown), and from
// LEVEL_COURTESY on a rival with the kid close behind moves out of its lane. Separately a level is a rung of a
// charge-and-pace ladder: rungs 1-9 each give one rival one more Boost charge (the slower two first, the pace-setter
// last, three at most, like the kid's), fired only while chasing (the kid CHASE_MIN..CHASE_MAX m ahead) and never
// within RIVAL_BOOST_GAP_MS of another rival's boost; rungs 10-15 raise a slower rival's top by LEVEL_TOP_STEP;
// rungs 16-18 let a rival take its three charges back each lap, as the kid does from road pickups. The rung is the
// level (none below 0), except on the tracks LEVEL_SHIFT moves up. Ordered by a simulated sweep so win rates fall
// with level, roughly (a track can wobble by up to 15 points between neighbours, and Snow Track, Industrial Ruins and
// Night Run drop 30-36 from -1 to 0): raising rival pace first made races easier (less traffic to hit). No rival's
// dry-road top exceeds the kid's, and the slower two stay below 1.0: rival 2 level with the pace-setter forms a
// two-abreast wall.
constexpr uint8_t LADDER_CHARGE[] = {1, 2, 1, 2, 1, 2, 3, 3, 3};   // the rival given a charge at levels 1..9
constexpr uint8_t LADDER_TOP[] = {2, 2, 1, 1, 1, 1};               // the rival a step faster at levels 10..15
constexpr uint8_t LADDER_REFILL[] = {3, 1, 2};                     // the rival recharged each lap from 16..18
constexpr int LEVEL_MIN = -16, LEVEL_MAX = sizeof LADDER_CHARGE + sizeof LADDER_TOP + sizeof LADDER_REFILL;
constexpr int LEVEL_COURTESY = -7, LEVEL_SEED_MAX = 3;   // a never-raced track starts one below the best, in 0..3
constexpr int LEVEL_GENTLE = 3, LEVEL_EASY = sizeof LADDER_CHARGE, LEVEL_HIGH_LOSS = 3;
// By track: how many rungs up the charge-and-pace ladder a level sits from 0 up, fading out by -LEVEL_SHIFT_FADE (the
// slow-down below 0 is not shifted). Industrial Ruins and Night Run hand a capable driver a long lead, so their
// rivals need more to keep the race close: there level 0 already has rival boosts.
constexpr int8_t LEVEL_SHIFT[] = {0, 12, 0, 8};
constexpr int LEVEL_SHIFT_FADE = 8;
constexpr float LEVEL_SLOW_STEP = 0.0035f, LEVEL_TOP_STEP = 0.01f;
constexpr float CHASE_MIN = 3, CHASE_MAX = 40;
constexpr uint32_t RUNAWAY_LED = 3, RUNAWAY_OF = 5;   // a runaway: the kid led for 3/5 of the race
constexpr uint32_t RIVAL_BOOST_GAP_MS = 2500;
// The fixed difficulty settings race at one level and never move the adaptive ones: Easy slower, courteous rivals;
// Normal level 0 (the brief's rivals, though on Industrial Ruins and Night Run level 0 sits up the ladder, so their
// rivals there carry boosts); Hard, by track, a level a competent driver wins 33-53% of (the sweep's closest to
// 35-50%: Snow Track goes 53% at 1, 25% at 2).
constexpr int EASY_LEVEL = -8;
constexpr int8_t HARD_LEVEL[] = {4, 6, 1, 1};
// By track: the rivals' pace at LEVEL_MIN, kept above what a kid who never steers manages there (Desert Canyon's
// straights let one finish within 18% of a clean drive).
constexpr float LEVEL_FLOOR[] = {0.87f, 0.80f, 0.80f, 0.80f};
// For level < 0 only: LEVEL_SLOW_STEP a level near 0 (gentle where small steps matter), bending down to the track's floor.
constexpr float slowdown(int level, float floor) {
  return 1 + LEVEL_SLOW_STEP * level - (1 + LEVEL_SLOW_STEP * LEVEL_MIN - floor) * level * level / (LEVEL_MIN * LEVEL_MIN);
}
}  // namespace racing
