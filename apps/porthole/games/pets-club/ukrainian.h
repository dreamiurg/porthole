// Pets Club's Ukrainian words (the design page "Українська в Porthole", approved 2026-11-21): one kid's rating on a
// ladder of ten steps, a quick calibration that finds it from the easiest word up, and questions picked for it. Pure
// logic, no rendering; host/test_pets_uk.cpp exercises it. Three exercises:
//   MEANING    a Ukrainian word, three English meanings to pick from;
//   LOOKALIKE  an English meaning, three Ukrainian words that differ by a letter or two (кіт / кит / кут);
//   SYLLABLES  an English meaning, the word's syllables and one more to tap in order (со-ба-ка).
// Ratings follow the shared engine's page (100 rating = one step, about 80% right once calibrated, K from 160 down
// to 40) so they can move into it: until then they live in the kid's Pets Club save (pet.h's UkWords).
#pragma once
#include <math.h>
#include <stdint.h>
#include <string.h>
#include "content_uk.h"
#include "pet.h"
#include "puzzles.h"

namespace learn::uk {
enum Kind : uint8_t { MEANING, LOOKALIKE, SYLLABLES, KIND_COUNT };
constexpr int ROUND = 5, OPTIONS = 3, MAX_TILES = 4, STEPS = 10;
constexpr int16_t STEP = 100;                                  // rating per step: step s is rated s * STEP
constexpr int16_t KIND_OFFSET[KIND_COUNT] = {0, 100, 50};      // the same word, harder to find among lookalikes or to build
constexpr int16_t TARGET_BELOW = 85;   // with three options, 80% right is an item 85 under the kid's rating
constexpr int16_t RATING_MIN = 50, RATING_MAX = 1100;
constexpr int CALIBRATION_MAX = 15;    // answers: calibration ends by then whatever happens
// Faster than this is a guess, not a read (it moves nothing up); slower is right but not yet fluent (0.7 of a point).
constexpr uint32_t GUESS_MS[KIND_COUNT] = {700, 900, 1500};
constexpr uint32_t SLOW_MS[KIND_COUNT] = {10000, 10000, 15000};

inline int16_t beta(const Word& w, Kind k) { return (int16_t)(STEP * w.step + KIND_OFFSET[k]); }
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

// ---------------------------------------------------------------- the rating and the calibration
inline bool started(const UkWords& p) { return p.rating != 0; }
inline bool calibrating(const UkWords& p) { return p.jump != 0; }
inline int stepOf(const UkWords& p) {   // the step the kid reads at now, 1..STEPS
  if (calibrating(p)) return p.at;
  const int s = (p.rating + STEP / 2) / STEP;
  return s < 1 ? 1 : s > STEPS ? STEPS : s;
}
// A new kid starts at the easiest step (or one a grown-up set), jumping up 1, 2, 4 steps while the answers are right.
inline void start(UkWords& p, int fromStep = 1) {
  if (started(p)) return;
  p = UkWords{};
  p.at = (uint8_t)(fromStep < 1 ? 1 : fromStep > STEPS ? STEPS : fromStep);
  p.rating = (int16_t)(STEP * p.at); p.jump = 1;
}
// The difficulty the next question should have: the step being tried, then the kid's rating less TARGET_BELOW.
inline int16_t target(const UkWords& p) { return calibrating(p) ? (int16_t)(STEP * p.at) : (int16_t)(p.rating - TARGET_BELOW); }
// The chance of a right answer: a third by guessing, the rest by how far the kid is above the question.
inline float expected(int16_t rating, int16_t b) { return 1.0f / 3 + (2.0f / 3) / (1.0f + expf(-(rating - b) / 100.0f)); }
inline float score(bool right, Kind k, uint32_t ms) { return !right || ms < GUESS_MS[k] ? 0.0f : ms > SLOW_MS[k] ? 0.7f : 1.0f; }

namespace detail {
inline int clampStep(int s) { return s < 1 ? 1 : s > STEPS ? STEPS : s; }
// One calibration answer (the page's flowchart): up the jump and double it while right; the first miss turns back by
// half the last jump, and from then on every answer moves by a jump that halves down to one step. Done after two
// misses at one step, at the top, or after CALIBRATION_MAX answers; the rating is the last step answered right.
inline bool calibrate(UkWords& p, bool ok) {
  const bool fine = p.searching ? p.jump == 1 : p.jump <= 2;   // the last move was one step or none
  bool done = false;
  if (ok) p.lastRight = p.at;
  else if (fine) p.misses++;
  if (!p.searching) {
    if (ok) {
      done = p.at == STEPS;
      p.at = (uint8_t)clampStep(p.at + p.jump);
      p.jump = (uint8_t)(p.jump < 16 ? p.jump * 2 : 16);
    } else {
      p.searching = 1;
      const int back = p.jump / 4 < 1 ? 1 : p.jump / 4;
      p.at = (uint8_t)clampStep(p.at - back);
      p.jump = (uint8_t)(back / 2 < 1 ? 1 : back / 2);
    }
  } else {
    p.at = (uint8_t)clampStep(p.at + (ok ? p.jump : -p.jump));
    p.jump = (uint8_t)(p.jump / 2 < 1 ? 1 : p.jump / 2);
  }
  if (!done && p.misses < 2 && p.answers < CALIBRATION_MAX) return false;
  p.rating = (int16_t)(STEP * (p.lastRight ? p.lastRight : 1));
  p.jump = 0; p.at = 0;
  return true;
}
}  // namespace detail

// An answer to a question of difficulty b. True when it ended the calibration (the kid's step is found).
inline bool answer(UkWords& p, int16_t b, bool right, Kind k, uint32_t ms) {
  start(p);
  p.answers = p.answers < 0xFFFF ? (uint16_t)(p.answers + 1) : p.answers;
  if (right && p.right < 0xFFFF) p.right++;
  const float s = score(right, k, ms);
  if (calibrating(p)) return detail::calibrate(p, s > 0);
  const float kf = 160.0f / (1.0f + p.answers / 5.0f);
  const float delta = (kf < 40 ? 40 : kf) * (s - expected(p.rating, b));
  const int r = p.rating + (int)lroundf(delta);
  p.rating = (int16_t)(r < RATING_MIN ? RATING_MIN : r > RATING_MAX ? RATING_MAX : r);
  return false;
}

// ---------------------------------------------------------------- the review queue: a missed word comes back tomorrow
inline void missed(UkWords& p, uint16_t word, uint16_t today) {
  int at = -1;
  for (int i = 0; i < UK_REVIEWS; i++) if (p.reviewWord[i] == word + 1) at = i;
  if (at < 0) {   // a new one goes last; a full queue lets its oldest go
    at = 0;
    while (at < UK_REVIEWS && p.reviewWord[at]) at++;
    if (at == UK_REVIEWS) {
      memmove(p.reviewWord, p.reviewWord + 1, sizeof p.reviewWord - sizeof *p.reviewWord);
      memmove(p.reviewDue, p.reviewDue + 1, sizeof p.reviewDue - sizeof *p.reviewDue);
      at = UK_REVIEWS - 1;
    }
  }
  p.reviewWord[at] = (uint16_t)(word + 1); p.reviewDue[at] = (uint16_t)(today + 1);
}
inline void reviewed(UkWords& p, uint16_t word) {   // a review answered right leaves the queue
  for (int i = 0; i < UK_REVIEWS; i++) {
    if (p.reviewWord[i] != word + 1) continue;
    for (int j = i; j + 1 < UK_REVIEWS; j++) { p.reviewWord[j] = p.reviewWord[j + 1]; p.reviewDue[j] = p.reviewDue[j + 1]; }
    p.reviewWord[UK_REVIEWS - 1] = p.reviewDue[UK_REVIEWS - 1] = 0;
    return;
  }
}
inline int due(const UkWords& p, uint16_t today) {   // the oldest word due today, or -1
  for (int i = 0; i < UK_REVIEWS; i++)
    if (p.reviewWord[i] && p.reviewWord[i] - 1 < WORD_COUNT && p.reviewDue[i] <= today) return p.reviewWord[i] - 1;
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

// A word for exercise k near difficulty t: one of those within 50 of the closest, at random, none of `avoid`.
inline int pick(Kind k, int16_t t, uint32_t& rng, const uint16_t* avoid, int nAvoid) {
  auto avoided = [&](int i) { for (int j = 0; j < nAvoid; j++) if (avoid[j] == i) return true; return false; };
  int best = 1 << 30;
  for (int i = 0; i < WORD_COUNT; i++)
    if (fits(WORDS[i], k) && !avoided(i)) { const int d = abs(beta(WORDS[i], k) - t); best = d < best ? d : best; }
  int chosen = -1, seen = 0;
  for (int i = 0; i < WORD_COUNT; i++)
    if (fits(WORDS[i], k) && !avoided(i) && abs(beta(WORDS[i], k) - t) <= best + 50 && puzzles::below(rng, ++seen) == 0) chosen = i;
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
