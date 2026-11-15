// The Library (Little reads first, the stories, and the way to the discoveries), a story's pages, its choice and the chosen ending, and
// the page reader the discoveries share. Reading a story to its end counts as today's reading; its first finish
// earns its stars.
#include <stdio.h>
#include "content_stories.h"
#include "game.h"
#include "generated/scenes.h"
#include "gfx565.h"
#include "personalize.h"
#include "ui565.h"

namespace biscuit {
using namespace ui565;
namespace {
constexpr int SHELF_ROWS = 2, SHELF_PAGES = (1 + NUM_STORIES + SHELF_ROWS - 1) / SHELF_ROWS;   // Little reads, then the stories
constexpr Button DISCOVER = {PAIR[0], "Discoveries", &FONT20, PURPLE, true};
constexpr Button NOTEBOOK_BUTTON = {PAIR[1], "Notebook", &FONT20, PEACH, true};
uint32_t opensOn(const Save& p, int id) { return storyDay(p, id, STORIES[id].unlockDay); }
bool opened(const Save& p, int id) { return p.daysTogether >= opensOn(p, id); }
// The shelf's first row is Little reads (with the kid's level), then a story in each row under the pair; false for the
// empty row on the last page.
bool storyRow(const Save& p, uint8_t age, int row, int page, Button& b, char (&label)[64]) {
  const int id = page * SHELF_ROWS + row - 1;
  if (id < 0) {
    snprintf(label, sizeof label, "Little reads: level %d", readLevel(p, age));
    b = {ROW[row + 1], label, &FONT20, SAGE, awake(p)};
    return true;
  }
  if (id >= NUM_STORIES) return false;
  const Story& st = STORIES[id];
  const uint32_t day = opensOn(p, id);
  const bool open = p.daysTogether >= day;
  if (!open) snprintf(label, sizeof label, "%s: %s", day == p.daysTogether + 1 ? "Tomorrow" : "Soon", st.title);
  else snprintf(label, sizeof label, "%s%s", st.title, storyRead(p, id) ? " *" : "");
  b = {ROW[row + 1], label, &FONT20, PURPLE, open};
  return true;
}
// Next turns into Choose on a story's last screen and into The end on an ending's; an ending's first Previous goes
// back to the choice.
Nav reader(bool ending, int at, int total) {
  const bool last = at + 1 >= total;
  return {"Previous", !last ? "Next" : ending ? "The end" : "Choose", at > 0 || ending, true};
}
Button choiceButton(int story, int i) { return {CHOICE[i], STORIES[story].choices[i].label, &FONT20, i ? PEACH : SAGE, true}; }
}  // namespace

// ---------------------------------------------------------------- the page reader
void Game::openPages(const char* const* pages, int count, int at) {
  size_t n = 0;   // the pages filled and joined with a space (the content gate joins them the same way)
  for (int i = 0; i < count && n + 1 < sizeof read_; i++) {
    if (i) read_[n++] = ' ';
    n += personalize(pages[i], who_.name, save_.petName, read_ + n, sizeof read_ - n);
  }
  const int screens = font::pageBreaks(PAGE.box, read_, shown_, READ_SCREENS, MIN_LAST_WORDS);
  total_ = screens < READ_SCREENS ? screens : READ_SCREENS;   // the content gate holds every text to that
  at_ = at < 0 ? total_ + at : at;
}
const char* Game::pageText() const { return at_ >= 0 && at_ < total_ ? shown_[at_] : ""; }

// ---------------------------------------------------------------- Library
void Game::updateLibrary() {
  if (tapped(in_, BACK_BUTTON)) { go(libraryBack_); return; }
  if (tapped(in_, DISCOVER)) { showFacts(TODAYS_THREE, 0); topicsFrom_ = TODAYS_THREE; go(SC_DISCOVERIES); return; }
  if (tapped(in_, NOTEBOOK_BUTTON)) { showFacts(NOTEBOOK, 0); topicsFrom_ = NOTEBOOK; go(SC_DISCOVERIES); return; }
  const int turn = navTapped(in_, page_, SHELF_PAGES);
  if (turn) { page_ += turn; return; }
  for (int row = 0; row < SHELF_ROWS; row++) {
    Button b; char label[64];
    if (!storyRow(save_, who_.age, row, page_, b, label) || !tapped(in_, b)) continue;
    if (page_ * SHELF_ROWS + row == 0) startReads();
    else openStory(page_ * SHELF_ROWS + row - 1);
    return;
  }
}
void Game::drawLibrary() {
  gfx565::clear(PAPER);
  top(in_, "Our bookshelf", starCount());
  button(in_, DISCOVER);
  button(in_, NOTEBOOK_BUTTON);
  for (int row = 0; row < SHELF_ROWS; row++) {
    Button b; char label[64];
    if (storyRow(save_, who_.age, row, page_, b, label)) button(in_, b);
  }
  nav(in_, page_, SHELF_PAGES);
}

// ---------------------------------------------------------------- Story, Choice, Ending
void Game::openStory(int id) {
  if (id < 0 || id >= NUM_STORIES || !opened(save_, id) || !awake(save_)) return;
  story_ = id;
  openPages(STORIES[id].pages, STORY_PAGES, 0);
  go(SC_STORY);
}
// Every page turn changes what Previous and Next mean at the ends, so each waits for the fresh-screen pause.
void Game::updateStory() {
  const bool ending = screen_ == SC_ENDING;
  if (tapped(in_, BACK_BUTTON)) { go(SC_LIBRARY); return; }
  const int turn = navTapped(in_, reader(ending, at_, total_));
  if (!turn) return;
  if (turn < 0 && at_ == 0) { go(SC_CHOICE); return; }   // an ending's first page: back to the choice
  if (turn > 0 && at_ + 1 >= total_) {
    if (!ending) { go(SC_CHOICE); return; }
    finishStory(save_, (uint8_t)story_, now_); markDirty();
    go(SC_HOME);
    say(SAY_STORY, SCENE_READ);
    return;
  }
  at_ += turn; fresh();
}
void Game::drawStory() {
  const bool ending = screen_ == SC_ENDING;
  gfx565::clear(PAPER);
  top(in_, ending ? "The story continues" : "Story time", starCount());
  text(PAGE, pageText(), INK);
  nav(in_, reader(ending, at_, total_), at_, total_);
}
void Game::updateChoice() {
  if (tapped(in_, BACK_BUTTON)) { openPages(STORIES[story_].pages, STORY_PAGES, -1); go(SC_STORY); return; }
  for (int i = 0; i < 2; i++) {
    if (!tapped(in_, choiceButton(story_, i))) continue;
    choice_ = i;
    openPages(STORIES[story_].choices[i].ending, ENDING_PAGES, 0);
    go(SC_ENDING);
    return;
  }
}
void Game::drawChoice() {
  gfx565::clear(PAPER);
  top(in_, "What shall we do?", starCount());
  char s[256];
  personalize(STORIES[story_].prompt, who_.name, save_.petName, s, sizeof s);
  text(PROMPT, s, INK);
  for (int i = 0; i < 2; i++) button(in_, choiceButton(story_, i));
}
}  // namespace biscuit
