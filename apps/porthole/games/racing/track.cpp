#include "track.h"

namespace racing {
namespace {
constexpr Feature gate(uint16_t at, int8_t x, uint8_t item, bool raised = false) { return {Kind::Pickup, at, 0, x, 0, item, raised}; }
constexpr Feature ramp(uint16_t at, int8_t x) { return {Kind::Ramp, at, 0, x, 35, 0, false}; }
constexpr Feature slow(uint16_t at, uint16_t len, int8_t x, int8_t w) { return {Kind::Slow, at, len, x, w, 0, false}; }
constexpr Feature ice(uint16_t at, uint16_t len, int8_t x, int8_t w) { return {Kind::Ice, at, len, x, w, 0, false}; }
constexpr Feature block(uint16_t at, int8_t x, int8_t w) { return {Kind::Obstacle, at, 6, x, w, 0, false}; }

// Desert Canyon: broad bends, one mud patch, a dirt ramp on the left with a flat bypass, two isolated rocks.
constexpr Section DESERT_S[] = {{0, 0}, {20, 70}, {40, -20}, {55, 0}, {72, -50}, {81, 50}, {90, 0}};
constexpr Feature DESERT_F[] = {
    gate(80, 0, BOOST), slow(450, 40, -45, 35), gate(470, 50, ROCKET), ramp(600, -50), gate(611, -50, BOOST, true),
    block(760, 75, 16), block(850, -75, 16), gate(940, 30, ROCKET)};
// Industrial Ruins: a left bend with crates on the inside shoulder, a yard of two oil patches, a loading-dock ramp
// over rough ground, a gantry chicane with crates on alternate shoulders.
constexpr Section INDUSTRIAL_S[] = {{0, 0}, {18, -60}, {38, 0}, {54, 0}, {70, 55}, {79, -55}, {88, 0}};
constexpr Feature INDUSTRIAL_F[] = {
    gate(90, 0, ROCKET), block(280, -80, 18), gate(330, 40, ROCKET), slow(420, 30, -55, 30), slow(460, 30, 55, 30),
    gate(510, 0, BOOST), ramp(600, 50), slow(612, 8, 50, 30), gate(611, 50, BOOST, true), block(750, -80, 18),
    block(840, 80, 18), gate(930, -30, BOOST)};
// Snow Track: dark grippy asphalt, marked ice patches on the outside of the long bends, slush outside the S-bend,
// a snow-bank ramp with a wide dry landing.
constexpr Section SNOW_S[] = {{0, 0}, {20, 60}, {40, -50}, {49, 50}, {58, 0}, {74, -55}, {90, 0}};
constexpr Feature SNOW_F[] = {
    gate(80, 0, BOOST), ice(280, 40, -45, 35), slow(430, 30, 75, 25), gate(470, 0, ROCKET), slow(520, 30, -75, 25),
    ramp(630, 45), gate(641, 45, BOOST, true), ice(800, 40, 45, 35), gate(940, -30, ROCKET)};
// Night Run: lit bends, puddles on alternating sides of the plaza, an overpass ramp, two marked barriers.
constexpr Section NIGHT_S[] = {{0, 0}, {18, 60}, {38, 0}, {54, 0}, {72, -55}, {81, 55}, {90, 0}};
constexpr Feature NIGHT_F[] = {
    gate(80, 0, BOOST), gate(300, -30, ROCKET), slow(410, 30, -55, 30), gate(455, 0, ROCKET), slow(470, 30, 55, 30),
    ramp(610, -50), gate(621, -50, BOOST, true), block(770, 80, 18), block(860, -80, 18), gate(940, 30, BOOST)};
template <class T, int N> constexpr int count(const T (&)[N]) { return N; }
}  // namespace

const Track TRACK[TRACKS] = {
    {"Desert Canyon", "Desert", "Canyon", 900, DESERT_S, count(DESERT_S), DESERT_F, count(DESERT_F)},
    {"Industrial Ruins", "Industrial", "Ruins", 1050, INDUSTRIAL_S, count(INDUSTRIAL_S), INDUSTRIAL_F, count(INDUSTRIAL_F)},
    {"Snow Track", "Snow", "Track", 1140, SNOW_S, count(SNOW_S), SNOW_F, count(SNOW_F)},
    {"Night Run", "Night", "Run", 1050, NIGHT_S, count(NIGHT_S), NIGHT_F, count(NIGHT_F)},
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
