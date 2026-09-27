// The marble: tilt to acceleration, rolling on felt, bouncing off the pegs, the home knob and the rim (open in the
// goal's mouth), and the goal. No drawing, so host/test_marble.cpp runs it as is. Floats in panel px from the tray's
// center (+y down), at a fixed STEP_MS substep: at V_MAX the ball moves 3 px a substep, and even a 7 px peg is 27 px
// from the ball's center at contact, so it cannot pass through anything.
#pragma once
#include "levels.h"
#include "tune.h"

namespace marble {
constexpr int PITCH_R = 188;   // the felt's edge: the inside of the tray's wall
constexpr int BALL_R = 20;      // 40 px: about 4.5 mm on the glass, big enough to follow
constexpr int POST_R = 7;      // the brass goalposts, either side of the gap, set into the wall just clear of the felt:
constexpr int POST_RING = PITCH_R + POST_R + 1;   // they mark the mouth (physics.cpp, rim()); nothing bumps into them
constexpr int KNOB_Y = 195, KNOB_R = 30;   // the back knob sits on the wall at the bottom, and the ball bumps it

struct Vec { float x, y; };
struct Ball {
  const Level* level;
  Vec p, v;
  bool goal;    // crossed the rim inside the gap: nothing moves any more
};

struct Grav { int x, y, z; };   // gravity, milli-g, screen frame (Input::gx/gy/gz)

// The tilt away from the kid's neutral, in milli-g: gravity turned by the rotation that lays the neutral flat (face
// up, (0, 0, -1000)), then its x/y. So every grip gets the same range both ways: from upright, leaning back rolls the
// ball up and leaning forward rolls it down, as far as from lying flat.
Vec tiltFrom(Grav g, Grav neutral);
// That tilt through the dead zone and the clamp (tune.h): px/s^2.
Vec tiltAccel(Grav g, Grav neutral);
Vec postAt(const Level& l, int side);   // side -1 left, +1 right
bool inGap(const Level& l, Vec p);      // in the goal's mouth: above the middle, the ball's center clear of each post
Ball start(const Level& l);
void step(Ball& b, Vec accel);          // one STEP_MS substep
}  // namespace marble
