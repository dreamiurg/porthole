// Little reads, from the Library's first row: a round of READ_ROUND short passages, each read in the story reader
// and followed by one question with three answers. The passage comes from the kid's level (pet.h's answerRead moves
// it after every answer), so the next one is already pitched by the last. A right answer is kept (every
// READS_PER_STAR are a star); a miss shows the right one and the line that holds it, and moves on. No timer, no score.
#include <stdio.h>
#include "content_puzzles.h"
#include "content_reads.h"
#include "game.h"
#include "gfx565.h"
#include "personalize.h"
#include "ui565.h"

namespace biscuit {
using namespace ui565;
namespace {
constexpr int READ_ROUND = 3;
constexpr uint32_t SUMMARY_MS = 1000;   // a slow second tap on the last Next must not skip the round's end
static_assert(READ_COUNT <= 64, "readsSeen_ holds a bit per read");
constexpr uint16_t FILLS[READ_ANSWERS] = {SAGE, PEACH, PURPLE};
constexpr Button NEXT_READ = {WIDE, "Next", &FONT20, SAGE, true}, MORE_READS = {WIDE, "More reads", &FONT20, SAGE, true};
Nav reader(int at, int total) { return {"Previous", at + 1 >= total ? "Question" : "Next", at > 0, true}; }
}  // namespace

void Game::startReads() {
  readsAsked_ = readsRight_ = 0; readsSeen_ = 0;
  readStars_ = earnedStars(save_); readLevelWas_ = readLevel(save_, who_.age);
  nextRead();
}
// One of the level's passages this round hasn't had, any of them when it had them all (a level has eight).
void Game::nextRead() {
  const uint8_t level = readLevel(save_, who_.age);
  int pick = -1, seen = 0;
  for (int pass = 0; pass < 2 && pick < 0; pass++)
    for (int i = 0; i < READ_COUNT; i++) {
      if (READS[i].level != level || (!pass && (readsSeen_ >> i) & 1)) continue;
      if (puzzles::below(rng_, ++seen) == 0) pick = i;   // one at random, in one pass
    }
  little_ = pick < 0 ? 0 : pick;
  readsSeen_ |= 1ull << little_;
  readAnswer_ = -1;
  openPages(&READS[little_].text, 1, 0);
  go(SC_READ);
}

// ---------------------------------------------------------------- the passage
void Game::updateRead() {
  if (tapped(in_, BACK_BUTTON)) { go(SC_LIBRARY); return; }   // the answers given so far are already kept
  const int turn = navTapped(in_, reader(at_, total_));
  if (!turn) return;
  if (turn > 0 && at_ + 1 >= total_) { go(SC_READ_ASK); return; }
  at_ += turn; fresh();
}
void Game::drawRead() {
  gfx565::clear(PAPER);
  top(in_, READS[little_].title, starCount());
  text(PAGE, pageText(), INK);
  nav(in_, reader(at_, total_), at_, total_);
}

// ---------------------------------------------------------------- the question, the answer, the round's end
namespace {
Button answerButton(const Read& r, int i, const Profile& who, const Save& p, char (&label)[96]) {
  personalize(r.answers[i], who.name, p.petName, label, sizeof label);
  return {READ_ANSWER[i], label, &FONT20, FILLS[i], true};
}
}  // namespace
void Game::updateReadAsk() {
  if (readsAsked_ >= READ_ROUND) {
    if (tapped(in_, BACK_BUTTON)) { go(SC_LIBRARY); return; }
    if (ms_ - readEndMs_ >= SUMMARY_MS && tapped(in_, MORE_READS)) startReads();
    return;
  }
  if (readAnswer_ >= 0) {
    if (tapped(in_, BACK_BUTTON)) { go(SC_LIBRARY); return; }
    if (!tapped(in_, NEXT_READ)) return;
    readEndMs_ = ms_;
    if (++readsAsked_ < READ_ROUND) nextRead();
    else fresh();
    return;
  }
  // Back reads the passage again, from its last screen, as the choice's Back does a story's.
  if (tapped(in_, BACK_BUTTON)) { openPages(&READS[little_].text, 1, -1); go(SC_READ); return; }
  for (int i = 0; i < READ_ANSWERS; i++) {
    char label[96];
    if (!tapped(in_, answerButton(READS[little_], i, who_, save_, label))) continue;
    readAnswer_ = i;
    const bool right = i == READS[little_].right;
    readsRight_ += right;
    answerRead(save_, right, who_.age, now_); markDirty();
    fresh();
    return;
  }
}
void Game::drawReadAsk() {
  gfx565::clear(PAPER);
  const Read& r = READS[little_];
  char line[320], s[320], answer[96];
  if (readsAsked_ >= READ_ROUND) {
    top(in_, "All done!", starCount());
    const int got = earnedStars(save_) - readStars_, level = readLevel(save_, who_.age);
    char stars[48];
    if (got > 0) snprintf(stars, sizeof stars, "+%d star%s!", got, got > 1 ? "s" : "");
    else snprintf(stars, sizeof stars, "%d more right for a star.", (int)(READS_PER_STAR - save_.readsRight % READS_PER_STAR));
    snprintf(line, sizeof line, "%d of %d right!\n%s\n%s", readsRight_, READ_ROUND, stars,
             level > readLevelWas_ ? "Next time: longer reads!" : "{pet} loves reading with you.");
    personalize(line, who_.name, save_.petName, s, sizeof s);
    text(READ_AFTER, s, INK);
    button(in_, MORE_READS);
    return;
  }
  if (readAnswer_ < 0) {   // the question takes the title's place
    top(in_, "", starCount());
    personalize(r.question, who_.name, save_.petName, s, sizeof s);
    text(READ_QUESTION, s, INK);
    for (int i = 0; i < READ_ANSWERS; i++) button(in_, answerButton(r, i, who_, save_, answer));
    return;
  }
  top(in_, r.title, starCount());
  const bool right = readAnswer_ == r.right;
  personalize(r.answers[r.right], who_.name, save_.petName, answer, sizeof answer);
  const bool star = right && save_.readsRight % READS_PER_STAR == 0;   // this answer earned a star
  if (right) snprintf(line, sizeof line, "Yes! %s.\n%s", answer, star ? STAR_CHEER : CHEERS[readsAsked_]);
  else snprintf(line, sizeof line, "It's: %s.\n%s", answer, r.why);
  personalize(line, who_.name, save_.petName, s, sizeof s);
  text(READ_AFTER, s, INK);
  button(in_, NEXT_READ);
}
}  // namespace biscuit
