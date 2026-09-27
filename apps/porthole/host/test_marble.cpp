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
  void touch() {
    if (down) { if (--frames <= 0) down = false; return; }
    if (rand(25)) return;
    static const int SPOTS[][2] = {{80, 110}, {80, 145}, {50, 50}, {110, 78}, {80, 60}, {30, 90}};
    const int k = rand(8);
    x = k < 6 ? SPOTS[k][0] : 20 + rand(120); y = k < 6 ? SPOTS[k][1] : 20 + rand(120);
    down = true; frames = 1 + rand(3) + (rand(6) ? 0 : 20);   // mostly taps, sometimes a hold past 600 ms
  }
  void tilt(uint32_t dt) {
    if (level >= 0) {   // the solution's step at this time, as the sim's playtests feed it
      const Level& l = LEVELS[level];
      uint32_t at = solveMs += dt;
      for (int s = 0; s < l.steps; s++, at -= l.solution[s - 1].ms)
        if (at < l.solution[s].ms) { grav = gravityFor(l.solution[s].tx, l.solution[s].ty, UPRIGHT); return; }
      level = -1; grav = UPRIGHT;
      return;
    }
    if (!rand(15)) grav = rand(12) ? gravityFor(rand(1400) - 700, rand(1400) - 700, UPRIGHT) : Grav{1800, 0, 900};
  }
};
// Now and then (not mid-solution), calibrate as held upright and play a level through.
static void maybeSolve(Driver& d, marble::Game& game, InputTracker& tracker) {
  if (d.level >= 0 || d.rand(300)) return;
  d.grav = UPRIGHT; d.level = d.rand(NUM_LEVELS); d.solveMs = 0; d.down = false;
  Input held = tracker.step(false, 0, 0, d.ms);   // a frame in the grip it calibrates in
  held.gx = (int16_t)UPRIGHT.x; held.gy = (int16_t)UPRIGHT.y; held.gz = (int16_t)UPRIGHT.z;
  game.update(d.ms / 1000, d.ms, held);
  char cmd[16];
  snprintf(cmd, sizeof cmd, "level%d", d.level + 1);
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
  memcpy(shown, whole, gfx565::W * gfx565::H * 2);   // report each mistake once
  return false;
}
static void incrementalEqualsFull() {
  static uint16_t bufs[2][gfx565::W * gfx565::H], whole[gfx565::W * gfx565::H];
  static const char* const PAGES[] = {"mk_calibrate", "mk_play", "mk_goal", "mk_done"};
  for (auto& b : bufs) for (uint16_t& p : b) p = 0xF81F;
  static marble::Game game;
  const Profile kid = {0, "Sam", 0, 6, 0};
  const SaveSlot none = {nullptr, 0};
  Driver d;
  InputTracker tracker;
  game.enter({&kid, &kid, &none, 1, 0, d.ms});
  int bad = 0, seen[marble::Game::SC_COUNT] = {};
  for (int f = 0; f < 6000; f++) {
    const uint32_t dt = d.level >= 0 || d.rand(10) ? 40 : d.rand(5) ? 0 : 500;
    d.ms += dt;
    d.tilt(dt);
    if (d.level < 0) d.touch();
    maybeSolve(d, game, tracker);
    Input in = tracker.step(d.down && d.level < 0, d.x, d.y, d.ms);
    in.gx = (int16_t)d.grav.x; in.gy = (int16_t)d.grav.y; in.gz = (int16_t)d.grav.z;
    game.update(d.ms / 1000, d.ms, in);
    if (game.wantsHome()) { game.leave(); game.enter({&kid, &kid, &none, 1, 0, d.ms}); }
    for (int s = 0; s < marble::Game::SC_COUNT; s++) seen[s] += !strcmp(game.screenName(), PAGES[s]);
    if (!drawnRight(game, bufs[f & 1], whole) && bad++ < 5) printf("frame %d on %s: the incremental frame differs\n", f, game.screenName());
  }
  printf("render check: %d frames differ; frames per page %d %d %d %d\n", bad, seen[0], seen[1], seen[2], seen[3]);
  CHECK(bad == 0);
  for (int s : seen) CHECK(s > 0);   // every page was drawn both ways
}

int main(int argc, char** argv) {
  if (argc > 1 && !strcmp(argv[1], "--write-playtests")) { playtestSolutions(true); return 0; }
  tiltMapping();
  neverLeavesTheTray();
  noTunneling();
  goalDetection();
  noPockets();
  postsClearOfTheRimLane();
  noParking();
  saveBlob();
  everyLevelSolvable();
  incrementalEqualsFull();
  playtestSolutions(false);
  printf("test_marble: all %d checks passed (%d levels, sizeof Save = %zu)\n", checks, NUM_LEVELS, sizeof(Save));
  return 0;
}
