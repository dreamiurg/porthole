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
#include "games/biscuit/review.h"

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
  assert(!strcmp(word, q.solved) && q.clue[0] && strchr(q.ask, '_') == strrchr(q.ask, '_'));
  const size_t n = strlen(word);
  assert(q.item.level == 3 ? n >= 7 : q.item.level == 2 ? n >= 5 && n <= 6 : n == 3u + q.item.level);
  for (int i = 0; i < biscuit::puzzles::ANSWERS; i++) {   // no wrong letter spells another word
    if (i == q.right) continue;
    char other[24];
    snprintf(other, sizeof other, "%s", q.solved);
    other[q.item.b] = q.answers[i][0];
    assert(!biscuit::puzzles::detail::listed(other));
  }
}
static bool between(int v, int lo, int hi) { return lo <= v && v <= hi; }
// The kind of numbers each level asks: within ten, across ten, two-digit and one-digit without and with carrying,
// then two two-digit numbers.
static bool plusShape(int level, int a, int b) {
  switch (level) {
    case 0: return a + b <= 10;
    case 2: return a <= 9 && b <= 9 && a + b > 10;
    case 3: return a >= 10 && b <= 9 && a % 10 + b < 10;
    case 4: return a >= 10 && b <= 9 && a % 10 + b >= 10;
    case 5: return a >= 10 && b >= 10;
    default: return false;
  }
}
static bool minusShape(int level, int a, int b) {
  switch (level) {
    case 1: return a <= 10;
    case 2: return a > 10 && a <= 18 && b <= 9 && a - b < 10;
    case 3: return a >= 10 && b <= 9 && a % 10 >= b;
    case 4: return a >= 10 && b <= 9 && a % 10 < b;
    case 5: return a >= 10 && b >= 10;
    default: return false;
  }
}
// A plus or minus question: the right answer is the result, nothing negative, and the level's own kind of numbers.
static void checkNumber(const biscuit::puzzles::Puzzle& q) {
  int a, b; char op;
  assert(sscanf(q.ask, "%d %c %d", &a, &op, &b) == 3 && !q.clue[0]);
  const int want = op == '+' ? a + b : a - b;
  assert(atoi(q.answers[q.right]) == want && between(want, 0, 99));
  assert(op == '+' ? plusShape(q.item.level, a, b) : minusShape(q.item.level, a, b));
}
// A times story: groups ask the product, sharing the size of a share.
static void checkGroups(const biscuit::puzzles::Puzzle& q) {
  using namespace biscuit::puzzles;
  const Item& i = q.item;
  assert(q.clue[0] && strchr(q.clue, '.'));
  const int want = i.c & 16 ? i.b : i.a * i.b;
  assert(atoi(q.answers[q.right]) == want);
  if (i.level == 0) assert(!(i.c & 16) && between(i.a, 2, 3) && between(i.b, 2, 4));
  else if (i.level <= 3) {
    static constexpr int SIZES[3][2] = {{2, 10}, {3, 4}, {6, 9}};
    const int* size = SIZES[i.level - 1];
    assert(!(i.c & 16) && between(i.b, size[0], size[1]) && (i.level > 1 || i.b == 2 || i.b == 5 || i.b == 10));
  } else assert(i.c & 16);
}
// One made-up question: three different answers, none negative, the right one right; the item it came from plays,
// and rebuilds as the same question.
static void checkQuestion(const biscuit::puzzles::Puzzle& q, int strand, int level) {
  using namespace biscuit::puzzles;
  assert(q.right < ANSWERS && q.item.strand == strand && q.item.level == level && playable(q.item));
  for (int i = 0; i < ANSWERS; i++) {
    assert(atoi(q.answers[i]) >= (i == q.right || strand == WORDS ? 0 : 1));   // a wrong 0 is too easy
    for (int j = i + 1; j < ANSWERS; j++) assert(strcmp(q.answers[i], q.answers[j]));
  }
  if (strand == WORDS) checkSpelling(q);
  else if (strand == GROUPS) checkGroups(q);
  else checkNumber(q);
  uint32_t other = 99;
  const Puzzle again = build(q.item, other);
  assert(!strcmp(again.ask, q.ask) && !strcmp(again.solved, q.solved) && !strcmp(again.clue, q.clue) && again.key == q.key);
}
// Made-up questions at every level of every strand; a round never asks one twice; a damaged item never plays.
static void puzzleQuestions() {
  using namespace biscuit::puzzles;
  uint32_t rng = 7;
  for (int strand = 0; strand < STRAND_COUNT; strand++)
    for (uint8_t level = 0; level < LEVELS[strand]; level++)
      for (int n = 0; n < 2000; n++) checkQuestion(make((Strand)strand, level, rng, 0), strand, level);
  uint32_t seen = 0;
  for (int n = 0; n < ROUND; n++) { const Puzzle q = make(GROUPS, 1, rng, seen); assert(!(seen & (1u << q.key))); seen |= 1u << q.key; }
  assert(!playable({STRAND_COUNT, 0, 1, 1, 0}) && !playable({NUMBERS, LEVELS[NUMBERS], 1, 1, 0}));
  assert(!playable({NUMBERS, 1, 3, 5, 1}) && !playable({NUMBERS, 0, 60, 50, 0}) && !playable({NUMBERS, 0, 3, 0, 0}));
  assert(!playable({GROUPS, 0, 1, 5, 0}) && !playable({GROUPS, 0, 3, 5, (uint8_t)biscuit::GROUP_THING_COUNT}));
  assert(!playable({WORDS, 2, (uint8_t)biscuit::LONG_WORD_COUNT, 0, 0}) && !playable({WORDS, 0, 0, 9, 0}));
  assert(keyOf({WORDS, 0, 3, 0, 0}) == keyOf({WORDS, 0, 3, 2, 0}) && keyOf({GROUPS, 1, 3, 5, 0}) == keyOf({GROUPS, 1, 3, 5, 2}));
}
// Every word is asked at some level, at a gap whose wrong letters spell no word.
static void puzzleWords() {
  using namespace biscuit::puzzles;
  for (const biscuit::SpellWord& w : biscuit::SHORT_WORDS) {
    bool asked = false;
    for (int gap = 0; w.word[gap]; gap++) asked |= detail::askable(w.word, gap);
    assert(asked);
  }
  for (const biscuit::SpellWord& w : biscuit::LONG_WORDS) {
    bool asked = false;
    for (int gap = 0; w.word[gap]; gap++) asked |= detail::askable(w.word, gap);
    assert(asked && (detail::fits(2, w.word) || detail::fits(3, w.word)));
  }
  assert(!detail::askable("bug", 1));   // bag, beg, big, bog: no wrong vowel is left
}
// Levels: the age a kid starts at, and a round's five answers moving it (up on five, down on two or fewer, never off
// the ladder).
static void startLevels() {
  using namespace biscuit::puzzles;
  assert(startLevel(NUMBERS, 6) == 0 && startLevel(NUMBERS, 0) == 0 && startLevel(NUMBERS, 7) == 2 && startLevel(NUMBERS, 8) == 4);
  assert(startLevel(GROUPS, 7) == 0 && startLevel(GROUPS, 8) == 1 && startLevel(GROUPS, 9) == 2);
  assert(startLevel(WORDS, 6) == 0 && startLevel(WORDS, 8) == 2 && startLevel(WORDS, 9) == 3);
}
static void puzzleLevels() {
  using namespace biscuit::puzzles;
  startLevels();
  for (int strand = 0; strand < STRAND_COUNT; strand++) {
    const Strand st = (Strand)strand;
    assert(startLevel(st, 10) < LEVELS[st] && startLevel(st, 99) < LEVELS[st]);
    assert(nextLevel(st, 0, 5) == 1 && nextLevel(st, 1, 4) == 1 && nextLevel(st, 1, 3) == 1 && nextLevel(st, 1, 2) == 0);
    assert(nextLevel(st, 0, 0) == 0 && nextLevel(st, LEVELS[st] - 1, 5) == LEVELS[st] - 1);
  }
  // one question of a round comes from the level below; at the bottom, all five are at the level
  for (int n = 0; n < ROUND; n++) { assert(roundLevel(3, n, 2) == (n == 2 ? 2 : 3)); assert(roundLevel(0, n, 2) == 0); }
}
// A v3 save (before levels): every strand never played, nothing to review.
static void puzzlesFromAV3Save() {
  Save old = namedPet(Now);
  biscuit::seal(old);
  uint8_t v3[biscuit::SAVE_V3_SIZE];
  old.version = 3; old.size = biscuit::SAVE_V3_SIZE;
  memcpy(v3, &old, sizeof v3 - 4);
  const uint32_t crc = os::crc32(v3, sizeof v3 - 4);
  memcpy(v3 + sizeof v3 - 4, &crc, 4);
  Save out;
  assert(biscuit::loadBlob(v3, sizeof v3, out) && out.version == biscuit::SAVE_VERSION && out.size == sizeof(Save));
  for (uint8_t l : out.puzzleLevels) assert(!l);
  assert(!out.puzzleReviewCount && !out.puzzleReviews[0].due);
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

// The review queue: a miss is due tomorrow, asked in its strand from then on, off the queue when answered right
// while due, a day later when missed again; a full queue drops its oldest; a damaged count or item never plays.
static void puzzleReviews() {
  using namespace biscuit::puzzles;
  namespace rv = biscuit::review;
  Save p = namedPet(Now);
  const int32_t today = 20000;
  const Item sum = {NUMBERS, 2, 8, 5, 0}, story = {GROUPS, 1, 3, 4, 0};
  rv::answered(p, sum, true, today);
  assert(p.puzzleReviewCount == 0);
  rv::answered(p, sum, false, today);
  rv::answered(p, story, false, today);
  assert(p.puzzleReviewCount == 2 && p.puzzleReviews[0].due == today + 1 && p.puzzleReviews[0].misses == 1);
  assert(rv::due(p, NUMBERS, today) < 0 && rv::due(p, NUMBERS, today + 1) == 0 && rv::due(p, GROUPS, today + 5) == 1);
  rv::answered(p, sum, true, today);   // not due yet: right today keeps it for tomorrow
  assert(p.puzzleReviewCount == 2);
  rv::answered(p, sum, false, today + 1);
  assert(p.puzzleReviewCount == 2 && p.puzzleReviews[0].misses == 2 && p.puzzleReviews[0].due == today + 2);
  rv::answered(p, sum, true, today + 3);
  assert(p.puzzleReviewCount == 1 && rv::same(p.puzzleReviews[0], story) && !p.puzzleReviews[1].due);
  for (uint8_t a = 1; a <= biscuit::PUZZLE_REVIEWS; a++) rv::answered(p, {NUMBERS, 0, a, 1, 0}, false, today);
  assert(p.puzzleReviewCount == biscuit::PUZZLE_REVIEWS && rv::find(p, story) < 0 && p.puzzleReviews[0].a == 1);
  p.puzzleReviewCount = 200;   // damaged: read as full, never past the queue
  assert(rv::count(p) == biscuit::PUZZLE_REVIEWS && rv::due(p, NUMBERS, today + 1) == 0);
  p.puzzleReviews[0].a = 200;   // an item that no longer plays is skipped
  assert(rv::due(p, NUMBERS, today + 1) == 1 && rv::due(p, WORDS, today + 1) < 0);
}
// After a miss, one line on how to solve it, for every kind of question; none for words.
static void puzzleHow() {
  using namespace biscuit::puzzles;
  struct Case { Item q; const char* want; };
  static const Case CASES[] = {
    {{NUMBERS, 0, 3, 5, 0}, "Start at 5, count on 3."}, {{NUMBERS, 2, 8, 5, 0}, "8 + 2 = 10, then 3 more."},
    {{NUMBERS, 3, 43, 5, 0}, "3 + 5 = 8, plus the 40 is 48."}, {{NUMBERS, 4, 41, 9, 0}, "1 + 9 = 10, plus the 40 is 50."},
    {{NUMBERS, 5, 36, 47, 0}, "36 + 40 = 76, then 7 more."}, {{NUMBERS, 1, 9, 3, 1}, "Count back 3 from 9."},
    {{NUMBERS, 2, 13, 5, 1}, "13 - 3 = 10, then 2 less."}, {{NUMBERS, 3, 47, 3, 1}, "7 - 3 = 4, plus the 40 is 44."},
    {{NUMBERS, 4, 30, 7, 1}, "Count back 7 from 30."}, {{NUMBERS, 5, 72, 30, 1}, "Count back 3 tens from 72."}, {{NUMBERS, 5, 50, 28, 1}, "50 - 20 = 30, then 8 less."},
    {{NUMBERS, 5, 10, 50, 0}, "Count on 5 tens from 10."}, {{NUMBERS, 4, 47, 5, 0}, "47 + 3 = 50, then 2 more."},
    {{NUMBERS, 4, 40, 3, 0}, "40 and 3 make 43."}, {{NUMBERS, 0, 0, 10, 0}, "10 + 0 is still 10."},
    {{NUMBERS, 1, 3, 3, 1}, "Take all 3 away, and 0 is left."},
    {{GROUPS, 1, 3, 4, 0}, "4 + 4 + 4 = 12"}, {{GROUPS, 4, 3, 4, 16}, "3 x 4 = 12, so 4 each."},
    {{WORDS, 0, 0, 1, 0}, ""},
  };
  for (const Case& c : CASES) { char out[48]; how(c.q, out); if (strcmp(out, c.want)) printf("how: got \"%s\" want \"%s\"\n", out, c.want); assert(!strcmp(out, c.want)); }
  uint32_t rng = 3;   // every question at every level has a line that fits its buffer
  for (int strand = 0; strand < WORDS; strand++)
    for (uint8_t level = 0; level < LEVELS[strand]; level++)
      for (int n = 0; n < 500; n++) {
        char out[48];
        const Item q = pick((Strand)strand, level, rng);
        how(q, out);
        assert(out[0] && strlen(out) < sizeof out - 1);
        char sum[16];   // never just the sum again
        snprintf(sum, sizeof sum, "= %d.", q.c ? q.a - q.b : q.a + q.b);
        assert(strand != NUMBERS || !strstr(out, sum));
      }
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
  puzzleLevels();
  puzzleWords();
  puzzlesFromAV3Save();
  puzzleReviews();
  puzzleHow();
  legacyV1();
  legacyV2V3();
  legacyRejects();
  printf("test_biscuit: all checks passed (sizeof Save = %zu)\n", sizeof(Save));
  return 0;
}
