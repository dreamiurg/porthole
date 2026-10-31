// Pure network-clock validation and local-wall-clock conversion; no platform dependencies.
#pragma once
#include <stddef.h>
#include <stdint.h>
#include <string.h>

namespace netclock {
constexpr uint32_t MIN_UTC = 1704067200u;   // 2024-01-01
constexpr uint32_t MAX_UTC = 4102444800u;   // 2100-01-01; RTC's two-digit years stop here
inline bool validOffset(int minutes) { return minutes >= -720 && minutes <= 840 && minutes % 15 == 0; }
inline uint32_t local(uint32_t utc, int minutes) { return (uint32_t)((int64_t)utc + minutes * 60); }
inline uint32_t advanced(uint32_t epoch, uint32_t sampledMs, uint32_t nowMs) {
  return epoch + (nowMs - sampledMs) / 1000u;   // unsigned subtraction also spans a millis wrap
}
inline uint32_t read32(const uint8_t* p) {
  return ((uint32_t)p[0] << 24) | ((uint32_t)p[1] << 16) | ((uint32_t)p[2] << 8) | p[3];
}
// A reply must echo this request's unpredictable transmit token in its originate field: a stale datagram cannot
// count as a fresh sync. Reject unsynchronized servers, kiss-of-death responses, and non-server/unsupported packets.
inline bool reply(const uint8_t* p, size_t n, const uint8_t token[8], uint32_t& utc) {
  if (n < 48 || (p[0] >> 6) == 3 || (p[0] & 7) != 4) return false;
  const int version = (p[0] >> 3) & 7;
  if (version < 3 || version > 4 || !p[1] || p[1] > 15 || memcmp(p + 24, token, 8)) return false;
  uint64_t seconds = read32(p + 40);
  if (!seconds && !read32(p + 44)) return false;   // NTP zero timestamp means unknown
  if (seconds < 2208988800ull) seconds += 4294967296ull;   // NTP era rolls in February 2036
  const uint64_t unixSec = seconds - 2208988800ull;
  if (unixSec < MIN_UTC || unixSec >= MAX_UTC) return false;
  utc = (uint32_t)unixSec;
  return true;
}
}  // namespace netclock
