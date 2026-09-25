// Tricks (six, each unlocking on a day together) and Training: watch the lesson's cues, then tap them on the pad.
// Three lessons master a trick; a wrong cue just shows the lesson again.
#include <stdio.h>
#include <string.h>
#include "content_daily.h"
#include "game.h"
#include "generated/scenes.h"
#include "gfx565.h"
#include "ui565.h"

namespace biscuit {
using namespace ui565;
namespace {
constexpr int TRICK_PAGES = (NUM_TRICKS + TRICK_ROWS - 1) / TRICK_ROWS;
const char* const CUE_NAME[5] = {"Left", "Up", "Right", "Down", "Paw"};   // by Cue
constexpr Button MY_TURN = {WIDE, "My turn", &FONT20, PURPLE, true}, PEEK = {WIDE, "Peek again", &FONT20, PEACH, true};
bool unlocked(const Save& p, int id) { return p.daysTogether >= trickUnlockDay((uint8_t)id); }
Lesson current(const Save& p, int id) { return lesson((uint8_t)id, p.tricks[id]); }   // lesson() caps at the third
Button trickRow(const Save& p, int row, int page, char* label, size_t cap) {
  const int id = page * TRICK_ROWS + row;
  if (unlocked(p, id)) snprintf(label, cap, "%s   %d/3", TRICKS[id].name, p.tricks[id]);
  else snprintf(label, cap, "%s   Day %d", TRICKS[id].name, trickUnlockDay((uint8_t)id));
  return {TRICK_ROW[row], label, &FONT20, PURPLE, unlocked(p, id)};
}
}  // namespace

void Game::updateTricks() {
  if (tapped(in_, BACK_BUTTON)) { go(SC_WORLD); return; }
  const int turn = navTapped(in_, page_, TRICK_PAGES);
  if (turn) { page_ += turn; return; }
  for (int row = 0; row < TRICK_ROWS; row++) {
    char label[48];
    if (tapped(in_, trickRow(save_, row, page_, label, sizeof label))) { openTrick(page_ * TRICK_ROWS + row); return; }
  }
}
void Game::drawTricks() {
  gfx565::clear(PAPER);
  top(in_, "Little paws, big ideas", stars(save_));
  for (int row = 0; row < TRICK_ROWS; row++) {
    char label[48];
    button(in_, trickRow(save_, row, page_, label, sizeof label));
  }
  nav(in_, page_, TRICK_PAGES);
}

// A mastered trick is shown off at home (and still counts as today's practice); any other opens its next lesson.
void Game::openTrick(int id) {
  if (id < 0 || id >= NUM_TRICKS || !unlocked(save_, id)) return;
  trick_ = id; step_ = 0; watching_ = true;
  if (save_.tricks[id] < 3) { go(SC_TRAINING); return; }
  practice(save_, (uint8_t)id, now_); markDirty();
  go(SC_HOME);
  say(SAY_SHOW_OFF, (uint8_t)(SCENE_SIT + id));
}
void Game::updateTraining() {
  if (tapped(in_, BACK_BUTTON)) { go(SC_TRICKS); return; }
  if (watching_) { if (tapped(in_, MY_TURN)) { watching_ = false; step_ = 0; fresh(); } return; }   // the pad appears
  if (tapped(in_, PEEK)) { watching_ = true; step_ = 0; fresh(); return; }
  const Lesson l = current(save_, trick_);
  for (int c = 0; c < 5; c++) {
    if (!tapped(in_, {CUE_KEY[c], CUE_NAME[c], &FONT20, PURPLE, true})) continue;
    if (c != (int)l.cues[step_]) { watching_ = true; step_ = 0; fresh(); return; }   // show it again, nothing lost
    if (++step_ < l.length) return;
    practice(save_, (uint8_t)trick_, now_); markDirty();
    go(SC_HOME);
    say(save_.tricks[trick_] >= 3 ? SAY_MASTERED : SAY_LESSON, (uint8_t)(SCENE_SIT + trick_));
    return;
  }
}
void Game::drawTraining() {
  gfx565::clear(PAPER);
  const Lesson l = current(save_, trick_);
  if (watching_) {
    top(in_, TRICKS[trick_].name, stars(save_));
    text(TRAIN_HINT, "Have a look, then try with me.", INK);
    char cues[64] = "";
    for (int i = 0; i < l.length; i++) {
      snprintf(cues + strlen(cues), sizeof cues - strlen(cues), "%s%s", i ? " - " : "", CUE_NAME[(int)l.cues[i]]);
    }
    text(TRAIN_CUES, cues, INK);
    button(in_, MY_TURN);
    return;
  }
  char title[40]; snprintf(title, sizeof title, "Your turn!  %d / %d", step_, l.length);
  top(in_, title, stars(save_));
  for (int c = 0; c < 5; c++) button(in_, {CUE_KEY[c], CUE_NAME[c], &FONT20, PURPLE, true});
  button(in_, PEEK);
}
}  // namespace biscuit
