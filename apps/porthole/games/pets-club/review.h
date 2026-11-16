// Missed puzzles come back on a later day (docs/design/adaptive-practice.md, piece B). The queue lives in the save
// (puzzleReviews, oldest first): a miss is due the next day, a round in its strand on or after that day asks it once,
// right then takes it off and wrong pushes it a day. Being away punishes nothing: an overdue item is simply due.
// Pure logic on the save; host/test_pets_learn.cpp exercises it.
#pragma once
#include "pet.h"
#include "puzzles.h"

namespace learn::review {
inline puzzles::Item itemOf(const PuzzleReview& r) { return {r.strand, r.level, r.a, r.b, r.c}; }
inline bool same(const PuzzleReview& r, const puzzles::Item& q) {
  return r.strand == q.strand && r.level == q.level && r.a == q.a && r.b == q.b && r.c == q.c;
}
// How many entries to trust: a damaged count never reads past the queue.
inline int count(const Save& p) { return p.puzzleReviewCount < PUZZLE_REVIEWS ? p.puzzleReviewCount : PUZZLE_REVIEWS; }
inline int find(const Save& p, const puzzles::Item& q) {
  for (int i = 0; i < count(p); i++) if (same(p.puzzleReviews[i], q)) return i;
  return -1;
}
inline void drop(Save& p, int i) {
  const int n = count(p);
  for (int j = i; j + 1 < n; j++) p.puzzleReviews[j] = p.puzzleReviews[j + 1];
  p.puzzleReviews[n - 1] = {};
  p.puzzleReviewCount = (uint8_t)(n - 1);
}
// The oldest item due today or before in this strand that still plays, else -1.
inline int due(const Save& p, puzzles::Strand strand, int32_t today) {
  for (int i = 0; i < count(p); i++) {
    const PuzzleReview& r = p.puzzleReviews[i];
    if (r.strand == strand && r.due && r.due <= today && puzzles::playable(itemOf(r))) return i;
  }
  return -1;
}
// An answer: a miss goes on the queue (or stays, a day later); a right answer to an item that was due takes it off.
// A full queue drops its oldest item for a new one.
inline void answered(Save& p, const puzzles::Item& q, bool right, int32_t today) {
  int i = find(p, q);
  if (right) {
    if (i >= 0 && p.puzzleReviews[i].due <= today) drop(p, i);
    return;
  }
  if (i < 0) {
    if (count(p) == PUZZLE_REVIEWS) drop(p, 0);
    i = count(p);
    p.puzzleReviews[i] = {q.strand, q.level, q.a, q.b, q.c, 0, 0};
    p.puzzleReviewCount = (uint8_t)(i + 1);
  }
  PuzzleReview& r = p.puzzleReviews[i];
  if (r.misses < 255) r.misses++;
  r.due = (uint16_t)(today + 1);
}
}  // namespace learn::review
