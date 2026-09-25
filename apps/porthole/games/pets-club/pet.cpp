#include "pet.h"
#include "content.h"
#include "crc32.h"

static_assert(NUM_BOOKS <= 32, "booksDoneMask is 32 bits: split or widen before adding more stories");

namespace pet {
uint32_t crc32(const void* d, size_t n) { return os::crc32(d, n); }
bool valid(const Save& s) {
  return s.magic == SAVE_MAGIC && s.version == SAVE_VERSION && s.size == sizeof(Save) &&
         s.crc == crc32(&s, sizeof(Save) - sizeof(uint32_t));
}
bool loadBlob(const void* d, size_t n, Save& out) {
  if (n == sizeof(Save)) { memcpy(&out, d, n); return valid(out); }
  if (n == SAVE_V1_SIZE) {
    const uint8_t* p = (const uint8_t*)d;
    uint32_t crc; memcpy(&crc, p + SAVE_V1_SIZE - 4, 4);
    uint16_t ver; memcpy(&ver, p + 4, 2);
    if (ver != 1 || crc != crc32(p, SAVE_V1_SIZE - 4)) return false;
    memset(&out, 0, sizeof out);
    memcpy(&out, p, SAVE_V1_SIZE - 4);   // every v1 field keeps its offset; new fields stay zero (age unknown)
    seal(out);
    return true;
  }
  return false;
}
void levelRange(int age, int& lo, int& hi) {
  if (age <= 6) { lo = 1; hi = 1; } else if (age == 7) { lo = 1; hi = 2; } else if (age == 8) { lo = 2; hi = 3; } else { lo = 3; hi = 3; }
}
void wordLenRange(int age, int& lo, int& hi) {
  if (age <= 6) { lo = 3; hi = 4; } else if (age == 7) { lo = 4; hi = 5; } else if (age == 8) { lo = 5; hi = 7; } else { lo = 6; hi = 8; }
}
int bookListFor(const Save& s, uint8_t* out) {
  int lo, hi; levelRange(s.kidAge ? s.kidAge : 7, lo, hi);
  for (;;) {
    int n = 0;
    for (int i = 0; i < NUM_BOOKS; i++) if (BOOKS[i].level >= lo && BOOKS[i].level <= hi) { if (out) out[n] = (uint8_t)i; n++; }
    if (n >= STARTER_BOOKS + 3 || lo <= 1) return n;
    lo--;  // not enough stories at this level yet: offer the tier below as well
  }
}
void seal(Save& s) { s.magic = SAVE_MAGIC; s.version = SAVE_VERSION; s.size = sizeof(Save); s.crc = crc32(&s, sizeof(Save) - sizeof(uint32_t)); }

uint32_t rnd(Save& s) { uint32_t x = s.seed ? s.seed : 0x9E3779B9u; x ^= x << 13; x ^= x >> 17; x ^= x << 5; s.seed = x; return x; }

static uint8_t clamp100(int v) { return (uint8_t)(v < 0 ? 0 : v > 100 ? 100 : v); }
static void decayTo(uint8_t& v, int amount, int floor) { int nv = (int)v - amount; if (nv < floor) nv = v < floor ? v : floor; v = (uint8_t)nv; }

void adopt(Save& s, uint32_t now, const char* kid, const char* pet) {
  memset(&s, 0, sizeof s);
  strncpy(s.kidName, kid, sizeof s.kidName - 1); strncpy(s.petName, pet, sizeof s.petName - 1);
  s.adoptedAt = s.lastSeen = now;
  s.food = 70; s.fun = 80; s.energy = 90; s.clean = 100;
  s.booksUnlocked = STARTER_BOOKS;
  s.streak = 1; s.lastPlayDay = (uint16_t)dayIndex(now); s.lastGiftDay = (uint16_t)dayIndex(now);  // first gift tomorrow
  s.seed = now ^ 0xC0FFEEu;
  s.nextPoopAt = now + POOP_MIN_SEC;
  s.bondDay = (uint16_t)dayIndex(now); s.treatsDay = s.bondDay;
  seal(s);
}

int ageDays(const Save& s, uint32_t now) { return (int)(dayIndex(now) - dayIndex(s.adoptedAt)); }
Stage stageFor(const Save& s, uint32_t now) { int d = ageDays(s, now); return d >= STAGE_GROWN_DAYS ? STAGE_GROWN : d >= STAGE_DOG_DAYS ? STAGE_DOG : STAGE_PUPPY; }
int hearts(const Save& s) { int h = s.bond / 100; return h > 10 ? 10 : h; }

static void schedulePoop(Save& s, uint32_t now) { s.nextPoopAt = now + (uint32_t)rndRange(s, (int)POOP_MIN_SEC, (int)POOP_MAX_SEC); }

uint32_t simulate(Save& s, uint32_t now, bool online) {
  uint32_t ev = EV_NONE;
  if (now <= s.lastSeen) { s.lastSeen = now; return ev; }
  uint32_t elapsed = now - s.lastSeen;
  if (elapsed > 300) online = false;  // a jump this big means the device was off (or the clock was set)
  if (!online && elapsed > 3600) ev |= EV_LONG_AWAY;
  // Only the first OFFLINE_CAP_SEC of an absence count for decay; beyond that the pet just waits.
  uint32_t budget = online ? elapsed : (elapsed > OFFLINE_CAP_SEC ? OFFLINE_CAP_SEC : elapsed);
  uint32_t t = s.lastSeen;
  // accumulate fractional decay in 1/3600ths so short online ticks still add up
  static uint32_t accFood = 0, accFun = 0, accEnergy = 0, accClean = 0, accSleep = 0;
  while (t < now) {
    uint32_t step = now - t; if (step > 600) step = 600;
    bool counted = budget > 0; if (counted) budget = budget > step ? budget - step : 0;
    int h = hourOf(t);
    // schedule-driven sleep when unattended
    if (!online) {
      if (isNightHour(h) && !s.asleep) { s.asleep = 1; ev |= EV_FELL_ASLEEP; }
      if (!isNightHour(h) && s.asleep && s.energy >= 90) { s.asleep = 0; ev |= EV_WOKE; }
    }
    if (counted) {
      if (s.asleep) {
        accFood += step * 1; accFun += step * 1;
        accSleep += step * (uint32_t)(online ? SLEEP_ENERGY_ONLINE * 60 : SLEEP_ENERGY_OFFLINE);
      } else {
        accFood += step * DECAY_FOOD; accFun += step * DECAY_FUN; accEnergy += step * DECAY_ENERGY; accClean += step * DECAY_CLEAN;
        if (s.poop && t - s.poopSince > 2 * 3600) accFun += step * 3;
      }
    }
    if (!s.asleep && !s.poop && t >= s.nextPoopAt) { s.poop = 1; s.poopSince = t; s.clean = clamp100(s.clean - 15); ev |= EV_POOPED; }
    t += step;
  }
  while (accFood >= 3600) { decayTo(s.food, 1, FLOOR_FOOD); accFood -= 3600; }
  while (accFun >= 3600) { decayTo(s.fun, 1, FLOOR_FUN); accFun -= 3600; }
  while (accEnergy >= 3600) { decayTo(s.energy, 1, FLOOR_ENERGY); accEnergy -= 3600; }
  while (accClean >= 3600) { decayTo(s.clean, 1, FLOOR_CLEAN); accClean -= 3600; }
  while (accSleep >= 3600) { s.energy = clamp100(s.energy + 1); accSleep -= 3600; }
  if (s.asleep && s.energy >= 100 && !isNightHour(hourOf(now))) { s.asleep = 0; ev |= EV_WOKE; }
  Stage st = stageFor(s, now);
  if (st != s.stage) {
    s.stage = st; ev |= EV_STAGE_UP;
    int reading = s.booksRead * 2 + s.wordsSpelled / 5, playing = s.gamesPlayed * 2 + s.bonesCaught / 10;
    s.variant = reading > playing + 2 ? VAR_BOOKWORM : playing > reading + 2 ? VAR_SPORTY : VAR_CLASSIC;
    if (st == STAGE_GROWN) earnSticker(s, ST_GROWN);
  }
  if (dayIndex(now) != s.lastPlayDay) ev |= EV_NEW_DAY;
  s.lastSeen = now;
  return ev;
}

void addBond(Save& s, uint32_t now, int n) {
  uint16_t d = (uint16_t)dayIndex(now);
  if (s.bondDay != d) { s.bondDay = d; s.bondToday = 0; }
  int room = BOND_DAILY_CAP - s.bondToday; if (n > room) n = room; if (n <= 0) return;
  s.bondToday = (uint16_t)(s.bondToday + n);
  int nb = s.bond + n; if (nb > 1000) nb = 1000; s.bond = (uint16_t)nb;
  if (hearts(s) >= 10) earnSticker(s, ST_BESTFRIENDS);
}

bool feed(Save& s, uint32_t now, Food f, int* outDelta) {
  if (s.asleep) return false;
  uint16_t d = (uint16_t)dayIndex(now);
  if (s.treatsDay != d) { s.treatsDay = d; s.treatsToday = 0; }
  int dFood = 0, dFun = 0;
  switch (f) {
    case FOOD_KIBBLE: dFood = 35; break;
    case FOOD_BONE: dFood = 20; dFun = 10; break;
    case FOOD_COOKIE: if (s.treatsToday >= 3) return false; dFood = 10; dFun = 20; s.treatsToday++; break;
  }
  if (s.food >= 100) return false;  // full
  bool wasHungry = s.food < 50;
  s.food = clamp100(s.food + dFood); s.fun = clamp100(s.fun + dFun);
  if (outDelta) *outDelta = dFood;
  addBond(s, now, wasHungry ? 6 : 2);
  if (!s.poop && s.nextPoopAt > now + POOP_AFTER_MEAL_SEC) s.nextPoopAt = now + POOP_AFTER_MEAL_SEC + (uint32_t)rndRange(s, 0, 1800);
  earnSticker(s, ST_FIRST_MEAL);
  return true;
}
bool cleanPoop(Save& s, uint32_t now) {
  if (!s.poop) return false;
  s.poop = 0; s.clean = clamp100(s.clean + 15); schedulePoop(s, now); addBond(s, now, 3); return true;
}
bool bathe(Save& s, uint32_t now) {
  s.dirty = 0; s.clean = 100; s.baths++; addBond(s, now, 6);
  if (s.baths >= 5) earnSticker(s, ST_CLEAN);
  return true;
}
bool petDog(Save& s, uint32_t now) {
  if (s.asleep) return false;
  uint16_t d = (uint16_t)dayIndex(now);
  if (s.treatsDay != d) { s.treatsDay = d; s.treatsToday = 0; s.petsToday = 0; }
  s.petsGiven++;
  s.fun = clamp100(s.fun + 2);
  if (s.petsToday < 20) { s.petsToday++; addBond(s, now, 1); }
  return true;
}
void playResult(Save& s, uint32_t now, int score, bool gotMuddy) {
  s.gamesPlayed++; s.bonesCaught = (uint16_t)(s.bonesCaught + score);
  s.fun = clamp100(s.fun + 20 + score); s.energy = clamp100(s.energy - 15); s.food = clamp100(s.food - 5);
  if (gotMuddy) { s.dirty = 1; s.clean = clamp100(s.clean - 30); }
  addBond(s, now, 8 + (score >= 10 ? 4 : 0));
  if (s.bonesCaught >= 50) earnSticker(s, ST_BONES);
}
void wordsResult(Save& s, uint32_t now, int correct) {
  s.gamesPlayed++; s.wordsSpelled = (uint16_t)(s.wordsSpelled + correct);
  s.fun = clamp100(s.fun + 10 + correct * 3);
  addBond(s, now, 6 + correct * 2);
  if (s.wordsSpelled >= 25) earnSticker(s, ST_SPELLER);
}
void bookFinished(Save& s, uint32_t now, int bookIndex, bool right) {
  bool first = !(s.booksDoneMask & (1u << bookIndex));
  s.booksDoneMask |= (1u << bookIndex);
  if (first) s.booksRead++;
  s.fun = clamp100(s.fun + 15);
  addBond(s, now, (first ? 15 : 4) + (right ? 5 : 0));
  earnSticker(s, ST_FIRST_STORY);
  if (s.booksRead >= 5) earnSticker(s, ST_BOOKWORM);
  if (s.booksRead >= 12) earnSticker(s, ST_LIBRARY);
}
bool canWake(const Save& s) { return s.energy >= 40; }
void setAsleep(Save& s, uint32_t now, bool asleep) { (void)now; s.asleep = asleep ? 1 : 0; }

bool trickLearned(const Save& s, int i) { return s.trickProgress[i] >= 3; }
bool trickAvailable(const Save& s, int i) { return hearts(s) >= TRICK_UNLOCK_HEARTS[i]; }
bool trickLessonToday(const Save& s, uint32_t now, int i) { return s.trickLastDay[i] == (uint16_t)dayIndex(now) && s.trickProgress[i] > 0; }
void trickLessonPassed(Save& s, uint32_t now, int i) {
  if (trickLearned(s, i)) return;
  s.trickProgress[i]++; s.trickLastDay[i] = (uint16_t)dayIndex(now);
  addBond(s, now, 8);
  int n = learnedTrickCount(s);
  if (n >= 3) earnSticker(s, ST_TRICKSTAR);
  if (n >= 6) earnSticker(s, ST_SHOWDOG);
}
int learnedTrickCount(const Save& s) { int n = 0; for (int i = 0; i < NUM_TRICKS; i++) if (trickLearned(s, i)) n++; return n; }

bool giftReady(const Save& s, uint32_t now) { return dayIndex(now) > s.lastGiftDay; }
Gift openGift(Save& s, uint32_t now) {
  Gift g = {GIFT_TREATS, 0};
  s.lastGiftDay = (uint16_t)dayIndex(now);
  int n = s.giftsOpened++;
  int age = ageDays(s, now);
  if (age == 7 && !(s.stickersMask & (1u << ST_PARTY))) {
    earnSticker(s, ST_PARTY); g.kind = GIFT_PARTY; g.index = 2; s.hatsMask |= (1u << 2); addBond(s, now, 10); return g;
  }
  if ((n % 3) == 2) {  // every third gift: a hat, if any remain
    for (int h = 1; h < NUM_HATS; h++) if (!(s.hatsMask & (1u << h))) {
      s.hatsMask |= (1u << h); g.kind = GIFT_HAT; g.index = (uint8_t)h;
      int hats = 0; for (int k = 1; k < NUM_HATS; k++) if (s.hatsMask & (1u << k)) hats++;
      if (hats >= 5) earnSticker(s, ST_HATS);
      addBond(s, now, 10); return g;
    }
  }
  uint8_t list[32]; int nb = bookListFor(s, list);
  if (s.booksUnlocked < nb) { g.kind = GIFT_BOOK; g.index = list[s.booksUnlocked++]; addBond(s, now, 10); return g; }
  s.fun = clamp100(s.fun + 10); s.food = clamp100(s.food + 10); addBond(s, now, 10);
  return g;
}
void touchDay(Save& s, uint32_t now) {
  uint16_t d = (uint16_t)dayIndex(now);
  if (d == s.lastPlayDay) return;
  s.streak = (uint16_t)(d == s.lastPlayDay + 1 ? s.streak + 1 : 1);
  s.lastPlayDay = d; s.treatsToday = 0; s.petsToday = 0; s.treatsDay = d;
  if (s.streak >= 7) earnSticker(s, ST_WEEK);
  if (s.streak >= 14) earnSticker(s, ST_TWO_WEEKS);
  if (hourOf(now) < 8) earnSticker(s, ST_EARLYBIRD);
}
bool earnSticker(Save& s, int id) {
  if (s.stickersMask & (1u << id)) return false;
  s.stickersMask |= (uint16_t)(1u << id); return true;
}
Want computeWant(const Save& s, uint32_t now) {
  if (s.asleep) return WANT_NONE;
  if (s.poop && now - s.poopSince > 600) return WANT_POOP;
  int lowest = 101; Want w = WANT_NONE;
  if (s.food < 45 && s.food < lowest) { lowest = s.food; w = WANT_FOOD; }
  if (s.energy < 30 && s.energy < lowest) { lowest = s.energy; w = WANT_SLEEP; }
  if (s.dirty || (s.clean < 35 && s.clean < lowest)) { lowest = s.clean; w = WANT_BATH; }
  if (s.fun < 45 && s.fun < lowest) { lowest = s.fun; w = WANT_PLAY; }
  if (w == WANT_NONE && isNightHour(hourOf(now)) && s.energy < 80) w = WANT_SLEEP;   // bedtime nudge
  if (w == WANT_NONE && s.fun < 70 && ((now / 1800u) % 3u) == 0u) w = WANT_STORY;
  return w;
}
}  // namespace pet
