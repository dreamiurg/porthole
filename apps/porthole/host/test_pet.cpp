// Self-check for the pet simulation. Run: make test
// One pup's life, in order: each step starts from the state the previous one left.
#include "host/check.h"
#include <stdio.h>
#include "games/pets-club/pet.h"
#include "games/pets-club/content.h"
#include "games/pets-club/game.h"

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

static_assert(sizeof(Save::wallNames[0]) == sizeof(Save::petName), "a wall frame holds the whole pet name");

// An older blob: the first n - 4 bytes of s under its version, with a good CRC.
static void oldBlob(uint8_t* out, const Save& s, size_t n, uint16_t ver) {
  memcpy(out, &s, n - 4);
  uint16_t size = (uint16_t)n; memcpy(out + 4, &ver, 2); memcpy(out + 6, &size, 2);
  uint32_t crc = pet::crc32(out, n - 4); memcpy(out + n - 4, &crc, 4);
}

// v1 blob (140 bytes, version 1) migrates: old fields keep their values, new fields are zero, age unknown
static void saveV1(const Save& s) {
  uint8_t v1[SAVE_V1_SIZE]; oldBlob(v1, s, sizeof v1, 2);
  Save m; assert(!pet::loadBlob(v1, sizeof v1, m));                // 140 bytes claiming v2: rejected, even with a good CRC
  oldBlob(v1, s, sizeof v1, 1);
  assert(pet::loadBlob(v1, sizeof v1, m));
  assert(pet::valid(m) && m.version == SAVE_VERSION && m.size == sizeof(Save));
  assert(!strcmp(m.petName, "Biscuit") && m.booksRead == s.booksRead && m.bond == s.bond && m.hasDog == 1);
  assert(m.kidAge == 0 && m.pin == 0 && m.restUntil == 0);
  v1[20] ^= 1; assert(!pet::loadBlob(v1, sizeof v1, m));          // corrupted v1 rejected
  assert(pet::loadBlob(&s, sizeof s, m) && m.streak == s.streak);   // current-version round trip
  assert(!pet::loadBlob(&s, 17, m));
}

// v2 blob (156 bytes, version 2) migrates: it still has its dog, a Golden pup, and an empty wall
static void saveV2Migrates(const Save& s) {
  uint8_t v2[SAVE_V2_SIZE]; oldBlob(v2, s, sizeof v2, 2);
  Save m; assert(pet::loadBlob(v2, sizeof v2, m));
  assert(pet::valid(m) && m.version == SAVE_VERSION && m.size == sizeof(Save));
  assert(m.breed == 0 && m.hasDog == 1 && m.wallCount == 0 && m.dogsRaised == 0 && m.streak == s.streak);
  assert(!strcmp(m.petName, s.petName) && m.kidAge == s.kidAge && m.playSec == s.playSec);
  v2[20] ^= 1; assert(!pet::loadBlob(v2, sizeof v2, m));          // corrupted v2 rejected
  oldBlob(v2, s, sizeof v2, 3); assert(!pet::loadBlob(v2, sizeof v2, m));   // a v2 size must say v2, even with a good CRC
}

static void shelterPupStartsFresh(Save& s, uint32_t now) {
  pet::adoptFromShelter(s, now, 3, "Choco");
  assert(pet::valid(s) && s.hasDog == 1 && s.breed == 3 && !strcmp(s.petName, "Choco"));
  assert(s.trickProgress[0] == 0 && s.bond == 0 && s.asleep == 0 && s.stage == STAGE_PUPPY && pet::ageDays(s, now) == 0);
  assert(s.food == 70 && s.fun == 80 && s.energy == 90 && s.clean == 100);
  assert(s.stickersMask == 0x5 && s.hatsMask == 3 && s.wallCount == 1 && s.dogsRaised == 1);
}

// Right after retire, before any shelter pup: none of the old dog's own state is left in the empty room.
static void retiredDogIsGone(const Save& s) {
  assert(s.petName[0] == 0 && s.bond == 0 && s.trickProgress[0] == 0);
  assert(s.asleep == 0 && s.poop == 0 && s.dirty == 0 && s.want == WANT_NONE && s.hat == 0);
}

// The farm: the dog moves on, the kid keeps everything they earned, the next pup starts fresh.
static void retireKeepsKidProgress() {
  Save s; pet::adopt(s, day0, "Sam", "Connor");
  s.stickersMask = 0x5; s.hatsMask = 3; s.hat = 2; s.booksRead = 7; s.trickProgress[0] = 3; s.bond = 900; s.asleep = 1;
  s.poop = 1; s.dirty = 1; s.want = WANT_SLEEP; s.stage = STAGE_GROWN;
  const uint32_t now = day0 + 31 * 86400;
  assert(!pet::canRetire(s, day0 + 29 * 86400) && pet::canRetire(s, day0 + 30 * 86400) && pet::canRetire(s, now));
  pet::retire(s, now);
  assert(pet::valid(s) && !pet::canRetire(s, now + 60 * 86400));
  assert(s.stickersMask == 0x5 && s.hatsMask == 3 && s.booksRead == 7 && s.streak == 1 && !strcmp(s.kidName, "Sam"));
  assert(s.hasDog == 0 && s.wallCount == 1 && !strcmp(s.wallNames[0], "Connor") && s.dogsRaised == 1);
  retiredDogIsGone(s);
  shelterPupStartsFresh(s, now);
}
// The wall keeps the last four dogs, oldest first; dogsRaised counts them all.
static void wallRollsAtFour() {
  Save s; pet::adopt(s, day0, "Sam", "A");
  const char* names[] = {"A", "B", "C", "D", "E"};
  for (int i = 0; i < 5; i++) {
    if (i) pet::adoptFromShelter(s, day0, (uint8_t)i, names[i]);
    pet::retire(s, day0);
  }
  pet::retire(s, day0);   // no dog: nothing to put on the wall
  assert(s.wallCount == WALL_SLOTS && s.dogsRaised == 5);
  for (int i = 0; i < WALL_SLOTS; i++) assert(!strcmp(s.wallNames[i], names[i + 1]) && s.wallBreeds[i] == i + 1);
}

// An empty room (dog on the farm, no pup yet) stands still: nothing grows, decays or poops.
static void simulateEmptyRoom() {
  Save s; pet::adopt(s, day0, "Sam", "Connor"); pet::retire(s, day0);
  const Save before = s;
  const uint32_t later = day0 + 7 * 86400;
  assert(pet::simulate(s, later, false) == pet::EV_NONE);
  assert(s.lastSeen == later && s.stage == before.stage && s.poop == 0);
  assert(s.food == before.food && s.fun == before.fun && s.energy == before.energy && s.clean == before.clean);
}

static void breedSurvivesRoundTrip() {
  Save s; pet::adopt(s, day0, "Sam", "Connor"); pet::retire(s, day0); pet::adoptFromShelter(s, day0, 5, "Waffles");
  Save m; assert(pet::loadBlob(&s, sizeof s, m));
  assert(m.breed == 5 && m.hasDog == 1 && m.wallCount == 1 && !strcmp(m.wallNames[0], "Connor") && m.dogsRaised == 1);
  pet::retire(s, day0); assert(pet::loadBlob(&s, sizeof s, m) && m.hasDog == 0 && m.wallBreeds[1] == 5);
}

static void levels(const Save& s) {
  // reading levels by age
  { int lo, hi; pet::levelRange(6, lo, hi); assert(lo == 1 && hi == 1); pet::levelRange(8, lo, hi); assert(lo == 2 && hi == 3); pet::levelRange(10, lo, hi); assert(lo == 3 && hi == 3);
    pet::wordLenRange(6, lo, hi); assert(lo == 3 && hi == 4); pet::wordLenRange(9, lo, hi); assert(lo == 6 && hi == 8); }
  // story lists by age: a 6 year old only sees level 1, an 8 year old never sees level 1
  { uint8_t list[32]; Save r = s; r.kidAge = 6; int n6 = pet::bookListFor(r, list); assert(n6 >= 6); for (int i = 0; i < n6; i++) assert(BOOKS[list[i]].level == 1);
    r.kidAge = 8; int n8 = pet::bookListFor(r, list); assert(n8 >= 6); for (int i = 0; i < n8; i++) assert(BOOKS[list[i]].level >= 2); }
}

// 22:00 in the game: Auto (AppEnter::daylight false) is night; a sun picked on the launcher is day, with no stars.
static void dayOnPickedLight() {
  static Game g; const Profile kid = {0, "Sam", 0, 8, 0}; const SaveSlot none = {nullptr, 0};
  const uint32_t night = day0 + 12 * 3600;
  g.enter({&kid, &kid, &none, 1, night, 0}); assert(g.tint() == TINT_NIGHT && g.nightSky());
  g.enter({&kid, &kid, &none, 1, night, 0, true}); assert(g.tint() == TINT_DAY && !g.nightSky());
}

// A breed byte past the art (a corrupt save) draws the Golden pup at home, never out of bounds; a Corgi (5) draws as itself.
static void homeFrame(uint8_t breed, uint8_t* out) {
  static Game g; const Profile kid = {0, "Sam", 0, 8, 0};
  Save s; pet::adopt(s, day0, "Sam", "Rex"); s.breed = breed; pet::seal(s);
  const SaveSlot slot = {&s, sizeof s};
  g.enter({&kid, &kid, &slot, 1, day0, 0}); g.update(day0, 2000, Input{}); g.render();   // past the splash: home
  memcpy(out, gfx::fb, sizeof gfx::fb);
}
static void breedClampDraws() {
  static uint8_t golden[sizeof gfx::fb], corrupt[sizeof gfx::fb], corgi[sizeof gfx::fb];
  homeFrame(0, golden); homeFrame(200, corrupt); homeFrame(5, corgi);
  assert(!memcmp(golden, corrupt, sizeof golden) && memcmp(golden, corgi, sizeof golden));
}

// A dog that falls asleep at 16:00 and moves to the farm leaves the room in daylight: the night tint follows the dog.
static Tint homeTintAt1600(bool retired) {
  static Game g; const Profile kid = {0, "Sam", 0, 8, 0};
  const uint32_t t = day0 + 6 * 3600;
  Save s; pet::adopt(s, t, "Sam", "Rex"); pet::setAsleep(s, t, true); s.energy = 20;   // too tired to wake by itself
  if (retired) pet::retire(s, t);
  pet::seal(s);
  const SaveSlot slot = {&s, sizeof s};
  g.enter({&kid, &kid, &slot, 1, t, 0}); g.update(t, 2000, Input{});
  assert(!strcmp(g.screenName(), "home"));
  return g.tint();
}
static void retiredSleeperLeavesDay() { assert(homeTintAt1600(false) == TINT_NIGHT && homeTintAt1600(true) == TINT_DAY); }

static void petting(uint32_t t) {
  // Petting: always a little fun, but bond only from the first 20 pets a day; never while asleep
  Save s; pet::adopt(s, t, "Sam", "Biscuit");
  s.asleep = 1; assert(!pet::petDog(s, t) && s.petsGiven == 0);
  s.asleep = 0; s.fun = 50;
  for (int i = 0; i < 25; i++) assert(pet::petDog(s, t));
  assert(s.petsGiven == 25 && s.petsToday == 20 && s.fun == 100 && s.bondToday == 20);
  assert(pet::petDog(s, t + 86400) && s.petsToday == 1 && s.bondToday == 1);   // a new day starts over
}

int main() {
  Save s;
  decayAndGrowth(s);
  uint32_t t = day0 + 3 * 86400;
  feeding(s, t);
  bondAndTricks(s, t);
  giftsStreakReading(s, t);
  saveV1(s);
  saveV2Migrates(s);
  retireKeepsKidProgress();
  wallRollsAtFour();
  simulateEmptyRoom();
  breedSurvivesRoundTrip();
  levels(s);
  dayOnPickedLight();
  breedClampDraws();
  retiredSleeperLeavesDay();
  petting(day0);
  printf("test_pet: all checks passed (sizeof Save = %zu)\n", sizeof(Save));
  return 0;
}
