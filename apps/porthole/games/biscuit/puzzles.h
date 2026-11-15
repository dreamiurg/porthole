// Biscuit's puzzles: one question with three answers to pick from, made up on the spot. Pure logic, no rendering;
// host/test_biscuit.cpp exercises it. Three strands, each a ladder of levels tied to a grade standard
// (docs/design/adaptive-practice.md): plus and minus from sums within ten (1.OA.6) to two-digit numbers (2.NBT.5),
// times as groups and sharing in a story (3.OA.3), and words with a missing letter from three letters to eight. The
// kid picks the strand; the level is the kid's own and moves after every round (nextLevel).
#pragma once
#include <stdint.h>
#include <stdio.h>
#include <string.h>
#include "content_puzzles.h"

namespace biscuit::puzzles {
enum Strand : uint8_t { NUMBERS, GROUPS, WORDS, STRAND_COUNT };
constexpr uint8_t LEVELS[STRAND_COUNT] = {6, 6, 4};
constexpr int ANSWERS = 3, ROUND = 5;

// One question, exactly: rebuilt the same from these five bytes (the review queue keeps them).
// NUMBERS: a op b, c 0 = plus, 1 = minus. GROUPS: a groups (or pups) of b, c's bit 4 = sharing, its low bits the
// story's things (GROUP_THINGS). WORDS: word a of the level's list, b the missing letter.
struct Item { uint8_t strand, level, a, b, c; };

struct Puzzle {
  Item item;
  char clue[48];           // a word's clue or a times story, else empty
  char ask[24];            // "8 + 5 = ?", "f r _ e n d" or "How many?"
  char solved[24];         // "8 + 5 = 13", "friend" or "3 x 4 = 12"
  char answers[ANSWERS][12];
  uint8_t right;           // the index of the right answer
  uint8_t key;             // which question this was (0..31, a bit in a round's seen mask), so it isn't asked twice
};

inline uint32_t next(uint32_t& s) {   // xorshift32; s never 0
  if (!s) s = 0x9e3779b9u;
  s ^= s << 13; s ^= s >> 17; s ^= s << 5;
  return s;
}
inline int below(uint32_t& s, int n) { return (int)(next(s) % (uint32_t)n); }
inline int between(uint32_t& s, int lo, int hi) { return lo + below(s, hi - lo + 1); }

// Where a kid starts a strand they have never played, by age (0 = not given: six).
inline uint8_t startLevel(Strand strand, int age) {
  if (age <= 0) age = 6;
  switch (strand) {
    case NUMBERS: return age <= 6 ? 0 : age == 7 ? 2 : 4;
    case GROUPS: return age <= 7 ? 0 : age == 8 ? 1 : 2;
    default: return age <= 6 ? 0 : age == 7 ? 1 : age == 8 ? 2 : 3;
  }
}
// A round of five moves the level: all right, one up; two or fewer, one down. That keeps a kid near 70-90% right.
inline uint8_t nextLevel(Strand strand, uint8_t level, int right) {
  if (right >= ROUND && level + 1 < LEVELS[strand]) return (uint8_t)(level + 1);
  if (right <= 2 && level > 0) return (uint8_t)(level - 1);
  return level;
}

namespace detail {
constexpr const char* VOWELS = "aeiou";
inline bool vowel(char c) { return strchr(VOWELS, c) != nullptr; }
// "f r _ e n d": the word's letters spaced out, '_' at gap. out holds a word of up to 11 letters (check_content.py).
inline void spaced(const char* word, int gap, char (&out)[24]) {
  int at = 0;
  for (int i = 0; word[i] && at + 3 < (int)sizeof out; i++) {
    if (i) out[at++] = ' ';
    out[at++] = i == gap ? '_' : word[i];
  }
  out[at] = 0;
}
// Answers: the right one at a random place, two other different ones in the rest.
template <class Pick> inline void fill(Puzzle& p, uint32_t& s, const char* answer, Pick wrong) {
  p.right = (uint8_t)below(s, ANSWERS);
  for (int i = 0; i < ANSWERS; i++) {
    if (i == p.right) { snprintf(p.answers[i], sizeof p.answers[i], "%s", answer); continue; }
    bool taken;
    do {
      wrong(p.answers[i]);
      taken = false;
      for (int j = 0; j < i; j++) taken |= !strcmp(p.answers[j], p.answers[i]);
      taken |= !strcmp(p.answers[i], answer);
    } while (taken);
  }
}
// A number answer; the wrong ones are a step or two either way, or one off, never below one (a wrong 0 is too easy
// to rule out).
inline void numberAnswers(Puzzle& p, uint32_t& s, int result, int step) {
  char right[12];
  snprintf(right, sizeof right, "%d", result);
  fill(p, s, right, [&](char (&out)[12]) {
    static constexpr int OFFSETS[] = {-2, -1, 1, 2};
    int v = below(s, 3) ? result + step * OFFSETS[below(s, 4)] : result + (below(s, 2) ? 1 : -1);
    if (v < 1) v = result + step * 3 + below(s, 3);
    snprintf(out, sizeof out, "%d", v > 999 ? 999 : v);
  });
}
// The words of a WORDS level: three letters, four, five or six, then seven and more.
inline const SpellWord* words(uint8_t level, int& count) {
  if (level >= 2) { count = LONG_WORD_COUNT; return LONG_WORDS; }
  count = SHORT_WORD_COUNT;
  return SHORT_WORDS;
}
inline bool fits(uint8_t level, const char* word) {
  const size_t n = strlen(word);
  return level == 3 ? n >= 7 : level == 2 ? n >= 5 && n <= 6 : n == 3u + level;
}
inline bool listed(const char* w) {
  for (const SpellWord& x : SHORT_WORDS) if (!strcmp(x.word, w)) return true;
  for (const SpellWord& x : LONG_WORDS) if (!strcmp(x.word, w)) return true;
  for (const char* x : REAL_WORDS) if (!strcmp(x, w)) return true;
  return false;
}
// The wrong letters for a word's gap: a vowel for a vowel, a consonant for a consonant, none that spells a word.
inline int wrongLetters(const char* word, int gap, char (&out)[16]) {
  static constexpr const char* CONSONANTS = "bcdfghklmnprstw";
  const char* from = vowel(word[gap]) ? VOWELS : CONSONANTS;
  char w[12];
  snprintf(w, sizeof w, "%s", word);
  int n = 0;
  for (const char* c = from; *c; c++) {
    w[gap] = *c;
    if (*c != word[gap] && !listed(w)) out[n++] = *c;
  }
  return n;
}
inline bool askable(const char* word, int gap) {
  char letters[16];
  return wrongLetters(word, gap, letters) >= 2;
}
}  // namespace detail

namespace detail {
// Plus and minus at each level: within ten, across ten, two-digit and one-digit without and with carrying, then two
// two-digit numbers. t and o are a random two-digit number's tens (1..8) and ones (0..8).
struct Pair { int a, b; };
inline Pair plus(uint8_t level, uint32_t& s) {
  const int t = 1 + below(s, 8), o = below(s, 9);
  int a, b;
  switch (level) {
    case 0: a = below(s, 10); b = between(s, 1, 10 - a); break;    // 4 + 5
    case 2: a = between(s, 2, 9); b = between(s, 11 - a, 9); break;  // 8 + 5
    case 3: a = 10 * t + o; b = between(s, 1, 9 - o); break;         // 43 + 5
    case 4: a = 10 * t + 1 + o; b = between(s, 9 - o, 9); break;     // 47 + 8
    default: a = between(s, 10, 89); b = between(s, 10, 99 - a); break;   // 36 + 47
  }
  return {a, b};
}
inline Pair minus(uint8_t level, uint32_t& s) {
  const int t = 1 + below(s, 8), o = below(s, 9);
  int a, b;
  switch (level) {
    case 1: a = between(s, 2, 10); b = between(s, 1, a); break;                    // 9 - 3
    case 2: a = between(s, 11, 18); b = between(s, a - 9 > 2 ? a - 9 : 2, 9); break;   // 13 - 5
    case 3: a = 10 * t + 1 + o; b = between(s, 1, 1 + o); break;                   // 47 - 3
    case 4: a = 10 * (t + 1) + o; b = between(s, o + 1, 9); break;                 // 52 - 7
    default: a = between(s, 20, 99); b = between(s, 10, a - 10); break;            // 72 - 38
  }
  return {a, b};
}
inline Item number(uint8_t level, uint32_t& s) {
  const int c = level == 0 ? 0 : level == 1 ? 1 : below(s, 2);   // plus and minus mixed from crossing ten on
  const Pair q = c ? minus(level, s) : plus(level, s);
  return {NUMBERS, level, (uint8_t)q.a, (uint8_t)q.b, (uint8_t)c};
}
// Groups small enough to count (2 or 3 of 2 to 4), of 2, 5 and 10, of 3 and 4, of 6 to 9; then sharing.
inline Item groups(uint8_t level, uint32_t& s) {
  static constexpr uint8_t SIZES[3][4] = {{2, 5, 10, 2}, {3, 4, 3, 4}, {6, 7, 8, 9}};
  const int most = level == 4 ? 5 : 9, thing = below(s, GROUP_THING_COUNT);
  if (level >= 4) return {GROUPS, level, (uint8_t)between(s, 2, most), (uint8_t)between(s, 2, most), (uint8_t)(16 | thing)};
  if (level == 0) return {GROUPS, 0, (uint8_t)between(s, 2, 3), (uint8_t)between(s, 2, 4), (uint8_t)thing};
  return {GROUPS, level, (uint8_t)between(s, 2, 5), SIZES[level - 1][below(s, 4)], (uint8_t)thing};
}
inline Item word(uint8_t level, uint32_t& s) {
  int count, a;
  const SpellWord* list = words(level, count);
  int gap;
  do {   // every word has a gap that can be asked (host test), so this ends
    a = below(s, count);
    gap = below(s, (int)strlen(list[a].word));
  } while (!fits(level, list[a].word) || !askable(list[a].word, gap));
  return {WORDS, level, (uint8_t)a, (uint8_t)gap, 0};
}
}  // namespace detail

namespace detail {
inline bool playableWord(const Item& q) {
  int count;
  const SpellWord* list = words(q.level, count);
  return !q.c && q.a < count && fits(q.level, list[q.a].word) && q.b < strlen(list[q.a].word) &&
         askable(list[q.a].word, q.b);
}
}  // namespace detail

// A new question at this level (any strand, any level below LEVELS), chosen at random.
inline Item pick(Strand strand, uint8_t level, uint32_t& s) {
  return strand == NUMBERS ? detail::number(level, s) : strand == GROUPS ? detail::groups(level, s) : detail::word(level, s);
}
// Which question an item is, for not asking it twice in a round: a word whatever its gap, a story whatever its things.
inline uint8_t keyOf(const Item& q) {
  if (q.strand == WORDS) return (uint8_t)(q.a % 32);
  const int c = q.strand == GROUPS ? q.c >> 4 : q.c;
  return (uint8_t)((q.a * 11 + q.b + c * 7) % 32);
}

// A question that will play: its strand and level exist and its numbers or word make sense. The review queue keeps
// only items this passes, so a damaged save never shows a broken question.
inline bool playable(const Item& q) {
  if (q.strand >= STRAND_COUNT || q.level >= LEVELS[q.strand]) return false;
  if (q.strand == NUMBERS) return q.c <= 1 && q.b >= 1 && (q.c ? q.b <= q.a && q.a <= 99 : q.a + q.b <= 99);
  if (q.strand == GROUPS) return q.a >= 2 && q.a <= 9 && q.b >= 2 && q.b <= 10 && (q.c & ~16) < GROUP_THING_COUNT;
  return detail::playableWord(q);
}

// The question an item asks, with its three answers in a random order.
inline Puzzle build(const Item& q, uint32_t& s) {
  Puzzle p{};
  p.item = q;
  p.key = keyOf(q);
  switch (q.strand) {
    case NUMBERS: {
      const char op = q.c ? '-' : '+';
      const int result = q.c ? q.a - q.b : q.a + q.b;
      snprintf(p.ask, sizeof p.ask, "%d %c %d = ?", q.a, op, q.b);
      snprintf(p.solved, sizeof p.solved, "%d %c %d = %d", q.a, op, q.b, result);
      detail::numberAnswers(p, s, result, q.level == 5 ? 10 : 1);
      break;
    }
    case GROUPS: {
      const GroupThing& t = GROUP_THINGS[q.c & 15];
      if (q.c & 16) {   // sharing: a pups share a x b things
        snprintf(p.clue, sizeof p.clue, "%d %s shared by %d pups.", q.a * q.b, t.things, q.a);
        snprintf(p.ask, sizeof p.ask, "%s", SHARE_ASK);
        snprintf(p.solved, sizeof p.solved, "%d each", q.b);
        detail::numberAnswers(p, s, q.b, 1);
      } else {
        snprintf(p.clue, sizeof p.clue, "%d %s, %d %s in each.", q.a, t.groups, q.b, t.things);
        snprintf(p.ask, sizeof p.ask, "%s", GROUP_ASK);
        snprintf(p.solved, sizeof p.solved, "%d x %d = %d", q.a, q.b, q.a * q.b);
        detail::numberAnswers(p, s, q.a * q.b, q.b);
      }
      break;
    }
    default: {
      int count;
      const SpellWord& w = detail::words(q.level, count)[q.a];
      const char missing = w.word[q.b];
      snprintf(p.clue, sizeof p.clue, "%s", w.clue);
      detail::spaced(w.word, q.b, p.ask);
      snprintf(p.solved, sizeof p.solved, "%s", w.word);
      const char right[2] = {missing, 0};
      char letters[16];
      const int n = detail::wrongLetters(w.word, q.b, letters);
      detail::fill(p, s, right, [&](char (&out)[12]) { out[0] = letters[below(s, n)]; out[1] = 0; });
      break;
    }
  }
  return p;
}

namespace detail {
// "a op first = mid, then rest more." (or less), the two steps of adding or taking away through a ten.
inline void twoSteps(char (&out)[48], int a, char op, int first, int rest) {
  const int mid = op == '+' ? a + first : a - first;
  if (rest) snprintf(out, sizeof out, "%d %c %d = %d, then %d %s.", a, op, first, mid, rest, op == '+' ? "more" : "less");
  else snprintf(out, sizeof out, "%d %c %d = %d.", a, op, first, mid);
}
inline void howPlus(int a, int b, char (&out)[48]) {
  if (a + b <= 10) snprintf(out, sizeof out, "Start at %d, count on %d.", a > b ? a : b, a > b ? b : a);
  else if (b >= 10) twoSteps(out, a, '+', b / 10 * 10, b % 10);
  else if (a % 10 + b >= 10) twoSteps(out, a, '+', 10 - a % 10, b - (10 - a % 10));
  else snprintf(out, sizeof out, "%d + %d = %d, so %d.", a % 10, b, a % 10 + b, a + b);
}
inline void howMinus(int a, int b, char (&out)[48]) {
  if (a <= 10 || a % 10 == 0) snprintf(out, sizeof out, "Count back %d from %d.", b, a);
  else if (b >= 10) twoSteps(out, a, '-', b / 10 * 10, b % 10);
  else if (a % 10 < b) twoSteps(out, a, '-', a % 10, b - a % 10);
  else snprintf(out, sizeof out, "%d - %d = %d, so %d.", a % 10, b, a % 10 - b, a - b);
}
inline void howGroups(const Item& q, char (&out)[48]) {
  if (q.c & 16) { snprintf(out, sizeof out, "%d x %d = %d, so %d each.", q.a, q.b, q.a * q.b, q.b); return; }
  int at = 0;
  for (int i = 0; i < q.a; i++) at += snprintf(out + at, sizeof out - at, i ? " + %d" : "%d", q.b);
  snprintf(out + at, sizeof out - at, " = %d", q.a * q.b);
}
}  // namespace detail

// One line on how to solve a question, shown after a miss (principle 9): through ten, by tens, groups as repeated
// adding. Empty for words, where the right word itself is the help.
inline void how(const Item& q, char (&out)[48]) {
  out[0] = 0;
  if (q.strand == NUMBERS) q.c ? detail::howMinus(q.a, q.b, out) : detail::howPlus(q.a, q.b, out);
  else if (q.strand == GROUPS) detail::howGroups(q, out);
}

// A new question at this level, not one whose key is in seen.
inline Puzzle make(Strand strand, uint8_t level, uint32_t& s, uint32_t seen) {
  Item q{};
  for (int tries = 0; tries < 16; tries++) {
    q = pick(strand, level, s);
    if (!(seen & (1u << keyOf(q)))) break;
  }
  return build(q, s);
}
// The level a round's n-th question is at: one of the five, at a place picked when the round starts (easyAt), comes
// from the level below, so every round mixes a little of what the kid already knows back in.
inline uint8_t roundLevel(uint8_t level, int n, int easyAt) { return n == easyAt && level ? (uint8_t)(level - 1) : level; }
}  // namespace biscuit::puzzles
