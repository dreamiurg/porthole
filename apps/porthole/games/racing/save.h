// Rally's save: a kid's best race time and best lap on each track, for the leaderboard every profile shares. Persisted
// as a raw blob per profile (racing/s<id>, written by the shell). Append-only from this first version: a new field
// goes right before crc (its zero must be a sensible default: loadBlob zero-fills it for older blobs), SAVE_VERSION
// goes up, nothing is reordered or resized. Header-only, like Marble Kick's.
#pragma once
#include <stddef.h>
#include <stdint.h>
#include <string.h>
#include "crc32.h"
#include "track.h"

namespace racing {
constexpr uint32_t SAVE_MAGIC = 0x594C4C52;   // "RLLY"
constexpr uint16_t SAVE_VERSION = 1;
constexpr size_t SAVE_V1_SIZE = 44;
constexpr size_t SAVE_SIZES[] = {0, SAVE_V1_SIZE};   // by version: a blob is exactly its version's size
struct Best { uint32_t raceMs, lapMs; };              // 0 = not driven yet
struct Save {
  uint32_t magic;
  uint16_t version, size;
  Best best[TRACKS];   // by track index (TRACK[] order: never reordered)
  uint32_t crc;
};
static_assert(sizeof(Save) == SAVE_V1_SIZE, "Save is persisted: append before crc, never resize");
static_assert(sizeof SAVE_SIZES / sizeof SAVE_SIZES[0] == SAVE_VERSION + 1, "one size per version");

inline void seal(Save& s) {
  s.magic = SAVE_MAGIC; s.version = SAVE_VERSION; s.size = sizeof(Save);
  s.crc = os::crc32(&s, sizeof s - sizeof s.crc);
}
// Any version up to this one: a blob is its own size with its crc in the last four bytes. Anything else is ignored:
// the kid simply has no times yet, never a half-loaded record.
inline bool loadBlob(const void* data, size_t n, Save& out) {
  if (n < SAVE_V1_SIZE || n > sizeof(Save) || n % 4) return false;
  const uint8_t* b = (const uint8_t*)data;
  uint32_t crc;
  memcpy(&crc, b + n - 4, 4);
  Save s{};
  memcpy(&s, b, n - 4);
  if (s.magic != SAVE_MAGIC || !s.version || s.version > SAVE_VERSION || s.size != n || n != SAVE_SIZES[s.version] ||
      crc != os::crc32(b, n - 4)) return false;
  seal(s);
  out = s;
  return true;
}
// Keep the better of each time; says which improved (0 = neither), so a faster lap in a slower race is not called a
// best race.
enum : uint8_t { BEST_RACE = 1, BEST_LAP = 2 };
inline uint8_t record(Best& b, uint32_t raceMs, uint32_t lapMs) {
  uint8_t better = 0;
  if (raceMs && (!b.raceMs || raceMs < b.raceMs)) { b.raceMs = raceMs; better |= BEST_RACE; }
  if (lapMs && (!b.lapMs || lapMs < b.lapMs)) { b.lapMs = lapMs; better |= BEST_LAP; }
  return better;
}
}  // namespace racing
