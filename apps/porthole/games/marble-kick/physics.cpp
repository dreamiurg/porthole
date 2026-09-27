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
// Where the ball's center may be: the felt (its edge stops at the wall), and above the middle the goal's mouth, a
// channel between the posts as wide as the ball can use. Outside it, the ball goes back to the nearer of the two
// edges: rolling along the rim it flows into the mouth, with no corner at a post to park in and no jump.
float mouth(const Level& l) { return (float)(l.goalHalf - POST_R - BALL_R); }
void bounceOff(Ball& b, Vec n, float restitution) {   // n: the outward normal of the edge the ball is against
  const float vn = dot(b.v, n);
  if (vn > 0) b.v = {b.v.x - (1 + restitution) * vn * n.x, b.v.y - (1 + restitution) * vn * n.y};
}
void rim(Ball& b) {
  const float d = sqrtf(dot(b.p, b.p)), lim = PITCH_R - BALL_R, m = mouth(*b.level);
  if (d <= lim || inGap(*b.level, b.p)) return;
  const float side = b.p.x < 0 ? -1.0f : 1.0f;
  if (b.p.y < 0 && fabsf(b.p.x) - m < d - lim) {   // nearer the mouth's side than the felt's edge
    b.p.x = side * m;
    bounceOff(b, {side, 0}, RIM_BOUNCE);
    return;
  }
  const Vec n = {b.p.x / d, b.p.y / d};
  b.p = {n.x * lim, n.y * lim};
  bounceOff(b, n, RIM_BOUNCE);
}
// Felt drag and rolling friction together take this much speed; a ball they would stop, stops (never reverses).
void roll(Ball& b) {
  const float s = sqrtf(dot(b.v, b.v)), lose = s * DAMPING * DT + ROLL_DECEL * DT;
  const float keep = s <= lose ? 0 : (s - lose > V_MAX ? V_MAX : s - lose) / s;
  b.v = {b.v.x * keep, b.v.y * keep};
}
}  // namespace

Vec tiltFrom(Grav g, Grav neutral) {
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
Vec tiltAccel(Grav g, Grav neutral) {
  const Vec t = tiltFrom(g, neutral);
  const float tx = t.x, ty = t.y, m = sqrtf(tx * tx + ty * ty);
  if (m <= DEAD_MG) return {0, 0};
  const float k = m >= FULL_MG ? 1 : (m - DEAD_MG) / (FULL_MG - DEAD_MG);
  return {tx / m * k * ACCEL_FULL, ty / m * k * ACCEL_FULL};
}
Vec postAt(const Level& l, int side) {
  return {(float)(side * l.goalHalf), -sqrtf((float)(POST_RING * POST_RING - l.goalHalf * l.goalHalf))};
}
bool inGap(const Level& l, Vec p) { return p.y < 0 && fabsf(p.x) <= mouth(l); }   // its sides included
Ball start(const Level& l) { return {&l, {(float)l.startX, (float)l.startY}, {0, 0}, false}; }

void step(Ball& b, Vec accel) {
  if (b.goal) return;
  b.v = {b.v.x + accel.x * DT, b.v.y + accel.y * DT};
  roll(b);
  b.p = {b.p.x + b.v.x * DT, b.p.y + b.v.y * DT};
  const Level& l = *b.level;
  for (int i = 0; i < l.pegCount; i++) bumpOff(b, {(float)l.pegs[i].x, (float)l.pegs[i].y}, (float)l.pegs[i].r);
  bumpOff(b, {0, (float)KNOB_Y}, KNOB_R);
  rim(b);
  b.goal = inGap(l, b.p) && dot(b.p, b.p) > (float)(PITCH_R * PITCH_R);
}
}  // namespace marble
