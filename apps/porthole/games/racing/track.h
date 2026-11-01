// The four authored circuits (docs/superpowers/specs/porthole-racing/tracks.md). A lap is `length` meters; positions
// are percent of a lap so the tables read like the design. One curvature table drives both the car physics and the
// road drawing, so what the kid sees bending is what pushes the car.
#pragma once
#include <stdint.h>

namespace racing {
enum class Kind : uint8_t { Pickup, Ramp, Slow, Ice, Obstacle };
enum Item : uint8_t { BOOST, ROCKET };
struct Section { uint8_t from; int8_t curve; };   // curve in hundredths: +100 = the sharpest right bend
struct Feature {
  Kind kind;
  uint16_t at, len;   // tenths of a percent of the lap: start and length (0 for gates)
  int8_t x, w;        // lateral center and half-width, hundredths of a road half-width
  uint8_t item;       // Pickup: BOOST/ROCKET
  bool raised;        // Pickup: only while airborne
  uint8_t rung;       // 0: always there; else a Pro hazard, on only from this ladder rung up (tune.h LEVEL_PRO)
};
struct Track {
  const char* name;
  const char* line1;  // the track card's two lines
  const char* line2;
  float length;       // m per lap
  const Section* sections; int nSections;
  const Feature* features; int nFeatures;
};
constexpr int TRACKS = 4;
extern const Track TRACK[TRACKS];

constexpr int MAX_FEATURES = 16;   // bits in Race::taken and Car::solid
inline float lapPos(const Track& t, float dist) {   // 0 .. length
  float p = dist - (float)(int)(dist / t.length) * t.length;
  if (p < 0) p += t.length;
  return p;
}
inline bool active(const Feature& f, int rung) { return !f.rung || f.rung <= rung; }   // rung: Race::rung
inline float featureAt(const Track& t, const Feature& f) { return f.at * t.length / 1000; }
inline float featureLen(const Track& t, const Feature& f) { return f.len * t.length / 1000; }
// m from dist to the middle of f (where an obstacle is drawn), wrapped to within half a lap either way.
inline float middleAhead(const Track& t, const Feature& f, float dist) {
  const float ahead = featureAt(t, f) + featureLen(t, f) / 2 - lapPos(t, dist);
  return ahead < -t.length / 2 ? ahead + t.length : ahead > t.length / 2 ? ahead - t.length : ahead;
}
float curve(const Track& t, float dist);   // -1..1, blended across the first 4% of each section
}  // namespace racing
