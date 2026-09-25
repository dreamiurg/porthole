// Profiles: one small record per kid, persisted as porthole/p<id>, plus the per-profile rest budget.
// Pure logic over a Store; no rendering, no platform calls. host/test_shell.cpp exercises it directly.
#pragma once
#include <stddef.h>
#include <stdint.h>
#include <stdlib.h>
#include "app.h"

namespace shell {
constexpr const char* NS = "porthole";   // never rename: it orphans every profile on a device
constexpr const char* MIGRATED = "m";    // porthole/m: present once the Pets Club houses became profiles
constexpr const char* MIGRATED_BISCUIT = "mb";   // porthole/mb: present once Biscuit's old save found its profile
// Turn taking, same numbers Pets Club used: play 6 min, then rest 10. Only with 2+ profiles on the device.
// Accepted: a rest that begins mid-minigame closes the game there (its save is flushed, the round is lost).
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
  uint16_t pin, reserved1;                 // 4-digit code + 1 (pinCode), 0 = none
  uint32_t restUntil, playSec, lastPlayed; // rest budget; lastPlayed also restores the clock when the RTC is lost
  uint32_t crc;
};
static_assert(sizeof(Record) == 44, "Record is persisted: append before crc, never resize");

struct Profiles {   // indexed by stable id (a migrated house keeps its slot number); a deleted id is reused
  Record rec[MAX_PROFILES];
  bool used[MAX_PROFILES];
  int count() const;
  int nth(int i) const;   // id of the i-th used profile, -1 past the end
};

const char* key(char prefix, int id);   // "p0", "s3" (static buffer)
void seal(Record& r);
bool loadRecord(const void* data, size_t n, Record& out);
// The stored form of a typed code ("0000".."9999"): value + 1, so 0000 is a real code; nullptr or "" = none.
inline uint16_t pinCode(const char* digits) { return digits && *digits ? (uint16_t)(atoi(digits) + 1) : 0; }
void loadAll(Store& st, Profiles& p);   // porthole/p0..p3; migrates Pets Club houses until porthole/m exists
void saveRecord(Store& st, Profiles& p, int id);
// name/avatar/age/pin from draft; erases any s<id> left in the app stores first. The id, or -1 when full.
int  create(Store& st, Profiles& p, const Record& draft, const char* const* stores, int nStores);
// Every app's s<id> first, the record last: power loss in between leaves a profile without saves, never saves
// waiting for whoever takes the id next.
void removeProfile(Store& st, Profiles& p, int id, const char* const* stores, int nStores);
Profile toProfile(const Profiles& p, int id);
void migrate(Store& st, Profiles& p);   // migrate.cpp: Pets Club houses -> profiles; safe to re-run after power loss
// migrate.cpp: Biscuit's pre-Porthole save -> biscuit/s<id>, once (porthole/mb); safe to re-run after power loss.
// TODO(biscuit screens PR): call it at the end of loadAll (after migrate: it matches the migrated names) in the
// same change that adds Biscuit to APPS. Before that no game reads biscuit/s<id>, and profile delete, create and
// serial R do not erase it.
void migrateBiscuit(Store& st, Profiles& p);

// Rest budget.
// Never more than REST_SEC from now: a clock set backwards must not strand a kid on the rest screen for hours.
inline bool resting(const Record& r, uint32_t now) { return r.restUntil > now && r.restUntil - now <= REST_SEC; }
void recharge(Record& r, uint32_t now);                         // a real break (REST_SEC away) refills the budget
bool play(Record& r, uint32_t now, uint32_t dt, int nProfiles);  // counts play time; true when a rest just began
}  // namespace shell
