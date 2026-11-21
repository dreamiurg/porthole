// Pets Club's Ukrainian words (the design page "Українська в Porthole", approved 2026-11-21): questions picked for the
// kid's rating in the shared skills engine (os/skills.h, skill WORDS_UK), which also calibrates a new kid and pays the
// stars. Pure logic, no rendering; host/test_pets_uk.cpp exercises it. Three exercises:
//   MEANING    a Ukrainian word, three English meanings to pick from;
//   LOOKALIKE  an English meaning, three Ukrainian words that differ by a letter or two (кіт / кит / кут);
//   SYLLABLES  an English meaning, the word's syllables and one more to tap in order (со-ба-ка).
// A word's step s (1..STEPS, tools/uk_words.py) is engine level s - 1; the exercise adds KIND_OFFSET.
#pragma once
#include <stdint.h>
#include <stdlib.h>
#include <string.h>
#include "content_uk.h"
#include "pet.h"
#include "puzzles.h"
#include "review.h"
#include "skills.h"

namespace learn::uk {
enum Kind : uint8_t { MEANING, LOOKALIKE, SYLLABLES, KIND_COUNT };
constexpr int ROUND = 5, OPTIONS = 3, MAX_TILES = 4, STEPS = 7;
constexpr int KIND_OFFSET[KIND_COUNT] = {0, 100, 50};   // the same word, harder to find among lookalikes or to build
// Faster than this is a guess, not a read (the engine counts it wrong); slower is right but not yet fluent.
constexpr uint32_t READ_MS[KIND_COUNT] = {700, 900, 1500};
constexpr uint32_t SLOW_MS[KIND_COUNT] = {10000, 10000, 15000};

inline int beta(const Word& w, Kind k) { return skills::LEVEL * (w.step - 1) + KIND_OFFSET[k]; }
// The answers a guess picks from: three options; building in order is no guess.
constexpr int choices(Kind k) { return k == SYLLABLES ? 1 : OPTIONS; }
inline bool fits(const Word& w, Kind k) { return k == MEANING || (k == LOOKALIKE ? w.look != 0 : w.tiles != 0); }
inline int parts(const Word& w) { int n = 1; for (const char* c = w.syl; *c; c++) n += *c == '-'; return n; }
// Syllable `part` of w into out (NUL-terminated).
inline void syllable(const Word& w, int part, char* out, size_t cap) {
  const char* s = w.syl;
  for (int i = 0; i < part && s; i++) { s = strchr(s, '-'); if (s) s++; }
  size_t n = 0;
  for (; s && *s && *s != '-' && n + 1 < cap; s++) out[n++] = *s;
  out[n] = 0;
}

// The difficulty to serve for exercise k: the engine's level for this kid, as a beta.
inline int target(const skills::Book& b, Kind k) { return skills::LEVEL * skills::target(b, skills::WORDS_UK, STEPS + 1, choices(k)); }
// An answer, for skills::answer: the item (its key tells a repeat today) and how fast it came.
inline skills::Item item(Kind k, int word, uint32_t ms) {
  return {skills::WORDS_UK, beta(WORDS[word], k), choices(k), READ_MS[k], (uint16_t)(k << 12 | word),
          ms > SLOW_MS[k] ? skills::SLOW : skills::QUICK};
}

// ---------------------------------------------------------------- the review queue: a missed word comes back tomorrow
// The puzzles' own queue (review.h, pet.h puzzleReviews: eight places, oldest first) with a strand past the puzzles'
// (which never ask it): the word's index in a and b. A miss is due the next day; a right answer when due takes it off.
constexpr uint8_t REVIEW_STRAND = puzzles::STRAND_COUNT;
inline puzzles::Item reviewItem(uint16_t word) { return {REVIEW_STRAND, 0, (uint8_t)(word & 0xFF), (uint8_t)(word >> 8), 0}; }
inline void answered(Save& s, uint16_t word, bool right, uint16_t today) { review::answered(s, reviewItem(word), right, today); }
inline int due(const Save& s, uint16_t today) {   // the oldest word due today, or -1 (a word past the list is skipped)
  for (int i = 0; i < review::count(s); i++) {
    const PuzzleReview& r = s.puzzleReviews[i];
    const int word = r.a | r.b << 8;
    if (r.strand == REVIEW_STRAND && r.due && r.due <= today && word < WORD_COUNT) return word;
  }
  return -1;
}

// ---------------------------------------------------------------- questions
struct Tile { uint16_t word; uint8_t part; };
struct Question {
  Kind kind;
  uint16_t word;
  uint16_t options[OPTIONS];   // MEANING and LOOKALIKE: the words whose English or Ukrainian is shown
  uint8_t right;               // MEANING and LOOKALIKE: options[right] == word
  uint8_t tileCount;           // SYLLABLES: the word's syllables and one decoy, shuffled
  Tile tiles[MAX_TILES];
};

// The round's i-th exercise: the three take turns, from a place the round's seed picks.
inline Kind kindAt(int i, uint32_t roundSeed) { return (Kind)((roundSeed + (uint32_t)i) % KIND_COUNT); }

// A word for exercise k near difficulty t: one of those within half a level of the closest, at random, none of `avoid`.
inline int pick(Kind k, int t, uint32_t& rng, const uint16_t* avoid, int nAvoid) {
  auto avoided = [&](int i) { for (int j = 0; j < nAvoid; j++) if (avoid[j] == i) return true; return false; };
  int best = 1 << 30;
  for (int i = 0; i < WORD_COUNT; i++)
    if (fits(WORDS[i], k) && !avoided(i)) { const int d = abs(beta(WORDS[i], k) - t); best = d < best ? d : best; }
  int chosen = -1, seen = 0;
  for (int i = 0; i < WORD_COUNT; i++)
    if (fits(WORDS[i], k) && !avoided(i) && abs(beta(WORDS[i], k) - t) <= best + skills::LEVEL / 2 && puzzles::below(rng, ++seen) == 0) chosen = i;
  return chosen;
}

namespace detail {
template <typename T, int N> void shuffle(T (&a)[N], int n, uint32_t& rng) {
  for (int i = n - 1; i > 0; i--) { const int j = puzzles::below(rng, i + 1); const T x = a[i]; a[i] = a[j]; a[j] = x; }
}
// A wrong meaning: a word near the same step with another English meaning than any option so far.
inline uint16_t distractor(const Question& q, int have, uint32_t& rng) {
  const int step = WORDS[q.word].step;
  for (int spread = 2; spread <= STEPS; spread += 3) {
    int chosen = -1, seen = 0;
    for (int i = 0; i < WORD_COUNT; i++) {
      if (abs(WORDS[i].step - step) > spread) continue;
      bool clash = false;
      for (int j = 0; j < have; j++) clash |= !strcmp(WORDS[i].en, WORDS[q.options[j]].en);
      if (!clash && puzzles::below(rng, ++seen) == 0) chosen = i;
    }
    if (chosen >= 0) return (uint16_t)chosen;
  }
  return 0;   // unreachable with more than three meanings
}
}  // namespace detail

inline Question build(Kind k, int word, uint32_t& rng) {
  Question q{};
  q.kind = k; q.word = (uint16_t)word;
  if (k == SYLLABLES) {
    const Word& w = WORDS[word];
    const int n = parts(w);
    for (int i = 0; i < n; i++) q.tiles[i] = {(uint16_t)word, (uint8_t)i};
    for (;;) {   // the decoy: a syllable of another word made of tiles (so it is as short) that is not one of this word's
      const int other = puzzles::below(rng, WORD_COUNT), part = puzzles::below(rng, parts(WORDS[other]));
      if (!WORDS[other].tiles) continue;
      char d[16], s[16]; syllable(WORDS[other], part, d, sizeof d);
      bool same = other == word;
      for (int i = 0; i < n && !same; i++) { syllable(w, i, s, sizeof s); same = !strcmp(s, d); }
      if (!same) { q.tiles[n] = {(uint16_t)other, (uint8_t)part}; break; }
    }
    q.tileCount = (uint8_t)(n + 1);
    detail::shuffle(q.tiles, q.tileCount, rng);
    return q;
  }
  if (k == LOOKALIKE) for (int i = 0; i < OPTIONS; i++) q.options[i] = LOOKS[WORDS[word].look - 1].words[i];
  else {
    q.options[0] = (uint16_t)word;
    for (int i = 1; i < OPTIONS; i++) q.options[i] = detail::distractor(q, i, rng);
  }
  detail::shuffle(q.options, OPTIONS, rng);
  for (int i = 0; i < OPTIONS; i++) if (q.options[i] == word) q.right = (uint8_t)i;
  return q;
}
inline const char* hint(const Question& q) { return q.kind == LOOKALIKE ? LOOKS[WORDS[q.word].look - 1].hint : nullptr; }
}  // namespace learn::uk
