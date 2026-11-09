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

// Right after retire, before any shelter pup: the empty room is calm (no sleep, mess, want or hat), and the dog's own
// fields wait in the save for Bring back.
static void retiredRoomIsCalm(const Save& s) {
  assert(s.asleep == 0 && s.poop == 0 && s.dirty == 0 && s.want == WANT_NONE && s.hat == 0);
  assert(!strcmp(s.petName, "Connor") && s.bond == 900 && s.trickProgress[0] == 3 && s.stage == STAGE_GROWN);
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
  retiredRoomIsCalm(s);
  shelterPupStartsFresh(s, now);
}
// Bring back undoes retire until a pup is adopted: the same dog (name, breed, age, tricks, bond, stats) and the wall
// as it was; the empty room's days are not time away. Only a full wall differs: the oldest frame retire took down
// stays down.
static Save dogWithWall(int frames) {   // frames shelter dogs on the wall, then Rex the Corgi at home with a past
  Save s; pet::adopt(s, day0, "Sam", "A");
  const char* names[] = {"A", "B", "C", "D"};
  for (int i = 0; i < frames; i++) { if (i) pet::adoptFromShelter(s, day0, (uint8_t)i, names[i]); pet::retire(s, day0); }
  pet::adoptFromShelter(s, day0, 5, "Rex");
  s.stage = STAGE_GROWN; s.bond = 640; s.trickProgress[2] = 3; s.trickLastDay[2] = 7; s.food = 55; s.fun = 33; s.energy = 41;
  s.clean = 62; s.hat = 1; pet::seal(s);
  return s;
}
static void sameDog(const Save& a, const Save& b) {
  assert(!strcmp(a.petName, b.petName) && a.breed == b.breed && a.adoptedAt == b.adoptedAt && a.stage == b.stage);
  assert(a.bond == b.bond && !memcmp(a.trickProgress, b.trickProgress, sizeof a.trickProgress));
  assert(!memcmp(a.trickLastDay, b.trickLastDay, sizeof a.trickLastDay));
  assert(a.food == b.food && a.fun == b.fun && a.energy == b.energy && a.clean == b.clean);
}
static void bringBackRestores() {
  const uint32_t t = day0 + 40 * 86400, later = t + 3 * 86400;
  Save s = dogWithWall(2); const Save before = s;
  assert(!pet::canBringBack(s) && !pet::bringBack(s, t));   // a dog at home: nothing to bring back
  pet::retire(s, t);
  assert(pet::canBringBack(s) && s.wallCount == 3 && s.dogsRaised == 3);
  Save m; assert(pet::loadBlob(&s, sizeof s, m) && !memcmp(&m, &s, sizeof m));   // a restart keeps the empty room whole
  assert(pet::simulate(m, later, false) == pet::EV_NONE);
  assert(pet::bringBack(m, later) && pet::valid(m) && m.hasDog == 1 && m.lastSeen == later && m.hat == 0);
  sameDog(m, before); assert(pet::ageDays(m, later) == pet::ageDays(before, later));
  assert(m.wallCount == before.wallCount && m.dogsRaised == before.dogsRaised);
  assert(!memcmp(m.wallNames, before.wallNames, sizeof m.wallNames) && !memcmp(m.wallBreeds, before.wallBreeds, sizeof m.wallBreeds));
  assert(m.nextPoopAt > later && !(pet::simulate(m, later + 60, true) & pet::EV_POOPED) && m.food >= 54 && m.poop == 0);
  assert(!pet::bringBack(m, later));   // once
}
static void adoptionEndsBringBack() {
  Save m = dogWithWall(1); pet::retire(m, day0); pet::adoptFromShelter(m, day0, 3, "Choco");
  assert(!pet::canBringBack(m) && !pet::bringBack(m, day0) && !strcmp(m.petName, "Choco") && m.bond == 0);
}
// A full wall: retire took the oldest frame (A) down to hang Rex, and Bring back does not hang it again.
static void bringBackOntoFullWall() {
  Save f = dogWithWall(WALL_SLOTS); pet::retire(f, day0);
  assert(f.wallCount == WALL_SLOTS && !strcmp(f.wallNames[0], "B") && !strcmp(f.wallNames[3], "Rex"));
  assert(pet::bringBack(f, day0) && f.wallCount == WALL_SLOTS - 1 && f.dogsRaised == WALL_SLOTS);
  for (int i = 0; i < WALL_SLOTS - 1; i++) assert(f.wallBreeds[i] == i + 1);
  assert(!strcmp(f.wallNames[0], "B") && !strcmp(f.wallNames[2], "D") && f.wallNames[3][0] == 0 && !strcmp(f.petName, "Rex"));
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

// A crc-valid v3 blob with bytes past the art and the wall loads safe: every reader indexes with them unchecked.
static void corruptFieldsLoadSafe() {
  Save s; pet::adopt(s, day0, "Sam", "Rex");
  s.breed = 200; s.wallBreeds[2] = 99; s.wallCount = 9; s.hasDog = 7; pet::seal(s);
  Save m; assert(pet::loadBlob(&s, sizeof s, m) && pet::valid(m));
  assert(m.breed == 0 && m.wallBreeds[2] == 0 && m.wallCount == WALL_SLOTS && m.hasDog == 1);
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

// A Corgi (5) draws as itself at home, not as the Golden pup.
static void homeFrame(uint8_t breed, uint8_t* out) {
  static Game g; const Profile kid = {0, "Sam", 0, 8, 0};
  Save s; pet::adopt(s, day0, "Sam", "Rex"); s.breed = breed; pet::seal(s);
  const SaveSlot slot = {&s, sizeof s};
  g.enter({&kid, &kid, &slot, 1, day0, 0}); g.update(day0, 2000, Input{}); g.render();   // past the splash: home
  memcpy(out, gfx::fb, sizeof gfx::fb);
}
static void breedDraws() {
  static uint8_t golden[sizeof gfx::fb], corgi[sizeof gfx::fb];
  homeFrame(0, golden); homeFrame(5, corgi);
  assert(memcmp(golden, corgi, sizeof golden));
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

// The week's streak sticker comes with the day's first touch, the empty room's too: its card shows no dog.
static int celebrateDogInk(bool retired) {
  static Game g; const Profile kid = {0, "Sam", 0, 8, 0};
  const uint32_t t = day0 + 7 * 86400;
  Save s; pet::adopt(s, day0, "Sam", "Rex"); s.streak = 6; s.lastPlayDay = (uint16_t)(pet::dayIndex(t) - 1);
  if (retired) pet::retire(s, t);
  pet::seal(s);
  const SaveSlot slot = {&s, sizeof s};
  g.enter({&kid, &kid, &slot, 1, t, 0}); g.update(t, 2000, Input{});   // past the splash: home
  Input press; press.down = press.pressed = true; press.x = 80; press.y = 150;
  g.update(t, 3000, press); g.render();
  assert(!strcmp(g.screenName(), "celebrate"));
  int ink = 0;   // under the card, where the dog stands: anything but the night sky, its stars and the confetti
  for (int y = 100; y < 134; y++) for (int x = 50; x < 110; x++) { const uint8_t c = gfx::fb[y * gfx::W + x]; ink += c != C_NAVY && (c < C_RED || c > C_PEACH); }
  return ink;
}
static void emptyRoomCelebratesWithoutDog() { assert(celebrateDogInk(false) > 0 && celebrateDogInk(true) == 0); }

// A long habit's press on the farm's Yes: Go home, then Bring back in the empty room (after a restart too) puts the
// same dog home (its age, tricks and stats, and the wall as it was) and asks for the one-second checkpoint.
struct Finger {   // frames of 20 ms through the real InputTracker, the clock standing still (no tick, no decay)
  Game& g; uint32_t now, ms = 2000; InputTracker tr;
  void hold(bool down, int x, int y, uint32_t dur) { for (uint32_t e = 0; e < dur; e += 20) { ms += 20; g.update(now, ms, tr.step(down, x, y, ms)); } }
  void tap(int x, int y) { hold(true, x, y, 60); hold(false, x, y, 500); }
};
static Save taken(Game& g) { const void* d = nullptr; size_t n = 0; assert(g.takeSave(&d, &n, true) && n == sizeof(Save)); Save s; memcpy(&s, d, n); return s; }
static void enterAt(Game& g, const Save& s, uint32_t t) {
  static const Profile kid = {0, "Sam", 0, 8, 0}; const SaveSlot slot = {&s, sizeof s};
  g.enter({&kid, &kid, &slot, 1, t, 0}); g.update(t, 2000, Input{});
}
static void bringBackFromTheEmptyRoom() {
  static Game g;
  const uint32_t t = day0 + 40 * 86400;
  Save s = dogWithWall(1); s.adoptedAt = t - 31 * 86400; s.lastSeen = t; s.lastPlayDay = (uint16_t)pet::dayIndex(t); pet::seal(s);
  enterAt(g, s, t);
  Finger f{g, t};
  f.hold(false, 0, 0, 500); f.tap(40, 27); f.tap(80, 77);
  assert(!strcmp(g.screenName(), "confirm_farm"));
  const Save before = taken(g);
  f.hold(true, 115, 102, 2100); f.hold(false, 115, 102, 500);
  assert(!strcmp(g.screenName(), "farm") && g.saveUrgent());
  f.tap(80, 137);
  assert(!strcmp(g.screenName(), "home"));
  const Save gone = taken(g); assert(gone.hasDog == 0 && gone.wallCount == 2);
  f.tap(43, 103);   // a slow double tap's second press, 560 ms into the room: past the fresh gate, not taken
  assert(!strcmp(g.screenName(), "home") && !g.saveUrgent());
  f.hold(false, 0, 0, 500);
  f.hold(true, 43, 103, 60); f.hold(false, 43, 103, 20);   // the release frame: home, before the dog's first idle roll
  assert(!strcmp(g.screenName(), "home") && g.saveUrgent());
  const Save back = taken(g);
  assert(back.hasDog == 1 && !strcmp(back.petName, "Rex") && back.breed == 5 && back.hat == 0);
  sameDog(back, before); assert(back.wallCount == 1 && !strcmp(back.wallNames[0], "A") && back.dogsRaised == 1);
  // a restart in the empty room: Bring back is still there
  enterAt(g, gone, t + 600);
  Finger r{g, t + 600}; r.hold(false, 0, 0, 500);
  r.hold(true, 43, 103, 2500); r.hold(false, 43, 103, 20);   // a long press works too
  const Save again = taken(g);
  assert(!strcmp(g.screenName(), "home") && again.hasDog == 1 && again.lastSeen == t + 600 && again.wallCount == 1);
  sameDog(again, before);
}

// Start over's Yes erases the pup, so like the farm's it takes a 2 s hold: a tap or a kid's slow 1 s press only
// wiggles "Hold Yes"; No! acts on release however long it is held.
static void startOverTakesAHold() {
  static Game g;
  const uint32_t t = day0 + 3 * 86400;
  Save s = dogWithWall(0); s.lastSeen = t; s.lastPlayDay = (uint16_t)pet::dayIndex(t); pet::seal(s);
  enterAt(g, s, t);
  Finger f{g, t};
  auto ask = [&] { f.hold(false, 0, 0, 500); f.tap(40, 27); f.hold(true, 80, 148, 700); f.hold(false, 80, 148, 500); };
  ask(); assert(!strcmp(g.screenName(), "confirm_reset"));
  f.hold(true, 45, 102, 1500); f.hold(false, 45, 102, 500); assert(!strcmp(g.screenName(), "stats"));   // a slow No!
  f.tap(80, 12); ask();
  f.tap(115, 102); assert(!strcmp(g.screenName(), "confirm_reset"));
  f.hold(true, 115, 102, 1000); f.hold(false, 115, 102, 500); assert(!strcmp(g.screenName(), "confirm_reset"));
  f.hold(true, 115, 102, 2100); assert(!strcmp(g.screenName(), "intro"));
}

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
  corruptFieldsLoadSafe();
  levels(s);
  dayOnPickedLight();
  breedDraws();
  retiredSleeperLeavesDay();
  emptyRoomCelebratesWithoutDog();
  petting(day0);
  bringBackRestores();
  adoptionEndsBringBack();
  bringBackOntoFullWall();
  bringBackFromTheEmptyRoom();
  startOverTakesAHold();
  printf("test_pet: all checks passed (sizeof Save = %zu)\n", sizeof(Save));
  return 0;
}
