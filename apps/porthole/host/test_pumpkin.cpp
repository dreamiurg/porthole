#include "host/check.h"
#include <math.h>
#include <stdio.h>
#include <string.h>
#include <initializer_list>
#include <vector>
#include "games/pumpkin-hide-seek/game.h"
#include "games/pumpkin-hide-seek/ghost.h"
#include "gfx565.h"
#include "host/grip.h"

using namespace pumpkin;
static int checks = 0;
static void tick(Round& r, int ms, float roll = 0) {
  for (int t = 0; t < ms; t += STEP_MS) step(r, roll, false);
}
// Hold the round in a phase until it ends, so a test can stage one exact Grandma move.
static void stage(Round& r, Move move, int spot = -1, bool fromRight = true) {
  r.phase = Phase::Warn; r.move = move; r.spot = spot; r.fromRight = fromRight;
  r.phaseMs = 0; r.phaseLen = 1000; r.moves = 1;
}
static void controls() {
  Round r;
  tick(r, 1000, 35);
  CHECK(r.x == 80 && r.velocity == 0);  // inside the dead zone
  step(r, 0, true);
  CHECK(r.hidden == 1);
  tick(r, 1000, 1000);
  CHECK(r.x == 80 && r.hidden == 1);  // tilt never ejects
  step(r, 0, true);
  CHECK(r.hidden == -1);
  r = Round{}; r.x = 58; step(r, 0, true);
  CHECK(r.hidden == -1);
  r = Round{}; tick(r, 3000, -1000);
  CHECK(r.x == 24 && r.velocity == 0);
  r = Round{}; tick(r, 300, 500);
  const float speed = r.velocity;
  tick(r, 500);
  CHECK(r.velocity < speed / 10);
  for (float spot : SPOTS) {
    r = Round{}; r.x = spot + 13;
    step(r, 0, true); CHECK(r.hidden >= 0 && r.x == spot);
  }
}
static void openingSweep() {
  Round r = newRound(12345);
  tick(r, 2990); CHECK(!warning(r) && !searching(r));
  tick(r, 20); CHECK(warning(r) && r.move == Move::Sweep && checking(r) < 0);
  tick(r, 2000); CHECK(searching(r) && beamX(r) > 130);  // from Grandma's side
  tick(r, 5000); CHECK(r.caught && !r.won);  // standing in the open at the table
  const uint32_t stopped = r.elapsed;
  tick(r, 5000); CHECK(r.elapsed == stopped);
  r = newRound(12345); step(r, 0, true); tick(r, 10000);
  CHECK(!r.caught && !searching(r));  // hidden through a whole sweep
}
static void candyAndDoor() {
  Round r = newRound(7);
  for (int i = 0; i < 2; i++) CHECK(r.candy[i] > SPOTS[i] + 13 && r.candy[i] < SPOTS[i + 1] - 13);  // open floor
  CHECK(r.candy[2] >= 130 && r.candy[2] <= 136);  // out by the plant, reachable at the right wall
  r.phaseLen = 600000;  // a quiet house
  tick(r, 4000, 1000);
  CHECK(r.x == 136 && r.got == 6 && r.sinceCandy < 4000 && !doorOpen(r));
  tick(r, 6000, -1000);  // back over the last candy and out the door
  CHECK(r.got == ALL_CANDY && r.won && !r.caught);
  r = newRound(7); r.phaseLen = 600000; r.x = 30;
  tick(r, 1000, -1000);
  CHECK(r.x == 24 && !r.won);  // the door stays shut until every candy is in
  r = newRound(7); r.x = r.candy[0]; r.hidden = 0; tick(r, 100);
  CHECK(r.got == 0);  // a hidden pumpkin picks nothing up
  r = newRound(7); r.got = ALL_CANDY; r.x = 30; step(r, 0, true); tick(r, 1000, -1000);
  CHECK(r.hidden == 0 && !r.won);  // hiding in the chair by the door is not leaving
}
static void spotCheck() {
  for (int spot = 0; spot < 3; spot++) {
    Round r; r.x = SPOTS[spot]; r.hidden = spot;
    stage(r, Move::Check, spot);
    CHECK(warning(r) && checking(r) == spot && beamX(r) == SPOTS[spot]);
    tick(r, 1000 + CATCH_MS - 2 * STEP_MS); CHECK(searching(r) && !r.caught);
    tick(r, STEP_MS); CHECK(r.caught);  // she looks right behind it
    r = Round{}; r.x = SPOTS[(spot + 1) % 3]; r.hidden = (spot + 1) % 3;
    stage(r, Move::Check, spot);
    tick(r, 3000); CHECK(!r.caught);  // a different hiding place is safe
  }
  Round r; r.x = SPOTS[1]; r.hidden = 1;
  stage(r, Move::Check, 1);
  tick(r, 500); step(r, 0, true);
  tick(r, 1500, 1000);  // came out during the warning and rolled away
  CHECK(!r.caught && r.x > SPOTS[1] + 20);
}
static void sweepsAndPhew() {
  Round r; r.x = SPOTS[1]; r.hidden = 1;
  stage(r, Move::Sweep);
  tick(r, 1000 + 2500); CHECK(r.covered && !r.caught && r.phew == 0);
  tick(r, 1500); CHECK(!r.covered && r.phew > 0 && !r.caught);
  tick(r, PHEW_MS); CHECK(r.phew == 0);
  r = Round{}; r.x = SPOTS[2]; r.hidden = 2; stage(r, Move::TurnBack);
  tick(r, 1000 + 2000); CHECK(beamX(r) < 80 && beamX(r) > 60);  // turns back short of the chair
  tick(r, 1990); CHECK(beamX(r) > 130);
  r = Round{}; r.x = 30; stage(r, Move::TurnBack);
  tick(r, 5000); CHECK(!r.caught);  // the far end is safe from a turn-back
  r = Round{}; stage(r, Move::Fast, -1, false);
  tick(r, 1000); CHECK(searching(r) && r.phaseLen < 5000 && beamX(r) == 24);
  r = Round{}; r.x = 24; stage(r, Move::Fast, -1, false);
  tick(r, 1000 + 400); CHECK(r.exposure > 0);
  r.hidden = 0; r.x = SPOTS[0]; tick(r, 3000);
  CHECK(!r.caught && r.exposure == 0);  // ducking mid-exposure saves it
}
// Wiggling hard while hidden rustles the spot: Grandma's next move checks it. It never ejects.
static void rustle() {
  Round r = newRound(5); r.x = SPOTS[1]; r.hidden = 1; r.phaseLen = 600000;  // a long quiet rest
  tick(r, RUSTLE_MS - 100, 1000); CHECK(r.heard < 0 && r.rustle > 0);  // a short wiggle is fine
  tick(r, 300); CHECK(r.rustle == 0);                                    // holding still calms it
  tick(r, 1000, RUSTLE_MG - 50); CHECK(r.rustle == 0 && r.heard < 0);    // a gentle tilt is not a rustle
  tick(r, RUSTLE_MS, -1000); CHECK(r.heard == 1 && r.hidden == 1 && r.x == SPOTS[1]);
  r.moves = 3; r.phaseMs = r.phaseLen - STEP_MS; step(r, 0, false);
  CHECK(warning(r) && r.move == Move::Check && checking(r) == 1 && r.phaseLen == 3500 && r.heard < 0);
  r = newRound(5); r.phaseLen = 600000; tick(r, 2000, 1000);
  CHECK(r.heard < 0 && r.rustle == 0);  // rolling in the open is not rustling
  r = newRound(5); r.x = SPOTS[0]; r.hidden = 0; tick(r, RUSTLE_MS + 100, 1000);
  CHECK(r.heard == 0);
  tick(r, 3000 - RUSTLE_MS - 100); CHECK(warning(r) && r.move == Move::Sweep && r.heard == 0);  // the opener still teaches
}
// The ghost at the window knocks something over once a round: Grandma goes to look.
static void ghostTap() {
  Round r; r.x = 30; stage(r, Move::Sweep);
  step(r, 0, false, true);
  CHECK(r.ghostUsed && r.phase == Phase::Rest && r.phaseLen == GHOST_REST_MS && r.distract > 0);
  tick(r, GHOST_REST_MS - 100); CHECK(!warning(r) && !searching(r));
  const uint32_t len = r.phaseLen;
  step(r, 0, false, true); CHECK(r.phaseLen == len);  // only once a round
  r = Round{}; r.x = 130; stage(r, Move::Sweep); tick(r, 1000 + 100); CHECK(r.exposure > 0);
  step(r, 0, false, true); CHECK(r.exposure == 0 && !searching(r) && !r.caught);  // a mid-search rescue
  r = Round{}; tick(r, 1000); step(r, 0, false, true);
  CHECK(r.phase == Phase::Rest && r.phaseLen == FIRST_REST_MS + GHOST_EXTRA_MS);  // a quiet house stays quiet longer
  tick(r, DISTRACT_MS); CHECK(r.distract == 0);
  tick(r, FIRST_REST_MS + GHOST_EXTRA_MS - 1000 - DISTRACT_MS);
  CHECK(warning(r) && r.move == Move::Sweep && r.fromRight);  // still the teaching opener
  r = Round{}; r.distract = 5; step(r, 0, false); CHECK(r.distract == 0);  // saturates, never wraps
}
// A ghost during a warning takes back the announced move: the opener and a heard check survive the rest.
static void ghostUndo() {
  Round r = newRound(4); tick(r, FIRST_REST_MS + 500); CHECK(warning(r) && r.moves == 1);
  step(r, 0, false, true); tick(r, GHOST_REST_MS);
  CHECK(warning(r) && r.move == Move::Sweep && r.fromRight && r.moves == 1);
  r = newRound(4); r.x = SPOTS[1]; r.hidden = 1; r.heard = 1; r.moves = 3; r.phaseMs = r.phaseLen - STEP_MS;
  step(r, 0, false); CHECK(warning(r) && checking(r) == 1 && r.heard < 0);
  step(r, 0, false, true); CHECK(r.phase == Phase::Rest && r.heard == 1);
  tick(r, GHOST_REST_MS); CHECK(warning(r) && r.move == Move::Check && checking(r) == 1 && r.heard < 0);
}
// A simple player: fetch the far candy, then head for the door, hiding whenever Grandma comes.
struct Choice { float roll; bool action; };
// A fidgety bot keeps tilting while hidden, so its spot rustles.
static Choice bot(const Round& r, bool fidget = false) {
  const float toward = r.got & 4 ? -500 : 500;
  const bool danger = warning(r) || searching(r);
  if (r.hidden >= 0) {  // holds still while hidden, so it never rustles
    const bool leave = !danger || checking(r) == r.hidden;
    return {leave || fidget ? toward : 0, leave};
  }
  if (danger && nearby(r) >= 0 && nearby(r) != checking(r)) return {0, true};
  return {toward, false};
}
// Plays 100 seeded rounds deciding on what it saw `lateMs` ago, like a kid reacting.
static int wins(uint32_t lateMs, uint32_t* slowest, bool fidget = false) {
  const uint32_t lag = lateMs / STEP_MS + 1;
  std::vector<Round> seen(lag);
  int won = 0;
  for (uint32_t seed = 1; seed <= 100; seed++) {
    Round r = newRound(seed);
    for (uint32_t i = 0; i < lag; i++) seen[i] = r;
    for (uint32_t n = 0; !r.won && !r.caught && r.elapsed < 180000; n++) {
      seen[n % lag] = r;
      const Choice c = bot(seen[(n + 1) % lag], fidget);
      step(r, c.roll, c.action && r.hidden == seen[(n + 1) % lag].hidden);
    }
    if (r.won) { won++; *slowest = r.elapsed > *slowest ? r.elapsed : *slowest; }
  }
  return won;
}
static void winnable() {
  uint32_t quick = 0, late = 0, fidgety = 0;
  const int sharp = wins(0, &quick), kid = wins(700, &late), shaky = wins(0, &fidgety, true);
  printf("pumpkin bot: %d/100 wins (slowest %us), 0.7s late: %d/100 (slowest %us), fidgety: %d/100 (slowest %us)\n",
         sharp, (unsigned)(quick / 1000), kid, (unsigned)(late / 1000), shaky, (unsigned)(fidgety / 1000));
  CHECK(sharp >= 95 && kid >= 90 && shaky >= 90);
}
static void variety() {
  int moves[4] = {}, spots[3] = {}, followed = 0, picks = 0;
  for (uint32_t seed = 1; seed <= 40; seed++) {
    Round r = newRound(seed); r.x = SPOTS[1]; r.hidden = 1;
    uint32_t calm = 0;
    for (int t = 0; t < 120000 && !r.caught; t += STEP_MS) {
      const Phase was = r.phase;
      step(r, 0, false);
      if (was == Phase::Rest && r.phase == Phase::Warn) {
        moves[(int)r.move]++;
        if (r.move == Move::Check) { spots[r.spot]++; picks++; followed += r.spot == 1; }
        CHECK(calm >= 2500 && r.phaseLen >= (r.move == Move::Check ? 3500u : 2000u));
      }
      calm = r.phase == Phase::Rest ? (was == Phase::Rest ? calm + STEP_MS : STEP_MS) : calm;
    }
  }
  for (int m : moves) CHECK(m > 0);
  for (int s : spots) CHECK(s > 0);
  // Rounds end at the first catch, usually that first followed check; uniform picks would still fail this.
  CHECK(followed * 2 > picks);  // checks favor the occupied spot
  CHECK(newRound(9).candy[0] == newRound(9).candy[0] && newRound(0).seed != 0);
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
  Run haunted; haunted.start(); haunted.wait(500); haunted.tap(34, 42); haunted.frame();
  CHECK(haunted.game.round().ghostUsed);
  while (haunted.at("pumpkin_play")) haunted.frame();
  haunted.wait(2200); CHECK(haunted.at("pumpkin_lost"));
  haunted.wait(500); haunted.tap(); CHECK(haunted.at("pumpkin_play") && !haunted.game.round().ghostUsed);
  haunted.wait(500); haunted.tap(34, 42); haunted.frame();
  CHECK(haunted.game.round().ghostUsed);  // a new round has a new ghost
  Run won; won.start();
  for (int i = 0; i < 18000 && won.at("pumpkin_play"); i++) {
    const Choice c = bot(won.game.round());
    won.gravity = grip::gravityFor((int)c.roll, 0, grip::FLAT);
    if (c.action) won.tap(); else won.frame();
  }
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
// The ghost sprite, bobbed down 1 px, stays inside its tap box {22,30,24,24} and the glass.
static void ghostSprite() {
  const int left = 34 - ghost::W / 2;
  for (int y = 0; y < ghost::H; y++) for (int x = 0; x < ghost::W; x++) {
    if (ghost::PX[y * ghost::W + x] == ghost::CLEAR) continue;
    for (int top : {31, 32}) {
      const int px = left + x, py = top + y;
      CHECK(px >= 22 && px < 46 && py >= 30 && py < 54 && gfx::inCircle(px, py));
    }
  }
}
static void crispBlocks() {
  bool crisp = true;
  for (int y = 0; y < 480; y += 3) for (int x = 0; x < 480; x += 3)
    for (int dy = 0; dy < 3; dy++) for (int dx = 0; dx < 3; dx++)
      crisp = crisp && pixels[y * 480 + x] == pixels[(y + dy) * 480 + x + dx];
  CHECK(crisp);
}
// Only the play label sits on row 49. It clears the ghost's tap box {22,30,24,24} while the ghost can be
// tapped (centre x 92) and centres once it's used, off Grandma's glasses.
static void labelRow() {
  gfx565::target(pixels); gfx::textLogEnabled = true;
  for (bool ready : {true, false}) {
    for (int cue = 0; cue < 8; cue++) {
      View v; v.page = Page::Play; v.ghostReady = ready; v.hidden = cue < 4 ? 1 : -1; v.x = SPOTS[1];
      v.rustling = cue == 0 || cue == 1; v.heard = cue == 0 || cue == 2; v.exposure = cue == 3 || cue == 4 ? 0.5f : 0;
      v.distract = cue == 5 ? 100 : 0; v.warning = cue == 6; v.checking = cue == 6 ? 2 : -1; v.doorOpen = cue == 7;
      gfx::textLogReset(); paint::draw(v);
      int found = 0;
      for (int i = 0; i < gfx::textLogCount; i++) {
        const auto b = gfx::textLog[i];
        if (b.y != 49) continue;
        found++;
        CHECK(ready ? b.x >= 46 && 2 * b.x + b.w == 183 : 2 * b.x + b.w == 159);
      }
      CHECK(found == 1);
    }
  }
  gfx::textLogEnabled = false;
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
    v.animationMs = 350;
    gfx::textLogReset(); paint::draw(v); auditText();
  }
  for (int spot = 0; spot < 3; spot++) {  // spot-check bubble, pointing arm and its label
    View v; v.page = Page::Play; v.warning = true; v.checking = spot;
    gfx::textLogReset(); paint::draw(v); auditText(); crispBlocks();
  }
  {
    View v; v.page = Page::Play; v.doorOpen = true; v.phew = true; v.shiver = true; v.hidden = 0;
    v.x = SPOTS[0]; v.sinceCandy = 100; v.got = ALL_CANDY;
    gfx::textLogReset(); paint::draw(v); auditText(); crispBlocks();
    v.page = Page::Won; gfx::textLogReset(); paint::draw(v); auditText(); crispBlocks();
  }
  for (int quip = 0; quip < QUIPS; quip++) {  // every ending line fits the glass
    for (Page page : {Page::Catch, Page::Lost, Page::Won}) {
      View v; v.page = page; v.quip = (uint8_t)quip;
      gfx::textLogReset(); paint::draw(v); auditText(); crispBlocks();
    }
  }
  for (uint32_t distract : {0u, 100u, 700u, 1400u}) {  // the ghost, its crash, rustle and heard
    View v; v.page = Page::Play; v.ghostReady = distract == 0; v.distract = distract; v.animationMs = distract;
    v.searching = distract == 0; v.warning = distract == 100;
    gfx::textLogReset(); paint::draw(v); auditText(); crispBlocks();
    for (int spot = 0; spot < 3; spot++) {
      v.hidden = spot; v.x = SPOTS[spot]; v.rustling = true; v.heard = spot == 1;
      gfx::textLogReset(); paint::draw(v); auditText(); crispBlocks();
    }
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
  controls(); ghostUndo(); openingSweep(); candyAndDoor(); spotCheck(); sweepsAndPhew(); variety(); winnable(); rustle(); ghostTap(); grips(); calibration(); gatesAndHome(); results(); heldAcrossPages(); clocks(); ghostSprite(); renderPages(); labelRow();
  printf("pumpkin: %d checks passed\n", checks);
}
