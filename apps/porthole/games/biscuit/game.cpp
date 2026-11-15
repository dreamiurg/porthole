#include "launcher_icon.h"
#include "game.h"
#include "content_daily.h"
#include <stdio.h>
#include <string.h>
#include "generated/scenes.h"
#include "personalize.h"

namespace biscuit {
namespace {
constexpr uint32_t TICK_SEC = 30, CHECKPOINT_SEC = 300;   // needs move every 30 s; a quiet pup is saved every 5 min
constexpr uint32_t SAY_MS = 4000, ONE_SHOT_MS = 800, SURPRISE_MS = 90000;
}  // namespace

const gfx::Sprite& Game::icon() const { return LAUNCHER_ICON; }

void Game::enter(const AppEnter& e) {
  who_ = *e.who; now_ = e.nowSec; ms_ = e.ms; daylight_ = e.daylight; wallet_ = e.stars;
  save_ = create(now_);
  for (int i = 0; i < e.n; i++)   // only this kid's pup: other profiles' saves are theirs
    if (e.all[i].id == who_.id && e.saves[i].len) loadBlob(e.saves[i].data, e.saves[i].len, save_);   // bad: a new pup
  biscuit::tick(save_, now_);
  payStars(save_, wallet_);   // a pup from before the shared stars pays its earned ones in at its first visit
  dirty_ = save_.named; wantsHome_ = false;   // the visit (and its new day, if any) is saved right away
  lastTickSec_ = lastCheckpointSec_ = now_; lastSurpriseMs_ = ms_;
  fetch_ = -1; quiet(); homeFrames_.reset();   // the buffers hold the launcher
  claimed_ = save_.dailyClaimed; stickerNews_ = false;
  rng_ = now_ ^ (ms_ << 7);   // the puzzles' dice (next() turns a 0 into a seed)
  if (save_.named) go(SC_HOME);
  else { startNaming("Biscuit"); go(SC_SETUP_PET); }
}

void Game::go(Screen s) { screen_ = s; fresh(); }
// The controls under the finger just changed (a new screen, or fetch, the cue pad, a nap on this one): a tap acts on
// one layout only, and the next one waits for the fresh-screen pause (os/ui.h).
void Game::fresh() { gate_.shown(ms_); in_.tap = in_.pressed = in_.longPress = false; }
void Game::say(Say line, uint8_t activity) {
  said_ = line;
  personalize(SAY[line], who_.name, save_.petName, speech_, sizeof speech_);
  sayUntilMs_ = (ms_ + SAY_MS) | 1;   // | 1: 0 means nothing pending
  lastSurpriseMs_ = ms_;
  animate(activity);
}
void Game::sayTrick(int id) {
  const bool mastered = save_.tricks[id] >= 3;
  say(mastered ? SAY_MASTERED : SAY_LESSON, (uint8_t)(SCENE_SIT + id));
  char line[96];
  if (mastered) snprintf(line, sizeof line, "Watch me! I can do %s now!", TRICKS[id].name);
  else snprintf(line, sizeof line, "Watch me try %s! Lesson %d of 3 done.", TRICKS[id].name, save_.tricks[id]);
  personalize(line, who_.name, save_.petName, speech_, sizeof speech_);
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
  static const ScreenFn UPDATE[] = {
    &Game::updateSetupPet, &Game::updateHome, &Game::updateWorld, &Game::updateTricks, &Game::updateTraining,
    &Game::updateProfile, &Game::updateRenamePet, &Game::updateLibrary, &Game::updateStory, &Game::updateChoice,
    &Game::updateStory, &Game::updateDiscoveries, &Game::updateTopics, &Game::updateDiscovery, &Game::updateSource,
    &Game::updateToday, &Game::updateWord, &Game::updateStickers, &Game::updatePuzzles, &Game::updateTables,
    &Game::updatePuzzle, &Game::updateDoghouse, &Game::updateShop, &Game::updateItem};
  static_assert(sizeof UPDATE / sizeof UPDATE[0] == SC_COUNT, "one update per screen, in Screen order");
  now_ = nowSec; ms_ = ms; in_ = in;
  gate_.filter(in_, ms_);
  if (in_.pressed) lastSurpriseMs_ = ms_;
  tick();
  if (sayUntilMs_ && (int32_t)(ms_ - sayUntilMs_) >= 0) { personalize(SAY[SAY_IDLE], who_.name, save_.petName, speech_, sizeof speech_); sayUntilMs_ = 0; }
  if (fetch_ < 0 && actUntilMs_ && (int32_t)(ms_ - actUntilMs_) >= 0) animate(SCENE_IDLE);
  surprise();
  if (shelfAtMs_ && (int32_t)(ms_ - shelfAtMs_) >= 0) {   // the pup has pulled out a book: the Library
    shelfAtMs_ = 0;
    if (screen_ == SC_HOME && awake(save_) && fetch_ < 0) openLibrary(SC_HOME);
  }
  const ScreenFn fn = UPDATE[screen_];   // never (this->*TABLE[i])(): gcc 13.3/14.2 -fsanitize=bounds on aarch64 miscompiles it
  (this->*fn)();
  if (payStars(save_, wallet_)) markDirty();   // stars earned on this screen go into the kid's balance
  stickerNews();
}
// Today's adventure done: its sticker is earned, wherever the last activity happened. The pup says so the next time
// he is at home (over what he was saying: this is the bigger news).
void Game::stickerNews() {
  if (save_.dailyClaimed && !claimed_) stickerNews_ = true;
  claimed_ = save_.dailyClaimed;   // a new day clears it
  if (!stickerNews_ || screen_ != SC_HOME || fetch_ >= 0) return;
  stickerNews_ = false;
  say(SAY_STICKER, activity_);
}
void Game::openLibrary(Screen back) { libraryBack_ = back; page_ = 0; go(SC_LIBRARY); }
void Game::openTricks(Screen back) { tricksBack_ = back; page_ = 0; go(SC_TRICKS); }
void Game::render() {
  static const ScreenFn DRAW[] = {
    &Game::drawSetupPet, &Game::drawHome, &Game::drawWorld, &Game::drawTricks, &Game::drawTraining, &Game::drawProfile,
    &Game::drawRenamePet, &Game::drawLibrary, &Game::drawStory, &Game::drawChoice, &Game::drawStory,
    &Game::drawDiscoveries, &Game::drawTopics, &Game::drawDiscovery, &Game::drawSource, &Game::drawToday,
    &Game::drawWord, &Game::drawStickers, &Game::drawPuzzles, &Game::drawTables, &Game::drawPuzzle, &Game::drawDoghouse, &Game::drawShop, &Game::drawItem};
  static_assert(sizeof DRAW / sizeof DRAW[0] == SC_COUNT, "one draw per screen, in Screen order");
  const ScreenFn fn = DRAW[screen_];     // see update()
  if (screen_ != SC_HOME) homeFrames_.reset();   // another screen draws over what the buffers held of Home
  (this->*fn)();
}
Surface Game::surface() const { return screen_ == SC_SETUP_PET || screen_ == SC_RENAME_PET ? SURFACE_INDEXED : SURFACE_RGB565; }

// The light of the old firmware's scenes: evening from 17:00, night from 20:00 to 6:00 (local time); always day when the
// kid picked a brightness on the launcher.
Tint Game::tint() const {
  if (daylight_) return TINT_DAY;
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
                            "biscuit_profile", "biscuit_rename_pet", "biscuit_library", "biscuit_story",
                            "biscuit_choice", "biscuit_ending", "biscuit_discoveries", "biscuit_topics",
                            "biscuit_discovery", "biscuit_source", "biscuit_today", "biscuit_word", "biscuit_stickers",
                            "biscuit_puzzles", "biscuit_tables", "biscuit_puzzle", "biscuit_doghouse", "biscuit_shop",
                            "biscuit_item"};
  static_assert(sizeof N / sizeof N[0] == SC_COUNT, "one name per screen");
  return N[screen_];
}
void Game::debugPrint() {
  const Save& p = save_;
  printf("[biscuit pet=%s named=%d days=%u stage=%d fullness=%d happiness=%d energy=%d friendship=%u sleeping=%d stars=%d earned=%d decor=%u]\n",
         p.petName, p.named, (unsigned)p.daysTogether, (int)stage(p), (int)(p.fullness + 0.5f), (int)(p.happiness + 0.5f),
         (int)(p.energy + 0.5f), (unsigned)p.friendship, p.sleeping, starCount(), earnedStars(p), (unsigned)p.decor);
  printf("screen=%s fetch=%d activity=%d feeds=%u plays=%u pets=%u daily=%u tricks=%d%d%d%d%d%d trick=%d step=%d watching=%d page=%d\n",
         screenName(), fetch_, activity_, (unsigned)p.careCounts[0], (unsigned)p.careCounts[1], (unsigned)p.careCounts[2],
         p.dailyCompleted, p.tricks[0], p.tricks[1], p.tricks[2], p.tricks[3], p.tricks[4], p.tricks[5], trick_, step_, watching_, page_);
  int kept = 0;
  for (uint32_t w : p.discoveries) for (; w; w &= w - 1) kept++;
  printf("puzzles=%u kind=%d table=%d asked=%d right=%d answered=%d answer=%d books=%u more=%u\n", (unsigned)p.puzzles,
         (int)kind_, table_, asked_, right_, answered_, puzzle_.right, (unsigned)p.booksDay, (unsigned)p.moreStories);
  printf("stories=%u story=%d choice=%d at=%d of=%d facts=%d topic=%d fact=%d kept=%d stickers=%u claimed=%u said=%d\n",
         p.stories, story_, choice_, at_, total_, (int)facts_, topic_, fact_, kept, p.stickers, p.dailyClaimed, said_);
}
// The test states that only change the save; false for any other command.
static bool debugState(Save& p, const char* cmd) {
  if (!strcmp(cmd, "hungry")) p.fullness = p.happiness = p.energy = NEED_FLOOR;
  else if (!strcmp(cmd, "unlock")) { if (p.daysTogether < 7) p.daysTogether = 7; }
  else if (!strcmp(cmd, "young")) { p.daysTogether = 3; p.friendship = 20; }
  else if (!strcmp(cmd, "grown")) { p.daysTogether = 7; p.friendship = 60; }
  else if (!strcmp(cmd, "tricks")) memset(p.tricks, 3, sizeof p.tricks);
  else if (!strcmp(cmd, "practiced")) memset(p.tricks, 2, sizeof p.tricks);   // every trick on its last lesson
  else if (!strcmp(cmd, "books")) { p.booksDay = 1; if (p.daysTogether < 30) p.daysTogether = 30; }
  else if (!strcmp(cmd, "read")) {
    for (int id = 0; id < NUM_STORIES; id++) markStoryRead(p, id);
    for (int i = 0; i < NUM_DISCOVERIES; i++) p.discoveries[i / 32] |= 1u << (i % 32);
  }
  else return false;
  if (p.booksDay > p.daysTogether) p.booksDay = p.daysTogether;   // a younger pup stays valid
  return true;
}
void Game::debugCmd(const char* cmd) {
  if (!save_.named) return;
  if (!strcmp(cmd, "rich")) ::stars::award(wallet_, ::stars::MAX);   // the most stars a kid can hold
  else if (!debugState(save_, cmd)) return;
  payStars(save_, wallet_);
  markDirty();
}}  // namespace biscuit
