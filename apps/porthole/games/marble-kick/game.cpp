#include "game.h"
#include <math.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "gfx565.h"

namespace marble {
namespace {
// Logical hit areas (the audit sees each as its box): the home knob on the wall at the bottom, the big button on the
// Calibrate page (and a little lower and smaller on the Done page, under its coins), and the Done page's coins, three
// rows of four.
constexpr int KNOB_HX = 80, KNOB_HY = 145, KNOB_HR = 12;
constexpr int GO_HX = 80, GO_HY = 110, GO_HR = 14, DONE_GO_HY = 117, DONE_GO_HR = 12;
constexpr int COIN_HX[4] = {41, 67, 93, 119}, COIN_HY[3] = {38, 64, 90}, COIN_HALF = 12;
static_assert(NUM_LEVELS <= 12, "the Done page holds twelve coins");
static_assert(NUM_LEVELS <= STAR_LEVELS, "the save holds stars for every level");
constexpr int CX = gfx565::CX, CY = gfx565::CY;   // the tray's center: physics coordinates are relative to it
// The level's coin sits on the rim at the lower right, where no goal ever turns (they stay between -45 and 60 degrees).
constexpr int DISH_X = CX, DISH_Y = 196, COIN_X = 392, COIN_Y = 376;
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
int coinX(int i) { return COIN_HX[i % 4]; }
int coinY(int i) { return COIN_HY[i / 4]; }
// A goalpost where the goal is drawn: at the quarter degree its net is drawn at (paint::Mouth), so the posts never
// move without the key that says the net did.
Vec postOnScreen(const Level& l, int side, const paint::Mouth& m) {
  const float lx = (float)(side * l.goalHalf), ly = -sqrtf((float)(POST_RING * POST_RING - l.goalHalf * l.goalHalf));
  return {CX + lx * m.c - ly * m.s, CY + lx * m.s + ly * m.c};
}
Rail onScreen(const Rail& r) { return {(int16_t)(CX + r.x0), (int16_t)(CY + r.y0), (int16_t)(CX + r.x1), (int16_t)(CY + r.y1)}; }
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
  steady_ = {}; ref_.reset(); dish_ = dishV_ = {0, 0}; dishMs_ = 0;
  go(SC_CALIBRATE);
}

// A new page: every touch waits for the fresh-page pause, and both panel buffers get the whole page.
void Game::go(Screen s) {
  screen_ = s;
  gate_.shown(ms_);
  pageMs_ = ms_;
  starting_ = false;
  frames_.reset();
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
  steady_.add(in_.gx, in_.gy, in_.gz, ms_);
  hold_.step(screen_ == SC_PLAY && pressing(KNOB_HX, KNOB_HY, KNOB_HR), ms_);
  if (leaving()) { wantsHome_ = true; return; }
  switch (screen_) {
    case SC_CALIBRATE: updateCalibrate(dt); break;
    case SC_PLAY: updatePlay(dt); break;
    case SC_GOAL: updateGoal(); break;
    default: updateDone(); break;
  }
}
bool Game::pressingBox(int cx, int cy, int half) const {   // the same, for a square: a Done page coin's tap box
  return in_.down && abs(in_.downX - cx) <= half && abs(in_.downY - cy) <= half && abs(in_.x - cx) <= half && abs(in_.y - cy) <= half;
}
bool Game::goPressed() const {   // the red button: under the finger, or on Calibrate pressed and waiting for a steady grip
  if (screen_ == SC_DONE) return pressing(GO_HX, DONE_GO_HY, DONE_GO_HR);
  return pressing(GO_HX, GO_HY, GO_HR) || starting_;
}
// Home: a tap on the knob, except during play, where a hand on the case may brush it: there it takes a hold.
bool Game::leaving() {
  if (screen_ != SC_PLAY) return in_.tapInCircle(KNOB_HX, KNOB_HY, KNOB_HR);
  in_.hit(KNOB_HX - KNOB_HR, KNOB_HY - KNOB_HR, 2 * KNOB_HR, 2 * KNOB_HR);   // for the UI audit
  return hold() >= 1;   // the ring's rule (os/ui.h), not the tracker's long press
}
float Game::hold() const { return screen_ == SC_PLAY ? hold_.progress() : 0; }

// The dish shows the tilt away from where the device has been held lately: a move rolls its ball the way the game's
// will, holding still lets it settle in the middle. Pressing the button starts play once the hold is steady.
void Game::updateCalibrate(uint32_t dt) {
  ref_.add(in_.gx, in_.gy, in_.gz, dt);
  rollDish(dt);
  if (in_.tapInCircle(GO_HX, GO_HY, GO_HR)) starting_ = true;
  Grav n;
  if (starting_ && steady_.get(ms_, &n)) { neutral_ = n; play(level_); }
}
void Game::rollDish(uint32_t dt) {
  const Vec a = tiltAccel({in_.gx, in_.gy, in_.gz}, ref_.get());
  const float h = STEP_MS / 1000.0f;
  dishMs_ += dt > MAX_FRAME_MS ? MAX_FRAME_MS : dt;
  for (; dishMs_ >= STEP_MS; dishMs_ -= STEP_MS) {   // whole substeps, the rest carried to the next frame
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
  const uint8_t got = (uint8_t)starCount(ball_.stars);
  if (save_.stars[level_] < got) { save_.stars[level_] = got; dirty_ = true; }
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
  if (in_.tapInCircle(GO_HX, DONE_GO_HY, DONE_GO_HR)) play(0);
}
int Game::pressedCoin() const {
  for (int i = 0; screen_ == SC_DONE && i < NUM_LEVELS; i++) if (pressingBox(coinX(i), coinY(i), COIN_HALF)) return i;
  return -1;
}

// What moves on this page, each with a box (shadow included) and a key: the pixels in the box are fully named by the
// box and the key, so an unchanged pair needs no repaint.
Game::Mover Game::ballMover() const {
  if (sinking(ball_) && ball_.sinkMs >= SINK_MS) return {paint::NONE, -1};   // gone into the hole, not back yet
  return {paint::ballBox(px(ball_.p.x), px(ball_.p.y)), ball_.sinkMs};
}
int Game::movers(Mover* out) const {
  if (screen_ == SC_CALIBRATE) {
    const int x = DISH_X + (int)lroundf(dish_.x), y = DISH_Y + (int)lroundf(dish_.y), r = paint::DISH_BALL_R + 2;
    out[0] = {{x - r, y - r, x + r + 1, y + r + 1}, 0};
    return 1;
  }
  if (screen_ != SC_PLAY) return 0;
  const Level& l = LEVELS[level_];
  int n = 0;
  out[n++] = ballMover();
  for (int i = 0; i < l.pegCount; i++) {
    if (l.pegs[i].kind == STILL) continue;
    const Vec c = pegAt(l, l.pegs[i], ball_.ms);
    const int x = px(c.x), y = px(c.y), r = l.pegs[i].r;
    out[n++] = {{x - r - 1, y - r - 1, x + r + paint::SHADOW_DX + 2, y + r + paint::SHADOW_DY + 2}, 0};
  }
  if (l.goal.mode != FIXED) {
    const paint::Mouth m = paint::mouth(l.goalHalf, goalAngle(l, ball_.ms));
    out[n++] = {m.box, m.key};
  }
  return n;
}
uint32_t Game::look() const {
  const uint32_t since = ms_ - pageMs_;
  uint32_t k = pressing(KNOB_HX, KNOB_HY, KNOB_HR) | (uint32_t)(hold() * 24) << 1;
  if (screen_ == SC_CALIBRATE || screen_ == SC_DONE) k |= (uint32_t)goPressed() << 6;
  if (screen_ == SC_CALIBRATE) k |= (uint32_t)ready() << 7;
  if (screen_ == SC_PLAY) k |= (uint32_t)ball_.stars << 8;   // a star picked up
  if (screen_ == SC_DONE) k |= (uint32_t)(pressedCoin() + 1) << 8 | (since < paint::CONFETTI_MS ? since + 1 : 0) << 13;
  return k;
}

using DrawFn = void (Game::*)();
void Game::draw() {
  static const DrawFn DRAW[] = {&Game::drawCalibrate, &Game::drawPlay, &Game::drawGoal, &Game::drawDone};
  static_assert(sizeof DRAW / sizeof DRAW[0] == SC_COUNT, "one draw per page, in Screen order");
  const DrawFn fn = DRAW[screen_];   // never (this->*TABLE[i])(): see Biscuit's game.cpp
  (this->*fn)();
}
void Game::render() {
  Mover now[MAX_MOVERS];
  const int n = movers(now);
  frames_.render(now, n, look(), [this] { draw(); });
}

// Every shadow first, then what casts them, so a shadow never lies on top of a peg; holes and stars lie flat on the
// felt. `things`: the level's pegs, rails, holes and stars (the Done page shows a bare tray).
void Game::drawTray(bool things) {
  const Level& l = LEVELS[level_];
  const paint::Mouth m = paint::mouth(l.goalHalf, goalAngle(l, ball_.ms));
  paint::tray(m);
  if (things) for (int i = 0; i < l.pegCount; i++)
    if (l.pegs[i].kind == DEFENDER) paint::groove(onScreen({l.pegs[i].x, l.pegs[i].y, l.pegs[i].bx, l.pegs[i].by}), m);
  drawShadows(things, m);
  if (things) drawThings();
  for (int side = -1; side <= 1; side += 2) { const Vec p = postOnScreen(l, side, m); paint::post((int)lroundf(p.x), (int)lroundf(p.y)); }
  if (screen_ == SC_PLAY || screen_ == SC_GOAL) drawBall();
  paint::knob(CX, CY + KNOB_Y, pressing(KNOB_HX, KNOB_HY, KNOB_HR), hold());
}
void Game::drawShadows(bool things, const paint::Mouth& m) const {
  const Level& l = LEVELS[level_];
  for (int i = 0; things && i < l.pegCount; i++) {
    const Vec c = pegAt(l, l.pegs[i], ball_.ms);
    paint::shadow(px(c.x), px(c.y), l.pegs[i].r, m);
  }
  for (int i = 0; things && i < l.railCount; i++) paint::railShadow(onScreen(l.rails[i]), m);
  for (int side = -1; side <= 1; side += 2) { const Vec p = postOnScreen(l, side, m); paint::shadow((int)lroundf(p.x), (int)lroundf(p.y), POST_R, m); }
  paint::shadow(CX, CY + KNOB_Y, KNOB_R, m);
  if ((screen_ == SC_PLAY || screen_ == SC_GOAL) && !sinking(ball_)) paint::shadow(px(ball_.p.x), px(ball_.p.y), BALL_R, m);
}
void Game::drawThings() const {
  const Level& l = LEVELS[level_];
  for (int i = 0; i < l.holeCount; i++) paint::hole(CX + l.holes[i].x, CY + l.holes[i].y, l.holes[i].r);
  for (int i = 0; i < STARS; i++) if (!(ball_.stars >> i & 1)) paint::star(CX + l.stars[i].x, CY + l.stars[i].y, STAR_R, true);
  for (int i = 0; i < l.railCount; i++) paint::rail(onScreen(l.rails[i]));
  for (int i = 0; i < l.pegCount; i++) {
    const Vec c = pegAt(l, l.pegs[i], ball_.ms);
    if (l.pegs[i].kind == KEEPER) paint::keeper(px(c.x), px(c.y));
    else paint::peg(px(c.x), px(c.y), l.pegs[i].r);
  }
}
// The ball; dropping into a hole, it shrinks and darkens for SINK_MS, then is gone until it comes back at the start.
void Game::drawBall() const {
  if (!sinking(ball_)) { paint::ball(px(ball_.p.x), px(ball_.p.y), BALL_R, 0); return; }
  if (ball_.sinkMs >= SINK_MS) return;
  const int r = BALL_R - BALL_R * 2 / 3 * ball_.sinkMs / SINK_MS, dark = ball_.sinkMs < 150 ? 0 : ball_.sinkMs < 330 ? 1 : 2;
  paint::ball(px(ball_.p.x), px(ball_.p.y), r, dark);
}
void Game::drawFlags() const {
  const Level& l = LEVELS[level_];
  const paint::Mouth m = paint::mouth(l.goalHalf, goalAngle(l, ball_.ms));
  const float a = atan2f(m.s, m.c);
  for (int side = -1; side <= 1; side += 2) {
    const Vec p = postOnScreen(l, side, m);
    paint::flag((int)lroundf(p.x), (int)lroundf(p.y), a, side);
  }
}
void Game::drawCalibrate() {
  drawTray(true);
  paint::coin(COIN_X, COIN_Y, level_ + 1, 22);
  paint::dish(DISH_X, DISH_Y, (int)lroundf(dish_.x), (int)lroundf(dish_.y), ready());
  paint::playButton(GO_HX * 3, GO_HY * 3, goPressed());
}
void Game::drawPlay() {
  drawTray(true);
  paint::coin(COIN_X, COIN_Y, level_ + 1, 22);
}
// GOAL! on wooden blocks, pennants on the posts, and the three stars of this run: picked up ones in brass.
void Game::drawGoal() {
  drawPlay();
  drawFlags();
  paint::blocks("GOAL!", CX, 206, 7);
  for (int i = 0; i < STARS; i++) paint::star(CX - 56 + 56 * i, 306, 20, ball_.stars >> i & 1);
}
// All done: pennants up, confetti, every level's coin (its best stars under it) to play again, and the button to
// start over.
void Game::drawDone() {
  drawTray(false);
  drawFlags();
  const int pressed = pressedCoin();
  for (int i = 0; i < NUM_LEVELS; i++) {
    const int x = coinX(i) * 3, y = coinY(i) * 3 + (i == pressed ? 2 : 0);
    paint::coin(x, y, i + 1, 27);
    for (int k = 0; k < STARS; k++) paint::star(x - 14 + 14 * k, y + 33, 6, k < save_.stars[i]);
  }
  paint::playButton(GO_HX * 3, DONE_GO_HY * 3, goPressed());
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
  printf("stars=%d best=%u sinking=%d clock=%u\n", starCount(ball_.stars), (unsigned)save_.stars[level_], sinking(ball_),
         (unsigned)ball_.ms);
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
