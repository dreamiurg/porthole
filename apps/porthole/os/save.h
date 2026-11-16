// The envelope every game's save shares, and all the shell may read of a blob it did not write: eight bytes of
// magic, version and size first, a crc32 of everything before it in the last four. host/test_saves.cpp holds every
// saving game to it.
#pragma once
#include <stddef.h>
#include <stdint.h>
#include <string.h>
#include "crc32.h"

namespace os {
namespace save {
struct Head { uint32_t magic; uint16_t version, size; };

// A whole save: its size field is its length and its crc holds.
inline bool sealed(const void* d, size_t n, Head* out = nullptr) {
  Head h; uint32_t crc;
  if (!d || n < sizeof h + sizeof crc) return false;
  memcpy(&h, d, sizeof h); memcpy(&crc, (const uint8_t*)d + n - sizeof crc, sizeof crc);
  if (h.size != n || crc != os::crc32(d, n - sizeof crc)) return false;
  if (out) *out = h;
  return true;
}
// `stored` is this game's save from a newer firmware than the one that made `mine` (an older build flashed over a
// newer one): this build cannot read it and must not replace it. A damaged blob is nobody's save and is not kept.
inline bool newer(const void* stored, size_t sn, const void* mine, size_t mn) {
  Head s, m;
  if (!stored || sn < sizeof s || mn < sizeof m) return false;
  memcpy(&s, stored, sizeof s); memcpy(&m, mine, sizeof m);
  return s.magic == m.magic && s.version > m.version && sealed(stored, sn);
}
}  // namespace save
}  // namespace os
