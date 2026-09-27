// Marble Kick's save: the level to play next and the best stars per level. Persisted as a raw blob per profile
// (marblekick/s<id>, written by the shell). Append-only from this first version: a new field goes right before crc (its
// zero must be a sensible default, loadBlob zero-fills it for older blobs), SAVE_VERSION goes up, nothing is reordered
// or resized.
// Header-only, like Biscuit's pet.h.
//
// A downgrade loses progress: once a later version appends a field, an older firmware sees a blob bigger than its Save
// (and a version above its own) and rejects it, so the kid starts again from level 1 (the next save overwrites it).
// Accepted while progress is a handful of levels; weigh it before appending anything a kid would miss.
#pragma once
#include <stddef.h>
#include <stdint.h>
#include <string.h>
#include "crc32.h"

namespace marble {
constexpr const char* STORE = "marblekick";   // NVS namespace: never renamed once shipped
constexpr uint32_t SAVE_MAGIC = 0x4B524D4D;   // "MMRK"
constexpr uint16_t SAVE_VERSION = 2;
constexpr size_t SAVE_V1_SIZE = 16, SAVE_V2_SIZE = 48;
constexpr size_t SAVE_SIZES[] = {0, SAVE_V1_SIZE, SAVE_V2_SIZE};   // by version: a blob is exactly its version's size
constexpr int STAR_LEVELS = 32;                                    // room for this many levels' stars
struct Save {
  uint32_t magic;
  uint16_t version, size;
  uint8_t level;         // levels finished in a row from the first: the index of the level to play next, up to
                         // NUM_LEVELS (all done). Kept as is when it exceeds this build's levels (a newer firmware's).
  uint8_t reserved[3];   // zero; explicit so the layout has no hidden padding
  uint8_t stars[STAR_LEVELS];   // v2: the most stars picked up in one go at each level (0-3), by level index
  uint32_t crc;
};
static_assert(sizeof(Save) == SAVE_V2_SIZE, "Save is persisted: append before crc, never resize");
static_assert(sizeof SAVE_SIZES / sizeof SAVE_SIZES[0] == SAVE_VERSION + 1, "one size per version");

inline void seal(Save& s) {
  s.magic = SAVE_MAGIC; s.version = SAVE_VERSION; s.size = sizeof(Save); memset(s.reserved, 0, sizeof s.reserved);
  s.crc = os::crc32(&s, sizeof s - sizeof s.crc);
}
// Any version up to this one: a blob is its own size (a prefix of today's layout) with its crc in the last four
// bytes, so fields appended after it was written load as zero.
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
}  // namespace marble
