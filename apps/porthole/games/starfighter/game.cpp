#include "game.h"
#include <stdio.h>
#include <string.h>
#include "render.h"

namespace starfighter {
namespace {
bool hit(const Input& in, const paint::Box& b) { return in.hit(b.x, b.y, b.w, b.h); }
bool tapped(const Input& in, const paint::Box& b) { return hit(in, b) && in.tap; }
bool pressing(const Input& in, const paint::Box& b) {
  return hit(in, b) && in.down && in.downX >= b.x && in.downX < b.x + b.w && in.downY >= b.y && in.downY < b.y + b.h;
}
}  // namespace
const gfx::Sprite& Game::icon() const { return paint::icon(); }

void Game::enter(const AppEnter& e) {
  now_ = e.nowSec; rawMs_ = e.ms; seed_ = e.ms; ms_ = runNo_ = 0;
  daylight_ = e.daylight; home_ = false; in_ = {}; neutral_ = {0, 1000, 0};
  restart(run_, seed_);
  go(Screen::Title);
}
void Game::go(Screen screen) {
  screen_ = screen;
  if (screen == Screen::Calibrate) calibration_ = {};
  transition();
}
void Game::transition() {
  gate_.shown(ms_); hold_ = {}; taps_ = stepMs_ = 0;
}
void Game::startRun() {
  restart(run_, seed_ + ++runNo_);
  bank_ = 0;
  go(Screen::Flight);
}
bool Game::terminal() const { return screen_ == Screen::Flight && !playing(run_.stage); }
bool Game::leaving() {
  if (screen_ != Screen::Flight || terminal()) return tapped(in_, paint::EXIT);
  hold_.step(pressing(in_, paint::HOLD), ms_);
  return hold_.progress() >= 1;
}
void Game::update(uint32_t nowSec, uint32_t ms, const Input& in) {
  const int32_t elapsed = (int32_t)(ms - rawMs_);
  const uint32_t dt = elapsed <= 0 ? 0 : (uint32_t)elapsed > MAX_FRAME_MS ? MAX_FRAME_MS : (uint32_t)elapsed;
  rawMs_ = ms; now_ = nowSec; ms_ += dt; in_ = in;
  gate_.filter(in_, ms_);
  if (leaving()) { home_ = true; return; }
  switch (screen_) {
    case Screen::Title: if (tapped(in_, paint::MAIN)) go(Screen::Calibrate); break;
    case Screen::Calibrate: updateCalibrate(); break;
    case Screen::Brief: if (tapped(in_, paint::MAIN)) startRun(); break;
    case Screen::Flight: updateFlight(dt); break;
  }
}
void Game::updateCalibrate() {
  const tilt::Grav gravity = {in_.gx, in_.gy, in_.gz};
  calibration_.add(gravity, rawMs_);
  bank_ = controls::bank(gravity, neutral_);
  if (tapped(in_, paint::MAIN) && calibration_.get(&neutral_)) go(Screen::Brief);
}
void Game::updateFlight(uint32_t dt) {
  if (terminal()) {
    if (tapped(in_, paint::MAIN)) go(Screen::Calibrate);
    return;
  }
  const float target = controls::bank({in_.gx, in_.gy, in_.gz}, neutral_);
  taps_ += in_.tap ? 1 : 0;
  stepMs_ += dt;
  const Stage before = run_.stage;
  while (stepMs_ >= STEP_MS) {
    stepMs_ -= STEP_MS;
    bank_ += (target - bank_) * (STEP_MS / (100.0f + STEP_MS));
    step(run_, bank_, taps_ > 0);
    if (taps_) --taps_;
    if (run_.stage != before) { transition(); break; }
  }
}
void Game::render() {
  tilt::Grav neutral{};
  const paint::Page pages[] = {paint::Page::Title, paint::Page::Calibrate, paint::Page::Brief, paint::Page::Flight};
  const paint::View view = {pages[(int)screen_], &run_, ms_, bank_, calibration_.progress(),
                            (uint16_t)(hold_.progress() * 1000), calibration_.get(&neutral)};
  paint::draw(view);
}
const char* Game::screenName() const {
  static const char* N[] = {"sf_title", "sf_calibrate", "sf_brief", "sf_squadron", "sf_waveclear", "sf_entry",
                            "sf_trench", "sf_target", "sf_torpedo", "sf_explosion", "sf_victory", "sf_defeat"};
  return N[screen_ == Screen::Flight ? 3 + (int)run_.stage : (int)screen_];
}
void Game::debugPrint() {
  printf("[starfighter screen=%s stage=%s lives=%u score=%u wave=%u volleys=%u x=%d bank=%d invulnerable=%u targetPasses=%u clock=%u]\n",
         screenName(), stageName(run_.stage), run_.lives, (unsigned)run_.score, run_.wave + 1, (unsigned)run_.volleys,
         (int)lroundf(run_.player.x), (int)lroundf(bank_ * 1000), (unsigned)run_.player.invulnerableMs,
         (unsigned)run_.targetPasses, (unsigned)run_.ms);
  tilt::Grav n{};
  printf("neutralX=%d neutralY=%d neutralZ=%d ready=%d calibration=%u\n",
         neutral_.x, neutral_.y, neutral_.z, calibration_.get(&n), calibration_.progress());
  printf("targetX=%d aligned=%d\n", (int)lroundf(run_.targetX), aligned(run_));
}
void Game::debugStage(Stage stage) {
  if (controls::upright({in_.gx, in_.gy, in_.gz})) neutral_ = {in_.gx, in_.gy, in_.gz};
  startRun();
  restart(run_, 1);
  run_.stage = stage;
  if (stage == Stage::Defeat) run_.lives = 0;
}
void Game::debugCmd(const char* cmd) {
  if (!strcmp(cmd, "title")) { go(Screen::Title); return; }
  if (!strcmp(cmd, "calibrate")) { go(Screen::Calibrate); return; }
  if (!strcmp(cmd, "brief")) { go(Screen::Brief); return; }
  if (!strcmp(cmd, "start")) { debugStage(Stage::Squadron); return; }
  if (!strcmp(cmd, "lastwave")) { debugStage(Stage::WaveClear); run_.wave = WAVES - 1; return; }
  if (!strcmp(cmd, "trench-end")) { debugStage(Stage::Trench); run_.stageMs = TRENCH_MS - 100; return; }
  if (!strcmp(cmd, "hit")) {
    if (run_.lives > 0 && !run_.player.invulnerableMs) {
      --run_.lives; run_.player.invulnerableMs = INVULNERABLE_MS;
      if (!run_.lives) { run_.stage = Stage::Defeat; transition(); }
    }
    return;
  }
  const char* commands[] = {"squadron", "waveclear", "entry", "trench", "target", "torpedo", "explosion", "victory", "defeat"};
  for (unsigned i = 0; i < sizeof commands / sizeof commands[0]; ++i)
    if (!strcmp(cmd, commands[i])) { debugStage((Stage)i); return; }
}
}  // namespace starfighter
