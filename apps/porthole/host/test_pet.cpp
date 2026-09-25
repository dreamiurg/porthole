// Self-check for the pet simulation. Run: make test
// One pup's life, in order: each step starts from the state the previous one left.
#include <assert.h>
#include <stdio.h>
#include "games/pets-club/pet.h"
#include "games/pets-club/content.h"

static const uint32_t day0 = 1790000000u - (1790000000u % 86400u) + 10 * 3600;  // 10:00

static void decayAndGrowth(Save& s) {
  pet::adopt(s, day0, "Sam", "Biscuit");
  assert(pet::valid(s));
  assert(s.food == 70 && s.booksUnlocked == pet::STARTER_BOOKS && s.streak == 1);

  // 3 hours online awake: food -12, fun -9, energy -15
  uint32_t ev = pet::simulate(s, day0 + 3 * 3600, true);
  assert(s.food == 58); assert(s.fun == 71); assert(s.energy == 75); (void)ev;

  // Offline for 3 days: decay capped at 12h and floored; nights put the dog to sleep and restore energy
  pet::simulate(s, day0 + 3 * 86400, false);
  assert(s.food >= pet::FLOOR_FOOD && s.food <= 58);
  assert(s.fun >= pet::FLOOR_FUN);
  assert(s.energy >= 75);
  assert(pet::ageDays(s, day0 + 3 * 86400) == 3 && pet::stageFor(s, day0 + 3 * 86400) == STAGE_PUPPY);
  assert(pet::stageFor(s, day0 + 4 * 86400) == STAGE_DOG);
  assert(pet::stageFor(s, day0 + 10 * 86400) == STAGE_GROWN);
}

static void feeding(Save& s, uint32_t t) {
  // Feeding: kibble +35, capped at 100, refused when full
  s.asleep = 0; s.food = 50; int d = 0;
  assert(pet::feed(s, t, FOOD_KIBBLE, &d) && s.food == 85 && d == 35);
  assert(pet::feed(s, t, FOOD_KIBBLE, &d) && s.food == 100);
  assert(!pet::feed(s, t, FOOD_KIBBLE, &d));
  // treats capped at 3/day
  s.food = 10; assert(pet::feed(s, t, FOOD_COOKIE, &d)); assert(pet::feed(s, t, FOOD_COOKIE, &d)); assert(pet::feed(s, t, FOOD_COOKIE, &d));
  assert(!pet::feed(s, t, FOOD_COOKIE, &d));

  // Poop shows up after a meal, cleaning reschedules
  pet::simulate(s, t + 3 * 3600, true);
  assert(s.poop == 1);
  assert(pet::cleanPoop(s, t + 3 * 3600) && s.poop == 0 && s.nextPoopAt > t + 3 * 3600 + pet::POOP_MIN_SEC - 1);
}

static void bondAndTricks(Save& s, uint32_t t) {
  // Bond daily cap
  s.bond = 0; s.bondToday = 0; s.bondDay = 0;
  for (int i = 0; i < 50; i++) pet::addBond(s, t, 10);
  assert(s.bond == pet::BOND_DAILY_CAP);
  assert(pet::hearts(s) == 1);

  // Tricks: one lesson per day, 3 lessons to learn
  assert(pet::trickAvailable(s, 0) && !pet::trickLearned(s, 0));
  pet::trickLessonPassed(s, t, 0); assert(pet::trickLessonToday(s, t, 0));
  pet::trickLessonPassed(s, t + 86400, 0); pet::trickLessonPassed(s, t + 2 * 86400, 0);
  assert(pet::trickLearned(s, 0));
  assert(!pet::trickAvailable(s, 7));
}

static void giftsStreakReading(Save& s, uint32_t t) {
  // Gifts: one per calendar day; books first, hat on every third
  uint32_t g0 = t + 86400; s.lastGiftDay = (uint16_t)pet::dayIndex(g0) - 1;
  assert(pet::giftReady(s, g0));
  pet::Gift g = pet::openGift(s, g0); assert(g.kind == pet::GIFT_BOOK && s.booksUnlocked == pet::STARTER_BOOKS + 1);
  assert(!pet::giftReady(s, g0));
  g = pet::openGift(s, g0 + 86400); assert(g.kind == pet::GIFT_BOOK);
  g = pet::openGift(s, g0 + 2 * 86400); assert(g.kind == pet::GIFT_HAT && (s.hatsMask & (1u << g.index)));

  // Streak: consecutive days increment, a gap resets
  s.streak = 1; s.lastPlayDay = (uint16_t)pet::dayIndex(t);
  pet::touchDay(s, t + 86400); assert(s.streak == 2);
  pet::touchDay(s, t + 86400 + 100); assert(s.streak == 2);
  pet::touchDay(s, t + 4 * 86400); assert(s.streak == 1);

  // Reading
  int before = s.booksRead; pet::bookFinished(s, t, 0, true); pet::bookFinished(s, t, 0, false);
  assert(s.booksRead == before + 1 && (s.booksDoneMask & 1u) && (s.stickersMask & (1u << ST_FIRST_STORY)));

  // Save integrity
  pet::seal(s); assert(pet::valid(s)); s.food ^= 1; assert(!pet::valid(s)); s.food ^= 1; pet::seal(s);
}

// v1 blob (140 bytes, version 1) migrates: old fields keep their values, new fields are zero, age unknown
static void saveV1(const Save& s) {
  uint8_t v1[SAVE_V1_SIZE]; memcpy(v1, &s, SAVE_V1_SIZE - 4);
  uint16_t ver = 1, size = (uint16_t)SAVE_V1_SIZE; memcpy(v1 + 4, &ver, 2); memcpy(v1 + 6, &size, 2);
  uint32_t crc = pet::crc32(v1, SAVE_V1_SIZE - 4); memcpy(v1 + SAVE_V1_SIZE - 4, &crc, 4);
  Save m; assert(pet::loadBlob(v1, sizeof v1, m));
  assert(pet::valid(m) && m.version == 2 && m.size == sizeof(Save));
  assert(!strcmp(m.petName, "Biscuit") && m.booksRead == s.booksRead && m.bond == s.bond);
  assert(m.kidAge == 0 && m.pin == 0 && m.restUntil == 0);
  v1[20] ^= 1; assert(!pet::loadBlob(v1, sizeof v1, m));          // corrupted v1 rejected
  assert(pet::loadBlob(&s, sizeof s, m) && m.streak == s.streak);   // v2 round trip
  assert(!pet::loadBlob(&s, 17, m));
}

static void levels(const Save& s) {
  // reading levels by age
  { int lo, hi; pet::levelRange(6, lo, hi); assert(lo == 1 && hi == 1); pet::levelRange(8, lo, hi); assert(lo == 2 && hi == 3); pet::levelRange(10, lo, hi); assert(lo == 3 && hi == 3);
    pet::wordLenRange(6, lo, hi); assert(lo == 3 && hi == 4); pet::wordLenRange(9, lo, hi); assert(lo == 6 && hi == 8); }
  // story lists by age: a 6 year old only sees level 1, an 8 year old never sees level 1
  { uint8_t list[32]; Save r = s; r.kidAge = 6; int n6 = pet::bookListFor(r, list); assert(n6 >= 6); for (int i = 0; i < n6; i++) assert(BOOKS[list[i]].level == 1);
    r.kidAge = 8; int n8 = pet::bookListFor(r, list); assert(n8 >= 6); for (int i = 0; i < n8; i++) assert(BOOKS[list[i]].level >= 2); }
}

int main() {
  Save s;
  decayAndGrowth(s);
  uint32_t t = day0 + 3 * 86400;
  feeding(s, t);
  bondAndTricks(s, t);
  giftsStreakReading(s, t);
  saveV1(s);
  levels(s);
  printf("test_pet: all checks passed (sizeof Save = %zu)\n", sizeof(Save));
  return 0;
}
