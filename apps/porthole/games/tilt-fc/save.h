// Tilt FC's save: matches won and the difficulty level they earned. Persisted as a raw blob per profile (tiltfc/s<id>,
// written by the shell). Append-only from this first version: a new field goes right before crc (its zero must be a
// sensible default, loadBlob zero-fills it for older blobs), SAVE_VERSION goes up, nothing is reordered or resized.
// Header-only, like Marble Kick's save.h.
#pragma once
#include <stddef.h>
#include <stdint.h>
#include <string.h>
#include "os/save.h"   // not "save.h": a game may have its own

namespace fc {
constexpr const char* STORE = "tiltfc";       // NVS namespace: never renamed once shipped
constexpr uint32_t SAVE_MAGIC = 0x43464C54;   // "TLFC"
constexpr uint16_t SAVE_VERSION = 1;
constexpr size_t SAVE_V1_SIZE = 16;
constexpr size_t SAVE_SIZES[] = {0, SAVE_V1_SIZE};   // by version: a blob is exactly its version's size
struct Save {
  uint32_t magic;
  uint16_t version, size;
  uint16_t wins;       // matches won, ever
  uint8_t level;       // the next match's difficulty (tune.h LEVELS): one up per win, one down per loss by two or more
  uint8_t reserved;    // zero; explicit so the layout has no hidden padding
  uint32_t crc;
};
static_assert(sizeof(Save) == SAVE_V1_SIZE, "Save is persisted: append before crc, never resize");
static_assert(sizeof SAVE_SIZES / sizeof SAVE_SIZES[0] == SAVE_VERSION + 1, "one size per version");

inline void seal(Save& s) {
  s.reserved = 0;
  os::save::seal(&s, sizeof s, SAVE_MAGIC, SAVE_VERSION);
}
// Any version up to this one: a blob is its own size (a prefix of today's layout) with its crc in the last four
// bytes, so fields appended after it was written load as zero. A blob that is not a save leaves `out` as it was.
inline bool loadBlob(const void* data, size_t n, Save& out) {
  os::save::Head h;
  if (n > sizeof(Save) || !os::save::mine(data, n, SAVE_MAGIC, SAVE_VERSION, &h) || n != SAVE_SIZES[h.version]) return false;
  Save s{};
  memcpy(&s, data, n - sizeof s.crc);
  seal(s);
  out = s;
  return true;
}
}  // namespace fc
