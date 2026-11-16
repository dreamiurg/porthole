// Self-check for Pets Club's puzzles and little reads (games/pets-club/puzzles.h, review.h, content_puzzles.h,
// content_reads.h and pet.h's learning rules): made-up questions at every level, the review queue, the "how" line after
// a miss, the reading level's staircase and the stars, and every line measured in the box Pets Club draws it in
// (learn.cpp). The puzzle cases came from Biscuit with the puzzles (2026-11-16).
// Run: make test
#include "host/check.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "games/pets-club/content_reads.h"
#include "games/pets-club/game.h"
#include "games/pets-club/pet.h"
#include "games/pets-club/review.h"
#include "gfx.h"

constexpr uint32_t Now = 1814400000u;    // local seconds

// A spelling question: the right letter fills its one gap and spells the word.
static void checkSpelling(const learn::puzzles::Puzzle& q) {
  char word[24]; int w = 0;
  for (const char* c = q.ask; *c; c++) if (*c != ' ') word[w++] = *c == '_' ? q.answers[q.right][0] : *c;
  word[w] = 0;
  assert(!strcmp(word, q.solved) && q.clue[0] && strchr(q.ask, '_') == strrchr(q.ask, '_'));
  const size_t n = strlen(word);
  assert(q.item.level == 3 ? n >= 7 : q.item.level == 2 ? n >= 5 && n <= 6 : n == 3u + q.item.level);
  for (int i = 0; i < learn::puzzles::ANSWERS; i++) {   // no wrong letter spells another word
    if (i == q.right) continue;
    char other[24];
    snprintf(other, sizeof other, "%s", q.solved);
    other[q.item.b] = q.answers[i][0];
    assert(!learn::puzzles::detail::listed(other));
  }
}
static bool between(int v, int lo, int hi) { return lo <= v && v <= hi; }
// The kind of numbers each level asks: within ten, across ten, two-digit and one-digit without and with carrying,
// then two two-digit numbers.
static bool plusShape(int level, int a, int b) {
  switch (level) {
    case 0: return a + b <= 10;
    case 2: return a <= 9 && b <= 9 && a + b > 10;
    case 3: return a >= 10 && b <= 9 && a % 10 + b < 10;
    case 4: return a >= 10 && b <= 9 && a % 10 + b >= 10;
    case 5: return a >= 10 && b >= 10;
    default: return false;
  }
}
static bool minusShape(int level, int a, int b) {
  switch (level) {
    case 1: return a <= 10;
    case 2: return a > 10 && a <= 18 && b <= 9 && a - b < 10;
    case 3: return a >= 10 && b <= 9 && a % 10 >= b;
    case 4: return a >= 10 && b <= 9 && a % 10 < b;
    case 5: return a >= 10 && b >= 10;
    default: return false;
  }
}
// A plus or minus question: the right answer is the result, nothing negative, and the level's own kind of numbers.
static void checkNumber(const learn::puzzles::Puzzle& q) {
  int a, b; char op;
  assert(sscanf(q.ask, "%d %c %d", &a, &op, &b) == 3 && !q.clue[0]);
  const int want = op == '+' ? a + b : a - b;
  assert(atoi(q.answers[q.right]) == want && between(want, 0, 99));
  assert(op == '+' ? plusShape(q.item.level, a, b) : minusShape(q.item.level, a, b));
}
// A times story: groups ask the product, sharing the size of a share.
static void checkGroups(const learn::puzzles::Puzzle& q) {
  using namespace learn::puzzles;
  const Item& i = q.item;
  assert(q.clue[0] && strchr(q.clue, '.'));
  const int want = i.c & 16 ? i.b : i.a * i.b;
  assert(atoi(q.answers[q.right]) == want);
  if (i.level == 0) assert(!(i.c & 16) && between(i.a, 2, 3) && between(i.b, 2, 4));
  else if (i.level <= 3) {
    static constexpr int SIZES[3][2] = {{2, 10}, {3, 4}, {6, 9}};
    const int* size = SIZES[i.level - 1];
    assert(!(i.c & 16) && between(i.b, size[0], size[1]) && (i.level > 1 || i.b == 2 || i.b == 5 || i.b == 10));
  } else assert(i.c & 16);
}
// One made-up question: three different answers, none negative, the right one right; the item it came from plays,
// and rebuilds as the same question.
static void checkQuestion(const learn::puzzles::Puzzle& q, int strand, int level) {
  using namespace learn::puzzles;
  assert(q.right < ANSWERS && q.item.strand == strand && q.item.level == level && playable(q.item));
  for (int i = 0; i < ANSWERS; i++) {
    assert(atoi(q.answers[i]) >= (i == q.right || strand == WORDS ? 0 : 1));   // a wrong 0 is too easy
    for (int j = i + 1; j < ANSWERS; j++) assert(strcmp(q.answers[i], q.answers[j]));
  }
  if (strand == WORDS) checkSpelling(q);
  else if (strand == GROUPS) checkGroups(q);
  else checkNumber(q);
  uint32_t other = 99;
  const Puzzle again = build(q.item, other);
  assert(!strcmp(again.ask, q.ask) && !strcmp(again.solved, q.solved) && !strcmp(again.clue, q.clue) && again.key == q.key);
}
// Made-up questions at every level of every strand; a round never asks one twice; a damaged item never plays.
static void puzzleQuestions() {
  using namespace learn::puzzles;
  uint32_t rng = 7;
  for (int strand = 0; strand < STRAND_COUNT; strand++)
    for (uint8_t level = 0; level < LEVELS[strand]; level++)
      for (int n = 0; n < 2000; n++) checkQuestion(make((Strand)strand, level, rng, 0), strand, level);
  uint32_t seen = 0;
  for (int n = 0; n < ROUND; n++) { const Puzzle q = make(GROUPS, 1, rng, seen); assert(!(seen & (1u << q.key))); seen |= 1u << q.key; }
  assert(!playable({STRAND_COUNT, 0, 1, 1, 0}) && !playable({NUMBERS, LEVELS[NUMBERS], 1, 1, 0}));
  assert(!playable({NUMBERS, 1, 3, 5, 1}) && !playable({NUMBERS, 0, 60, 50, 0}) && !playable({NUMBERS, 0, 3, 0, 0}));
  assert(!playable({GROUPS, 0, 1, 5, 0}) && !playable({GROUPS, 0, 3, 5, (uint8_t)learn::GROUP_THING_COUNT}));
  assert(!playable({WORDS, 2, (uint8_t)learn::LONG_WORD_COUNT, 0, 0}) && !playable({WORDS, 0, 0, 9, 0}));
  assert(keyOf({WORDS, 0, 3, 0, 0}) == keyOf({WORDS, 0, 3, 2, 0}) && keyOf({GROUPS, 1, 3, 5, 0}) == keyOf({GROUPS, 1, 3, 5, 2}));
}
// Every word is asked at some level, at a gap whose wrong letters spell no word.
static void puzzleWords() {
  using namespace learn::puzzles;
  for (const learn::SpellWord& w : learn::SHORT_WORDS) {
    bool asked = false;
    for (int gap = 0; w.word[gap]; gap++) asked |= detail::askable(w.word, gap);
    assert(asked);
  }
  for (const learn::SpellWord& w : learn::LONG_WORDS) {
    bool asked = false;
    for (int gap = 0; w.word[gap]; gap++) asked |= detail::askable(w.word, gap);
    assert(asked && (detail::fits(2, w.word) || detail::fits(3, w.word)));
  }
  assert(!detail::askable("bug", 1));   // bag, beg, big, bog: no wrong vowel is left
}
// Levels: the age a kid starts at, and a round's five answers moving it (up on five, down on two or fewer, never off
// the ladder).
static void startLevels() {
  using namespace learn::puzzles;
  assert(startLevel(NUMBERS, 6) == 0 && startLevel(NUMBERS, 0) == 0 && startLevel(NUMBERS, 7) == 2 && startLevel(NUMBERS, 8) == 4);
  assert(startLevel(GROUPS, 7) == 0 && startLevel(GROUPS, 8) == 1 && startLevel(GROUPS, 9) == 2);
  assert(startLevel(WORDS, 6) == 0 && startLevel(WORDS, 8) == 2 && startLevel(WORDS, 9) == 3);
}
static void puzzleLevels() {
  using namespace learn::puzzles;
  startLevels();
  for (int strand = 0; strand < STRAND_COUNT; strand++) {
    const Strand st = (Strand)strand;
    assert(startLevel(st, 10) < LEVELS[st] && startLevel(st, 99) < LEVELS[st]);
    assert(nextLevel(st, 0, 5) == 1 && nextLevel(st, 1, 4) == 1 && nextLevel(st, 1, 3) == 1 && nextLevel(st, 1, 2) == 0);
    assert(nextLevel(st, 0, 0) == 0 && nextLevel(st, LEVELS[st] - 1, 5) == LEVELS[st] - 1);
  }
  // one question of a round comes from the level below; at the bottom, all five are at the level
  for (int n = 0; n < ROUND; n++) { assert(roundLevel(3, n, 2) == (n == 2 ? 2 : 3)); assert(roundLevel(0, n, 2) == 0); }
}
// The review queue: a miss is due tomorrow, asked in its strand from then on, off the queue when answered right
// while due, a day later when missed again; a full queue drops its oldest; a damaged count or item never plays.
static void puzzleReviews() {
  using namespace learn::puzzles;
  namespace rv = learn::review;
  Save p; pet::adopt(p, Now, "Sam", "Rex");
  const int32_t today = 20000;
  const Item sum = {NUMBERS, 2, 8, 5, 0}, story = {GROUPS, 1, 3, 4, 0};
  rv::answered(p, sum, true, today);
  assert(p.puzzleReviewCount == 0);
  rv::answered(p, sum, false, today);
  rv::answered(p, story, false, today);
  assert(p.puzzleReviewCount == 2 && p.puzzleReviews[0].due == today + 1 && p.puzzleReviews[0].misses == 1);
  assert(rv::due(p, NUMBERS, today) < 0 && rv::due(p, NUMBERS, today + 1) == 0 && rv::due(p, GROUPS, today + 5) == 1);
  rv::answered(p, sum, true, today);   // not due yet: right today keeps it for tomorrow
  assert(p.puzzleReviewCount == 2);
  rv::answered(p, sum, false, today + 1);
  assert(p.puzzleReviewCount == 2 && p.puzzleReviews[0].misses == 2 && p.puzzleReviews[0].due == today + 2);
  rv::answered(p, sum, true, today + 3);
  assert(p.puzzleReviewCount == 1 && rv::same(p.puzzleReviews[0], story) && !p.puzzleReviews[1].due);
  for (uint8_t a = 1; a <= PUZZLE_REVIEWS; a++) rv::answered(p, {NUMBERS, 0, a, 1, 0}, false, today);
  assert(p.puzzleReviewCount == PUZZLE_REVIEWS && rv::find(p, story) < 0 && p.puzzleReviews[0].a == 1);
  p.puzzleReviewCount = 200;   // damaged: read as full, never past the queue
  assert(rv::count(p) == PUZZLE_REVIEWS && rv::due(p, NUMBERS, today + 1) == 0);
  p.puzzleReviews[0].a = 200;   // an item that no longer plays is skipped
  assert(rv::due(p, NUMBERS, today + 1) == 1 && rv::due(p, WORDS, today + 1) < 0);
}
// After a miss, one line on how to solve it, for every kind of question; none for words.
static void puzzleHow() {
  using namespace learn::puzzles;
  struct Case { Item q; const char* want; };
  static const Case CASES[] = {
    {{NUMBERS, 0, 3, 5, 0}, "Start at 5, count on 3."}, {{NUMBERS, 2, 8, 5, 0}, "8 + 2 = 10, then 3 more."},
    {{NUMBERS, 3, 43, 5, 0}, "3 + 5 = 8, plus the 40 is 48."}, {{NUMBERS, 4, 41, 9, 0}, "1 + 9 = 10, plus the 40 is 50."},
    {{NUMBERS, 5, 36, 47, 0}, "36 + 40 = 76, then 7 more."}, {{NUMBERS, 1, 9, 3, 1}, "Count back 3 from 9."},
    {{NUMBERS, 2, 13, 5, 1}, "13 - 3 = 10, then 2 less."}, {{NUMBERS, 3, 47, 3, 1}, "7 - 3 = 4, plus the 40 is 44."},
    {{NUMBERS, 4, 30, 7, 1}, "Count back 7 from 30."}, {{NUMBERS, 5, 72, 30, 1}, "Count back 3 tens from 72."}, {{NUMBERS, 5, 50, 28, 1}, "50 - 20 = 30, then 8 less."},
    {{NUMBERS, 5, 10, 50, 0}, "Count on 5 tens from 10."}, {{NUMBERS, 4, 47, 5, 0}, "47 + 3 = 50, then 2 more."},
    {{NUMBERS, 4, 40, 3, 0}, "40 and 3 make 43."}, {{NUMBERS, 0, 0, 10, 0}, "10 + 0 is still 10."},
    {{NUMBERS, 1, 3, 3, 1}, "Take all 3 away, and 0 is left."},
    {{GROUPS, 1, 3, 4, 0}, "4 + 4 + 4 = 12"}, {{GROUPS, 4, 3, 4, 16}, "3 x 4 = 12, so 4 each."},
    {{WORDS, 0, 0, 1, 0}, ""},
  };
  for (const Case& c : CASES) { char out[48]; how(c.q, out); if (strcmp(out, c.want)) printf("how: got \"%s\" want \"%s\"\n", out, c.want); assert(!strcmp(out, c.want)); }
  uint32_t rng = 3;   // every question at every level has a line that fits its buffer
  for (int strand = 0; strand < WORDS; strand++)
    for (uint8_t level = 0; level < LEVELS[strand]; level++)
      for (int n = 0; n < 500; n++) {
        char out[48];
        const Item q = pick((Strand)strand, level, rng);
        how(q, out);
        assert(out[0] && strlen(out) < sizeof out - 1);
        char sum[16];   // never just the sum again
        snprintf(sum, sizeof sum, "= %d.", q.c ? q.a - q.b : q.a + q.b);
        assert(strand != NUMBERS || !strstr(out, sum));
      }
}

// Stars: one per PUZZLES_PER_STAR right puzzles and per READS_PER_STAR right reads; a miss earns nothing and costs
// nothing; right answers bring the dog closer (learningRules). Reads move the level up after three right in a row and down after a
// miss, never more than one below where the kid's age starts it.
static void learningRules() {
  Save p; pet::adopt(p, Now, "Sam", "Rex"); p.kidAge = 8;
  int stars = 0;
  for (int i = 0; i < 12; i++) stars += pet::puzzleAnswered(p, Now, i % 6 != 5);
  assert(p.puzzlesRight == 10 && stars == 2 && p.bond == 10 * pet::LEARN_BOND);
}
static void readLevels() {
  Save p; pet::adopt(p, Now, "Sam", "Rex"); p.kidAge = 8; p.puzzlesRight = 10;
  assert(pet::ageReadLevel(0) == 1 && pet::ageReadLevel(6) == 1 && pet::ageReadLevel(8) == 3 && pet::ageReadLevel(12) == pet::READ_LEVELS);
  assert(pet::readLevel(p) == 3 && !pet::readAnswered(p, Now, true) && !pet::readAnswered(p, Now, true));
  assert(pet::readAnswered(p, Now, true) && p.readLevel == 4 && p.readStreak == 0 && p.readsRight == 3);
  assert(!pet::readAnswered(p, Now, false) && p.readLevel == 3);
  assert(!pet::readAnswered(p, Now, false) && p.readLevel == 2);   // one below the age's start
  assert(!pet::readAnswered(p, Now, false) && p.readLevel == 2);   // and no further
  p.readLevel = pet::READ_LEVELS; p.readStreak = 2;
  assert(!pet::readAnswered(p, Now, true) && p.readLevel == pet::READ_LEVELS);   // the top stays the top
  pet::adoptFromShelter(p, Now, 2, "Choco");   // a new pup: the kid's levels and counts stay
  assert(p.readsRight == 4 && p.puzzlesRight == 10 && p.readLevel == pet::READ_LEVELS);
}

// Every line Pets Club draws, at its widest: the longest names fill {pet} and {name}.
static void fill(const char* src, char* out, size_t cap) {
  size_t at = 0;
  while (*src && at + 1 < cap) {
    const char* put = !strncmp(src, "{pet}", 5) ? "Wolfgang" : !strncmp(src, "{name}", 6) ? "Maximili" : nullptr;
    if (!put) { out[at++] = *src++; continue; }
    src += !strncmp(src, "{pet}", 5) ? 5 : 6;
    for (; *put && at + 1 < cap; put++) out[at++] = *put;
  }
  out[at] = 0;
}
static int lineCount(const char* s, int width, int most) {   // wrapped lines, most + 1 when it would not fit in most
  char l[60][40]; char f[900]; fill(s, f, sizeof f);
  return gfx::wrap(f, width, l, most + 1);
}
static bool noLongWord(const char* s, int width) {   // wrap never cuts a word: one wider than the line overflows it
  char f[900]; fill(s, f, sizeof f);
  for (char* w = strtok(f, " \n"); w; w = strtok(nullptr, " \n")) if (gfx::textWidth(w) > width) return false;
  return true;
}
static void readsFit() {
  for (const learn::Read& r : learn::READS) {
    char title[2][40];
    const int tn = gfx::wrap(r.title, 108, title, 3);
    assert(tn <= 2);
    assert(lineCount(r.text, 122, learn::READ_LINES) <= learn::READ_LINES && noLongWord(r.text, 122));
    assert(lineCount(r.question, 116, 3) <= 3);
    for (const char* a : r.answers) assert(lineCount(a, 102, 2) <= 2);
    char ans[128], line[160]; fill(r.answers[r.right], ans, sizeof ans);
    snprintf(line, sizeof line, "The answer: %s.", ans);
    const int an = lineCount(line, 124, 3);
    assert(an <= 3 && lineCount(r.why, 124, 7 - an) <= 7 - an);   // learn.cpp: the why fills what the answer leaves
  }
}
static void puzzleLinesFit() {
  for (const char* c : learn::CHEERS) assert(lineCount(c, 124, 2) <= 2);
  for (const char* c : learn::NUDGES) assert(lineCount(c, 124, 2) <= 2);
  assert(lineCount(learn::STAR_CHEER, 124, 2) <= 2 && lineCount(learn::LEVEL_UP, 112, 2) <= 2);
  for (const char* s : learn::STRAND_LABELS) assert(gfx::textWidth(s) <= 100);
  for (const learn::SpellWord& w : learn::SHORT_WORDS) assert(lineCount(w.clue, 124, 2) <= 2);
  for (const learn::SpellWord& w : learn::LONG_WORDS) assert(lineCount(w.clue, 124, 2) <= 2);
  uint32_t rng = 11;   // every question: its clue in two lines, its ask on one, its answers on their buttons
  for (int strand = 0; strand < learn::puzzles::STRAND_COUNT; strand++)
    for (uint8_t level = 0; level < learn::puzzles::LEVELS[strand]; level++)
      for (int n = 0; n < 300; n++) {
        const learn::puzzles::Puzzle q = learn::puzzles::make((learn::puzzles::Strand)strand, level, rng, 0);
        char how[48]; learn::puzzles::how(q.item, how);
        assert(lineCount(q.clue, 124, 2) <= 2 && gfx::textWidth(q.ask) <= 124 && lineCount(how, 124, 2) <= 2);
        char solved[40]; snprintf(solved, sizeof solved, "Yes! %s", q.solved); assert(gfx::textWidth(solved) <= 124);
        for (const char* a : q.answers) assert(gfx::textWidth(a) <= 30);
      }
}

int main() {
  puzzleQuestions();
  puzzleLevels();
  puzzleWords();
  puzzleReviews();
  puzzleHow();
  learningRules();
  readLevels();
  readsFit();
  puzzleLinesFit();
  printf("test_pets_learn: all checks passed\n");
  return 0;
}
