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
  const ui::Box pages[][6] = {{CARD[0], CARD[1], CARD[2], CARD[3], HOME, MODE_CHIP},
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

// Backing out of Recenter and leaving for another track starts that track, never the abandoned race.
void recenterThenLeave() {
  Rig r;
  r.toRace(0);
  r.wait(4000);
  for (int i = 0; i < 18; i++) r.frame(true, paint::LAP.x + 4, paint::LAP.y + 4);
  r.frame();
  r.tap(paint::RECENTER);
  r.tap(paint::BACK);
  r.tap(paint::TRACKS_BTN); r.tap(paint::YES);
  r.tap(paint::CARD[2]);
  r.wait(600);
  r.tap(paint::NEXT);
  assert(r.g.screen() == Game::Screen::Race && r.g.race().phase == Phase::Countdown && r.g.race().track == &TRACK[2]);
}

// The grip page centers on whatever grip is held steady, before Go: no pull left over from the last grip (playtest
// M1). Laying the puck down after a steady grip still keeps Go shut.
void recenterPreview() {
  Rig r;
  r.grav = rolled(10);
  r.toRace(0);
  r.wait(4000);
  for (int i = 0; i < 18; i++) r.frame(true, paint::LAP.x + 4, paint::LAP.y + 4);
  r.frame();
  r.tap(paint::RECENTER);
  r.grav = rolled(0, {0, 707, -707});   // leaned back, upright in the screen plane
  r.wait(800);
  assert(fabsf(r.g.steering()) < 0.01f);
  r.grav = rolled(12, {0, 707, -707});
  r.wait(80);
  assert(r.g.steering() > 0.3f);   // turning from the new grip shows at once
  r.wait(1500);
  assert(r.g.steering() > 0.3f);   // ... and stays: holding a turn does not become the new straight (playtest f1)
  r.grav = {0, 0, -1000};
  r.wait(100);
  r.tap(paint::NEXT);
  assert(r.g.screen() == Game::Screen::Calibrate);
}

void gripLost() {
  Rig r;
  r.toRace(0);
  r.wait(4000);
  const tilt::Grav back = {0, 342, -940};   // leaned back 70 degrees from upright: still racing, still steering
  r.grav = back;
  r.wait(1500);
  assert(r.g.screen() == Game::Screen::Race);
  r.grav = rolled(15, back);
  r.wait(400);
  assert(r.g.steering() > 0.5f);
  r.grav = rolled(80);   // a hard turn far past full lock: racing
  r.wait(1000);
  assert(r.g.screen() == Game::Screen::Race && r.g.steering() > 0.9f);
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
  r.wait(3600);
  assert(r.g.screen() == Game::Screen::Race && r.g.race().phase == Phase::Finished);   // "Finish!" on the line first
  r.wait(1500);
  assert(r.g.screen() == Game::Screen::Results && r.g.race().place == CARS && !strcmp(r.g.screenName(), "race_results"));
  r.g.debugPrint();
  r.tap(paint::REPLAY);
  const Race& fresh = r.g.race();
  assert(r.g.screen() == Game::Screen::Race && fresh.phase == Phase::Countdown && fresh.inventory[BOOST] == 1 &&
         fresh.inventory[ROCKET] == 1 && !fresh.car[PLAYER].finished && fresh.finishedCount == 0);
  r.g.debugCmd("lastlap");
  r.wait(5500);
  assert(r.g.screen() == Game::Screen::Results);
  r.tap(paint::RESULT_TRACKS);
  assert(r.g.screen() == Game::Screen::Tracks);
  r.g.debugCmd("go");
  r.g.debugCmd("lastlap");
  r.wait(5500);
  r.tap(paint::RESULT_HOME);
  assert(r.g.wantsHome());
}

// The save: a sealed blob loads back (older ones: saveMigration); a wrong crc, size or magic
// is ignored (no times, never half a record); only a better time replaces one.
void saveBlob() {
  Save s{};
  s.best[2] = {95000, 30100};
  s.level[2] = -5; s.level[3] = 3; s.mode = HARD;
  seal(s);
  Save back{};
  assert(loadBlob(&s, sizeof s, back) && back.best[2].raceMs == 95000 && back.best[2].lapMs == 30100);
  assert(back.version == 2 && back.level[2] == -5 && back.level[3] == 3 && back.mode == HARD);
  Save bad = s; bad.best[2].raceMs = 1;
  assert(!loadBlob(&bad, sizeof bad, back));
  assert(!loadBlob(&s, sizeof s - 4, back));   // a v2 blob cut to v1's size is not a v1 blob
  Best b{};
  assert(record(b, 100000, 32000) == (BEST_RACE | BEST_LAP) && b.raceMs == 100000);
  assert(!record(b, 101000, 33000) && b.raceMs == 100000 && b.lapMs == 32000);   // slower: kept
  assert(record(b, 102000, 31000) == BEST_LAP && b.raceMs == 100000 && b.lapMs == 31000);   // a lap, not a race
}

// Three kids share the board: everyone's best on the track, fastest first, this kid's row marked; a slower race
// keeps the record, a faster one becomes the new best and is handed to the shell once.
void leaderboard() {
  static const Profile all[3] = {{0, "Zoe", 0, 9, 0}, {1, "Sam", 1, 7, 0}, {2, "Kai", 2, 6, 0}};
  Save zoe{}, kai{};
  zoe.best[0] = {91200, 29800}; seal(zoe);
  kai.best[0] = {99000, 32000}; seal(kai);
  const uint8_t junk[8] = {1, 2, 3};
  const SaveSlot slots[3] = {{&zoe, sizeof zoe}, {junk, sizeof junk}, {&kai, sizeof kai}};
  Rig r;
  r.g.enter(AppEnter{&all[1], all, slots, 3, 1000000, r.ms, false});
  const void* data = nullptr; size_t len = 0;
  assert(!r.g.takeSave(&data, &len, true));   // nothing new yet
  r.toRace(0);
  r.wait(500);
  r.g.debugCmd("lastplace");
  r.wait(5200);
  assert(r.g.screen() == Game::Screen::Results);
  assert(r.g.takeSave(&data, &len, true) && len == sizeof(Save) && !r.g.takeSave(&data, &len, true));
  Save mine{};
  assert(loadBlob(data, len, mine) && mine.best[0].raceMs == r.g.race().car[PLAYER].finishMs);
  assert(mine.level[0] == -2 && mine.level[1] == 0 && mine.level[3] == 0);   // fourth: down two; unraced: never below 0
  gfx::textLogEnabled = true; gfx::textLogReset();
  r.frame();
  gfx::textLogEnabled = false;
  for (int i = 0; i < gfx::textLogCount; i++) assert(inside({gfx::textLog[i].x, gfx::textLog[i].y, gfx::textLog[i].w, gfx::textLog[i].h}, 1));
  r.tap(paint::REPLAY);
  r.g.debugCmd("lastplace");   // the same short fixture: no better time, but the level moved, so a save
  r.wait(5200);
  assert(r.g.takeSave(&data, &len, true) && loadBlob(data, len, mine) && mine.best[0].raceMs == r.g.race().car[PLAYER].finishMs);
  assert(mine.level[0] == -4 && mine.level[1] == 0);
  r.g.debugCmd("level -16");
  assert(r.g.takeSave(&data, &len, true));   // the fixture's level is saved
  r.g.debugCmd("lastplace");   // at the floor a loss changes nothing: nothing to save
  r.wait(5200);
  assert(r.g.race().level == LEVEL_MIN && !r.g.takeSave(&data, &len, true));
  r.g.debugCmd("level 99");   // clamped to the top; a runaway win there stays there
  r.g.debugCmd("lastlap");
  r.wait(5200);
  assert(r.g.takeSave(&data, &len, true) && loadBlob(data, len, mine) && mine.level[0] == LEVEL_MAX);
  assert(mine.level[1] == LEVEL_SEED_MAX && mine.level[3] == LEVEL_SEED_MAX);   // a new track still starts gently
  r.g.debugPrint();
}

// The difficulty chip cycles Auto, Easy, Normal, Hard and back, saving at once. A fixed setting races at its own
// level and leaves the adaptive levels alone; the setting is the kid's own, back on the next entry.
void modes() {
  static const Profile all[2] = {{0, "Zoe", 0, 9, 0}, {1, "Sam", 1, 6, 0}};
  Save zoe{};
  zoe.mode = EASY; zoe.level[0] = 5; zoe.best[0] = {91000, 30000};
  seal(zoe);
  const SaveSlot slots[2] = {{&zoe, sizeof zoe}, {nullptr, 0}};
  Rig r;
  r.g.enter(AppEnter{&all[0], all, slots, 2, 1000000, r.ms, false});
  r.toRace(0);
  assert(r.g.race().level == EASY_LEVEL);   // Zoe's own setting
  r.wait(500);
  r.g.debugCmd("lastlap");   // a win on Easy: a record, but her adaptive level stays 5
  r.wait(5200);
  const void* data = nullptr; size_t len = 0;
  Save mine{};
  assert(r.g.screen() == Game::Screen::Results && r.g.race().place == 1);
  assert(r.g.takeSave(&data, &len, true) && loadBlob(data, len, mine) && mine.level[0] == 5 && mine.mode == EASY);
  r.tap(paint::RESULT_TRACKS);
  const uint8_t order[] = {NORMAL, HARD, AUTO, EASY};
  for (uint8_t want : order) {
    r.tap(paint::MODE_CHIP);
    assert(r.g.saveUrgent());   // checkpointed at once, not on the 5 s cadence
    assert(r.g.screen() == Game::Screen::Tracks && r.g.takeSave(&data, &len, true) && loadBlob(data, len, mine) && mine.mode == want);
  }
  r.tap(paint::MODE_CHIP); r.tap(paint::MODE_CHIP);   // Hard
  r.g.debugCmd("lastplace");
  assert(r.g.race().level == HARD_LEVEL[0]);
  r.wait(5200);
  assert(r.g.takeSave(&data, &len, true) && loadBlob(data, len, mine) && mine.level[0] == 5 && mine.mode == HARD);
  Rig sam;   // Sam has no save: Auto, at his own level
  sam.g.enter(AppEnter{&all[1], all, slots, 2, 1000000, sam.ms, false});
  sam.toRace(0);
  assert(sam.g.race().level == 0);
  sam.g.debugPrint();
}

// Older and newer blobs: a v1 blob (before levels) loads with every level 0 and Auto; a setting this firmware does
// not know loads as Auto.
void saveMigration() {
  Save s{}, back{};
  s.level[2] = -5; s.mode = MODES;
  seal(s);
  assert(loadBlob(&s, sizeof s, back) && back.mode == AUTO && back.level[2] == -5);
  struct V1 { uint32_t magic; uint16_t version, size; Best best[TRACKS]; uint32_t crc; } v1 = {SAVE_MAGIC, 1, 44, {}, 0};
  static_assert(sizeof(V1) == SAVE_V1_SIZE, "the shipped v1 layout");
  v1.best[1] = {88000, 28000};
  v1.crc = os::crc32(&v1, sizeof v1 - 4);
  Save old{};
  old.level[1] = 9; old.mode = EASY;   // must be overwritten, not merged
  assert(loadBlob(&v1, sizeof v1, old) && old.best[1].raceMs == 88000 && old.version == SAVE_VERSION && old.mode == AUTO);
  for (int8_t l : old.level) assert(l == 0);
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
  assert(paint::icon().w == 38 && paint::icon().h == 28);
}

// A race with the player alone `ahead` m before the middle of feature f, at x; rivals parked far behind.
Race alone(int track, const Feature& o, float ahead, float x, int level) {
  Race r;
  start(r, TRACK[track], 5, level);
  r.phase = Phase::Racing;
  for (Car& c : r.car) { c.dist = -500; c.speed = 0; }
  Car& p = r.car[PLAYER];
  p.dist = featureAt(TRACK[track], o) + featureLen(TRACK[track], o) / 2 - ahead;
  p.x = x; p.speed = 1;
  return r;
}
// Every obstacle hits where it is drawn: it is drawn as wide as it hits, a car passing it with a few pixels of clear
// road between the two pictures never hits, and one overlapping it by a few pixels hits once, while the obstacle is
// drawn level with the car (its middle between the car's dist and the top of the car's picture).
void obstaclesHitWhereDrawn() {
  constexpr float PX = 1 / 217.5f;   // a pixel at the car's depth, in road half-widths (render.cpp ROAD_SLOPE * CAR_DEPTH)
  assert(fabsf(paint::carHalf() - CAR_HALF_W) < 2 * PX && fabsf(paint::carReach() - CAR_REACH) < 0.05f);
  for (int t = 0; t < TRACKS; t++)
    for (int f = 0; f < TRACK[t].nFeatures; f++) {
      const Feature& o = TRACK[t].features[f];
      if (o.kind != Kind::Obstacle) continue;
      const float half = paint::obstacleHalf(t, o), ox = o.x / 100.0f;
      assert(fabsf(half - o.w / 100.0f) < 2 * PX);
      for (int side = -1; side <= 1; side += 2)
        for (float gap : {4 * PX, -4 * PX}) {
          const float x = ox + side * (half + paint::carHalf() + gap);
          Race r = alone(t, o, 15, x, LEVEL_MAX);
          for (int n = 0; n < 45; n++) {
            r.car[PLAYER].x = x; r.car[PLAYER].vx = 0;   // held on its line
            const uint32_t before = r.impacts;
            step(r, Controls{});
            const float ahead = featureAt(TRACK[t], o) + featureLen(TRACK[t], o) / 2 - r.car[PLAYER].dist;
            if (r.impacts != before) assert(ahead >= -0.01f && ahead <= paint::carReach());
          }
          assert(r.impacts == (gap > 0 ? 0u : 1u));
        }
    }
}
// A Pro hazard is drawn from its rung up and not below it.
void proHazardsDrawn() {
  std::vector<uint16_t> fb(480 * 480), below(480 * 480);
  gfx565::target(fb.data());
  auto frame = [&](int t, const Feature& o, int rung) {
    const Race r = alone(t, o, 12, o.x > 0 ? -0.6f : 0.6f, rung - LEVEL_SHIFT[t]);
    assert(r.rung == rung);
    paint::View v;
    v.page = paint::Page::Race; v.race = &r; v.track = t;
    paint::draw(v);
  };
  for (int t = 0; t < TRACKS; t++)
    for (int f = 0; f < TRACK[t].nFeatures; f++) {
      const Feature& o = TRACK[t].features[f];
      if (!o.rung) continue;
      frame(t, o, o.rung - 1);
      below = fb;
      frame(t, o, o.rung);
      int changed = 0;
      for (int i = 0; i < 480 * 480; i++) changed += fb[i] != below[i];
      assert(changed > 100);   // the rungs differ only in this hazard: rivals are parked and alike from 18 up
      bool other = false;      // with no lower Pro hazard in view, the rung below looks like the top of the ladder
      for (int g = 0; g < TRACK[t].nFeatures; g++) {
        const float d = featureAt(TRACK[t], TRACK[t].features[g]) - featureAt(TRACK[t], o);
        other = other || (TRACK[t].features[g].rung && TRACK[t].features[g].rung < o.rung && d > -30 && d < 250);
      }
      if (other) continue;
      frame(t, o, LADDER_RUNGS);
      assert(fb == below);
    }
}
}  // namespace

int main() {
  obstaclesHitWhereDrawn();
  proHazardsDrawn();
  geometry();
  wheel();
  calibrationGate();
  countdownAndButtons();
  pauseHoldAndResume();
  recenterAndLeave();
  recenterThenLeave();
  recenterPreview();
  gripLost();
  frameRateInvariant();
  finishAndReplay();
  textOnGlass();
  saveBlob();
  saveMigration();
  leaderboard();
  modes();
  puts("test_racing: ok");
  return 0;
}
