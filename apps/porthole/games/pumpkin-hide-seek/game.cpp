#include "game.h"
#include <stdio.h>

namespace pumpkin {
namespace {
constexpr ui::Box HOME = {68, 8, 24, 22}, ACTION = {40, 119, 80, 24};
bool inside(int x, int y, ui::Box b) {
  return x >= b.x && y >= b.y && x < b.x + b.w && y < b.y + b.h;
}
bool tapped(const Input& in, ui::Box b) {
  UiAudit::add(b.x, b.y, b.w, b.h);
  return in.tap && inside(in.downX, in.downY, b) && inside(in.x, in.y, b);
}
}  // namespace
void Game::enter(const AppEnter& e) {
  ms_ = e.ms;
  animationMs_ = 0;
  in_ = Input{};
  round_ = Round{};
  steady_ = tilt::Steady{};
  neutral_ = {0, 0, -1000};
  home_ = false;
  go(Page::Intro);
}
void Game::go(Page page) {
  page_ = page;
  pageMs_ = stepMs_ = 0;
  starting_ = actionPending_ = false;
  hold_ = ui::Hold{};
  gate_.shown(ms_);
  if (page == Page::Calibrate) steady_ = tilt::Steady{};
}
void Game::start() {
  round_ = Round{};
  go(Page::Play);
}
bool Game::action() const { return tapped(in_, ACTION); }
bool Game::ready() const {
  tilt::Grav neutral;
  return steady_.get(ms_, &neutral);
}
bool Game::leaving() {
  UiAudit::add(HOME.x, HOME.y, HOME.w, HOME.h);
  const bool pressed = in_.down && inside(in_.downX, in_.downY, HOME) && inside(in_.x, in_.y, HOME);
  hold_.step(page_ == Page::Play && pressed, ms_);
  return page_ == Page::Play ? hold_.progress() >= 1 : tapped(in_, HOME);
}
void Game::calibrate() {
  if (action()) starting_ = true;
  tilt::Grav neutral;
  if (starting_ && steady_.get(ms_, &neutral)) { neutral_ = neutral; start(); }
}
void Game::play(uint32_t dt) {
  stepMs_ += dt;
  actionPending_ = actionPending_ || action();
  const tilt::Vec tilt = tilt::from({in_.gx, in_.gy, in_.gz}, neutral_);
  while (stepMs_ >= STEP_MS && !round_.caught && !round_.won) {
    step(round_, tilt.x, actionPending_);
    actionPending_ = false;
    stepMs_ -= STEP_MS;
  }
  if (round_.caught) go(Page::Catch);
  else if (round_.won) go(Page::Won);
}
void Game::update(uint32_t, uint32_t ms, const Input& in) {
  const uint32_t gap = ms - ms_;
  const uint32_t dt = gap > MAX_FRAME_MS ? MAX_FRAME_MS : gap;
  ms_ = ms;
  pageMs_ += dt;
  animationMs_ += dt;
  in_ = in;
  gate_.filter(in_, ms_);
  steady_.add(in_.gx, in_.gy, in_.gz, ms_);
  if (leaving()) { home_ = true; return; }
  switch (page_) {
    case Page::Intro: if (action()) go(Page::Calibrate); break;
    case Page::Calibrate: calibrate(); break;
    case Page::Play: play(dt); break;
    case Page::Catch: if (pageMs_ >= 600) go(Page::Icing); break;
    case Page::Icing: if (pageMs_ >= 1600) go(Page::Lost); break;
    case Page::Lost: case Page::Won: if (action()) start(); break;
  }
}
void Game::render() {
  View v;
  v.page = page_;
  v.x = round_.x; v.velocity = round_.velocity;
  v.hidden = round_.hidden; v.nearby = nearby(round_);
  v.elapsed = round_.elapsed; v.pageMs = pageMs_; v.animationMs = animationMs_;
  v.beamX = beamX(round_);
  v.searching = searching(round_); v.warning = warning(round_);
  v.ready = ready(); v.starting = starting_;
  v.exposure = (float)round_.exposure / CATCH_MS;
  v.homeHold = hold_.progress();
  paint::draw(v);
}
const char* Game::screenName() const {
  static const char* N[] = {"pumpkin_intro", "pumpkin_calibrate", "pumpkin_play", "pumpkin_catch",
                                "pumpkin_icing", "pumpkin_lost", "pumpkin_won"};
  return N[(unsigned)page_];
}
void Game::debugPrint() {
  printf("[pumpkin x=%d hidden=%d elapsed=%u exposure=%u ready=%d starting=%d]\n", (int)round_.x,
         round_.hidden, (unsigned)round_.elapsed, (unsigned)round_.exposure, ready(), starting_);
  printf("neutralX=%d neutralY=%d neutralZ=%d\n", neutral_.x, neutral_.y, neutral_.z);
  printf("screen=%s\n", screenName());
}
}  // namespace pumpkin
