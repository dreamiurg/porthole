#include "game.h"
#include "content_daily.h"
#include <stdio.h>
#include <string.h>
#include "generated/icon.h"
#include "generated/scenes.h"
#include "personalize.h"

namespace biscuit {
namespace {
constexpr uint32_t TICK_SEC = 30, CHECKPOINT_SEC = 300;   // needs move every 30 s; a quiet pup is saved every 5 min
constexpr uint32_t SAY_MS = 4000, ONE_SHOT_MS = 800, SURPRISE_MS = 90000;
}  // namespace

const gfx::Sprite& Game::icon() const { return SPR_BISCUIT_ICON; }

void Game::enter(const AppEnter& e) {
  who_ = *e.who; now_ = e.nowSec; ms_ = e.ms;
  save_ = create(now_);
  for (int i = 0; i < e.n; i++)   // only this kid's pup: other profiles' saves are theirs
    if (e.all[i].id == who_.id && e.saves[i].len) loadBlob(e.saves[i].data, e.saves[i].len, save_);   // bad: a new pup
  biscuit::tick(save_, now_);
  dirty_ = save_.named; wantsHome_ = false;   // the visit (and its new day, if any) is saved right away
  lastTickSec_ = lastCheckpointSec_ = now_; lastSurpriseMs_ = ms_;
  fetch_ = -1; quiet();
  if (save_.named) go(SC_HOME);
  else { startNaming("Biscuit"); go(SC_SETUP_PET); }
}

void Game::go(Screen s) { screen_ = s; fresh(); }
// The controls under the finger just changed (a new screen, or fetch, the cue pad, a nap on this one): a tap acts on
// one layout only, and the next one waits for the fresh-screen pause (os/ui.h).
void Game::fresh() { gate_.shown(ms_); in_.tap = in_.pressed = in_.longPress = false; }
void Game::say(Say line, uint8_t activity) {
  personalize(SAY[line], who_.name, save_.petName, speech_, sizeof speech_);
  sayUntilMs_ = (ms_ + SAY_MS) | 1;   // | 1: 0 means nothing pending
  lastSurpriseMs_ = ms_;
  animate(activity);
}
void Game::animate(uint8_t activity) {
  const bool oneShot = activity == SCENE_SHELF || activity == SCENE_FERN;
  activity_ = activity; actStartMs_ = ms_; actUntilMs_ = activity == SCENE_IDLE ? 0 : (ms_ + (oneShot ? ONE_SHOT_MS : SAY_MS)) | 1;
}
void Game::quiet() { personalize(SAY[SAY_IDLE], who_.name, save_.petName, speech_, sizeof speech_); sayUntilMs_ = 0; animate(SCENE_IDLE); }

// Needs and the day move on every TICK_SEC. The shell writes a dirty save within 5 s, so a quiet pup only marks one
// at a new day or a checkpoint; every care action marks its own.
void Game::tick() {
  if (!save_.named || now_ - lastTickSec_ < TICK_SEC) return;   // a clock set back wraps: it ticks now (a no-op)
  const int32_t day = save_.lastVisitDay;
  biscuit::tick(save_, now_);
  lastTickSec_ = now_;
  if (save_.lastVisitDay != day || now_ - lastCheckpointSec_ >= CHECKPOINT_SEC) { markDirty(); lastCheckpointSec_ = now_; }
}
// A pup that has mastered a trick shows it off on its own, after SURPRISE_MS with no touch and nothing said.
void Game::surprise() {
  if (screen_ != SC_HOME || !awake(save_) || fetch_ >= 0 || sayUntilMs_ || actUntilMs_ || ms_ - lastSurpriseMs_ < SURPRISE_MS) return;
  lastSurpriseMs_ = ms_;
  for (int n = 0; n < NUM_TRICKS; n++) {
    const int id = (int)((ms_ / SURPRISE_MS + n) % NUM_TRICKS);
    if (save_.tricks[id] >= 3) { say(SAY_SHOW_OFF, (uint8_t)(SCENE_SIT + id)); return; }
  }
}

using ScreenFn = void (Game::*)();
void Game::update(uint32_t nowSec, uint32_t ms, const Input& in) {
  static const ScreenFn UPDATE[] = {&Game::updateSetupPet, &Game::updateHome, &Game::updateWorld, &Game::updateTricks,
                                    &Game::updateTraining, &Game::updateProfile, &Game::updateRenamePet};
  static_assert(sizeof UPDATE / sizeof UPDATE[0] == SC_COUNT, "one update per screen, in Screen order");
  now_ = nowSec; ms_ = ms; in_ = in;
  gate_.filter(in_, ms_);
  if (in_.pressed) lastSurpriseMs_ = ms_;
  tick();
  if (sayUntilMs_ && (int32_t)(ms_ - sayUntilMs_) >= 0) { personalize(SAY[SAY_IDLE], who_.name, save_.petName, speech_, sizeof speech_); sayUntilMs_ = 0; }
  if (fetch_ < 0 && actUntilMs_ && (int32_t)(ms_ - actUntilMs_) >= 0) animate(SCENE_IDLE);
  surprise();
  const ScreenFn fn = UPDATE[screen_];   // never (this->*TABLE[i])(): gcc 13.3/14.2 -fsanitize=bounds on aarch64 miscompiles it
  (this->*fn)();
}
void Game::render() {
  static const ScreenFn DRAW[] = {&Game::drawSetupPet, &Game::drawHome, &Game::drawWorld, &Game::drawTricks,
                                  &Game::drawTraining, &Game::drawProfile, &Game::drawRenamePet};
  static_assert(sizeof DRAW / sizeof DRAW[0] == SC_COUNT, "one draw per screen, in Screen order");
  const ScreenFn fn = DRAW[screen_];     // see update()
  (this->*fn)();
}
Surface Game::surface() const { return screen_ == SC_SETUP_PET || screen_ == SC_RENAME_PET ? SURFACE_INDEXED : SURFACE_RGB565; }

// The light of the old firmware's scenes: evening from 17:00, night from 20:00 to 6:00 (local time).
Tint Game::tint() const {
  const uint32_t h = now_ / 3600 % 24;
  return h >= 20 || h < 6 ? TINT_NIGHT : h >= 17 ? TINT_EVENING : TINT_DAY;
}

bool Game::takeSave(const void** data, size_t* len, bool allowed) {
  if (!allowed || !dirty_ || !save_.named) return false;   // an unnamed pup has nothing worth keeping
  seal(save_); out_ = save_;
  *data = &out_; *len = sizeof out_;
  dirty_ = false;
  return true;
}

const char* Game::screenName() const {
  static const char* N[] = {"biscuit_setup_pet", "biscuit_home", "biscuit_world", "biscuit_tricks", "biscuit_training",
                            "biscuit_profile", "biscuit_rename_pet"};
  static_assert(sizeof N / sizeof N[0] == SC_COUNT, "one name per screen");
  return N[screen_];
}
void Game::debugPrint() {
  const Save& p = save_;
  printf("[biscuit pet=%s named=%d days=%u stage=%d fullness=%d happiness=%d energy=%d friendship=%u sleeping=%d stars=%d]\n",
         p.petName, p.named, (unsigned)p.daysTogether, (int)stage(p), (int)(p.fullness + 0.5f), (int)(p.happiness + 0.5f),
         (int)(p.energy + 0.5f), (unsigned)p.friendship, p.sleeping, stars(p));
  printf("screen=%s fetch=%d activity=%d feeds=%u plays=%u pets=%u daily=%u tricks=%d%d%d%d%d%d trick=%d step=%d watching=%d page=%d\n",
         screenName(), fetch_, activity_, (unsigned)p.careCounts[0], (unsigned)p.careCounts[1], (unsigned)p.careCounts[2],
         p.dailyCompleted, p.tricks[0], p.tricks[1], p.tricks[2], p.tricks[3], p.tricks[4], p.tricks[5], trick_, step_, watching_, page_);
}
void Game::debugCmd(const char* cmd) {
  if (!save_.named) return;
  if (!strcmp(cmd, "hungry")) save_.fullness = save_.happiness = save_.energy = NEED_FLOOR;
  else if (!strcmp(cmd, "unlock")) { if (save_.daysTogether < 7) save_.daysTogether = 7; }
  else if (!strcmp(cmd, "young")) { save_.daysTogether = 3; save_.friendship = 20; }
  else if (!strcmp(cmd, "grown")) { save_.daysTogether = 7; save_.friendship = 60; }
  else if (!strcmp(cmd, "tricks")) memset(save_.tricks, 3, sizeof save_.tricks);
  else if (!strcmp(cmd, "practiced")) memset(save_.tricks, 2, sizeof save_.tricks);   // every trick on its last lesson
  else return;
  markDirty();
}
}  // namespace biscuit
