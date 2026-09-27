// Self-check for Marble Kick's rules (games/marble-kick/physics.*, levels.h, save.h): the tilt mapping, the ball
// staying on the tray except through the goal, no tunneling through a peg, goal detection, the save blob, and every
// level's recorded solution replayed to a goal.
// Run: make test
#include <assert.h>
#include <math.h>
#include <stdio.h>
#include <string.h>
#include "games/marble-kick/physics.h"
#include "games/marble-kick/save.h"

using namespace marble;
static int checks = 0;
#define CHECK(c) (assert(c), checks++)

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

static void tiltMapping() {
  static const Grav HOLDS[] = {FLAT, LEANED, UPRIGHT, FACE_DOWN, {500, 500, -707}};
  for (Grav n : HOLDS) {
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
  // held upright, the way the kid moves it: leaning back rolls the ball up, leaning forward rolls it down, turning it
  // like a steering wheel rolls it sideways (30 and 20 degrees: full tilt, well past the dead zone)
  CHECK(tiltAccel({0, 866, -500}, UPRIGHT).y < -ACCEL_FULL + 1 && tiltAccel({0, 866, 500}, UPRIGHT).y > ACCEL_FULL - 1);
  const Vec wheel = tiltAccel({342, 940, 0}, UPRIGHT);
  CHECK(wheel.x > 0 && fabsf(wheel.y) < 0.5f * wheel.x);
}

// A made-up level for the rules: one post-sized peg (the thinnest thing on any tray) in the middle.
static const Level PIN = {0, 120, 60, 1, {{0, 0, POST_R}}, nullptr, 0};

static bool onTray(const Ball& b) {
  const float d = len(b.p);
  return d <= PITCH_R - BALL_R + 0.01f || (inGap(*b.level, b.p) && d <= PITCH_R);
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
      if (b.goal) { CHECK(inGap(l, b.p) && len(b.p) > PITCH_R); b = start(l); continue; }
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
    CHECK(b.goal == inGap(PIN, b.p));
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
  CHECK(b.goal && len(b.p) > PITCH_R && inGap(PIN, b.p));
  const Vec at = b.p;
  step(b, {ACCEL_FULL, 0});
  CHECK(b.p.x == at.x && b.p.y == at.y);
  // at the bottom, where the rim is closed, never
  b = start(PIN);
  for (int i = 0; i < 2000; i++) step(b, {0, ACCEL_FULL});
  CHECK(!b.goal && b.p.y > 0);
  // the gap is only at the top, between the posts
  // the gap is the mouth between the posts, where the whole ball fits: at the top, the ball's center clear of each post
  const float mouth = PIN.goalHalf - POST_R - BALL_R;
  CHECK(inGap(PIN, {0, -180}) && inGap(PIN, {-(mouth - 1), -170}) && !inGap(PIN, {mouth + 1, -170}) && !inGap(PIN, {0, 180}));
  CHECK(near(len(postAt(PIN, -1)), POST_RING) && near(postAt(PIN, 1).x, PIN.goalHalf) && postAt(PIN, -1).y < 0);
  // a ball resting in the dead zone stays where it is: nothing drifts it on its own
  b = start(LEVELS[0]);
  for (int i = 0; i < 1000; i++) step(b, tiltAccel(gravityFor(40, -40, UPRIGHT), UPRIGHT));
  CHECK(b.p.x == LEVELS[0].startX && b.p.y == LEVELS[0].startY);
}

// No pockets by construction: every gap the ball could try, between a peg, a goalpost or the back knob and the rim or
// each other, is closed (under 2 px) or wide enough to roll through with room to spare (the ball's width + 8 px). A
// gap in between is where a ball gets wedged.
static void noPockets() {
  constexpr float CLOSED = 2, OPEN = 2 * BALL_R + 8;
  int pockets = 0;
  for (const Level& l : LEVELS) {
    struct Round { Vec c; float r; } things[12];
    int n = 0;
    for (int i = 0; i < l.pegCount; i++) things[n++] = {{(float)l.pegs[i].x, (float)l.pegs[i].y}, (float)l.pegs[i].r};
    things[n++] = {postAt(l, -1), POST_R};
    things[n++] = {postAt(l, 1), POST_R};
    things[n++] = {{0, KNOB_Y}, KNOB_R};
    for (int i = 0; i < n; i++) {
      const float rim = PITCH_R - len(things[i].c) - things[i].r;
      if (rim >= CLOSED && rim < OPEN) printf("level %d: thing %d is %.1f px from the rim\n", (int)(&l - LEVELS) + 1, i, rim);
      pockets += rim >= CLOSED && rim < OPEN;
      for (int j = i + 1; j < n; j++) {
        const float gap = len({things[i].c.x - things[j].c.x, things[i].c.y - things[j].c.y}) - things[i].r - things[j].r;
        if (gap >= CLOSED && gap < OPEN) printf("level %d: things %d and %d are %.1f px apart\n", (int)(&l - LEVELS) + 1, i, j, gap);
        pockets += gap >= CLOSED && gap < OPEN;
      }
    }
  }
  CHECK(pockets == 0);
}
// The goalposts sit in the wall, clear of the lane a ball rolls along the rim in: a ball pressed to the rim slides
// past them.
static void postsClearOfTheRimLane() {
  for (const Level& l : LEVELS)
    for (int side = -1; side <= 1; side += 2) CHECK(len(postAt(l, side)) - POST_R >= PITCH_R);
}

// Holding one natural tilt toward the goal gets there. Level 5 from upright, leaned back 30 degrees and turned 17
// (the reviewer's repro: it used to wedge the ball against a guard for good); and a ball pressed to the rim on level 1
// slides up it into the goal (it used to park on a goalpost).
static int holdTilt(const Level& l, Vec from, Grav g, Grav neutral, int ms) {
  Ball b = start(l);
  b.p = from;
  const Vec a = tiltAccel(g, neutral);
  for (int t = 0; t < ms; t += STEP_MS) { step(b, a); if (b.goal) return t; }
  return -1;
}
static void noParking() {
  CHECK(holdTilt(LEVELS[4], {LEVELS[4].startX, LEVELS[4].startY}, {-150, 500, -852}, UPRIGHT, 2500) >= 0);
  for (int side = -1; side <= 1; side += 2)
    CHECK(holdTilt(LEVELS[0], {side * (PITCH_R - BALL_R - 1.0f), 0}, gravityFor(0, -FULL_MG, FLAT), FLAT, 3000) >= 0);
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
  for (const Level& l : LEVELS)
    for (Grav n : NEUTRALS) {
      int total;
      const int goalMs = replay(l, n, &total);
      CHECK(goalMs >= 0 && total - goalMs <= 1000);
    }
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

int main() {
  tiltMapping();
  neverLeavesTheTray();
  noTunneling();
  goalDetection();
  noPockets();
  postsClearOfTheRimLane();
  noParking();
  saveBlob();
  everyLevelSolvable();
  printf("test_marble: all %d checks passed (%d levels, sizeof Save = %zu)\n", checks, NUM_LEVELS, sizeof(Save));
  return 0;
}
