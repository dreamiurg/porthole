// Biscuit's rules: needs, visits, daily adventures, stickers, stories, discoveries and tricks. A mechanical port of
// the pet model Biscuit shipped with before Porthole: same numbers, same guards. Pure logic, no rendering, no
// platform calls; host/test_biscuit.cpp exercises it directly. Header-only like the model it came from (and so the
// shell's migration can use it without a second pet.cpp in the build).
// Everything is in namespace biscuit: Pets Club owns the global Save and namespace pet, and both link into one image.
// Time is Porthole's local wall-clock seconds (the RTC keeps local time), so a day is simply now / 86400.
#pragma once
#include <math.h>
#include <stddef.h>
#include <stdint.h>
#include <string.h>
#include "crc32.h"

namespace biscuit {
constexpr const char* STORE = "biscuit";   // NVS namespace, keys s<profile id>. Never rename it once it ships.
constexpr size_t NAME_LIMIT = 12;
constexpr uint32_t MAX_COUNT = 1000000;
constexpr int NUM_TRICKS = 6, NUM_STORIES = 7, NUM_DISCOVERIES = 96, NUM_STICKERS = 12;

// ---- calibration knobs (per real hour unless noted) ----
constexpr uint32_t ELAPSED_CAP_SEC = 8 * 3600;   // time away counts this much at most: a weekend off is not neglect
constexpr float NEED_FLOOR = 20, NEED_MAX = 100;
constexpr float FULLNESS_AWAKE = 4, FULLNESS_ASLEEP = 2, HAPPINESS_AWAKE = 2, ENERGY_AWAKE = 3, ENERGY_ASLEEP = 30;
constexpr float FEED_FULLNESS = 18, PLAY_HAPPINESS = 14, PLAY_ENERGY = 8, PET_HAPPINESS = 6, STORY_HAPPINESS = 10;
constexpr uint32_t FIRST_TODAY_FRIENDSHIP = 2, ADVENTURE_FRIENDSHIP = 4;
constexpr uint8_t TRICK_UNLOCK_DAY[NUM_TRICKS] = {1, 1, 2, 3, 5, 7};   // days together before a trick can be trained
// The daily adventure: which activities (Action bits) today asks for, a 7-day cycle. Completing it earns the sticker
// of a 12-day cycle.
constexpr uint8_t DAILY_MASKS[7] = {1 | 8 | 2, 16 | 4 | 32, 8 | 1 | 4, 2 | 16 | 8, 4 | 1 | 32, 16 | 8 | 32, 2 | 4 | 16};

enum class Action : uint8_t { Feed = 1, Play = 2, Petting = 4, Read = 8, Train = 16, Rest = 32 };
enum class Stage : uint8_t { Puppy, YoungPup, StoryDog };
enum class Cue : uint8_t { Left, Up, Right, Down, Paw };

// Persisted as a raw blob per profile (biscuit/s<id>, written by the shell). Append-only: new fields go right before
// crc (their zero must be a sensible default, loadBlob zero-fills them for older blobs), bump SAVE_VERSION, never
// reorder or resize a field. The kid's name and the daily play budget live in the shell's profile, not here.
constexpr uint32_t SAVE_MAGIC = 0x54435342;   // "BSCT"
constexpr uint16_t SAVE_VERSION = 1;
constexpr size_t SAVE_V1_SIZE = 96;
struct Save {
  uint32_t magic;
  uint16_t version, size;
  uint32_t createdAt, updatedAt;       // local seconds; updatedAt only moves backwards at the first naming
  int32_t lastVisitDay;                // the day the daily fields below belong to (also across a clock set back)
  float fullness, happiness, energy;   // NEED_FLOOR..NEED_MAX
  uint32_t friendship, daysTogether;   // daysTogether counts visits (days played), not calendar days
  uint32_t careCounts[3];              // Feed, Play, Petting
  uint32_t discoveries[3];             // NUM_DISCOVERIES bits
  uint16_t stickers;                   // NUM_STICKERS bits
  uint8_t stories;                     // NUM_STORIES bits: finished at least once
  uint8_t tricks[NUM_TRICKS];          // lessons passed, 3 = mastered
  uint8_t dailyCompleted, dailyClaimed, sleeping;
  char petName[NAME_LIMIT + 1];
  uint8_t named;                       // nothing counts (no decay, no visits, no progress) until the pup has a name
  uint8_t reserved[2];
  uint32_t crc;
};
static_assert(sizeof(Save) == SAVE_V1_SIZE, "Save is persisted: append before crc, never resize");

inline int32_t dayOf(uint32_t now) { return (int32_t)(now / 86400u); }
inline float clampNeed(float v) { return v < NEED_FLOOR ? NEED_FLOOR : v > NEED_MAX ? NEED_MAX : v; }
inline uint32_t addCount(uint32_t v, uint32_t n) { return v + n > MAX_COUNT ? MAX_COUNT : v + n; }

// ---- names ----
inline bool isLetter(char c) { return (c >= 'A' && c <= 'Z') || (c >= 'a' && c <= 'z'); }
inline bool isJoiner(char c) { return c == ' ' || c == '-' || c == '\''; }
// Trimmed, inner spaces collapsed; letters joined by single spaces, hyphens or apostrophes; 1..NAME_LIMIT chars.
// false (and out empty) for anything else.
inline bool normalizeName(const char* in, char (&out)[NAME_LIMIT + 1]) {
  memset(out, 0, sizeof out);
  size_t first = 0, last = strlen(in), n = 0;
  while (in[first] == ' ') first++;
  while (last > first && in[last - 1] == ' ') last--;
  bool letter = false;
  for (size_t i = first; i < last; i++) {
    const char c = in[i];
    if (isLetter(c)) letter = true;
    else if (c == ' ' && out[n - 1] == ' ') continue;   // n > 0: in[first] is not a space
    else if (isJoiner(c) && letter) letter = false;
    else { letter = false; break; }
    if (n == NAME_LIMIT) { letter = false; break; }
    out[n++] = c;
  }
  if (!letter) memset(out, 0, sizeof out);
  return letter;
}
// A stored name: terminated, and either empty or already normalized.
inline bool validName(const char (&name)[NAME_LIMIT + 1]) {
  if (!memchr(name, 0, sizeof name)) return false;
  char norm[NAME_LIMIT + 1];
  return !name[0] || (normalizeName(name, norm) && !strcmp(norm, name));
}

// ---- lessons and cycles ----
struct Lesson { Cue cues[6]; uint8_t length; };
inline uint8_t trickUnlockDay(uint8_t id) { return id < NUM_TRICKS ? TRICK_UNLOCK_DAY[id] : 255; }
inline Lesson lesson(uint8_t id, uint8_t practice) {
  using C = Cue;
  static constexpr Lesson LESSONS[NUM_TRICKS][3] = {
    {{{C::Paw, C::Down, C::Paw}, 3}, {{C::Up, C::Paw, C::Down, C::Paw}, 4}, {{C::Up, C::Paw, C::Left, C::Down, C::Paw}, 5}},
    {{{C::Left, C::Paw, C::Right}, 3}, {{C::Left, C::Paw, C::Right, C::Paw}, 4}, {{C::Left, C::Paw, C::Up, C::Right, C::Paw}, 5}},
    {{{C::Left, C::Up, C::Right}, 3}, {{C::Left, C::Up, C::Right, C::Down}, 4}, {{C::Left, C::Up, C::Right, C::Down, C::Left, C::Paw}, 6}},
    {{{C::Up, C::Down, C::Paw}, 3}, {{C::Up, C::Paw, C::Down, C::Paw}, 4}, {{C::Up, C::Left, C::Paw, C::Down, C::Right, C::Paw}, 6}},
    {{{C::Down, C::Up, C::Paw}, 3}, {{C::Down, C::Up, C::Up, C::Paw}, 4}, {{C::Left, C::Down, C::Up, C::Right, C::Up, C::Paw}, 6}},
    {{{C::Left, C::Down, C::Right}, 3}, {{C::Left, C::Down, C::Right, C::Up}, 4}, {{C::Paw, C::Left, C::Down, C::Right, C::Up, C::Paw}, 6}},
  };
  return id < NUM_TRICKS ? LESSONS[id][practice < 2 ? practice : 2] : Lesson{};
}
inline constexpr uint8_t cycle(int32_t day, uint8_t length) { return (uint8_t)((day % length + length) % length); }
inline constexpr uint8_t dailyMask(int32_t day) { return DAILY_MASKS[cycle(day, 7)]; }
inline constexpr uint8_t dailySticker(int32_t day) { return cycle(day, NUM_STICKERS); }

// ---- the companion ----
inline Save create(uint32_t now) {
  Save p{};
  p.createdAt = p.updatedAt = now;
  p.lastVisitDay = dayOf(now);
  p.fullness = 78; p.happiness = 86; p.energy = 80;
  p.daysTogether = 1;
  return p;
}

// Names the pup, or renames it. The first naming is when the companion starts: the time before it never decays
// needs or counts visits, and today's adventure starts fresh. A rename keeps everything but the name.
inline bool setPetName(Save& p, const char* dog, uint32_t now) {
  char name[NAME_LIMIT + 1];
  if (!normalizeName(dog, name)) return false;
  if (!p.named) {
    const int32_t day = dayOf(now);
    if (now < p.createdAt) p.createdAt = now;
    p.updatedAt = now;
    if (day > p.lastVisitDay) p.lastVisitDay = day;
    p.dailyCompleted = p.dailyClaimed = 0;
  }
  memcpy(p.petName, name, sizeof name);
  p.named = 1;
  return true;
}

// Advances needs to now (at most ELAPSED_CAP_SEC of it) and starts a new visit on a new day. A clock set backwards
// changes nothing: updatedAt and lastVisitDay only move forward.
inline void tick(Save& p, uint32_t now) {
  const int32_t day = dayOf(now);
  const uint32_t elapsed = now > p.updatedAt ? now - p.updatedAt : 0;
  if (now > p.updatedAt) p.updatedAt = now;
  if (!p.named) { if (day > p.lastVisitDay) p.lastVisitDay = day; return; }
  const float hours = (float)(elapsed < ELAPSED_CAP_SEC ? elapsed : ELAPSED_CAP_SEC) / 3600.0f;
  p.fullness = clampNeed(p.fullness - hours * (p.sleeping ? FULLNESS_ASLEEP : FULLNESS_AWAKE));
  p.happiness = clampNeed(p.happiness - hours * (p.sleeping ? 0 : HAPPINESS_AWAKE));
  p.energy = clampNeed(p.energy + hours * (p.sleeping ? ENERGY_ASLEEP : -ENERGY_AWAKE));
  if (day > p.lastVisitDay) {
    p.daysTogether = addCount(p.daysTogether, 1);
    p.lastVisitDay = day;
    p.dailyCompleted = p.dailyClaimed = 0;
  }
}

// An activity counts once a day toward friendship; doing all of today's adventure pays a bonus and its sticker.
inline void recordActivity(Save& p, Action action) {
  const uint8_t bit = (uint8_t)action, requested = dailyMask(p.lastVisitDay);
  const bool first = !(p.dailyCompleted & bit);
  p.dailyCompleted |= bit;
  const bool reward = !p.dailyClaimed && (p.dailyCompleted & requested) == requested;
  p.friendship = addCount(p.friendship, (first ? FIRST_TODAY_FRIENDSHIP : 0) + (reward ? ADVENTURE_FRIENDSHIP : 0));
  if (reward) {
    p.dailyClaimed = 1;
    p.stickers |= (uint16_t)(1u << dailySticker(p.lastVisitDay));
  }
}

inline bool awake(const Save& p) { return p.named && !p.sleeping; }   // the only state where care and progress count

inline void act(Save& p, Action action, uint32_t now) {   // Feed, Play or Petting; anything else is ignored
  tick(p, now);
  if (!awake(p)) return;
  unsigned index;
  switch (action) {
    case Action::Feed: index = 0; p.fullness = clampNeed(p.fullness + FEED_FULLNESS); break;
    case Action::Play: index = 1; p.happiness = clampNeed(p.happiness + PLAY_HAPPINESS); p.energy = clampNeed(p.energy - PLAY_ENERGY); break;
    case Action::Petting: index = 2; p.happiness = clampNeed(p.happiness + PET_HAPPINESS); break;
    default: return;
  }
  p.careCounts[index] = addCount(p.careCounts[index], 1);
  recordActivity(p, action);
}

inline void toggleSleep(Save& p, uint32_t now) {
  tick(p, now);
  if (!p.named) return;
  p.sleeping = !p.sleeping;
  if (p.sleeping) recordActivity(p, Action::Rest);
}

inline void recordReading(Save& p, uint32_t now) {
  tick(p, now);
  if (awake(p)) recordActivity(p, Action::Read);
}

inline uint8_t stars(const Save& p) {   // 3 per story finished
  uint8_t count = 0;
  for (int bit = 0; bit < NUM_STORIES; bit++) count += (p.stories >> bit) & 1;
  return (uint8_t)(count * 3);
}

// Reading always counts toward today; only the first finish of a story adds its star and the happiness.
inline void finishStory(Save& p, uint8_t id, uint32_t now) {
  if (id >= NUM_STORIES) return;
  recordReading(p, now);
  if (!awake(p) || (p.stories & (1u << id))) return;
  p.stories |= (uint8_t)(1u << id);
  p.happiness = clampNeed(p.happiness + STORY_HAPPINESS);
}

inline bool discovered(const Save& p, uint8_t id) {
  return id < NUM_DISCOVERIES && (p.discoveries[id / 32] & (1u << (id % 32)));
}
inline void discover(Save& p, uint8_t id, uint32_t now) {
  if (id >= NUM_DISCOVERIES) return;
  recordReading(p, now);
  if (awake(p)) p.discoveries[id / 32] |= 1u << (id % 32);
}

inline void practice(Save& p, uint8_t id, uint32_t now) {   // one lesson passed
  tick(p, now);
  if (id >= NUM_TRICKS || !awake(p) || p.daysTogether < trickUnlockDay(id)) return;
  if (p.tricks[id] < 3) p.tricks[id]++;
  recordActivity(p, Action::Train);
}

inline Stage stage(const Save& p) {
  if (p.daysTogether >= 7 && p.friendship >= 60) return Stage::StoryDog;
  if (p.daysTogether >= 3 && p.friendship >= 20) return Stage::YoungPup;
  return Stage::Puppy;
}

// ---- validity and persistence ----
inline bool validNeed(float v) { return isfinite(v) && v >= NEED_FLOOR && v <= NEED_MAX; }
inline bool validCounts(const Save& p) {
  if (p.friendship > MAX_COUNT || p.daysTogether < 1 || p.daysTogether > MAX_COUNT) return false;
  for (uint32_t c : p.careCounts) if (c > MAX_COUNT) return false;
  for (uint8_t t : p.tricks) if (t > 3) return false;
  return true;
}
inline bool validFlags(const Save& p) {
  return !(p.stories & ~0x7fu) && !(p.stickers & ~0xfffu) && !(p.dailyCompleted & ~0x3fu) && p.dailyClaimed <= 1
      && p.sleeping <= 1 && p.named <= 1 && (bool)p.named == (p.petName[0] != 0);
}
// Today's adventure is claimed exactly when it is complete, and a claim always carries its sticker.
inline bool validDaily(const Save& p) {
  const uint8_t mask = dailyMask(p.lastVisitDay);
  const bool complete = (p.dailyCompleted & mask) == mask;
  return complete == (bool)p.dailyClaimed && (!complete || (p.stickers & (1u << dailySticker(p.lastVisitDay))));
}
// Contents only (magic, size and crc are loadBlob's): a snapshot that passes can be played without surprises.
inline bool valid(const Save& p) {
  return p.createdAt <= p.updatedAt && p.lastVisitDay >= 0 && p.lastVisitDay <= dayOf(UINT32_MAX)   // a day now can reach
      && validNeed(p.fullness) && validNeed(p.happiness) && validNeed(p.energy)
      && validCounts(p) && validFlags(p) && validName(p.petName) && validDaily(p);
}

inline void seal(Save& s) {
  s.magic = SAVE_MAGIC; s.version = SAVE_VERSION; s.size = sizeof(Save); s.reserved[0] = s.reserved[1] = 0;
  s.crc = os::crc32(&s, sizeof s - sizeof s.crc);
}
// Any version up to this one: a blob is its own size (a prefix of today's layout) with its crc in the last four
// bytes, so fields appended after it was written load as zero.
inline bool loadBlob(const void* data, size_t n, Save& out) {
  if (n < SAVE_V1_SIZE || n > sizeof(Save) || n % 4) return false;
  const uint8_t* b = (const uint8_t*)data;
  uint32_t crc;
  memcpy(&crc, b + n - 4, 4);
  Save s{};
  memcpy(&s, b, n - 4);
  if (s.magic != SAVE_MAGIC || s.size != n || !s.version || s.version > SAVE_VERSION || crc != os::crc32(b, n - 4)) return false;
  if (!valid(s)) return false;
  seal(s);
  out = s;
  return true;
}

// ---- the save Biscuit had before Porthole ----
// Read once by the shell's migration (shell/migrate.cpp). The old firmware stored {magic, pet, FNV-1a over pet} as a
// native-endian blob in three generations: v1 (no names), v2 (kid and pup names), v3 (plus a daily play budget in
// what was v2's trailing padding). Those layouts are frozen; the static_asserts pin them.
namespace legacy {
constexpr uint32_t MAGIC_V1 = 0x5a4f4531, MAGIC_V2 = 0x5a4f4532, MAGIC_V3 = 0x5a4f4533;
// The old clock was UTC and its day counter used Pacific time (it hardcoded that zone); Porthole's is local time.
// ponytail: a fixed PST offset. During daylight time the stamps land an hour early, which only means up to one extra
// hour of decay on the first visit. Needs a zone table if a device ever migrates from elsewhere.
constexpr uint64_t UTC_OFFSET_SEC = 8 * 3600;
struct PetV1 {
  uint64_t createdAt, updatedAt;   // UTC seconds
  uint32_t version;
  int32_t lastVisitDay;            // local day ordinal, same epoch as dayOf()
  float fullness, happiness, energy;
  uint32_t friendship, daysTogether, careCounts[3], discoveries[3];
  uint16_t stickers;
  uint8_t stories, tricks[NUM_TRICKS], dailyCompleted, dailyClaimed, sleeping;
};
struct PetV3 {                     // v2 is the same bytes; its playSeconds is padding and never read
  PetV1 base;
  char playerName[NAME_LIMIT + 1], petName[NAME_LIMIT + 1];
  uint8_t setupComplete;
  uint32_t playSeconds;
};
template <class P> struct Blob { uint32_t magic; P pet; uint32_t hash; };
static_assert(sizeof(PetV1) == 80 && sizeof(PetV3) == 112 && offsetof(PetV3, playSeconds) == 108,
              "the old firmware's layouts are frozen");
static_assert(sizeof(Blob<PetV1>) == 96 && sizeof(Blob<PetV3>) == 128, "the old firmware's NVS blob sizes");

inline uint32_t fnv1a(const void* d, size_t n) {
  uint32_t h = 2166136261u;
  for (const uint8_t* p = (const uint8_t*)d; n--; p++) h = (h ^ *p) * 16777619u;
  return h;
}
template <class B> inline bool unwrap(const void* data, size_t n, uint32_t magic, B& b) {
  if (n != sizeof b) return false;
  memcpy(&b, data, n);
  return b.magic == magic && b.hash == fnv1a(&b.pet, sizeof b.pet);
}
inline bool convert(const PetV1& o, uint32_t version, Save& s) {
  if (o.version != version || o.createdAt > o.updatedAt || o.createdAt < UTC_OFFSET_SEC
      || o.updatedAt - UTC_OFFSET_SEC > UINT32_MAX) return false;
  s = Save{};
  s.createdAt = (uint32_t)(o.createdAt - UTC_OFFSET_SEC); s.updatedAt = (uint32_t)(o.updatedAt - UTC_OFFSET_SEC);
  s.lastVisitDay = o.lastVisitDay;
  s.fullness = o.fullness; s.happiness = o.happiness; s.energy = o.energy;
  s.friendship = o.friendship; s.daysTogether = o.daysTogether;
  memcpy(s.careCounts, o.careCounts, sizeof s.careCounts); memcpy(s.discoveries, o.discoveries, sizeof s.discoveries);
  s.stickers = o.stickers; s.stories = o.stories; memcpy(s.tricks, o.tricks, sizeof s.tricks);
  s.dailyCompleted = o.dailyCompleted; s.dailyClaimed = o.dailyClaimed; s.sleeping = o.sleeping;
  return true;
}
}  // namespace legacy

// The old save as a sealed Save, plus the kid's name it held ("" when it had none, as in v1). Only a blob with the
// right size, magic and FNV-1a, whose contents pass the old rules, converts. An unnamed v2/v3 pup (setup never
// finished, or a reset) has no progress, since nothing counts before naming, so it does not convert either.
inline bool fromLegacy(const void* data, size_t n, Save& out, char (&player)[NAME_LIMIT + 1]) {
  using namespace legacy;
  Blob<PetV1> a;
  Blob<PetV3> b;
  Save s{};
  char kid[NAME_LIMIT + 1] = "";
  if (unwrap(data, n, MAGIC_V1, a)) {
    if (!convert(a.pet, 1, s)) return false;
    memcpy(s.petName, "Biscuit", sizeof "Biscuit");   // v1 had no names: its pup was always Biscuit
  } else if (unwrap(data, n, MAGIC_V2, b) || unwrap(data, n, MAGIC_V3, b)) {
    const uint32_t version = b.magic == MAGIC_V2 ? 2 : 3;
    if (b.pet.setupComplete != 1 || !convert(b.pet.base, version, s) || !validName(b.pet.playerName) || !b.pet.playerName[0])
      return false;
    memcpy(s.petName, b.pet.petName, sizeof s.petName);
    memcpy(kid, b.pet.playerName, sizeof kid);
  } else {
    return false;
  }
  s.named = 1;
  if (!valid(s)) return false;
  seal(s);
  out = s;
  memcpy(player, kid, sizeof kid);
  return true;
}
}  // namespace biscuit
