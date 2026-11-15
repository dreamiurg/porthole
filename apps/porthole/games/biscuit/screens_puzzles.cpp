// Puzzles with the pup: pick a kind (or a times table), then a round of five questions with three answers each. A
// right answer is kept (the scrapbook counts them, and every PUZZLES_PER_STAR of them is a star); a wrong one shows
// the right one and moves on. Any answer counts as today's training, and every PUZZLES_PER_LESSON right ones teach
// the pup a lesson of a trick: the round's end says which, and Show me has him do it at home. No timer, no score.
#include <stdio.h>
#include "content_daily.h"
#include "game.h"
#include "generated/scenes.h"
#include "gfx565.h"
#include "personalize.h"
#include "ui565.h"

namespace biscuit {
using namespace ui565;
using puzzles::Kind;
namespace {
constexpr int GRID_COUNT = 6;
static_assert(sizeof KIND_LABELS / sizeof *KIND_LABELS == puzzles::KIND_COUNT && GRID_COUNT == puzzles::KIND_COUNT);
static_assert(sizeof CHEERS / sizeof *CHEERS >= puzzles::ROUND && sizeof NUDGES / sizeof *NUDGES >= puzzles::ROUND);
constexpr uint16_t FILLS[GRID_COUNT] = {SAGE, PEACH, PURPLE, SAGE, PEACH, PURPLE};
Button gridButton(int i, const char* const* labels) { return {GRID[i], labels[i], &FONT20, FILLS[i], true}; }
Button answerButton(const puzzles::Puzzle& p, int i) { return {ANSWER[i], p.answers[i], &FONT28, FILLS[i], true}; }
constexpr uint32_t SUMMARY_MS = 1000;
constexpr Button NEXT_PUZZLE = {WIDE, "Next", &FONT20, SAGE, true}, MORE = {WIDE, "More puzzles", &FONT20, SAGE, true};
constexpr Button MORE_SHORT = {PUZZLE_MORE, "More", &FONT20, SAGE, true}, SHOW_ME = {PUZZLE_SHOW, "Show me", &FONT20, PURPLE, true};
// The kinds' and the tables' screens: six buttons under a title.
int tappedGrid(const Input& in, const char* const* labels) {
  for (int i = 0; i < GRID_COUNT; i++)
    if (tapped(in, gridButton(i, labels))) return i;
  return -1;
}
void drawGrid(const Input& in, const char* title, int stars, const char* const* labels) {
  gfx565::clear(PAPER);
  top(in, title, stars);
  for (int i = 0; i < GRID_COUNT; i++) button(in, gridButton(i, labels));
}
}  // namespace

void Game::updatePuzzles() {
  if (tapped(in_, BACK_BUTTON)) { go(SC_WORLD); return; }
  const int i = tappedGrid(in_, KIND_LABELS);
  if (i == puzzles::TIMES) go(SC_TABLES);
  else if (i >= 0) startPuzzles((Kind)i, 0);
}
void Game::drawPuzzles() { drawGrid(in_, "Puzzles", starCount(), KIND_LABELS); }
void Game::updateTables() {
  if (tapped(in_, BACK_BUTTON)) { go(SC_PUZZLES); return; }
  const int i = tappedGrid(in_, TABLE_LABELS);   // the last button mixes them all (table 0)
  if (i >= 0) startPuzzles(puzzles::TIMES, i < GRID_COUNT - 1 ? (uint8_t)(puzzles::FIRST_TABLE + i) : 0);
}
void Game::drawTables() { drawGrid(in_, "Times tables", starCount(), TABLE_LABELS); }

void Game::startPuzzles(Kind kind, uint8_t table) {
  kind_ = kind; table_ = table; asked_ = right_ = 0; seen_ = 0; taught_ = -1; roundStars_ = earnedStars(save_);
  nextPuzzle();
  go(SC_PUZZLE);
}
void Game::nextPuzzle() {
  puzzle_ = puzzles::make(kind_, table_, rng_, seen_);
  seen_ |= 1u << puzzle_.key;
  answered_ = -1;
}
// Back always goes to the kinds: the answers given so far are already kept.
void Game::updatePuzzle() {
  if (tapped(in_, BACK_BUTTON)) { go(SC_PUZZLES); return; }
  if (asked_ >= puzzles::ROUND) {
    if (ms_ - roundEndMs_ < SUMMARY_MS) return;   // a slow second tap on the last Next must not skip the round's end
    if (taught_ < 0 ? tapped(in_, MORE) : tapped(in_, MORE_SHORT)) { go(SC_PUZZLES); return; }
    if (taught_ < 0 || !tapped(in_, SHOW_ME)) return;
    go(SC_HOME);
    sayTrick(taught_);
    return;
  }
  if (answered_ >= 0) {
    if (!tapped(in_, NEXT_PUZZLE)) return;
    if (++asked_ < puzzles::ROUND) nextPuzzle();
    roundEndMs_ = ms_;
    fresh();
    return;
  }
  for (int i = 0; i < puzzles::ANSWERS; i++) {
    if (!tapped(in_, answerButton(puzzle_, i))) continue;
    answered_ = i;
    const bool right = i == puzzle_.right;
    right_ += right;
    const int taught = answerPuzzle(save_, right, now_); markDirty();
    if (taught >= 0) taught_ = taught;
    fresh();
    return;
  }
}
// The round's end: how it went, the stars it earned (or how close the next one is), and the trick it taught.
void Game::summary(char* line, size_t cap) const {
  char stars[48];
  const int got = earnedStars(save_) - roundStars_;
  if (got > 0) snprintf(stars, sizeof stars, "+%d star%s!", got, got > 1 ? "s" : "");
  else snprintf(stars, sizeof stars, "%d more right for a star.", (int)(PUZZLES_PER_STAR - save_.puzzles % PUZZLES_PER_STAR));
  if (taught_ < 0) snprintf(line, cap, "%d right this time!\n%s\n{pet} is so proud of you.", right_, stars);
  else if (save_.tricks[taught_] >= 3) snprintf(line, cap, "%d right this time!\n%s\n{pet} can do %s now!", right_, stars, TRICKS[taught_].name);
  else snprintf(line, cap, "%d right this time!\n%s\n{pet} learned %s: %d of 3.", right_, stars, TRICKS[taught_].name, save_.tricks[taught_]);
}
void Game::drawPuzzle() {
  gfx565::clear(PAPER);
  char s[160], line[96];
  if (asked_ >= puzzles::ROUND) {
    top(in_, "All done!", starCount());
    summary(line, sizeof line);
    personalize(line, who_.name, save_.petName, s, sizeof s);
    text(PUZZLE_DONE, s, INK);
    if (taught_ < 0) { button(in_, MORE); return; }
    button(in_, MORE_SHORT);
    button(in_, SHOW_ME);
    return;
  }
  snprintf(line, sizeof line, "Puzzle %d of %d", asked_ + 1, puzzles::ROUND);
  top(in_, line, starCount());
  if (puzzle_.clue) text(CLUE, puzzle_.clue, INK);
  text(ASK, puzzle_.ask, INK);
  if (answered_ < 0) {
    for (int i = 0; i < puzzles::ANSWERS; i++) button(in_, answerButton(puzzle_, i));
    return;
  }
  const bool right = answered_ == puzzle_.right;
  const bool star = right && save_.puzzles % PUZZLES_PER_STAR == 0;   // this answer earned a star
  const char* after = star ? STAR_CHEER : right ? CHEERS[asked_ % puzzles::ROUND] : NUDGES[asked_ % puzzles::ROUND];
  snprintf(line, sizeof line, "%s %s\n%s", right ? "Yes!" : "It's", puzzle_.solved, after);
  personalize(line, who_.name, save_.petName, s, sizeof s);
  text(FEEDBACK, s, INK);
  button(in_, NEXT_PUZZLE);
}
}  // namespace biscuit
