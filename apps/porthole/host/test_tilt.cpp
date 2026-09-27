// Self-check for the shared tilt input (os/tilt.h): the tilt away from any grip turns the way the device leans, the
// same amount both ways, and the steady neutral ignores a jolt. What each game does with a tilt is in its own test.
// Run: make test
#include <assert.h>
#include <math.h>
#include <stdio.h>
#include "host/grip.h"
#include "tilt.h"

using namespace grip;
static int checks = 0;
#define CHECK(c) (assert(c), checks++)

// From every grip: holding it as calibrated is no tilt, and a tilt away from it reads back as itself.
static void everyGrip() {
  static const Grav HOLDS[] = {FLAT, LEANED, UPRIGHT, FACE_DOWN, {500, 500, -707}};
  for (Grav n : HOLDS) {
    const tilt::Vec none = tilt::from(n, n);
    CHECK(fabsf(none.x) < 1 && fabsf(none.y) < 1);
    for (int t = -600; t <= 600; t += 200) {
      const tilt::Vec a = tilt::from(gravityFor(t, 0, n), n), b = tilt::from(gravityFor(0, t, n), n);
      CHECK(fabsf(a.x - t) < 3 && fabsf(a.y) < 3 && fabsf(b.y - t) < 3 && fabsf(b.x) < 3);
    }
  }
  const tilt::Vec raw = tilt::from({120, -80, -990}, {0, 0, 0});   // no neutral at all: read as lying flat
  CHECK(raw.x == 120 && raw.y == -80);
}
// Leaned back 45 degrees, checked with plain rotations rather than gravityFor (which is tilt::from's own inverse):
// gravity in the device's frame is (0, cos a, -sin a) leaned back by a from upright, so tipping the top edge 20 degrees
// further away tilts up and 20 degrees back toward you tilts down; turning about the screen's up axis so the right edge
// drops 20 degrees, gravity (0, 707, -707) becomes (707 sin 20, 707, -707 cos 20), and it tilts right.
static void asHeld() {
  const float deg = 3.14159265f / 180;
  const Grav away = {0, (int)lroundf(1000 * cosf(65 * deg)), (int)lroundf(-1000 * sinf(65 * deg))};
  const Grav toward = {0, (int)lroundf(1000 * cosf(25 * deg)), (int)lroundf(-1000 * sinf(25 * deg))};
  const Grav rightDown = {(int)lroundf(707 * sinf(20 * deg)), 707, (int)lroundf(-707 * cosf(20 * deg))};
  // 20 degrees from the grip is a 1000 sin 20 = 342 mg tilt; the turn, about an axis leaned 45 degrees, 707 sin 20 = 242
  const tilt::Vec up = tilt::from(away, LEANED), down = tilt::from(toward, LEANED), right = tilt::from(rightDown, LEANED);
  CHECK(fabsf(up.y + 342) < 3 && fabsf(up.x) < 3 && fabsf(down.y - 342) < 3 && fabsf(down.x) < 3);
  CHECK(fabsf(right.x - 242) < 3 && fabsf(right.y) < 0.2f * right.x);
  // held upright, leaning back 30 degrees tilts up, leaning forward tilts down, a steering-wheel turn tilts sideways
  CHECK(tilt::from({0, 866, -500}, UPRIGHT).y < -490 && tilt::from({0, 866, 500}, UPRIGHT).y > 490);
  const tilt::Vec wheel = tilt::from({342, 940, 0}, UPRIGHT);
  CHECK(fabsf(wheel.x - 342) < 3 && fabsf(wheel.y) < 3);
}
// The neutral is the average of the last 300 ms, and nothing while a reading in it is not a plausible 1 g.
static void steadyNeutral() {
  tilt::Steady s;
  Grav n = {9, 9, 9};
  CHECK(!s.get(0, &n) && n.x == 9);   // nothing held yet
  for (uint32_t ms = 0; ms <= 1000; ms += 40) s.add(ms < 500 ? 0 : 20, 1000, ms < 500 ? 0 : -40, ms);
  CHECK(s.get(1000, &n) && n.x == 20 && n.y == 1000 && n.z == -40);   // only the last 300 ms count
  s.add(1800, 0, 900, 1040);   // a jolt: no neutral until it is out of the window
  CHECK(!s.get(1040, &n));
  for (uint32_t ms = 1080; ms <= 1400; ms += 40) s.add(20, 1000, -40, ms);
  CHECK(s.get(1400, &n) && n.x == 20);
  s = {};
  s.add(0, 300, -300, 0);   // weightless or falling (0.42 g) is not a way of holding it either
  CHECK(!s.get(0, &n));
}

int main() {
  everyGrip();
  asHeld();
  steadyNeutral();
  printf("test_tilt: all %d checks passed\n", checks);
  return 0;
}
