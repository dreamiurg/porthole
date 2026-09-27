#include "game.h"
#include <math.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "gfx565.h"

namespace marble {
namespace {
// Logical hit circles (the audit sees each as its box): the back knob on the wall at the bottom, and the big button
// on the Calibrate and Done pages.
constexpr int KNOB_HX = 80, KNOB_HY = 145, KNOB_HR = 12;
constexpr int GO_HX = 80, GO_HY = 110, GO_HR = 14;
constexpr int VIAL_X = 240, VIAL_Y = 196, COIN_X = 390, COIN_Y = 90;
constexpr int C = 240;   // the tray's center: physics coordinates are relative to it

// The launcher icon, in the shell's indexed palette (the launcher is the shell's): the tray from above, the goal cut
// into its rim at the top, a red peg and the ball. k outline, w walnut, m maple, f/F felt stripes, n net, g brass
// posts, r/R peg and its shine, c the ball.
constexpr char ICON_ART[] =
  "............nnnnnnnn............" ".........kkknnnnnnnnkkk........." "........kkwwnnnnnnnnwwkk........"
  "......kkwwwwnnnnnnnnwwwwkk......" ".....kkwwwwgnnnnnnnngwwwwkk....." "....kwwwwmgggFFFFFFgggmwwwwk...."
  "...kkwwwmmmgFFFFFFFFgmmmwwwkk..." "...kwwwmmFFFFFFFFFFFFFFmmwwwk..." "..kwwwmmffffffffffffffffmmwwwk.."
  ".kkwwmmffffffffffffffffffmmwwkk." ".kwwwmmffffffffffffrrrfffmmwwwk." ".kwwmmffffffffffffrRRrrfffmmwwk."
  "kkwwmmFFFFFFFFFFFFrRRrrFFFmmwwkk" "kwwwmFFFFFFFFFFFFFrrrrrFFFFmwwwk" "kwwwmFFFFFFFFFFFFFFrrrFFFFFmwwwk"
  "kwwwmFFFFFFFFFFFFFFFFFFFFFFmwwwk" "kwwwmffffffffffffffffffffffmwwwk" "kwwwmffffffffffffffffffffffmwwwk"
  "kwwwmffffffffcccccfffffffffmwwwk" "kkwwmmffffffcccccccfffffffmmwwkk" ".kwwmmFFFFFFccckcccFFFFFFFmmwwk."
  ".kwwwmmFFFFFcckkkccFFFFFFmmwwwk." ".kkwwmmFFFFFccckcccFFFFFFmmwwkk." "..kwwwmmFFFFcccccccFFFFFmmwwwk.."
  "...kwwwmmffffcccccfffffmmwwwk..." "...kkwwwmmmffffffffffmmmwwwkk..." "....kwwwwmmmmffffffmmmmwwwwk...."
  ".....kkwwwwmmmmmmmmmmwwwwkk....." "......kkwwwwwwwwwwwwwwwwkk......" "........kkwwwwwwwwwwwwkk........"
  ".........kkkkwwwwwwkkkk........." "............kkkkkkkk............";
static_assert(sizeof ICON_ART == 32 * 32 + 1, "the launcher slot fits a 32x32 icon");
constexpr char ICON_KEYS[] = "kwmfFngrRc";
constexpr uint8_t ICON_COLS[] = {C_BLACK, C_DKBROWN, C_TAN, C_DKGREEN, C_LEAF, C_SLATE, C_GOLD, C_BROWN, C_PEACH, C_CREAM};
static_assert(sizeof ICON_KEYS - 1 == sizeof ICON_COLS, "one color per key");
constexpr uint8_t iconColor(char c) {
  for (int i = 0; ICON_KEYS[i]; i++) if (ICON_KEYS[i] == c) return ICON_COLS[i];
  return C_T;
}
struct IconPx { uint8_t px[32 * 32]; };
constexpr IconPx makeIcon() {
  IconPx ic{};
  for (int i = 0; i < 32 * 32; i++) ic.px[i] = iconColor(ICON_ART[i]);
  return ic;
}
constexpr IconPx ICON_PX = makeIcon();
const gfx::Sprite ICON = {32, 32, ICON_PX.px};

int px(float v) { return C + (int)lroundf(v); }
}  // namespace

const gfx::Sprite& Game::icon() const { return ICON; }

void Game::enter(const AppEnter& e) {
  now_ = e.nowSec; ms_ = e.ms; in_ = Input{};
  save_ = Save{};
  for (int i = 0; i < e.n; i++)   // only this kid's save; a bad one starts from the first level
    if (e.all[i].id == e.who->id && e.saves[i].len) loadBlob(e.saves[i].data, e.saves[i].len, save_);
  level_ = save_.level < NUM_LEVELS ? save_.level : NUM_LEVELS - 1;   // all done: the last level again
  dirty_ = wantsHome_ = false;
  ball_ = start(LEVELS[level_]);
  go(SC_CALIBRATE);
}

// A new page: every touch waits for the fresh-page pause, and both panel buffers get the whole page.
void Game::go(Screen s) {
  screen_ = s;
  gate_.shown(ms_);
  in_.tap = in_.pressed = in_.longPress = false;
  held_[0].fb = held_[1].fb = nullptr;
}
void Game::play(int level) {
  level_ = level;
  ball_ = start(LEVELS[level]);
  stepMs_ = 0;
  go(SC_PLAY);
}

void Game::update(uint32_t nowSec, uint32_t ms, const Input& in) {
  const uint32_t dt = ms - ms_;
  now_ = nowSec; ms_ = ms; in_ = in;
  gate_.filter(in_, ms_);
  if (in_.tapInCircle(KNOB_HX, KNOB_HY, KNOB_HR)) { wantsHome_ = true; return; }   // Back, on every page
  switch (screen_) {
    case SC_CALIBRATE: updateCalibrate(); break;
    case SC_PLAY: updatePlay(dt); break;
    case SC_GOAL: updateGoal(); break;
    default: updateDone(); break;
  }
}
// The kid holds the device the way they like and taps: that is level from now on.
void Game::updateCalibrate() {
  if (!in_.tapInCircle(GO_HX, GO_HY, GO_HR)) return;
  neutral_ = {in_.gx, in_.gy, in_.gz};
  play(level_);
}
// Fixed substeps (tune.h), the tilt as it is this frame. Taps do nothing here: a hand on the case may brush the glass.
void Game::updatePlay(uint32_t dt) {
  stepMs_ += dt > MAX_FRAME_MS ? MAX_FRAME_MS : dt;
  const Vec a = tiltAccel({in_.gx, in_.gy, in_.gz}, neutral_);
  for (; stepMs_ >= STEP_MS && !ball_.goal; stepMs_ -= STEP_MS) step(ball_, a);
  if (!ball_.goal) return;
  if (save_.level < level_ + 1) { save_.level = (uint8_t)(level_ + 1); dirty_ = true; }
  goalMs_ = ms_;
  go(SC_GOAL);
}
void Game::updateGoal() {
  if (!in_.tap && ms_ - goalMs_ < GOAL_MS) return;
  if (level_ + 1 < NUM_LEVELS) play(level_ + 1);
  else go(SC_DONE);
}
void Game::updateDone() {
  if (in_.tapInCircle(GO_HX, GO_HY, GO_HR)) play(NUM_LEVELS - 1);
}

int Game::bubble(int axis) const {   // a bubble floats to the high side: away from gravity, as far as the vial allows
  const float gx = (float)-in_.gx, gy = (float)-in_.gy, m = sqrtf(gx * gx + gy * gy);
  return (int)lroundf((axis ? gy : gx) * paint::BUBBLE_TRAVEL / (m > 1000 ? m : 1000));
}
paint::Box Game::moving() const {
  if (screen_ == SC_PLAY) return paint::ballBox(px(ball_.p.x), px(ball_.p.y));
  if (screen_ != SC_CALIBRATE) return paint::NONE;
  const int x = VIAL_X + bubble(0), y = VIAL_Y + bubble(1), r = paint::BUBBLE_R + 3;
  return {x - r, y - r, x + r + 1, y + r + 1};
}

using DrawFn = void (Game::*)();
void Game::render() {
  static const DrawFn DRAW[] = {&Game::drawCalibrate, &Game::drawPlay, &Game::drawGoal, &Game::drawDone};
  static_assert(sizeof DRAW / sizeof DRAW[0] == SC_COUNT, "one draw per page, in Screen order");
  const paint::Box now = moving();
  Held* h = held_[0].fb == gfx565::fb ? &held_[0] : held_[1].fb == gfx565::fb ? &held_[1] : nullptr;
  if (!h) { h = &held_[held_[0].fb ? 1 : 0]; h->fb = gfx565::fb; paint::clip(paint::FULL); }
  else if (gfx::textLogEnabled) paint::clip(paint::FULL);   // the UI audit's frame: every glyph drawn, so every one logs
  else if (paint::same(now, h->moving)) return;             // this buffer already shows this frame
  else paint::clip(paint::unite(now, h->moving));
  const DrawFn fn = DRAW[screen_];   // never (this->*TABLE[i])(): see Biscuit's game.cpp
  (this->*fn)();
  h->moving = now;
}

// Every shadow first, then what casts them, so a shadow never lies on top of a peg.
void Game::drawTray(bool pegs) {
  const Level& l = LEVELS[level_];
  const Vec left = postAt(l, -1), right = postAt(l, 1);
  const bool ball = screen_ == SC_PLAY || screen_ == SC_GOAL;
  paint::tray(l.goalHalf);
  for (int i = 0; pegs && i < l.pegCount; i++) paint::shadow(C + l.pegs[i].x, C + l.pegs[i].y, l.pegs[i].r, l.goalHalf);
  paint::shadow(px(left.x), px(left.y), POST_R, l.goalHalf);
  paint::shadow(px(right.x), px(right.y), POST_R, l.goalHalf);
  paint::shadow(C, C + KNOB_Y, KNOB_R, l.goalHalf);
  if (ball) paint::shadow(px(ball_.p.x), px(ball_.p.y), BALL_R, l.goalHalf);
  for (int i = 0; pegs && i < l.pegCount; i++) paint::peg(C + l.pegs[i].x, C + l.pegs[i].y, l.pegs[i].r);
  paint::post(px(left.x), px(left.y));
  paint::post(px(right.x), px(right.y));
  if (ball) paint::ball(px(ball_.p.x), px(ball_.p.y));
  paint::knob(C, C + KNOB_Y);
}
void Game::drawCalibrate() {
  drawTray(false);
  paint::coin(COIN_X, COIN_Y, level_ + 1);
  paint::vial(VIAL_X, VIAL_Y, bubble(0), bubble(1));
  paint::playButton(GO_HX * 3, GO_HY * 3);
}
void Game::drawPlay() {
  drawTray(true);
  paint::coin(COIN_X, COIN_Y, level_ + 1);
}
void Game::drawGoal() {
  drawPlay();
  const Level& l = LEVELS[level_];
  paint::flag(px(postAt(l, -1).x), px(postAt(l, -1).y), -1);
  paint::flag(px(postAt(l, 1).x), px(postAt(l, 1).y), 1);
  paint::blocks("GOAL!", C, 206, 7);
}
// All done: every level's coin in an arc, the ball, and the button that plays the last level again.
void Game::drawDone() {
  drawTray(false);
  for (int i = 0; i < NUM_LEVELS; i++) {
    const float a = (200 + i * 140.0f / (NUM_LEVELS - 1)) * 3.14159f / 180;
    paint::coin(C + (int)lroundf(cosf(a) * 128), C + (int)lroundf(sinf(a) * 128), i + 1);
  }
  paint::ball(C, 232);
  paint::playButton(GO_HX * 3, GO_HY * 3);
}

bool Game::takeSave(const void** data, size_t* len, bool allowed) {
  if (!allowed || !dirty_) return false;
  seal(save_); out_ = save_;
  *data = &out_; *len = sizeof out_;
  dirty_ = false;
  return true;
}

const char* Game::screenName() const {
  static const char* N[] = {"mk_calibrate", "mk_play", "mk_goal", "mk_done"};
  static_assert(sizeof N / sizeof N[0] == SC_COUNT, "one name per page");
  return N[screen_];
}
void Game::debugPrint() {
  printf("[marblekick level=%d reached=%u x=%d y=%d speed=%d goal=%d neutralX=%d neutralY=%d neutralZ=%d]\n", level_ + 1,
         (unsigned)save_.level, (int)lroundf(ball_.p.x), (int)lroundf(ball_.p.y),
         (int)lroundf(sqrtf(ball_.v.x * ball_.v.x + ball_.v.y * ball_.v.y)), ball_.goal, neutral_.x, neutral_.y, neutral_.z);
  printf("screen=%s\n", screenName());
}
void Game::debugCmd(const char* cmd) {
  if (strncmp(cmd, "level", 5)) return;
  const int n = atoi(cmd + 5);
  if (n < 1 || n > NUM_LEVELS) return;
  neutral_ = {in_.gx, in_.gy, in_.gz};
  play(n - 1);
}
}  // namespace marble
