// Home (the room: the pup, its needs, what it says, the four actions, fetch) and World (where the rest of the game is).
#include <ctype.h>
#include <math.h>
#include <stdio.h>
#include <string.h>
#include "content_daily.h"
#include "game.h"
#include "generated/scenes.h"
#include "gfx565.h"
#include "personalize.h"
#include "ui565.h"

namespace biscuit {
using namespace ui565;
namespace {
constexpr const char* DOT = " \xC2\xB7 ";   // " · " in UTF-8: the font has the middle dot
constexpr uint32_t SCENE_FRAME_MS = 230, SCENE_SLEEP_MS = 600;   // the scene frame steps generated/scenes.h names
int need(float v) { return (int)lroundf(v); }
Button action(int i, bool sleeping) {   // Feed, Play, Pet, More (Wake while napping: nothing else then)
  static const char* const LABEL[4] = {"Feed", "Play", "Pet", "More"};
  static const uint16_t FILL[4] = {PEACH, SAGE, PURPLE, PURPLE};
  return {ACTIONS[i], i == 3 && sleeping ? "Wake" : LABEL[i], &FONT20, FILL[i], i == 3 || !sleeping};
}
constexpr Icon ACTION_ICON[4] = {Icon::Bowl, Icon::Ball, Icon::Heart, Icon::Paw};
constexpr Button ALL_DONE_BUTTON = {ALL_DONE, "All done", &FONT20, PURPLE, true};
int mastered(const Save& p) { int n = 0; for (uint8_t t : p.tricks) n += t >= 3; return n; }
}  // namespace

// ---------------------------------------------------------------- Home
void Game::toggleNap() {
  toggleSleep(save_, now_);
  fetch_ = -1;
  say(save_.sleeping ? SAY_NIGHT : SAY_MORNING, SCENE_IDLE);
  markDirty();
  if (screen_ != SC_HOME) go(SC_HOME); else fresh();   // napping disables (or brings back) most of Home
}
void Game::updateHome() {
  if (homeTapped(in_)) { fetch_ = -1; wantsHome_ = true; return; }   // out to the launcher, from any moment
  if (fetch_ >= 0) homeFetch(); else homeTaps();
}
void Game::homeTaps() {
  const bool up = awake(save_);
  if (tapped(in_, action(0, save_.sleeping))) {
    biscuit::act(save_, Action::Feed, now_); markDirty();
    say(SAY_FED, SCENE_FEED);
  } else if (tapped(in_, action(1, save_.sleeping))) {
    fetch_ = 0; sayUntilMs_ = 0; animate(SCENE_PLAY); fresh();
  } else if (tapped(in_, action(2, save_.sleeping)) || (up && in_.tapIn(PUP.x, PUP.y, PUP.w, PUP.h))) {
    biscuit::act(save_, Action::Petting, now_); markDirty();
    say(SAY_PETTED, SCENE_PET);
  } else if (tapped(in_, action(3, save_.sleeping))) {
    if (save_.sleeping) toggleNap(); else go(SC_WORLD);
  } else if (in_.tapIn(WINDOW.x, WINDOW.y, WINDOW.w, WINDOW.h)) {
    toggleNap();
  } else if (up && in_.tapIn(FERN.x, FERN.y, FERN.w, FERN.h)) {
    say(SAY_FERN, SCENE_FERN);
  } else if (up && in_.tapIn(SHELF.x, SHELF.y, SHELF.w, SHELF.h)) {
    animate(SCENE_SHELF);   // TODO(biscuit 7b): the pup pulls out a book, then the Library opens (800 ms)
  }
}
// Fetch: the ball waits at one of five spots; five catches and the play counts. Nothing is lost by stopping early.
void Game::homeFetch() {
  if (tapped(in_, ALL_DONE_BUTTON)) { fetch_ = -1; quiet(); fresh(); return; }
  const Box& b = BALL[fetch_ % 5];
  if (!in_.tapIn(b.x, b.y, b.w, b.h) || ++fetch_ < 5) return;
  fetch_ = -1; fresh();
  biscuit::act(save_, Action::Play, now_); markDirty();
  say(SAY_FETCHED, SCENE_PLAY);
}

void Game::drawScene() {
  const bool oneShot = activity_ == SCENE_SHELF || activity_ == SCENE_FERN;
  const int act = save_.sleeping ? (int)SCENE_SLEEP : fetch_ >= 0 ? (int)SCENE_PLAY : activity_;
  const uint32_t since = ms_ - actStartMs_, step = save_.sleeping ? SCENE_SLEEP_MS : SCENE_FRAME_MS;   // scenes.h
  const int frame = oneShot && !save_.sleeping ? (since / step < 3 ? (int)(since / step) : 3) : (int)(ms_ / step % 4);
  gfx565::blitRle(SCENES[(int)stage(save_)][timeOfDay()][act][frame], 0, 0, 3);   // the whole glass
}
void Game::drawHomeTop() {
  char name[NAME_LIMIT + 1], mood[48];
  for (size_t i = 0; i < sizeof name; i++) name[i] = (char)toupper((unsigned char)save_.petName[i]);
  if (save_.sleeping) snprintf(mood, sizeof mood, "dreaming of stories");
  else snprintf(mood, sizeof mood, "Day %u%s%s", (unsigned)save_.daysTogether, DOT, STAGES[(int)stage(save_)]);
  if (!save_.sleeping) mood[strlen(mood) - strlen(STAGES[(int)stage(save_)])] = (char)tolower(STAGES[(int)stage(save_)][0]);
  const uint16_t ink = save_.sleeping || timeOfDay() == 2 ? PAPER : INK;   // the room is dark
  text(HOME_MOOD, mood, ink);
  const int room = HOME_MOOD.x + HOME_MOOD.box.w - font::textWidth(FONT16, mood) - HOME_NAME_GAP - HOME_NAME.x;
  const font::Font* f = HOME_NAME.box.font;
  const char* shown = name;
  if (font::textWidth(*f, shown) > room) f = &FONT16;
  if (font::textWidth(*f, shown) > room) shown = save_.petName;   // ponytail: a 12-letter W name can still touch the mood
  font::text(*f, shown, HOME_NAME.x, HOME_NAME.y + (HOME_NAME.box.font->lineHeight - f->lineHeight), ink);
  ui565::need(0, Icon::Bowl, need(save_.fullness), BOWL);
  ui565::need(1, Icon::Heart, need(save_.happiness), HEART);
  ui565::need(2, Icon::Moon, need(save_.energy), MOON);
}
void Game::drawHome() {
  drawScene();
  drawHomeTop();
  home(in_);
  if (fetch_ >= 0) {
    char s[40]; snprintf(s, sizeof s, "Catch the ball!  %d / 5", fetch_);
    text(FETCH_SCORE, s, timeOfDay() == 2 ? PAPER : INK);   // over the sky, dark at night
    const Box& b = BALL[fetch_ % 5];   // the box is the target, not the ball's pixels
    tennisBall(b.x * 3 + (b.w * 3 - 30) / 2, b.y * 3 + (b.h * 3 - 30) / 2);
    button(in_, ALL_DONE_BUTTON);
    return;
  }
  if (awake(save_)) { hotspot(in_, SHELF); hotspot(in_, FERN); hotspot(in_, PUP); }
  hotspot(in_, WINDOW);
  bubble(speech_);
  for (int i = 0; i < 4; i++) actionButton(in_, action(i, save_.sleeping), ACTION_ICON[i]);
}

// ---------------------------------------------------------------- World
static Tile worldTile(int i, const Save& p) {
  static char tricks[24];
  snprintf(tricks, sizeof tricks, "%d of 6 mastered", mastered(p));
  if (i == 0) return {WORLD_TRICKS, "Learn tricks", tricks, Icon::Paw, SAGE};
  if (i == 1) return {WORLD_NAP, p.sleeping ? "Wake up" : "Cozy nap", "A lovely place to pause", Icon::Moon, PURPLE};
  return {WORLD_BOOK, "Our scrapbook", "Growing up, page by page", Icon::Book, PEACH};
}
void Game::updateWorld() {
  const Box& back = WORLD_BACK;
  if (in_.tapIn(back.x, back.y, back.w, back.h)) { go(SC_HOME); return; }
  for (int i = 0; i < 3; i++) {
    const Box& b = worldTile(i, save_).box;
    if (!in_.tapIn(b.x, b.y, b.w, b.h)) continue;
    if (i == 0) { page_ = 0; go(SC_TRICKS); }
    else if (i == 1) toggleNap();
    else go(SC_PROFILE);
    return;
  }
}
void Game::drawWorld() {
  gfx565::clear(PAPER);
  roundButton(in_, WORLD_BACK, "<");
  char s[80];
  personalize("{pet} & {name}", who_.name, save_.petName, s, sizeof s);
  text(WORLD_NAMES, font::textWidth(*WORLD_NAMES.box.font, s) > WORLD_NAMES.box.w ? save_.petName : s, INK);
  snprintf(s, sizeof s, "Day %u%s%s%s%d stars", (unsigned)save_.daysTogether, DOT, STAGES[(int)stage(save_)], DOT, stars(save_));
  text(WORLD_LINE, s, INK);
  for (int i = 0; i < 3; i++) tile(in_, worldTile(i, save_));
}
}  // namespace biscuit
