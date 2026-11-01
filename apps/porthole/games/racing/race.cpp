#include "race.h"
#include <math.h>

namespace racing {
static_assert(sizeof LEVEL_SHIFT == TRACKS && sizeof LEVEL_FLOOR / sizeof LEVEL_FLOOR[0] == TRACKS, "one per track");
namespace {
float clampf(float v, float lo, float hi) { return v < lo ? lo : v > hi ? hi : v; }
uint32_t down(uint32_t v) { return v > STEP_MS ? v - STEP_MS : 0; }
uint32_t rnd(uint32_t& s) { s = s * 1664525u + 1013904223u; return s >> 8; }
// Did a car moving from prev to cur pass the gate at `at` (meters into a lap), on any lap? Swept, so a long step
// (or a boost) can never hop over a gate, ramp or line.
bool crossed(const Track& t, float prev, float cur, float at) {
  return floorf((cur - at) / t.length) > floorf((prev - at) / t.length);
}
bool within(const Track& t, float dist, float at, float len, float margin) {
  const float z = lapPos(t, dist);
  return z + margin >= at && z - margin <= at + len;
}
void puff(Race& r, float dist, float x) {
  int oldest = 0;
  for (int i = 1; i < MAX_PUFFS; i++) if (r.puff[i].ms < r.puff[oldest].ms) oldest = i;
  r.puff[oldest] = {dist, x, 400};
}
void impact(Race& r, int i, float factor) {
  Car& c = r.car[i];
  if (c.protectMs) return;   // protection only blocks a new instantaneous loss
  c.speed = hitSpeed(c.speed, factor);
  c.protectMs = PROTECT_MS;
  c.wobbleMs = 500;
  if (i == PLAYER) r.impacts++;
  puff(r, c.dist, c.x);
}
float capFor(const Race& r, const Car& c, bool* ice) {
  float cap = c.boostMs ? BOOST_CAP : c.top;
  *ice = false;
  if (!airborne(c)) {
    const float terrain = terrainCap(*r.track, r.rung, c.dist, c.x, ice);
    if (terrain < 1 && terrain < cap) cap = terrain;   // terrain wins over Boost; never multiplied
  }
  return cap;
}
void throttle(Car& c, float cap) {
  const float accel = ACCEL * (c.boostMs ? 2 : 1) * DT;
  if (c.speed < cap) c.speed = c.speed + accel > cap ? cap : c.speed + accel;
  else c.speed = c.speed - DECEL * DT < cap ? cap : c.speed - DECEL * DT;
}
void slide(const Race& r, Car& c, float steer, bool ice) {
  const float bend = curve(*r.track, c.dist);
  float response = STEER_RATE * (0.4f + 0.6f * fminf(c.speed, 1));
  if (ice) response *= ICE_RESPONSE;
  if (airborne(c)) response *= 0.5f;
  const float target = clampf(steer, -1, 1) * response - bend * DRIFT * c.speed * c.speed;
  const float tau = ice ? ICE_TAU : GRIP_TAU;
  c.vx += (target - c.vx) * fminf(1, DT / tau);
  c.x += c.vx * DT;
  if (fabsf(c.x) > RAIL) { c.x = copysignf(RAIL, c.x); c.vx = 0; }
}
float drawnAhead(const Track& t, const Feature& o, float dist) {   // m from a car to where an obstacle is drawn, wrapped
  const float ahead = featureAt(t, o) + featureLen(t, o) / 2 - lapPos(t, dist);
  return ahead < -t.length / 2 ? ahead + t.length : ahead > t.length / 2 ? ahead - t.length : ahead;
}
// Solid obstacles: one speed loss on entry; while overlapping, the car is eased sideways to the nearer free road.
void obstacles(Race& r, int i) {
  Car& c = r.car[i];
  const Track& t = *r.track;
  for (int f = 0; f < t.nFeatures; f++) {
    const Feature& o = t.features[f];
    if (o.kind != Kind::Obstacle || !active(o, r.rung)) continue;
    const float ox = o.x / 100.0f, ow = o.w / 100.0f + CAR_HALF_W;
    const uint16_t bit = (uint16_t)(1u << f);
    const float ahead = drawnAhead(t, o, c.dist);
    const bool inside = !airborne(c) && ahead >= 0 && ahead <= CAR_REACH && fabsf(c.x - ox) < ow;
    if (inside && !(c.solid & bit)) impact(r, i, OBSTACLE_LOSS);
    if (inside) {
      const bool right = ox - ow < -RAIL + 0.1f || (c.x >= ox && ox + ow < RAIL);
      c.x += (right ? OBSTACLE_PUSH : -OBSTACLE_PUSH) * DT;
      c.vx = 0;
    }
    c.solid = inside ? (uint16_t)(c.solid | bit) : (uint16_t)(c.solid & ~bit);
  }
}
void ramps(Race& r, int i, float prev) {
  Car& c = r.car[i];
  const Track& t = *r.track;
  for (int f = 0; f < t.nFeatures; f++) {
    const Feature& o = t.features[f];
    if (o.kind != Kind::Ramp || !active(o, r.rung) || airborne(c) || !crossed(t, prev, c.dist, featureAt(t, o))) continue;
    if (fabsf(c.x - o.x / 100.0f) > o.w / 100.0f) continue;
    c.airMs = AIR_MS;
    if (i == PLAYER) r.jumps++;
  }
}
void pickups(Race& r, float prev) {
  Car& c = r.car[PLAYER];
  const Track& t = *r.track;
  for (int f = 0; f < t.nFeatures; f++) {
    const Feature& o = t.features[f];
    if (o.kind != Kind::Pickup || !active(o, r.rung) || o.raised != airborne(c) || !crossed(t, prev, c.dist, featureAt(t, o))) continue;
    if (fabsf(c.x - o.x / 100.0f) > PICKUP_HALF_W || (r.taken & (1u << f))) continue;
    if (r.inventory[o.item] >= MAX_INVENTORY) { r.fullMs[o.item] = 900; continue; }   // stays for a later lap
    r.inventory[o.item]++;
    r.taken |= (uint16_t)(1u << f);
    r.pickups++;
  }
}
// A rival's line: its lane, the inside of the coming bend, around what it sees ahead, now and then a wobble.
float aroundHazards(const Race& r, int i, float line) {
  const Track& t = *r.track;
  const Car& c = r.car[i];
  for (int f = 0; f < t.nFeatures; f++) {
    const Feature& o = t.features[f];
    if (o.kind == Kind::Pickup || !active(o, r.rung) || (o.kind != Kind::Obstacle && i == 1)) continue;   // the slowest rival ignores patches
    const float ahead = featureAt(t, o) - lapPos(t, c.dist);
    const float ox = o.x / 100.0f, room = o.w / 100.0f + 0.3f;
    if (ahead > 0 && ahead < 60 && fabsf(line - ox) < room) line = ox + (ox > 0 ? -room : room);
  }
  return line;
}
float aroundCars(const Race& r, int i, float line) {   // never sit in the back of a car just ahead
  const Car& c = r.car[i];
  for (int j = 0; j < CARS; j++) {
    const Car& o = r.car[j];
    if (j == i || o.finished || o.dist < c.dist || o.dist - c.dist > 14 || fabsf(o.x - line) > 0.35f) continue;
    line = o.x + (o.x > 0 ? -0.4f : 0.4f);
  }
  return line;
}
// Low levels only: a rival with the kid close behind it in its lane moves half a road out of the way, the closer
// the lower the level (up to 16 m), so a struggling kid can get past.
float courtesy(const Race& r, int i, float line) {
  if (r.level > LEVEL_COURTESY) return line;
  const int reach = 2 * (LEVEL_COURTESY + 1 - r.level);
  const Car& p = r.car[PLAYER];
  const float gap = r.car[i].dist - p.dist;
  if (gap <= 0 || gap >= (reach < 16 ? reach : 16) || fabsf(p.x - line) >= 0.4f) return line;
  return p.x + (p.x > 0 ? -0.5f : 0.5f);
}
float rivalLine(Race& r, int i) {
  Car& c = r.car[i];
  if (c.lineMs == 0) { c.lineMs = 6000 + rnd(r.seed) % 7000; c.wobbleMs = rnd(r.seed) % 4 == 0 ? 1200 : 0; }
  float line = (i - 2) * 0.45f + curve(*r.track, c.dist + 30) * 0.35f;
  if (c.wobbleMs > 500) line += (c.lineMs & 1) ? 0.9f : -0.9f;   // a seeded line error, then recovery
  return clampf(courtesy(r, i, aroundCars(r, i, aroundHazards(r, i, line))), -0.85f, 0.85f);
}
float rivalSteer(Race& r, int i) {
  Car& c = r.car[i];
  const float feed = curve(*r.track, c.dist) * DRIFT * c.speed * c.speed / STEER_RATE;
  return clampf((rivalLine(r, i) - c.x) * 3 + feed, -1, 1);
}
// The moment inside this step a car moving from prev to cur crossed `line`.
uint32_t crossingMs(const Race& r, float prev, float cur, float line) {
  return r.raceMs - STEP_MS + (uint32_t)lroundf((line - prev) / (cur - prev) * STEP_MS);
}
// The player's lap times: each lap runs from one crossing of the start line to the next (the first from the flying
// crossing just after GO), so every lap is timed the same way the finish is.
void lapClock(Race& r, float prev, float cur) {
  const float len = r.track->length;
  for (int k = 0; k <= LAPS; k++) {
    if (prev >= k * len || cur < k * len) continue;
    const uint32_t at = crossingMs(r, prev, cur, k * len);
    if (k > 0) r.lapMs[k - 1] = at - r.lapStartMs;
    r.lapStartMs = at;
  }
}
void drive(Race& r, int i, float steer) {
  Car& c = r.car[i];
  const float prev = c.dist;
  bool ice = false;
  throttle(c, capFor(r, c, &ice));
  c.dist += c.speed * V_MAX * DT;
  slide(r, c, steer, ice);
  obstacles(r, i);
  if (i == PLAYER) { pickups(r, prev); lapClock(r, prev, c.dist); }
  else if (c.refill && crossed(*r.track, prev, c.dist, 0)) c.boosts = MAX_INVENTORY;
  ramps(r, i, prev);
  const float line = LAPS * r.track->length;
  if (prev < line && c.dist >= line) {   // interpolate the crossing inside this step: a stable, fair order
    c.finished = true;
    c.finishMs = crossingMs(r, prev, c.dist, line);
    r.finishedCount++;
  }
}
void contacts(Race& r) {
  int pair = 0;
  for (int i = 0; i < CARS; i++) for (int j = i + 1; j < CARS; j++, pair++) {
    Car &a = r.car[i], &b = r.car[j];
    const uint8_t bit = (uint8_t)(1u << pair);
    const bool touch = !a.finished && !b.finished && !airborne(a) && !airborne(b) &&
                       fabsf(a.dist - b.dist) < CAR_LEN && fabsf(a.x - b.x) < 2 * CAR_HALF_W;
    if (touch && !(r.contact & bit)) impact(r, a.dist < b.dist ? i : j, CONTACT_LOSS);   // the car behind
    if (touch) {   // ease apart sideways until separated
      const float push = (a.x < b.x ? -1.2f : 1.2f) * DT;
      a.x = clampf(a.x + push, -RAIL, RAIL); b.x = clampf(b.x - push, -RAIL, RAIL);
    }
    r.contact = touch ? (uint8_t)(r.contact | bit) : (uint8_t)(r.contact & ~bit);
  }
}
void fire(Race& r) {
  const Car& p = r.car[PLAYER];
  for (Rocket& k : r.rocket) {
    if (k.live) continue;
    k = {p.dist + CAR_LEN, p.x, 0, true};
    r.inventory[ROCKET]--;
    r.cooldownMs = ROCKET_COOLDOWN_MS;
    r.rocketsFired++;
    return;
  }
}
void flyRocket(Race& r, Rocket& k) {
  const float prev = k.dist;
  k.dist += ROCKET_SPEED * V_MAX * DT;
  k.x -= curve(*r.track, k.dist) * DRIFT * DT;   // flies along the tangent: a bend carries it toward the edge
  k.ageMs += STEP_MS;
  if (k.ageMs >= ROCKET_LIFE_MS || fabsf(k.x) >= RAIL) {
    if (fabsf(k.x) >= RAIL) puff(r, k.dist, k.x);
    k.live = false;
    return;
  }
  for (int i = 1; i < CARS; i++) {
    Car& c = r.car[i];
    if (c.finished || c.dist < prev - CAR_LEN / 2 || c.dist > k.dist + CAR_LEN / 2) continue;
    if (fabsf(c.x - k.x) > CAR_HALF_W + 0.06f) continue;
    impact(r, i, ROCKET_LOSS);
    r.rocketHits++;
    r.hitMs = 900;
    puff(r, c.dist, c.x);
    k.live = false;
    return;
  }
}
void tick(Race& r) {
  for (Car& c : r.car) {
    c.boostMs = down(c.boostMs); c.protectMs = down(c.protectMs); c.airMs = down(c.airMs);
    c.wobbleMs = down(c.wobbleMs); c.lineMs = down(c.lineMs);
  }
  for (Puff& p : r.puff) p.ms = down(p.ms);
  r.rivalBoostMs = down(r.rivalBoostMs);
  r.cooldownMs = down(r.cooldownMs); r.calloutMs = down(r.calloutMs); r.hitMs = down(r.hitMs);
  r.fullMs[0] = down(r.fullMs[0]); r.fullMs[1] = down(r.fullMs[1]);
}
// A rival spends a charge only while chasing (the kid a little ahead of it), never as a hidden head start, and never
// right after another rival's boost.
void rivalBoosts(Race& r) {
  for (int i = 1; i < CARS && !r.rivalBoostMs; i++) {
    Car& c = r.car[i];
    const float gap = r.car[PLAYER].dist - c.dist;
    if (!c.boosts || c.boostMs || c.finished || gap <= CHASE_MIN || gap >= CHASE_MAX) continue;
    c.boosts--;
    c.boostMs = BOOST_MS;
    r.rivalBoostMs = RIVAL_BOOST_GAP_MS;
  }
}
void actions(Race& r, const Controls& in) {
  Car& p = r.car[PLAYER];
  if (in.boost && r.inventory[BOOST] && !p.boostMs) { r.inventory[BOOST]--; p.boostMs = BOOST_MS; }
  if (in.rocket && r.inventory[ROCKET] && !r.cooldownMs) fire(r);
}
void laps(Race& r) {
  const int lap = lapOf(r, PLAYER);
  if (lap != r.playerLap) {
    if (r.playerLap) r.calloutMs = 1500;
    r.playerLap = (uint8_t)lap;
    r.taken = 0;   // every gate comes back on the next lap
  }
  r.place = (uint8_t)placeOf(r, PLAYER);
  if (r.place == 1) r.firstMs += STEP_MS;
  if (r.car[PLAYER].finished) r.phase = Phase::Finished;   // the place is locked here, once
}
// The ladder in tune.h, one step a level: slower below 0, then charges, pace, and charges back each lap.
void ladder(Car& c, int i, int level, int rung, float floor) {
  constexpr int TOPS = sizeof LADDER_CHARGE, REFILLS = TOPS + sizeof LADDER_TOP;
  if (level < 0) c.top *= slowdown(level, floor);   // keyed on the kid's level: below 0 rivals are slower on every track
  for (int k = 0; k < rung && k < LADDER_RUNGS; k++) {   // Pro rungs change the track, not the rivals
    if (k < TOPS) c.boosts = (uint8_t)(c.boosts + (LADDER_CHARGE[k] == i));
    else if (k < REFILLS) { if (LADDER_TOP[k - TOPS] == i) c.top = fminf(1, c.top + LEVEL_TOP_STEP); }   // never past the kid
    else c.refill = c.refill || LADDER_REFILL[k - REFILLS] == i;
  }
}
}  // namespace

uint32_t bestLap(const Race& r) {
  uint32_t best = 0;
  for (uint32_t ms : r.lapMs) if (ms && (!best || ms < best)) best = ms;
  return best;
}
float hitSpeed(float before, float factor) {
  const float after = before * factor > HIT_FLOOR ? before * factor : HIT_FLOOR;
  return after < before ? after : before;
}
bool airborne(const Car& c) { return c.airMs > 0; }

float terrainCap(const Track& t, int rung, float dist, float x, bool* onIce) {
  float cap = fabsf(x) > 1 ? OFFROAD_CAP : 1;
  for (int f = 0; f < t.nFeatures; f++) {
    const Feature& o = t.features[f];
    if ((o.kind != Kind::Slow && o.kind != Kind::Ice) || !active(o, rung)) continue;
    if (!within(t, dist, featureAt(t, o), featureLen(t, o), 0) || fabsf(x - o.x / 100.0f) > o.w / 100.0f) continue;
    if (o.kind == Kind::Ice) *onIce = true;
    else if (SLOW_CAP < cap) cap = SLOW_CAP;
  }
  return cap;
}

int nextLevel(int level, int track, int place, bool runaway) {
  int next = level - 2;
  if (place == 1) next = level + (runaway ? 2 : 1);
  else if (level > LEVEL_GENTLE && rungOf(level, track) > LEVEL_EASY) next = level - LEVEL_HIGH_LOSS;
  else if (level > LEVEL_GENTLE) next = -2;   // rivals here only boost to pass a kid who leads: behind, the race would not change
  else if (level > 0) next = 0;
  return next < LEVEL_MIN ? LEVEL_MIN : next > LEVEL_MAX ? LEVEL_MAX : next;
}
bool runaway(const Race& r) { return r.firstMs * RUNAWAY_OF >= r.raceMs * RUNAWAY_LED; }
int rungOf(int level, int track) {
  // The track's shift in full from level 0 up, fading out by -LEVEL_SHIFT_FADE: a kid who struggles reaches the same
  // easy rivals after the same number of losses on every track.
  const int fade = level >= 0 ? LEVEL_SHIFT_FADE : level + LEVEL_SHIFT_FADE;
  return level + (fade > 0 ? LEVEL_SHIFT[track] * fade / LEVEL_SHIFT_FADE : 0);
}
int clampLevel(int level, int track) {   // past the track's top the ladder has no rungs left
  const int top = LEVEL_MAX - LEVEL_SHIFT[track];
  return level < LEVEL_MIN ? LEVEL_MIN : level > top ? top : level;
}

void start(Race& r, const Track& t, uint32_t seed, int level) {
  r = Race{};
  r.track = &t;
  r.seed = seed ? seed : 1;
  const int track = (int)(&t - TRACK);
  r.level = (int8_t)clampLevel(level, track);
  r.rung = (int8_t)rungOf(r.level, track);
  static const float TOP[CARS] = {1.0f, 0.92f, 0.97f, 1.0f};
  for (int i = 0; i < CARS; i++) {
    Car& c = r.car[i];
    const int slot = i == PLAYER ? CARS : i;   // the player starts at the back of the staggered grid
    c.grid = (uint8_t)slot;
    c.dist = -(float)(slot * GRID_SPACING);
    c.x = slot % 2 ? -0.35f : 0.35f;
    c.top = TOP[i];
    if (i != PLAYER) ladder(c, i, r.level, r.rung, LEVEL_FLOOR[track]);
    c.lineMs = 2000 + rnd(r.seed) % 3000;
  }
  r.playerLap = 1;
  r.place = CARS;
}

void step(Race& r, const Controls& in) {
  r.ms += STEP_MS;
  if (r.phase == Phase::Countdown) {   // nothing moves or spends before GO
    if (r.ms >= COUNTDOWN_MS) r.phase = Phase::Racing;
    return;
  }
  if (r.phase == Phase::Finished) return;
  r.raceMs += STEP_MS;
  tick(r);
  actions(r, in);
  rivalBoosts(r);
  for (int i = 0; i < CARS; i++) {
    if (r.car[i].finished) continue;   // finished rivals keep their order and leave the road
    drive(r, i, i == PLAYER ? in.steer : rivalSteer(r, i));
  }
  contacts(r);
  for (Rocket& k : r.rocket) if (k.live) flyRocket(r, k);
  laps(r);
}

int lapOf(const Race& r, int car) {
  const Car& c = r.car[car];
  if (c.finished) return LAPS;
  const int lap = (int)floorf(c.dist / r.track->length) + 1;
  return lap < 1 ? 1 : lap > LAPS ? LAPS : lap;
}

void order(const Race& r, uint8_t out[CARS]) {
  for (int i = 0; i < CARS; i++) out[i] = (uint8_t)i;
  auto ahead = [&](int a, int b) {   // finished by crossing time, others by progress, ties by grid
    const Car &x = r.car[a], &y = r.car[b];
    if (x.finished != y.finished) return x.finished;
    if (x.finished && x.finishMs != y.finishMs) return x.finishMs < y.finishMs;
    if (!x.finished && x.dist != y.dist) return x.dist > y.dist;
    return x.grid < y.grid;
  };
  for (int i = 1; i < CARS; i++)
    for (int j = i; j > 0 && ahead(out[j], out[j - 1]); j--) { const uint8_t t = out[j]; out[j] = out[j - 1]; out[j - 1] = t; }
}
int placeOf(const Race& r, int car) {
  uint8_t o[CARS];
  order(r, o);
  for (int i = 0; i < CARS; i++) if (o[i] == car) return i + 1;
  return CARS;
}
}  // namespace racing
