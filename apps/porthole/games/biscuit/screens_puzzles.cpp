// Puzzles with the pup: pick a strand (plus and minus, times, words), then a round of five questions with three
// answers each, at the kid's own level for that strand: all five right moves it up, two or fewer down
// (puzzles::nextLevel), and one question of the round is from the level below. A right answer is kept (the
// scrapbook counts them, and every PUZZLES_PER_STAR of them is a star); a wrong one shows the right one and moves on.
// Any answer counts as today's training, and every PUZZLES_PER_LESSON right ones teach the pup a lesson of a trick:
// the round's end says which, and Show me has him do it at home. No timer, no score.
#include <stdio.h>
#include "content_daily.h"
#include "game.h"
#include "generated/scenes.h"
#include "gfx565.h"
#include "personalize.h"
#include "review.h"
#include "ui565.h"

namespace biscuit {
using namespace ui565;
using puzzles::Strand;
namespace {
static_assert(sizeof STRAND_LABELS / sizeof *STRAND_LABELS == puzzles::STRAND_COUNT && puzzles::STRAND_COUNT == PUZZLE_STRANDS);
static_assert(sizeof CHEERS / sizeof *CHEERS >= puzzles::ROUND && sizeof NUDGES / sizeof *NUDGES >= puzzles::ROUND);
constexpr uint16_t FILLS[3] = {SAGE, PEACH, PURPLE};
Button strandButton(int i) { return {STRAND_BUTTON[i], STRAND_LABELS[i], &FONT20, FILLS[i], true}; }
Button answerButton(const puzzles::Puzzle& p, int i) { return {ANSWER[i], p.answers[i], &FONT28, FILLS[i], true}; }
constexpr uint32_t SUMMARY_MS = 1000;
constexpr Button NEXT_PUZZLE = {WIDE, "Next", &FONT20, PURPLE, true}, MORE = {WIDE, "More puzzles", &FONT20, SAGE, true};
constexpr Button MORE_SHORT = {PUZZLE_MORE, "More", &FONT20, SAGE, true}, SHOW_ME = {PUZZLE_SHOW, "Show me", &FONT20, PURPLE, true};
}  // namespace

void Game::updatePuzzles() {
  if (tapped(in_, BACK_BUTTON)) { go(SC_WORLD); return; }
  for (int i = 0; i < puzzles::STRAND_COUNT; i++)
    if (tapped(in_, strandButton(i))) { startPuzzles((Strand)i); return; }
}
void Game::drawPuzzles() {
  gfx565::clear(PAPER);
  top(in_, "Puzzles", starCount());
  for (int i = 0; i < puzzles::STRAND_COUNT; i++) button(in_, strandButton(i));
}

// The kid's level for a strand: the one saved, or where their age starts it the first time.
uint8_t Game::puzzleLevel(Strand strand) const {
  const uint8_t saved = save_.puzzleLevels[strand];
  if (!saved) return puzzles::startLevel(strand, who_.age);
  return saved <= puzzles::LEVELS[strand] ? (uint8_t)(saved - 1) : (uint8_t)(puzzles::LEVELS[strand] - 1);
}
void Game::startPuzzles(Strand strand) {
  strand_ = strand; level_ = puzzleLevel(strand); easyAt_ = puzzles::below(rng_, puzzles::ROUND); levelUp_ = false;
  asked_ = right_ = 0; seen_ = 0; taught_ = -1; roundStars_ = earnedStars(save_);
  const int due = review::due(save_, strand, dayOf(now_));   // a missed item due again takes one place of the five
  reviewAt_ = due < 0 ? -1 : (easyAt_ + 1 + puzzles::below(rng_, puzzles::ROUND - 1)) % puzzles::ROUND;
  if (due >= 0) { review_ = review::itemOf(save_.puzzleReviews[due]); seen_ = 1u << puzzles::keyOf(review_); }
  nextPuzzle();
  go(SC_PUZZLE);
}
void Game::nextPuzzle() {
  question_ = asked_ == reviewAt_ ? review_ : puzzles::fresh(strand_, puzzles::roundLevel(level_, asked_, easyAt_), rng_, seen_);
  questionSeed_ = puzzles::next(rng_);
  seen_ |= 1u << puzzles::keyOf(question_);
  answered_ = -1;
}
// Back always goes to the strands: the answers given so far are already kept, and only a whole round moves the level.
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
    else endRound();
    roundEndMs_ = ms_;
    fresh();
    return;
  }
  const puzzles::Puzzle p = puzzle();
  for (int i = 0; i < puzzles::ANSWERS; i++) {
    if (!tapped(in_, answerButton(p, i))) continue;
    answered_ = i;
    const bool right = i == p.right;
    right_ += right;
    review::answered(save_, p.item, right, dayOf(now_));
    const int taught = answerPuzzle(save_, right, now_); markDirty();
    if (taught >= 0) taught_ = taught;
    fresh();
    return;
  }
}
// A whole round moves the kid's level for the strand (saved as level + 1: 0 means never played).
void Game::endRound() {
  const uint8_t level = puzzles::nextLevel(strand_, level_, right_);
  levelUp_ = level > level_;
  save_.puzzleLevels[strand_] = (uint8_t)(level + 1);
  markDirty();
}
// The round's end: how it went, the stars it earned (or how close the next one is), and the trick it taught.
void Game::summary(char* line, size_t cap) const {
  char stars[48];
  const int got = earnedStars(save_) - roundStars_;
  if (got > 0) snprintf(stars, sizeof stars, "+%d star%s!", got, got > 1 ? "s" : "");
  else snprintf(stars, sizeof stars, "%d more right for a star.", (int)(PUZZLES_PER_STAR - save_.puzzles % PUZZLES_PER_STAR));
  char pup[64];
  if (taught_ < 0) snprintf(pup, sizeof pup, "{pet} is so proud of you.");
  else if (save_.tricks[taught_] >= 3) snprintf(pup, sizeof pup, "{pet} can do %s now!", TRICKS[taught_].name);
  else snprintf(pup, sizeof pup, "{pet} learned %s: %d of 3.", TRICKS[taught_].name, save_.tricks[taught_]);
  snprintf(line, cap, "%d right this time!\n%s\n%s%s%s", right_, stars, pup, levelUp_ ? "\n" : "", levelUp_ ? LEVEL_UP : "");
}
// The first times level, drawn so a kid who can't read the story yet can count it: a bowls of b biscuits each.
static void drawGroups(const puzzles::Item& q) {
  static constexpr int8_t SPOT[4][2] = {{-11, -11}, {11, -11}, {-11, 11}, {11, 11}};
  for (int g = 0; g < q.a; g++) {
    const int x = gfx565::CX + (2 * g - (q.a - 1)) * 40, y = 178;
    gfx565::circle(x, y, 32, BOWL);
    for (int i = 0; i < q.b; i++) gfx565::circle(x + SPOT[i][0], y + SPOT[i][1], 8, PEACH);
  }
}
// After an answer: the solved question, then a cheer, the star it earned, or after a miss how to solve it (the pup's
// nudge when there is no how, as for words).
void Game::feedback(const puzzles::Puzzle& p, bool right, char* line, size_t cap) const {
  const bool star = right && save_.puzzles % PUZZLES_PER_STAR == 0;   // this answer earned a star
  char how[48];
  puzzles::how(p.item, how);
  const char* after = star ? STAR_CHEER : right ? CHEERS[asked_ % puzzles::ROUND] : how[0] ? how : NUDGES[asked_ % puzzles::ROUND];
  snprintf(line, cap, "%s %s\n%s", right ? "Yes!" : "It's", p.solved, after);
}
void Game::drawPuzzle() {
  const puzzles::Puzzle p = puzzle();
  const bool right = answered_ >= 0 && answered_ == p.right && asked_ < puzzles::ROUND;
  gfx565::clear(right ? SAGE : PAPER);   // green behind a right answer, so it looks different from a miss
  char s[192], line[144];
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
  if (p.item.strand == puzzles::GROUPS && p.item.level == 0) drawGroups(p.item);
  else if (p.clue[0]) text(CLUE, p.clue, INK);
  text(ASK, p.ask, INK);
  if (answered_ < 0) {
    for (int i = 0; i < puzzles::ANSWERS; i++) button(in_, answerButton(p, i));
    return;
  }
  feedback(p, right, line, sizeof line);
  personalize(line, who_.name, save_.petName, s, sizeof s);
  text(FEEDBACK, s, INK);
  button(in_, NEXT_PUZZLE);
}
}  // namespace biscuit
