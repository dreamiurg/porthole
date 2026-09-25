// CRC-32 (IEEE, reflected) for persisted blobs: Pets Club saves and shell profile records.
#pragma once
#include <stddef.h>
#include <stdint.h>

namespace os {
inline uint32_t crc32(const void* d, size_t n) {
  const uint8_t* p = (const uint8_t*)d; uint32_t c = 0xFFFFFFFFu;
  for (size_t i = 0; i < n; i++) { c ^= p[i]; for (int k = 0; k < 8; k++) c = (c >> 1) ^ (0xEDB88320u & (0u - (c & 1))); }
  return ~c;
}
}  // namespace os
