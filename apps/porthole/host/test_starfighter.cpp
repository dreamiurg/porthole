// Controller proof: the physical upright bank, steady calibration and touch lifecycle.
#include "host/check.h"
#include <initializer_list>
#include <math.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "games/starfighter/controls.h"
#include "games/starfighter/game.h"

using namespace starfighter;
static int checks = 0;

static tilt::Grav rolled(float degrees, tilt::Grav n = {0, 1000, 0}) {
  const float a = degrees * 3.14159265f / 180, c = cosf(a), s = sinf(a);
  return {(int)lroundf(n.x * c + n.y * s), (int)lroundf(n.y * c - n.x * s), n.z};
}
static void banking() {
  for (tilt::Grav n : {tilt::Grav{0, 1000, 0}, tilt::Grav{200, 900, -380}, tilt::Grav{-250, 900, 350}}) {
    CHECK(fabsf(controls::bank(n, n)) < 0.001f);
    CHECK(controls::bank(rolled(2, n), n) == 0);
    CHECK(controls::bank(rolled(10, n), n) > 0.35f);
    CHECK(controls::bank(rolled(-10, n), n) < -0.35f);
    CHECK(controls::bank(rolled(20, n), n) > 0.99f);
    CHECK(controls::bank(rolled(65, n), n) == 1);
    CHECK(controls::bank(rolled(-65, n), n) == -1);
  }
  CHECK(controls::bank({0, 0, -1000}, {0, 0, -1000}) == 0);
  CHECK(controls::bank({0, 0, 0}, {0, 1000, 0}) == 0);
  CHECK(controls::bank({10, 10, -1000}, {0, 1000, 0}) == 0);
}
static void calibration() {
  controls::Calibration c;
  tilt::Grav n{};
  c.add({0, 1000, 0}, 0);
  CHECK(!c.get(&n));
  for (uint32_t ms = 20; ms <= 400; ms += 20) c.add({10, 999, -10}, ms);
  CHECK(c.get(&n) && abs(n.x - 10) <= 1 && n.y == 999);
  c.add({0, 0, 0}, 420);
  CHECK(!c.get(&n));
  for (uint32_t ms = 440; ms <= 1000; ms += 20) c.add({0, 0, -1000}, ms);
  CHECK(!c.get(&n));
  c.add({0, 1000, 0}, 5000);
  CHECK(!c.get(&n));
  for (uint32_t ms = 5020; ms <= 5500; ms += 20) c.add(rolled(ms % 40 ? 12 : -12), ms);
  CHECK(!c.get(&n));
  for (uint32_t ms = 5520; ms <= 5920; ms += 20) c.add({0, 1000, 0}, ms);
  CHECK(c.get(&n));
  c.add({0, 1000, 0}, 5000);
  CHECK(!c.get(&n));  // a backward sensor timestamp cannot manufacture steady time
  c.add({0, 1000, 0}, 20000);
  CHECK(!c.get(&n));
}

struct Session {
  Game game;
  InputTracker finger;
  uint32_t ms = 0, now = 1790000000;
  tilt::Grav gravity = {0, 1000, 0};
  Session() {
    const AppEnter e = {nullptr, nullptr, nullptr, 0, now, ms, true};
    game.enter(e);
  }
  void frame(bool down = false, int x = 80, int y = 80, uint32_t dt = 20) {
    ms += dt;
    Input in = finger.step(down, x, y, ms);
    in.gx = gravity.x; in.gy = gravity.y; in.gz = gravity.z;
    game.update(now, ms, in);
  }
  void wait(uint32_t duration = 500) { for (uint32_t i = 0; i < duration; i += 20) frame(); }
  void tap(int x = 80, int y = 120) { frame(true, x, y); frame(false, x, y); }
  bool on(const char* name) const { return !strcmp(game.screenName(), name); }
  void start() { wait(); tap(); wait(); tap(); wait(); tap(); wait(); }
};
static void naturalFlow() {
  Session s;
  CHECK(s.on("sf_title"));
  s.tap();
  CHECK(s.on("sf_title"));  // launcher touch cannot begin a run
  s.wait(); s.tap();
  CHECK(s.on("sf_calibrate"));
  s.tap();
  CHECK(s.on("sf_calibrate"));
  s.wait(); s.tap();
  CHECK(s.on("sf_brief"));
  s.tap();
  CHECK(s.on("sf_brief"));
  s.wait(); s.tap();
  CHECK(s.on("sf_squadron"));
  CHECK(s.game.run().lives == 3 && s.game.run().volleys == 0);
  s.frame(true, 80, 80); s.wait();
  CHECK(s.game.run().volleys == 0);
  s.wait(); s.tap(80, 80);
  CHECK(s.game.run().volleys == 1);
  const void* data = nullptr; size_t size = 0;
  CHECK(!s.game.takeSave(&data, &size, true));
  CHECK(!s.game.soundOn(s.ms) && !s.game.asleep() && s.game.tint() == TINT_DAY);
  CHECK(s.game.surface() == SURFACE_RGB565);
}
static void invalidGrip() {
  Session s;
  s.gravity = {0, 0, -1000};
  s.wait(); s.tap(); s.wait(); s.tap();
  CHECK(s.on("sf_calibrate"));
  s.gravity = {0, 0, 0}; s.wait(); s.tap();
  CHECK(s.on("sf_calibrate"));
  s.gravity = {200, 900, -380}; s.wait(); s.tap();
  CHECK(s.on("sf_brief"));
  Session stale; stale.game.debugCmd("calibrate"); stale.wait();
  stale.frame(false, 80, 80, 10000); stale.tap();
  CHECK(stale.on("sf_calibrate"));  // an old steady grip cannot authorize a new sensor sample after a pause
  stale.wait(); stale.tap();
  CHECK(stale.on("sf_brief"));
}
static void queuedTaps() {
  Session s; s.start();
  CHECK(s.on("sf_squadron"));
  const uint32_t shots = s.game.run().volleys;
  s.frame(true, 35, 80, 1); s.frame(false, 35, 80, 1);
  CHECK(s.game.run().volleys == shots);
  s.frame(false, 35, 80, 18);
  CHECK(s.game.run().volleys == shots + 1);
  s.frame(true, 125, 80, 1); s.frame(false, 125, 80, 1);
  s.frame(true, 80, 21, 1); s.frame(false, 80, 21, 1);
  s.frame(false, 80, 21, 16); s.frame();
  CHECK(s.game.run().volleys == shots + 3);  // even a short tap on the held exit fires
  CHECK(!s.game.wantsHome());
  for (int i = 0; i < 40; ++i) s.frame(true, 80, 80);
  s.frame(false, 80, 80);
  CHECK(s.game.run().volleys == shots + 3);  // held fingers never autofire
}
static void steeringAndClocks() {
  Session s; s.start();
  s.gravity = rolled(20); s.wait(400);
  CHECK(s.game.run().player.x > 270 && s.game.steering() > 0.9f);
  s.gravity = rolled(-20); s.wait(400);
  CHECK(s.game.steering() < -0.9f);
  s.gravity = {0, 1000, 0}; s.wait(500);
  CHECK(fabsf(s.game.steering()) < 0.05f);
  const uint32_t clock = s.game.run().ms;
  s.frame(false, 80, 80, 100000);
  CHECK(s.game.run().ms == clock + MAX_FRAME_MS);
  const uint32_t after = s.game.run().ms;
  s.ms -= 500; s.now -= 86400; s.frame();
  CHECK(s.game.run().ms == after);
  s.frame();
  CHECK(s.game.run().ms == after + STEP_MS);
}
static void backEverywhere() {
  const char* pages[] = {"title", "calibrate", "brief", "victory", "defeat"};
  for (const char* p : pages) {
    Session s; s.game.debugCmd(p); s.wait(); s.tap(80, 144);
    CHECK(s.game.wantsHome());
    CHECK(!s.game.wantsHome());
  }
  const char* stages[] = {"squadron", "waveclear", "entry", "trench", "target", "torpedo", "explosion"};
  for (const char* p : stages) {
    Session s; s.game.debugCmd(p); s.wait();
    s.tap(80, 21);
    CHECK(!s.game.wantsHome());
    Session h; h.game.debugCmd(p); h.wait();
    for (int i = 0; i < 32; ++i) h.frame(true, 80, 21);
    CHECK(h.game.wantsHome());
  }
}
static void heldExitGuard() {
  Session s; s.start();
  for (int i = 0; i < 25; ++i) s.frame(true, 80, 21);
  s.frame(true, 110, 70);
  s.frame(true, 80, 21);
  s.wait();
  CHECK(!s.game.wantsHome());
  for (int i = 0; i < 20; ++i) s.frame(true, 110, 70);
  for (int i = 0; i < 40; ++i) s.frame(true, 80, 21);
  CHECK(!s.game.wantsHome());  // a hold begun outside the sign cannot slide into it
  s.frame(false); s.frame(true, 80, 21);
  s.frame(true, 80, 21, 50000);
  CHECK(!s.game.wantsHome());  // waking after a pause cannot complete the hold
}
static void replay() {
  for (const char* result : {"victory", "defeat"}) {
    Session s; s.game.debugCmd(result);
    s.tap();
    CHECK(!strcmp(s.game.screenName(), !strcmp(result, "victory") ? "sf_victory" : "sf_defeat"));
    s.wait(); s.tap();
    CHECK(s.on("sf_calibrate"));
    s.wait(); s.tap(); s.wait(); s.tap();
    CHECK(s.on("sf_squadron"));
    CHECK(s.game.run().lives == 3 && s.game.run().score == 0 && s.game.run().volleys == 0);
  }
}
static void stageTransitions() {
  Session s; s.game.debugCmd("waveclear");
  s.wait();
  for (int i = 0; i < (int)WAVE_CLEAR_MS / 20 + 1; ++i) s.frame(true, 80, 80);
  CHECK(s.on("sf_squadron"));
  s.frame(false, 80, 80);
  CHECK(s.game.run().volleys == 0);
  s.game.debugCmd("entry"); s.wait(TRENCH_ENTRY_MS + 20);
  CHECK(s.on("sf_trench"));
  s.game.debugCmd("torpedo"); s.wait(TORPEDO_MS + 20);
  CHECK(s.on("sf_explosion"));
  s.wait(EXPLOSION_MS + 20);
  CHECK(s.on("sf_victory"));
}
int main() {
  banking(); calibration(); naturalFlow(); invalidGrip(); queuedTaps(); steeringAndClocks();
  backEverywhere(); heldExitGuard(); replay(); stageTransitions();
  printf("Starfighter controls: %d checks\n", checks);
}
