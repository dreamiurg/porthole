// Self-check for Biscuit's rules (games/biscuit/pet.h): naming, care and time away, reading progress, adventures,
// tricks and growth, the save blob, and conversion of the save Biscuit had before Porthole. The cases are the old
// model's own test suite, ported; the daily play budget cases went to the shell with the budget.
// Run: make test
#include "host/check.h"
#include <math.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "games/biscuit/pet.h"
#include "games/biscuit/puzzles.h"

using biscuit::Action;
using biscuit::Save;
constexpr uint32_t Now = 1814400000u;    // local seconds
constexpr int32_t Day = 21000;           // == Now / 86400
constexpr uint32_t D = 86400;
static_assert(Now / D == Day, "Now starts day Day");

static Save namedPet(uint32_t now) {
  Save p = biscuit::create(now);
  assert(biscuit::setPetName(p, "Biscuit", now));
  return p;
}
static bool normalized(const char* in, const char* want) {
  char out[biscuit::NAME_LIMIT + 1];
  return biscuit::normalizeName(in, out) ? !strcmp(out, want) : !*want && !out[0];
}

static void nothingBeforeNaming() {
  Save p = biscuit::create(Now);
  assert(biscuit::valid(p) && !p.named && !p.petName[0]);
  biscuit::act(p, Action::Feed, Now);            // nothing counts before the pup has a name
  biscuit::toggleSleep(p, Now);
  biscuit::recordReading(p, Now);
  biscuit::finishStory(p, 0, Now);
  biscuit::discover(p, 0, Now);
  biscuit::practice(p, 0, Now);
  biscuit::tick(p, Now + 30 * D);
  assert(p.fullness == 78 && p.happiness == 86 && p.energy == 80);
  assert(p.friendship == 0 && !p.sleeping && p.stories == 0 && !biscuit::discovered(p, 0));
  assert(p.tricks[0] == 0 && p.daysTogether == 1 && p.updatedAt == Now + 30 * D && p.lastVisitDay == Day + 30);
}

static void nameRules() {
  Save p = biscuit::create(Now);
  assert(normalized("  PiP   Ann  ", "PiP Ann") && normalized("  PiP  ", "PiP"));
  assert(normalized("Mary-Jane", "Mary-Jane") && normalized("D'Arcy", "D'Arcy") && normalized("Abcdefghijkl", "Abcdefghijkl"));
  static const char* const BAD[] = {"", "   ", "Abcdefghijklm", "-Pip", "Pip-", "A--B", "A -B", "A''B", "A\tB", "Pip\n",
                                    "Pip2", "Pip\xc3\xa9", "<Pip>", "Ab2c"};
  for (const char* bad : BAD) {
    assert(normalized(bad, ""));
    assert(!biscuit::setPetName(p, bad, Now) && !p.named && !p.petName[0]);
  }
}

static void firstNaming() {
  Save p = biscuit::create(Now);
  biscuit::tick(p, Now + 30 * D);
  assert(biscuit::setPetName(p, "  D'Arcy ", Now + 32 * D));
  assert(p.named && biscuit::valid(p) && !strcmp(p.petName, "D'Arcy"));
  assert(p.updatedAt == Now + 32 * D && p.lastVisitDay == Day + 32 && p.daysTogether == 1);
  assert(p.dailyCompleted == 0 && p.dailyClaimed == 0);
  biscuit::act(p, Action::Feed, Now + 32 * D);
  assert(p.fullness == 96 && p.daysTogether == 1);   // setup time never decays needs or adds visits
  const uint32_t friendship = p.friendship;
  assert(biscuit::setPetName(p, "Scout", Now + 33 * D) && p.friendship == friendship && p.careCounts[0] == 1);
  assert(p.updatedAt == Now + 32 * D && p.lastVisitDay == Day + 32);   // a rename is not a visit
  assert(!biscuit::setPetName(p, "Invalid!", Now) && !strcmp(p.petName, "Scout"));
}

static void nameValidity() {
  Save partial = biscuit::create(Now);
  partial.named = 1;
  assert(!biscuit::valid(partial));                  // named without a name
  partial = namedPet(Now); partial.named = 0;
  assert(!biscuit::valid(partial));                  // a name without named
  partial = namedPet(Now); memset(partial.petName, 'A', sizeof partial.petName);
  assert(!biscuit::valid(partial));                  // unterminated
  partial = namedPet(Now); strcpy(partial.petName, " Pip");
  assert(!biscuit::valid(partial));                  // not normalized
}

static void renamePreservesCompanion() {
  Save p = namedPet(Now);
  biscuit::act(p, Action::Feed, Now);
  biscuit::finishStory(p, 0, Now);
  const Save before = p;
  assert(biscuit::setPetName(p, "  Maple  ", Now + 5 * D) && !strcmp(p.petName, "Maple"));
  memcpy(p.petName, before.petName, sizeof p.petName);
  assert(!memcmp(&p, &before, sizeof p));
  assert(!biscuit::setPetName(p, "Invalid!", Now) && !memcmp(&p, &before, sizeof p));
}

static void careOncePerDay() {
  Save p = namedPet(Now);
  biscuit::act(p, Action::Feed, Now);
  assert(p.fullness == 96 && p.careCounts[0] == 1 && p.friendship == 2);
  biscuit::act(p, Action::Feed, Now);
  assert(p.fullness == 100 && p.careCounts[0] == 2 && p.friendship == 2);   // friendship once a day per activity
}

static void timeAway() {
  Save absent = namedPet(Now);
  biscuit::tick(absent, Now + 30 * D);                // 30 days away decay like 8 hours
  assert(absent.fullness == 46 && absent.happiness == 70 && absent.energy == 56);
  assert(absent.daysTogether == 2);                   // visits, not missed calendar days
  biscuit::tick(absent, Now);                         // a clock set back changes nothing
  assert(absent.fullness == 46 && absent.updatedAt == Now + 30 * D && absent.daysTogether == 2);
  for (uint32_t i = 1; i <= 10; i++) biscuit::tick(absent, Now + (30 + i) * D);
  assert(absent.fullness == 20 && absent.happiness == 20 && absent.energy == 20);   // floors, never below
  assert(biscuit::valid(absent));
}

static void sleeping() {
  Save p = namedPet(Now);
  biscuit::act(p, Action::Feed, Now);
  biscuit::act(p, Action::Feed, Now);
  biscuit::toggleSleep(p, Now);
  const uint32_t friendship = p.friendship;
  biscuit::act(p, Action::Play, Now);                 // a sleeping pup does nothing
  biscuit::finishStory(p, 0, Now);
  biscuit::discover(p, 0, Now);
  biscuit::practice(p, 0, Now);
  assert(p.careCounts[1] == 0 && p.stories == 0 && !biscuit::discovered(p, 0) && p.tricks[0] == 0);
  assert(p.friendship == friendship);
  biscuit::tick(p, Now + 3600);
  assert(p.fullness == 98 && p.energy == 100 && p.happiness == 86);   // asleep: slower hunger, rest, no boredom
  biscuit::toggleSleep(p, Now + 3600);
  assert(!p.sleeping && biscuit::valid(p));
}

static void permanentReadingProgress() {
  Save p = namedPet(Now);
  biscuit::finishStory(p, 0, Now);
  const float happiness = p.happiness;
  biscuit::finishStory(p, 0, Now);
  assert(biscuit::earnedStars(p) == 3 && p.happiness == happiness && p.friendship == 2);   // stars are idempotent
  for (uint8_t id = 1; id < biscuit::NUM_STORIES; id++) biscuit::finishStory(p, id, Now);
  assert(biscuit::earnedStars(p) == 45 && p.stories == 0x7f && p.moreStories == 0xff);
  for (uint8_t id = 0; id < biscuit::NUM_DISCOVERIES; id++) biscuit::discover(p, id, Now);
  for (uint32_t bits : p.discoveries) assert(bits == 0xffffffffu);
  biscuit::discover(p, 95, Now);
  biscuit::discover(p, 96, Now);                      // out of range: no-op
  biscuit::finishStory(p, 15, Now);
  assert(p.friendship == 2 && biscuit::earnedStars(p) == 45 && !biscuit::discovered(p, 96) && p.moreStories == 0xff);

  const Save saved = p;
  biscuit::tick(p, Now + D);
  assert(!memcmp(p.discoveries, saved.discoveries, sizeof p.discoveries) && p.stories == saved.stories && p.moreStories == saved.moreStories);
  assert(p.dailyCompleted == 0);
  biscuit::discover(p, 95, Now + D);
  assert(p.friendship == 4);                          // re-reading counts toward today's reading, once
  biscuit::discover(p, 0, Now + D);
  assert(p.friendship == 4 && biscuit::valid(p));
}

static void adventuresTricksAndGrowth() {
  Save p = namedPet(Now);
  assert(biscuit::stage(p) == biscuit::Stage::Puppy);
  biscuit::practice(p, 2, Now);                       // locked until day 2
  assert(p.tricks[2] == 0 && p.friendship == 0);
  for (int i = 0; i < 4; i++) biscuit::practice(p, 0, Now);
  assert(p.tricks[0] == 3 && p.friendship == 2);      // mastery saturates at 3

  for (int32_t visit = 0; visit < 12; visit++) {      // every activity every day: all 12 stickers, one per day
    const uint32_t now = Now + (uint32_t)visit * D;
    biscuit::act(p, Action::Feed, now);
    biscuit::act(p, Action::Play, now);
    biscuit::act(p, Action::Petting, now);
    biscuit::recordReading(p, now);
    biscuit::practice(p, 0, now);
    biscuit::toggleSleep(p, now);
    biscuit::toggleSleep(p, now);
    assert(p.dailyCompleted == 63 && p.dailyClaimed == 1);
    assert(p.friendship == (uint32_t)(visit + 1) * 16);
    biscuit::act(p, Action::Feed, now);
    assert(p.friendship == (uint32_t)(visit + 1) * 16);   // the adventure pays once a day
    assert(biscuit::valid(p));
    if (visit == 2) assert(biscuit::stage(p) == biscuit::Stage::YoungPup);
  }
  assert(p.stickers == 0xfff && p.daysTogether == 12 && biscuit::stage(p) == biscuit::Stage::StoryDog);
  for (uint8_t id = 0; id < biscuit::NUM_TRICKS; id++) {
    Save locked = namedPet(Now);
    locked.daysTogether = biscuit::trickUnlockDay(id) - 1u;
    if (locked.daysTogether) { biscuit::practice(locked, id, Now); assert(locked.tricks[id] == 0); }
    locked.daysTogether = biscuit::trickUnlockDay(id);
    biscuit::practice(locked, id, Now);
    assert(locked.tricks[id] == 1);
    assert(biscuit::lesson(id, 0).length == 3 && biscuit::lesson(id, 1).length == 4 && biscuit::lesson(id, 2).length >= 5);
    assert(!memcmp(biscuit::lesson(id, 3).cues, biscuit::lesson(id, 2).cues, sizeof biscuit::Lesson::cues));
  }
  assert(biscuit::lesson(0, 0).cues[0] == biscuit::Cue::Paw && biscuit::lesson(6, 0).length == 0);
  assert(biscuit::trickUnlockDay(6) == 255);
  assert(biscuit::dailyMask(-1) == biscuit::dailyMask(6) && biscuit::dailySticker(-1) == 11);   // negative days wrap
}

static void rejectsBrokenSnapshots() {
  const Save good = namedPet(Now);
  Save p = good; p.energy = NAN; assert(!biscuit::valid(p));
  p = good; p.tricks[5] = 4; assert(!biscuit::valid(p));
  p = good; p.updatedAt = Now - 1; assert(!biscuit::valid(p));
  p = good; p.dailyCompleted = biscuit::dailyMask(Day); assert(!biscuit::valid(p));   // complete but unclaimed
  p = good; p.sleeping = 2; assert(!biscuit::valid(p));
  p = good; p.stickers = 0x1000; assert(!biscuit::valid(p));
  p = good; p.careCounts[1] = biscuit::MAX_COUNT + 1; assert(!biscuit::valid(p));
  p = good; p.moreStories = 0x100; assert(!biscuit::valid(p));          // only eight newer books
  p = good; p.booksDay = 0; assert(!biscuit::valid(p));
  p = good; p.booksDay = p.daysTogether + 1; assert(!biscuit::valid(p));
  p = good; p.puzzles = biscuit::MAX_COUNT + 1; assert(!biscuit::valid(p));
  p = good; p.starsPaid = 1; assert(!biscuit::valid(p));                // more paid in than ever earned
  p = good; p.decor = 1u << biscuit::NUM_DECOR; assert(!biscuit::valid(p));
}

// The newer books open one at a time: on their own days for a young pup, every NEW_BOOK_GAP days from the update for
// a pup that is already past those days. The first seven keep their days.
static void newerBooksArrive() {
  Save p = namedPet(Now);
  assert(p.booksDay == 1 && biscuit::storyDay(p, 0, 1) == 1 && biscuit::storyDay(p, 6, 7) == 7);
  assert(biscuit::storyDay(p, 7, 8) == 8 && biscuit::storyDay(p, 14, 22) == 22);
  p.daysTogether = p.booksDay = 30;
  assert(biscuit::storyDay(p, 6, 7) == 7 && biscuit::storyDay(p, 7, 8) == 30 && biscuit::storyDay(p, 8, 10) == 32);
  assert(biscuit::storyDay(p, 14, 22) == 44);
  // A v1 blob (from before the newer books): they start arriving on the day it is loaded, nothing else changes.
  Save old = namedPet(Now);
  old.daysTogether = 12; old.booksDay = 1; old.stories = 0x15;
  biscuit::seal(old);
  uint8_t v1[biscuit::SAVE_V1_SIZE];
  old.version = 1; old.size = biscuit::SAVE_V1_SIZE;
  memcpy(v1, &old, sizeof v1 - 4);
  const uint32_t crc = os::crc32(v1, sizeof v1 - 4);
  memcpy(v1 + sizeof v1 - 4, &crc, 4);
  Save out;
  assert(biscuit::loadBlob(v1, sizeof v1, out) && out.version == biscuit::SAVE_VERSION && out.size == sizeof(Save));
  assert(out.booksDay == 12 && out.moreStories == 0 && out.puzzles == 0 && out.stories == 0x15 && biscuit::earnedStars(out) == 9);
}

// Every answer counts as today's training; only a right one is kept. A napping pup does no puzzles.
static void puzzleAnswers() {
  Save p = namedPet(Now);
  biscuit::answerPuzzle(p, false, Now);
  assert(p.puzzles == 0 && (p.dailyCompleted & (uint8_t)Action::Train) && p.friendship == 2);
  biscuit::answerPuzzle(p, true, Now);
  biscuit::answerPuzzle(p, true, Now);
  assert(p.puzzles == 2 && p.friendship == 2 && biscuit::valid(p));   // training counts once a day
  assert(biscuit::earnedStars(p) == 0);
  for (int i = 0; i < 3; i++) biscuit::answerPuzzle(p, true, Now);
  assert(p.puzzles == 5 && biscuit::earnedStars(p) == 1);   // a star for every five right
  biscuit::toggleSleep(p, Now);
  biscuit::answerPuzzle(p, true, Now);
  assert(p.puzzles == 5);
}

static void puzzleLessonsOpenTricks(Save& p);
// Every PUZZLES_PER_LESSON right answers are a lesson of the first trick not yet mastered, which opens it early.
static void puzzlesTeachTricks() {
  Save p = namedPet(Now);
  assert(biscuit::trickOpen(p, 0) && !biscuit::trickOpen(p, 2) && !biscuit::trickOpen(p, -1) && !biscuit::trickOpen(p, 6));
  int taught[16], n = 0;
  for (int i = 0; i < 16 * (int)biscuit::PUZZLES_PER_LESSON; i++) {
    const int t = biscuit::answerPuzzle(p, true, Now);
    assert((t >= 0) == ((i + 1) % (int)biscuit::PUZZLES_PER_LESSON == 0));
    if (t >= 0) taught[n++] = t;
    assert(biscuit::answerPuzzle(p, false, Now) == -1);   // a wrong one never teaches
  }
  assert(n == 16 && taught[0] == 0 && taught[2] == 0 && taught[3] == 1 && taught[6] == 2 && taught[15] == 5);
  assert(p.tricks[0] == 3 && p.tricks[2] == 3 && p.tricks[5] == 1 && biscuit::valid(p));
  puzzleLessonsOpenTricks(p);
}
static void puzzleLessonsOpenTricks(Save& p) {
  assert(p.daysTogether < biscuit::trickUnlockDay(5) && biscuit::trickOpen(p, 5));   // open before its day
  biscuit::practice(p, 5, Now);
  assert(p.tricks[5] == 2);                           // and trainable
  memset(p.tricks, 3, sizeof p.tricks);
  for (int i = 0; i < (int)biscuit::PUZZLES_PER_LESSON - 1; i++) biscuit::answerPuzzle(p, true, Now);
  assert(biscuit::answerPuzzle(p, true, Now) == -1);   // all mastered: nothing left to teach
}

static void starsWithoutABalance(const Save& p);
static void doghouseBuys(Save& p, uint32_t& wallet);
// The kid's stars: earned here, paid into the shell's balance once, spent in the doghouse.
static void starsAndDoghouse() {
  Save p = namedPet(Now);
  uint32_t wallet = 7;
  assert(!biscuit::payStars(p, &wallet) && wallet == 7);
  biscuit::markStoryRead(p, 0);
  for (int i = 0; i < 10; i++) biscuit::answerPuzzle(p, true, Now);
  assert(biscuit::earnedStars(p) == 5 && biscuit::payStars(p, &wallet) && wallet == 12 && p.starsPaid == 5);
  assert(!biscuit::payStars(p, &wallet) && wallet == 12);   // paid once
  doghouseBuys(p, wallet);
}
static void doghouseBuys(Save& p, uint32_t& wallet) {
  assert(!biscuit::buyDecor(p, &wallet, 0, 13) && wallet == 12 && !biscuit::owns(p, 0));   // short
  assert(biscuit::buyDecor(p, &wallet, 0, 12) && wallet == 0 && biscuit::owns(p, 0) && biscuit::valid(p));
  wallet = 50;
  assert(!biscuit::buyDecor(p, &wallet, 0, 1) && wallet == 50);   // owned already
  assert(!biscuit::buyDecor(p, &wallet, biscuit::NUM_DECOR, 1) && !biscuit::buyDecor(p, &wallet, -1, 1) && wallet == 50);
  assert(!biscuit::owns(p, -1) && !biscuit::owns(p, biscuit::NUM_DECOR));
  assert(biscuit::buyDecor(p, &wallet, biscuit::NUM_DECOR - 1, 50) && p.decor == (1u | 1u << (biscuit::NUM_DECOR - 1)));
  starsWithoutABalance(p);
}
static void starsWithoutABalance(const Save& p) {
  // no balance (a test, a shell that keeps none): nothing is paid in or bought, and nothing breaks
  Save q = p; q.starsPaid = 0;
  assert(biscuit::payStars(q, nullptr) && q.starsPaid == 5 && !biscuit::buyDecor(q, nullptr, 1, 0));
  // os/stars.h: the balance stops at MAX
  uint32_t full = stars::MAX - 2;
  stars::award(&full, 5); assert(full == stars::MAX);
  stars::award(&full, 1); assert(full == stars::MAX);
  assert(stars::balance(nullptr) == 0 && !stars::spend(nullptr, 0) && stars::spend(&full, stars::MAX) && full == 0);
}
static void starsFromAV2Save() {
  // a v2 save (before the doghouse): nothing paid in yet, nothing bought; the first visit pays its stars in
  Save old = namedPet(Now);
  biscuit::markStoryRead(old, 1);
  biscuit::seal(old);
  uint8_t v2[biscuit::SAVE_V2_SIZE];
  old.version = 2; old.size = biscuit::SAVE_V2_SIZE;
  memcpy(v2, &old, sizeof v2 - 4);
  const uint32_t crc = os::crc32(v2, sizeof v2 - 4);
  memcpy(v2 + sizeof v2 - 4, &crc, 4);
  Save out;
  assert(biscuit::loadBlob(v2, sizeof v2, out) && out.version == biscuit::SAVE_VERSION && out.starsPaid == 0 && out.decor == 0);
  uint32_t kid = 0;
  assert(biscuit::payStars(out, &kid) && kid == 3);
}

// A spelling question: the right letter fills its one gap and spells the word.
static void checkSpelling(const biscuit::puzzles::Puzzle& q) {
  char word[24]; int w = 0;
  for (const char* c = q.ask; *c; c++) if (*c != ' ') word[w++] = *c == '_' ? q.answers[q.right][0] : *c;
  word[w] = 0;
  assert(!strcmp(word, q.solved) && q.clue && strchr(q.ask, '_') == strrchr(q.ask, '_'));
}
static bool between(int v, int lo, int hi) { return lo <= v && v <= hi; }
// A number question: the right answer is the sum, nothing negative, the numbers in the kind's range.
static void checkNumber(const biscuit::puzzles::Puzzle& q, int kind, int table) {
  using namespace biscuit::puzzles;
  int a, b; char op;
  assert(sscanf(q.ask, "%d %c %d", &a, &op, &b) == 3 && !q.clue);
  const int want = op == '+' ? a + b : op == '-' ? a - b : a * b;
  assert(atoi(q.answers[q.right]) == want);
  for (int i = 0; i < ANSWERS; i++) assert(atoi(q.answers[i]) >= 0);
  struct Range { int aLo, aHi, bLo, bHi, most; };   // ADD, TAKE, BIG, TIMES
  static constexpr Range RANGES[] = {{0, 10, 1, 10, 20}, {2, 20, 1, 10, 19}, {10, 89, 10, 89, 99},
                                     {FIRST_TABLE, LAST_TABLE, 1, 10, 60}};
  const Range& r = RANGES[kind];
  assert(between(a, r.aLo, r.aHi) && between(b, r.bLo, r.bHi) && between(want, 0, r.most));
  assert(!table || a == table);
}
// Made-up questions: three different answers, the right one right, a spelling gap the missing letter fills.
static void puzzleQuestions() {
  using namespace biscuit::puzzles;
  uint32_t rng = 7;
  for (int kind = 0; kind < KIND_COUNT; kind++)
    for (int n = 0; n < 2000; n++) {
      const uint8_t table = kind == TIMES && n % 6 ? (uint8_t)(n % 6 + 1) : 0;
      const Puzzle q = make((Kind)kind, table, rng, 0);
      assert(q.right < ANSWERS);
      for (int i = 0; i < ANSWERS; i++)
        for (int j = i + 1; j < ANSWERS; j++) assert(strcmp(q.answers[i], q.answers[j]));
      if (kind >= SHORT) checkSpelling(q);
      else checkNumber(q, kind, table);
    }
  uint32_t seen = 0;   // a round doesn't ask the same times-table question twice
  for (int n = 0; n < ROUND; n++) { const Puzzle q = make(TIMES, 3, rng, seen); assert(!(seen & (1u << q.key))); seen |= 1u << q.key; }
}

static void blobRoundTrip() {
  Save p = namedPet(Now);
  biscuit::finishStory(p, 3, Now);
  biscuit::seal(p);
  Save out;
  assert(biscuit::loadBlob(&p, sizeof p, out) && !memcmp(&out, &p, sizeof p));
  Save bad = p; bad.friendship ^= 1;
  assert(!biscuit::loadBlob(&bad, sizeof bad, out));                     // crc
  assert(!biscuit::loadBlob(&p, sizeof p - 4, out));                     // truncated
  bad = p; bad.version = biscuit::SAVE_VERSION + 1; bad.crc = os::crc32(&bad, sizeof bad - 4);
  assert(!biscuit::loadBlob(&bad, sizeof bad, out));                     // from a newer firmware
  bad = p; bad.sleeping = 2; biscuit::seal(bad);
  assert(!biscuit::loadBlob(&bad, sizeof bad, out));                     // sealed, but not a playable pup
  bad = p; bad.magic ^= 1; bad.crc = os::crc32(&bad, sizeof bad - 4);
  assert(!biscuit::loadBlob(&bad, sizeof bad, out));                     // not a Biscuit save
  bad = p; bad.size = 92; bad.crc = os::crc32(&bad, sizeof bad - 4);
  assert(!biscuit::loadBlob(&bad, sizeof bad, out));                     // says it is another size
  uint8_t big[sizeof p + 4] = {};
  memcpy(big, &p, sizeof p);
  assert(!biscuit::loadBlob(big, sizeof big, out) && !biscuit::loadBlob(big, sizeof p - 2, out));   // too big, odd
  p.lastVisitDay = biscuit::dayOf(UINT32_MAX) + 1; biscuit::seal(p);
  assert(!biscuit::loadBlob(&p, sizeof p, out));                         // a day the clock can never reach
}

// Old-firmware blobs, built as that firmware wrote them: {magic, pet, FNV-1a over pet}, stamps in UTC.
template <class P> static biscuit::legacy::Blob<P> wrap(uint32_t magic, const P& pet) {
  biscuit::legacy::Blob<P> b;
  memset(&b, 0, sizeof b);
  b.magic = magic; b.pet = pet; b.hash = biscuit::legacy::fnv1a(&b.pet, sizeof b.pet);
  return b;
}
static biscuit::legacy::PetV1 oldV1() {
  const uint64_t utc = Now + 8ull * 3600;   // the old clock ran on UTC; local is 8 hours behind it (PST)
  return {utc - D, utc, 1, Day, 55, 66, 77, 111, 12, {4, 5, 6}, {0xffffffffu, 0x12345678u, 0xabcdef01u}, 0xfff, 0x7f,
          {1, 2, 3, 3, 2, 1}, 63, 1, 1};
}

static void legacyV1() {
  const biscuit::legacy::PetV1 old = oldV1();
  auto blob = wrap(biscuit::legacy::MAGIC_V1, old);
  Save p; char kid[biscuit::NAME_LIMIT + 1] = "junk";
  assert(biscuit::fromLegacy(&blob, sizeof blob, p, kid) && biscuit::valid(p));
  assert(!kid[0] && p.named && !strcmp(p.petName, "Biscuit"));   // v1 never knew the kid's name
  assert(p.createdAt == Now - D && p.updatedAt == Now && p.lastVisitDay == old.lastVisitDay);   // UTC -> local
  assert(p.fullness == old.fullness && p.happiness == old.happiness && p.energy == old.energy);
  assert(p.friendship == old.friendship && p.daysTogether == old.daysTogether);
  assert(!memcmp(p.careCounts, old.careCounts, sizeof p.careCounts) && !memcmp(p.discoveries, old.discoveries, sizeof p.discoveries));
  assert(p.stickers == old.stickers && p.stories == old.stories && !memcmp(p.tricks, old.tricks, sizeof p.tricks));
  assert(p.dailyCompleted == old.dailyCompleted && p.dailyClaimed == old.dailyClaimed && p.sleeping == old.sleeping);
  Save out; assert(biscuit::loadBlob(&p, sizeof p, out));   // sealed, ready for the store
}

static void legacyV2V3() {
  const biscuit::legacy::PetV1 old = oldV1();
  Save p; char kid[biscuit::NAME_LIMIT + 1] = "";
  biscuit::legacy::PetV3 v3 = {};
  v3.base = old; v3.base.version = 3; v3.setupComplete = 1; v3.playSeconds = 99;
  strcpy(v3.playerName, "Sam"); strcpy(v3.petName, "Pip");
  auto b3 = wrap(biscuit::legacy::MAGIC_V3, v3);
  assert(biscuit::fromLegacy(&b3, sizeof b3, p, kid) && !strcmp(kid, "Sam") && !strcmp(p.petName, "Pip"));
  assert(p.friendship == old.friendship);
  v3.base.version = 2;
  auto b2 = wrap(biscuit::legacy::MAGIC_V2, v3);
  assert(biscuit::fromLegacy(&b2, sizeof b2, p, kid) && !strcmp(kid, "Sam"));
}

static void legacyRejects() {
  Save p = namedPet(Now); const Save before = p;
  char kid[biscuit::NAME_LIMIT + 1] = "";
  auto check = [&](const void* blob, size_t n) {
    assert(!biscuit::fromLegacy(blob, n, p, kid) && !memcmp(&p, &before, sizeof p));
  };
  biscuit::legacy::PetV1 old = oldV1();
  auto good = wrap(biscuit::legacy::MAGIC_V1, old);
  auto bad = good; bad.hash ^= 1; check(&bad, sizeof bad);                          // checksum
  check(&good, sizeof good - 1);                                                    // size
  bad = good; bad.magic = biscuit::legacy::MAGIC_V3; check(&bad, sizeof bad);       // magic for another size
  old.version = 2; bad = wrap(biscuit::legacy::MAGIC_V1, old); check(&bad, sizeof bad);   // version mismatch
  old = oldV1(); old.energy = NAN; bad = wrap(biscuit::legacy::MAGIC_V1, old); check(&bad, sizeof bad);
  old = oldV1(); old.updatedAt = old.createdAt - 1; bad = wrap(biscuit::legacy::MAGIC_V1, old); check(&bad, sizeof bad);
  old = oldV1(); old.updatedAt = 1ull << 40; bad = wrap(biscuit::legacy::MAGIC_V1, old); check(&bad, sizeof bad);   // past 2106

  biscuit::legacy::PetV3 v3 = {};
  v3.base = oldV1(); v3.base.version = 3; v3.setupComplete = 1;
  strcpy(v3.playerName, "Sam"); strcpy(v3.petName, "Pip");
  auto b3 = wrap(biscuit::legacy::MAGIC_V3, v3);
  assert(biscuit::fromLegacy(&b3, sizeof b3, p, kid));
  p = before;
  auto b = b3; b.pet.setupComplete = 0; b.hash = biscuit::legacy::fnv1a(&b.pet, sizeof b.pet); check(&b, sizeof b);   // never named
  b = b3; strcpy(b.pet.playerName, " Sam"); b.hash = biscuit::legacy::fnv1a(&b.pet, sizeof b.pet); check(&b, sizeof b);
  b = b3; b.pet.petName[0] = 0; b.hash = biscuit::legacy::fnv1a(&b.pet, sizeof b.pet); check(&b, sizeof b);
  b = b3; b.magic = biscuit::legacy::MAGIC_V2; b.hash = biscuit::legacy::fnv1a(&b.pet, sizeof b.pet); check(&b, sizeof b);   // v3 bytes, v2 magic
}

int main() {
  nothingBeforeNaming();
  nameRules();
  firstNaming();
  nameValidity();
  renamePreservesCompanion();
  careOncePerDay();
  timeAway();
  sleeping();
  permanentReadingProgress();
  adventuresTricksAndGrowth();
  rejectsBrokenSnapshots();
  blobRoundTrip();
  newerBooksArrive();
  puzzleAnswers();
  puzzlesTeachTricks();
  starsAndDoghouse();
  starsFromAV2Save();
  puzzleQuestions();
  legacyV1();
  legacyV2V3();
  legacyRejects();
  printf("test_biscuit: all checks passed (sizeof Save = %zu)\n", sizeof(Save));
  return 0;
}
