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
// Session defaults: play 15 min, then rest 10. Parent-configured session limits apply even with one player.
// Accepted: a rest that begins mid-minigame closes the game there (its save is flushed, the round is lost).
constexpr uint32_t SESSION_SEC = 15 * 60, REST_SEC = 10 * 60;
// The daily cap: 30 min per profile per local day, across every game, even alone on the device. The clock is local
// wall-clock seconds, so a day is now / DAY_SEC and it ends at local midnight.
constexpr uint32_t DAY_SEC = 86400, DAILY_SEC = 30 * 60;
// Device-wide parent settings in "sys"/"limits". Minutes, 1..1440; absent/invalid blobs use the shipped defaults.
// Independent of profile/game records: changing a limit never resets usage or moves a save.
struct Limits { uint16_t sessionMinutes = SESSION_SEC / 60, dailyMinutes = DAILY_SEC / 60; };
static_assert(sizeof(Limits) == 4, "sys/limits is two uint16 minute counts");
// No touch and no move of the board (ActivityTracker, os/input.h) for this long is not play (counts toward neither
// budget). Matches the backlight's first dimming step.
constexpr uint32_t IDLE_MS = 60000;
constexpr size_t BLOB_MAX = 2048;        // bounded per-profile buffer, including Kingdom's campaign snapshot

// Storage both platforms implement: firmware -> board NVS, host -> files, tests -> memory.
class Store {
 public:
  virtual size_t load(const char* ns, const char* key, void* buf, size_t max) = 0;   // 0 = missing or bigger than max
  virtual bool save(const char* ns, const char* key, const void* data, size_t len) = 0;   // false = it did not land
  virtual void erase(const char* ns, const char* key) = 0;

 protected:
  ~Store() = default;
};
Limits loadLimits(Store& st);
bool saveLimits(Store& st, const Limits& limits);   // validates and verifies persistence; caller updates RAM only on success

// Separate, versioned device schedule. Local play is allowed [startMinute, endMinute); an overnight
// interval is valid. Equal endpoints are rejected instead of hiding an ambiguous all-day/no-play setting.
struct PlayWindow {
  uint16_t version = 1, startMinute = 14 * 60, endMinute = 19 * 60 + 30;
  uint8_t enabled = 1, reserved = 0;
};
static_assert(sizeof(PlayWindow) == 8, "sys/window is a versioned independent blob");
PlayWindow loadPlayWindow(Store& st);
bool savePlayWindow(Store& st, const PlayWindow& window);
bool windowAllows(const PlayWindow& window, uint32_t now);
uint32_t windowWait(const PlayWindow& window, uint32_t now);
// A deliberate adult exception, in RAM only. Unsigned elapsed time handles millis() rollover; reboot clears it.
struct Allowance {
  uint32_t startedMs = 0; bool enabled = false;
  void start(uint32_t ms) { startedMs = ms; enabled = true; }
  void stop() { enabled = false; }
  void tick(uint32_t ms) { if (enabled && ms - startedMs >= 1800000u) stop(); }
  bool active(uint32_t ms) const { return enabled && ms - startedMs < 30u * 60u * 1000u; }
  uint32_t minutesLeft(uint32_t ms) const { return active(ms) ? (1800000u - (ms - startedMs) + 59999u) / 60000u : 0; }
};

// Persisted as a raw blob, append-only like Pets Club's Save: new fields go right before crc, bump the version,
// and teach loadRecord the older size. Never reorder or resize a field.
constexpr uint32_t REC_MAGIC = 0x50525450;   // "PTRP"
constexpr uint16_t REC_VERSION = 2;
constexpr size_t REC_V1_SIZE = 44;           // v1: no daily cap fields; loads with them zeroed
struct Record {
  uint32_t magic;
  uint16_t version, size;
  char name[12];
  uint8_t avatar, age, muted, reserved0;   // age 0 = not asked yet (profiles migrated from v1 Pets Club saves)
  uint16_t pin, reserved1;                 // 4-digit code + 1 (pinCode), 0 = none
  uint32_t restUntil, playSec, lastPlayed; // rest budget; lastPlayed also restores the clock when the RTC is lost
  uint32_t dayPlaySec, playDay;            // v2: play counted on local day playDay (now / DAY_SEC)
  uint32_t crc;
};
static_assert(sizeof(Record) == 52, "Record is persisted: append before crc, never resize");
static_assert(offsetof(Record, dayPlaySec) == REC_V1_SIZE - sizeof(uint32_t), "v2 fields start where v1's crc was");

struct Profiles {   // indexed by stable id (a migrated house keeps its slot number); a deleted id is reused
  Record rec[MAX_PROFILES];
  bool used[MAX_PROFILES];
  bool blocked[MAX_PROFILES];   // a record is there but will not load: not shown, and never handed to a new profile
  int count() const;
  int firstFree() const;  // the id a new profile gets, -1 when none is left
  int nth(int i) const;   // id of the i-th used profile, -1 past the end
};

const char* key(char prefix, int id);   // "p0", "s3" (static buffer)
void seal(Record& r);
bool loadRecord(const void* data, size_t n, Record& out);
// The stored form of a typed code ("0000".."9999"): value + 1, so 0000 is a real code; nullptr or "" = none.
inline uint16_t pinCode(const char* digits) { return digits && *digits ? (uint16_t)(atoi(digits) + 1) : 0; }
// porthole/p0..p3; migrates Pets Club houses until porthole/m exists, then Biscuit's old save until porthole/mb does.
void loadAll(Store& st, Profiles& p);
bool saveRecord(Store& st, Profiles& p, int id);   // false = the write failed
// name/avatar/age/pin from draft; erases any s<id> left in the app stores, and f<id>, first. The id, or -1 when full.
int  create(Store& st, Profiles& p, const Record& draft, const char* const* stores, int nStores);
// Every app's s<id> and f<id> first, the record last: power loss in between leaves a profile without saves, never
// saves waiting for whoever takes the id next.
void removeProfile(Store& st, Profiles& p, int id, const char* const* stores, int nStores);
Profile toProfile(const Profiles& p, int id);
void migrate(Store& st, Profiles& p);   // migrate.cpp: Pets Club houses -> profiles; safe to re-run after power loss
// migrate.cpp: Biscuit's pre-Porthole save -> biscuit/s<id>, once (porthole/mb); safe to re-run after power loss.
void migrateBiscuit(Store& st, Profiles& p);

// Home and the Library. Every game has a stable id, 1..32 (APP_IDS in games/apps.h): a bit in a mask is bit id - 1.
// A kid's favorites are porthole/f<profile id>, append-only like Record; missing or invalid is the default (the first
// MAX_FAVS enabled games, in launcher order). A grown-up's games switched off are "sys"/"games"; missing is all on.
// An id this image does not know (a game that left the firmware) is skipped, never an error.
constexpr int MAX_FAVS = 5, MAX_GAME_ID = 32;
constexpr uint32_t gameBit(uint8_t id) { return id >= 1 && id <= MAX_GAME_ID ? 1u << (id - 1) : 0; }
constexpr bool validIds(const uint8_t* ids, int n) {   // unique, 1..MAX_GAME_ID: apps.h asserts it at compile time
  uint32_t seen = 0;
  for (int k = 0; k < n; k++) {
    if (!gameBit(ids[k]) || (seen & gameBit(ids[k]))) return false;
    seen |= gameBit(ids[k]);
  }
  return true;
}
constexpr uint32_t FAV_MAGIC = 0x56414650;   // "PFAV"
constexpr uint16_t FAV_VERSION = 1;
struct Favorites {
  uint32_t magic;
  uint16_t version, size;
  uint8_t count, picked;      // picked 0: the kid never starred anything, Home shows the default
  uint8_t ids[MAX_FAVS], reserved;   // Home's order: the newest star first
  uint32_t seen;              // games known when this was first written or opened since; the rest wear a "new" dot
  uint32_t crc;
};
static_assert(sizeof(Favorites) == 24, "porthole/f<id> is persisted: append before crc, never resize");
bool loadFavorites(Store& st, int profileId, Favorites& out);       // false: missing or invalid (out is the default)
bool saveFavorites(Store& st, int profileId, Favorites& f);         // seals; false unless it reads back
struct GamesOff { uint16_t version, size; uint32_t off, crc; };    // "sys"/"games"
static_assert(sizeof(GamesOff) == 12, "sys/games is persisted: append before crc, never resize");
uint32_t loadGamesOff(Store& st);                                    // 0 (all on) when missing or invalid
bool saveGamesOff(Store& st, uint32_t off);                          // false unless it reads back
// The image's games a kid sees (indexes into the app table, launcher order): never none, so a mask that switches
// off every game in the image (they all left but the switched-off ones) counts as all on.
int enabledGames(const uint8_t* ids, int n, uint32_t off, int* out);
// Home: every enabled game while they fit one page (no Library then), else the kid's favorites that are enabled, the
// newest star first (the default when none were ever picked). The Library has the rest; true from libraryOn().
int homeGames(const Favorites& f, const uint8_t* ids, int n, uint32_t off, int* out);
inline bool libraryOn(const uint8_t* ids, int n, uint32_t off) { int all[MAX_GAME_ID]; return enabledGames(ids, n, off, all) > MAX_FAVS + 1; }
// Star mode's tap on game id: unstars it, or stars it first on Home. False (and nothing changes) for a sixth
// star. A switched-off or unknown favorite is kept hidden until its place is needed for a new star (the oldest such first).
bool toggleStar(Favorites& f, uint8_t id, const uint8_t* ids, int n, uint32_t off);

// Rest budget.
// Never more than REST_SEC from now: a clock set backwards must not strand a kid on the rest screen for hours.
inline bool resting(const Record& r, uint32_t now) { return r.restUntil > now && r.restUntil - now <= REST_SEC; }
// The daily cap is kept as (playDay, dayPlaySec), not as a restUntil: it ends at the next midnight by construction,
// and a clock set back to an earlier day lifts it instead of stranding anyone.
inline bool playedToday(const Record& r, uint32_t now, const Limits& limits = {}) {
  return r.playDay == now / DAY_SEC && r.dayPlaySec >= (uint32_t)limits.dailyMinutes * 60;
}
// Seconds until this profile may play again, 0 = now. Every player gets a break after their session.
inline uint32_t restLeft(const Record& r, uint32_t now, const Limits& limits = {}) {
  if (playedToday(r, now, limits)) return DAY_SEC - now % DAY_SEC;
  return resting(r, now) ? r.restUntil - now : 0;
}
// The child's prompt reflects both the budget release and the schedule at that time.
enum RestHint : uint8_t { REST_DAY, REST_WINDOW, REST_BREAK };
RestHint restHint(const Record& r, const PlayWindow& window, uint32_t now, const Limits& limits = {});
// The clock was guessed (RTC lost: the firmware resumes from the newest lastPlayed, which a capped kid never moves),
// so "today" cannot be trusted: lift every day's cap, like a clock set back. Saves only the records it changed.
void liftCaps(Store& st, Profiles& p);
// A grown-up's fresh start for everyone: clear daily usage, session usage and rest deadlines, saving changed
// records. Keep lastPlayed (clock recovery), profile details and every game's save intact. False if any write did
// not read back: that record keeps its old in-memory limits so the grown-up can retry (others may have reset).
bool resetLimits(Store& st, Profiles& p);
void recharge(Record& r, uint32_t now);   // a real break (REST_SEC away) refills the session budget
// Counts dt seconds of play (none while idle or resting); true when the profile must stop now (a rest began, or the
// day's play is used up).
// Count ordinary active play while an explicit allowance bypasses restrictions, including a pre-existing rest.
void playUnrestricted(Record& r, uint32_t now, uint32_t dt, bool idle);
// Apply a cap/session boundary after an allowance expires or settings are lowered (even on the launcher).
bool enforceBudget(Record& r, uint32_t now, const Limits& limits = {});
bool play(Record& r, uint32_t now, uint32_t dt, bool idle, const Limits& limits = {});
}  // namespace shell
