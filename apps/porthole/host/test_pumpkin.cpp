#include <assert.h>
#include <math.h>
#include <stdio.h>
#include <string.h>
#include <initializer_list>
#include "games/pumpkin-hide-seek/game.h"
#include "gfx565.h"
#include "host/grip.h"

using namespace pumpkin;
static int checks = 0;
#define CHECK(c) (assert(c), checks++)
static void tick(Round& r, int ms, float roll = 0) {
  for (int t = 0; t < ms; t += STEP_MS) step(r, roll, false);
}
static void rules() {
  Round r;
  tick(r, 1000, 35);
  CHECK(r.x == 80 && r.velocity == 0);
  step(r, 0, true);
  CHECK(r.hidden == 1);
  tick(r, 1000, 1000);
  CHECK(r.x == 80 && r.hidden == 1);
  step(r, 0, true);
  CHECK(r.hidden == -1);
  tick(r, 3000, 1000);
  CHECK(r.x == 136 && fabsf(r.velocity) < 0.01f);
  r = Round{}; r.x = 58;
  step(r, 0, true);
  CHECK(r.hidden == -1);
  r = Round{}; tick(r, 3000);
  CHECK(warning(r) && !searching(r));
  tick(r, 2000);
  CHECK(searching(r) && !warning(r));
  tick(r, 5000);
  CHECK(r.caught && !r.won);
  const uint32_t stopped = r.elapsed;
  tick(r, 5000);
  CHECK(r.elapsed == stopped);
  r = Round{}; step(r, 0, true); tick(r, ROUND_MS);
  CHECK(r.won && !r.caught && r.elapsed == ROUND_MS);
  r = Round{};
  CHECK(r.hidden == -1 && r.exposure == 0 && r.elapsed == 0);
}
static void exposureAndBounds() {
  Round r; r.elapsed = 6800;
  tick(r, 200);
  CHECK(r.exposure > 0 && !r.caught);
  step(r, 0, true);
  CHECK(r.hidden == 1 && r.exposure == 0);
  step(r, 0, true);
  CHECK(r.exposure == STEP_MS && !r.caught);
  r.x = 136; step(r, 0, false);
  CHECK(r.exposure == 0);
  r = Round{}; tick(r, 3000, -1000);
  CHECK(r.x == 24 && r.velocity == 0);
  r = Round{}; tick(r, 300, 500);
  const float speed = r.velocity;
  tick(r, 500);
  CHECK(r.velocity < speed / 10);
  for (float spot : SPOTS) {
    r = Round{}; r.x = spot + 13;
    step(r, 0, true); CHECK(r.hidden >= 0 && r.x == spot);
    tick(r, ROUND_MS, 1000); CHECK(r.won && !r.caught);
  }
}
// Feed the actual tracker, so fresh-page gates and held fingers match hardware.
struct Run {
  Game game;
  InputTracker touch;
  tilt::Grav gravity = grip::FLAT;
  uint32_t ms = 0, wall = 100000;
  void enter() { game.enter({nullptr, nullptr, nullptr, 0, wall, ms}); }
  void frame(bool down = false, int x = 80, int y = 131, uint32_t dt = 10) {
    ms += dt;
    Input in = touch.step(down, x, y, ms);
    in.gx = gravity.x; in.gy = gravity.y; in.gz = gravity.z;
    game.update(wall, ms, in);
  }
  void wait(int n) { for (int i = 0; i < n; i += 10) frame(); }
  void tap(int x = 80, int y = 131) { frame(true, x, y); frame(false, x, y); }
  bool at(const char* screen) { return !strcmp(game.screenName(), screen); }
  void start() {
    enter(); wait(500); tap();
    CHECK(at("pumpkin_calibrate"));
    wait(500); tap();
    CHECK(at("pumpkin_play"));
  }
};
static void grips() {
  const tilt::Grav neutral[] = {grip::FLAT, grip::LEANED, grip::UPRIGHT, grip::FACE_DOWN};
  for (tilt::Grav n : neutral) {
    Run r; r.gravity = n; r.start(); r.wait(600);
    CHECK(r.game.round().x == 80);
    r.gravity = grip::gravityFor(400, 0, n); r.wait(500);
    CHECK(r.game.round().x > 85);
    r.enter(); r.wait(500); r.tap(); r.wait(500); r.tap();
    CHECK(r.at("pumpkin_play"));
    r.wait(600);
    CHECK(r.game.round().x == 80);  // the new grip replaced the old neutral
  }
}
static void calibration() {
  Run r; r.enter(); r.wait(500); r.tap(); r.wait(500);
  r.gravity = {2000, 2000, 2000}; r.tap();
  CHECK(r.at("pumpkin_calibrate"));
  r.gravity = grip::UPRIGHT; r.wait(400);
  CHECK(r.at("pumpkin_play"));
  r.wait(600); CHECK(r.game.round().x == 80);
}
static void gatesAndHome() {
  Run r; r.enter(); r.tap();
  CHECK(r.at("pumpkin_intro"));
  r.wait(500); r.tap(); r.wait(200); r.tap();
  CHECK(r.at("pumpkin_calibrate"));
  r.wait(300); r.tap();
  CHECK(r.at("pumpkin_play"));
  r.tap(); CHECK(r.game.round().hidden == -1);
  r.wait(500); r.tap(); CHECK(r.game.round().hidden == 1);
  r.tap(80, 19); CHECK(!r.game.wantsHome() && r.game.round().hidden == 1);
  for (int i = 0; i < 50; i++) r.frame(true, 80, 19);
  CHECK(!r.game.wantsHome());
  r.frame(true, 40, 60); r.frame(true, 80, 19);
  CHECK(!r.game.wantsHome());
  for (int i = 0; i < 61; i++) r.frame(true, 80, 19);
  CHECK(r.game.wantsHome()); CHECK(!r.game.wantsHome());
  r.frame(); r.enter(); r.wait(500); r.tap(80, 19);
  CHECK(r.game.wantsHome());
}
static void results() {
  Run lost; lost.start();
  while (lost.at("pumpkin_play")) lost.frame();
  CHECK(lost.at("pumpkin_catch"));
  lost.tap(); CHECK(lost.at("pumpkin_catch"));
  lost.wait(600); CHECK(lost.at("pumpkin_icing"));
  lost.wait(1600); CHECK(lost.at("pumpkin_lost"));
  lost.tap(); CHECK(lost.at("pumpkin_lost"));
  lost.wait(500); lost.tap(); CHECK(lost.at("pumpkin_play"));
  CHECK(lost.game.round().elapsed == 0 && lost.game.round().exposure == 0);
  Run won; won.start(); won.wait(500); won.tap(); won.wait(ROUND_MS);
  CHECK(won.at("pumpkin_won"));
  won.wait(500); won.tap(); CHECK(won.at("pumpkin_play"));
  CHECK(won.game.round().hidden == -1);
  const void* data = nullptr; size_t len = 0;
  CHECK(!won.game.takeSave(&data, &len, true));
  CHECK(!won.game.soundOn(won.ms) && !won.game.asleep());
}
static void heldAcrossPages() {
  Run r; r.start();
  while (r.at("pumpkin_play")) r.frame();
  for (int i = 0; i < 300; i++) r.frame(true);
  CHECK(r.at("pumpkin_lost"));
  r.frame(false); CHECK(r.at("pumpkin_lost"));
  r.tap(); CHECK(r.at("pumpkin_play"));
  r.wait(500);
  // A drag onto Home from the action sign is never a Home hold.
  r.frame(true, 80, 131);
  for (int i = 0; i < 70; i++) r.frame(true, 80, 19);
  CHECK(!r.game.wantsHome());
  r.frame(false); CHECK(r.game.round().hidden == -1);
}
static void clocks() {
  Run r; r.ms = UINT32_MAX - 1400; r.start(); r.wait(600);
  CHECK(r.game.round().elapsed == 600);  // crosses millis rollover
  const uint32_t before = r.game.round().elapsed;
  r.wall = 1; r.frame(); r.wall = UINT32_MAX; r.frame();
  CHECK(r.game.round().elapsed == before + 20);
  r.frame(false, 80, 131, 86400000);
  CHECK(r.game.round().elapsed == before + 120);
  r.enter(); CHECK(r.at("pumpkin_intro") && r.game.round().elapsed == 0);
}
static uint16_t pixels[gfx565::W * gfx565::H];
static void auditText() {
  for (int i = 0; i < gfx::textLogCount; i++) {
    const auto b = gfx::textLog[i];
    CHECK(gfx::inCircle(b.x, b.y) && gfx::inCircle(b.x + b.w - 1, b.y));
    CHECK(gfx::inCircle(b.x, b.y + b.h - 1) && gfx::inCircle(b.x + b.w - 1, b.y + b.h - 1));
  }
}
static void crispBlocks() {
  bool crisp = true;
  for (int y = 0; y < 480; y += 3) for (int x = 0; x < 480; x += 3)
    for (int dy = 0; dy < 3; dy++) for (int dx = 0; dx < 3; dx++)
      crisp = crisp && pixels[y * 480 + x] == pixels[(y + dy) * 480 + x + dx];
  CHECK(crisp);
}
static void renderPages() {
  gfx565::target(pixels); gfx::textLogEnabled = true;
  const Page pages[] = {Page::Intro, Page::Calibrate, Page::Play, Page::Catch, Page::Icing, Page::Lost, Page::Won};
  for (Page page : pages) {
    View v; v.page = page;
    gfx::textLogReset(); paint::draw(v); auditText(); crispBlocks();
    CHECK(page == Page::Icing || gfx::textLogCount > 0);
    CHECK(pixels[240 * 480 + 240] != 0);
  }
  for (int hidden = -1; hidden < 3; hidden++) {
    View v; v.page = Page::Play; v.hidden = hidden; v.nearby = hidden;
    v.searching = true; v.warning = true; v.exposure = 0.6f; v.homeHold = 0.5f;
    v.x = hidden < 0 ? 24 : SPOTS[hidden]; v.beamX = v.x; v.velocity = 10;
    v.animationMs = 350; v.elapsed = 7000;
    gfx::textLogReset(); paint::draw(v); auditText();
  }
  View v; v.page = Page::Calibrate; v.ready = true; v.starting = true;
  gfx::textLogReset(); paint::draw(v); auditText();
  v.page = Page::Icing;
  for (uint32_t ms : {400u, 800u, 1590u}) {
    v.pageMs = ms; gfx::textLogReset(); paint::draw(v); auditText();
  }
  gfx::textLogEnabled = false;
}
int main() {
  rules(); exposureAndBounds(); grips(); calibration(); gatesAndHome(); results(); heldAcrossPages(); clocks(); renderPages();
  printf("pumpkin: %d checks passed\n", checks);
}
