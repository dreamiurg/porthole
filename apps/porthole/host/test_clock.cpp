// Network samples must be fresh, synchronized, plausible and converted to the selected local offset.
#include <assert.h>
#include <stdio.h>
#include <initializer_list>
#include "netclock.h"
#include "timezone.h"

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
static void automaticRetries() {
  timezones::DetectionRetry offline;
  offline.completed(0);   // an offline join produced no successful UTC connection
  assert(!offline.afterSync(0, 1000, true));
  assert(offline.afterSync(1, 2000, true));   // Wi-Fi returns and NTP succeeds: one automatic retry
  assert(!offline.afterSync(1, 3000, true));   // the same sample cannot launch another request
  assert(!offline.afterSync(2, 3600000, true));
  assert(!offline.afterSync(3, timezones::DetectionRetry::PERIOD_MS + 1999, true));
  assert(offline.afterSync(4, timezones::DetectionRetry::PERIOD_MS + 2000, true));
  timezones::DetectionRetry provider;
  provider.completed(7);   // this NTP sample powered the failed provider lookup
  assert(!provider.afterSync(7, 10, true));
  assert(provider.afterSync(8, 20, true));   // only a later connection may retry
  assert(!provider.afterSync(9, 86400000, false));   // manual selection or a successful cached guess is authoritative
  timezones::DetectionRetry wrapping;
  const uint32_t start = UINT32_MAX - 1000u;
  const uint32_t next = start + timezones::DetectionRetry::PERIOD_MS;
  assert(wrapping.afterSync(1, start, true));
  assert(!wrapping.afterSync(2, next - 1, true));
  assert(wrapping.afterSync(3, next, true));   // cooldown spans millis wrap
  timezones::DetectionRetry longOffline;
  assert(longOffline.afterSync(1, 1000, true));
  assert(!longOffline.afterSync(1, 1000 + timezones::DetectionRetry::PERIOD_MS, true));   // no new sample, but expiry latches
  assert(longOffline.afterSync(2, UINT32_MAX - 10, true));   // much later successful connection, beyond signed-deadline horizon
  assert(!longOffline.afterSync(2, 10 + timezones::DetectionRetry::PERIOD_MS, true));
  assert(longOffline.afterSync(3, 20, true));   // expired cooldown cannot revive at a wrapped millis position
  const auto pending = timezones::settings("America/Los_Angeles", true);
  const auto cached = timezones::settings("Europe/London", true, true);
  const auto manual = timezones::settings("Europe/London", false, true);
  assert(timezones::validSettings(pending) && !pending.reserved[0]);
  assert(timezones::validSettings(cached) && cached.reserved[0]);
  assert(timezones::validSettings(manual) && !manual.reserved[0] && !manual.automatic);
}
int main() {
  for (int offset = -750; offset <= 870; offset++)
    assert(netclock::validOffset(offset) == (offset >= -720 && offset <= 840 && offset % 15 == 0));
  assert(netclock::local(1792368000u, -720) == 1792324800u);
  assert(netclock::local(1792368000u, 840) == 1792418400u);
  assert(netclock::local(1792368000u, 345) == 1792388700u);
  assert(netclock::advanced(1792368000u, 1000, 6799) == 1792368005u);
  assert(netclock::advanced(1792368000u, UINT32_MAX - 499, 2500) == 1792368003u);
  responses(); automaticRetries();
  puts("clock: fresh NTP, era rollover, elapsed time and bounded automatic-zone retries after reconnect");
}
