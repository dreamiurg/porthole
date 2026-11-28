// A kid's stars: one balance per profile, earned in any game (Pets Club's puzzles and little reads, a flashcard round)
// and spent in any game (Pets Club's hats). The shell keeps it in the profile's record and saves it when it changes; a
// game gets the playing kid's balance as AppEnter::stars and changes it only through these. nullptr (a test, or a
// shell that keeps none) makes them no-ops. Header-only.
#pragma once
#include <stdint.h>

namespace stars {
constexpr uint32_t MAX = 99999;   // the most a kid can hold: the header's "99999 stars" still fits
inline uint32_t balance(const uint32_t* wallet) { return wallet ? *wallet : 0; }
inline void award(uint32_t* wallet, uint32_t n) {
  if (wallet) *wallet = *wallet >= MAX || n >= MAX - *wallet ? MAX : *wallet + n;
}
inline bool spend(uint32_t* wallet, uint32_t n) {   // false (and nothing taken) when the kid has fewer than n
  if (!wallet || *wallet < n) return false;
  *wallet -= n;
  return true;
}
}  // namespace stars
