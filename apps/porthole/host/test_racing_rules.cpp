// Racing's pure rules: every track finishes for any driver, places are stable and locked once, and the speed,
// inventory, jump and rocket rules from the design brief hold at their boundaries.
#include <assert.h>
#include <math.h>
#include <stdio.h>
#include <string.h>
#include <algorithm>
#include "games/racing/race.h"

using namespace racing;

namespace {
// A driver who holds the middle of the road against the bend: what a kid steering by eye does.
float follow(const Race& r, float aim) {
  const Car& p = r.car[PLAYER];
  const float feed = curve(*r.track, p.dist) * DRIFT * p.speed * p.speed / STEER_RATE;
  const float s = (aim - p.x) * 2.5f + feed;
  return s < -1 ? -1 : s > 1 ? 1 : s;
}
struct Result { uint32_t ms; int place; bool finished; uint32_t lapMs[LAPS]; };
Result run(int track, int style, uint32_t seed, int level = 0) {   // style 0 idle, 1 steady, 2 steady + every action
  Race r;
  start(r, TRACK[track], seed, level);
  Result out{};
  int lap = 1;
  for (int n = 0; n < 60 * 60 * 50 && r.phase != Phase::Finished; n++) {
    Controls c;
    if (style) c.steer = follow(r, 0.1f * sinf(n / 200.0f));
    if (style == 2) { c.boost = n % 97 == 0; c.rocket = n % 61 == 0; }
    step(r, c);
    if (lapOf(r, PLAYER) != lap) { out.lapMs[lap - 1] = r.raceMs; lap = lapOf(r, PLAYER); }
    assert(r.inventory[BOOST] <= MAX_INVENTORY && r.inventory[ROCKET] <= MAX_INVENTORY);
    assert(r.car[PLAYER].speed >= 0 && r.car[PLAYER].speed <= BOOST_CAP + 1e-4f);
    assert(fabsf(r.car[PLAYER].x) <= RAIL + 1e-4f);
  }
  out.ms = r.car[PLAYER].finishMs; out.place = r.place; out.finished = r.phase == Phase::Finished;
  // Lap times add up to the finish, less the run from the grid to the line, and the best is one of them.
  const uint32_t sum = r.lapMs[0] + r.lapMs[1] + r.lapMs[2];
  assert(r.lapMs[0] && r.lapMs[1] && r.lapMs[2] && sum < r.car[PLAYER].finishMs && r.car[PLAYER].finishMs - sum < 5000);
  assert(bestLap(r) == std::min(r.lapMs[0], std::min(r.lapMs[1], r.lapMs[2])));
  return out;
}

void everyTrackFinishes() {
  for (int t = 0; t < TRACKS; t++) {
    const Result idle = run(t, 0, 7), steady = run(t, 1, 7), busy = run(t, 2, 7);
    printf("%-16s idle %5.1fs %d | steady %5.1fs %d (lap1 %4.1fs) | actions %5.1fs %d\n", TRACK[t].name,
           idle.ms / 1000.0, idle.place, steady.ms / 1000.0, steady.place, steady.lapMs[0] / 1000.0,
           busy.ms / 1000.0, busy.place);
    assert(idle.finished && steady.finished && busy.finished);   // even hands off, the player finishes
    assert(idle.place == 4);                                     // ... and fourth is a finish, not a failure
    assert(steady.ms < idle.ms);
    // Even the rivals at their slowest and most courteous beat a kid who never steers; the same seed and level is the
    // same race, to the millisecond.
    for (int level : {LEVEL_MIN, LEVEL_MAX}) assert(run(t, 0, 7, level).place == 4);
    assert(run(t, 2, 11, LEVEL_MIN).ms == run(t, 2, 11, LEVEL_MIN).ms);
  }
}

// The level after a finished race: a win steps up one (two after a runaway), a loss at 0 or below steps down two,
// from 1..LEVEL_GENTLE lands at 0, from LEVEL_GENTLE+1..LEVEL_EASY at -2 (behind, those levels race alike and below 0
// the rivals are slower), above that steps down LEVEL_HIGH_LOSS; always inside LEVEL_MIN..LEVEL_MAX. A level out of range is clamped when the race starts.
void levels() {
  assert(nextLevel(-3, 1, false) == -2 && nextLevel(-3, 1, true) == -1 && nextLevel(-3, 2, false) == -5);
  assert(nextLevel(1, 2, false) == 0 && nextLevel(LEVEL_GENTLE, 4, false) == 0);
  assert(nextLevel(LEVEL_GENTLE + 1, 2, false) == -2 && nextLevel(LEVEL_EASY, 4, false) == -2);
  assert(nextLevel(LEVEL_MAX, 3, false) == LEVEL_MAX - LEVEL_HIGH_LOSS && nextLevel(LEVEL_EASY + 1, 2, false) > 0);
  assert(nextLevel(LEVEL_MAX, 1, true) == LEVEL_MAX && nextLevel(LEVEL_MIN + 1, 3, false) == LEVEL_MIN);
}
void ladder() {
  Race r;
  r.raceMs = 100; r.firstMs = 60;
  assert(runaway(r));   // led exactly 60% of the race: a runaway, even with a rival close behind at the line
  r.firstMs = 59;
  assert(!runaway(r));
  start(r, TRACK[1], 1, 0);   // Industrial Ruins sits LEVEL_SHIFT rungs up from 0, the shift gone by -LEVEL_SHIFT_FADE
  assert(r.rung == LEVEL_SHIFT[1] && r.car[3].boosts == MAX_INVENTORY);
  start(r, TRACK[1], 1, -2);   // below 0 the rivals are slower on every track, shifted or not, still with charges
  assert(r.rung > 0 && r.car[3].top < 1 && r.car[1].boosts > 0);
  start(r, TRACK[1], 1, -LEVEL_SHIFT_FADE);
  assert(r.rung == -LEVEL_SHIFT_FADE && !r.car[1].boosts);
  start(r, TRACK[1], 1, LEVEL_MAX);   // a level past the ladder's last rung is that rung
  assert(r.level == LEVEL_MAX - LEVEL_SHIFT[1] && r.rung == LEVEL_MAX);
  start(r, TRACK[0], 1, LEVEL_MIN - 5);
  assert(r.level == LEVEL_MIN && r.car[PLAYER].top == 1 && r.car[3].top < 1);   // slower rivals, never the kid
  start(r, TRACK[0], 1, 0);
  assert(r.car[1].top == 0.92f && r.car[2].top == 0.97f && r.car[3].top == 1);   // level 0: the brief's pace
  assert(!r.car[1].boosts && !r.car[2].boosts && !r.car[3].boosts);
  start(r, TRACK[0], 1, LEVEL_MAX);   // the top: three charges each, like the kid's cap, and nobody faster than the kid
  for (int i = 1; i < CARS; i++) assert(r.car[i].boosts == MAX_INVENTORY && r.car[i].top < 1 + 1e-6f);
  assert(r.car[1].top < 1 && r.car[2].top < 1);
}

void countdownFreezes() {
  Race r;
  start(r, TRACK[0], 3, 0);
  const float d = r.car[PLAYER].dist;
  Controls c; c.boost = c.rocket = true; c.steer = 1;
  for (uint32_t t = 0; t + STEP_MS < COUNTDOWN_MS; t += STEP_MS) step(r, c);
  assert(r.phase == Phase::Countdown && r.car[PLAYER].dist == d && r.car[PLAYER].x == 0.35f);
  assert(r.inventory[BOOST] == 1 && r.inventory[ROCKET] == 1 && r.rocketsFired == 0);
  step(r, Controls{});
  assert(r.phase == Phase::Racing && lapOf(r, PLAYER) == 1 && r.car[PLAYER].dist < 0);   // the grid is not a lap
}

void impactRule() {
  assert(hitSpeed(1.0f, CONTACT_LOSS) == 0.65f);
  assert(hitSpeed(0.10f, OBSTACLE_LOSS) == 0.10f);   // never lifts a slow car to the floor
  assert(hitSpeed(0.40f, OBSTACLE_LOSS) == HIT_FLOOR);
  bool ice = false;
  const Track& t = TRACK[0];
  assert(terrainCap(t, 0, 1.2f, &ice) == OFFROAD_CAP);
  const Feature& mud = t.features[1];
  const float inMud = featureAt(t, mud) + 2;
  assert(terrainCap(t, inMud, mud.x / 100.0f, &ice) == SLOW_CAP && !ice);
  assert(terrainCap(t, inMud, -1.3f, &ice) == OFFROAD_CAP);   // strongest cap, not a product
  const Feature& patch = TRACK[2].features[1];
  ice = false;
  terrainCap(TRACK[2], featureAt(TRACK[2], patch) + 2, patch.x / 100.0f, &ice);
  assert(ice);
}

Race racingAt(int track, float dist, float x, int level = 0) {
  Race r;
  start(r, TRACK[track], 5, level);
  r.phase = Phase::Racing;
  for (Car& c : r.car) { c.dist = -500; c.speed = 0; }   // rivals out of the way
  r.car[PLAYER].dist = dist; r.car[PLAYER].x = x; r.car[PLAYER].speed = 1;
  return r;
}

void boostAndTerrain() {
  Race r = racingAt(0, 100, 0);
  Controls press; press.boost = true;
  step(r, press);
  assert(r.inventory[BOOST] == 0 && r.car[PLAYER].boostMs > 0);
  step(r, press);   // an empty or active boost spends nothing
  assert(r.inventory[BOOST] == 0);
  for (int i = 0; i < 40; i++) step(r, Controls{});
  assert(r.car[PLAYER].speed > 1.2f);
  r.inventory[BOOST] = 1;
  const uint32_t left = r.car[PLAYER].boostMs;
  step(r, press);   // pressing during a boost neither spends nor queues
  assert(r.inventory[BOOST] == 1 && r.car[PLAYER].boostMs < left);
  // Boost is no immunity to terrain: off the road the lower cap wins.
  r.car[PLAYER].x = 1.25f;
  for (int i = 0; i < 40; i++) step(r, Controls{});
  assert(r.car[PLAYER].speed <= OFFROAD_CAP + 0.01f);
}

void obstacleOnce() {
  const Track& t = TRACK[0];
  const Feature& rock = t.features[5];
  assert(rock.kind == Kind::Obstacle);
  Race r = racingAt(0, featureAt(t, rock) - 10, rock.x / 100.0f);
  int hits = 0;
  for (int i = 0; i < 100; i++) {
    const uint32_t before = r.impacts;
    step(r, Controls{});
    hits += r.impacts != before;
  }
  assert(hits == 1);   // one loss per contact, then eased out of the rock rather than trapped
  assert(fabsf(r.car[PLAYER].x - rock.x / 100.0f) >= rock.w / 100.0f + CAR_HALF_W - 0.05f);
}

void jumpsAndPickups() {
  const Track& t = TRACK[0];
  const Feature& ramp = t.features[3];
  assert(ramp.kind == Kind::Ramp && t.features[4].raised);
  Race r = racingAt(0, featureAt(t, ramp) - 5, ramp.x / 100.0f);
  r.inventory[BOOST] = 0;
  int air = 0;
  for (int i = 0; i < 60; i++) { step(r, Controls{}); air += airborne(r.car[PLAYER]); }
  assert(r.jumps == 1 && air == (int)(AIR_MS / STEP_MS));   // one fixed hop
  assert(r.inventory[BOOST] == 1);                           // the raised pickup is reached only in the air
  // The flat bypass: no jump, and the raised pickup stays out of reach.
  Race b = racingAt(0, featureAt(t, ramp) - 5, 0.5f);
  b.inventory[BOOST] = 0;
  for (int i = 0; i < 60; i++) step(b, Controls{});
  assert(b.jumps == 0 && b.inventory[BOOST] == 0);
  // A full inventory leaves the gate for a later lap and says so.
  const Feature& g = t.features[0];
  Race f = racingAt(0, featureAt(t, g) - 3, g.x / 100.0f);
  f.inventory[BOOST] = MAX_INVENTORY;
  for (int i = 0; i < 20; i++) step(f, Controls{});
  assert(f.inventory[BOOST] == MAX_INVENTORY && f.fullMs[BOOST] > 0 && f.pickups == 0);
  // At boost speed a gate is still collected exactly once.
  Race q = racingAt(0, featureAt(t, g) - 3, g.x / 100.0f);
  q.inventory[BOOST] = 0; q.car[PLAYER].speed = BOOST_CAP; q.car[PLAYER].boostMs = BOOST_MS;
  for (int i = 0; i < 20; i++) step(q, Controls{});
  assert(q.inventory[BOOST] == 1 && q.pickups == 1);
}

void rockets() {
  Race r = racingAt(0, 100, 0);
  Car& target = r.car[2];   // the rival whose lane is the middle
  target.dist = 140; target.x = 0; target.speed = 1;
  Controls fire; fire.rocket = true;
  step(r, fire);
  step(r, fire);   // cooldown: the second press spends nothing
  assert(r.rocketsFired == 1 && r.inventory[ROCKET] == 0);
  float slowest = 1;
  for (int i = 0; i < 60; i++) { step(r, Controls{}); slowest = fminf(slowest, target.speed); }
  assert(r.rocketHits == 1 && slowest < 0.7f && !target.finished);
  for (int i = 0; i < 200; i++) step(r, Controls{});
  assert(target.speed > 0.9f);   // recovers by itself: no destruction
  // A miss despawns at the end of its life; an empty rack fires nothing.
  Race m = racingAt(0, 100, 0.8f);
  step(m, fire);
  for (int i = 0; i < 80; i++) step(m, Controls{});
  assert(m.rocketsFired == 1 && m.rocketHits == 0 && !m.rocket[0].live);
  step(m, fire);
  assert(m.rocketsFired == 1);
}

void contactOnce() {
  Race r = racingAt(0, 100, 0);
  Car& o = r.car[2];
  o.dist = 104; o.x = 0.05f; o.speed = 0.6f;
  int hits = 0, last = -100;
  for (int i = 0; i < 60; i++) {
    const uint32_t b = r.impacts;
    step(r, Controls{});
    if (r.impacts == b) continue;
    assert(i - last >= (int)(PROTECT_MS / STEP_MS));   // never a loss every frame while still touching
    hits++; last = i;
  }
  assert(hits >= 1);
}

void finishOrder() {
  Race r;
  start(r, TRACK[0], 9, 0);
  r.phase = Phase::Racing;
  const float line = LAPS * TRACK[0].length;
  r.car[1].dist = line - 1; r.car[2].dist = line - 1; r.car[3].dist = line - 100;
  r.car[1].speed = r.car[2].speed = r.car[1].top = r.car[2].top = 1; r.car[1].x = -0.6f; r.car[2].x = 0.6f;
  for (int i = 0; i < 5; i++) step(r, Controls{});
  assert(r.car[1].finished && r.car[2].finished && r.finishedCount == 2);
  assert(placeOf(r, 1) == 1 && placeOf(r, 2) == 2);   // same crossing time: grid order, no flicker
  r.car[PLAYER].dist = line - 0.5f; r.car[PLAYER].speed = 1; r.car[PLAYER].x = 0;
  step(r, Controls{});
  assert(r.phase == Phase::Finished && r.place == 3);
  const uint32_t ms = r.car[PLAYER].finishMs;
  r.car[3].dist = line + 50;   // anything after the line changes nothing
  step(r, Controls{});
  assert(r.place == 3 && r.car[PLAYER].finishMs == ms);
}

// A rival spends a Boost only while chasing, the kid 3-40 m ahead of it: never far behind, never in front, never
// twice at once, and with the kid's own cap and duration.
void rivalBoosts() {
  Race r = racingAt(0, 100, 0, LEVEL_MAX);
  Car& c = r.car[3];
  c.dist = 100 - 45; c.x = 0.6f; c.speed = 1;   // the kid 45 m ahead: too far
  step(r, Controls{});
  assert(c.boosts == MAX_INVENTORY && !c.boostMs);
  c.dist = r.car[PLAYER].dist + 10;   // ahead of the kid
  step(r, Controls{});
  assert(c.boosts == MAX_INVENTORY && !c.boostMs);
  c.dist = r.car[PLAYER].dist - 2;   // alongside
  step(r, Controls{});
  assert(c.boosts == MAX_INVENTORY && !c.boostMs);
  c.dist = r.car[PLAYER].dist - 20;   // chasing: one charge, the kid's boost
  step(r, Controls{});
  assert(c.boosts == MAX_INVENTORY - 1 && c.boostMs == BOOST_MS);
  for (int i = 0; i < 40; i++) { c.dist = r.car[PLAYER].dist - 20; step(r, Controls{}); }
  assert(c.boosts == MAX_INVENTORY - 1 && c.speed <= BOOST_CAP + 1e-4f);   // none spent during a boost
  Car& d = r.car[2];   // a second chaser waits out RIVAL_BOOST_GAP_MS after the first boost: they never gang up
  d.x = -0.6f; d.speed = 1;
  int since = 40;   // steps since the first rival's boost
  while (!d.boostMs && since < 500) { d.dist = r.car[PLAYER].dist - 25; step(r, Controls{}); since++; }
  assert(since == (int)(RIVAL_BOOST_GAP_MS / STEP_MS));
  // The refill levels, one rival a level in LADDER_REFILL order; such a rival takes its charges back at the lap line.
  for (int level = LEVEL_MAX - 3; level <= LEVEL_MAX; level++) {
    Race q = racingAt(0, 100, 0, level);
    for (int k = 0; k < 3; k++) assert(q.car[LADDER_REFILL[k]].refill == (level > LEVEL_MAX - 3 + k));
    Car& e = q.car[LADDER_REFILL[0]];
    e.boosts = 0; e.dist = TRACK[0].length - 2; e.speed = 1;   // ahead of the kid: nothing to chase
    for (int i = 0; i < 10; i++) step(q, Controls{});
    assert(e.boosts == (level > LEVEL_MAX - 3 ? MAX_INVENTORY : 0));
  }
}

// A competent driver: steers its favourite line around cars and hazards ahead, presses Boost after a seeded 0.2-1.2 s
// reaction whenever it holds a charge, and fires when a rival is lined up. Line and reaction vary driver to driver.
float aimAround(const Race& r, float aim) {
  const Car& p = r.car[PLAYER];
  for (int i = 1; i < CARS; i++) {
    const Car& o = r.car[i];
    const float d = o.dist - p.dist;
    if (!o.finished && d > -2 && d < 24 && fabsf(o.x - aim) < 0.45f) aim = o.x > 0 ? o.x - 0.5f : o.x + 0.5f;
  }
  const Track& t = *r.track;
  for (int f = 0; f < t.nFeatures; f++) {
    const Feature& o = t.features[f];
    const float d = featureAt(t, o) - lapPos(t, p.dist), ox = o.x / 100.0f, room = o.w / 100.0f + 0.25f;
    if (o.kind != Kind::Pickup && o.kind != Kind::Ramp && d > 0 && d < 40 && fabsf(aim - ox) < room) aim = ox > 0 ? ox - room : ox + room;
  }
  return std::max(-0.85f, std::min(0.85f, aim));
}
bool linedUp(const Race& r) {
  for (int i = 1; i < CARS; i++) {
    const float d = r.car[i].dist - r.car[PLAYER].dist;
    if (d > 5 && d < 50 && fabsf(r.car[i].x - r.car[PLAYER].x) < 0.15f) return true;
  }
  return false;
}
uint32_t next(uint32_t& s) { s = s * 1664525u + 1013904223u; return s >> 8; }
int competent(int track, uint32_t seed, int level, bool* ran = nullptr, int* raced = nullptr) {
  Race r;
  start(r, TRACK[track], seed, level);
  uint32_t s = seed * 2654435761u + 7;
  int wait = -1;
  const float lane = ((int)(next(s) % 61) - 30) / 100.0f;   // each driver's favourite line
  for (int n = 0; n < 60 * 60 * 50 && r.phase != Phase::Finished; n++) {
    Controls c;
    c.steer = follow(r, aimAround(r, lane));
    if (r.phase == Phase::Racing && r.inventory[BOOST] && !r.car[PLAYER].boostMs) {
      if (wait < 0) wait = (200 + (int)(next(s) % 1000)) / (int)STEP_MS;
      if (wait-- == 0) { c.boost = true; wait = -1; }
    }
    c.rocket = linedUp(r);
    step(r, c);
  }
  assert(r.phase == Phase::Finished);
  if (ran) *ran = runaway(r);
  if (raced) *raced = r.level;   // the level the race clamped it to, as the game stores
  return r.place;
}

// The rule the levels exist for: a competent driver climbing from level 0 keeps winning some races and losing others
// on every track once the level has settled (the last 40 of 80 races: 45-80% wins, about two in three).
void staircase() {
  for (int t = 0; t < TRACKS; t++) {
    int level = 0, wins = 0, settled = 0;
    for (int k = 0; k < 80; k++) {
      bool ran = false;
      int raced = level;
      const int place = competent(t, 5000 + k * 104729u, level, &ran, &raced);
      wins += place == 1;
      settled += k >= 40 && place == 1;
      level = clampLevel(nextLevel(raced, place, ran), t);
    }
    printf("%-16s competent driver from level 0: %d/80 wins, %d of the last 40, level %+d\n", TRACK[t].name, wins, settled, level);
    assert(settled >= 18 && settled <= 32);
  }
}
// The fixed settings stay what the chip promises on every track: Normal and Hard are races a competent driver
// both wins and loses, and Hard is never easier than Normal.
void modeBands() {
  for (int t = 0; t < TRACKS; t++) {
    int normal = 0, hard = 0;
    for (int k = 0; k < 40; k++) {
      normal += competent(t, 9000 + k * 7919u, 0) == 1;
      hard += competent(t, 9000 + k * 7919u, HARD_LEVEL[t]) == 1;
    }
    printf("%-16s competent driver: Normal %d/40, Hard %d/40\n", TRACK[t].name, normal, hard);
    assert(normal >= 8 && normal <= 32 && hard >= 8 && hard <= 26 && hard <= normal);
  }
}
}  // namespace

int main() {
  countdownFreezes();
  impactRule();
  boostAndTerrain();
  obstacleOnce();
  jumpsAndPickups();
  rockets();
  contactOnce();
  finishOrder();
  levels();
  ladder();
  rivalBoosts();
  everyTrackFinishes();
  staircase();
  modeBands();
  puts("test_racing_rules: ok");
  return 0;
}
