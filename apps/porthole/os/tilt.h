// Tilt from however the kid holds the device, shared by every tilt game (Marble Kick, Tilt FC). A game stores the
// kid's own neutral (Steady, on its Calibrate page) and reads the tilt away from it (from); what the tilt then does (an
// acceleration, a running direction, a dead zone) stays in the game. Header-only; host/test_tilt.cpp checks it.
#pragma once
#include <math.h>
#include <stdint.h>

namespace tilt {
struct Grav { int x, y, z; };   // gravity, milli-g, screen frame (Input::gx/gy/gz)
struct Vec { float x, y; };     // a tilt in the screen's plane, milli-g: +x toward the right edge, +y toward the bottom

// The tilt away from the kid's neutral: gravity turned by the rotation that lays the neutral flat (face up,
// (0, 0, -1000)), then its x/y. So every grip gets the same range both ways: from upright, leaning back tilts up and
// leaning forward tilts down, as far as from lying flat.
inline Vec from(Grav g, Grav neutral) {
  const float n = sqrtf((float)(neutral.x * neutral.x + neutral.y * neutral.y + neutral.z * neutral.z));
  if (n < 1) return {(float)g.x, (float)g.y};   // no reading to be level with: lying flat
  const float ax = neutral.x / n, ay = neutral.y / n, az = neutral.z / n, x = (float)g.x, y = (float)g.y, z = (float)g.z;
  if (az > 0.9999f) return {x, -y};   // face down: half a turn about x (the rotation below is undefined there)
  // Rodrigues, from a (the neutral) to (0, 0, -1): v = a x (0, 0, -1) = (-ay, ax, 0), cos = -az;
  // g' = g + v x g + v x (v x g) / (1 + cos). Only x and y are needed.
  const float vx = -ay, vy = ax, k = 1 / (1 - az);
  const float cx = vy * z, cy = -vx * z, cz = vx * y - vy * x;   // v x g
  return {x + cx + vy * cz * k, y + cy - vx * cz * k};
}

// The neutral: the average gravity of the last WINDOW_MS, as long as every reading in it is a plausible 1 g (a jolt
// or a shake is not a way of holding it; a kid's press then waits until it has passed). Fed every frame.
class Steady {
 public:
  static constexpr uint32_t WINDOW_MS = 300;
  static constexpr int MIN_MG = 800, MAX_MG = 1200;
  void add(int x, int y, int z, uint32_t ms) { s_[n_++ % SAMPLES] = {(int16_t)x, (int16_t)y, (int16_t)z, ms}; }
  bool get(uint32_t ms, Grav* neutral) const {
    long sx = 0, sy = 0, sz = 0;
    int n = 0;
    for (int i = 0; i < SAMPLES && i < n_; i++) {
      const Sample& s = s_[i];
      if (ms - s.ms > WINDOW_MS) continue;
      const long m2 = (long)s.x * s.x + (long)s.y * s.y + (long)s.z * s.z;
      if (m2 < (long)MIN_MG * MIN_MG || m2 > (long)MAX_MG * MAX_MG) return false;
      sx += s.x; sy += s.y; sz += s.z; n++;
    }
    if (!n) return false;
    *neutral = {(int)lroundf((float)sx / n), (int)lroundf((float)sy / n), (int)lroundf((float)sz / n)};
    return true;
  }

 private:
  static constexpr int SAMPLES = 24;   // WINDOW_MS at up to 80 frames a second
  struct Sample { int16_t x, y, z; uint32_t ms; };
  Sample s_[SAMPLES] = {};
  int n_ = 0;
};

// A Calibrate page's preview: a reference that follows gravity over about a second (each frame closes dt / 1000 of the
// gap, the first reading sets it), so a move shows as a tilt away from it and holding still settles it. Fed every frame.
class Follow {
 public:
  void reset() { set_ = false; }
  void add(int x, int y, int z, uint32_t dt) {
    const float g[3] = {(float)x, (float)y, (float)z}, k = dt >= 1000 ? 1 : dt / 1000.0f;
    for (int i = 0; i < 3; i++) r_[i] = set_ ? r_[i] + (g[i] - r_[i]) * k : g[i];
    set_ = true;
  }
  Grav get() const { return {(int)lroundf(r_[0]), (int)lroundf(r_[1]), (int)lroundf(r_[2])}; }

 private:
  float r_[3] = {0, 0, -1000};
  bool set_ = false;
};
}  // namespace tilt
