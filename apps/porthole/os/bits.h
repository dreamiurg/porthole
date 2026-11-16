// Bit packing for the saves that would not fit otherwise: the board's NVS is 20 KB for the whole household, and a save
// kept as its raw struct spends most of it on zeros (games/living-kingdom, games/times-cards).
//
// One description of a layout both writes and reads it, so the two cannot drift apart: `v = b.io(v, n)` stores v's low
// n bits (and gives v back), or, reading, gives back the n bits stored there. Bits go in from each byte's low end; a
// value that does not fit its n bits, or a buffer too short either way, clears ok() and nothing past the end is touched.
#pragma once
#include <stddef.h>
#include <stdint.h>

namespace os {
struct Bits {
  const uint8_t* in;   // reading from here; nullptr while writing
  uint8_t* out;        // writing here (zeroed by the caller); nullptr while reading
  size_t cap;          // bytes in the buffer
  size_t at;           // bits used so far
  bool bad;
  static Bits reader(const uint8_t* data, size_t bytes) { return {data, nullptr, bytes, 0, false}; }
  static Bits writer(uint8_t* zeroed, size_t bytes) { return {nullptr, zeroed, bytes, 0, false}; }
  bool ok() const { return !bad && at <= cap * 8; }
  size_t bytes() const { return (at + 7) / 8; }
  uint32_t io(uint32_t v, int n) {
    if (out && n < 32 && v >> n) bad = true;
    uint32_t got = 0;
    for (int i = 0; i < n; i++, at++) {
      if (at >= cap * 8) continue;
      const uint8_t bit = (uint8_t)(1u << (at % 8));
      if (in) got |= (uint32_t)((in[at / 8] & bit) != 0) << i;
      else if (v >> i & 1) out[at / 8] |= bit;
    }
    return in ? got : v;
  }
  // A value that is usually zero: one bit, and its n bits only when it is not.
  uint32_t sparse(uint32_t v, int n) { return io(v != 0, 1) ? io(v, n) : 0; }
};
// The fewest bits that hold v.
inline int bitsFor(uint32_t v) { int n = 0; while (n < 32 && v >> n) n++; return n; }
}  // namespace os
