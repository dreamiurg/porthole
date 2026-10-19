// Starfighter's pure, fixed-step simulation. No render/debug shortcuts in the winning pilot.
#include <assert.h>
#include <math.h>
#include <stdio.h>
#include "games/starfighter/rules.h"
using namespace starfighter;
static int checks = 0;
#define CHECK(c) (assert(c), checks++)

static void startAndSteer() {
  Run r{};
  restart(r, 42);
  CHECK(r.stage == Stage::Squadron && r.lives == 3 && r.score == 0);
  for (int i = 0; i < 100; ++i) step(r, 0, false);
  CHECK(r.player.x == 240);
  for (int i = 0; i < 100; ++i) step(r, 9, false);
  CHECK(r.player.x <= PLAYER_MAX_X && r.player.bank <= 1);
  for (int i = 0; i < 200; ++i) step(r, -9, false);
  CHECK(r.player.x >= PLAYER_MIN_X && r.player.bank >= -1);
}
static Run arena() {
  Run r{};
  restart(r, 17);
  r.groupsSpawned = GROUPS_PER_WAVE;
  auto& e = r.enemies[0];
  e.x = e.homeX = 150;
  e.y = 100;
  e.fireMs = 100000;
  e.hp = 1;
  e.active = true;
  return r;
}
static int countBolts(const Run& r) {
  int n = 0;
  for (const auto& b : r.bolts) if (b.active && !b.enemy) ++n;
  return n;
}
static void volleys() {
  Run r = arena();
  step(r, 0, true);
  CHECK(r.volleys == 1 && countBolts(r) == 2);
  CHECK(r.bolts[0].x < r.player.x - 40 && r.bolts[1].x > r.player.x + 40);
  CHECK(r.bolts[0].y == r.player.y - CANNON_Y - PLAYER_BOLT_SPEED * DT);
  for (int i = 0; i < 8; ++i) step(r, 0, false);
  CHECK(r.volleys == 1 && countBolts(r) == 2);
  step(r, 0, true);
  CHECK(r.volleys == 2 && countBolts(r) == 4);
  for (auto& b : r.bolts) b = {40, 300, 0, 0, 1, false, true};
  r.bolts[0].active = false;
  step(r, 0, true);
  CHECK(r.volleys == 2 && countBolts(r) == MAX_BOLTS - 1);
  r.bolts[1].active = false;
  step(r, 0, true);
  CHECK(r.volleys == 3 && countBolts(r) == MAX_BOLTS);
}
static void pressureRamp() {
  Run first{}, last{};
  restart(first, 37);
  restart(last, 37);
  last.wave = WAVES - 1;
  step(first, 0, false);
  step(last, 0, false);
  CHECK(first.enemies[0].formation == last.enemies[0].formation);
  CHECK(last.enemies[0].y > first.enemies[0].y);
  CHECK(first.enemies[0].fireMs - last.enemies[0].fireMs == (WAVES - 1) * WAVE_FIRE_GAIN_MS);
  first.enemies[0].fireMs = last.enemies[0].fireMs = STEP_MS;
  step(first, 0, false);
  step(last, 0, false);
  CHECK(first.enemies[0].fireMs - last.enemies[0].fireMs == (WAVES - 1) * WAVE_FIRE_GAIN_MS);
}
static void fullCapacity() {
  Run r = arena();
  for (auto& e : r.effects) e = {240, 360, 10000, true, true};
  for (auto& b : r.bolts) b = {40, 300, 0, 0, 0, false, true};
  r.bolts[0] = {r.player.x, r.player.y, 0, 0, 0, true, true};
  step(r, 0, true);
  CHECK(r.lives == 2 && r.volleys == 0);
  int effects = 0;
  for (const auto& e : r.effects) if (e.active) ++effects;
  CHECK(effects == MAX_EFFECTS);
  r.player.invulnerableMs = 100000;
  for (auto& e : r.enemies) {
    e = r.enemies[0];
    e.fireMs = STEP_MS;
  }
  for (int i = 0; i < 500; ++i) step(r, 0, true);
  CHECK(r.stage == Stage::Squadron && r.lives == 2);
}
static void collisions() {
  Run r = arena();
  for (auto& b : r.bolts) b = {r.player.x, r.player.y, 0, 0, 0, true, true};
  step(r, 0, false);
  CHECK(r.lives == 2 && r.player.invulnerableMs == INVULNERABLE_MS);
  for (int i = 0; i < 50; ++i) {
    r.bolts[0] = {r.player.x, r.player.y, 0, 0, 0, true, true};
    step(r, 0, false);
  }
  CHECK(r.lives == 2 && r.player.invulnerableMs >= 500);
  while (r.player.invulnerableMs) step(r, 0, false);
  r.bolts[0] = {r.player.x, r.player.y, 0, 0, 0, true, true};
  step(r, 0, false);
  CHECK(r.lives == 1);
  r.player.invulnerableMs = 0;
  r.bolts[0] = {r.player.x, r.player.y, 0, 0, 0, true, true};
  step(r, 0, false);
  CHECK(r.stage == Stage::Defeat && r.lives == 0);
  const uint32_t ms = r.ms, score = r.score;
  for (int i = 0; i < 500; ++i) step(r, 1, true);
  CHECK(r.ms == ms && r.score == score && r.lives == 0);
}
static void heavyEnemy() {
  Run r = arena();
  auto& e = r.enemies[0];
  e.x = e.homeX = 240;
  e.y = 240;
  e.hp = 2;
  e.heavy = true;
  e.formation = Formation::Heavy;
  step(r, 0, true);
  for (int i = 0; i < 20; ++i) step(r, 0, false);
  CHECK(e.active && e.hp == 1 && e.heavy && r.score == 0);
  CHECK(e.lastVolley == 1);  // Two red beams from one tap cannot count twice.
  step(r, 0, true);
  for (int i = 0; i < 20; ++i) step(r, 0, false);
  CHECK(!e.active && r.score == 150 && r.stage == Stage::WaveClear);
}
static void visibleWingHits() {
  Run r = arena();
  auto& e = r.enemies[0];
  r.bolts[0] = {e.x + 27, e.y, 0, 0, 1, false, true};
  step(r, 0, false);
  CHECK(!e.active && r.score == 100);
  r = arena();
  r.enemies[0].heavy = true;
  r.enemies[0].hp = 1;  // Heavy silhouette stays large after its first hit.
  r.bolts[0] = {r.enemies[0].x + 38, r.enemies[0].y, 0, 0, 2, false, true};
  step(r, 0, false);
  CHECK(!r.enemies[0].active && r.score == 150);
}
static void noFreeClears() {
  Run r = arena();
  r.player.invulnerableMs = 1000000;
  auto& e = r.enemies[0];
  e.y = 404.9f;
  step(r, 0, false);
  CHECK(e.active && e.y < 100 && r.score == 0 && r.stage == Stage::Squadron);
  for (int i = 0; i < 10000; ++i) step(r, 0, false);
  CHECK(e.active && r.stage == Stage::Squadron && r.score == 0);
  CHECK(e.x >= PLAYER_MIN_X - 12 && e.x <= PLAYER_MAX_X + 12);
}
static void trenchCollisions() {
  Run r{};
  restart(r, 5);
  r.stage = Stage::Trench;
  r.obstacles[0] = {r.player.x, r.player.y, true};
  step(r, 0, false);
  CHECK(r.lives == 2 && !r.obstacles[0].active);
  r.obstacles[0] = {240, 240, true};
  step(r, 0, true);
  for (int i = 0; i < 20; ++i) step(r, 0, false);
  CHECK(!r.obstacles[0].active && r.score == 50);
  for (int i = 0; i < 100; ++i) step(r, -1, false);
  CHECK(r.player.x >= 145);
  for (int i = 0; i < 100; ++i) step(r, 1, false);
  CHECK(r.player.x <= 335);
}
static void finishTorpedo(Run& r) {
  const float launchX = r.player.x, targetX = r.targetX;
  for (int i = 0; i < 40; ++i) step(r, i % 2 ? -1 : 1, true);
  CHECK(r.stage == Stage::Torpedo && r.player.x == launchX && r.targetX == targetX);
  while (playing(r.stage)) step(r, 0, true);
  CHECK(r.stage == Stage::Victory && r.score == 1750);
}
static void forgivingTarget() {
  Run r{};
  restart(r, 9);
  r.stage = Stage::Target;
  r.player.x = 145;
  for (int pass = 0; pass < 20; ++pass) {
    for (uint32_t t = 0; t < TARGET_PASS_MS; t += STEP_MS) step(r, -1, true);
    CHECK(r.stage == Stage::Target && r.lives == 3 && r.score == 0);
    CHECK(r.targetPasses == static_cast<uint32_t>(pass + 1));
  }
  while (r.player.x < 238) step(r, 1, false);
  CHECK(aligned(r));
  step(r, 0, true);
  CHECK(r.stage == Stage::Torpedo && r.score == 1750);
  finishTorpedo(r);
  restart(r, 9);
  CHECK(r.stage == Stage::Squadron && r.lives == 3 && !r.score && !r.targetPasses && !r.volleys);
}
static float limit(float v) { return v < -1 ? -1 : v > 1 ? 1 : v; }
static float spaceAim(const Run& r, int& target) {
  if (target < 0 || !r.enemies[target].active || r.enemies[target].y > 315) {
    target = -1;
    for (int i = 0; i < MAX_ENEMIES; ++i) {
      if (!r.enemies[i].active || r.enemies[i].y > 315) continue;
      if (target < 0 || fabsf(r.enemies[i].x - r.player.x) < fabsf(r.enemies[target].x - r.player.x)) target = i;
    }
  }
  if (target < 0) return 240;
  const auto& e = r.enemies[target];
  const bool swoop = e.formation == Formation::Swoop;
  const float flight = (r.player.y - 22 - e.y) / (PLAYER_BOLT_SPEED + (swoop ? 19 : 13));
  return e.homeX + sinf(e.phase + flight * (swoop ? 1.15f : 0.55f)) * (swoop ? 52 : 22);
}
static float trenchAim(const Run& r) {
  float aim = 240;
  for (const auto& o : r.obstacles) {
    if (o.active && o.y > 230 && fabsf(o.x - r.player.x) < 40) aim = o.x < 240 ? 300 : 180;
  }
  for (const auto& b : r.bolts) {
    if (b.active && b.enemy && b.y > 230 && b.y < 380 && fabsf(b.x - r.player.x) < 40) aim = b.x < 240 ? 310 : 170;
  }
  return aim;
}
static void pilot(Run& r, int& target) {
  float aim = r.stage == Stage::Squadron ? spaceAim(r, target) : trenchAim(r);
  if (r.stage == Stage::Target) aim = r.targetX;
  const bool tap = r.ms % 240 == 0;
  step(r, limit((aim - r.player.x) / 24), tap);
}
static void naturalRuns() {
  uint32_t shortest = 240000, longest = 0, spaceMin = 240000, spaceMax = 0;
  for (uint32_t seed = 1; seed <= 12; ++seed) {
    Run r{};
    restart(r, seed);
    int target = -1;
    uint32_t spaceMs = 0;
    while (playing(r.stage) && r.ms < 240000) {
      if (r.stage == Stage::TrenchEntry && !spaceMs) spaceMs = r.ms;
      pilot(r, target);
    }
    if (r.ms < shortest) shortest = r.ms;
    if (r.ms > longest) longest = r.ms;
    if (spaceMs < spaceMin) spaceMin = spaceMs;
    if (spaceMs > spaceMax) spaceMax = spaceMs;
    CHECK(r.stage == Stage::Victory);
    CHECK(r.ms >= 120000 && r.ms <= 240000);
    CHECK(spaceMs >= 75000 && spaceMs <= 110000);
    CHECK(r.score >= 5800 && r.lives > 0);
  }
  printf("12 pilots won: %u-%u ms; space %u-%u ms\n", shortest, longest, spaceMin, spaceMax);
}
static void noviceRuns() {
  int victories = 0, startingLives = 0;
  for (uint32_t seed = 1; seed <= 30; ++seed) {
    Run r{};
    restart(r, seed);
    float aim = 240;
    int target = -1;
    while (playing(r.stage) && r.ms < 240000) {
      if (r.ms % 400 == 0) {
        if (r.stage == Stage::Squadron) {
          target = -1;
          spaceAim(r, target);
          if (target >= 0) aim = r.enemies[target].x;  // Reaction delay and no leading shots.
        } else aim = r.stage == Stage::Target ? r.targetX : trenchAim(r);
      }
      step(r, limit((aim - r.player.x) / 40) * 0.65f, r.ms % 600 == 0);
    }
    if (r.stage == Stage::Victory) { ++victories; startingLives += r.lives; }
    CHECK(r.ms <= 180000);
  }
  printf("novice %d wins, %d lives\n", victories, startingLives);
  CHECK(victories >= 27 && startingLives >= 54);
}
static void idleLoss() {
  Run r{};
  restart(r, 11);
  while (playing(r.stage) && r.ms < 120000) step(r, 0, false);
  CHECK(r.stage == Stage::Defeat && r.lives == 0 && r.score == 0);
  CHECK(r.ms >= 10000);  // Enough time for the child to find their grip.
}
static void deterministicFrame(Run& a, Run& b, int& target) {
  float aim = a.stage == Stage::Squadron ? spaceAim(a, target) : trenchAim(a);
  if (a.stage == Stage::Target) aim = a.targetX;
  const float bank = limit((aim - a.player.x) / 24);
  const bool tap = a.ms % 240 == 0;
  step(a, bank, tap);
  step(b, bank, tap);
}
static void deterministicAndBounded() {
  Run a{}, b{};
  restart(a, 23);
  restart(b, 23);
  int target = -1;
  for (int i = 0; i < 12000 && playing(a.stage); ++i) {
    deterministicFrame(a, b, target);
    CHECK(a.stage == b.stage && a.seed == b.seed && a.score == b.score && a.lives == b.lives);
    CHECK(a.player.x == b.player.x && isfinite(a.player.x));
    for (int n = 0; n < MAX_ENEMIES; ++n) {
      CHECK(a.enemies[n].x == b.enemies[n].x && a.enemies[n].hp == b.enemies[n].hp);
      CHECK(!a.enemies[n].active || (a.enemies[n].hp > 0 && a.enemies[n].hp <= 2));
    }
    for (int n = 0; n < MAX_BOLTS; ++n) CHECK(a.bolts[n].x == b.bolts[n].x && a.bolts[n].active == b.bolts[n].active);
  }
  CHECK(a.stage == Stage::Victory);
  restart(a, 1);
  restart(b, 2);
  step(a, 0, false);
  step(b, 0, false);
  CHECK(a.enemies[0].phase != b.enemies[0].phase);
  restart(a, 0);
  step(a, NAN, false);
  CHECK(isfinite(a.player.x) && a.seed != 0);
}
int main() {
  startAndSteer();
  volleys();
  pressureRamp();
  fullCapacity();
  collisions();
  heavyEnemy();
  visibleWingHits();
  noFreeClears();
  trenchCollisions();
  forgivingTarget();
  naturalRuns();
  noviceRuns();
  idleLoss();
  deterministicAndBounded();
  printf("Starfighter rules: %d checks passed\n", checks);
}
