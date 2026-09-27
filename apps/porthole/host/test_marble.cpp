// Self-check for Marble Kick (games/marble-kick/): the tilt mapping, the ball staying on the tray except through the
// goal, no tunneling through a peg, no pockets or parking spots, goal detection, the save blob, every level's recorded
// solution replayed to a goal from three grips (and kept in step with the playtests), and the renderer's shortcut:
// every frame drawn incrementally into the panel's two buffers equals the frame painted whole.
// Run: make test
#include <assert.h>
#include <dirent.h>
#include <math.h>
#include <stdio.h>
#include <string.h>
#include <string>
#include "games/marble-kick/game.h"
#include "games/marble-kick/physics.h"
#include "games/marble-kick/save.h"

using namespace marble;
static int checks = 0;
static bool lab = false;   // --levels: report every level's checks, fail none (for designing levels)
#define CHECK(c) (lab || (assert(c), true), checks++)

static float len(Vec v) { return sqrtf(v.x * v.x + v.y * v.y); }
static bool near(float a, float b) { return fabsf(a - b) < 0.01f; }

// The ways a kid holds it: lying flat, leaned back 45 degrees, upright (the sim's default hold), and upside down.
static const Grav FLAT = {0, 0, -1000}, LEANED = {0, 707, -707}, UPRIGHT = {0, 1000, 0}, FACE_DOWN = {0, 0, 1000};
static const Grav NEUTRALS[] = {FLAT, LEANED, UPRIGHT};
static float mag(Grav g) { return sqrtf((float)(g.x * g.x + g.y * g.y + g.z * g.z)); }

// The real gravity (1 g, whole milli-g, as the sensor reports it) when the device is tilted by (tx, ty) milli-g away
// from `neutral`: the tilt as it would be lying flat, turned back by the rotation that lays the neutral flat.
static Grav gravityFor(int tx, int ty, Grav neutral) {
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

static void tiltFromGrip(Grav n) {
  // holding the device the way it was calibrated is no tilt at all
  CHECK(near(len(tiltAccel(n, n)), 0));
  // every test input is a real 1 g reading
  for (int t = -900; t <= 900; t += 150) {
    CHECK(fabsf(mag(gravityFor(t, 0, n)) - 1000) < 2 && fabsf(mag(gravityFor(0, t, n)) - 1000) < 2);
  }
  // the dead zone: up to DEAD_MG away from neutral, in any direction, nothing
  CHECK(near(len(tiltAccel(gravityFor(DEAD_MG - 3, 0, n), n)), 0));
  CHECK(near(len(tiltAccel(gravityFor(-58, 58, n), n)), 0));   // 82 mg diagonally
  // past it, it grows with the tilt, in the tilt's direction, the same both ways (leaning forward or back)
  const int mid = (DEAD_MG + FULL_MG) / 2;
  const Vec up = tiltAccel(gravityFor(0, -mid, n), n), down = tiltAccel(gravityFor(0, mid, n), n);
  CHECK(fabsf(up.x) < 3 && up.y < 0 && fabsf(-up.y - ACCEL_FULL / 2) < 3);
  CHECK(fabsf(down.x) < 3 && fabsf(down.y + up.y) < 3);
  const Vec right = tiltAccel(gravityFor(200, 0, n), n);
  CHECK(right.x > 0 && fabsf(right.y) < 3 && right.x < ACCEL_FULL);
  // clamped at full tilt: twice as far is no faster
  CHECK(fabsf(len(tiltAccel(gravityFor(FULL_MG + 5, 0, n), n)) - ACCEL_FULL) < 0.5f);
  const Vec d = tiltAccel(gravityFor(-700, 700, n), n);
  CHECK(near(len(d), ACCEL_FULL) && fabsf(d.x + d.y) < 3);
}
static void tiltAsHeld() {
  // held upright, the way the kid moves it: leaning back rolls the ball up, leaning forward rolls it down, turning it
  // like a steering wheel rolls it sideways (30 and 20 degrees: full tilt, well past the dead zone)
  CHECK(tiltAccel({0, 866, -500}, UPRIGHT).y < -ACCEL_FULL + 1 && tiltAccel({0, 866, 500}, UPRIGHT).y > ACCEL_FULL - 1);
  const Vec wheel = tiltAccel({342, 940, 0}, UPRIGHT);
  CHECK(wheel.x > 0 && fabsf(wheel.y) < 0.5f * wheel.x);
  // Leaned back 45 degrees, checked with plain rotations rather than gravityFor (which is tiltFrom's own inverse):
  // gravity in the device's frame is (0, cos a, -sin a) leaned back by a from upright, so tipping the top edge 20
  // degrees further away rolls the ball up and 20 degrees back toward you rolls it down; turning about the screen's
  // up axis so the right edge drops 20 degrees, gravity (0, 707, -707) becomes (707 sin 20, 707, -707 cos 20), and
  // the ball rolls right.
  const float deg = 3.14159265f / 180;
  const Grav away = {0, (int)lroundf(1000 * cosf(65 * deg)), (int)lroundf(-1000 * sinf(65 * deg))};
  const Grav toward = {0, (int)lroundf(1000 * cosf(25 * deg)), (int)lroundf(-1000 * sinf(25 * deg))};
  const Grav rightDown = {(int)lroundf(707 * sinf(20 * deg)), 707, (int)lroundf(-707 * cosf(20 * deg))};
  // 20 degrees from the grip is a 1000 sin 20 = 342 mg tilt; the turn, about an axis leaned 45 degrees, 707 sin 20 = 242
  const Vec up = tiltFrom(away, LEANED), down = tiltFrom(toward, LEANED), right = tiltFrom(rightDown, LEANED);
  CHECK(fabsf(up.y + 342) < 3 && fabsf(up.x) < 3 && fabsf(down.y - 342) < 3 && fabsf(down.x) < 3);
  CHECK(fabsf(right.x - 242) < 3 && fabsf(right.y) < 0.2f * right.x);
  CHECK(tiltAccel(away, LEANED).y < 0 && tiltAccel(toward, LEANED).y > 0 && tiltAccel(rightDown, LEANED).x > 0);
}
static void tiltMapping() {
  static const Grav HOLDS[] = {FLAT, LEANED, UPRIGHT, FACE_DOWN, {500, 500, -707}};
  for (Grav n : HOLDS) tiltFromGrip(n);
  tiltAsHeld();
}

// A made-up level for the rules: one post-sized peg (the thinnest thing on any tray) in the middle.
static const Level PIN = {0, 120, 60, TOP, 1, {peg(0, 0, POST_R)}, 0, {}, 0, {}, {}, nullptr, 0};

static bool onTray(const Ball& b) {
  const float d = len(b.p);
  return d <= PITCH_R - BALL_R + 0.01f || (inGap(*b.level, b.p, b.ms) && d <= PITCH_R);
}

// Full tilt in a direction that turns a little every quarter second, for a minute, from every level: the ball stays on
// the tray at every substep, or it scored through the gap (and then it starts again). The shots at the wall below
// score exactly when aimed at the gap.
static void neverLeavesTheTray() {
  for (const Level& l : LEVELS) {
    Ball b = start(l);
    for (int i = 0; i < 12000; i++) {
      const float angle = (float)(i / 50) * 2.4f;
      step(b, {cosf(angle) * ACCEL_FULL, sinf(angle) * ACCEL_FULL});
      if (b.goal) { CHECK(inGap(l, b.p, b.ms) && len(b.p) > PITCH_R); b = start(l); continue; }
      assert(onTray(b) && len(b.v) <= V_MAX + 0.01f);
    }
    checks++;
  }
  // straight at the wall at top speed, beside the gap and below it: bounced back, never out
  for (float angle = 0; angle < 6.28f; angle += 0.05f) {
    Ball b = start(PIN);
    b.p = {cosf(angle) * 150, sinf(angle) * 150};
    b.v = {cosf(angle) * V_MAX, sinf(angle) * V_MAX};
    for (int i = 0; i < 200 && !b.goal; i++) { step(b, {cosf(angle) * ACCEL_FULL, sinf(angle) * ACCEL_FULL}); assert(b.goal || onTray(b)); }
    CHECK(b.goal == inGap(PIN, b.p, b.ms));
  }
}

// At top speed, pushed on at full tilt, the ball meets the thinnest peg from every side and never ends up past it.
static void noTunneling() {
  for (float angle = 0; angle < 6.28f; angle += 0.1f) {
    const Vec dir = {cosf(angle), sinf(angle)};
    Ball b = start(PIN);
    b.p = {dir.x * 60, dir.y * 60};
    b.v = {-dir.x * V_MAX, -dir.y * V_MAX};
    for (int i = 0; i < 400; i++) {
      step(b, {-dir.x * ACCEL_FULL, -dir.y * ACCEL_FULL});
      assert(len(b.p) >= POST_R + BALL_R - 0.01f);            // never inside the peg
      assert(b.p.x * dir.x + b.p.y * dir.y > 0);              // never past it: it slides round, it does not go through
    }
    checks++;
  }
}

static void goalDetection() {
  // rolling straight up the middle: a goal once the center crosses the rim, and then the ball stays put
  Ball b = start(PIN);
  b.p = {30, 60};   // clear of the pin in the middle
  int n = 0;
  while (!b.goal && n < 2000) { step(b, {0, -ACCEL_FULL}); n++; }
  CHECK(b.goal && len(b.p) > PITCH_R && inGap(PIN, b.p, b.ms));
  const Vec at = b.p;
  step(b, {ACCEL_FULL, 0});
  CHECK(b.p.x == at.x && b.p.y == at.y);
  // at the bottom, where the rim is closed, never
  b = start(PIN);
  for (int i = 0; i < 2000; i++) step(b, {0, ACCEL_FULL});
  CHECK(!b.goal && b.p.y > 0);
  // the gap is the mouth between the posts, where the whole ball fits: at the top, the ball's center clear of each post
  const float mouth = PIN.goalHalf - POST_R - BALL_R;
  CHECK(inGap(PIN, {0, -180}, 0) && inGap(PIN, {-(mouth - 1), -170}, 0) && !inGap(PIN, {mouth + 1, -170}, 0) && !inGap(PIN, {0, 180}, 0));
  CHECK(near(len(postAt(PIN, -1, 0)), POST_RING) && near(postAt(PIN, 1, 0).x, PIN.goalHalf) && postAt(PIN, -1, 0).y < 0);
  // a ball resting in the dead zone stays where it is: nothing drifts it on its own
  b = start(LEVELS[0]);
  for (int i = 0; i < 1000; i++) step(b, tiltAccel(gravityFor(40, -40, UPRIGHT), UPRIGHT));
  CHECK(b.p.x == LEVELS[0].startX && b.p.y == LEVELS[0].startY);
}

// No gap the size of the ball, where it would wedge: between still pegs, rails, the home knob and the rim, every gap is
// either narrower than the ball (a wedge it cannot enter; neverStuck below shows it always backs out) or wide enough to
// roll through with room to spare. Moving pegs pass; they are not checked here.
struct Shape { Vec a, b; float r; };   // a capsule; a circle has a == b
static float segDist(Vec p, Vec a, Vec b) {
  const Vec ab = {b.x - a.x, b.y - a.y};
  const float l2 = ab.x * ab.x + ab.y * ab.y, t = l2 > 0 ? ((p.x - a.x) * ab.x + (p.y - a.y) * ab.y) / l2 : 0;
  const float k = t < 0 ? 0 : t > 1 ? 1 : t;
  return len({p.x - a.x - ab.x * k, p.y - a.y - ab.y * k});
}
static float gap(const Shape& s, const Shape& t) {   // sampled along s every px: plenty for a rule about 40 px gaps
  const float n = fmaxf(1, len({s.b.x - s.a.x, s.b.y - s.a.y}));
  float best = 1e9f;
  for (float k = 0; k <= n; k++) {
    const Vec p = {s.a.x + (s.b.x - s.a.x) * k / n, s.a.y + (s.b.y - s.a.y) * k / n};
    best = fminf(best, segDist(p, t.a, t.b));
  }
  return best - s.r - t.r;
}
static int shapes(const Level& l, Shape* out) {
  int n = 0;
  for (int i = 0; i < l.pegCount; i++)
    if (l.pegs[i].kind == STILL) { const Vec c = {(float)l.pegs[i].x, (float)l.pegs[i].y}; out[n++] = {c, c, (float)l.pegs[i].r}; }
  for (int i = 0; i < l.railCount; i++)
    out[n++] = {{(float)l.rails[i].x0, (float)l.rails[i].y0}, {(float)l.rails[i].x1, (float)l.rails[i].y1}, RAIL_R};
  out[n++] = {{0, KNOB_Y}, {0, KNOB_Y}, KNOB_R};
  return n;
}
static void noBallSizedGaps() {
  constexpr float WEDGE = 2 * BALL_R - 2, OPEN = 2 * BALL_R + 8;
  int bad = 0;
  for (const Level& l : LEVELS) {
    Shape sh[MAX_PEGS + MAX_RAILS + 1];
    const int n = shapes(l, sh);
    for (int i = 0; i < n; i++) {
      const float rim = PITCH_R - fmaxf(len(sh[i].a), len(sh[i].b)) - sh[i].r;
      if (rim >= WEDGE && rim < OPEN) { printf("level %d: shape %d is %.1f px from the rim\n", (int)(&l - LEVELS) + 1, i, rim); bad++; }
      for (int j = i + 1; j < n; j++) {
        const float g = gap(sh[i], sh[j]);
        if (g >= WEDGE && g < OPEN) { printf("level %d: shapes %d and %d are %.1f px apart\n", (int)(&l - LEVELS) + 1, i, j, g); bad++; }
      }
    }
  }
  CHECK(bad == 0);
}
// The goalposts sit in the wall, clear of the lane a ball rolls along the rim in: a ball pressed to the rim slides
// past them.
// Every goal stays clear of the level coin on the rim at the lower right (about 131 degrees): the net never slides under it.
static void goalsClearOfTheCoin() {
  for (const Level& l : LEVELS)
    for (uint32_t ms = 0; ms < 60000; ms += 100) CHECK(fabsf(goalAngle(l, ms) * 180 / 3.14159265f) < 90);
}
static void postsClearOfTheRimLane() {
  for (const Level& l : LEVELS)
    for (uint32_t ms = 0; ms < 20000; ms += 1250)
      for (int side = -1; side <= 1; side += 2) CHECK(len(postAt(l, side, ms)) - POST_R >= PITCH_R - 0.01f);
}

// A ball pressed to the rim on level 1 slides up it into the goal (it used to park on a goalpost).
static int holdTilt(const Level& l, Vec from, Grav g, Grav neutral, int ms) {
  Ball b = start(l);
  b.p = from;
  const Vec a = tiltAccel(g, neutral);
  for (int t = 0; t < ms; t += STEP_MS) { step(b, a); if (b.goal) return t; }
  return -1;
}
static void noParking() {
  for (int side = -1; side <= 1; side += 2)
    CHECK(holdTilt(LEVELS[0], {side * (PITCH_R - BALL_R - 1.0f), 0}, gravityFor(0, -FULL_MG, FLAT), FLAT, 3000) >= 0);
}

// ---- the level elements, one made-up level each ----
// A rail stops the ball from every side at top speed, and never lets it through.
static const Level BAR = {0, 120, 60, TOP, 0, {}, 1, {{-60, 0, 60, 0}}, 0, {}, {}, nullptr, 0};
static void railsHold() {
  for (float angle = 0; angle < 6.28f; angle += 0.1f) {
    const Vec dir = {cosf(angle), sinf(angle)};
    if (fabsf(dir.y) < 0.5f) continue;   // shots along the bar round its end, rightly
    Ball b = start(BAR);
    b.p = {dir.x * 70, dir.y * 70};
    b.v = {-dir.x * V_MAX, -dir.y * V_MAX};
    for (int i = 0; i < 400; i++) {
      const Vec was = b.p;
      step(b, {-dir.x * ACCEL_FULL, -dir.y * ACCEL_FULL});
      assert(segDist(b.p, {-60, 0}, {60, 0}) >= RAIL_R + BALL_R - 0.01f);          // never inside the bar
      assert(!(was.y * b.p.y <= 0 && fabsf(was.x) <= 60 && fabsf(b.p.x) <= 60));   // never across it where it is
    }
    checks++;
  }
}
// Nothing moves until the kid tilts; then a defender walks into a resting ball and pushes it along, the same way
// every time.
static const Level WALKER = {0, 60, 60, TOP, 1, {defender(-100, 0, 100, 0, 4000)}, 0, {}, 0, {}, {}, nullptr, 0};
static void defendersNudge() {
  Ball b = start(WALKER);
  b.p = {0, 0};
  for (int i = 0; i < 400; i++) step(b, {0, 0});
  CHECK(!b.started && b.ms == 0 && pegAt(WALKER, WALKER.pegs[0], b.ms).x == -100);
  step(b, {0, 50});   // a tilt just past the dead zone: too gentle to beat the felt, but the clock starts
  CHECK(b.started && b.ms == STEP_MS);
  float before = 0, after = 0;
  Ball twin = b;
  for (int i = 0; i < 400; i++) { step(b, {0, 0}); step(twin, {0, 0}); }   // 2 s: across the middle to the far end
  before = b.p.x; after = twin.p.x;
  CHECK(before > 5 && before == after);   // pushed right, identically
  CHECK(len(pegAt(WALKER, WALKER.pegs[0], 1000)) < 1 && near(pegAt(WALKER, WALKER.pegs[0], 2000).x, 100));
}
// A slow ball over a hole drops in, sinks, and comes back at the start, still, its stars kept; a fast one skims over.
static const Level PIT = {0, 120, 60, TOP, 0, {}, 0, {}, 1, {{0, 0, 24}}, {{0, 60}, {100, 100}, {-100, 100}}, nullptr, 0};
static void holesSendBack() {
  Ball b = start(PIT);
  b.p = {0, 90};
  int t = 0;
  while (!sinking(b) && t < 3000) { step(b, {0, -150}); t += STEP_MS; }
  CHECK(sinking(b) && (b.stars & 1) && len(b.p) < 24);
  while (sinking(b)) { step(b, {0, -ACCEL_FULL}); t += STEP_MS; }
  CHECK(b.p.x == PIT.startX && b.p.y == PIT.startY && b.v.x == 0 && b.v.y == 0 && (b.stars & 1));
  b = start(PIT);
  b.p = {0, 60};
  b.v = {0, -V_MAX};
  for (int i = 0; i < 30; i++) step(b, {0, -ACCEL_FULL});
  CHECK(!sinking(b) && b.p.y < -20);   // skimmed over at top speed
  CHECK(starCount(7) == 3 && starCount(5) == 2 && starCount(0) == 0);
}
// The goal moves round the rim on the level's clock, its posts and the keeper with it; a ball can only score through
// the mouth where it is now.
static const Level TURN = {0, 120, 60, {SPIN, 0, 30, 0}, 1, {keeper(-150, 40, 3000)}, 0, {}, 0, {}, {}, nullptr, 0};
static const Level SWAY = {0, 120, 60, {SWING, -40, 40, 8000}, 0, {}, 0, {}, 0, {}, {}, nullptr, 0};
static void goalMoves() {
  const float deg = 3.14159265f / 180;
  CHECK(near(goalAngle(TURN, 0), 0) && near(goalAngle(TURN, 3000), 90 * deg));
  CHECK(near(goalAngle(SWAY, 0), -40 * deg) && near(goalAngle(SWAY, 4000), 40 * deg) && near(goalAngle(SWAY, 8000), -40 * deg));
  const Vec m = mouthAt(TURN, 3000), post = postAt(TURN, -1, 3000), k = pegAt(TURN, TURN.pegs[0], 3000);
  CHECK(fabsf(m.x - PITCH_R) < 0.1f && fabsf(m.y) < 0.1f);   // at 3 o'clock
  CHECK(near(len(post), POST_RING) && post.x > 0 && k.x > 100);
  CHECK(inGap(TURN, {180, 0}, 3000) && !inGap(TURN, {0, -180}, 3000));
  // held still at 3 o'clock, the mouth comes round and the ball rolls into it... only when it is there
  Ball b = start(SWAY);
  b.p = {0, 0};
  int t = 0;
  while (!b.goal && t < 20000) { const Vec at = mouthAt(SWAY, b.ms); const float d = len({at.x - b.p.x, at.y - b.p.y});
    step(b, {(at.x - b.p.x) / d * ACCEL_FULL, (at.y - b.p.y) / d * ACCEL_FULL}); t += STEP_MS; }
  CHECK(b.goal && inGap(SWAY, b.p, b.ms));
}

// ---- challenge ----
// The greedy player: always full tilt straight at the goal's mouth, wherever it is now, re-aimed every frame, from
// each grip. It may win the first two levels (they teach tilting); from level 3 on it must not within 20 s: every
// level asks for a route, some patience, or both.
static bool greedyScores(const Level& l, Grav grip, int ms) {
  Ball b = start(l);
  for (int t = 0; t < ms; t += 40) {
    const Vec at = mouthAt(l, b.ms);
    const float d = fmaxf(1, len({at.x - b.p.x, at.y - b.p.y}));
    const Vec a = tiltAccel(gravityFor((int)lroundf((at.x - b.p.x) / d * 600), (int)lroundf((at.y - b.p.y) / d * 600), grip), grip);
    for (int k = 0; k < 8; k++) { step(b, a); if (b.goal) return true; }
  }
  return false;
}
static void greedyFails() {
  printf("greedy wins of 3 grips, by level:");
  int bad = 0;
  for (int i = 0; i < NUM_LEVELS; i++) {
    int wins = 0;
    for (Grav n : NEUTRALS) wins += greedyScores(LEVELS[i], n, 20000);
    printf(" %d:%d", i + 1, wins);
    bad += i >= 2 && wins;
  }
  printf("\n");
  CHECK(bad == 0);
}
// Never stuck: wherever the ball comes to rest under some tilt (pinned in a cup, a corner, a wedge), some other tilt
// gets it well away within 3 s. Resting in a hole does not count: the hole sends it back to the start.
static bool freed(Ball b) {
  const Vec at = b.p;
  for (int k = 0; k < 8; k++) {
    Ball t = b;
    const Vec a = {cosf(k * 0.785f) * ACCEL_FULL, sinf(k * 0.785f) * ACCEL_FULL};
    for (int i = 0; i < 600 && !t.goal; i++) step(t, a);
    if (t.goal || len({t.p.x - at.x, t.p.y - at.y}) > 2 * BALL_R) return true;
  }
  return false;
}
static void neverStuck() {
  uint32_t rng = 7;
  auto rnd = [&rng](int n) { rng = rng * 1664525u + 1013904223u; return (int)((rng >> 8) % (uint32_t)n); };
  int spots = 0, stuck = 0;
  for (const Level& l : LEVELS)
    for (int trial = 0; trial < 40; trial++) {
      Ball b = start(l);
      for (int hold = 0; hold < 3 && !b.goal; hold++) {   // three random holds of 1.5 s, the last one kept on
        const float ang = rnd(628) / 100.0f, k = 0.4f + rnd(60) / 100.0f;
        for (int i = 0; i < 300 && !b.goal; i++) step(b, {cosf(ang) * ACCEL_FULL * k, sinf(ang) * ACCEL_FULL * k});
      }
      if (b.goal || sinking(b) || len(b.v) > 5) continue;
      spots++;
      if (!freed(b)) { stuck++; printf("level %d: stuck at %.0f,%.0f\n", (int)(&l - LEVELS) + 1, b.p.x, b.p.y); }
    }
  printf("never stuck: %d resting spots, %d stuck\n", spots, stuck);
  CHECK(spots > 50 && stuck == 0);
}

// Replays a level's solution as the game gets it: each step a real 1 g reading (whole milli-g) held for whole frames
// of substeps, from `neutral`. Returns the ms the goal came at, or -1; *total is the solution's length.
static int replay(const Level& l, Grav neutral, int* total) {
  Ball b = start(l);
  int ms = 0, goalMs = -1;
  *total = 0;
  for (int s = 0; s < l.steps; s++) {
    const Step& st = l.solution[s];
    CHECK(st.ms % 40 == 0);
    *total += st.ms;
    const Grav g = gravityFor(st.tx, st.ty, neutral);
    CHECK(fabsf(mag(g) - 1000) < 2);
    const Vec a = tiltAccel(g, neutral);
    for (int t = 0; t < st.ms; t += STEP_MS, ms += STEP_MS) {
      step(b, a);
      if (b.goal && goalMs < 0) goalMs = ms;
    }
  }
  if (goalMs < 0) printf("level %d from %d,%d,%d: no goal, the ball stopped at %.0f,%.0f\n", (int)(&l - LEVELS) + 1,
                         neutral.x, neutral.y, neutral.z, b.p.x, b.p.y);
  return goalMs;
}
// Each level's solution scores from every way of holding the device, and in its last second: the playtests stop
// tilting when the sequence ends, before the Goal page moves on.
static void everyLevelSolvable() {
  int bad = 0;
  for (const Level& l : LEVELS)
    for (Grav n : NEUTRALS) {
      int total;
      const int goalMs = replay(l, n, &total);
      bad += !(goalMs >= 0 && total - goalMs <= 1000);
    }
  CHECK(bad == 0);
}

// The playtests replay the same solutions through the sim, as `tilt` lines: every block in tests/playtests/*.txt
// from "# solution <level> from <x> <y> <z>" (the grip it was calibrated in, one of NEUTRALS) to "# end solution" must
// be exactly what this writes, so a changed level or solution fails here until `build/host/test_marble
// --write-playtests` rewrites them. Returns how many blocks there were, per neutral in `seen`.
static std::string block(int level, Grav n) {
  std::string out = "wait 80\n";   // a still frame first: the sim's snap renders one without moving the clock
  char line[64];
  const Level& l = LEVELS[level - 1];
  for (int i = 0; i < l.steps; i++) {
    const Grav g = gravityFor(l.solution[i].tx, l.solution[i].ty, n);
    snprintf(line, sizeof line, "tilt %d %d %d\nwait %d\n", g.x, g.y, g.z, l.solution[i].ms);
    out += line;
  }
  snprintf(line, sizeof line, "tilt %d %d %d\n", n.x, n.y, n.z);   // back to the grip, still
  return out + line;
}
static int neutralIndex(Grav g) {
  for (int i = 0; i < 3; i++) if (NEUTRALS[i].x == g.x && NEUTRALS[i].y == g.y && NEUTRALS[i].z == g.z) return i;
  return -1;
}
// One file: its text with every block as it should be; counts blocks per neutral, and the ones that differ.
static std::string fixBlocks(const std::string& text, int seen[3], int* stale) {
  std::string out;
  size_t at = 0;
  while (at < text.size()) {
    const size_t eol = text.find('\n', at), next = eol == std::string::npos ? text.size() : eol + 1;
    const std::string line = text.substr(at, next - at);
    out += line;
    at = next;
    int level; Grav n;
    if (sscanf(line.c_str(), "# solution %d from %d %d %d", &level, &n.x, &n.y, &n.z) != 4) continue;
    const size_t end = text.find("# end solution", at);
    const int k = neutralIndex(n);
    CHECK(level >= 1 && level <= NUM_LEVELS && k >= 0 && end != std::string::npos);
    seen[k]++;
    const std::string want = block(level, n);
    *stale += text.compare(at, end - at, want) != 0;
    out += want;
    at = end;
  }
  return out;
}
static void playtestSolutions(bool write) {
  int seen[3] = {0, 0, 0}, stale = 0;
  DIR* d = opendir("tests/playtests");
  if (!d) printf("test_marble: no tests/playtests here (run from apps/porthole)\n");
  CHECK(d);
  while (dirent* e = readdir(d)) {
    const std::string path = std::string("tests/playtests/") + e->d_name;
    if (path.size() < 4 || path.compare(path.size() - 4, 4, ".txt")) continue;
    FILE* f = fopen(path.c_str(), "rb");
    if (!f) { printf("test_marble: cannot read %s (run from apps/porthole)\n", path.c_str()); CHECK(false); }
    std::string text;
    for (int c; (c = fgetc(f)) != EOF;) text += (char)c;
    fclose(f);
    const int before = stale;
    const std::string fixed = fixBlocks(text, seen, &stale);
    if (write && stale != before) {
      f = fopen(path.c_str(), "wb");
      if (!f) { printf("test_marble: cannot write %s\n", path.c_str()); CHECK(false); }
      fputs(fixed.c_str(), f);
      fclose(f);
      printf("rewrote %s\n", path.c_str());
    }
  }
  closedir(d);
  if (stale && !write) printf("test_marble: %d playtest solution blocks are stale: run build/host/test_marble --write-playtests\n", stale);
  CHECK(write || stale == 0);
  CHECK(seen[1] > 0 && seen[2] > 0);   // played from a 45 degree grip and from upright, at least once each
}

static void saveBlob() {
  Save s{};
  s.level = 4;
  seal(s);
  Save back{};
  CHECK(loadBlob(&s, sizeof s, back) && back.level == 4 && back.version == SAVE_VERSION && back.size == sizeof(Save));
  // a level past this build's table (a newer firmware's save) still loads, as is
  s.level = 200; seal(s);
  CHECK(loadBlob(&s, sizeof s, back) && back.level == 200);
  // a wrong crc, a wrong size, another game's magic or a version from the future: rejected, and out is untouched
  Save bad = s; bad.crc ^= 1;
  back.level = 7;
  CHECK(!loadBlob(&bad, sizeof bad, back) && back.level == 7);
  CHECK(!loadBlob(&s, sizeof s - 4, back) && !loadBlob(&s, sizeof s + 4, back) && !loadBlob(&s, 0, back));
  bad = s; bad.size = 20; bad.crc = os::crc32(&bad, sizeof bad - 4);
  CHECK(!loadBlob(&bad, sizeof bad, back));
  bad = s; bad.magic = 0x54435342; bad.crc = os::crc32(&bad, sizeof bad - 4);
  CHECK(!loadBlob(&bad, sizeof bad, back));
  bad = s; bad.version = SAVE_VERSION + 1; bad.crc = os::crc32(&bad, sizeof bad - 4);
  CHECK(!loadBlob(&bad, sizeof bad, back) && back.level == 7);
}
// Version 2 adds the best stars per level; a version 1 save still loads.
static void saveMigration() {
  Save s{}, back{};
  s.level = 3; s.stars[0] = 3; s.stars[2] = 1; s.stars[31] = 2;
  seal(s);
  CHECK(loadBlob(&s, sizeof s, back) && back.stars[0] == 3 && back.stars[1] == 0 && back.stars[2] == 1 && back.stars[31] == 2);
  // version 1 (16 bytes, before stars) still loads: its level kept, no stars yet, sealed as today's version
  struct V1 { uint32_t magic; uint16_t version, size; uint8_t level, reserved[3]; uint32_t crc; };
  static_assert(sizeof(V1) == SAVE_V1_SIZE, "version 1 as it shipped");
  V1 v1 = {SAVE_MAGIC, 1, SAVE_V1_SIZE, 5, {0, 0, 0}, 0};
  v1.crc = os::crc32(&v1, sizeof v1 - 4);
  Save up;
  memset(&up, 0xAA, sizeof up);
  CHECK(loadBlob(&v1, sizeof v1, up) && up.level == 5 && up.version == SAVE_VERSION && up.size == sizeof(Save));
  int stars = 0;
  for (uint8_t st : up.stars) stars += st;
  CHECK(stars == 0);
  // a blob whose size does not match its version (version 1 padded to today's size) is not a save
  Save odd = s; odd.version = 1; odd.crc = os::crc32(&odd, sizeof odd - 4);
  CHECK(!loadBlob(&odd, sizeof odd, back));
}

// Drives the game like the sim drives the device, frame by frame, with two panel buffers used by turns: random real
// tilts (and jolts), taps, holds and brushes anywhere and on its buttons, frames of 0 ms and stalls of 500 ms, and from
// time to time a level's solution replayed so every page comes up. After each frame the buffer just drawn must equal
// the same frame painted whole into a third buffer by a copy of the game.
struct Driver {
  uint32_t rng = 12345, ms = 1000;
  int rand(int n) { rng = rng * 1664525u + 1013904223u; return (int)((rng >> 8) % (uint32_t)n); }
  Grav grav = UPRIGHT;
  bool down = false; int x = 0, y = 0, frames = 0;
  int level = -1; uint32_t solveMs = 0;   // a solution being replayed (level index), and how far into it
  int greedy = 0;                          // frames left of playing greedy (straight at the goal: into the holes)
  int runs = 0;
  void touch() {
    if (down) { if (--frames <= 0) down = false; return; }
    if (rand(25)) return;
    static const int SPOTS[][2] = {{80, 110}, {80, 145}, {50, 50}, {110, 78}, {80, 60}, {30, 90}};
    const int k = rand(8);
    x = k < 6 ? SPOTS[k][0] : 20 + rand(120); y = k < 6 ? SPOTS[k][1] : 20 + rand(120);
    down = true; frames = 1 + rand(3) + (rand(6) ? 0 : 20);   // mostly taps, sometimes a hold past 600 ms
  }
  void tilt(uint32_t dt, const marble::Game& game) {
    if (greedy > 0) {
      greedy--;
      const Ball& b = game.ball();
      const Vec at = mouthAt(*b.level, b.ms);
      const float d = fmaxf(1, len({at.x - b.p.x, at.y - b.p.y}));
      grav = gravityFor((int)lroundf((at.x - b.p.x) / d * 600), (int)lroundf((at.y - b.p.y) / d * 600), UPRIGHT);
      return;
    }
    if (level >= 0) {   // the solution's step at this time, as the sim's playtests feed it
      const Level& l = LEVELS[level];
      uint32_t at = solveMs;   // the frame's start: its substeps run on the step in force then
      solveMs += dt;
      for (int s = 0; s < l.steps; s++, at -= l.solution[s - 1].ms)
        if (at < l.solution[s].ms) { grav = gravityFor(l.solution[s].tx, l.solution[s].ty, UPRIGHT); return; }
      level = -1; grav = UPRIGHT;
      return;
    }
    if (!rand(15)) grav = rand(12) ? gravityFor(rand(1400) - 700, rand(1400) - 700, UPRIGHT) : Grav{1800, 0, 900};
  }
};
// Now and then (not mid-run), calibrate as held upright and play a level: through with its solution, or greedily for
// 3 s (which drops the ball into a level's holes).
static void maybeSolve(Driver& d, marble::Game& game, InputTracker& tracker) {
  if (d.level >= 0 || d.greedy > 0 || d.rand(300)) return;
  d.grav = UPRIGHT; d.level = d.rand(NUM_LEVELS); d.solveMs = 0; d.down = false;
  if (d.runs++ % 3 == 0) d.level = NUM_LEVELS - 1;   // the last level often: its goal leads to the Done page
  const int pick = d.level;
  if (d.runs % 2) { d.greedy = 75; d.level = -1; }
  Input held = tracker.step(false, 0, 0, d.ms);   // a frame in the grip it calibrates in
  held.gx = (int16_t)UPRIGHT.x; held.gy = (int16_t)UPRIGHT.y; held.gz = (int16_t)UPRIGHT.z;
  game.update(d.ms / 1000, d.ms, held);
  char cmd[16];
  snprintf(cmd, sizeof cmd, "level%d", pick + 1);
  game.debugCmd(cmd);
}
// Draws the frame into the panel buffer `shown` as the game does, and whole into `whole` by a copy: true if equal.
static bool drawnRight(marble::Game& game, uint16_t* shown, uint16_t* whole) {
  gfx565::target(shown);
  game.render();
  marble::Game copy = game;
  gfx565::target(whole);
  copy.render();
  if (!memcmp(shown, whole, gfx565::W * gfx565::H * 2)) return true;
  int x0 = 999, y0 = 999, x1 = -1, y1 = -1;
  for (int i = 0; i < gfx565::W * gfx565::H; i++)
    if (shown[i] != whole[i]) { const int x = i % gfx565::W, y = i / gfx565::W; x0 = x < x0 ? x : x0; y0 = y < y0 ? y : y0; x1 = x > x1 ? x : x1; y1 = y > y1 ? y : y1; }
  const Ball& b = game.ball();
  printf("  level %d, ball %.0f,%.0f sinking %d, clock %u: pixels differ in %d,%d-%d,%d\n", (int)(b.level - LEVELS) + 1,
         b.p.x, b.p.y, b.sinkMs, (unsigned)b.ms, x0, y0, x1, y1);
  memcpy(shown, whole, gfx565::W * gfx565::H * 2);   // report each mistake once
  return false;
}
// One frame of the drive: maybe a new run, the time, the tilt, a touch, and the game's update.
static void oneFrame(Driver& d, marble::Game& game, InputTracker& tracker) {
  maybeSolve(d, game, tracker);   // calibrated at the last frame's time: the run's first frame is a whole one
  const bool driven = d.level >= 0 || d.greedy > 0;
  const uint32_t dt = driven || d.rand(10) ? 40 : d.rand(5) ? 0 : 500;
  d.ms += dt;
  d.tilt(dt, game);
  if (!driven) d.touch();
  Input in = tracker.step(d.down && !driven, d.x, d.y, d.ms);
  in.gx = (int16_t)d.grav.x; in.gy = (int16_t)d.grav.y; in.gz = (int16_t)d.grav.z;
  game.update(d.ms / 1000, d.ms, in);
}
// What the drive covered: frames per page, with the ball sinking, with the goal on the move, and frames drawn wrong.
struct Tally {
  int seen[marble::Game::SC_COUNT] = {}, sinkFrames = 0, turningFrames = 0, bad = 0;
  void count(const marble::Game& game) {
    static const char* const PAGES[] = {"mk_calibrate", "mk_play", "mk_goal", "mk_done"};
    for (int s = 0; s < marble::Game::SC_COUNT; s++) seen[s] += !strcmp(game.screenName(), PAGES[s]);
    const bool playing = !strcmp(game.screenName(), "mk_play");
    sinkFrames += playing && sinking(game.ball());
    turningFrames += playing && game.ball().started && game.ball().level->goal.mode != FIXED;
  }
};
static void incrementalEqualsFull() {
  static uint16_t bufs[2][gfx565::W * gfx565::H], whole[gfx565::W * gfx565::H];
  for (auto& b : bufs) for (uint16_t& p : b) p = 0xF81F;
  static marble::Game game;
  const Profile kid = {0, "Sam", 0, 6, 0};
  const SaveSlot none = {nullptr, 0};
  Driver d;
  InputTracker tracker;
  game.enter({&kid, &kid, &none, 1, 0, d.ms});
  Tally t;
  for (int f = 0; f < 8000; f++) {
    oneFrame(d, game, tracker);
    if (game.wantsHome()) { game.leave(); game.enter({&kid, &kid, &none, 1, 0, d.ms}); }
    t.count(game);
    if (!drawnRight(game, bufs[f & 1], whole) && t.bad++ < 5) printf("frame %d on %s: the incremental frame differs\n", f, game.screenName());
  }
  printf("render check: %d frames differ; frames per page %d %d %d %d, sinking %d, goal moving %d\n", t.bad, t.seen[0],
         t.seen[1], t.seen[2], t.seen[3], t.sinkFrames, t.turningFrames);
  CHECK(t.bad == 0);
  for (int s : t.seen) CHECK(s > 0);   // every page was drawn both ways
  CHECK(t.sinkFrames > 20 && t.turningFrames > 100);   // and the ball sinking into a hole, and a goal on the move
}

int main(int argc, char** argv) {
  if (argc > 1 && !strcmp(argv[1], "--write-playtests")) { playtestSolutions(true); return 0; }
  if (argc > 1 && !strcmp(argv[1], "--levels")) { lab = true; noBallSizedGaps(); greedyFails(); neverStuck(); everyLevelSolvable(); return 0; }
  tiltMapping();
  neverLeavesTheTray();
  noTunneling();
  goalDetection();
  noBallSizedGaps();
  railsHold();
  defendersNudge();
  holesSendBack();
  goalMoves();
  postsClearOfTheRimLane();
  goalsClearOfTheCoin();
  noParking();
  saveBlob();
  saveMigration();
  greedyFails();
  neverStuck();
  everyLevelSolvable();
  incrementalEqualsFull();
  playtestSolutions(false);
  printf("test_marble: all %d checks passed (%d levels, sizeof Save = %zu)\n", checks, NUM_LEVELS, sizeof(Save));
  return 0;
}
