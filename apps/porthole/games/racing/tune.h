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
// An obstacle hits only while drawn level with a car: its middle between the car's dist (the picture's bottom row)
// and CAR_REACH m ahead (its top row; render.h carReach), and closer sideways than both drawn half-widths
// (render.cpp draws an obstacle exactly 2 * w wide). Two cars touch the same way: their dists less than CAR_REACH
// apart (one's picture overlaps the other's rows) and closer than both half-widths. The wider windows (CAR_LEN) hit
// a hazard still drawn a car length away and bumped a rival with road visible between the cars.
constexpr float CAR_REACH = 2.85f, OBSTACLE_PUSH = 4;   // OBSTACLE_PUSH: half-widths a second out of one
constexpr float PICKUP_HALF_W = 0.26f;
constexpr int GRID_SPACING = 7;                    // m between grid slots
// The garage's Power parts (a user decision of 2026-11-21): five parts, bought in order up to three tiers, each tier +1
// Power. Only the kid's own car changes, never boost strength (BOOST_CAP), rocket damage or the post-hit floor, so
// rivals that boost with the same 1.35 stay a match; a race together runs stock. By tier, stock first. Part numbers are
// persisted (save.h tiers): never renumbered, only appended. A full kit (Power 15) gives a competent driver at most
// 1% a race, and both a competent and a kid driver win Hard at most 24/40, the kid clearly more than stock
// (host/test_racing_rules.cpp fairKit). The brief's figures (Engine 1.01-1.03, Turbo 2.2-1.6 s, Nitro 1800-2400 ms) won
// every Normal and Hard race: rivals never pass the kid's top, so 0.35% of pace decides a race (LEVEL_SLOW_STEP below).
// KIT_SCALE is the largest one factor on those three steps that keeps both drivers within 24/40 on Hard on every track
// (a scan of 0-100% on 2026-11-21: 4.5% gives Industrial Ruins 28/40); Tires and Suspension, which barely moved a
// race, keep the brief's figures.
enum Part : uint8_t { ENGINE, TURBO, TIRES, SUSPENSION, NITRO, PARTS };
constexpr int TIERS = 3;
constexpr float KIT_SCALE = 0.0425f;   // the brief's Engine, Turbo and Nitro steps, scaled alike on every tier
constexpr float KIT_TOP[] = {1, 1 + 0.01f * KIT_SCALE, 1 + 0.02f * KIT_SCALE, 1 + 0.03f * KIT_SCALE};   // Engine: dry top
constexpr float KIT_ACCEL_S[] = {2.5f, 2.5f - 0.3f * KIT_SCALE, 2.5f - 0.6f * KIT_SCALE, 2.5f - 0.9f * KIT_SCALE};   // Turbo
constexpr float KIT_STEER[] = {1.5f, 1.6f, 1.7f, 1.8f};               // Tires: STEER_RATE
constexpr float KIT_ICE_TAU[] = {0.25f, 0.20f, 0.15f, 0.11f};         // Tires: ICE_TAU
constexpr float KIT_ICE_RESPONSE[] = {0.75f, 0.80f, 0.85f, 0.90f};    // Tires: ICE_RESPONSE
constexpr float KIT_OFFROAD[] = {0.55f, 0.61f, 0.67f, 0.72f};         // Suspension: OFFROAD_CAP
constexpr float KIT_SLOW[] = {0.65f, 0.70f, 0.74f, 0.78f};            // Suspension: SLOW_CAP
constexpr uint32_t KIT_BOOST_MS[] = {1500, 1500 + uint32_t(300 * KIT_SCALE + 0.5f), 1500 + uint32_t(600 * KIT_SCALE + 0.5f),
                                     1500 + uint32_t(900 * KIT_SCALE + 0.5f)};   // Nitro tank: BOOST_MS
// Adaptive rivals (a user decision of 2026-10-31): each kid has a level per track (race.h nextLevel). A win adds one,
// two after a runaway (the kid led for RUNAWAY_LED/RUNAWAY_OF of the race). A loss at 0 or below takes two; from
// 1..LEVEL_GENTLE it drops to 0; above that, while the rung is at most LEVEL_EASY, straight to -2 (there the rivals
// only boost to pass a kid who leads, so a kid stuck behind would get the same race again; at -2 they are slower);
// from a higher rung it takes LEVEL_HIGH_LOSS (keyed on the rung). From each track's top a losing streak goes 14, 11,
// 8, 5, 2, 0 on Desert Canyon; 15, 12, 9, 6, 3, 0 on Industrial Ruins; 19, 16, 13, 10, 7, -2 on Snow Track; 16, 13,
// 10, 7, 4, -2 on Night Run. A competent driver settles at 55-70% wins (the staircase's last 40 races: Desert Canyon 55%,
// Industrial Ruins 60%, Night Run 65%, Snow Track 70%).
// Below 0, on every track, the rivals' top speed drops, gently first and faster deeper down (slowdown), and from
// LEVEL_COURTESY on a rival with the kid close behind moves out of its lane. Separately a level is a rung of a
// charge-and-pace ladder: rungs 1-9 each give one rival one more Boost charge (the slower two first, the pace-setter
// last, three at most, like the kid's), fired only while chasing (the kid CHASE_MIN..CHASE_MAX m ahead) and never
// within RIVAL_BOOST_GAP_MS of another rival's boost; rungs 10-15 raise a slower rival's top by LEVEL_TOP_STEP;
// rungs 16-18 let a rival take its three charges back each lap, as the kid does from road pickups; 19-21 are Pro
// (below). The rung is the level plus the track's LEVEL_SHIFT; a rung at or below 0 gives nothing. Ordered by a
// simulated sweep so win rates fall with level, roughly (neighbours differ by at most 25 points but at two cliffs of
// 42-45: where the slow-down starts, -1 to 0, since even 0.35% off the pace-setter's top hands a competent driver the
// race, and rung 7, the pace-setter's first charge: `neighbourLevels`): raising rival pace first made races easier (less
// traffic to hit). No rival's dry-road top exceeds the kid's, and the slower two stay below 1.0: rival 2 level with
// the pace-setter forms a two-abreast wall.
constexpr uint8_t LADDER_CHARGE[] = {1, 2, 1, 2, 1, 2, 3, 3, 3};   // the rival given a charge at levels 1..9
constexpr uint8_t LADDER_TOP[] = {2, 2, 1, 1, 1, 1};               // the rival a step faster at levels 10..15
constexpr uint8_t LADDER_REFILL[] = {3, 1, 2};                     // the rival recharged each lap from 16..18
constexpr int LADDER_RUNGS = sizeof LADDER_CHARGE + sizeof LADDER_TOP + sizeof LADDER_REFILL;   // 18
// Pro (a user request of 2026-10-31: tracks get harder as a kid masters them): rungs 19-21, each track's top three
// levels, leave the rivals as at 18 and can each switch on one more of the track's Pro hazards (track.cpp,
// Feature::rung; kept only where a sweep found the race no easier, host/test_racing_rules.cpp proNoEasier).
constexpr int LEVEL_PRO = 3;
constexpr int LEVEL_MIN = -16, LEVEL_MAX = LADDER_RUNGS + LEVEL_PRO;
constexpr int LEVEL_COURTESY = -7, LEVEL_SEED_MAX = 3;   // a never-raced track starts one below the best, in 0..3
constexpr int LEVEL_GENTLE = 3, LEVEL_EASY = sizeof LADDER_CHARGE, LEVEL_HIGH_LOSS = 3;
// By track: how many rungs up the charge-and-pace ladder every level sits, one rung a level (race.cpp rungOf; the
// slow-down below 0 is keyed on the level): each track's base strength, so level 0 (Normal) is a race a competent
// driver wins 45-73% of everywhere (since cars touch only where drawn and rivals keep clear of a whole patch,
// 2026-10-31). A track's top is LEVEL_MAX less its shift: 14, 15, 19, 16.
constexpr int8_t LEVEL_SHIFT[] = {7, 6, 2, 5};
constexpr float LEVEL_SLOW_STEP = 0.0035f, LEVEL_TOP_STEP = 0.01f;
constexpr float CHASE_MIN = 3, CHASE_MAX = 40;
constexpr uint32_t RUNAWAY_LED = 3, RUNAWAY_OF = 5;   // a runaway: the kid led for 3/5 of the race
constexpr uint32_t RIVAL_BOOST_GAP_MS = 2500;
// Every race the three colors are shuffled over the rivals' fixed parts (car 1 slowest .. 3 pace-setter), so the
// shuffle never moves the balance, and each rival gets its day inside these bands (the racing brief has the whole rule).
constexpr float RIVAL_TOP[] = {0.92f, 0.97f, 1.0f};   // by part: the brief's pace
constexpr float RIVAL_CAP[] = {0.99f, 0.99f, 1.0f};   // by part: never past the kid, the slower two below 1, ladder too
// By part: a rival's pace on the day, its part's +- this, then capped. The pace-setter keeps the kid's pace: since
// nothing passes the kid's top, its form could only ever slow it, and that made Normal easier on every track.
constexpr float RIVAL_FORM[] = {0.035f, 0.035f, 0};
constexpr float RIVAL_CUT_MIN = 0.2f, RIVAL_CUT_MAX = 0.5f;   // how much of the road it cuts toward a bend's inside
constexpr int RIVAL_WOBBLE_MIN = 2, RIVAL_WOBBLE_MAX = 6;     // a line error: one new line in this many
constexpr uint8_t RIVAL_CAREFUL[] = {0, 3, 3};        // by part: steers round slow patches, this many in RIVAL_CAREFUL_OF
constexpr uint32_t RIVAL_CAREFUL_OF = 4;
// The fixed difficulty settings race at one level and never move the adaptive ones: Easy slower, courteous rivals;
// Normal level 0 on every track (its strength set by the track's LEVEL_SHIFT); Hard, by track, a level both the
// competent and the kid driver win less often than Normal: the competent one 33-43% (re-swept with the rivals' varied form, 2026-11-01).
constexpr int EASY_LEVEL = -8;
constexpr int8_t HARD_LEVEL[] = {3, 1, 2, 4};
// By track: the rivals' pace at LEVEL_MIN, kept above what a kid who never steers manages there (Desert Canyon's
// straights let one finish within 18% of a clean drive).
constexpr float LEVEL_FLOOR[] = {0.87f, 0.80f, 0.80f, 0.80f};
// For level < 0 only: LEVEL_SLOW_STEP a level near 0 (gentle where small steps matter), bending down to the track's floor.
constexpr float slowdown(int level, float floor) {
  return 1 + LEVEL_SLOW_STEP * level - (1 + LEVEL_SLOW_STEP * LEVEL_MIN - floor) * level * level / (LEVEL_MIN * LEVEL_MIN);
}
}  // namespace racing
