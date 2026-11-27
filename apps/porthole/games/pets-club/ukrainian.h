// Pets Club's Ukrainian words (the design page "Українська в Porthole", approved 2026-11-21): questions picked for the
// kid's rating in the shared skills engine (os/skills.h, skill WORDS_UK), which also calibrates a new kid and pays the
// stars. Pure logic, no rendering; host/test_pets_uk.cpp exercises it. Three exercises with words:
//   MEANING    a Ukrainian word, three English meanings to pick from;
//   LOOKALIKE  an English meaning, three Ukrainian words that differ by a letter or two (кіт / кит / кут);
//   SYLLABLES  an English meaning, the word's syllables and one more to tap in order (со-ба-ка); two more from step 4.
// A word's step s (1..STEPS, tools/uk_words.py) is engine level s - 1; the exercise adds KIND_OFFSET.
// And one with sentences (sentences_uk.h, the "Sentence builder" page, 2026-11-22), rated by its own skill READ_UK:
//   SENTENCE   a sentence, its translation built from tiles tapped in order, among decoys.
#pragma once
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "content_uk.h"
#include "sentences_uk.h"
#include "pet.h"
#include "puzzles.h"
#include "review.h"
#include "skills.h"

namespace learn::uk {
enum Kind : uint8_t { MEANING, LOOKALIKE, SYLLABLES, SENTENCE, KIND_COUNT };
constexpr int WORD_KINDS = SENTENCE;   // the round's word exercises take turns; sentences have their own places
constexpr int ROUND = 5, OPTIONS = 3, MAX_TILES = 5, STEPS = 7;
constexpr int KIND_OFFSET[KIND_COUNT] = {0, 175, 50, 0};   // the same word, harder to find among lookalikes or to build
// (lookalike 100 -> 175 on 2026-11-27: 96 answers right 64% where 100 predicted 77%; the likelihood fit peaks near 180)
// Faster than this is a guess, not a read (the engine counts it wrong); slower is right but not yet fluent.
constexpr uint32_t READ_MS[KIND_COUNT] = {700, 900, 1500, 2500};
constexpr uint32_t SLOW_MS[KIND_COUNT] = {10000, 10000, 15000, 60000};
constexpr int DECOY_STEP = 4;   // from this step the syllables come with two decoys

inline int beta(const Word& w, Kind k) { return skills::LEVEL * (w.step - 1) + KIND_OFFSET[k]; }
// The answers a guess picks from: three options; building in order is no guess.
constexpr int choices(Kind k) { return k == SYLLABLES || k == SENTENCE ? 1 : OPTIONS; }
inline bool fits(const Word& w, Kind k) { return k == MEANING || (k == LOOKALIKE ? w.look != 0 : k == SYLLABLES && w.tiles != 0); }
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
inline Kind kindAt(int i, uint32_t roundSeed) { return (Kind)((roundSeed + (uint32_t)i) % WORD_KINDS); }

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
    q.tileCount = (uint8_t)n;
    // the decoys: syllables of other words made of tiles (so they are as short), none like a tile already in
    for (int want = n + (w.step >= DECOY_STEP ? 2 : 1); q.tileCount < want;) {
      const int other = puzzles::below(rng, WORD_COUNT), part = puzzles::below(rng, parts(WORDS[other]));
      if (!WORDS[other].tiles || other == word) continue;
      char d[16], s[16]; syllable(WORDS[other], part, d, sizeof d);
      bool same = false;
      for (int i = 0; i < q.tileCount && !same; i++) { syllable(WORDS[q.tiles[i].word], q.tiles[i].part, s, sizeof s); same = !strcmp(s, d); }
      if (!same) q.tiles[q.tileCount++] = {(uint16_t)other, (uint8_t)part};
    }
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

// ---------------------------------------------------------------- sentences: the translation built from tiles
// A sentence's step s (1..SENTENCE_STEPS) is READ_UK level s - 1. Sentences take SENTENCE_PLACES of a round's five once
// the kid reads words at step SENTENCE_FROM; before that the round is all words.
constexpr int MAX_BANK = 9, TILE_CHARS = 32, SENTENCE_PLACES = 2, SENTENCE_FROM = 3;
constexpr uint16_t SENTENCE_KEY = 0x8000;   // a sentence among the words asked lately: its index | this
inline bool sentencesOn(const skills::Book& b) { return skills::level(b, skills::WORDS_UK) + 1 >= SENTENCE_FROM; }
inline int sentenceBeta(int i) { return skills::LEVEL * (SENTENCES[i].step - 1); }
inline int sentenceTarget(const skills::Book& b) { return skills::LEVEL * skills::target(b, skills::READ_UK, SENTENCE_STEPS, 1); }

// Item i of a '|'-separated list into out (NUL-terminated); the count of items.
inline int count(const char* list) { int n = *list ? 1 : 0; for (const char* c = list; *c; c++) n += *c == '|'; return n; }
inline void listItem(const char* list, int i, char* out, size_t cap = TILE_CHARS) {
  const char* s = list;
  for (int k = 0; k < i && s; k++) { s = strchr(s, '|'); if (s) s++; }
  size_t n = 0;
  for (; s && *s && *s != '|' && n + 1 < cap; s++) out[n++] = *s;
  out[n] = 0;
}
inline int answerTiles(const Sentence& s) { return count(s.tiles); }

// The tiles to tap: the translation's and the decoys, shuffled. tile[i] < answer is a tile of the translation.
struct Bank { uint8_t n, answer, tile[MAX_BANK]; };
inline void bankText(const Sentence& s, const Bank& b, int i, char* out) {
  if (b.tile[i] < b.answer) listItem(s.tiles, b.tile[i], out); else listItem(s.decoys, b.tile[i] - b.answer, out);
}
inline Bank bank(int sentence, uint32_t& rng) {
  const Sentence& s = SENTENCES[sentence];
  Bank b{};
  b.answer = (uint8_t)answerTiles(s);
  b.n = (uint8_t)(b.answer + count(s.decoys));
  for (int i = 0; i < b.n; i++) b.tile[i] = (uint8_t)i;
  detail::shuffle(b.tile, b.n, rng);
  return b;
}

// Right orders: the translation's own, then each of alt. Order o's p-th tile into out; false past the orders.
inline bool orderTile(const Sentence& s, int o, int p, char* out) {
  if (o == 0) { listItem(s.tiles, p, out); return true; }
  char seq[MAX_BANK + 1];
  if (o > count(s.alt)) return false;
  listItem(s.alt, o - 1, seq, sizeof seq);
  listItem(s.tiles, seq[p] - '0', out);
  return true;
}
// Whether `next` can follow the tiles built so far (their texts, in order) in some right order: tiles that read the
// same are the same tile.
inline bool fitsNext(const Sentence& s, const char (*built)[TILE_CHARS], int nBuilt, const char* next) {
  char t[TILE_CHARS];
  for (int o = 0; orderTile(s, o, 0, t); o++) {
    bool same = true;
    for (int p = 0; p < nBuilt && same; p++) { orderTile(s, o, p, t); same = !strcmp(t, built[p]); }
    if (!same) continue;
    orderTile(s, o, nBuilt, t);
    if (!strcmp(t, next)) return true;
  }
  return false;
}
// A tile of the translation (it has a place, only not this one yet), not a decoy.
inline bool inAnswer(const Sentence& s, const char* text) {
  char t[TILE_CHARS];
  for (int i = 0; i < answerTiles(s); i++) { listItem(s.tiles, i, t); if (!strcmp(t, text)) return true; }
  return false;
}

// The first letter of s made capital: Latin, or Cyrillic in UTF-8.
inline void capitalFirst(char* s) {
  unsigned char* c = (unsigned char*)s;
  if (c[0] >= 'a' && c[0] <= 'z') c[0] = (unsigned char)(c[0] - 32);
  else if (c[0] == 0xD0 && c[1] >= 0xB0 && c[1] <= 0xBF) c[1] = (unsigned char)(c[1] - 0x20);   // а..п
  else if (c[0] == 0xD1 && c[1] >= 0x80 && c[1] <= 0x8F) { c[0] = 0xD0; c[1] = (unsigned char)(c[1] + 0x20); }   // р..я
  else if (c[0] == 0xD1 && c[1] >= 0x90 && c[1] <= 0x9F) { c[0] = 0xD0; c[1] = (unsigned char)(c[1] - 0x10); }   // є, і, ї
  else if (c[0] == 0xD2 && c[1] == 0x91) c[1] = 0x90;   // ґ
}
// The translation as built: its tiles joined by spaces, the first letter capital (Latin or Cyrillic), and a full stop
// once it is all there.
inline void joined(const char (*built)[TILE_CHARS], int n, bool done, char* out, size_t cap) {
  size_t at = 0;
  out[0] = 0;
  for (int i = 0; i < n; i++) at += (size_t)snprintf(out + at, cap - at, "%s%s", i ? " " : "", built[i]);
  if (done && at + 1 < cap) { out[at++] = '.'; out[at] = 0; }
  capitalFirst(out);
}
// A sentence for difficulty t: one of those within half a level of the closest, at random, none asked lately (avoid
// holds SENTENCE_KEY | index).
inline int pickSentence(int t, uint32_t& rng, const uint16_t* avoid, int nAvoid) {
  auto avoided = [&](int i) { for (int j = 0; j < nAvoid; j++) if (avoid[j] == (SENTENCE_KEY | i)) return true; return false; };
  int best = 1 << 30;
  for (int i = 0; i < SENTENCE_COUNT; i++) if (!avoided(i)) { const int d = abs(sentenceBeta(i) - t); best = d < best ? d : best; }
  int chosen = -1, seen = 0;
  for (int i = 0; i < SENTENCE_COUNT; i++)
    if (!avoided(i) && abs(sentenceBeta(i) - t) <= best + skills::LEVEL / 2 && puzzles::below(rng, ++seen) == 0) chosen = i;
  return chosen;
}
// A built sentence, for skills::answer: right with no miss (1), right with one (0.7, as slow), wrong with more.
inline bool sentenceRight(int misses) { return misses <= 1; }
inline skills::Item sentenceItem(int i, uint32_t ms, int misses) {
  return {skills::READ_UK, sentenceBeta(i), 1, READ_MS[SENTENCE], (uint16_t)(SENTENCE << 12 | i),
          misses == 1 || ms > SLOW_MS[SENTENCE] ? skills::SLOW : skills::QUICK};
}
// The round's places for sentences: SENTENCE_PLACES of the five, never `skip` (the review's), as bits.
inline uint8_t sentencePlaces(uint32_t& rng, int skip) {
  uint8_t at = 0;
  for (int n = 0; n < SENTENCE_PLACES;) {
    const int i = puzzles::below(rng, ROUND);
    if (i != skip && !((at >> i) & 1)) { at |= (uint8_t)(1u << i); n++; }
  }
  return at;
}
}  // namespace learn::uk
