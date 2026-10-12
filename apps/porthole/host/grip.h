// For the tilt games' host tests (test_tilt, test_marble, test_tiltfc): the ways a kid holds the device, and the real
// gravity a tilt away from such a grip reads as, the inverse of tilt::from (os/tilt.h). Not built into the firmware.
#pragma once
#include <math.h>
#include "tilt.h"

namespace grip {
using tilt::Grav;
// Lying flat, leaned back 45 degrees, upright (the sim's default hold), and upside down.
inline constexpr Grav FLAT = {0, 0, -1000}, LEANED = {0, 707, -707}, UPRIGHT = {0, 1000, 0}, FACE_DOWN = {0, 0, 1000};
inline float mag(Grav g) { return sqrtf((float)(g.x * g.x + g.y * g.y + g.z * g.z)); }

// The real gravity (1 g, whole milli-g, as the sensor reports it) when the device is tilted by (tx, ty) milli-g away
// from `neutral`: the tilt as it would be lying flat, turned back by the rotation that lays the neutral flat.
inline Grav gravityFor(int tx, int ty, Grav neutral) {
  const float n = mag(neutral), ax = neutral.x / n, ay = neutral.y / n, az = neutral.z / n;
  const float fx = (float)tx, fy = (float)ty, fz = -sqrtf(1e6f - fx * fx - fy * fy);
  float gx, gy, gz;
  if (az > 0.9999f) { gx = fx; gy = -fy; gz = -fz; }   // face down: half a turn about x
  else {   // Rodrigues with v = a x (0,0,-1) = (-ay, ax, 0), c = -az, inverted: g = f - v x f + v x (v x f) / (1 + c)
    const float vx = -ay, vy = ax, k = 1 / (1 - az);
    const float cx = vy * fz, cy = -vx * fz, cz = vx * fy - vy * fx;   // v x f
    const float dx = vy * cz, dy = -vx * cz, dz = vx * cy - vy * cx;   // v x (v x f)
    gx = fx - cx + dx * k; gy = fy - cy + dy * k; gz = fz - cz + dz * k;
  }
  return {(int)lroundf(gx), (int)lroundf(gy), (int)lroundf(gz)};
}
}  // namespace grip
