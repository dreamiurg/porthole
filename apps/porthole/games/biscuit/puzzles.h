// Biscuit's puzzles: one question with three answers to pick from, made up on the spot. Pure logic, no rendering;
// host/test_biscuit.cpp exercises it. Kinds run from the start of first grade (adding within ten-ish, short words) to
// the start of third (two-digit sums, times tables 2 to 6, long words); every kid picks the kind they want.
#pragma once
#include <stdint.h>
#include <stdio.h>
#include <string.h>
#include "content_puzzles.h"

namespace biscuit::puzzles {
enum Kind : uint8_t { ADD, TAKE, BIG, TIMES, SHORT, LONG, KIND_COUNT };
constexpr int ANSWERS = 3, ROUND = 5;
constexpr uint8_t FIRST_TABLE = 2, LAST_TABLE = 6;   // TIMES: table 0 mixes them all

struct Puzzle {
  const char* clue;        // a spelling word's clue, else nullptr
  char ask[24];            // "4 x 6 = ?" or "f r _ e n d"
  char solved[24];         // "4 x 6 = 24" or "friend"
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
struct Sum { int a; char op; int b; int step; };   // "a op b"; the wrong answers are a step or two off
inline void number(Puzzle& p, uint32_t& s, Sum q) {
  const int result = q.op == '+' ? q.a + q.b : q.op == '-' ? q.a - q.b : q.a * q.b, step = q.step;
  snprintf(p.ask, sizeof p.ask, "%d %c %d = ?", q.a, q.op, q.b);
  snprintf(p.solved, sizeof p.solved, "%d %c %d = %d", q.a, q.op, q.b, result);
  char right[12];
  snprintf(right, sizeof right, "%d", result);
  fill(p, s, right, [&](char (&out)[12]) {   // near misses: a step or two either way, or one off
    static constexpr int OFFSETS[] = {-2, -1, 1, 2};
    int v = below(s, 3) ? result + step * OFFSETS[below(s, 4)] : result + (below(s, 2) ? 1 : -1);
    if (v < 0) v = result + step * 3 + below(s, 3);
    snprintf(out, sizeof out, "%d", v > 999 ? 999 : v);
  });
}
inline void spell(Puzzle& p, uint32_t& s, const SpellWord& w) {
  const int n = (int)strlen(w.word), gap = below(s, n);
  const char missing = w.word[gap];
  p.clue = w.clue;
  spaced(w.word, gap, p.ask);
  snprintf(p.solved, sizeof p.solved, "%s", w.word);
  const char right[2] = {missing, 0};
  fill(p, s, right, [&](char (&out)[12]) {   // a vowel for a vowel, a consonant for a consonant
    static constexpr const char* CONSONANTS = "bcdfghklmnprstw";
    const char* from = vowel(missing) ? VOWELS : CONSONANTS;
    out[0] = from[below(s, (int)strlen(from))]; out[1] = 0;
  });
}
}  // namespace detail

// A new question of this kind (TIMES: table 2..6, or 0 for any of them), not one whose key is in seen.
inline Puzzle make(Kind kind, uint8_t table, uint32_t& s, uint32_t seen) {
  Puzzle p{};
  for (int tries = 0; tries < 16; tries++) {
    p = Puzzle{};
    int a = 0, b = 0, key;
    switch (kind) {
      case ADD: a = below(s, 11); b = 1 + below(s, 10); detail::number(p, s, {a, '+', b, 1}); key = a * 11 + b; break;
      case TAKE: a = 2 + below(s, 19); b = 1 + below(s, a < 11 ? a : 10); detail::number(p, s, {a, '-', b, 1}); key = a * 11 + b; break;
      case BIG:
        a = 10 + below(s, 80);
        if (below(s, 2)) { b = 10 + below(s, 90 - a); detail::number(p, s, {a, '+', b, 10}); }
        else { b = 10 + below(s, a - 9); detail::number(p, s, {a, '-', b, 10}); }
        key = a * 11 + b;   // not a + b: 30 + 20 and 40 + 10 are different questions
        break;
      case TIMES: {
        a = table >= FIRST_TABLE && table <= LAST_TABLE ? table : FIRST_TABLE + below(s, LAST_TABLE - FIRST_TABLE + 1);
        b = 1 + below(s, 10);
        detail::number(p, s, {a, 'x', b, a});
        key = a * 10 + b;
        break;
      }
      case SHORT: { const int i = below(s, SHORT_WORD_COUNT); detail::spell(p, s, SHORT_WORDS[i]); key = i; break; }
      default: { const int i = below(s, LONG_WORD_COUNT); detail::spell(p, s, LONG_WORDS[i]); key = i; break; }
    }
    p.key = (uint8_t)(key % 32);
    if (!(seen & (1u << p.key))) break;
  }
  return p;
}
}  // namespace biscuit::puzzles
