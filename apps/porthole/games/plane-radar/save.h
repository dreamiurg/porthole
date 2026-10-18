// Plane Radar's save: the kid's settings (range, which way is up, what each plane's label shows). Persisted as a raw blob per profile
// (radar/s<id>, written by the shell). Append-only from this first version: a new field goes right before crc (its zero
// must be a sensible default, loadBlob zero-fills it for older blobs), SAVE_VERSION goes up, nothing is reordered or
// resized. Header-only, like Tilt FC's save.h.
#pragma once
#include <stddef.h>
#include <stdint.h>
#include <string.h>
#include "crc32.h"

namespace radar {
constexpr uint32_t SAVE_MAGIC = 0x52444152;   // "RADR"
constexpr uint16_t SAVE_VERSION = 1;
constexpr size_t SAVE_V1_SIZE = 20;
constexpr size_t SAVE_SIZES[] = {0, SAVE_V1_SIZE};   // by version: a blob is exactly its version's size
struct Save {
  uint32_t magic;
  uint16_t version, size;
  uint8_t range;      // index into scope.h RANGE_MI
  uint8_t up;         // which way is at the top of the screen: 0 N, 1 E, 2 S, 3 W
  uint8_t hideCall;   // 1: no flight number beside the planes
  uint8_t hideRoute;  // 1: no SEA-SFO
  uint8_t hideAlt;    // 1: no altitude
  uint8_t reserved[3];   // zero; explicit so the layout has no hidden padding
  uint32_t crc;
};
static_assert(sizeof(Save) == SAVE_V1_SIZE, "Save is persisted: append before crc, never resize");
static_assert(sizeof SAVE_SIZES / sizeof SAVE_SIZES[0] == SAVE_VERSION + 1, "one size per version");

inline void seal(Save& s) {
  s.magic = SAVE_MAGIC; s.version = SAVE_VERSION; s.size = sizeof(Save); memset(s.reserved, 0, sizeof s.reserved);
  s.crc = os::crc32(&s, sizeof s - sizeof s.crc);
}
// Any version up to this one: a blob is its own size (a prefix of today's layout) with its crc in the last four
// bytes, so fields appended after it was written load as zero. A blob that is not a save leaves `out` as it was.
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
}  // namespace radar
