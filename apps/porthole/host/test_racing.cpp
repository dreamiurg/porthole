// Rally's app layer: the wheel, every page reachable and drawn, touch rules, pause/resume/replay, and the HUD held
// inside the round glass. The pure race rules are test_racing_rules.cpp.
#include <assert.h>
#include <math.h>
#include <stdio.h>
#include <string.h>
#include <vector>
#include "gfx565.h"
#include "games/racing/game.h"

using namespace racing;
namespace {
tilt::Grav rolled(float degrees, tilt::Grav n = {0, 1000, 0}) {   // positive: the puck turned clockwise
  const float a = degrees * 3.14159265f / 180, c = cosf(a), s = sinf(a);
  return {(int)lroundf(n.x * c + n.y * s), (int)lroundf(n.y * c - n.x * s), n.z};
}

struct Rig {
  Game g;
  InputTracker tracker;
  uint32_t ms = 1000;
  tilt::Grav grav = {0, 1000, 0};
  std::vector<uint16_t> fb = std::vector<uint16_t>(480 * 480);
  Rig() {
    static const Profile who = {0, "Sam", 0, 8, 0};
    gfx565::target(fb.data());
    g.enter(AppEnter{&who, &who, nullptr, 1, 1000000, ms, false});
  }
  void frame(bool down = false, int x = 0, int y = 0, uint32_t dt = 40) {
    ms += dt;
    Input in = tracker.step(down, x, y, ms);
    in.gx = (int16_t)grav.x; in.gy = (int16_t)grav.y; in.gz = (int16_t)grav.z;
    g.update(1000000 + ms / 1000, ms, in);
    g.render();
  }
  void wait(uint32_t t, uint32_t dt = 40) { for (uint32_t e = 0; e < t; e += dt) frame(false, 0, 0, dt); }
  void tap(const ui::Box& b) { wait(500); frame(true, b.x + b.w / 2, b.y + b.h / 2); frame(true, b.x + b.w / 2, b.y + b.h / 2); frame(); }
  void toRace(int track) {
    tap(paint::CARD[track]);
    if (g.screen() == Game::Screen::HowTo) tap(paint::NEXT);
    assert(g.screen() == Game::Screen::Calibrate);
    wait(600);
    tap(paint::NEXT);
    assert(g.screen() == Game::Screen::Race && g.race().phase == Phase::Countdown);
  }
};

bool inside(const ui::Box& b, int margin) {
  for (int k = 0; k < 4; k++) {
    const int x = b.x + (k & 1) * b.w, y = b.y + (k >> 1) * b.h;
    if (!gfx::inCircle(x, y, margin)) return false;
  }
  return true;
}
bool apart(const ui::Box& a, const ui::Box& b) {
  return a.x + a.w + 2 <= b.x || b.x + b.w + 2 <= a.x || a.y + a.h + 2 <= b.y || b.y + b.h + 2 <= a.y;
}

void geometry() {
  using namespace paint;
  assert(inside(PLACE, 2) && apart(PLACE, LAP));   // the place is shown, never tapped
  const ui::Box pages[][6] = {{CARD[0], CARD[1], CARD[2], CARD[3], HOME, HOME},
                              {BACK, NEXT, BACK, NEXT, BACK, NEXT},
                              {LAP, ROCKET_BTN, BOOST_BTN, LAP, LAP, LAP},
                              {RESUME, RECENTER, RESTART, TRACKS_BTN, HOME_BTN, HOME_BTN},
                              {YES, NO, YES, NO, YES, NO},
                              {REPLAY, RESULT_TRACKS, RESULT_HOME, REPLAY, REPLAY, REPLAY}};
  for (const auto& page : pages)
    for (int i = 0; i < 6; i++) {
      assert(page[i].w >= 24 && page[i].h >= 22 && inside(page[i], 2));   // whole 8 mm targets, off the bezel
      for (int j = 0; j < i; j++) if (memcmp(&page[i], &page[j], sizeof page[i])) assert(apart(page[i], page[j]));
    }
}

void wheel() {
  const tilt::Grav n = rolled(25);   // any comfortable neutral grip
  assert(controls::steer(n, n) == 0);
  assert(controls::steer(rolled(2, n), n) == 0);           // inside the 3-degree dead zone
  assert(controls::steer(rolled(10, n), n) > 0.3f);         // clockwise steers right
  assert(controls::steer(rolled(-10, n), n) < -0.3f);        // counterclockwise steers left
  assert(controls::steer(rolled(65, n), n) == 1);           // clamped at full lock
  assert(controls::steer({0, 0, -1000}, n) == 0);           // flat on a table: no steering
  assert(!controls::upright({0, 0, -1000}));
}

void calibrationGate() {
  Rig r;
  r.tap(paint::CARD[0]);
  assert(r.g.screen() == Game::Screen::HowTo);
  r.tap(paint::BACK);
  assert(r.g.screen() == Game::Screen::Tracks);
  r.tap(paint::CARD[1]);
  r.tap(paint::NEXT);
  r.grav = {0, 0, -1000};
  r.wait(800);
  r.tap(paint::NEXT);
  assert(r.g.screen() == Game::Screen::Calibrate);   // flat cannot start a race
  r.tap(paint::BACK);
  assert(r.g.screen() == Game::Screen::Tracks);
  r.grav = rolled(-20);
  r.tap(paint::CARD[1]);   // the lesson was seen this visit: straight to the grip
  assert(r.g.screen() == Game::Screen::Calibrate);
  r.wait(600);
  r.tap(paint::NEXT);
  assert(r.g.screen() == Game::Screen::Race && !strcmp(r.g.screenName(), "race_countdown"));
  assert(r.g.race().track == &TRACK[1]);
}

void countdownAndButtons() {
  Rig r;
  r.toRace(0);
  r.frame(true, 142, 80); r.frame(); r.frame(true, 18, 80); r.frame();   // during the countdown: spent nothing
  r.wait(3200);
  assert(r.g.race().phase == Phase::Racing && r.g.race().inventory[BOOST] == 1 && r.g.race().inventory[ROCKET] == 1);
  for (int i = 0; i < 20; i++) r.frame(true, 142, 80);   // one press, held: one boost
  r.frame();
  assert(r.g.race().inventory[BOOST] == 0 && r.g.race().car[PLAYER].boostMs > 0);
  r.frame(true, 80, 80); for (int i = 0; i < 5; i++) r.frame(true, 18, 80);   // dragged in from the road: nothing
  r.frame();
  assert(r.g.race().inventory[ROCKET] == 1);
  r.frame(true, 18, 80); r.frame();
  assert(r.g.race().inventory[ROCKET] == 0 && r.g.race().rocketsFired == 1);
}

void pauseHoldAndResume() {
  Rig r;
  r.toRace(2);
  r.wait(5000);
  const ui::Box lap = paint::LAP;
  for (int i = 0; i < 10; i++) r.frame(true, lap.x + 4, lap.y + 4);   // 400 ms, then the finger slides off
  for (int i = 0; i < 20; i++) r.frame(true, lap.x + 60, lap.y + 4);
  r.frame();
  assert(r.g.screen() == Game::Screen::Race);
  for (int i = 0; i < 18; i++) r.frame(true, lap.x + 4, lap.y + 4);   // a full 600 ms hold
  assert(r.g.screen() == Game::Screen::Pause);
  const Race frozen = r.g.race();
  for (int i = 0; i < 10; i++) r.frame(true, paint::RESUME.x + 4, paint::RESUME.y + 4);   // the same touch, carried over
  r.frame();
  assert(r.g.screen() == Game::Screen::Pause);
  r.wait(2000);
  assert(!memcmp(&frozen.car, &r.g.race().car, sizeof frozen.car) && frozen.raceMs == r.g.race().raceMs);
  r.tap(paint::RESUME);
  assert(r.g.screen() == Game::Screen::Race);
  r.wait(900);
  assert(r.g.race().raceMs == frozen.raceMs);   // a one-second cue first, then the exact same race goes on
  r.wait(400);
  assert(r.g.race().raceMs > frozen.raceMs);
}

void recenterAndLeave() {
  Rig r;
  r.toRace(3);
  r.wait(4000);
  for (int i = 0; i < 18; i++) r.frame(true, paint::LAP.x + 4, paint::LAP.y + 4);
  r.frame();
  const uint32_t at = r.g.race().raceMs;
  r.tap(paint::RECENTER);
  assert(r.g.screen() == Game::Screen::Calibrate);
  r.tap(paint::BACK);
  assert(r.g.screen() == Game::Screen::Pause);
  r.tap(paint::RECENTER);
  r.grav = rolled(30);
  r.wait(600);
  r.tap(paint::NEXT);
  assert(r.g.screen() == Game::Screen::Race && r.g.race().raceMs == at);
  r.wait(1500);
  assert(fabsf(r.g.steering()) < 0.05f);   // the new grip is the new straight ahead
  for (int i = 0; i < 18; i++) r.frame(true, paint::LAP.x + 4, paint::LAP.y + 4);
  r.frame();
  const ui::Box leave[] = {paint::RESTART, paint::TRACKS_BTN, paint::HOME_BTN};
  for (const ui::Box& b : leave) { r.tap(b); assert(r.g.screen() == Game::Screen::Confirm); r.tap(paint::NO); assert(r.g.screen() == Game::Screen::Pause); }
  r.tap(paint::RESTART); r.tap(paint::YES);
  assert(r.g.screen() == Game::Screen::Race && r.g.race().phase == Phase::Countdown && r.g.race().raceMs == 0);
  r.wait(3500);
  for (int i = 0; i < 18; i++) r.frame(true, paint::LAP.x + 4, paint::LAP.y + 4);
  r.frame();
  r.tap(paint::TRACKS_BTN); r.tap(paint::YES);
  assert(r.g.screen() == Game::Screen::Tracks);
  r.tap(paint::HOME);
  assert(r.g.wantsHome() && !r.g.wantsHome());
}

void gripLost() {
  Rig r;
  r.toRace(0);
  r.wait(4000);
  r.grav = {0, 0, -1000};
  r.wait(200);
  assert(r.g.screen() == Game::Screen::Race);   // a brief wobble is not a lost grip
  r.wait(200);
  assert(r.g.screen() == Game::Screen::Pause && !strcmp(r.g.screenName(), "race_pause"));
  r.tap(paint::RESUME);
  assert(r.g.screen() == Game::Screen::Pause);   // Resume waits for a held wheel
  r.grav = {0, 1000, 0};
  r.tap(paint::RESUME);
  assert(r.g.screen() == Game::Screen::Race);
}

// Fast and slow frames reach the same race: the simulation steps on its own 20 ms clock.
void frameRateInvariant() {
  Rig a, b;
  a.toRace(1); b.toRace(1);
  a.grav = b.grav = rolled(8);
  a.wait(12000, 20);
  b.wait(12000, 60);
  assert(a.g.race().raceMs == b.g.race().raceMs);
  assert(fabsf(a.g.race().car[PLAYER].dist - b.g.race().car[PLAYER].dist) < 2.5f);
}

void finishAndReplay() {
  Rig r;
  r.toRace(0);
  r.wait(500);
  r.g.debugCmd("lastplace");
  r.wait(4000);
  assert(r.g.screen() == Game::Screen::Results && r.g.race().place == CARS && !strcmp(r.g.screenName(), "race_results"));
  r.g.debugPrint();
  r.tap(paint::REPLAY);
  const Race& fresh = r.g.race();
  assert(r.g.screen() == Game::Screen::Race && fresh.phase == Phase::Countdown && fresh.inventory[BOOST] == 1 &&
         fresh.inventory[ROCKET] == 1 && !fresh.car[PLAYER].finished && fresh.finishedCount == 0);
  r.g.debugCmd("lastlap");
  r.wait(4000);
  assert(r.g.screen() == Game::Screen::Results);
  r.tap(paint::RESULT_TRACKS);
  assert(r.g.screen() == Game::Screen::Tracks);
  r.g.debugCmd("go");
  r.g.debugCmd("lastlap");
  r.wait(4000);
  r.tap(paint::RESULT_HOME);
  assert(r.g.wantsHome());
}

// Every glyph drawn on every page stays on the glass.
void textOnGlass() {
  Rig r;
  gfx::textLogEnabled = true;
  auto check = [&] {
    gfx::textLogReset();
    r.frame();
    for (int i = 0; i < gfx::textLogCount; i++) {
      const gfx::TextBox& t = gfx::textLog[i];
      if (!inside({t.x, t.y, t.w, t.h}, 1)) printf("off glass: %s %d %d %d %d\n", r.g.screenName(), t.x, t.y, t.w, t.h);
      assert(inside({t.x, t.y, t.w, t.h}, 1));
    }
  };
  check();
  r.tap(paint::CARD[3]); check();
  r.tap(paint::NEXT); r.wait(600); check();
  r.tap(paint::NEXT); check();
  r.wait(9000); check();
  for (int i = 0; i < 18; i++) r.frame(true, paint::LAP.x + 4, paint::LAP.y + 4);
  r.frame(); check();
  r.tap(paint::HOME_BTN); check();
  gfx::textLogEnabled = false;
  r.g.debugCmd("tracks"); r.g.debugCmd("track 2"); r.g.debugCmd("calibrate");
  assert(r.g.screen() == Game::Screen::Calibrate);
  (void)paint::icon();
  assert(paint::icon().w == 32);
}
}  // namespace

int main() {
  geometry();
  wheel();
  calibrationGate();
  countdownAndButtons();
  pauseHoldAndResume();
  recenterAndLeave();
  gripLost();
  frameRateInvariant();
  finishAndReplay();
  textOnGlass();
  puts("test_racing: ok");
  return 0;
}
