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

static void tiltMapping() {
  // neutral subtraction: holding the device the way it was calibrated is no tilt at all, upright or lying flat
  CHECK(near(len(tiltAccel(0, 1000, 0, 1000)), 0) && near(len(tiltAccel(120, -40, 120, -40)), 0));
  // the dead zone: up to DEAD_MG away from neutral, in any direction, nothing
  CHECK(near(len(tiltAccel(DEAD_MG, 1000, 0, 1000)), 0));
  CHECK(near(len(tiltAccel(-60, 1000 + 60, 0, 1000)), 0));   // 85 mg diagonally
  // past it, it grows with the tilt, in the tilt's direction
  Vec a = tiltAccel(0, 1000 - (DEAD_MG + FULL_MG) / 2, 0, 1000);
  CHECK(near(a.x, 0) && a.y < 0 && near(-a.y, ACCEL_FULL / 2));
  Vec r = tiltAccel(200, 1000, 0, 1000);
  CHECK(r.x > 0 && near(r.y, 0) && r.x < ACCEL_FULL);
  // clamped at full tilt: twice as far is no faster
  CHECK(near(len(tiltAccel(FULL_MG, 1000, 0, 1000)), ACCEL_FULL));
  CHECK(near(len(tiltAccel(2 * FULL_MG, 1000, 0, 1000)), ACCEL_FULL));
  Vec d = tiltAccel(-900, 1000 + 900, 0, 1000);
  CHECK(near(len(d), ACCEL_FULL) && near(d.x, -d.y));
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
    CHECK(b.goal == (fabsf(b.p.x) < PIN.goalHalf && b.p.y < 0));
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
  CHECK(inGap(PIN, {0, -180}) && inGap(PIN, {-59, -170}) && !inGap(PIN, {61, -170}) && !inGap(PIN, {0, 180}));
  CHECK(near(len(postAt(PIN, -1)), PITCH_R) && near(postAt(PIN, 1).x, PIN.goalHalf) && postAt(PIN, -1).y < 0);
  // a ball resting in the dead zone stays where it is: nothing drifts it on its own
  b = start(LEVELS[0]);
  for (int i = 0; i < 1000; i++) step(b, tiltAccel(40, 1000 - 40, 0, 1000));
  CHECK(b.p.x == LEVELS[0].startX && b.p.y == LEVELS[0].startY);
}

// Each level's solution, applied exactly as the game does (the sim's upright neutral, whole frames of substeps),
// scores, and in its last second: the playtests stop tilting when the sequence ends, before the Goal page moves on.
static void everyLevelSolvable() {
  for (int i = 0; i < NUM_LEVELS; i++) {
    const Level& l = LEVELS[i];
    Ball b = start(l);
    int ms = 0, goalMs = -1, total = 0;
    for (int s = 0; s < l.steps; s++) {
      const Step& st = l.solution[s];
      CHECK(st.ms % 40 == 0);
      total += st.ms;
      const Vec a = tiltAccel(st.tx, 1000 + st.ty, 0, 1000);
      for (int t = 0; t < st.ms; t += STEP_MS, ms += STEP_MS) {
        step(b, a);
        if (b.goal && goalMs < 0) goalMs = ms;
      }
    }
    if (goalMs < 0) printf("level %d: no goal, the ball stopped at %.0f,%.0f\n", i + 1, b.p.x, b.p.y);
    CHECK(goalMs >= 0);
    CHECK(total - goalMs <= 1000);
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
  everyLevelSolvable();
  saveBlob();
  printf("test_marble: all %d checks passed (%d levels, sizeof Save = %zu)\n", checks, NUM_LEVELS, sizeof(Save));
  return 0;
}
