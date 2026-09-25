// Pet simulation: pure logic, no rendering, no platform calls. Testable on the host.
#pragma once
#include <stdint.h>
#include <string.h>

constexpr int NUM_TRICKS = 8;
constexpr int NUM_HATS = 8;

enum Stage : uint8_t { STAGE_PUPPY = 0, STAGE_DOG = 1, STAGE_GROWN = 2 };
enum Variant : uint8_t { VAR_CLASSIC = 0, VAR_BOOKWORM = 1, VAR_SPORTY = 2 };
enum Want : uint8_t { WANT_NONE = 0, WANT_FOOD, WANT_PLAY, WANT_SLEEP, WANT_BATH, WANT_POOP, WANT_STORY };
enum Food : uint8_t { FOOD_KIBBLE = 0, FOOD_BONE, FOOD_COOKIE };
enum Sticker : uint8_t {
  ST_FIRST_MEAL, ST_FIRST_STORY, ST_BOOKWORM, ST_LIBRARY, ST_BONES, ST_SPELLER, ST_WEEK, ST_TWO_WEEKS,
  ST_TRICKSTAR, ST_SHOWDOG, ST_CLEAN, ST_BESTFRIENDS, ST_EARLYBIRD, ST_PARTY, ST_GROWN, ST_HATS, NUM_STICKERS
};

// Persisted state. Plain data, saved as a blob. Bump SAVE_VERSION on layout changes.
constexpr uint32_t SAVE_MAGIC = 0x4F475243;  // "CRGO"
constexpr uint16_t SAVE_VERSION = 2;
constexpr size_t SAVE_V1_SIZE = 140;      // layout before the house/age fields; still accepted on load
constexpr int MAX_HOUSES = 3;             // houses (s0..s2) before Porthole: only the shell's migration reads them
struct Save {
  uint32_t magic;
  uint16_t version, size;
  char kidName[12], petName[12];
  uint32_t adoptedAt, lastSeen;
  uint8_t food, fun, energy, clean;       // 0..100
  uint8_t asleep, poop, dirty, stage, variant, hat, muted, tutorial;
  uint16_t bond;                          // 0..1000 -> hearts = bond/100
  uint16_t bondToday, bondDay;
  uint32_t nextPoopAt, poopSince;
  uint16_t streak, lastPlayDay, lastGiftDay, giftsOpened;
  uint16_t booksRead, wordsSpelled, bonesCaught, gamesPlayed, baths, petsGiven, tricksShown;
  uint32_t booksDoneMask;
  uint8_t booksUnlocked;
  uint8_t trickProgress[NUM_TRICKS];      // lessons passed; 3 = learned
  uint16_t trickLastDay[NUM_TRICKS];
  uint16_t hatsMask, stickersMask;
  uint8_t treatsToday, petsToday, want, wantSince;  // wantSince: minutes/10 since want began (display only)
  uint16_t treatsDay;
  uint32_t seed;
  // ---- v2: houses, reading level, turn taking. Since Porthole the profile owns the kid's name, age, code, mute and
  // rest budget: kidName/kidAge are copied in from the profile on entry; muted, pin, restUntil and playSec are only
  // read once, by the shell's migration. The fields stay: the layout is append-only.
  uint8_t kidAge, theme, reserved0, reserved1;   // kidAge 0 = not asked yet; theme = room color scheme
  uint16_t pin, reserved2;                       // 4-digit secret code, 0 = none
  uint32_t restUntil, playSec;                   // turn taking: resting until / seconds played this session
  uint32_t crc;
};

namespace pet {
// ---- calibration knobs (per real hour unless noted) ----
constexpr int DECAY_FOOD = 4, DECAY_FUN = 3, DECAY_ENERGY = 5, DECAY_CLEAN = 2;
constexpr int FLOOR_FOOD = 20, FLOOR_FUN = 25, FLOOR_ENERGY = 20, FLOOR_CLEAN = 20;
constexpr int SLEEP_ENERGY_OFFLINE = 40;    // per hour while asleep and device off/idle
constexpr int SLEEP_ENERGY_ONLINE = 30;     // per minute while asleep and being watched
constexpr uint32_t OFFLINE_CAP_SEC = 12 * 3600;
constexpr int BEDTIME_HOUR = 20, WAKE_HOUR = 7;
constexpr uint32_t POOP_MIN_SEC = 4 * 3600, POOP_MAX_SEC = 6 * 3600, POOP_AFTER_MEAL_SEC = 45 * 60;
constexpr int BOND_DAILY_CAP = 120;
constexpr int STAGE_DOG_DAYS = 4, STAGE_GROWN_DAYS = 10;
constexpr int STARTER_BOOKS = 3;
static const uint8_t TRICK_UNLOCK_HEARTS[NUM_TRICKS] = {0, 1, 2, 3, 4, 5, 6, 7};

uint32_t crc32(const void* d, size_t n);
bool valid(const Save& s);
bool loadBlob(const void* data, size_t n, Save& out);   // accepts v1 (migrates) and v2 blobs
void levelRange(int age, int& lo, int& hi);             // story levels 1..3 offered for a kid's age
void wordLenRange(int age, int& lo, int& hi);           // spelling word lengths for a kid's age
int bookListFor(const Save& s, uint8_t* out);          // story indices this kid may read (by age), widened if too few
void seal(Save& s);                          // sets magic/version/size/crc
void adopt(Save& s, uint32_t now, const char* kid, const char* pet);

// time helpers (local wall-clock seconds since 1970)
inline uint32_t dayIndex(uint32_t t) { return t / 86400u; }
inline int hourOf(uint32_t t) { return (int)((t / 3600u) % 24u); }
inline int minuteOf(uint32_t t) { return (int)((t / 60u) % 60u); }
inline bool isNightHour(int h) { return h >= BEDTIME_HOUR || h < WAKE_HOUR; }
int ageDays(const Save& s, uint32_t now);
Stage stageFor(const Save& s, uint32_t now);
int hearts(const Save& s);
uint32_t rnd(Save& s);                       // xorshift, persisted seed
inline int rndRange(Save& s, int lo, int hi) { return lo + (int)(rnd(s) % (uint32_t)(hi - lo + 1)); }

// Advance simulation from s.lastSeen to now. online=true means the device is on and showing the pet.
// Returns bitmask of notable things that happened (see EV_*).
enum Event : uint32_t { EV_NONE = 0, EV_POOPED = 1, EV_FELL_ASLEEP = 2, EV_WOKE = 4, EV_STAGE_UP = 8, EV_NEW_DAY = 16, EV_LONG_AWAY = 32 };
uint32_t simulate(Save& s, uint32_t now, bool online);

// actions (all return true if the action was accepted)
bool feed(Save& s, uint32_t now, Food f, int* outDelta);
void addBond(Save& s, uint32_t now, int n);
bool cleanPoop(Save& s, uint32_t now);
bool bathe(Save& s, uint32_t now);
bool petDog(Save& s, uint32_t now);
void playResult(Save& s, uint32_t now, int score, bool gotMuddy);
void wordsResult(Save& s, uint32_t now, int correct);
void bookFinished(Save& s, uint32_t now, int bookIndex, bool answeredRight);
bool canWake(const Save& s);
void setAsleep(Save& s, uint32_t now, bool asleep);
bool trickLearned(const Save& s, int i);
bool trickAvailable(const Save& s, int i);       // enough hearts to start training
bool trickLessonToday(const Save& s, uint32_t now, int i);
void trickLessonPassed(Save& s, uint32_t now, int i);
int  learnedTrickCount(const Save& s);
bool giftReady(const Save& s, uint32_t now);
enum GiftKind : uint8_t { GIFT_BOOK = 0, GIFT_HAT, GIFT_TREATS, GIFT_PARTY };
struct Gift { GiftKind kind; uint8_t index; };
Gift openGift(Save& s, uint32_t now);
void touchDay(Save& s, uint32_t now);            // call on any interaction: streak + daily resets
bool earnSticker(Save& s, int id);               // true if newly earned
Want computeWant(const Save& s, uint32_t now);
}  // namespace pet
