// Accelerometer-only steering wheel: screen-plane gravity rotates when the upright puck turns. The same
// validated math as Starfighter's banking (a precedent, kept local: games never include each other).
#pragma once
#include <math.h>
#include <stdint.h>
#include "tilt.h"

namespace racing::controls {
constexpr float DEAD_DEGREES = 3, FULL_DEGREES = 20;
constexpr uint32_t STEADY_MS = 350, SAMPLE_GAP_MS = 100;
inline bool plausible(tilt::Grav g) {
  const int64_t m = (int64_t)g.x * g.x + (int64_t)g.y * g.y + (int64_t)g.z * g.z;
  return m >= 800 * 800 && m <= 1200 * 1200;
}
// Still held like a wheel during a race: anything but nearly flat. Leaning back up to about 75 degrees from upright,
// or turning hard, keeps racing; only a puck laid on a table (under FLAT_MG in the screen plane) stops it.
constexpr int FLAT_MG = 250;
inline bool held(tilt::Grav g) {
  return plausible(g) && (int64_t)g.x * g.x + (int64_t)g.y * g.y >= (int64_t)FLAT_MG * FLAT_MG;
}
inline bool upright(tilt::Grav g) {
  return plausible(g) && g.y >= 400 && (int64_t)g.x * g.x + (int64_t)g.y * g.y >= 650 * 650;
}
inline float steer(tilt::Grav g, tilt::Grav neutral) {
  if (!plausible(g) || !upright(neutral)) return 0;
  if (!held(g)) return 0;
  const float cross = (float)g.x * neutral.y - (float)g.y * neutral.x;
  const float dot = (float)g.x * neutral.x + (float)g.y * neutral.y;
  const float angle = atan2f(cross, dot) * (180 / 3.14159265f);
  const float amount = (fabsf(angle) - DEAD_DEGREES) / (FULL_DEGREES - DEAD_DEGREES);
  if (amount <= 0) return 0;
  return copysignf(amount > 1 ? 1 : amount, angle);
}

// Steady supplies the recent average; duration and anchor tests also reject a single sample or a moving grip.
class Calibration {
 public:
  void add(tilt::Grav g, uint32_t ms) {
    const uint32_t dt = ms - lastMs_;
    lastMs_ = ms;
    if (!upright(g)) { reset(); return; }
    const int64_t dx = g.x - anchor_.x, dy = g.y - anchor_.y, dz = g.z - anchor_.z;
    if (!started_ || dt > SAMPLE_GAP_MS || dx * dx + dy * dy + dz * dz > 90 * 90) {
      reset(); anchor_ = g; started_ = true;
    } else duration_ = duration_ + dt > STEADY_MS ? STEADY_MS : duration_ + dt;
    steady_.add(g.x, g.y, g.z, ms);
  }
  bool get(tilt::Grav* neutral) const { return duration_ >= STEADY_MS && steady_.get(lastMs_, neutral) && upright(*neutral); }
  uint16_t progress() const { return (uint16_t)(duration_ * 1000 / STEADY_MS); }
 private:
  void reset() { steady_ = {}; duration_ = 0; started_ = false; }
  tilt::Steady steady_;
  tilt::Grav anchor_{};
  uint32_t lastMs_ = 0, duration_ = 0;
  bool started_ = false;
};
}  // namespace racing::controls
