// Self-check for Sand Jar's physics (games/sand-jar/physics.h): the jar keeps every grain, one per cell and all inside
// the glass; at 0 g nothing at rest moves; water levels out at any angle; a small change of tilt never lifts sand; the
// finger clears its disc; Orbits' bodies stay on the glass; the same inputs replay the same jar.
// Run: make test
#include <assert.h>
#include <math.h>
#include <stdio.h>
#include <string.h>
#include <initializer_list>
#include "games/sand-jar/physics.h"

using namespace sandjar;
static Jar a, b;   // ~170 KB each: static, like the game's one allocation

static int cellOf(const Jar& j, int i) { return (j.g.y[i] >> 8) * G + (j.g.x[i] >> 8); }
static float along(const Jar& j, int i, float gx, float gy) {   // height below the middle, along gravity (cells)
  return (Jar::pos(j.g.x[i]) - G / 2) * gx + (Jar::pos(j.g.y[i]) - G / 2) * gy;
}
static void tilt(Jar& j, float degrees, float g = 1) {   // 0 = upright (down the screen), positive = toward +x
  j.tiltX = g * sinf(degrees * 3.14159265f / 180); j.tiltY = g * cosf(degrees * 3.14159265f / 180);
}
static void run(Jar& j, int steps) { for (int s = 0; s < steps; s++) j.step(); }

// Every grain owns exactly its own cell, every cell it owns is inside the glass, and the kinds add up.
static void consistent(const Jar& j, int sand, int liquid) {
  int owned = 0, s = 0, l = 0;
  for (int c = 0; c < G * G; c++) if (j.occ[c] > 0) owned++;
  assert(owned == j.n);
  for (int i = 0; i < j.n; i++) {
    const int c = cellOf(j, i);
    assert(j.occ[c] == i + 1);
    const float dx = c % G + 0.5f - G / 2, dy = c / G + 0.5f - G / 2;
    assert(dx * dx + dy * dy <= 78.5f * 78.5f);
    (kindOf(j.g.look[i]) == SAND ? s : l)++;
  }
  assert(s == sand && l == liquid);
}

static void filled() {
  const int want[MODE_COUNT] = {34, 42, 46, 3};
  for (int m = 0; m < MODE_COUNT; m++) {
    tilt(a, 0);
    a.reset((Mode)m, 7);
    assert(a.inside > 19000 && a.inside < 19700);
    assert(a.n == a.inside * want[m] / 100);
    if (m == MODE_ORBITS) continue;
    const int sand = m == MODE_BEACH ? (int)lroundf(a.n * BEACH_SAND) : a.n;
    consistent(a, sand, a.n - sand);
    run(a, 200);
    consistent(a, sand, a.n - sand);   // conserved while it flows, sinks and bubbles
  }
  tilt(a, 0);
  a.reset(MODE_RAINBOW, 7);   // six strata, the first (pink) at the bottom
  int bottom = 0;
  for (int i = 0; i < a.n; i++) if (shadeOf(a.g.look[i]) == 0) bottom = a.g.y[i] > a.g.y[bottom] ? i : bottom;
  assert(shadeOf(a.g.look[bottom]) == 0 && (a.g.y[bottom] >> 8) >= 150);
  puts("fill: every mode's count, strata, conserved while flowing");
}

// The bug the user hit: rolling and spilling must scale with gravity's strength. A heap settled at 25 degrees, then the
// jar laid flat (0 g) while the last direction it saw points elsewhere: nothing may avalanche toward that direction.
static void restsAtZeroG() {
  for (Mode m : {MODE_SAND, MODE_RAINBOW}) {
    tilt(a, 25);
    a.reset(m, 11);
    run(a, 700);
    static uint16_t before[MAX_GRAINS];
    for (int i = 0; i < a.n; i++) before[i] = (uint16_t)cellOf(a, i);
    tilt(a, 0, 0);
    a.gxn = 1; a.gyn = 0;
    run(a, 400);
    int moved = 0;
    for (int i = 0; i < a.n; i++) moved += cellOf(a, i) != before[i];
    assert(moved == 0);
  }
  puts("rest: at 0 g a settled heap stays put");
}

// Water spreads along the true level line (the Bresenham walk between the two grid directions around it), so its
// surface ends up level along gravity at any angle, not in grid-aligned steps. Tipped 30 degrees from a level heap it
// takes the prototype's own ~2500 steps to settle within ~6 cells (a stuck walk leaves ~40); 45 is a grid diagonal.
static float surfaceSpread(float degrees) {
  tilt(a, degrees);
  a.reset(MODE_BEACH, 3);
  for (int i = 0; i < a.n; i++) a.g.look[i] = makeLook(WATER, 0, 0);
  run(a, 2500);
  const int up = DY[a.upDir] * G + DX[a.upDir];
  float lo = 1e9f, hi = -1e9f;
  for (int i = 0; i < a.n; i++) {
    if (a.occ[cellOf(a, i) + up] != EMPTY) continue;   // under water, or against the glass
    const float h = along(a, i, a.gxn, a.gyn);
    lo = fminf(lo, h); hi = fmaxf(hi, h);
  }
  return hi - lo;
}
static void waterLevels() {
  for (float deg : {30.0f, 45.0f, -60.0f}) {
    const float spread = surfaceSpread(deg);
    printf("level: water at %.0f degrees, surface within %.1f cells\n", deg, spread);
    assert(spread < 8);
  }
}

// Grains never move uphill: settle, tip the jar a little, and no grain may end higher along the new gravity.
static void neverClimbs() {
  for (float deg : {0.0f, 20.0f, -40.0f}) {
    tilt(a, deg);
    a.reset(MODE_SAND, 5);
    run(a, 700);
    tilt(a, deg + 6);
    static float before[MAX_GRAINS];
    for (int i = 0; i < a.n; i++) before[i] = along(a, i, a.tiltX, a.tiltY);
    run(a, 400);
    float worst = 0;
    for (int i = 0; i < a.n; i++) worst = fmaxf(worst, before[i] - along(a, i, a.tiltX, a.tiltY));
    assert(worst < 1);   // within a cell: a roll lands on the next cell's center
    consistent(a, a.n, 0);
  }
  puts("climb: a small tilt never lifts a grain");
}

// The finger is solid: after one step nothing is left inside its disc, and every grain it pushed is still in the jar.
static void fingerClears() {
  for (Mode m : {MODE_SAND, MODE_BEACH}) {
    tilt(a, 0);
    a.reset(m, 9);
    run(a, 300);
    const int sand = m == MODE_BEACH ? (int)lroundf(a.n * BEACH_SAND) : a.n;
    a.finger = {true, 80.5f, 138.5f, 0.3f, -0.4f};
    for (int s = 0; s < 30; s++) {
      a.finger.x += 0.4f;
      a.step();
      for (int i = 0; i < a.n; i++) {
        const float dx = (cellOf(a, i) % G) + 0.5f - a.finger.x, dy = (cellOf(a, i) / G) + 0.5f - a.finger.y;
        assert(dx * dx + dy * dy > FINGER_R * FINGER_R);
      }
    }
    consistent(a, sand, a.n - sand);
    a.finger.on = false;
    a.step();
    for (int c = 0; c < G * G; c++) assert(a.occ[c] != FINGER);   // lifted: its cells are free again
  }
  puts("finger: its disc is cleared and nothing is lost");
}

static void shakeScatters() {
  tilt(a, 0);
  a.reset(MODE_BEACH, 4);
  run(a, 200);
  const int sand = (int)lroundf(a.n * BEACH_SAND);
  a.shake();
  run(a, 60);
  consistent(a, sand, a.n - sand);
  puts("shake: grains scatter, none lost");
}

static void orbitsStayOnGlass() {
  tilt(a, 30, 0.5f);
  a.reset(MODE_ORBITS, 8);
  a.finger = {true, 100, 60, 0, 0};
  for (int s = 0; s < 600; s++) {
    if (s == 300) { a.finger.on = false; a.shake(); }
    a.step();
    for (int i = 0; i < a.n; i++) {
      const float dx = a.sky.body[i].x - 240, dy = a.sky.body[i].y - 240;
      assert(dx * dx + dy * dy <= 232.5f * 232.5f && a.sky.body[i].vx == a.sky.body[i].vx);
    }
  }
  puts("orbits: every body stays on the glass");
}

static void replays() {
  for (Jar* j : {&a, &b}) {
    tilt(*j, 35);
    j->reset(MODE_BEACH, 21);
    j->finger = {true, 70, 120, 1, 0};
    run(*j, 50);
    j->finger.on = false;
    j->shake();
    run(*j, 50);
  }
  assert(!memcmp(a.g.x, b.g.x, sizeof a.g.x) && !memcmp(a.occ, b.occ, sizeof a.occ) && !memcmp(a.g.look, b.g.look, sizeof a.g.look));
  puts("replay: the same inputs make the same jar");
}

int main() {
  filled();
  restsAtZeroG();
  waterLevels();
  neverClimbs();
  fingerClears();
  shakeScatters();
  orbitsStayOnGlass();
  replays();
  puts("test_sand_jar: all passed");
  return 0;
}
