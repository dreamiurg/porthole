#include "timezone.h"
#include <ctype.h>
#include <initializer_list>
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

// A zone's recurring rule (the POSIX TZ text after its last listed transition) is evaluated here, not by the C library.
// The board's newlib and the host's glibc are two parsers, and the tests could only ever ask glibc: newlib reads a
// rule's time as unsigned hours, so a negative one (America/Nuuk's "M3.5.0/-1") cannot mean the same there. This reads
// exactly what tools/timezones.py emits: std offset [dst [offset] ,Mm.w.d[/time] ,Mm.w.d[/time]], names plain or <quoted>.
struct Edge { int month, week, day, time; };            // week 5 = the last; day 0 = Sunday; time: seconds, local
struct Rule { int standard, daylight; bool changes; Edge start, end; };   // offsets: seconds east of UTC
const char* skipName(const char* s) {
  if (*s == '<') { while (*s && *s != '>') s++; return *s ? s + 1 : s; }
  while (isalpha((unsigned char)*s)) s++;
  return s;
}
const char* clockTime(const char* s, int& seconds) {   // [+-]h[:mm[:ss]]
  const int sign = *s == '-' ? -1 : 1;
  if (*s == '-' || *s == '+') s++;
  int part[3] = {0, 0, 0};
  for (int i = 0; i < 3; i++) {
    while (isdigit((unsigned char)*s)) part[i] = part[i] * 10 + (*s++ - '0');
    if (*s != ':') break;
    s++;
  }
  seconds = sign * (part[0] * 3600 + part[1] * 60 + part[2]);
  return s;
}
bool parseRule(const char* s, Rule& rule) {
  int west = 0;
  s = clockTime(skipName(s), west);
  rule.standard = rule.daylight = -west; rule.changes = *s != 0;
  if (!rule.changes) return true;
  s = skipName(s); rule.daylight = rule.standard + 3600;
  if (*s != ',') { s = clockTime(s, west); rule.daylight = -west; }
  for (Edge* edge : {&rule.start, &rule.end}) {
    int used = 0;
    if (sscanf(s, ",M%d.%d.%d%n", &edge->month, &edge->week, &edge->day, &used) != 3) return false;
    s += used; edge->time = 7200;
    if (*s == '/') s = clockTime(s + 1, edge->time);
    if (edge->month < 1 || edge->month > 12 || edge->week < 1 || edge->week > 5 || edge->day < 0 || edge->day > 6) return false;
  }
  return *s == 0;
}
int64_t daysFromCivil(int y, int m, int d) {   // Howard Hinnant's algorithm; y >= 0 here
  y -= m <= 2;
  const int era = y / 400, yoe = y - era * 400, doy = (153 * (m + (m > 2 ? -3 : 9)) + 2) / 5 + d - 1;
  return (int64_t)era * 146097 + yoe * 365 + yoe / 4 - yoe / 100 + doy - 719468;
}
// The UTC second at which an edge happens in year y; its time is on the clock in force before it (offset, seconds east).
int64_t edgeUtc(const Edge& edge, int y, int offset) {
  const int64_t first = daysFromCivil(y, edge.month, 1);
  const int length = (int)(daysFromCivil(edge.month == 12 ? y + 1 : y, edge.month % 12 + 1, 1) - first);
  const int weekday = (int)((first + 4) % 7);   // 1970-01-01 was a Thursday
  int day = (edge.day - weekday + 7) % 7 + (edge.week - 1) * 7;   // days after the 1st
  if (day >= length) day -= 7;
  return (first + day) * 86400 + edge.time - offset;
}
int ruleOffsetMinutes(const char* text, uint32_t utc) {
  Rule rule{};
  if (!parseRule(text, rule)) return 0;
  if (!rule.changes) return rule.standard / 60;
  const time_t standardWall = (time_t)((int64_t)utc + rule.standard); tm date{};
  if (!gmtime_r(&standardWall, &date)) return rule.standard / 60;   // gmtime_r never looks at TZ
  const int64_t start = edgeUtc(rule.start, date.tm_year + 1900, rule.standard);
  const int64_t end = edgeUtc(rule.end, date.tm_year + 1900, rule.daylight);
  // Northern rules start before they end within a year; southern ones (and Ireland's winter time) wrap the new year.
  const bool daylight = start < end ? utc >= start && utc < end : utc >= start || utc < end;
  return (daylight ? rule.daylight : rule.standard) / 60;
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
  if (!z) return false;
  g_selected = (int)(z - ZONES);
  return true;
}
int16_t offset(uint32_t utc) {
  const KnownRange& range = RANGES[RANGE_INDEX[g_selected]];
  if (utc >= netclock::MIN_UTC && utc <= range.through) {
    int low = range.start, high = range.start + range.count;
    while (low + 1 < high) { const int mid = (low + high) / 2; if (transitionUtc(mid) <= utc) low = mid; else high = mid; }
    return transitionOffset(low);
  }
  return (int16_t)ruleOffsetMinutes(ZONES[g_selected].rule, utc);
}
uint32_t local(uint32_t utc) { return netclock::local(utc, offset(utc)); }
uint32_t toUtc(uint32_t localEpoch) {
  // Searched against the known transitions too, including finite rules such as Ramadan which POSIX cannot express.
  // The earliest matching UTC instant makes a repeated fall-back local hour deterministic.
  for (int minutes = 840; minutes >= -720; minutes -= 15) {
    const int64_t candidate = (int64_t)localEpoch - minutes * 60;
    if (candidate >= 0 && candidate <= UINT32_MAX && local((uint32_t)candidate) == localEpoch) return (uint32_t)candidate;
  }
  // No instant shows this wall time: it is in the hour a spring-forward skips. Read it on the clock in force before
  // the gap (a day earlier is clear of it), so 02:30 on such a night means 03:30.
  return netclock::local(localEpoch, -offset(localEpoch >= 86400 ? localEpoch - 86400 : 0));
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
