// Profiles: one small record per kid, persisted as porthole/p<id>, plus the per-profile rest budget.
// Pure logic over a Store; no rendering, no platform calls. host/test_shell.cpp exercises it directly.
#pragma once
#include <stddef.h>
#include <stdint.h>
#include "app.h"

namespace shell {
constexpr const char* NS = "porthole";   // never rename: it orphans every profile on a device
// Turn taking, same numbers Pets Club used: play 6 min, then rest 10. Only with 2+ profiles on the device.
constexpr uint32_t SESSION_SEC = 6 * 60, REST_SEC = 10 * 60;
constexpr size_t BLOB_MAX = 256;         // biggest app save the shell loads (Pets Club's Save is 156 bytes)

// Storage both platforms implement: firmware -> board NVS, host -> files, tests -> memory.
class Store {
 public:
  virtual size_t load(const char* ns, const char* key, void* buf, size_t max) = 0;   // 0 = missing or bigger than max
  virtual void save(const char* ns, const char* key, const void* data, size_t len) = 0;
  virtual void erase(const char* ns, const char* key) = 0;

 protected:
  ~Store() = default;
};

// Persisted as a raw blob, append-only like Pets Club's Save: new fields go right before crc, bump the version,
// and teach loadRecord the older size. Never reorder or resize a field.
constexpr uint32_t REC_MAGIC = 0x50525450;   // "PTRP"
constexpr uint16_t REC_VERSION = 1;
struct Record {
  uint32_t magic;
  uint16_t version, size;
  char name[12];
  uint8_t avatar, age, muted, reserved0;   // age 0 = not asked yet (profiles migrated from v1 Pets Club saves)
  uint16_t pin, reserved1;                 // 4-digit code, 0 = none
  uint32_t restUntil, playSec, lastPlayed; // rest budget; lastPlayed also restores the clock when the RTC is lost
  uint32_t crc;
};

struct Profiles {   // indexed by stable id: a deleted id is reused by the next new profile
  Record rec[MAX_PROFILES];
  bool used[MAX_PROFILES];
  int count() const;
  int nth(int i) const;   // id of the i-th used profile, -1 past the end
};

const char* key(char prefix, int id);   // "p0", "s3" (static buffer)
void seal(Record& r);
bool loadRecord(const void* data, size_t n, Record& out);
void loadAll(Store& st, Profiles& p);   // porthole/p0..p3; migrates Pets Club houses when there are none
void saveRecord(Store& st, Profiles& p, int id);
int  create(Store& st, Profiles& p, const Record& draft);   // name/avatar/age/pin from draft; id, or -1 when full
void removeProfile(Store& st, Profiles& p, int id, const char* const* stores, int nStores);   // record + every app's s<id>
Profile toProfile(const Profiles& p, int id);
bool migrate(Store& st, Profiles& p);   // migrate.cpp: Pets Club houses -> profiles, once

// Rest budget.
inline bool resting(const Record& r, uint32_t now) { return r.restUntil > now; }
void recharge(Record& r, uint32_t now);                         // a real break (REST_SEC away) refills the budget
bool play(Record& r, uint32_t now, uint32_t dt, int nProfiles);  // counts play time; true when a rest just began
}  // namespace shell
