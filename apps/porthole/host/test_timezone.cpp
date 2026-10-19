#include <assert.h>
#include <stdio.h>
#include <string.h>
#include <time.h>
#include <initializer_list>
#include "timezone.h"

static uint32_t epoch(int year, int month, int day, int hour = 0, int minute = 0) {
  tm value{}; value.tm_year = year - 1900; value.tm_mon = month - 1; value.tm_mday = day;
  value.tm_hour = hour; value.tm_min = minute; value.tm_sec = 0;
  return (uint32_t)timegm(&value);
}
static void dst() {
  assert(timezones::select("America/Los_Angeles"));
  const uint32_t spring = epoch(2026, 3, 8, 10), fall = epoch(2026, 11, 1, 9);
  assert(timezones::offset(spring - 1) == -480 && timezones::offset(spring) == -420);
  assert(timezones::local(spring - 1) == (epoch(2026, 3, 8, 1, 59) + 59));
  assert(timezones::local(spring) == epoch(2026, 3, 8, 3));
  assert(timezones::offset(fall - 1) == -420 && timezones::offset(fall) == -480);
  assert(timezones::local(fall - 1) == (epoch(2026, 11, 1, 1, 59) + 59));
  assert(timezones::local(fall) == epoch(2026, 11, 1, 1));
  assert(timezones::local(epoch(2028, 2, 29, 12)) == epoch(2028, 2, 29, 4));
  assert(timezones::toUtc(epoch(2026, 1, 2, 4)) == epoch(2026, 1, 2, 12));
  assert(timezones::toUtc(epoch(2026, 7, 2, 4)) == epoch(2026, 7, 2, 11));
  for (const char* zone : {"Africa/Casablanca", "Africa/El_Aaiun"}) {
    assert(timezones::select(zone));
    assert(timezones::offset(epoch(2026, 3, 1, 16)) == 0);   // Ramadan: finite IANA transitions, not the fixed POSIX tail
    assert(timezones::toUtc(epoch(2026, 3, 1, 16)) == epoch(2026, 3, 1, 16));
    assert(timezones::offset((epoch(2026, 2, 15, 1, 59) + 59)) == 60);
    assert(timezones::offset(epoch(2026, 2, 15, 2)) == 0);
    assert(timezones::offset((epoch(2026, 3, 22, 1, 59) + 59)) == 0);
    assert(timezones::offset(epoch(2026, 3, 22, 2)) == 60);
  }
  assert(timezones::select("Australia/Sydney"));
  assert(timezones::offset(epoch(2026, 1, 1)) == 660 && timezones::offset(epoch(2026, 7, 1)) == 600);
  assert(timezones::select("Australia/Lord_Howe"));
  assert(timezones::offset(epoch(2026, 1, 1)) == 660 && timezones::offset(epoch(2026, 7, 1)) == 630);
  assert(timezones::select("Asia/Kathmandu")); assert(timezones::offset(spring) == 345);
  assert(timezones::select("Pacific/Chatham")); assert(timezones::offset(spring) == 825);
  assert(timezones::select("Pacific/Kiritimati"));
  assert(timezones::local(epoch(2025, 12, 31, 12)) == epoch(2026, 1, 1, 2));
}
static void catalog() {
  assert(timezones::count() > 400);
  const timezones::Zone* pacific = timezones::find("US/Pacific");
  assert(pacific && !strcmp(pacific->id, timezones::DEFAULT_ZONE));
  assert(timezones::find("UTC") && timezones::find("Asia/Calcutta"));
  assert(!timezones::find("America/Made_Up") && !timezones::find(nullptr));
  assert(!timezones::select("../../etc/passwd"));
  bool label = false, name = false;
  for (int i = 0; i < timezones::count(); i++) {
    const auto& z = timezones::at(i);
    assert(timezones::find(z.id) == &z && timezones::select(z.id));
    const int jan = timezones::offset(epoch(2026, 1, 1)), jul = timezones::offset(epoch(2026, 7, 1));
    assert(jan >= -720 && jan <= 840 && jul >= -720 && jul <= 840);
    if (!strcmp(z.id, timezones::DEFAULT_ZONE)) {
      label = timezones::matches(i, "pacific"); name = timezones::matches(i, "LOS_ANGELES");
    }
  }
  assert(label && name && !timezones::matches(-1, "") && !timezones::matches(0, nullptr));
}
static void providerReplies() {
  char zone[64] = {};
  assert(timezones::detectedZone("America/Los_Angeles\n", 20, zone) && !strcmp(zone, timezones::DEFAULT_ZONE));
  assert(timezones::detectedZone("  US/Pacific\r\n", 14, zone) && !strcmp(zone, timezones::DEFAULT_ZONE));
  assert(!timezones::detectedZone(nullptr, 1, zone));
  assert(!timezones::detectedZone("", 0, zone));
  assert(!timezones::detectedZone("error", 5, zone));
  assert(!timezones::detectedZone("Undefined", 9, zone));
  assert(!timezones::detectedZone("{\"error\":true}", 14, zone));
  assert(!timezones::detectedZone("Europe/Invalid", 14, zone));
  char oversized[100]; memset(oversized, 'A', sizeof oversized);
  assert(!timezones::detectedZone(oversized, sizeof oversized, zone));
  const char embedded[] = "Etc/UTC\0garbage";
  assert(!timezones::detectedZone(embedded, sizeof embedded - 1, zone));
}
static void persistence() {
  auto settings = timezones::settings("US/Pacific", true);
  assert(timezones::validSettings(settings) && !strcmp(settings.zone, timezones::DEFAULT_ZONE) && settings.automatic);
  settings.zone[2] ^= 1; assert(!timezones::validSettings(settings));
  settings = timezones::settings("Europe/London", false);
  assert(timezones::validSettings(settings) && !settings.automatic);
  memset(settings.zone, 'X', sizeof settings.zone); assert(!timezones::validSettings(settings));
  const uint32_t wall = epoch(2026, 7, 2, 4), utc = epoch(2026, 7, 2, 11);
  assert(timezones::select(timezones::DEFAULT_ZONE));
  assert(timezones::legacyUtc(wall, true, -420) == utc);
  assert(timezones::legacyUtc(wall, false, 0) == wall);
  assert(timezones::fallbackUtc(wall, 0, false, 0) == wall);   // first migration: old default is UTC
  assert(timezones::fallbackUtc(wall, 0, true, -420) == utc);   // first migration: honor explicit old offset
  assert(timezones::fallbackUtc(wall, utc, false, 0) == utc);   // committed journal is UTC, regardless of mixed profile timestamps
  assert(timezones::fallbackUtc(wall + 86400, utc, true, 345) == utc);   // stale/changed-zone profile cannot shift UTC checkpoint
  assert(timezones::fallbackUtc(wall, 1, false, 0) == wall);   // invalid checkpoint is never trusted
  assert(timezones::legacyUtc(wall, true, 32000) == wall);   // missing/corrupt legacy offset preserves shipped UTC default
  auto prepared = timezones::journal(wall, utc, false); uint32_t recovered = 0;
  assert(timezones::validJournal(prepared));
  assert(timezones::recoverRtc(wall + 10, prepared, recovered) && recovered == utc + 10);   // before RTC write
  assert(timezones::recoverRtc(utc + 10, prepared, recovered) && recovered == utc + 10);   // after RTC write
  assert(!timezones::recoverRtc(utc + 20000, prepared, recovered));   // interrupted long enough to be ambiguous
  auto committed = timezones::journal(wall, utc, true);
  assert(timezones::recoverRtc(utc + 86400, committed, recovered) && recovered == utc + 86400);
  prepared.target ^= 1; assert(!timezones::recoverRtc(utc, prepared, recovered));
}
int main() {
  catalog(); dst(); providerReplies(); persistence();
  puts("timezone: canonical names, aliases, DST without a network, fractional offsets, leap day and migration recovery");
}
