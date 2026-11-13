#include "game.h"
#include <stdio.h>

namespace pumpkin {
namespace {
constexpr ui::Box HOME = {68, 8, 24, 22}, ACTION = {40, 119, 80, 24}, GHOST = {22, 30, 24, 24};
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
  starting_ = holding_ = ghostPending_ = false;
  refuseMs_ = 0;
  hold_ = ui::Hold{};
  gate_.shown(ms_);
  if (page == Page::Calibrate) steady_ = tilt::Steady{};
}
void Game::start() {
  round_ = newRound(ms_ * 2654435761u + ++plays_);  // a fresh layout and Grandma every round
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
void Game::sign(uint32_t dt, float roll) {
  // Hidden while a finger that touched the sign stays down, wherever it drifts (a 6-year-old presses 1-3 s, double
  // taps when worried, and slides a resting thumb on). One resting since before the round is hidden by gate_.
  // A slide-on arms only where it can hide at once: a finger crossing the sign in the open must not hide later.
  const bool on = in_.hit(ACTION.x, ACTION.y, ACTION.w, ACTION.h) && in_.down, near = cover(round_) >= 0;
  holding_ = in_.down && (holding_ || (on && (near || inside(in_.downX, in_.downY, ACTION))));
  // "No" from the frame a finger reaches the sign where it can't hide and for as long as it stays there or an armed
  // hold lasts, drifted off or not (a 6-year-old who stopped short with his finger down believed he was hiding, kid
  // playtest 4, m1), unless the hold is armed and the pumpkin really rolls into cover within 0.5 s (a slide-on isn't
  // armed in the open, so it always gets the "no"). Lifted, it lingers REFUSE_MS, so a quick tap still shows it.
  const bool no = (on || holding_) && round_.hidden < 0 && !near && !(holding_ && arriving(round_, roll, 500) >= 0);
  refuseMs_ = no ? REFUSE_MS : refuseMs_ > dt ? refuseMs_ - dt : 0;
}
void Game::play(uint32_t dt) {
  stepMs_ += dt;
  const tilt::Vec tilt = tilt::from({in_.gx, in_.gy, in_.gz}, neutral_);
  sign(dt, tilt.x);
  ghostPending_ = ghostPending_ || (!round_.ghostUsed && tapped(in_, GHOST));
  while (stepMs_ >= STEP_MS && !round_.caught && !round_.won) {
    step(round_, tilt.x, holding_, ghostPending_);
    ghostPending_ = false;
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
  v.hidden = round_.hidden; v.nearby = nearby(round_); v.cover = cover(round_); v.coverDir = coverDir(round_);
  v.pageMs = pageMs_; v.animationMs = animationMs_;
  v.beamX = beamX(round_);
  v.searching = searching(round_); v.warning = warning(round_);
  v.ready = ready(); v.starting = starting_;
  v.exposure = (float)round_.exposure / CATCH_MS;
  v.homeHold = hold_.progress();
  for (int i = 0; i < CANDIES; i++) v.candy[i] = round_.candy[i];
  v.got = round_.got; v.sinceCandy = round_.sinceCandy; v.doorOpen = doorOpen(round_);
  v.checking = checking(round_); v.shiver = covered(round_); v.phew = round_.phew > 0;
  v.ghostReady = !round_.ghostUsed; v.refused = refuseMs_ > 0;
  // Play-only cues: a round won mid-crash must not freeze the crash on Won. SHH! waits 300 ms so a brief bump
  // doesn't flash it.
  const bool play = page_ == Page::Play;
  v.distract = play ? round_.distract : 0;
  v.rustling = play && round_.rustle >= 300; v.heard = play && round_.heard >= 0;
  v.startle = play ? round_.startle : 0;
  v.quip = (uint8_t)(round_.seed % QUIPS);
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
  printf("got=%u door=%d phase=%d move=%d checking=%d phew=%d\n", (unsigned)round_.got, doorOpen(round_),
         (int)round_.phase, (int)round_.move, checking(round_), round_.phew > 0);
  printf("ghost=%d distract=%d heard=%d rustle=%d startle=%d refused=%d\n", round_.ghostUsed, round_.distract > 0,
         round_.heard, round_.rustle > 0, round_.startle > 0, refuseMs_ > 0);
  printf("neutralX=%d neutralY=%d neutralZ=%d\n", neutral_.x, neutral_.y, neutral_.z);
  printf("screen=%s\n", screenName());
}
}  // namespace pumpkin
