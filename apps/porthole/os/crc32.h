// CRC-32 (IEEE, reflected) for persisted blobs: every game's save and the shell's records. Also what a few screens hash
// to tell whether to repaint, every frame, so it goes a nibble at a time through a 16-word table (in flash on the
// board: no RAM) instead of bit by bit. host/test_crc32.cpp holds it to the bit-by-bit sum, byte for byte.
#pragma once
#include <stddef.h>
#include <stdint.h>

namespace os {
inline uint32_t crc32(const void* d, size_t n) {
  static constexpr uint32_t T[16] = {0x00000000u, 0x1DB71064u, 0x3B6E20C8u, 0x26D930ACu, 0x76DC4190u, 0x6B6B51F4u,
                                     0x4DB26158u, 0x5005713Cu, 0xEDB88320u, 0xF00F9344u, 0xD6D6A3E8u, 0xCB61B38Cu,
                                     0x9B64C2B0u, 0x86D3D2D4u, 0xA00AE278u, 0xBDBDF21Cu};
  const uint8_t* p = (const uint8_t*)d; uint32_t c = 0xFFFFFFFFu;
  for (size_t i = 0; i < n; i++) { c ^= p[i]; c = T[c & 15] ^ (c >> 4); c = T[c & 15] ^ (c >> 4); }
  return ~c;
}
}  // namespace os
