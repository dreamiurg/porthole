#include "physics.h"
#include <math.h>

namespace marble {
namespace {
constexpr float DT = STEP_MS / 1000.0f, PI = 3.14159265f;
float dot(Vec a, Vec b) { return a.x * b.x + a.y * b.y; }
Vec rotate(Vec p, float a) {   // clockwise on the screen (+y down) by a radians
  const float c = cosf(a), s = sinf(a);
  return {p.x * c - p.y * s, p.x * s + p.y * c};
}
float ease(uint32_t ms, uint16_t periodMs) { return periodMs ? (1 - cosf(2 * PI * (float)(ms % periodMs) / periodMs)) / 2 : 0; }

// Keeps the ball out of a circle moving at velocity u: pushed back to touching, and its speed into the circle, relative
// to the circle's own, reflected. A moving peg hits back with its own velocity: a nudge.
void bumpOff(Ball& b, Vec c, float r, Vec u) {
  const Vec d = {b.p.x - c.x, b.p.y - c.y};
  const float touch = r + BALL_R, d2 = dot(d, d);
  if (d2 >= touch * touch) return;
  const float dist = sqrtf(d2);
  const Vec n = dist > 0.001f ? Vec{d.x / dist, d.y / dist} : Vec{0, -1};
  b.p = {c.x + n.x * touch, c.y + n.y * touch};
  const float vn = dot({b.v.x - u.x, b.v.y - u.y}, n);
  if (vn < 0) b.v = {b.v.x - (1 + PEG_BOUNCE) * vn * n.x, b.v.y - (1 + PEG_BOUNCE) * vn * n.y};
}
// A rail is a capsule: the ball bounces off the nearest point of its middle line, RAIL_R + BALL_R away.
void bumpRail(Ball& b, const Rail& r) {
  const Vec a = {(float)r.x0, (float)r.y0}, ab = {(float)(r.x1 - r.x0), (float)(r.y1 - r.y0)};
  const float len2 = dot(ab, ab), t = len2 > 0 ? dot({b.p.x - a.x, b.p.y - a.y}, ab) / len2 : 0;
  const float k = t < 0 ? 0 : t > 1 ? 1 : t;
  bumpOff(b, {a.x + ab.x * k, a.y + ab.y * k}, RAIL_R, {0, 0});
}
// Where the ball's center may be: the felt (its edge stops at the wall), and the goal's mouth, a channel toward the
// goal as wide as the ball can use between the posts. Outside it, the ball goes back to the nearer of the two edges:
// rolling along the rim it flows into the mouth, with no corner at a post to park in and no jump. Worked out in the
// goal's own frame (its mouth at the top), wherever the goal is on the rim now.
float mouth(const Level& l) { return (float)(l.goalHalf - POST_R - BALL_R); }
void bounceOff(Ball& b, Vec n, float restitution) {   // n: the outward normal of the edge the ball is against
  const float vn = dot(b.v, n);
  if (vn > 0) b.v = {b.v.x - (1 + restitution) * vn * n.x, b.v.y - (1 + restitution) * vn * n.y};
}
void rim(Ball& b) {
  const float d = sqrtf(dot(b.p, b.p)), lim = PITCH_R - BALL_R, m = mouth(*b.level);
  if (d <= lim || inGap(*b.level, b.p, b.ms)) return;
  const float a = goalAngle(*b.level, b.ms);
  const Vec q = rotate(b.p, -a);   // the goal's frame
  if (q.y < 0 && fabsf(q.x) - m < d - lim) {   // nearer the mouth's side than the felt's edge
    const float side = q.x < 0 ? -1.0f : 1.0f;
    b.p = rotate({side * m, q.y}, a);
    bounceOff(b, rotate({side, 0}, a), RIM_BOUNCE);
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
void obstacles(Ball& b) {
  const Level& l = *b.level;
  for (int i = 0; i < l.pegCount; i++) {
    const Vec c = pegAt(l, l.pegs[i], b.ms), next = pegAt(l, l.pegs[i], b.ms + STEP_MS);
    bumpOff(b, c, (float)l.pegs[i].r, {(next.x - c.x) / DT, (next.y - c.y) / DT});
  }
  for (int i = 0; i < l.railCount; i++) bumpRail(b, l.rails[i]);
  bumpOff(b, {0, (float)KNOB_Y}, KNOB_R, {0, 0});
  rim(b);
}
// Stars under the ball are picked up; a slow ball over a hole drops in.
void pickUp(Ball& b) {
  const Level& l = *b.level;
  for (int i = 0; i < STARS; i++) {
    const float dx = b.p.x - l.stars[i].x, dy = b.p.y - l.stars[i].y, r = STAR_R + BALL_R;
    if (dx * dx + dy * dy < r * r) b.stars |= (uint8_t)(1 << i);
  }
  if (dot(b.v, b.v) >= HOLE_SKIM * HOLE_SKIM) return;
  for (int i = 0; i < l.holeCount; i++) {
    const float dx = b.p.x - l.holes[i].x, dy = b.p.y - l.holes[i].y, r = (float)(l.holes[i].r - HOLE_IN);
    if (dx * dx + dy * dy < r * r) { b.sinkMs = STEP_MS; b.hole = {(float)l.holes[i].x, (float)l.holes[i].y}; b.v = {0, 0}; return; }
  }
}
// In a hole: the ball settles into its middle as it sinks, then comes back at the start, still.
void sink(Ball& b) {
  b.sinkMs = (uint16_t)(b.sinkMs + STEP_MS);
  b.p = {b.p.x + (b.hole.x - b.p.x) * 0.1f, b.p.y + (b.hole.y - b.p.y) * 0.1f};
  if (b.sinkMs < RESPAWN_MS) return;
  b.sinkMs = 0;
  b.p = {(float)b.level->startX, (float)b.level->startY};
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

float goalAngle(const Level& l, uint32_t ms) {
  const GoalMove& g = l.goal;
  float deg = g.a0;
  if (g.mode == SWING) deg = g.a0 + (g.a1 - g.a0) * ease(ms, g.periodMs);
  if (g.mode == SPIN) {   // a1 degrees a second, turning back every periodMs
    const uint32_t P = g.periodMs, t = P ? ms % (2 * P) : ms;
    deg = g.a0 + g.a1 * (float)(P && t > P ? 2 * P - t : t) / 1000;
  }
  return deg * PI / 180;
}
Vec pegAt(const Level& l, const Peg& p, uint32_t ms) {
  const float k = p.kind == STILL ? 0 : ease(ms, p.periodMs);
  const Vec at = {p.x + (p.bx - p.x) * k, p.y + (p.by - p.y) * k};
  return p.kind == KEEPER ? rotate(at, goalAngle(l, ms)) : at;
}
Vec postAt(const Level& l, int side, uint32_t ms) {
  const Vec top = {(float)(side * l.goalHalf), -sqrtf((float)(POST_RING * POST_RING - l.goalHalf * l.goalHalf))};
  return rotate(top, goalAngle(l, ms));
}
Vec mouthAt(const Level& l, uint32_t ms) { return rotate({0, (float)-PITCH_R}, goalAngle(l, ms)); }
bool inGap(const Level& l, Vec p, uint32_t ms) {   // its sides included
  const Vec q = rotate(p, -goalAngle(l, ms));
  return q.y < 0 && fabsf(q.x) <= mouth(l);
}
Ball start(const Level& l) { return {&l, {(float)l.startX, (float)l.startY}, {0, 0}, false, false, 0, 0, 0, {0, 0}}; }
int starCount(uint8_t bits) { return (bits & 1) + (bits >> 1 & 1) + (bits >> 2 & 1); }

void step(Ball& b, Vec accel) {
  if (b.goal) return;
  b.started = b.started || accel.x != 0 || accel.y != 0;
  if (b.started) b.ms += STEP_MS;
  if (sinking(b)) { sink(b); return; }
  b.v = {b.v.x + accel.x * DT, b.v.y + accel.y * DT};
  roll(b);
  b.p = {b.p.x + b.v.x * DT, b.p.y + b.v.y * DT};
  obstacles(b);
  pickUp(b);
  b.goal = !sinking(b) && inGap(*b.level, b.p, b.ms) && dot(b.p, b.p) > (float)(PITCH_R * PITCH_R);
}
}  // namespace marble
