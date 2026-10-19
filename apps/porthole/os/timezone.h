// Bundled IANA names and current POSIX daylight-saving rules. No network is needed to keep local time.
#pragma once
#include <stddef.h>
#include <stdint.h>
namespace timezones {
constexpr const char* DEFAULT_ZONE = "America/Los_Angeles";
struct Zone { const char* id; const char* label; const char* rule; };
int count();
const Zone& at(int index);
const Zone* find(const char* id);   // canonicalizes bundled IANA aliases; nullptr for unsupported/invalid names
bool detectedZone(const char* body, size_t length, char (&out)[64]);   // bounded plain-text provider reply, no TZ mutation
bool matches(int index, const char* query);   // case-insensitive ID or friendly-label search
// The following call the C library's process TZ conversion: main core only, never a radio worker.
bool select(const char* id);
uint32_t local(uint32_t utc);
uint32_t toUtc(uint32_t localEpoch);
int16_t offset(uint32_t utc);
}  // namespace timezones

namespace timezones {
// New independent NVS blobs; existing profile/game/legacy sys/utc_off records are untouched.
struct Settings { uint32_t magic; char zone[64]; uint8_t automatic; uint8_t reserved[3]; uint32_t crc; };
static_assert(sizeof(Settings) == 76, "timezone settings layout");
Settings settings(const char* id, bool automatic, bool detected = false);
bool validSettings(const Settings& value);
// Main-core policy: only a genuinely later NTP success retries a failed automatic lookup. The sync that powered
// the failed lookup is consumed at completion, and automatic retries are limited to one per six hours.
class DetectionRetry {
 public:
  static constexpr uint32_t PERIOD_MS = 6 * 3600000u;
  void completed(uint32_t syncSequence) { seen_ = syncSequence; }
  bool afterSync(uint32_t syncSequence, uint32_t ms, bool failedAutomatic);
 private:
  uint32_t seen_ = 0, last_ = 0;
  bool cooling_ = false;
};
struct RtcJournal { uint32_t magic, source, target; uint8_t committed, reserved[3]; uint32_t crc; };
static_assert(sizeof(RtcJournal) == 20, "RTC journal layout");
RtcJournal journal(uint32_t source, uint32_t target, bool committed);
bool validJournal(const RtcJournal& value);
// A prepared journal recovers only when the RTC is close to one recorded side. Ambiguous long interruptions fail
// closed so the caller uses its conservative last-seen/build fallback, rather than converting the RTC twice.
bool recoverRtc(uint32_t raw, const RtcJournal& value, uint32_t& utc);
uint32_t fallbackUtc(uint32_t legacyWall, uint32_t utcCheckpoint, bool hasOffset, int16_t offsetMin);
uint32_t legacyUtc(uint32_t wall, bool hasOffset, int16_t offsetMin);
}  // namespace timezones
