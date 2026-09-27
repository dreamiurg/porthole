#include "game.h"
#include <math.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "gfx565.h"

namespace marble {
namespace {
// Logical hit areas (the audit sees each as its box): the home knob on the wall at the bottom, the big button on the
// Calibrate and Done pages, and the Done page's coins, two rows of three.
constexpr int KNOB_HX = 80, KNOB_HY = 145, KNOB_HR = 12;
constexpr int GO_HX = 80, GO_HY = 110, GO_HR = 14;
constexpr int COIN_HX[3] = {50, 80, 110}, COIN_HY[2] = {50, 78}, COIN_HALF = 12;
constexpr int CX = gfx565::CX, CY = gfx565::CY;   // the tray's center: physics coordinates are relative to it
constexpr int DISH_X = CX, DISH_Y = 196, COIN_X = 390, COIN_Y = 90;
constexpr uint32_t HOLD_MS = 600;       // Play: holding the knob this long leaves (InputTracker's long press)
constexpr uint32_t STEADY_MS = 300;     // the neutral is the average gravity over this long
constexpr int STEADY_MIN = 800, STEADY_MAX = 1200;   // milli-g: a reading outside is a jolt, not a way of holding it
constexpr float DISH_GAIN = 1.2f, DISH_SPRING = 9, DISH_DAMP = 3.5f;   // the dish: 1/s^2, 1/s

// The launcher icon, in the shell's indexed palette (the launcher is the shell's): the tray from above, the goal's net
// cut into the wood at the top between brass posts, a red peg and the white ball with its dark pentagon. k outline,
// w walnut, m maple, f/F felt stripes, n net, g brass, r/R peg and its shine, W the ball.
constexpr char ICON_ART[] =
  "............kkkkkkkk............" ".........kkkknnnnnnkkkk........." "........kkwwnnnnnnnnwwkk........"
  "......kkwwwgnnnnnnnngwwwkk......" ".....kkwwwwgnnnnnnnngwwwwkk....." "....kwwwwmmgnFFFFFFngmmwwwwk...."
  "...kkwwwmmffffffffffffmmwwwkk..." "...kwwwmmffffffffffffffmmwwwk..." "..kwwwmmfffffffffffkrrkfmmwwwk.."
  ".kkwwmmFFFFFFFFFFFkrRrrkFmmwwkk." ".kwwwmFFFFFFFFFFFFrRRrrrFFmwwwk." ".kwwmmFFFFFFFFFFFFrrrrrrFFmmwwk."
  "kkwwmmffffffffffffkrrrrkffmmwwkk" "kwwwmffffffffffffffkrrkffffmwwwk" "kwwmmffffffffffffffffffffffmmwwk"
  "kwwmmFFFFFFFFFFFFFFFFFFFFFFmmwwk" "kwwmmFFFFFFFFFFFFFFFFFFFFFFmmwwk" "kwwmmFFFFFFFkkWkkFFFFFFFFFFmmwwk"
  "kwwwmffffffkWWWWWkfffffffffmwwwk" "kkwwmmffffkWWWWWWWkfffffffmmwwkk" ".kwwmmffffkWWWkWWWkfffffffmmwwk."
  ".kwwwmFFFFWWWkkkWWWFFFFFFFmwwwk." ".kkwwmmFFFkWWkkkWWkFFFFFFmmwwkk." "..kwwwmmFFkWWWWWWWkFFFFFmmwwwk.."
  "...kwwwmmffkWWWWWkfffffmmwwwk..." "...kkwwwmmffkkWkkfffffmmwwwkk..." "....kwwwwmmmmffffffmmmmwwwwk...."
  ".....kkwwwwmmmmmmmmmmwwwwkk....." "......kkwwwwwwmmmmwwwwwwkk......" "........kkwwwwwwwwwwwwkk........"
  ".........kkkkwwwwwwkkkk........." "............kkkkkkkk............";
static_assert(sizeof ICON_ART == 32 * 32 + 1, "the launcher slot fits a 32x32 icon");
constexpr char ICON_KEYS[] = "kwmfFngrRW";
constexpr uint8_t ICON_COLS[] = {C_BLACK, C_DKBROWN, C_TAN, C_DKGREEN, C_LEAF, C_SLATE, C_GOLD, C_BROWN, C_PEACH, C_WHITE};
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

int px(float v) { return CX + (int)lroundf(v); }
bool within(int x, int y, int cx, int cy, int r) { return (x - cx) * (x - cx) + (y - cy) * (y - cy) <= r * r; }
int coinX(int i) { return COIN_HX[i % 3]; }
int coinY(int i) { return COIN_HY[i / 3]; }
}  // namespace

const gfx::Sprite& Game::icon() const { return ICON; }

// Every visit: the kid's own level, and the Calibrate page. After all of them, level 1 again.
void Game::enter(const AppEnter& e) {
  now_ = e.nowSec; ms_ = e.ms; in_ = Input{};
  save_ = Save{};
  for (int i = 0; i < e.n; i++)   // only this kid's save; a bad one starts from the first level
    if (e.all[i].id == e.who->id && e.saves[i].len) loadBlob(e.saves[i].data, e.saves[i].len, save_);
  level_ = save_.level < NUM_LEVELS ? save_.level : 0;
  dirty_ = wantsHome_ = false;
  ball_ = start(LEVELS[level_]);
  sampleCount_ = 0; refSet_ = false; dish_ = dishV_ = {0, 0};
  go(SC_CALIBRATE);
}

// A new page: every touch waits for the fresh-page pause, and both panel buffers get the whole page.
void Game::go(Screen s) {
  screen_ = s;
  gate_.shown(ms_);
  pageMs_ = ms_;
  starting_ = false;
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
  remember();
  if (leaving()) { wantsHome_ = true; return; }
  switch (screen_) {
    case SC_CALIBRATE: updateCalibrate(dt); break;
    case SC_PLAY: updatePlay(dt); break;
    case SC_GOAL: updateGoal(); break;
    default: updateDone(); break;
  }
}
bool Game::pressing(int cx, int cy, int r) const {
  return in_.down && within(in_.downX, in_.downY, cx, cy, r) && within(in_.x, in_.y, cx, cy, r);
}
// Home: a tap on the knob, except during play, where a hand on the case may brush it: there it takes a hold.
bool Game::leaving() {
  if (screen_ != SC_PLAY) return in_.tapInCircle(KNOB_HX, KNOB_HY, KNOB_HR);
  in_.hit(KNOB_HX - KNOB_HR, KNOB_HY - KNOB_HR, 2 * KNOB_HR, 2 * KNOB_HR);   // for the UI audit
  return in_.longPress && pressing(KNOB_HX, KNOB_HY, KNOB_HR);
}
float Game::hold() const {
  if (screen_ != SC_PLAY || !pressing(KNOB_HX, KNOB_HY, KNOB_HR)) return 0;
  return in_.heldMs >= HOLD_MS ? 1 : (float)in_.heldMs / HOLD_MS;
}

void Game::remember() {
  samples_[sampleCount_ % 24] = {in_.gx, in_.gy, in_.gz, ms_};
  sampleCount_++;
}
// The neutral: the average gravity of the last STEADY_MS, as long as every reading in it is a plausible 1 g (a
// jolt or a shake is not a way of holding it; the kid's press then waits until it has passed).
bool Game::steady(Grav* neutral) const {
  long sx = 0, sy = 0, sz = 0;
  int n = 0;
  for (int i = 0; i < 24 && i < sampleCount_; i++) {
    const Sample& s = samples_[i];
    if (ms_ - s.ms > STEADY_MS) continue;
    const long m2 = (long)s.x * s.x + (long)s.y * s.y + (long)s.z * s.z;
    if (m2 < (long)STEADY_MIN * STEADY_MIN || m2 > (long)STEADY_MAX * STEADY_MAX) return false;
    sx += s.x; sy += s.y; sz += s.z; n++;
  }
  if (!n) return false;
  *neutral = {(int)lroundf((float)sx / n), (int)lroundf((float)sy / n), (int)lroundf((float)sz / n)};
  return true;
}
// The dish shows the tilt away from where the device has been held lately: a move rolls its ball the way the game's
// will, holding still lets it settle in the middle. Pressing the button starts play once the hold is steady.
void Game::updateCalibrate(uint32_t dt) {
  const float g[3] = {(float)in_.gx, (float)in_.gy, (float)in_.gz}, k = dt >= 1000 ? 1 : dt / 1000.0f;
  for (int i = 0; i < 3; i++) ref_[i] = refSet_ ? ref_[i] + (g[i] - ref_[i]) * k : g[i];
  refSet_ = true;
  rollDish(dt);
  if (in_.tapInCircle(GO_HX, GO_HY, GO_HR)) starting_ = true;
  Grav n;
  if (starting_ && steady(&n)) { neutral_ = n; play(level_); }
}
void Game::rollDish(uint32_t dt) {
  const Vec a = tiltAccel({in_.gx, in_.gy, in_.gz}, {(int)lroundf(ref_[0]), (int)lroundf(ref_[1]), (int)lroundf(ref_[2])});
  const float h = STEP_MS / 1000.0f;
  for (uint32_t t = 0; t < (dt > MAX_FRAME_MS ? MAX_FRAME_MS : dt); t += STEP_MS) {
    dishV_ = {dishV_.x + (a.x * DISH_GAIN - dish_.x * DISH_SPRING - dishV_.x * DISH_DAMP) * h,
              dishV_.y + (a.y * DISH_GAIN - dish_.y * DISH_SPRING - dishV_.y * DISH_DAMP) * h};
    dish_ = {dish_.x + dishV_.x * h, dish_.y + dishV_.y * h};
    const float d = sqrtf(dish_.x * dish_.x + dish_.y * dish_.y);
    if (d > paint::DISH_TRAVEL) { dish_ = {dish_.x * paint::DISH_TRAVEL / d, dish_.y * paint::DISH_TRAVEL / d}; dishV_ = {0, 0}; }
  }
}
bool Game::ready() const {
  return dish_.x * dish_.x + dish_.y * dish_.y < 9 && dishV_.x * dishV_.x + dishV_.y * dishV_.y < 100;
}
// Fixed substeps (tune.h), the tilt as it is this frame. Taps do nothing here: a hand on the case may brush the glass.
void Game::updatePlay(uint32_t dt) {
  stepMs_ += dt > MAX_FRAME_MS ? MAX_FRAME_MS : dt;
  const Vec a = tiltAccel({in_.gx, in_.gy, in_.gz}, neutral_);
  for (; stepMs_ >= STEP_MS && !ball_.goal; stepMs_ -= STEP_MS) step(ball_, a);
  if (!ball_.goal) return;
  if (save_.level < level_ + 1) { save_.level = (uint8_t)(level_ + 1); dirty_ = true; }
  go(SC_GOAL);
}
void Game::updateGoal() {
  if (!in_.tap && ms_ - pageMs_ < GOAL_MS) return;
  if (level_ + 1 < NUM_LEVELS) play(level_ + 1);
  else go(SC_DONE);
}
// All done: any coin plays its level again, the button starts over from level 1.
void Game::updateDone() {
  for (int i = 0; i < NUM_LEVELS; i++)
    if (in_.tapIn(coinX(i) - COIN_HALF, coinY(i) - COIN_HALF, 2 * COIN_HALF, 2 * COIN_HALF)) { play(i); return; }
  if (in_.tapInCircle(GO_HX, GO_HY, GO_HR)) play(0);
}
int Game::pressedCoin() const {
  for (int i = 0; screen_ == SC_DONE && i < NUM_LEVELS; i++) if (pressing(coinX(i), coinY(i), COIN_HALF)) return i;
  return -1;
}

paint::Box Game::moving() const {
  if (screen_ == SC_PLAY) return paint::ballBox(px(ball_.p.x), px(ball_.p.y));
  if (screen_ != SC_CALIBRATE) return paint::NONE;
  const int x = DISH_X + (int)lroundf(dish_.x), y = DISH_Y + (int)lroundf(dish_.y), r = paint::DISH_BALL_R + 2;
  return {x - r, y - r, x + r + 1, y + r + 1};
}
uint32_t Game::look() const {
  const uint32_t since = ms_ - pageMs_;
  uint32_t k = pressing(KNOB_HX, KNOB_HY, KNOB_HR) | (uint32_t)(hold() * 24) << 1;
  if (screen_ == SC_CALIBRATE || screen_ == SC_DONE) k |= (uint32_t)pressing(GO_HX, GO_HY, GO_HR) << 6;
  if (screen_ == SC_CALIBRATE) k |= (uint32_t)ready() << 7;
  if (screen_ == SC_DONE) k |= (uint32_t)(pressedCoin() + 1) << 8 | (since < paint::CONFETTI_MS ? since + 1 : 0) << 12;
  return k;
}

using DrawFn = void (Game::*)();
void Game::render() {
  static const DrawFn DRAW[] = {&Game::drawCalibrate, &Game::drawPlay, &Game::drawGoal, &Game::drawDone};
  static_assert(sizeof DRAW / sizeof DRAW[0] == SC_COUNT, "one draw per page, in Screen order");
  const paint::Box now = moving();
  const uint32_t lk = look();
  Held* h = held_[0].fb == gfx565::fb ? &held_[0] : held_[1].fb == gfx565::fb ? &held_[1] : nullptr;
  if (!h) { h = &held_[held_[0].fb ? 1 : 0]; h->fb = gfx565::fb; paint::clip(paint::FULL); }
  else if (gfx::textLogEnabled || lk != h->look) paint::clip(paint::FULL);   // the UI audit's frame logs every glyph
  else if (paint::same(now, h->moving)) return;                               // this buffer already shows this frame
  else paint::clip(paint::unite(now, h->moving));
  const DrawFn fn = DRAW[screen_];   // never (this->*TABLE[i])(): see Biscuit's game.cpp
  (this->*fn)();
  h->moving = now;
  h->look = lk;
}

// Every shadow first, then what casts them, so a shadow never lies on top of a peg.
void Game::drawTray(bool pegs) {
  const Level& l = LEVELS[level_];
  const Vec left = postAt(l, -1), right = postAt(l, 1);
  const bool ball = screen_ == SC_PLAY || screen_ == SC_GOAL;
  paint::tray(l.goalHalf);
  for (int i = 0; pegs && i < l.pegCount; i++) paint::shadow(CX + l.pegs[i].x, CY + l.pegs[i].y, l.pegs[i].r, l.goalHalf);
  paint::shadow(px(left.x), px(left.y), POST_R, l.goalHalf);
  paint::shadow(px(right.x), px(right.y), POST_R, l.goalHalf);
  paint::shadow(CX, CY + KNOB_Y, KNOB_R, l.goalHalf);
  if (ball) paint::shadow(px(ball_.p.x), px(ball_.p.y), BALL_R, l.goalHalf);
  for (int i = 0; pegs && i < l.pegCount; i++) paint::peg(CX + l.pegs[i].x, CY + l.pegs[i].y, l.pegs[i].r);
  paint::post(px(left.x), px(left.y));
  paint::post(px(right.x), px(right.y));
  if (ball) paint::ball(px(ball_.p.x), px(ball_.p.y), BALL_R);
  paint::knob(CX, CY + KNOB_Y, pressing(KNOB_HX, KNOB_HY, KNOB_HR), hold());
}
void Game::drawCalibrate() {
  drawTray(false);
  paint::coin(COIN_X, COIN_Y, level_ + 1, 22);
  paint::dish(DISH_X, DISH_Y, (int)lroundf(dish_.x), (int)lroundf(dish_.y), ready());
  paint::playButton(GO_HX * 3, GO_HY * 3, pressing(GO_HX, GO_HY, GO_HR));
}
void Game::drawPlay() {
  drawTray(true);
  paint::coin(COIN_X, COIN_Y, level_ + 1, 22);
}
void Game::drawGoal() {
  drawPlay();
  const Level& l = LEVELS[level_];
  paint::flag(px(postAt(l, -1).x), px(postAt(l, -1).y), -1);
  paint::flag(px(postAt(l, 1).x), px(postAt(l, 1).y), 1);
  paint::blocks("GOAL!", CX, 206, 7);
}
// All done: pennants up, confetti, every level's coin to play again, and the button to start over.
void Game::drawDone() {
  drawTray(false);
  const Level& l = LEVELS[level_];
  paint::flag(px(postAt(l, -1).x), px(postAt(l, -1).y), -1);
  paint::flag(px(postAt(l, 1).x), px(postAt(l, 1).y), 1);
  const int pressed = pressedCoin();
  for (int i = 0; i < NUM_LEVELS; i++) paint::coin(coinX(i) * 3, coinY(i) * 3 + (i == pressed ? 2 : 0), i + 1, 30);
  paint::playButton(GO_HX * 3, GO_HY * 3, pressing(GO_HX, GO_HY, GO_HR));
  paint::confetti(ms_ - pageMs_);
}

bool Game::takeSave(const void** data, size_t* len, bool allowed) {
  if (!allowed || !dirty_) return false;
  seal(save_);
  *data = &save_; *len = sizeof save_;   // the shell copies it out before the next update
  dirty_ = false;
  return true;
}

const char* Game::screenName() const {
  static const char* N[] = {"mk_calibrate", "mk_play", "mk_goal", "mk_done"};
  static_assert(sizeof N / sizeof N[0] == SC_COUNT, "one name per page");
  return N[screen_];
}
void Game::debugPrint() {
  printf("[marblekick level=%d reached=%u x=%d y=%d speed=%d goal=%d neutralX=%d neutralY=%d neutralZ=%d ready=%d starting=%d]\n",
         level_ + 1, (unsigned)save_.level, (int)lroundf(ball_.p.x), (int)lroundf(ball_.p.y),
         (int)lroundf(sqrtf(ball_.v.x * ball_.v.x + ball_.v.y * ball_.v.y)), ball_.goal, neutral_.x, neutral_.y,
         neutral_.z, ready(), starting_);
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
