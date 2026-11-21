// Puzzles and little reads, from the Play menu (moved here from Biscuit on 2026-11-16: they were meant for Pets Club).
// Puzzles: pick a strand (plus and minus, times, words), then a round of five questions with three answers each, at
// the kid's own level for that strand, which the skills engine (os/skills.h) moves after every answer; one question of
// the round comes from the level below, and a puzzle missed comes back on a later day (review.h).
// Little reads: a round of READ_ROUND short passages at the kid's reading level, each read page by page like a book,
// then one question with three answers, the level following the engine's reading rating. A right answer brings the dog
// closer (pet::LEARN_BOND); at a round's end the engine pays its stars into the kid's wallet (os/stars.h), more for what
// is hard for this kid, and Hats spends them. A miss shows the right answer and how to get it, and moves on. No timer, no score.
#include <stdio.h>
#include <string.h>
#include "content_reads.h"
#include "game.h"
#include "review.h"
#include "sprites.h"
#include "stars.h"

using namespace gfx;
namespace puzzles = learn::puzzles;
namespace review = learn::review;
using learn::READS;
using learn::READ_COUNT;

namespace {
constexpr int READ_ROUND = 3;
constexpr uint32_t PUZZLE_READ_MS = 800, READ_ASK_MS = 1000;   // no right answer comes sooner unless guessed
constexpr uint32_t SUMMARY_MS = 1000;   // a slow second tap on the last Next must not skip the round's end
static_assert(READ_COUNT <= 64, "readsSeen_ holds a bit per read");
static_assert(sizeof learn::STRAND_LABELS / sizeof *learn::STRAND_LABELS == puzzles::STRAND_COUNT && puzzles::STRAND_COUNT == PUZZLE_STRANDS);
static_assert(sizeof learn::CHEERS / sizeof *learn::CHEERS >= puzzles::ROUND && sizeof learn::NUDGES / sizeof *learn::NUDGES >= puzzles::ROUND);
const uint8_t STRAND_COLS[3] = {C_GREEN, C_ORANGE, C_BLUE};
bool tapBox(const Input& in, const ui::Box& b) { return in.tapIn(b.x, b.y, b.w, b.h); }
ui::Box strandBox(int i) { return {26, 56 + i * 28, 108, 24}; }
ui::Box answerBox(int i) { return {24 + i * 39, 90, 34, 30}; }
const ui::Box NEXT_BTN = {56, 122, 48, 22}, MORE_BTN = {50, 118, 60, 22}, MORE_READS_BTN = {44, 118, 72, 22};
const int READ_ANSWER_STEP = 24, READ_Q_TOP = 32, PAGE_LINES = 8;
// A line of the round's end and the feedback, wrapped and centered from y down; returns the next free y.
int lines(int y, const char* s, uint8_t col, int maxLines = 3, int width = 124) {
  char l[6][40]; const int n = wrap(s, width, l, maxLines > 6 ? 6 : maxLines);
  for (int i = 0; i < n; i++) textCentered(80, y + i * 9, l[i], col);
  return y + n * 9;
}
}  // namespace

// {pet} and {name} in a line, filled in with this dog's and this kid's names.
void Game::fillNames(const char* src, char* out, size_t cap) const {
  size_t at = 0;
  while (*src && at + 1 < cap) {
    const char* put = !strncmp(src, "{pet}", 5) ? save_.petName : !strncmp(src, "{name}", 6) ? save_.kidName : nullptr;
    if (!put) { out[at++] = *src++; continue; }
    src += put == save_.petName ? 5 : 6;
    for (; *put && at + 1 < cap; put++) out[at++] = *put;
  }
  out[at] = 0;
}
// The round's points into stars at its end, by the engine's rules (os/skills.h): harder for this kid pays more.
void Game::payRound() {
  roundStars_ = skills::finish(*book_, pay_, now_, wallet_);
  if (roundStars_) { sound(SND_CHIME); spawn(80, 60, 4, 6); }
}
// The engine's skill for a strand: sums, times stories, spelling.
uint8_t Game::readLevelNow() const {
  return (uint8_t)(skills::target(*book_, skills::READ_EN, pet::READ_LEVELS, learn::READ_ANSWERS) + 1);
}
skills::Skill Game::puzzleSkill() const {
  return strand_ == puzzles::NUMBERS ? skills::ADD_SUB : strand_ == puzzles::GROUPS ? skills::TIMES_STORY : skills::SPELL_EN;
}
void Game::drawStarCount(int y) const {
  char b[24]; const uint32_t n = stars::balance(wallet_);
  snprintf(b, sizeof b, "%lu star%s", (unsigned long)n, n == 1 ? "" : "s");
  const int w = textWidth(b) + 10;
  blit(SPR_STAR, 80 - w / 2, y); text(80 - w / 2 + 10, y, b, C_DKBROWN);
}

// ---------------------------------------------------------------- puzzles: the strands
void Game::updatePuzzles() {
  if (backButton()) { go(SC_PLAYMENU); return; }
  for (int i = 0; i < puzzles::STRAND_COUNT; i++)
    if (tapBox(in_, strandBox(i))) { startPuzzles(i); return; }
}
void Game::drawPuzzles() {
  clear(C_WALL);
  textCentered(80, 30, "Puzzles", C_PLUM, 2);
  drawStarCount(46);
  for (int i = 0; i < puzzles::STRAND_COUNT; i++) ui::button(in_, {strandBox(i), learn::STRAND_LABELS[i], nullptr, STRAND_COLS[i]});
  drawBackButton();
}

// The kid's level for a strand, from the engine's rating. The first time, the rating starts from the level saved before
// the engine (known, no calibration) or else from where the kid's age starts the strand, and calibrates from there.
uint8_t Game::puzzleLevel(int strand) const {
  const skills::Skill k = strand == puzzles::NUMBERS ? skills::ADD_SUB : strand == puzzles::GROUPS ? skills::TIMES_STORY : skills::SPELL_EN;
  const uint8_t saved = save_.puzzleLevels[strand], top = puzzles::LEVELS[strand];
  if (saved) skills::seed(*book_, k, saved <= top ? saved - 1 : top - 1, true);
  else skills::seed(*book_, k, puzzles::startLevel((puzzles::Strand)strand, save_.kidAge), false);
  return (uint8_t)skills::target(*book_, k, top, puzzles::ANSWERS);
}
void Game::startPuzzles(int strand) {
  if (!rng_) rng_ = pet::rnd(save_);
  strand_ = (uint8_t)strand; level_ = puzzleLevel(strand); easyAt_ = (int8_t)puzzles::below(rng_, puzzles::ROUND);
  levelUp_ = false; asked_ = right_ = roundStars_ = 0; seen_ = 0; pay_ = skills::start(*book_, now_); roundLevel_ = level_;
  const int due = review::due(save_, (puzzles::Strand)strand, (int32_t)pet::dayIndex(now_));   // a missed one takes a place
  reviewAt_ = (int8_t)(due < 0 ? -1 : (easyAt_ + 1 + puzzles::below(rng_, puzzles::ROUND - 1)) % puzzles::ROUND);
  if (due >= 0) { review_ = review::itemOf(save_.puzzleReviews[due]); seen_ = 1u << puzzles::keyOf(review_); }
  nextPuzzle();
  go(SC_PUZZLE);
}
void Game::nextPuzzle() {
  question_ = asked_ == reviewAt_ ? review_ : puzzles::fresh((puzzles::Strand)strand_, puzzles::roundLevel(level_, asked_, easyAt_), rng_, seen_);
  questionSeed_ = puzzles::next(rng_);
  seen_ |= 1u << puzzles::keyOf(question_);
  answered_ = -1; screenMs_ = shownMs_ = ms_;   // a finger still down from Next answers nothing
}
learn::puzzles::Puzzle Game::puzzle() const { uint32_t s = questionSeed_; return puzzles::build(question_, s); }

// Back always goes to the strands: the answers given so far are kept, and only a whole round moves the level.
void Game::updatePuzzle() {
  if (backButton()) { go(SC_PUZZLES); return; }
  if (asked_ >= puzzles::ROUND) {
    if (ms_ - roundEndMs_ >= SUMMARY_MS && tapBox(in_, MORE_BTN) && pressedAfter(0)) go(SC_PUZZLES);
    return;
  }
  if (answered_ >= 0) {
    if (!tapBox(in_, NEXT_BTN) || !pressedAfter(0)) return;
    if (++asked_ < puzzles::ROUND) { nextPuzzle(); return; }
    const uint8_t level = puzzleLevel(strand_);   // the engine's, saved as level + 1 (for a build before it)
    levelUp_ = level > roundLevel_ || pay_.newBest; save_.puzzleLevels[strand_] = (uint8_t)(level + 1);
    pet::learnRoundDone(save_); payRound(); markDirty();
    roundEndMs_ = screenMs_ = ms_; sound(SND_HAPPY); spawn(80, 60, 4, 10);
    return;
  }
  const puzzles::Puzzle p = puzzle();
  for (int i = 0; i < puzzles::ANSWERS; i++) {
    if (!tapBox(in_, answerBox(i)) || !pressedAfter(0)) continue;
    answered_ = i; screenMs_ = ms_;
    const bool right = i == p.right;
    right_ += right;
    review::answered(save_, p.item, right, (int32_t)pet::dayIndex(now_));
    pet::puzzleAnswered(save_, now_, right);   // the counters and the bond; the stars are the engine's
    skills::answer(*book_, pay_, puzzleSkill(), p.item.level * skills::LEVEL, puzzles::ANSWERS, right, ms_ - shownMs_,
                   PUZZLE_READ_MS, (uint16_t)(p.item.strand << 12 | p.item.level << 8 | puzzles::keyOf(p.item)));
    level_ = puzzleLevel(strand_);   // the next question follows the rating at once
    markDirty();
    if (right) { sound(SND_YIP); spawn(answerBox(i).x + 17, 100, 1, 4); }
    return;
  }
}
// The first times level, drawn so a kid who can't read the story yet can count it: a bowls of b biscuits each.
static void drawGroups(const puzzles::Item& q) {
  static constexpr int8_t SPOT[4][2] = {{-4, -4}, {4, -4}, {-4, 4}, {4, 4}};
  for (int g = 0; g < q.a; g++) {
    const int x = 80 + (2 * g - (q.a - 1)) * 16, y = 52;
    circle(x, y + 1, 12, C_DKBROWN); circle(x, y, 12, C_TAN);
    for (int i = 0; i < q.b; i++) circle(x + SPOT[i][0], y + SPOT[i][1], 3, C_BROWN);
  }
}
void Game::drawPuzzle() {
  if (asked_ >= puzzles::ROUND) { drawPuzzleDone(); return; }
  const puzzles::Puzzle p = puzzle();
  const bool right = answered_ >= 0 && answered_ == p.right;
  clear(right ? C_MINT : C_WALL);   // green behind a right answer, so it looks different from a miss
  char b[32];
  snprintf(b, sizeof b, "Puzzle %d of %d", asked_ + 1, puzzles::ROUND); textCentered(80, 30, b, C_DKGRAY);
  if (p.item.strand == puzzles::GROUPS && p.item.level == 0) drawGroups(p.item);
  else if (p.clue[0]) lines(44, p.clue, C_NAVY, 2);
  const int big = textWidth(p.ask, 2) <= 124 ? 2 : 1;
  textCentered(80, big == 2 ? 68 : 72, p.ask, C_DKBROWN, big);
  if (answered_ < 0) drawPuzzleAnswers(p);
  else drawPuzzleFeedback(p, right);
  drawBackButton();
}
void Game::drawPuzzleAnswers(const puzzles::Puzzle& p) {
  for (int i = 0; i < puzzles::ANSWERS; i++) {
    const ui::Box a = answerBox(i); const bool pr = ui::pressing(in_, a) && pressedAfter(0);
    ui::drawButton({a, nullptr, nullptr, STRAND_COLS[i]}, pr);
    textCentered(a.x + a.w / 2, a.y + 8 + (pr ? 1 : 0), p.answers[i], ui::inkOn(STRAND_COLS[i]), textWidth(p.answers[i], 2) <= a.w - 4 ? 2 : 1);
  }
}
// After the answer: the sum solved, then a cheer, the star's cheer, or after a miss how to solve it.
void Game::drawPuzzleFeedback(const puzzles::Puzzle& p, bool right) {
  char b[96], s[96];
  snprintf(b, sizeof b, "%s %s", right ? "Yes!" : "It's", p.solved); textCentered(80, 90, b, right ? C_NAVY : C_PLUM);
  char how[48]; puzzles::how(p.item, how);
  const bool star = false;   // stars come at the round's end now (os/skills.h)
  const char* after = star ? learn::STAR_CHEER : right ? learn::CHEERS[asked_ % puzzles::ROUND] : how[0] ? how : learn::NUDGES[asked_ % puzzles::ROUND];
  fillNames(after, s, sizeof s); lines(102, s, C_DKBROWN, 2);
  ui::button(in_, {NEXT_BTN, "Next", nullptr, C_GREEN});
}
// The round's end: how it went, the stars it earned (or how close the next one is), and a harder level next time.
void Game::drawPuzzleDone() {
  clear(C_WALL);
  ui::panel({18, 30, 124, 84}, C_CREAM, C_DKBROWN);
  char b[64], s[64];
  snprintf(b, sizeof b, "%d right this time!", right_); textCentered(80, 36, b, C_PLUM);
  if (roundStars_) snprintf(b, sizeof b, "+%d star%s!", roundStars_, roundStars_ > 1 ? "s" : "");
  else snprintf(b, sizeof b, "Almost a star!");
  int y = lines(50, b, C_NAVY, 2, 112);
  fillNames("{pet} is so proud of you.", s, sizeof s); y = lines(y + 2, s, C_DKBROWN, 2, 112);
  if (levelUp_) lines(y + 2, learn::LEVEL_UP, C_DKGREEN, 2, 112);
  if (roundStars_) blit(SPR_STAR, 112, 50);
  ui::button(in_, {MORE_BTN, "More", nullptr, C_GREEN});
  drawBackButton();
}

// ---------------------------------------------------------------- little reads
void Game::startReads() {
  if (!rng_) rng_ = pet::rnd(save_);
  // The reading rating starts where Little reads stood (or the kid's age puts it): known, no calibration.
  skills::seed(*book_, skills::READ_EN, pet::readLevel(save_) - 1, true);
  save_.readLevel = readLevelNow();
  asked_ = right_ = roundStars_ = 0; readLevelWas_ = pet::readLevel(save_); pay_ = skills::start(*book_, now_);
  nextRead();
}
// One of the level's passages not had since Pets Club opened; when the level had them all, its cycle starts again.
void Game::nextRead() {
  const uint8_t level = pet::readLevel(save_);
  int pick = -1, seen = 0;
  for (int pass = 0; pass < 2 && pick < 0; pass++) {
    if (pass) readsSeen_ = 0;
    for (int i = 0; i < READ_COUNT; i++) {
      if (READS[i].level != level || (readsSeen_ >> i) & 1) continue;
      if (puzzles::below(rng_, ++seen) == 0) pick = i;   // one at random, in one pass
    }
  }
  little_ = (uint8_t)(pick < 0 ? 0 : pick);
  readsSeen_ |= 1ull << little_;
  answered_ = -1; page_ = 0;
  char buf[900]; fillNames(READS[little_].text, buf, sizeof buf);
  readLineCount_ = wrap(buf, 122, readLines_, learn::READ_LINES);
  char tl[2][40]; pageLines_ = (uint8_t)(wrap(READS[little_].title, 108, tl, 2) == 2 ? PAGE_LINES - 1 : PAGE_LINES);
  go(SC_LITTLE);
}
int Game::readPages() const { return (readLineCount_ + pageLines_ - 1) / pageLines_; }

// ---------------------------------------------------------------- the passage, page by page as a book
void Game::updateLittle() {
  if (backButton()) { go(SC_PLAYMENU); return; }   // the answers given so far are already kept
  if (!in_.tap || in_.y <= 26 || !pressedAfter(0)) return;
  if (in_.x < 64) { if (page_ > 0) page_--; return; }
  if (++page_ >= readPages()) go(SC_LITTLE_ASK);
}
void Game::drawLittle() {
  clear(C_CREAM);
  char tl[2][40]; const int tn = wrap(READS[little_].title, 108, tl, 2);
  const int top = tn == 2 ? 46 : 38;
  for (int y = top + 2; y < 160; y += 10) hline(24, y, 112, C_PEACH);   // ruled paper
  for (int i = 0; i < tn; i++) textCentered(80, (tn == 2 ? 16 : 24) + i * 9, tl[i], C_PLUM);
  hline(30, top - 5, 100, C_PLUM);
  const int from = page_ * pageLines_;
  for (int i = 0; i < pageLines_ && from + i < readLineCount_; i++) text(19, top + i * 10, readLines_[from + i], C_DKBROWN);
  const int np = readPages();
  for (int i = 0; i < np; i++) circle(80 - np * 3 + i * 6, 130, i == page_ ? 2 : 1, i == page_ ? C_PLUM : C_LTGRAY);
  blitTint(SPR_ARROW, 118, 124, C_ORANGE);
  if (page_ > 0) blitTint(SPR_ARROW, 36, 124, C_ORANGE, true);
  drawDog(58, 152, (ms_ / 700) % 5 == 0 ? P_LISTEN : P_SIT, false);
  drawBackButton();
}

// ---------------------------------------------------------------- the question, the answer, the round's end
int Game::readAnswerTop() const {
  char q[160], l[3][40]; fillNames(READS[little_].question, q, sizeof q);
  return READ_Q_TOP + wrap(q, 116, l, 3) * 9 + 4;
}
void Game::updateLittleAsk() {
  if (asked_ >= READ_ROUND) {
    if (backButton()) { go(SC_PLAYMENU); return; }
    if (ms_ - roundEndMs_ >= SUMMARY_MS && tapBox(in_, MORE_READS_BTN) && pressedAfter(0)) startReads();
    return;
  }
  if (answered_ >= 0) { updateLittleNext(); return; }
  // Back reads the passage again, from its last page, as a book's question does.
  if (backButton()) { page_ = readPages() - 1; go(SC_LITTLE); return; }
  const int ay = readAnswerTop();
  for (int i = 0; i < learn::READ_ANSWERS; i++) {
    if (!in_.tapIn(26, ay + i * READ_ANSWER_STEP, 108, 22) || !pressedAfter(0)) continue;
    answered_ = i; screenMs_ = ms_;
    const bool right = i == READS[little_].right;
    right_ += right;
    pet::readAnswered(save_, now_, right);   // the counters and the bond; the stars are the engine's
    skills::answer(*book_, pay_, skills::READ_EN, (READS[little_].level - 1) * skills::LEVEL, learn::READ_ANSWERS, right,
                   ms_ - shownMs_, READ_ASK_MS, (uint16_t)little_);
    save_.readLevel = readLevelNow();   // the next passage follows the rating
    markDirty();
    if (right) { sound(SND_YIP); spawn(80, ay + i * READ_ANSWER_STEP + 11, 1, 6); }
    return;
  }
}
// Answered: Next goes to the round's next passage, or after the last to the round's end.
void Game::updateLittleNext() {
  if (backButton()) { go(SC_PLAYMENU); return; }
  if (!tapBox(in_, NEXT_BTN) || !pressedAfter(0)) return;
  if (++asked_ < READ_ROUND) { nextRead(); return; }
  pet::learnRoundDone(save_); payRound(); markDirty();
  roundEndMs_ = screenMs_ = ms_; sound(SND_HAPPY); spawn(80, 60, 4, 10);
}
void Game::drawLittleAsk() {
  if (asked_ >= READ_ROUND) { drawLittleDone(); return; }
  const learn::Read& r = READS[little_];
  char s[320], a[96];
  if (answered_ < 0) {   // the question takes the title's place
    clear(C_CREAM);
    fillNames(r.question, s, sizeof s); lines(READ_Q_TOP, s, C_DKBROWN, 3, 116);
    const int ay = readAnswerTop();
    for (int i = 0; i < learn::READ_ANSWERS; i++) {
      const ui::Box box = {26, ay + i * READ_ANSWER_STEP, 108, 22}; const bool pr = ui::pressing(in_, box) && pressedAfter(0);
      ui::drawButton({box, nullptr, nullptr, C_BLUE}, pr);
      fillNames(r.answers[i], a, sizeof a);
      char al[2][40]; const int an = wrap(a, 102, al, 2);
      for (int k = 0; k < an; k++) textCentered(80, box.y + (pr ? 1 : 0) + (an == 2 ? 3 + k * 9 : 7), al[k], ui::inkOn(C_BLUE));
    }
    drawBackButton();
    return;
  }
  const bool right = answered_ == r.right;
  clear(right ? C_MINT : C_CREAM);
  textCentered(80, 32, right ? "Yes!" : "Oops!", right ? C_NAVY : C_PLUM, 2);
  fillNames(r.answers[r.right], a, sizeof a);
  char line[128]; snprintf(line, sizeof line, right ? "%s." : "The answer: %s.", a);
  int y = lines(52, line, C_NAVY, 3);
  const bool star = false;   // stars come at the round's end now (os/skills.h)
  fillNames(right ? (star ? learn::STAR_CHEER : learn::CHEERS[asked_ % puzzles::ROUND]) : r.why, s, sizeof s);
  lines(y + 2, s, C_DKBROWN, 7 - (y - 52) / 9);   // seven lines in all, above Next (host/test_pets_learn.cpp)
  ui::button(in_, {NEXT_BTN, "Next", nullptr, C_GREEN});
  drawBackButton();
}
void Game::drawLittleDone() {
  clear(C_WALL);
  ui::panel({18, 30, 124, 84}, C_CREAM, C_DKBROWN);
  char b[64], s[64];
  snprintf(b, sizeof b, "%d of %d right!", right_, READ_ROUND); textCentered(80, 36, b, C_PLUM);
  if (roundStars_) snprintf(b, sizeof b, "+%d star%s!", roundStars_, roundStars_ > 1 ? "s" : "");
  else snprintf(b, sizeof b, "Almost a star!");
  int y = lines(50, b, C_NAVY, 2, 112);
  fillNames(pet::readLevel(save_) > readLevelWas_ ? "Next time: longer reads!" : "{pet} loves reading with you.", s, sizeof s);
  lines(y + 2, s, C_DKBROWN, 2, 112);
  if (roundStars_) blit(SPR_STAR, 112, 50);
  ui::button(in_, {MORE_READS_BTN, "More reads", nullptr, C_GREEN});
  drawBackButton();
}
