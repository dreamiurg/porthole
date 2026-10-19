#include "timezone.h"
#include <ctype.h>
#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <time.h>
#include "crc32.h"
#include "netclock.h"
#include "generated/timezones.h"

namespace timezones {
namespace {
int g_selected = 0;
uint32_t transitionUtc(int index) { return netclock::MIN_UTC + (TRANSITIONS[index] >> 7) * 900u; }
int16_t transitionOffset(int index) { return (int16_t)(((int)(TRANSITIONS[index] & 127u) - 64) * 15); }
constexpr uint32_t SETTINGS_MAGIC = 0x315A5450u, JOURNAL_MAGIC = 0x31435452u;
bool contains(const char* text, const char* query) {
  if (!*query) return true;
  for (; *text; text++) {
    const char* a = text; const char* b = query;
    while (*a && *b && tolower((unsigned char)*a) == tolower((unsigned char)*b)) { a++; b++; }
    if (!*b) return true;
  }
  return false;
}
}  // namespace
int count() { return (int)(sizeof ZONES / sizeof ZONES[0]); }
const Zone& at(int index) { return ZONES[index >= 0 && index < count() ? index : 0]; }
const Zone* find(const char* id) {
  if (!id) return nullptr;
  for (const Zone& z : ZONES) if (!strcmp(id, z.id)) return &z;
  for (const Alias& a : ALIASES) if (!strcmp(id, a.id)) return &ZONES[a.canonical];
  return nullptr;
}
bool detectedZone(const char* body, size_t length, char (&out)[64]) {
  if (!body || !length || length >= 64) return false;
  while (length && isspace((unsigned char)body[length - 1])) length--;
  while (length && isspace((unsigned char)*body)) { body++; length--; }
  if (!length || memchr(body, 0, length)) return false;
  char id[64] = {}; memcpy(id, body, length);
  const Zone* zone = find(id);
  if (!zone) return false;
  snprintf(out, sizeof out, "%s", zone->id); return true;
}
bool matches(int index, const char* query) {
  if (index < 0 || index >= count() || !query) return false;
  const Zone& z = ZONES[index];
  return contains(z.id, query) || contains(z.label, query);
}
bool select(const char* id) {
  const Zone* z = find(id);
  if (!z || setenv("TZ", z->rule, 1)) return false;
  tzset(); g_selected = (int)(z - ZONES);
  return true;
}
int16_t offset(uint32_t utc) {
  const KnownRange& range = RANGES[RANGE_INDEX[g_selected]];
  if (utc >= netclock::MIN_UTC && utc <= range.through) {
    int low = range.start, high = range.start + range.count;
    while (low + 1 < high) { const int mid = (low + high) / 2; if (transitionUtc(mid) <= utc) low = mid; else high = mid; }
    return transitionOffset(low);
  }
  const time_t seconds = (time_t)utc; tm localTime{}, utcTime{};
  if (!localtime_r(&seconds, &localTime) || !gmtime_r(&seconds, &utcTime)) return 0;
  const int days = localTime.tm_year > utcTime.tm_year ? 1 : localTime.tm_year < utcTime.tm_year ? -1 :
                   localTime.tm_yday - utcTime.tm_yday;
  return (int16_t)(days * 1440 + (localTime.tm_hour - utcTime.tm_hour) * 60 + localTime.tm_min - utcTime.tm_min);
}
uint32_t local(uint32_t utc) { return netclock::local(utc, offset(utc)); }
uint32_t toUtc(uint32_t localEpoch) {
  const time_t wall = (time_t)localEpoch; tm broken{};
  if (!gmtime_r(&wall, &broken)) return localEpoch;
  broken.tm_isdst = -1;
  const time_t result = mktime(&broken);
  // Validate against known transitions, including finite rules such as Ramadan which POSIX cannot express.
  // The earliest matching UTC instant makes a repeated fall-back local hour deterministic.
  for (int minutes = 840; minutes >= -720; minutes -= 15) {
    const int64_t candidate = (int64_t)localEpoch - minutes * 60;
    if (candidate >= netclock::MIN_UTC && candidate < netclock::MAX_UTC && local((uint32_t)candidate) == localEpoch)
      return (uint32_t)candidate;
  }
  return result >= 0 && (uint64_t)result <= UINT32_MAX ? (uint32_t)result : localEpoch;
}
Settings settings(const char* id, bool automatic, bool detected) {
  Settings value{}; const Zone* z = find(id);
  value.magic = SETTINGS_MAGIC; value.automatic = automatic ? 1 : 0;
  value.reserved[0] = automatic && detected ? 1 : 0;
  snprintf(value.zone, sizeof value.zone, "%s", z ? z->id : DEFAULT_ZONE);
  value.crc = os::crc32(&value, offsetof(Settings, crc));
  return value;
}
bool validSettings(const Settings& value) {
  return value.magic == SETTINGS_MAGIC && value.automatic <= 1 && value.reserved[0] <= 1 &&
         !value.reserved[1] && !value.reserved[2] && memchr(value.zone, 0, sizeof value.zone) &&
         value.crc == os::crc32(&value, offsetof(Settings, crc)) && find(value.zone);
}
bool DetectionRetry::afterSync(uint32_t sequence, uint32_t ms, bool failedAutomatic) {
  // Latch expiry even without a new network sample; a later millis wrap cannot revive an old cooldown.
  if (cooling_ && ms - last_ >= PERIOD_MS) cooling_ = false;
  if (sequence == seen_) return false;
  seen_ = sequence;
  if (!failedAutomatic || cooling_) return false;
  cooling_ = true; last_ = ms;
  return true;
}
RtcJournal journal(uint32_t source, uint32_t target, bool committed) {
  RtcJournal value{};
  value.magic = JOURNAL_MAGIC; value.source = source; value.target = target; value.committed = committed ? 1 : 0;
  value.crc = os::crc32(&value, offsetof(RtcJournal, crc));
  return value;
}
bool validJournal(const RtcJournal& value) {
  return value.magic == JOURNAL_MAGIC && value.committed <= 1 &&
         value.crc == os::crc32(&value, offsetof(RtcJournal, crc)) &&
         value.source >= 1577836800u && value.target >= 1577836800u && value.target < netclock::MAX_UTC;
}
bool recoverRtc(uint32_t raw, const RtcJournal& value, uint32_t& utc) {
  if (!validJournal(value) || raw < 1577836800u || raw >= netclock::MAX_UTC) return false;
  if (value.committed) { utc = raw; return true; }
  if (raw >= value.target && raw - value.target < 120) { utc = raw; return true; }
  if (raw >= value.source && raw - value.source < 120) {
    utc = (uint32_t)((int64_t)raw + value.target - value.source); return true;
  }
  return false;
}
uint32_t fallbackUtc(uint32_t legacyWall, uint32_t utcCheckpoint, bool hasOffset, int16_t offsetMin) {
  if (utcCheckpoint >= 1577836800u && utcCheckpoint < netclock::MAX_UTC) return utcCheckpoint;
  return legacyUtc(legacyWall, hasOffset, offsetMin);
}
uint32_t legacyUtc(uint32_t wall, bool hasOffset, int16_t offsetMin) {
  return hasOffset && netclock::validOffset(offsetMin) ? netclock::local(wall, -offsetMin) : wall;
}
}  // namespace timezones
