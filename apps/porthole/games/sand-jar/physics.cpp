#include "physics.h"
#include <math.h>
#include <string.h>
#ifdef ESP_PLATFORM
#pragma GCC optimize("O2")   // the board builds -Os; the per-grain loops run ~1.5x faster at -O2
#endif

namespace sandjar {
namespace {
constexpr float SQ2 = 1.41421356f, PI = 3.14159265f;
constexpr float LEN[8] = {1, SQ2, 1, SQ2, 1, SQ2, 1, SQ2};
// The board's FPU has no divide or square root (each is a ~100-cycle library call): the per-grain paths multiply by
// these instead.
constexpr float INV_LEN[8] = {1, 1 / SQ2, 1, 1 / SQ2, 1, 1 / SQ2, 1, 1 / SQ2};
constexpr float INV_STEPS[5] = {0, 1, 1.0f / 2, 1.0f / 3, 1.0f / 4};   // travel's sub-steps (at most ceil(3.6 * F) = 3)
constexpr float F = 2.0f / 3;          // the prototype's speeds are in its 2 px grains; ours are 3 px
constexpr float GS = 0.15f * F;        // speed 1 g adds a step
constexpr float RC = G / 2 - 1.5f;     // the glass's radius in cells

// Per kind, the prototype's PARAM: share of gravity (bubbles rise), drag a step, top speed (2 px grains a step), speed
// kept along a surface after a hit, how downhill (1 = straight down, 0 = level) a slide after a hit must go, chance to
// roll down a diagonal when stopped, cells a liquid spreads along the level line, random jiggle. Nothing bounces.
struct Param { float grav, drag, vmax, fric, minDot, slide; int spread; float jit; };
constexpr Param PARAM[3] = {
  {1.0f, 0.996f, 3.4f, SAND_FRIC, 0.3f, SAND_SLIDE, 0, 0},
  {1.0f, 0.992f, 3.6f, 0.97f, -0.05f, 1.0f, 6, 0.015f},
  {-0.35f, 0.9f, 0.8f, 0, 1, 0, 0, 0.25f},
};

// This step's gravity as the grid sees it.
struct Field {
  float gx, gy, geff;          // the tilt (magnitude 0..1 g)
  float settle;                // chance a blocked grain tries to settle: grows with the tilt
  float dot[8]; int ord[8];    // cosine of each neighbor direction to gravity; directions ranked most downhill first
  int pe0[2], pe1[2]; float pfr[2];   // the level line on each side lies between pe0 and pe1: take pe1 on pfr of steps
  int down;                    // cell offset of ord[0]
};
struct Mover { int i, c; Kind k; float x, y, ux, uy; };   // a grain being moved: cell, position, velocity
struct Try { int cx, cy; float tx, ty; };                 // the cell it tried to enter, and where in it

// rnd, open and pull run per grain or body; the board's compiler kept them as calls, which cost as much as they do.
[[gnu::always_inline]] inline float rnd(Jar& j) {
  uint32_t s = j.rng; s ^= s << 13; s ^= s >> 17; s ^= s << 5; j.rng = s;
  return (float)(s >> 8) * (1.0f / 16777216);
}
uint16_t pos16(float v) { return (uint16_t)(v * 256); }
int16_t vel16(float v) { v *= 256; return (int16_t)(v > 32000 ? 32000 : v < -32000 ? -32000 : v >= 0 ? (int)(v + 0.5f) : -(int)(0.5f - v)); }
float center(int cell) { return cell + 0.5f; }
Kind kindAt(const Jar& j, int i) { return kindOf(j.g.look[i]); }
void setKind(Jar& j, int i, Kind k) { j.g.look[i] = (uint8_t)((j.g.look[i] & 0x3F) | k << 6); }

void field(Jar& j, Field& f) {
  f.gx = j.tiltX; f.gy = j.tiltY;
  const float m = sqrtf(f.gx * f.gx + f.gy * f.gy);
  f.geff = m;   // rolling and spilling scale with it: at 0 g nothing avalanches, grains only keep their momentum
  f.settle = fminf(1, m * 1.5f);
  if (m > 0.03f) {
    j.gxn = f.gx / m; j.gyn = f.gy / m;
    j.upDir = (uint8_t)(((int)lroundf(atan2f(f.gy, f.gx) / (PI / 4)) + 4) & 7);
  }
  for (int e = 0; e < 8; e++) {   // insertion sort: ties keep the grid order, so every run ranks them the same
    f.dot[e] = (DX[e] * j.gxn + DY[e] * j.gyn) / LEN[e];
    int k = e;
    for (; k > 0 && f.dot[f.ord[k - 1]] < f.dot[e]; k--) f.ord[k] = f.ord[k - 1];
    f.ord[k] = e;
  }
  for (int sd = 0; sd < 2; sd++) {   // the level line (perpendicular to gravity), one side, then the other
    const float tx = sd ? -j.gyn : j.gyn, ty = sd ? j.gxn : -j.gxn;
    const int e0 = ((int)floorf(atan2f(ty, tx) / (PI / 4)) % 8 + 8) % 8, e1 = (e0 + 1) & 7;
    const float ca = DX[e0] * ty - DY[e0] * tx, cb = DX[e1] * ty - DY[e1] * tx;
    f.pe0[sd] = e0; f.pe1[sd] = e1; f.pfr[sd] = fabsf(ca - cb) < 1e-6f ? 0 : ca / (ca - cb);
  }
  f.down = DY[f.ord[0]] * G + DX[f.ord[0]];
}

// Counting sort by height along gravity, O(n): order[] runs from the top of the pile to the bottom, and the step moves
// the bottom grains first so the ones above fall into the room they leave. The key is a quarter cell of height, in
// integers (gravity's direction to 1/1024): it runs twice over every grain each step.
struct Key { int kx, ky; };
int keyOf(const Jar& j, const Key& k, int i) {
  const int h = ((j.g.x[i] - G / 2 * 256) * k.kx + (j.g.y[i] - G / 2 * 256) * k.ky + G / 2 * 262144) >> 16;
  return h < 0 ? 0 : h >= Jar::KEYS ? Jar::KEYS - 1 : h;
}
void sortDownhill(Jar& j) {
  const Key k = {(int)lroundf(j.gxn * 1024), (int)lroundf(j.gyn * 1024)};
  memset(j.count, 0, sizeof j.count);
  for (int i = 0; i < j.n; i++) j.count[keyOf(j, k, i) + 1]++;
  for (int c = 1; c <= Jar::KEYS; c++) j.count[c] = (uint16_t)(j.count[c] + j.count[c - 1]);
  for (int i = 0; i < j.n; i++) j.g.order[j.count[keyOf(j, k, i)]++] = (uint16_t)i;
}

// The finger is a solid disc. A grain inside it is pushed out along the ray from its center (bent toward where the
// finger moves) to the first free cell outside: a pressed finger shoves sand aside and heaps it up around the rim.
bool pushAlong(Jar& j, int i, float ex, float ey) {
  const Finger& fg = j.finger;
  float x = Jar::pos(j.g.x[i]), y = Jar::pos(j.g.y[i]);
  const int from = (int)y * G + (int)x;
  for (int t = 0; t < G; t++) {   // the prototype stopped at 70: too short under a full Beach's water
    x += ex; y += ey;
    const int cx = (int)x, cy = (int)y;
    if (cx < 1 || cy < 1 || cx >= G - 1 || cy >= G - 1) return false;
    const int c = cy * G + cx, o = j.occ[c];
    if (o == WALL) return false;
    if (o != EMPTY || (x - fg.x) * (x - fg.x) + (y - fg.y) * (y - fg.y) <= FINGER_R * FINGER_R) continue;
    j.occ[from] = EMPTY; j.occ[c] = (int16_t)(i + 1);
    j.g.x[i] = pos16(center(cx)); j.g.y[i] = pos16(center(cy));
    const float kick = kindAt(j, i) == SAND ? 0.35f : 0.8f;
    j.g.vx[i] = vel16(fg.vx * kick + ex * 0.3f); j.g.vy[i] = vel16(fg.vy * kick + ey * 0.3f);
    return true;
  }
  return false;
}
// Pressed against the glass, the ray outward runs into the rim: then the grain squeezes out sideways, trying rays
// turned further and further from it (not in the prototype, where such grains stayed under the finger).
bool evict(Jar& j, int i, float dx, float dy) {
  static const float TURN[8][2] = {{1, 0}, {0.7071f, 0.7071f}, {0.7071f, -0.7071f}, {0, 1}, {0, -1},
                                   {-0.7071f, 0.7071f}, {-0.7071f, -0.7071f}, {-1, 0}};   // cos, sin
  float ex = dx + j.finger.vx * 1.5f, ey = dy + j.finger.vy * 1.5f;
  const float m = sqrtf(ex * ex + ey * ey);
  if (m > 0) { ex /= m; ey /= m; } else ex = 1;
  for (const auto& r : TURN) if (pushAlong(j, i, ex * r[0] - ey * r[1], ex * r[1] + ey * r[0])) return true;
  return false;
}
void stampFinger(Jar& j) {
  for (int q = 0; q < j.fingerCount; q++) if (j.occ[j.fingerCells[q]] == FINGER) j.occ[j.fingerCells[q]] = EMPTY;
  j.fingerCount = 0;
  if (!j.finger.on) return;
  const float R = FINGER_R, fx = j.finger.x, fy = j.finger.y;
  const int y0 = (int)(fy - R) < 1 ? 1 : (int)(fy - R), y1 = (int)(fy + R + 1) > G - 2 ? G - 2 : (int)(fy + R + 1);
  const int x0 = (int)(fx - R) < 1 ? 1 : (int)(fx - R), x1 = (int)(fx + R + 1) > G - 2 ? G - 2 : (int)(fx + R + 1);
  for (int y = y0; y <= y1; y++) for (int x = x0; x <= x1; x++) {
    const float dx = x + 0.5f - fx, dy = y + 0.5f - fy;
    const int c = y * G + x, o = j.occ[c];
    if (dx * dx + dy * dy > R * R || o == WALL || (o > 0 && !evict(j, o - 1, dx, dy))) continue;
    if (j.fingerCount < Jar::FINGER_MAX) { j.occ[c] = FINGER; j.fingerCells[j.fingerCount++] = (uint16_t)c; }
  }
}

// A bubble reached the surface: it turns back into water, and a new one starts deep down.
void popBubble(Jar& j, int i) {
  setKind(j, i, WATER);
  for (int tries = 0; tries < 6; tries++) {
    const int k = j.g.order[j.n - 1 - (int)(rnd(j) * j.n * 0.25f)];
    if (kindAt(j, k) == WATER) { setKind(j, k, BUBBLE); return; }
  }
}

void accelerate(Jar& j, const Field& f, Mover& m) {
  const Param& P = PARAM[m.k];
  const float gk = GS * P.grav;
  m.ux = (m.ux + f.gx * gk) * P.drag; m.uy = (m.uy + f.gy * gk) * P.drag;
  if (P.jit > 0) { m.ux += (rnd(j) - 0.5f) * P.jit * F; m.uy += (rnd(j) - 0.5f) * P.jit * F; }
  if (j.shaking > 0.01f) { m.ux += (rnd(j) - 0.5f) * j.shaking * F; m.uy += (rnd(j) - 0.5f) * j.shaking * F; }
  if (j.finger.on) {   // the wake: a moving finger drags what touches its rim (water a lot, sand barely)
    const float dx = m.x - j.finger.x, dy = m.y - j.finger.y, d2 = dx * dx + dy * dy, fr = FINGER_R + WAKE;
    if (d2 < fr * fr) {
      const float w = fminf(1, 1 - (sqrtf(d2) - FINGER_R) * (1 / WAKE)) * (m.k == SAND ? 0.06f : 0.3f);
      m.ux += j.finger.vx * w; m.uy += j.finger.vy * w;
    }
  }
  const float vm = P.vmax * F, sp2 = m.ux * m.ux + m.uy * m.uy;
  if (sp2 > vm * vm) { const float s = vm / sqrtf(sp2); m.ux *= s; m.uy *= s; }
}
void moveTo(Jar& j, Mover& m, int c) { j.occ[m.c] = EMPTY; j.occ[c] = (int16_t)(m.i + 1); m.c = c; }
// Where a stopped grain may go: an empty cell, or for sand, now and then, water (it sinks through, and slides under).
[[gnu::always_inline]] inline bool open(Jar& j, const Mover& m, int c) {
  const int o = j.occ[c];
  return o == EMPTY || (m.k == SAND && o > 0 && kindAt(j, o - 1) != SAND && rnd(j) < 0.3f);
}
// Into cell c; the water there, if any, takes the grain's old cell.
void take(Jar& j, Mover& m, int c) {
  const int o = j.occ[c];
  if (o <= 0) { moveTo(j, m, c); return; }
  j.occ[m.c] = (int16_t)o;
  j.g.x[o - 1] = pos16(center(m.c % G)); j.g.y[o - 1] = pos16(center(m.c / G));
  j.occ[c] = (int16_t)(m.i + 1); m.c = c;
}
// Sand sinks through water (now and then), a bubble rises through it: the two trade places.
bool swapped(Jar& j, Mover& m, const Try& t, int o) {
  const int other = o - 1;
  const Kind ko = kindAt(j, other);
  if (!((m.k == SAND && ko != SAND && rnd(j) < 0.3f) || (m.k == BUBBLE && ko == WATER))) return false;
  j.occ[m.c] = (int16_t)o;
  j.g.x[other] = pos16(center(m.c % G)); j.g.y[other] = pos16(center(m.c / G));
  j.g.vx[other] = vel16(Jar::vel(j.g.vx[other]) * 0.3f); j.g.vy[other] = vel16(Jar::vel(j.g.vy[other]) * 0.3f);
  const int nc = t.cy * G + t.cx;
  j.occ[nc] = (int16_t)(m.i + 1); m.c = nc; m.x = t.tx; m.y = t.ty;
  return true;
}
// Hit something. Slide along one axis only if that axis goes downhill enough (never up the slope), keeping fric of
// the speed along it; the blocked component is lost (no bouncing back up off a pile).
void hit(Jar& j, Mover& m, const Try& t) {
  const Param& P = PARAM[m.k];
  const int ccx = m.c % G, ccy = m.c / G, ax = t.cx - ccx, ay = t.cy - ccy;
  if (m.k != BUBBLE && ax && ax * j.gxn >= P.minDot && j.occ[ccy * G + t.cx] == EMPTY) {
    moveTo(j, m, ccy * G + t.cx); m.x = t.tx; m.ux *= P.fric; m.uy = 0;
  } else if (m.k != BUBBLE && ay && ay * j.gyn >= P.minDot && j.occ[t.cy * G + ccx] == EMPTY) {
    moveTo(j, m, t.cy * G + ccx); m.y = t.ty; m.uy *= P.fric; m.ux = 0;
  } else {
    m.ux = m.uy = 0;
  }
}
// Moves along the velocity one cell at a time. True when something stopped it.
bool travel(Jar& j, Mover& m) {
  const float ax = fabsf(m.ux), ay = fabsf(m.uy), most = ax > ay ? ax : ay;
  int steps = (int)most;
  steps += steps < most;   // ceil
  if (!steps) return true;
  const float sx = m.ux * INV_STEPS[steps], sy = m.uy * INV_STEPS[steps];
  for (int s = 0; s < steps; s++) {
    const Try t = {(int)(m.x + sx), (int)(m.y + sy), m.x + sx, m.y + sy};
    const int nc = t.cy * G + t.cx, o = j.occ[nc];
    if (nc == m.c) { m.x = t.tx; m.y = t.ty; continue; }
    if (o == EMPTY && m.k == BUBBLE) { popBubble(j, m.i); return true; }
    if (o == EMPTY) { moveTo(j, m, nc); m.x = t.tx; m.y = t.ty; continue; }
    if (o > 0 && swapped(j, m, t, o)) continue;
    hit(j, m, t);
    return true;
  }
  return false;
}
// A liquid that cannot fall walks along the level line, on a random side first, and stops early where it can fall again.
int spread(Jar& j, const Field& f, Mover& m, int cells) {
  const int s1 = rnd(j) < 0.5f ? 0 : 1;
  for (int t = 0; t < 2; t++) {
    const int sd = (s1 + t) & 1;
    int c = m.c, walked = 0, last = f.pe0[sd];
    float acc = rnd(j);
    for (int w = 0; w < cells; w++) {
      acc += f.pfr[sd];
      const int e = acc >= 1 ? f.pe1[sd] : f.pe0[sd];
      if (acc >= 1) acc -= 1;
      const int next = c + DY[e] * G + DX[e];
      if (j.occ[next] != EMPTY) break;
      c = next; walked++; last = e;
      if (j.occ[c + f.down] == EMPTY) break;
    }
    if (walked) { const float v = 0.45f * F * INV_LEN[last]; m.ux = DX[last] * v; m.uy = DY[last] * v; return c; }
  }
  return -1;
}
// Sand with no free neighbor downhill rolls over one: onto the free cell beside it (the two directions nearest
// level) and down beyond, when that ends at least ROLL_DROP lower along gravity. Not in the prototype, whose level heap
// ignored any tilt under ~65 degrees: its surface grains only ever had each other to roll into.
int rollOver(Jar& j, const Field& f, Mover& m) {
  const int first = rnd(j) < 0.5f ? 3 : 4;
  for (int t = 0; t < 2; t++) {
    const int e1 = f.ord[t ? 7 - first : first], over = m.c + DY[e1] * G + DX[e1], o = j.occ[over];
    if (o != EMPTY && (o < 0 || kindAt(j, o - 1) == SAND)) continue;   // over empty glass or water, not sand
    for (int k = 0; k < 3; k++) {
      const int e2 = f.ord[k], dx = DX[e1] + DX[e2], dy = DY[e1] + DY[e2];
      const float drop = dx * j.gxn + dy * j.gyn;
      if (drop < ROLL_DROP || (dx * dx <= 1 && dy * dy <= 1)) continue;   // a neighbor: settle() already tried it
      const int c = over + DY[e2] * G + DX[e2];
      if (open(j, m, c)) { const float v = 0.3f * F * INV_LEN[e2]; m.ux = DX[e2] * v; m.uy = DY[e2] * v; return c; }
    }
  }
  return -1;
}
// Stopped: fall straight down if that cell opened, else roll down a diagonal (with a chance: that chance is the
// angle of repose), else (sand) roll over a neighbor, else spill along the level line (liquids). Every candidate is
// ranked by its angle to real gravity, so nothing ever ends up higher.
void settle(Jar& j, const Field& f, Mover& m) {
  const Param& P = PARAM[m.k];
  const int ccx = m.c % G, ccy = m.c / G;
  const bool pick = rnd(j) < 0.5f;
  const int cand[3] = {f.ord[0], f.ord[pick ? 1 : 2], f.ord[pick ? 2 : 1]};
  int moved = -1;
  for (int e : cand) {
    if (f.dot[e] < 0.3f || (e != f.ord[0] && rnd(j) > P.slide)) continue;
    const int c = (ccy + DY[e]) * G + ccx + DX[e];
    if (open(j, m, c)) { moved = c; const float v = 0.3f * F * INV_LEN[e]; m.ux = DX[e] * v; m.uy = DY[e] * v; break; }
  }
  if (moved < 0 && m.k == SAND && rnd(j) < P.slide) moved = rollOver(j, f, m);
  if (moved < 0 && P.spread) moved = spread(j, f, m, P.spread);
  if (moved >= 0) { take(j, m, moved); m.x = center(moved % G); m.y = center(moved / G); }
}

// Every neighbor taken (for sand in Beach, by sand: it may sink into water): the grain can go nowhere this step, and has
// stopped as a hit would stop it. Most of a jar is buried, so this runs for nearly every grain every step: sand is the
// grains below j.sand (no look-up to tell it from water), and the eight reads are spelled out (the board's compiler
// kept a loop over their offsets, reloading each).
bool buried(const Jar& j, int c, bool wet) {
  const int most = wet ? j.sand : INT16_MAX;   // occ above this is a grain the sand may sink into
  const int16_t* p = j.occ + c;
  const auto room = [most](int o) { return o == EMPTY || o > most; };
  return !(room(p[-G]) || room(p[-G - 1]) || room(p[-G + 1]) || room(p[-1]) || room(p[1]) || room(p[G - 1]) ||
           room(p[G + 1]) || room(p[G]));
}

void moveGrain(Jar& j, const Field& f, int i) {
  const Kind k = kindAt(j, i);
  const int c = (j.g.y[i] >> 8) * G + (j.g.x[i] >> 8);
  if (k != BUBBLE && buried(j, c, k == SAND && j.mode == MODE_BEACH)) { j.g.vx[i] = j.g.vy[i] = 0; return; }
  Mover m = {i, c, k, Jar::pos(j.g.x[i]), Jar::pos(j.g.y[i]), Jar::vel(j.g.vx[i]), Jar::vel(j.g.vy[i])};
  const int start = m.c;
  accelerate(j, f, m);
  const bool blocked = travel(j, m) || m.c == start;
  if (blocked && f.geff > 0.02f && m.k != BUBBLE && rnd(j) < f.settle) settle(j, f, m);
  j.g.x[i] = pos16(m.x); j.g.y[i] = pos16(m.y); j.g.vx[i] = vel16(m.ux); j.g.vy[i] = vel16(m.uy);
}

// Orbits: free bodies in panel px, pulled by a dim sun at the center and, while touched, a heavy star under the finger.
[[gnu::always_inline]] inline void pull(float& ax, float& ay, float dx, float dy, float gm) {
  const float d2 = dx * dx + dy * dy + 16 * 16, r = invSqrt(d2), a = gm * r * r * r;   // softened within 16 px
  ax += dx * a; ay += dy * a;
}
void orbit(Jar& j, Body& b) {
  float ax = j.tiltX * 0.035f, ay = j.tiltY * 0.035f;   // tilt drifts every orbit the same way, like a wind
  pull(ax, ay, 240 - b.x, 240 - b.y, ORB_SUN);
  if (j.finger.on) pull(ax, ay, j.finger.x * 3 - b.x, j.finger.y * 3 - b.y, ORB_STAR);
  if (j.shaking > 0.01f) { ax += (rnd(j) - 0.5f) * j.shaking * 0.5f; ay += (rnd(j) - 0.5f) * j.shaking * 0.5f; }
  float ux = (b.vx + ax * 0.5f) * 0.9995f, uy = (b.vy + ay * 0.5f) * 0.9995f;
  const float sp2 = ux * ux + uy * uy;
  if (sp2 > 81) { const float s = 9 / sqrtf(sp2); ux *= s; uy *= s; }
  float x = b.x + ux * 0.5f, y = b.y + uy * 0.5f;
  const float rx = x - 240, ry = y - 240, r2 = rx * rx + ry * ry;
  if (r2 > 232 * 232) {   // bounce off the glass, losing a little
    const float r = sqrtf(r2), nx = rx / r, ny = ry / r, vn = ux * nx + uy * ny;
    if (vn > 0) { ux -= 1.7f * vn * nx; uy -= 1.7f * vn * ny; }
    x = 240 + nx * 232; y = 240 + ny * 232;
  }
  b.x = x; b.y = y; b.vx = ux; b.vy = uy;
}

// ---- filling a fresh jar
void walls(Jar& j) {
  j.inside = 0;
  for (int y = 0; y < G; y++) for (int x = 0; x < G; x++) {
    const float dx = x + 0.5f - G / 2, dy = y + 0.5f - G / 2;
    const bool in = sqrtf(dx * dx + dy * dy) <= RC;
    j.occ[y * G + x] = in ? EMPTY : WALL;
    j.inside += in;
  }
}
// What the i-th grain from the bottom is: Rainbow's six strata bottom to top, Beach's sand under its water.
uint8_t lookFor(Jar& j, int i) {
  static const float STRATA[6] = {1.25f, 0.8f, 1.1f, 0.9f, 1.2f, 0.75f};   // share of each band (of their 6.0 sum)
  const int tone = (int)(rnd(j) * 8), shade = (int)(rnd(j) * 6);
  if (j.mode == MODE_RAINBOW) {
    int band = 0, end = 0;
    for (; band < 5; band++) if (i < (end += (int)lroundf(j.n * STRATA[band] / 6.0f))) break;
    return makeLook(SAND, band, tone);
  }
  if (i >= j.sand) return makeLook(rnd(j) < 0.008f ? BUBBLE : WATER, 0, tone);
  return makeLook(SAND, shade, tone);
}
// Row by row from the bottom edge (the one gravity points at most), each row in random order: a level heap.
void fill(Jar& j) {
  const bool vertical = fabsf(j.gyn) >= fabsf(j.gxn);
  const bool fromEnd = vertical ? j.gyn >= 0 : j.gxn >= 0;
  int placed = 0;
  for (int r = 0; r < G && placed < j.n; r++) {
    const int row = fromEnd ? G - 1 - r : r;
    uint16_t cells[G]; int m = 0;
    for (int a = 0; a < G; a++) {
      const int c = vertical ? row * G + a : a * G + row;
      if (j.occ[c] == EMPTY) cells[m++] = (uint16_t)c;
    }
    for (int q = m - 1; q > 0; q--) { const int s = (int)(rnd(j) * (q + 1)); const uint16_t t = cells[q]; cells[q] = cells[s]; cells[s] = t; }
    for (int q = 0; q < m && placed < j.n; q++, placed++) {
      const int c = cells[q];
      j.g.x[placed] = pos16(center(c % G)); j.g.y[placed] = pos16(center(c / G)); j.g.vx[placed] = j.g.vy[placed] = 0;
      j.g.look[placed] = lookFor(j, placed); j.occ[c] = (int16_t)(placed + 1);
    }
  }
}
// A mostly shared spin, like a little galaxy.
void launch(Jar& j) {
  for (int i = 0; i < j.n; i++) {
    Body& b = j.sky.body[i];
    const float r = 30 + sqrtf(rnd(j)) * 190, a = rnd(j) * 2 * PI;
    const float v = sqrtf(ORB_SUN / r) * (0.85f + rnd(j) * 0.3f) * (rnd(j) < 0.85f ? 1 : -1);
    b.x = 240 + cosf(a) * r; b.y = 240 + sinf(a) * r;
    b.vx = -sinf(a) * v; b.vy = cosf(a) * v;
    b.shade = (uint8_t)(rnd(j) * 5); b.size = 0.35f + rnd(j) * rnd(j) * 1.6f;
  }
}
}  // namespace

void Jar::reset(Mode m, uint32_t seed) {
  mode = m; rng = seed ? seed : 1; frame = 0; shaking = 0; finger = {}; fingerCount = 0;
  const float g = sqrtf(tiltX * tiltX + tiltY * tiltY);
  if (g > 0.03f) { gxn = tiltX / g; gyn = tiltY / g; }
  walls(*this);
  const int want = inside * FILL_PCT[m] / 100, most = m == MODE_ORBITS ? MAX_BODIES : MAX_GRAINS;
  n = want < most ? want : most;
  sand = m == MODE_BEACH ? (int)lroundf(n * BEACH_SAND) : n;
  if (m == MODE_ORBITS) launch(*this); else fill(*this);
}

void Jar::step() {
  frame++;
  if (mode == MODE_ORBITS) {
    for (int sub = 0; sub < 2; sub++) for (int i = 0; i < n; i++) orbit(*this, sky.body[i]);
  } else {
    stampFinger(*this);
    Field f;
    field(*this, f);
    sortDownhill(*this);
    for (int q = n - 1; q >= 0; q--) moveGrain(*this, f, g.order[q]);
  }
  shaking *= SHAKE_FADE;
}
}  // namespace sandjar
