#include "physics.h"
#include <math.h>

namespace marble {
namespace {
constexpr float DT = STEP_MS / 1000.0f;
float dot(Vec a, Vec b) { return a.x * b.x + a.y * b.y; }

// Keeps the ball out of a fixed circle: pushed back to touching, and its speed into the circle reflected.
void bumpOff(Ball& b, Vec c, float r) {
  const Vec d = {b.p.x - c.x, b.p.y - c.y};
  const float touch = r + BALL_R, d2 = dot(d, d);
  if (d2 >= touch * touch) return;
  const float dist = sqrtf(d2);
  const Vec n = dist > 0.001f ? Vec{d.x / dist, d.y / dist} : Vec{0, -1};
  b.p = {c.x + n.x * touch, c.y + n.y * touch};
  const float vn = dot(b.v, n);
  if (vn < 0) b.v = {b.v.x - (1 + PEG_BOUNCE) * vn * n.x, b.v.y - (1 + PEG_BOUNCE) * vn * n.y};
}
// The tray's wall, open between the posts: the ball's edge stops at the felt's edge everywhere else.
void rim(Ball& b) {
  const float d = sqrtf(dot(b.p, b.p)), lim = PITCH_R - BALL_R;
  if (d <= lim || inGap(*b.level, b.p)) return;
  const Vec n = {b.p.x / d, b.p.y / d};
  b.p = {n.x * lim, n.y * lim};
  const float vn = dot(b.v, n);
  if (vn > 0) b.v = {b.v.x - (1 + RIM_BOUNCE) * vn * n.x, b.v.y - (1 + RIM_BOUNCE) * vn * n.y};
}
// Felt drag and rolling friction together take this much speed; a ball they would stop, stops (never reverses).
void roll(Ball& b) {
  const float s = sqrtf(dot(b.v, b.v)), lose = s * DAMPING * DT + ROLL_DECEL * DT;
  const float keep = s <= lose ? 0 : (s - lose > V_MAX ? V_MAX : s - lose) / s;
  b.v = {b.v.x * keep, b.v.y * keep};
}
}  // namespace

Vec tiltAccel(int gx, int gy, int neutralX, int neutralY) {
  const float tx = (float)(gx - neutralX), ty = (float)(gy - neutralY), m = sqrtf(tx * tx + ty * ty);
  if (m <= DEAD_MG) return {0, 0};
  const float k = m >= FULL_MG ? 1 : (m - DEAD_MG) / (FULL_MG - DEAD_MG);
  return {tx / m * k * ACCEL_FULL, ty / m * k * ACCEL_FULL};
}
Vec postAt(const Level& l, int side) {
  return {(float)(side * l.goalHalf), -sqrtf((float)(PITCH_R * PITCH_R - l.goalHalf * l.goalHalf))};
}
bool inGap(const Level& l, Vec p) { return p.y < 0 && fabsf(p.x) < l.goalHalf; }
Ball start(const Level& l) { return {&l, {(float)l.startX, (float)l.startY}, {0, 0}, false}; }

void step(Ball& b, Vec accel) {
  if (b.goal) return;
  b.v = {b.v.x + accel.x * DT, b.v.y + accel.y * DT};
  roll(b);
  b.p = {b.p.x + b.v.x * DT, b.p.y + b.v.y * DT};
  const Level& l = *b.level;
  for (int i = 0; i < l.pegCount; i++) bumpOff(b, {(float)l.pegs[i].x, (float)l.pegs[i].y}, (float)l.pegs[i].r);
  bumpOff(b, postAt(l, -1), POST_R);
  bumpOff(b, postAt(l, 1), POST_R);
  bumpOff(b, {0, (float)KNOB_Y}, KNOB_R);
  rim(b);
  b.goal = inGap(l, b.p) && dot(b.p, b.p) > (float)(PITCH_R * PITCH_R);
}
}  // namespace marble
