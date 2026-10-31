#include "game.h"
#include <math.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

namespace racing {
namespace {
using paint::Leave;
constexpr uint32_t GRIP_LOST_MS = 300, RESUME_MS = 1000;
bool inside(const ui::Box& b, int x, int y) { return x >= b.x && y >= b.y && x < b.x + b.w && y < b.y + b.h; }
bool click(const Input& in, const ui::Box& b) { return ui::clicked(in, b); }
// A fresh press that began on the button: holding, sliding in from elsewhere or a carried touch never fires.
bool press(const Input& in, const ui::Box& b) { return in.hit(b.x, b.y, b.w, b.h) && in.pressed && inside(b, in.downX, in.downY); }
bool holding(const Input& in, const ui::Box& b) {
  return in.hit(b.x, b.y, b.w, b.h) && in.down && inside(b, in.downX, in.downY);
}
}  // namespace

void Game::enter(const AppEnter& e) {
  now_ = e.nowSec; rawMs_ = e.ms; seed_ = e.ms ? e.ms : 1; ms_ = runs_ = 0;
  daylight_ = e.daylight; home_ = seenHowTo_ = recenter_ = gripLost_ = false; in_ = {};
  neutral_ = {0, 1000, 0};
  track_ = 0;
  start(race_, TRACK[track_], seed_);
  go(Screen::Tracks);
}

void Game::go(Screen s) {
  screen_ = s;
  if (s == Screen::Calibrate) calibration_ = {};
  gate_.shown(ms_); hold_ = {};
  boost_ = rocket_ = false;
  stepMs_ = badGripMs_ = 0;
}

void Game::startRace() {
  start(race_, TRACK[track_], seed_ + ++runs_);   // every run is fresh: inventory, timers and rivals reset
  steer_ = 0; resumeMs_ = 0; gripLost_ = recenter_ = false;
  go(Screen::Race);
}

void Game::update(uint32_t nowSec, uint32_t ms, const Input& in) {
  const int32_t elapsed = (int32_t)(ms - rawMs_);
  const uint32_t dt = elapsed <= 0 ? 0 : (uint32_t)elapsed > MAX_FRAME_MS ? MAX_FRAME_MS : (uint32_t)elapsed;
  rawMs_ = ms; now_ = nowSec; ms_ += dt; in_ = in;
  gate_.filter(in_, ms_);
  switch (screen_) {
    case Screen::Tracks: updateTracks(); break;
    case Screen::HowTo: updateHowTo(); break;
    case Screen::Calibrate: updateCalibrate(); break;
    case Screen::Race: updateRace(dt); break;
    case Screen::Pause: updatePause(); break;
    case Screen::Confirm: updateConfirm(); break;
    case Screen::Results: updateResults(); break;
  }
}

void Game::updateTracks() {
  for (int i = 0; i < TRACKS; i++)
    if (click(in_, paint::CARD[i])) { track_ = i; go(seenHowTo_ ? Screen::Calibrate : Screen::HowTo); return; }
  if (click(in_, paint::HOME)) home_ = true;
}

void Game::updateHowTo() {
  if (click(in_, paint::BACK)) go(Screen::Tracks);
  else if (click(in_, paint::NEXT)) { seenHowTo_ = true; go(Screen::Calibrate); }
}

void Game::updateCalibrate() {
  calibration_.add(gravity(), rawMs_);
  steer_ = controls::steer(gravity(), neutral_);   // the preview car follows the wheel as it is set now
  if (click(in_, paint::BACK)) { go(recenter_ ? Screen::Pause : Screen::Tracks); return; }
  tilt::Grav n{};
  if (!click(in_, paint::NEXT) || !calibration_.get(&n)) return;
  neutral_ = n;
  if (!recenter_) { startRace(); return; }
  recenter_ = false; gripLost_ = false; resumeMs_ = RESUME_MS; steer_ = 0;   // the race waits a beat, then goes on
  go(Screen::Race);
}

// A flat or implausible pose for GRIP_LOST_MS stops the race: no unstable steering, no driving blind.
bool Game::gripOk(uint32_t dt) {
  if (controls::upright(gravity())) { badGripMs_ = 0; return true; }
  badGripMs_ += dt;
  if (badGripMs_ < GRIP_LOST_MS) return true;
  gripLost_ = true;
  go(Screen::Pause);
  return false;
}

void Game::updateRace(uint32_t dt) {
  if (race_.phase == Phase::Finished) { go(Screen::Results); return; }
  if (!gripOk(dt)) return;
  hold_.step(holding(in_, paint::LAP), ms_);
  if (hold_.progress() >= 1) { gripLost_ = false; go(Screen::Pause); return; }
  boost_ = boost_ || press(in_, paint::BOOST_BTN);
  rocket_ = rocket_ || press(in_, paint::ROCKET_BTN);
  if (resumeMs_) { resumeMs_ = resumeMs_ > dt ? resumeMs_ - dt : 0; boost_ = rocket_ = false; return; }
  simulate(dt);
}

void Game::simulate(uint32_t dt) {
  const float target = controls::steer(gravity(), neutral_);
  stepMs_ += dt;
  while (stepMs_ >= STEP_MS && race_.phase != Phase::Finished) {
    stepMs_ -= STEP_MS;
    steer_ += (target - steer_) * (STEP_MS / (100.0f + STEP_MS));   // ~100 ms smoothing
    const bool racing = race_.phase == Phase::Racing;
    Controls c;
    c.steer = steer_;
    c.boost = racing && boost_;
    c.rocket = racing && rocket_;
    boost_ = rocket_ = false;   // spent or not, a press is used once; one before GO is simply dropped
    step(race_, c);
  }
}

void Game::updatePause() {
  if (click(in_, paint::RESUME) && controls::upright(gravity())) {
    gripLost_ = false; resumeMs_ = RESUME_MS; badGripMs_ = 0;
    const uint32_t keep = stepMs_;
    go(Screen::Race);
    stepMs_ = keep;   // exact simulation state, down to the partial step
    return;
  }
  if (click(in_, paint::RECENTER)) { recenter_ = true; go(Screen::Calibrate); return; }
  const ui::Box* boxes[] = {&paint::RESTART, &paint::TRACKS_BTN, &paint::HOME_BTN};
  for (int i = 0; i < 3; i++)
    if (click(in_, *boxes[i])) { leave_ = (Leave)i; go(Screen::Confirm); return; }
}

void Game::updateConfirm() {
  if (click(in_, paint::NO)) { go(Screen::Pause); return; }
  if (!click(in_, paint::YES)) return;
  if (leave_ == Leave::Restart) startRace();
  else if (leave_ == Leave::Tracks) go(Screen::Tracks);
  else home_ = true;
}

void Game::updateResults() {
  if (click(in_, paint::REPLAY)) startRace();
  else if (click(in_, paint::RESULT_TRACKS)) go(Screen::Tracks);
  else if (click(in_, paint::RESULT_HOME)) home_ = true;
}

void Game::render() {
  tilt::Grav n{};
  static const paint::Page PAGES[] = {paint::Page::Tracks, paint::Page::HowTo, paint::Page::Calibrate, paint::Page::Race,
                                      paint::Page::Pause, paint::Page::Confirm, paint::Page::Results};
  paint::View v;
  v.page = PAGES[(int)screen_];
  v.race = &race_; v.track = track_; v.ms = ms_; v.steer = steer_;
  v.calibration = calibration_.progress(); v.hold = (uint16_t)(hold_.progress() * 1000); v.resumeMs = resumeMs_;
  v.gripReady = calibration_.get(&n); v.gripUpright = controls::upright(gravity());
  v.gripLost = gripLost_; v.recenter = recenter_; v.leave = leave_;
  paint::draw(v);
}

const char* Game::screenName() const {
  static const char* N[] = {"race_tracks", "race_howto", "race_calibrate", "race_play", "race_pause", "race_confirm", "race_results"};
  if (screen_ == Screen::Race && race_.phase == Phase::Countdown) return "race_countdown";
  return N[(int)screen_];
}

void Game::debugPrint() {
  const Car& p = race_.car[PLAYER];
  printf("[racing screen=%s track=%d phase=%d lap=%d place=%u boost=%u rocket=%u speed=%d x=%d dist=%d]\n", screenName(),
         track_, (int)race_.phase, lapOf(race_, PLAYER), race_.place, race_.inventory[BOOST], race_.inventory[ROCKET],
         (int)lroundf(p.speed * 1000), (int)lroundf(p.x * 1000), (int)lroundf(p.dist));
  printf("race_ms=%u finished=%d finish_ms=%u cars_done=%u jumps=%u pickups=%u fired=%u hits=%u impacts=%u\n",
         (unsigned)race_.raceMs, p.finished, (unsigned)p.finishMs, race_.finishedCount, race_.jumps, race_.pickups,
         race_.rocketsFired, race_.rocketHits, race_.impacts);
  tilt::Grav n{};
  printf("steer=%d grip_lost=%d resume=%u ready=%d calibration=%u neutral=%d,%d,%d\n", (int)lroundf(steer_ * 1000),
         gripLost_, (unsigned)resumeMs_, calibration_.get(&n), calibration_.progress(), neutral_.x, neutral_.y, neutral_.z);
}

// Test fixtures only (reachability, not natural play): they start from the current upright grip.
void Game::debugFixture(const char* cmd) {
  if (controls::upright(gravity())) neutral_ = gravity();
  startRace();
  race_.phase = Phase::Racing;
  race_.ms = COUNTDOWN_MS;
  const float line = LAPS * race_.track->length;
  if (!strcmp(cmd, "lastlap")) race_.car[PLAYER].dist = line - 60;
  if (!strcmp(cmd, "lastplace")) {   // every rival already home: the player still finishes, fourth
    for (int i = 1; i < CARS; i++) { race_.car[i].dist = line + i; race_.car[i].finished = true; race_.car[i].finishMs = 1000u * (unsigned)i; }
    race_.finishedCount = CARS - 1;
    race_.car[PLAYER].dist = line - 60;
  }
}

void Game::debugCmd(const char* cmd) {
  if (!strncmp(cmd, "track ", 6)) { track_ = abs(atoi(cmd + 6)) % TRACKS; return; }
  if (!strcmp(cmd, "tracks")) { go(Screen::Tracks); return; }
  if (!strcmp(cmd, "calibrate")) { go(Screen::Calibrate); return; }
  if (!strcmp(cmd, "go") || !strcmp(cmd, "lastlap") || !strcmp(cmd, "lastplace")) debugFixture(cmd);
}
}  // namespace racing
