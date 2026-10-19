// Network samples must be fresh, synchronized, plausible and converted to the selected local offset.
#include <assert.h>
#include <stdio.h>
#include <initializer_list>
#include "netclock.h"

static void timestamp(uint8_t packet[48], uint32_t utc) {
  const uint32_t ntp = utc + 2208988800u;
  for (int i = 0; i < 4; i++) packet[40 + i] = (uint8_t)(ntp >> (24 - i * 8));
}
static void responses() {
  uint8_t p[48] = {}, token[8] = {1, 7, 9, 11, 12, 13, 14, 15}; uint32_t utc = 123;
  p[0] = 0x24; p[1] = 2; memcpy(p + 24, token, 8); timestamp(p, 1792368000u);
  assert(netclock::reply(p, sizeof p, token, utc) && utc == 1792368000u);
  for (size_t n = 0; n < 48; n++) assert(!netclock::reply(p, n, token, utc));
  p[24] ^= 1; assert(!netclock::reply(p, sizeof p, token, utc)); p[24] ^= 1;
  for (uint8_t invalid : {uint8_t(0xe4), uint8_t(0x23), uint8_t(0x14), uint8_t(0x2c)}) {
    p[0] = invalid; assert(!netclock::reply(p, sizeof p, token, utc));
  }
  p[0] = 0x1c; assert(netclock::reply(p, sizeof p, token, utc));   // NTP v3 server
  p[0] = 0x24;
  for (int stratum : {0, 16, 255}) { p[1] = (uint8_t)stratum; assert(!netclock::reply(p, sizeof p, token, utc)); }
  p[1] = 15;
  for (uint32_t invalid : {netclock::MIN_UTC - 1, 2085978496u, netclock::MAX_UTC}) {
    timestamp(p, invalid); assert(!netclock::reply(p, sizeof p, token, utc));
  }
  for (uint32_t valid : {netclock::MIN_UTC, 2085978495u, 2085978497u, netclock::MAX_UTC - 1}) {
    timestamp(p, valid); assert(netclock::reply(p, sizeof p, token, utc) && utc == valid);
  }
}
int main() {
  for (int offset = -750; offset <= 870; offset++)
    assert(netclock::validOffset(offset) == (offset >= -720 && offset <= 840 && offset % 15 == 0));
  assert(netclock::local(1792368000u, -720) == 1792324800u);
  assert(netclock::local(1792368000u, 840) == 1792418400u);
  assert(netclock::local(1792368000u, 345) == 1792388700u);
  assert(netclock::advanced(1792368000u, 1000, 6799) == 1792368005u);
  assert(netclock::advanced(1792368000u, UINT32_MAX - 499, 2500) == 1792368003u);
  responses();
  puts("clock: validated fresh replies, NTP era rollover, UTC offsets and sample elapsed time");
}
