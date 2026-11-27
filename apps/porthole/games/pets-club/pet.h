// Pet simulation: pure logic, no rendering, no platform calls. Testable on the host.
#pragma once
#include <stdint.h>
#include <string.h>
#include "day.h"

constexpr int NUM_TRICKS = 8;
constexpr int NUM_HATS = 8;
constexpr int NUM_COLLARS = 8;   // the Shop's collars (content.h COLLARS); collarsOwned bit i = collar i

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
constexpr uint16_t SAVE_VERSION = 5;
constexpr size_t SAVE_V1_SIZE = 140;      // layout before the house/age fields; still accepted on load
constexpr size_t SAVE_V2_SIZE = 156;      // layout before breed and the memory wall; still accepted on load
constexpr size_t SAVE_V3_SIZE = 212;      // layout before puzzles and little reads; still accepted on load
constexpr size_t SAVE_V4_SIZE = 288;      // layout before the two bookshelves; still accepted on load
constexpr int RETIRE_DAYS = 30;           // age at which the kid may move the dog to the farm
constexpr int WALL_SLOTS = 4;             // memory wall frames: the last four dogs
constexpr int NUM_BREEDS = 7;             // shelter breeds; 0 = the Golden pup every older save already has
constexpr int MAX_HOUSES = 3;             // houses (s0..s2) before Porthole: only the shell's migration reads them
constexpr int PUZZLE_STRANDS = 3, PUZZLE_REVIEWS = 8;
// A puzzle missed, to ask again on a later day (learn::puzzles::Item's bytes, so this header needs no puzzle content).
struct PuzzleReview { uint8_t strand, level, a, b, c, misses; uint16_t due; };   // due: a day index, 0 = empty
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
  uint8_t kidAge, theme, collar, collarsOwned;   // kidAge 0 = not asked yet; theme = room color scheme; collar worn
                                                 // (0 = Red) and a bit per bought collar (the Shop), once reserved0/1
  uint16_t pin, lastBuyDay;                      // 4-digit secret code, 0 = none; the day of the last buy (once reserved2)
  uint32_t restUntil, playSec;                   // turn taking: resting until / seconds played this session
  // ---- v3: breeds, the farm and the memory wall. hasDog 0 = empty room (dog on the farm, no pup adopted yet).
  uint8_t breed, dogsRaised, wallCount, hasDog;  // dogsRaised: dogs moved to the farm, ever
  char wallNames[WALL_SLOTS][12];                // oldest first
  uint8_t wallBreeds[WALL_SLOTS];
  // ---- v4: puzzles and little reads (learn.cpp). They belong to the kid, not the dog: a new pup keeps them.
  uint8_t puzzleLevels[PUZZLE_STRANDS], puzzleReviewCount;  // each strand's level + 1 (0: never played, starts by age)
  uint8_t readLevel, readStreak;                 // little reads level 1..READ_LEVELS (0: not yet, starts by age)
  uint16_t readsRight, puzzlesRight;             // right answers, ever: every READS_PER_STAR / PUZZLES_PER_STAR is a star
  uint8_t lastBuyItem, reserved3;                // the last buy: hat i, or 0x80 | collar i (for a later Give back)
  PuzzleReview puzzleReviews[PUZZLE_REVIEWS];    // missed puzzles that come back on a later day, oldest first
  // ---- v5: the two bookshelves (books.h): a bit per book read, English (BOOKS) and Ukrainian (UK_BOOKS). The old
  // booksDoneMask is copied into booksEn on load and no longer written; booksUnlocked is no longer read (every book is
  // on the shelf now, no gift unlocks one).
  uint8_t booksEn[8], booksUk[8];
  uint8_t booksEnMore[8];                        // English books 64..127 (once reserved4's first half): no v6 needed
  uint8_t reserved4[8];                          // kept for the Shop's room and toys, so they need no v6
  uint32_t crc;
};
static_assert(sizeof(Save) == 320, "Save is append-only: v5 is 320 bytes");

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
static const uint8_t TRICK_UNLOCK_HEARTS[NUM_TRICKS] = {0, 1, 2, 3, 4, 5, 6, 7};

uint32_t crc32(const void* d, size_t n);
bool valid(const Save& s);
bool loadBlob(const void* data, size_t n, Save& out);   // accepts v1..v4 (migrates) and v5 blobs
void wordLenRange(int age, int& lo, int& hi);           // spelling word lengths for a kid's age
void seal(Save& s);                          // sets magic/version/size/crc
void adopt(Save& s, uint32_t now, const char* kid, const char* pet);
bool canRetire(const Save& s, uint32_t now);  // old enough for the farm
void retire(Save& s, uint32_t now);           // dog to the farm and onto the wall; its own fields stay until the next pup
inline bool canBringBack(const Save& s) { return !s.hasDog && s.wallCount > 0; }   // an empty room: the newest frame
bool bringBack(Save& s, uint32_t now);        // undoes retire, until adoptFromShelter replaces the dog's fields
void adoptFromShelter(Save& s, uint32_t now, uint8_t breed, const char* name);  // next pup into the empty room

// time helpers (local wall-clock seconds since 1970)
inline uint32_t dayIndex(uint32_t t) { return os::dayOf(t); }
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
enum Event : uint32_t { EV_NONE = 0, EV_POOPED = 1, EV_FELL_ASLEEP = 2, EV_WOKE = 4, EV_STAGE_UP = 8, EV_LONG_AWAY = 32 };
uint32_t simulate(Save& s, uint32_t now, bool online);
// Decay is counted in fractions of a point; the fraction left over belongs to the visit that made it. Game::enter
// drops it, so one kid's never lands on the next kid's dog and a visit does not depend on who played before.
void dropCarry();

// actions (all return true if the action was accepted)
bool feed(Save& s, uint32_t now, Food f, int* outDelta);
void addBond(Save& s, uint32_t now, int n);
bool cleanPoop(Save& s, uint32_t now);
bool bathe(Save& s, uint32_t now);
bool petDog(Save& s, uint32_t now);
void playResult(Save& s, uint32_t now, int score, bool gotMuddy);
void wordsResult(Save& s, uint32_t now, int correct);
void bookFinished(Save& s, uint32_t now, bool first, bool answeredRight);   // a book read (books.h marks which)
bool canWake(const Save& s);
void setAsleep(Save& s, uint32_t now, bool asleep);
bool trickLearned(const Save& s, int i);
bool trickAvailable(const Save& s, int i);       // enough hearts to start training
bool trickLessonToday(const Save& s, uint32_t now, int i);
void trickLessonPassed(Save& s, uint32_t now, int i);
int  learnedTrickCount(const Save& s);
// Puzzles and little reads: a right answer is kept and brings the dog a little closer (bond); every PUZZLES_PER_STAR
// right puzzles or READS_PER_STAR right reads is a star for the kid's wallet (os/stars.h). Both return true when this
// answer earned one. Little reads move a level 1..READ_LEVELS: READ_STREAK_UP right in a row up one, a miss down one,
// never more than one below where the kid's age starts it.
constexpr int PUZZLES_PER_STAR = 5, READS_PER_STAR = 3, LEARN_BOND = 3;
constexpr uint8_t READ_LEVELS = 5, READ_STREAK_UP = 3;
bool puzzleAnswered(Save& s, uint32_t now, bool right);
uint8_t ageReadLevel(int age);
inline uint8_t readLevel(const Save& s) { return s.readLevel ? s.readLevel : ageReadLevel(s.kidAge); }
bool readAnswered(Save& s, uint32_t now, bool right);
void learnRoundDone(Save& s);                    // a whole round of puzzles or reads: the dog had fun too
void gotHat(Save& s, int h);                     // a gift's or a bought hat (Hats sells the rest for stars)
bool giftReady(const Save& s, uint32_t now);
enum GiftKind : uint8_t { GIFT_HAT = 1, GIFT_TREATS, GIFT_PARTY };   // 0 was a book: books are all on the shelf now
struct Gift { GiftKind kind; uint8_t index; };
Gift openGift(Save& s, uint32_t now);
void touchDay(Save& s, uint32_t now);            // call on any interaction: streak + daily resets
bool earnSticker(Save& s, int id);               // true if newly earned
Want computeWant(const Save& s, uint32_t now);
}  // namespace pet
