// The marble: tilt to acceleration, rolling on felt, bouncing off the rim, pegs, goalposts and the back knob, and the
// goal. No drawing, so host/test_marble.cpp runs it as is. Floats in panel px from the tray's center (+y down), at a
// fixed STEP_MS substep: at V_MAX the ball moves 3 px a substep, and the thinnest thing it can hit (a goalpost) is
// POST_R + BALL_R = 27 px from the ball's center at contact, so it cannot pass through anything.
#pragma once
#include "levels.h"
#include "tune.h"

namespace marble {
constexpr int PITCH_R = 188;   // the felt's edge: the inside of the tray's wall
constexpr int BALL_R = 20;      // 40 px: about 4.5 mm on the glass, big enough to follow
constexpr int POST_R = 7;      // the brass goalposts, on the wall's edge either side of the gap
constexpr int KNOB_Y = 195, KNOB_R = 30;   // the back knob sits on the wall at the bottom, and the ball bumps it

struct Vec { float x, y; };
struct Ball {
  const Level* level;
  Vec p, v;
  bool goal;    // crossed the rim inside the gap: nothing moves any more
};

// Gravity (gx, gy, milli-g, screen frame) minus the kid's neutral, through the dead zone and the clamp: px/s^2.
Vec tiltAccel(int gx, int gy, int neutralX, int neutralY);
Vec postAt(const Level& l, int side);   // side -1 left, +1 right
bool inGap(const Level& l, Vec p);      // above the middle and between the posts: the rim is open here
Ball start(const Level& l);
void step(Ball& b, Vec accel);          // one STEP_MS substep
}  // namespace marble
