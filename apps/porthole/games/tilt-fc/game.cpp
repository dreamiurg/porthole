#include "game.h"
#include <math.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "gfx565.h"

namespace fc {
namespace {
// Logical hit areas (the audit sees each as its box): the leave sign on the left of every page, the go sign under the
// centre circle on the Calibrate page (and drawn, as a hint, on the Full time page, where a tap anywhere plays on).
constexpr int SIGN_HX = paint::SIGN_X / 3, SIGN_HY = paint::SIGN_Y / 3, SIGN_HR = 12;
constexpr int GO_X = paint::CX, GO_Y = 348, FT_GO_Y = 376, GO_HX = GO_X / 3, GO_HY = GO_Y / 3, GO_HALF = 14;
constexpr int CX = paint::CX, CY = paint::CY;
constexpr int COUNTDOWN_Y = 92, GOAL_Y = 160;   // the big words' line tops
// Who is who besides the shirt numbers (paint::NUMBERS): hair and kits.
constexpr uint8_t HAIR[PLAYERS] = {0, 1, 2, 1, 3, 0};
constexpr paint::Kit KIT[PLAYERS] = {paint::KIT_TEAL, paint::KIT_TEAL, paint::KIT_TEAL_KEEPER,
                                     paint::KIT_CORAL, paint::KIT_CORAL, paint::KIT_CORAL_KEEPER};

// The launcher icon, in the shell's indexed palette (the launcher is the shell's): the street court from above, a
// goal at each end, the kid's player ringed in sun yellow with the ball at his feet, a coral player up the court.
// k outline, p paving, w chalk and the ball, s asphalt, n net, y sun, g/m teal, o/r coral.
constexpr char ICON_ART[] =
  "....kkkkkkkkkkkkkkkkkkkkkkkk...." "..kkppppppppppppppppppppppppkk.." ".kkppppppppwnnnnnnnnwppppppppkk."
  ".kpppppppppwnnnnnnnnwpppppppppk." "kppppppppppwnnnnnnnnwppppppppppk" "kpppppwwwwwwwwwwwwwwwwwwwwpppppk"
  "kppppwwsssssssssssssssssswwppppk" "kpppwwssssssssssssssooossswwpppk" "kpppwssssssssssssssorrrossswpppk"
  "kpppwsssssssssssssorrrrrosswpppk" "kpppwssssssssswwwworrrrrosswpppk" "kpppwsssssssswwwwworrrrrosswpppk"
  "kpppwssssssswwsssssorrrossswpppk" "kpppwsssssswwsssssssooosssswpppk" "kpppwsssssswwssssssswwssssswpppk"
  "kpppwwwwwwwwwwwwwwwwwwwwwwwwpppk" "kpppwwwwyyyyywwwwwwwwwwwwwwwpppk" "kpppwssyyyyyyysssssswwssssswpppk"
  "kpppwsyyygggyyyssssswwssssswpppk" "kpppwyyygmmmgyyyssswwsssssswpppk" "kpppwyygmmmmmgyywwwwssssssswpppk"
  "kpppwyygmmmmmgkkkwwsssssssswpppk" "kpppwyygmmmmmgkwksssssssssswpppk" "kpppwyyygmmmgykkksssssssssswpppk"
  "kpppwwyyygggyyyssssssssssswwpppk" "kppppwwyyyyyyyssssssssssswwppppk" "kpppppwwyyyyywwwwwwwwwwwwwpppppk"
  "kppppppppppwnnnnnnnnwppppppppppk" ".kpppppppppwnnnnnnnnwpppppppppk." ".kkppppppppwnnnnnnnnwppppppppkk."
  "..kkppppppppppppppppppppppppkk.." "....kkkkkkkkkkkkkkkkkkkkkkkk....";
static_assert(sizeof ICON_ART == 32 * 32 + 1, "the launcher slot fits a 32x32 icon");
constexpr char ICON_KEYS[] = "kpwsnygmor";
constexpr uint8_t ICON_COLS[] = {C_BLACK, C_TAN, C_WHITE, C_SLATE, C_DKGRAY, C_YELLOW, C_DKGREEN, C_MINT, C_BROWN, C_ROSE};
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

int sx(float x) { return CX + (int)lroundf(x); }   // match.h's court to the panel
int sy(float y) { return CY + (int)lroundf(y); }
bool within(int x, int y, int cx, int cy, int r) { return (x - cx) * (x - cx) + (y - cy) * (y - cy) <= r * r; }
// Players are drawn facing one of 16 ways (the head's place), so a key names exactly what a player's pixels show.
int dirIndex(Vec f) { return ((int)lroundf(atan2f(f.y, f.x) * 8 / 3.14159265f) + 16) % 16; }
Vec dirVec(int k) { const float a = k * 3.14159265f / 8; return {cosf(a), sinf(a)}; }
constexpr float ARROW_FULL_MG = 350;   // Calibrate: the arrow is longest from this tilt on
struct Arrow { int dir, len; bool ready; };
Arrow arrowFor(tilt::Vec t) {
  const float m = sqrtf(t.x * t.x + t.y * t.y);
  if (m <= DEAD_MG) return {0, 0, true};
  const float k = m >= ARROW_FULL_MG ? 1 : (m - DEAD_MG) / (ARROW_FULL_MG - DEAD_MG);
  return {((int)lroundf(atan2f(t.y, t.x) * 16 / 3.14159265f) + 32) % 32, 18 + 6 * (int)lroundf(k * 10), false};
}
Vec arrowDir(int k) { const float a = k * 3.14159265f / 16; return {cosf(a), sinf(a)}; }
}  // namespace

const gfx::Sprite& Game::icon() const { return ICON; }

// Every visit starts on the Calibrate page with the kid's own difficulty.
void Game::enter(const AppEnter& e) {
  now_ = e.nowSec; ms_ = e.ms; in_ = Input{};
  save_ = Save{};
  for (int i = 0; i < e.n; i++)   // only this kid's save; a bad one starts from the first level
    if (e.all[i].id == e.who->id && e.saves[i].len) loadBlob(e.saves[i].data, e.saves[i].len, save_);
  if (save_.level >= LEVELS) save_.level = LEVELS - 1;
  dirty_ = wantsHome_ = false;
  matchNo_ = 0;
  match_ = start(save_.level, 0);
  steady_ = {};
  go(SC_CALIBRATE);
}

// A new page: every touch waits for the fresh-page pause, and both panel buffers get the whole page.
void Game::go(Screen s) {
  screen_ = s;
  gate_.shown(ms_);
  pageMs_ = ms_;
  starting_ = tapPending_ = false;
  ref_.reset();
  stepMs_ = 0;
  frames_.reset();
}
void Game::newMatch() {
  match_ = start(save_.level, ++matchNo_);
  go(SC_KICKOFF);
}
// Full time: a win counts and makes the next match a little harder; a loss by two or more makes it a little easier.
void Game::finish() {
  const int teal = match_.score[TEAL], coral = match_.score[CORAL];
  if (teal > coral) { save_.wins++; if (save_.level + 1 < LEVELS) save_.level++; }
  else if (coral - teal >= 2 && save_.level > 0) save_.level--;
  dirty_ = true;
  go(SC_FULLTIME);
}

void Game::update(uint32_t nowSec, uint32_t ms, const Input& in) {
  const uint32_t dt = ms - ms_;
  now_ = nowSec; ms_ = ms; in_ = in;
  gate_.filter(in_, ms_);
  steady_.add(in_.gx, in_.gy, in_.gz, ms_);
  tilt_ = tilt::from({in_.gx, in_.gy, in_.gz}, neutral_);
  if (leaving()) { wantsHome_ = true; return; }
  switch (screen_) {
    case SC_CALIBRATE: updateCalibrate(dt); break;
    case SC_KICKOFF: updateKickoff(); break;
    case SC_MATCH: updateMatch(dt); break;
    case SC_GOAL: updateGoal(); break;
    default: updateFullTime(); break;
  }
}
bool Game::onSign(int x, int y) const { return within(x, y, SIGN_HX, SIGN_HY, SIGN_HR); }
// Leaving: a tap on the sign, except in a match (Kickoff, Match, Goal), where a hand on the case may brush it: there it
// takes a hold.
bool Game::leaving() {
  if (!inPlay()) return in_.tapInCircle(SIGN_HX, SIGN_HY, SIGN_HR);
  in_.hit(SIGN_HX - SIGN_HR, SIGN_HY - SIGN_HR, 2 * SIGN_HR, 2 * SIGN_HR);   // for the UI audit
  return hold() >= 1;
}
float Game::hold() const { return ui::holdProgress(in_, inPlay() && pressing(SIGN_HX, SIGN_HY, SIGN_HR)); }

tilt::Vec Game::calTilt() const {
  return tilt::from({in_.gx, in_.gy, in_.gz}, ref_.get());
}
bool Game::ready() const { return arrowFor(calTilt()).ready; }
// The arrow shows the tilt away from where the device has been held lately: a move points it the way the player will
// run, holding still settles it. The go sign starts the match once the grip is steady (tilt::Steady).
void Game::updateCalibrate(uint32_t dt) {
  ref_.add(in_.gx, in_.gy, in_.gz, dt);
  if (in_.tapIn(GO_HX - GO_HALF, GO_HY - GO_HALF, 2 * GO_HALF, 2 * GO_HALF)) starting_ = true;
  tilt::Grav n;
  if (starting_ && steady_.get(ms_, &n)) { neutral_ = n; newMatch(); }
}
void Game::updateKickoff() {
  if (ms_ - pageMs_ >= KICKOFF_MS) go(SC_MATCH);
}
// Fixed steps (tune.h), the tilt as it is this frame. A press anywhere but on the leave sign is the kid's one button.
void Game::updateMatch(uint32_t dt) {
  stepMs_ += dt > MAX_FRAME_MS ? MAX_FRAME_MS : dt;
  tapPending_ = tapPending_ || (in_.pressed && !onSign(in_.downX, in_.downY));
  for (; stepMs_ >= STEP_MS && match_.scored < 0 && !match_.over; stepMs_ -= STEP_MS) {
    step(match_, tilt_, tapPending_);
    tapPending_ = false;
  }
  if (match_.scored >= 0) go(SC_GOAL);
  else if (match_.over) finish();
}
void Game::updateGoal() {
  if (!(in_.tap && !onSign(in_.x, in_.y)) && ms_ - pageMs_ < GOAL_MS) return;
  if (finished(match_)) { finish(); return; }
  kickoff(match_, 1 - match_.scored);
  go(SC_KICKOFF);
}
void Game::updateFullTime() {
  in_.hit(GO_HX - GO_HALF, FT_GO_Y / 3 - GO_HALF, 2 * GO_HALF, 2 * GO_HALF);   // the hint; a tap anywhere plays on
  if (in_.tap && !onSign(in_.x, in_.y)) go(SC_CALIBRATE);
}

Kick Game::kidAim() const {
  if (match_.ball.owner != match_.control) return {Kick::ROLL, -1, {0, 0}};
  return aim(match_, match_.control, aimDir(match_, tilt_));
}
// What moves on this page, each with a box (shadow included) and a key: the pixels in the box are fully named by the
// box and the key, so an unchanged pair needs no repaint.
int Game::movers(Mover* out) const {
  if (screen_ == SC_FULLTIME) return 0;
  if (screen_ == SC_CALIBRATE) {
    const Arrow a = arrowFor(calTilt());
    const Vec d = arrowDir(a.dir);
    const int tx = CX + (int)lroundf(d.x * (40 + a.len)), ty = CY + (int)lroundf(d.y * (40 + a.len));
    const paint::Box me = paint::playerBox(CX, CY), tip = {tx - 16, ty - 16, tx + 17, ty + 17};
    out[0] = {paint::unite(me, a.ready ? paint::Box{CX - 36, CY - 36, CX + 37, CY + 37} : tip), a.ready ? -1 : a.dir << 8 | a.len};
    return 1;
  }
  const Kick k = kidAim();
  int n = 0;
  for (int i = 0; i < PLAYERS; i++) {
    const Player& p = match_.pl[i];
    const int key = dirIndex(p.face) | (i == match_.control) << 4 | (k.kind == Kick::PASS && k.to == i) << 5;
    out[n++] = {paint::playerBox(sx(p.p.x), sy(p.p.y)), key};
  }
  out[n++] = {paint::ballBox(sx(match_.ball.p.x), sy(match_.ball.p.y)), 0};
  out[n++] = {k.kind == Kick::SHOOT ? paint::aimBox(sx(k.at.x), sy(k.at.y) - NET_DEPTH / 2) : paint::NONE, 0};
  out[n++] = {paint::clockBox(), (int)((float)match_.ms / MATCH_MS * 24 + 0.5f)};
  return n;
}
uint32_t Game::look() const {
  const bool goPressed = screen_ == SC_CALIBRATE && (pressing(GO_HX, GO_HY, GO_HALF) || starting_);
  const uint32_t countdown = screen_ == SC_KICKOFF ? 3 - (ms_ - pageMs_) * 3 / KICKOFF_MS : 0;
  return (uint32_t)pressing(SIGN_HX, SIGN_HY, SIGN_HR) | (uint32_t)(hold() * 24) << 1 | (uint32_t)match_.score[TEAL] << 6 |
         (uint32_t)match_.score[CORAL] << 9 | countdown << 12 | (uint32_t)goPressed << 14;
}

using DrawFn = void (Game::*)();
void Game::draw() {
  static const DrawFn DRAW[] = {&Game::drawCalibrate, &Game::drawKickoff, &Game::drawPitch, &Game::drawGoal, &Game::drawFullTime};
  static_assert(sizeof DRAW / sizeof DRAW[0] == SC_COUNT, "one draw per page, in Screen order");
  const DrawFn fn = DRAW[screen_];   // never (this->*TABLE[i])(): see Biscuit's game.cpp
  (this->*fn)();
}
void Game::render() {
  Mover now[MAX_MOVERS];
  const int n = movers(now);
  frames_.render(now, n, look(), [this] { draw(); });
}

// Shadows first, then the kid's ring on the ground, the players nearest the top first (a player lower down stands in
// front), and the ball over them all: at a player's feet his head would hide it, and the ball is what the kid follows.
// Then the scoreboard and the leave sign.
void Game::drawPitch() {
  paint::court();
  const Kick k = kidAim();
  if (k.kind == Kick::SHOOT) paint::aimSpot(sx(k.at.x), sy(k.at.y) - NET_DEPTH / 2);
  for (const Player& p : match_.pl) paint::playerShadow(sx(p.p.x), sy(p.p.y));
  const Ball& b = match_.ball;
  paint::ballShadow(sx(b.p.x), sy(b.p.y));
  const Player& c = match_.pl[match_.control];
  paint::ring(sx(c.p.x), sy(c.p.y));
  int order[PLAYERS];
  for (int i = 0; i < PLAYERS; i++) {   // insertion sort by row, index breaking ties
    int j = i;
    for (; j > 0 && sy(match_.pl[order[j - 1]].p.y) > sy(match_.pl[i].p.y); j--) order[j] = order[j - 1];
    order[j] = i;
  }
  for (int i : order) {
    const Player& p = match_.pl[i];
    paint::player(sx(p.p.x), sy(p.p.y), dirVec(dirIndex(p.face)),
                  {KIT[i], HAIR[i], paint::NUMBERS[i], isKeeper(i), k.kind == Kick::PASS && k.to == i});
  }
  paint::ball(sx(b.p.x), sy(b.p.y));
  paint::scoreboard(match_.score[TEAL], match_.score[CORAL]);
  paint::clock((float)match_.ms / MATCH_MS);
  paint::leaveSign(pressing(SIGN_HX, SIGN_HY, SIGN_HR), hold());
}
// The kid's number 10 alone in the centre circle, an arrow the way a tilt would send him (a ring of sun when held
// still), the go sign under him.
void Game::drawCalibrate() {
  paint::court();
  const Arrow a = arrowFor(calTilt());
  paint::playerShadow(CX, CY);
  if (a.ready) paint::readyRing(CX, CY);
  else paint::arrow(CX, CY, arrowDir(a.dir), a.len);
  paint::player(CX, CY, {0, -1}, {paint::KIT_TEAL, HAIR[0], paint::NUMBERS[0], false, false});
  paint::goSign(GO_X, GO_Y, pressing(GO_HX, GO_HY, GO_HALF) || starting_);
  paint::leaveSign(pressing(SIGN_HX, SIGN_HY, SIGN_HR), 0);
}
void Game::drawKickoff() {
  static const char* const COUNT[] = {"1", "2", "3"};
  drawPitch();
  const uint32_t left = 3 - (ms_ - pageMs_) * 3 / KICKOFF_MS;   // look() counts down the same way
  paint::banner(COUNT[(left < 1 ? 1 : left > 3 ? 3 : left) - 1], COUNTDOWN_Y);
}
void Game::drawGoal() {
  drawPitch();
  paint::banner(paint::GOAL_WORD, GOAL_Y);
}
void Game::drawFullTime() {
  drawPitch();
  paint::result(match_.score[TEAL], match_.score[CORAL], match_.score[TEAL] > match_.score[CORAL]);
  paint::goSign(GO_X, FT_GO_Y, false);
}

bool Game::takeSave(const void** data, size_t* len, bool allowed) {
  if (!allowed || !dirty_) return false;
  seal(save_);
  *data = &save_; *len = sizeof save_;   // the shell copies it out before the next update
  dirty_ = false;
  return true;
}

const char* Game::screenName() const {
  static const char* N[] = {"fc_calibrate", "fc_kickoff", "fc_match", "fc_goal", "fc_fulltime"};
  static_assert(sizeof N / sizeof N[0] == SC_COUNT, "one name per page");
  return N[screen_];
}
void Game::debugPrint() {
  const Player& c = match_.pl[match_.control];
  printf("[tiltfc wins=%u level=%u teal=%d coral=%d clock=%u control=%d owner=%d x=%d y=%d bx=%d by=%d]\n",
         (unsigned)save_.wins, (unsigned)save_.level, match_.score[TEAL], match_.score[CORAL], (unsigned)(match_.ms / 1000),
         match_.control, match_.ball.owner, (int)lroundf(c.p.x), (int)lroundf(c.p.y), (int)lroundf(match_.ball.p.x),
         (int)lroundf(match_.ball.p.y));
  printf("neutralX=%d neutralY=%d neutralZ=%d ready=%d starting=%d\n", neutral_.x, neutral_.y, neutral_.z, ready(), starting_);
  printf("screen=%s\n", screenName());
}
void Game::debugCmd(const char* cmd) {
  if (!strcmp(cmd, "kickoff")) { neutral_ = {in_.gx, in_.gy, in_.gz}; newMatch(); return; }
  int t, c;
  if (sscanf(cmd, "score%d-%d", &t, &c) == 2 && t >= 0 && c >= 0 && t < GOALS_TO_WIN && c < GOALS_TO_WIN) {
    match_.score[TEAL] = (uint8_t)t; match_.score[CORAL] = (uint8_t)c;
    return;
  }
  if (sscanf(cmd, "clock%d", &t) == 1 && t >= 0 && (uint32_t)t * 1000 <= MATCH_MS) match_.ms = MATCH_MS - (uint32_t)t * 1000;
}
}  // namespace fc
