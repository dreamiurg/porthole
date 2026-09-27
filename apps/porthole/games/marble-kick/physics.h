// The marble: tilt to acceleration, rolling on felt, bouncing off pegs (still or moving), rails, the home knob and the
// rim (open in the goal's mouth, wherever the goal is on the rim), dropping into holes, picking up stars, and the goal.
// No drawing, so host/test_marble.cpp runs it as is. Floats in panel px from the tray's center (+y down), at a fixed
// STEP_MS substep: at V_MAX the ball moves 3 px a substep (a peg moving the other way adds at most 1 more), and the
// thinnest thing it can hit, a rail or a 7 px peg, is 27 px from the ball's center at contact, so it cannot pass
// through anything.
//
// Everything that moves (defenders, the keeper, the goal) follows the level's own clock, Ball::ms: it starts at the
// first substep with a real tilt (past the dead zone) and then runs on, through a drop into a hole too. So nothing
// moves until the kid starts, and a recorded solution replays exactly however long the page waited before it.
#pragma once
#include "levels.h"
#include "tune.h"

namespace marble {
constexpr int PITCH_R = 188;   // the felt's edge: the inside of the tray's wall
constexpr int BALL_R = 20;     // 40 px: about 4.5 mm on the glass, big enough to follow
constexpr int POST_R = 7;      // the brass goalposts, either side of the gap, set into the wall just clear of the felt:
constexpr int POST_RING = PITCH_R + POST_R + 1;   // they mark the mouth (physics.cpp, rim()); nothing bumps into them
constexpr int KNOB_Y = 195, KNOB_R = 30;   // the home knob sits on the wall at the bottom, and the ball bumps it
constexpr int RAIL_R = 8;      // a rail's half thickness
constexpr int STAR_R = 12;     // a star is picked up when the ball's center comes this close plus BALL_R
constexpr int HOLE_IN = 4;     // the ball drops in when its center is this far inside a hole's rim (and slow enough)

struct Vec { float x, y; };
struct Grav { int x, y, z; };   // gravity, milli-g, screen frame (Input::gx/gy/gz)
struct Ball {
  const Level* level;
  Vec p, v;
  bool goal;          // crossed the rim inside the mouth: nothing moves any more
  bool started;       // the level's clock is running (the first real tilt)
  uint32_t ms;        // the level's clock
  uint8_t stars;      // bit i: the level's star i picked up
  uint16_t sinkMs;    // in a hole for this long (0: not); back at the start after RESPAWN_MS
  Vec hole;           // where it dropped in
};

// The tilt away from the kid's neutral, in milli-g: gravity turned by the rotation that lays the neutral flat (face
// up, (0, 0, -1000)), then its x/y. So every grip gets the same range both ways: from upright, leaning back rolls the
// ball up and leaning forward rolls it down, as far as from lying flat.
Vec tiltFrom(Grav g, Grav neutral);
// That tilt through the dead zone and the clamp (tune.h): px/s^2.
Vec tiltAccel(Grav g, Grav neutral);

float goalAngle(const Level& l, uint32_t ms);          // radians clockwise from the top
Vec pegAt(const Level& l, const Peg& p, uint32_t ms);  // a peg's center now
Vec postAt(const Level& l, int side, uint32_t ms);     // side -1 left, +1 right, as seen from inside facing the goal
Vec mouthAt(const Level& l, uint32_t ms);              // the middle of the goal's mouth, on the rim
bool inGap(const Level& l, Vec p, uint32_t ms);        // in the goal's mouth: toward it, the ball's center clear of each post
Ball start(const Level& l);
void step(Ball& b, Vec accel);                         // one STEP_MS substep
inline bool sinking(const Ball& b) { return b.sinkMs > 0; }
int starCount(uint8_t bits);
}  // namespace marble
