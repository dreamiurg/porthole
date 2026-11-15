// Puzzles with the pup: pick a kind (or a times table), then a round of five questions with three answers each. A
// right answer is kept (the scrapbook counts them); a wrong one shows the right one and moves on. Any answer counts
// as today's training. No timer, no score to beat.
#include <stdio.h>
#include "game.h"
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
constexpr Button NEXT_PUZZLE = {WIDE, "Next", &FONT20, SAGE, true}, MORE = {WIDE, "More puzzles", &FONT20, SAGE, true};
// The kinds' and the tables' screens: six buttons under a title.
int tappedGrid(const Input& in, const char* const* labels) {
  for (int i = 0; i < GRID_COUNT; i++)
    if (tapped(in, gridButton(i, labels))) return i;
  return -1;
}
void drawGrid(const Input& in, const char* title, uint8_t stars, const char* const* labels) {
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
void Game::drawPuzzles() { drawGrid(in_, "Puzzles", stars(save_), KIND_LABELS); }
void Game::updateTables() {
  if (tapped(in_, BACK_BUTTON)) { go(SC_PUZZLES); return; }
  const int i = tappedGrid(in_, TABLE_LABELS);   // the last button mixes them all (table 0)
  if (i >= 0) startPuzzles(puzzles::TIMES, i < GRID_COUNT - 1 ? (uint8_t)(puzzles::FIRST_TABLE + i) : 0);
}
void Game::drawTables() { drawGrid(in_, "Times tables", stars(save_), TABLE_LABELS); }

void Game::startPuzzles(Kind kind, uint8_t table) {
  kind_ = kind; table_ = table; asked_ = right_ = 0; seen_ = 0;
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
  if (asked_ >= puzzles::ROUND) { if (tapped(in_, MORE)) go(SC_PUZZLES); return; }
  if (answered_ >= 0) {
    if (!tapped(in_, NEXT_PUZZLE)) return;
    if (++asked_ < puzzles::ROUND) nextPuzzle();
    fresh();
    return;
  }
  for (int i = 0; i < puzzles::ANSWERS; i++) {
    if (!tapped(in_, answerButton(puzzle_, i))) continue;
    answered_ = i;
    const bool right = i == puzzle_.right;
    right_ += right;
    answerPuzzle(save_, right, now_); markDirty();
    fresh();
    return;
  }
}
void Game::drawPuzzle() {
  gfx565::clear(PAPER);
  char s[160], line[96];
  if (asked_ >= puzzles::ROUND) {
    top(in_, "All done!", stars(save_));
    snprintf(line, sizeof line, "Five puzzles with {pet}!\n%d right this time.\n{pet} is so proud of you.", right_);
    personalize(line, who_.name, save_.petName, s, sizeof s);
    text(PUZZLE_DONE, s, INK);
    button(in_, MORE);
    return;
  }
  snprintf(line, sizeof line, "Puzzle %d of %d", asked_ + 1, puzzles::ROUND);
  top(in_, line, stars(save_));
  if (puzzle_.clue) text(CLUE, puzzle_.clue, INK);
  text(ASK, puzzle_.ask, INK);
  if (answered_ < 0) {
    for (int i = 0; i < puzzles::ANSWERS; i++) button(in_, answerButton(puzzle_, i));
    return;
  }
  const bool right = answered_ == puzzle_.right;
  const char* after = right ? CHEERS[asked_ % puzzles::ROUND] : NUDGES[asked_ % puzzles::ROUND];
  snprintf(line, sizeof line, "%s %s\n%s", right ? "Yes!" : "It's", puzzle_.solved, after);
  personalize(line, who_.name, save_.petName, s, sizeof s);
  text(FEEDBACK, s, INK);
  button(in_, NEXT_PUZZLE);
}
}  // namespace biscuit
