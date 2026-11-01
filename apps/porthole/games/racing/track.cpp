#include "track.h"
#include "tune.h"

namespace racing {
namespace {
constexpr Feature gate(uint16_t at, int8_t x, uint8_t item, bool raised = false) { return {Kind::Pickup, at, 0, x, 0, item, raised, 0}; }
constexpr Feature ramp(uint16_t at, int8_t x) { return {Kind::Ramp, at, 0, x, 35, 0, false, 0}; }
constexpr Feature slow(uint16_t at, uint16_t len, int8_t x, int8_t w, uint8_t rung = 0) { return {Kind::Slow, at, len, x, w, 0, false, rung}; }
constexpr Feature ice(uint16_t at, uint16_t len, int8_t x, int8_t w) { return {Kind::Ice, at, len, x, w, 0, false, 0}; }
constexpr Feature block(uint16_t at, int8_t x, int8_t w, uint8_t rung = 0) { return {Kind::Obstacle, at, 6, x, w, 0, false, rung}; }

// Desert Canyon: broad bends, one mud patch, a dirt ramp on the left with a flat bypass, two isolated rocks. Pro (rungs
// 19-21, tune.h LEVEL_PRO): a rock mid-road before the ramp, one on the left shoulder, one past the landing.
constexpr Section DESERT_S[] = {{0, 0}, {20, 70}, {40, -20}, {55, 0}, {72, -50}, {81, 50}, {90, 0}};
constexpr Feature DESERT_F[] = {
    gate(80, 0, BOOST), slow(450, 40, -45, 35), gate(470, 50, ROCKET), ramp(600, -50), gate(611, -50, BOOST, true),
    block(760, 75, 16), block(850, -75, 16), gate(940, 30, ROCKET),
    block(540, 0, 16, 19), block(500, -70, 16, 20), block(660, 40, 16, 21)};
// Industrial Ruins: a left bend with crates on the inside shoulder, a yard of two oil patches, a loading-dock ramp
// over rough ground, a gantry chicane with crates on alternate shoulders. Pro: oil on the left past the dock (the
// only candidate the sweep kept once obstacles hit where drawn; a mid-road crate made races easier).
constexpr Section INDUSTRIAL_S[] = {{0, 0}, {18, -60}, {38, 0}, {54, 0}, {70, 55}, {79, -55}, {88, 0}};
constexpr Feature INDUSTRIAL_F[] = {
    gate(90, 0, ROCKET), block(280, -80, 18), gate(330, 40, ROCKET), slow(420, 30, -55, 30), slow(460, 30, 55, 30),
    gate(510, 0, BOOST), ramp(600, 50), slow(612, 8, 50, 30), gate(611, 50, BOOST, true), block(750, -80, 18),
    block(840, 80, 18), gate(930, -30, BOOST),
    slow(660, 30, -70, 30, 19)};
// Snow Track: dark grippy asphalt, marked ice patches on the outside of the long bends, slush outside the S-bend,
// a snow-bank ramp with a wide dry landing. No Pro hazard: none the sweep tried made a race harder for both drivers.
constexpr Section SNOW_S[] = {{0, 0}, {20, 60}, {40, -50}, {49, 50}, {58, 0}, {74, -55}, {90, 0}};
constexpr Feature SNOW_F[] = {
    gate(80, 0, BOOST), ice(280, 40, -45, 35), slow(430, 30, 75, 25), gate(470, 0, ROCKET), slow(520, 30, -75, 25),
    ramp(630, 45), gate(641, 45, BOOST, true), ice(800, 40, 45, 35), gate(940, -30, ROCKET)};
// Night Run: lit bends, puddles on alternating sides of the plaza, an overpass ramp, two marked barriers. Pro: a
// barrier right of center after the overpass.
constexpr Section NIGHT_S[] = {{0, 0}, {18, 60}, {38, 0}, {54, 0}, {72, -55}, {81, 55}, {90, 0}};
constexpr Feature NIGHT_F[] = {
    gate(80, 0, BOOST), gate(300, -30, ROCKET), slow(410, 30, -55, 30), gate(455, 0, ROCKET), slow(470, 30, 55, 30),
    ramp(610, -50), gate(621, -50, BOOST, true), block(770, 80, 18), block(860, -80, 18), gate(940, 30, BOOST),
    block(660, 40, 18, 19)};
template <class T, int N> constexpr int count(const T (&)[N]) { return N; }
constexpr int lapGap(int a, int b) { const int d = a > b ? a - b : b - a; return d > 500 ? 1000 - d : d; }   // tenths, wrapped
constexpr int absi(int v) { return v < 0 ? -v : v; }
constexpr int gapTo(const Feature& h, int at) {   // tenths of a lap from a point to h's span, 0 inside it
  return at >= h.at && at <= h.at + h.len ? 0 : lapGap(h.at, at) < lapGap(h.at + h.len, at) ? lapGap(h.at, at) : lapGap(h.at + h.len, at);
}
constexpr int GATE_W = (int)(PICKUP_HALF_W * 100 + 0.5f);   // a gate's lane, hundredths of a half-width
constexpr int CAR_W = (int)(2 * CAR_HALF_W * 100 + 0.5f) + 5;   // room for a car between two things, and a margin
// A Pro hazard keeps 1% of a lap from every gate and ramp and out of their lane (a car taking them would touch it)
// within 3%, or for a ramp within a boosted jump's length, so it never blocks a pickup or a jump nor springs out just
// behind one or under a landing; and 1% from any other hazard in an overlapping lane, so none stack. length: m a lap.
constexpr bool clearOf(const Feature& h, const Feature& g, float length) {
  if (&h == &g) return true;
  const bool gate = g.kind == Kind::Pickup || g.kind == Kind::Ramp;
  const bool lane = absi(h.x - g.x) < h.w + (gate ? (g.w > GATE_W ? g.w : GATE_W) : g.w) + CAR_W;
  const int gap = gapTo(h, g.at) < gapTo(h, g.at + g.len) ? gapTo(h, g.at) : gapTo(h, g.at + g.len);
  const bool overlap = !(h.at + h.len < g.at || g.at + g.len < h.at);
  const int jump = (int)(AIR_MS / 1000.0f * BOOST_CAP * V_MAX / length * 1000) + 1;   // tenths of a lap
  const int near = g.kind == Kind::Ramp && jump > 30 ? jump : 30;
  if (gate) return gap >= 10 && !(lane && gap < near);
  return !lane || (!overlap && gap >= 10);
}
// Each Pro hazard sits on a Pro rung and on the road, clear of the rest (clearOf); a track has at most LEVEL_PRO.
template <int N> constexpr bool proFits(const Feature (&f)[N], float length) {
  int pro = 0;
  for (const Feature& h : f) {
    if (!h.rung) continue;
    pro++;
    if (h.rung <= LADDER_RUNGS || h.rung > LEVEL_MAX || absi(h.x) + h.w > 100) return false;
    for (const Feature& g : f) if (!clearOf(h, g, length)) return false;
  }
  return pro <= LEVEL_PRO;
}
constexpr float DESERT_M = 900, INDUSTRIAL_M = 1050, SNOW_M = 1140, NIGHT_M = 1050;   // a lap
static_assert(proFits(DESERT_F, DESERT_M) && proFits(INDUSTRIAL_F, INDUSTRIAL_M) && proFits(SNOW_F, SNOW_M) &&
              proFits(NIGHT_F, NIGHT_M), "a Pro hazard is misplaced");
// race.cpp keeps a bit per feature in 16-bit masks (gates taken, obstacles touched).
static_assert(count(DESERT_F) <= MAX_FEATURES && count(INDUSTRIAL_F) <= MAX_FEATURES && count(SNOW_F) <= MAX_FEATURES &&
              count(NIGHT_F) <= MAX_FEATURES, "a track has more features than the race's bitmasks hold");
}  // namespace

const Track TRACK[TRACKS] = {
    {"Desert Canyon", "Desert", "Canyon", DESERT_M, DESERT_S, count(DESERT_S), DESERT_F, count(DESERT_F)},
    {"Industrial Ruins", "Industrial", "Ruins", INDUSTRIAL_M, INDUSTRIAL_S, count(INDUSTRIAL_S), INDUSTRIAL_F, count(INDUSTRIAL_F)},
    {"Snow Track", "Snow", "Track", SNOW_M, SNOW_S, count(SNOW_S), SNOW_F, count(SNOW_F)},
    {"Night Run", "Night", "Run", NIGHT_M, NIGHT_S, count(NIGHT_S), NIGHT_F, count(NIGHT_F)},
};

float curve(const Track& t, float dist) {
  const float pct = lapPos(t, dist) * 100 / t.length;
  int i = 0;
  while (i + 1 < t.nSections && pct >= t.sections[i + 1].from) i++;
  const float to = t.sections[i].curve / 100.0f;
  const float from = t.sections[i == 0 ? t.nSections - 1 : i - 1].curve / 100.0f;
  const float blend = (pct - t.sections[i].from) / 4;   // ease in over 4% of the lap
  if (blend >= 1) return to;
  const float s = blend * blend * (3 - 2 * blend);
  return from + (to - from) * s;
}
}  // namespace racing
