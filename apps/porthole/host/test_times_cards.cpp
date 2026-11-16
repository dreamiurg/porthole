// Times Cards' self-check: the deck's rules (games/times-cards/deck.h): where a kid starts, how each grade moves a card,
// that only a remembered due card moves up, a round's makeup and a Wrong card coming back, the level opening the next
// table, the save and a clock set back; then the pages through the App interface, each one drawn.
#include "host/check.h"
#include <stdio.h>
#include <string.h>
#include "games/times-cards/deck.h"
#include "games/times-cards/game.h"
#include "host/fixtures/times_cards_save_v3.h"
#include "gfx565.h"

using namespace cards;
static int checks = 0;
constexpr int32_t DAY = 20000;

static void start() {
  CHECK(startTable(6) == 2 && startTable(8) == 4 && startTable(5) == 1 && startTable(0) == 1 && startTable(11) == 4);
  Save s;
  setUp(s, 4, DAY);
  CHECK(s.level == 4 && s.start == 4);
  for (int c = 0; c < CARDS; c++) {
    if (tableOf(c) < 4) CHECK(s.boxOf(c) == KNOWN_BOX && s.dueOf(c) > DAY && s.dueOf(c) <= DAY + INTERVAL[KNOWN_BOX]);
    else CHECK(s.boxOf(c) == 0);
  }
  CHECK(known(s, 3) == FACTS && known(s, 4) == 0);
  CHECK(answerOf(cardOf(7, 8)) == 56 && tableOf(cardOf(7, 8)) == 7 && factorOf(cardOf(7, 8)) == 8);
}

static void grades() {
  Save s;
  setUp(s, 2, DAY);
  const int c = cardOf(2, 3);
  grade(s, c, EASY, DAY);   // new and known at once: box 2, back in 2 days
  CHECK(s.boxOf(c) == 2 && s.dueOf(c) == DAY + 2);
  grade(s, c, EASY, DAY);   // the same day again (practice): no move
  CHECK(s.boxOf(c) == 2 && s.dueOf(c) == DAY + 2);
  grade(s, c, HARD, DAY);   // early Hard: no later than tomorrow, box kept
  CHECK(s.boxOf(c) == 2 && s.dueOf(c) == DAY + 1);
  grade(s, c, EASY, DAY + 1);   // still learning, another day: up to the mastery box
  CHECK(s.boxOf(c) == MASTER_BOX && s.dueOf(c) == DAY + 1 + INTERVAL[MASTER_BOX]);
  grade(s, c, EASY, DAY + 2);   // known and not due: no move
  CHECK(s.boxOf(c) == MASTER_BOX && s.dueOf(c) == DAY + 5);
  grade(s, c, HARD, DAY + 5);   // due Hard: kept, tomorrow
  CHECK(s.boxOf(c) == MASTER_BOX && s.dueOf(c) == DAY + 6);
  grade(s, c, WRONG, DAY + 5);   // Wrong: back to learning, tomorrow
  CHECK(s.boxOf(c) == 1 && s.dueOf(c) == DAY + 6);
  const int d = cardOf(2, 4);
  grade(s, d, HARD, DAY);
  CHECK(s.boxOf(d) == 1 && s.dueOf(d) == DAY + 1);
  for (int i = 0; i < 20; i++) grade(s, d, EASY, DAY + 1000 * (i + 1));   // never past the top box
  CHECK(s.boxOf(d) == MAX_BOX);
  CHECK(s.answers == 28);
}

static void levelUp() {
  Save s;
  setUp(s, 2, DAY);
  int32_t day = DAY;
  for (int n = 1; n <= FACTS; n++) CHECK(!grade(s, cardOf(2, n), EASY, day));   // box 2: not known yet
  CHECK(s.level == 2 && known(s, 2) == 0);
  for (int n = 1; n <= FACTS; n++) CHECK(!grade(s, cardOf(2, n), EASY, day));   // the same day twice: still not known
  CHECK(known(s, 2) == 0);
  day += 1;   // Easy on a second day makes each fact known
  for (int n = 1; n < FACTS; n++) CHECK(!grade(s, cardOf(2, n), EASY, day));
  CHECK(grade(s, cardOf(2, FACTS), EASY, day));   // the last fact known: the level moves on to 3x
  CHECK(s.level == 3 && known(s, 2) == FACTS);
  for (int n = 1; n <= FACTS; n++) { grade(s, cardOf(4, n), EASY, day); grade(s, cardOf(4, n), EASY, day + 1); }
  CHECK(s.level == 3);   // 4x known first: the level waits on 3x...
  for (int n = 1; n <= FACTS; n++) grade(s, cardOf(3, n), EASY, day);
  for (int n = 1; n <= FACTS; n++) grade(s, cardOf(3, n), EASY, day + 1);
  CHECK(s.level == 5);   // ...then skips the known 4x
  setUp(s, TABLES, DAY);
  for (int n = 1; n <= FACTS; n++) { grade(s, cardOf(TABLES, n), EASY, DAY); grade(s, cardOf(TABLES, n), EASY, DAY + 2); }
  CHECK(s.level == TABLES && allKnown(s));   // nothing past the last table
}

static int count(const Round& r, bool (*f)(const Save&, int, int32_t), const Save& s, int32_t day) {
  int k = 0;
  for (int i = 0; i < r.n; i++) k += f(s, r.card[i], day);
  return k;
}
static bool isNew(const Save& s, int c, int32_t) { return s.boxOf(c) == 0; }
static bool due(const Save& s, int c, int32_t day) { return isDue(s, c, day); }

static void rounds() {
  Save s;
  setUp(s, 4, DAY);
  Round r;
  const int first = newAllowed(s);   // a fresh deck at the starting pace: three new facts
  CHECK(first == 3);
  build(r, s, DAY, 7);   // day one: nothing due yet: the new facts, 4x and (taking turns) division, topped up with known ones
  CHECK(r.n == MIN_ROUND && count(r, isNew, s, DAY) == first);
  for (int i = 0; i < r.n; i++) {
    const int c = r.card[i];
    if (!isNew(s, c, DAY)) CHECK(s.boxOf(c) >= MASTER_BOX);
    else CHECK(isDiv(c) ? c == divOf(2, 1) : tableOf(c) == 4 && factorOf(c) <= 2);   // 4x1, 4x2 in order; 2 / 2 from the 2x family
  }
  build(r, s, DAY + 30, 7);   // a month away: everything known is due; room is still kept for new cards
  CHECK(r.n == ROUND && count(r, isNew, s, DAY + 30) == first && count(r, due, s, DAY + 30) == ROUND - first);
  setUp(s, 1, DAY);
  build(r, s, DAY, 3);   // a first-timer on 1x: only its new cards exist, and only a few come at once
  CHECK(r.n == first);
  {
    bool before[ALL] = {};
    for (int i = 0; i < r.n; i++) before[r.card[i]] = true;
    answer(r, s, WRONG, DAY);   // a miss, then Easy: the next round that day brings new facts, not the missed one alone
    while (!r.done()) answer(r, s, EASY, DAY);
    build(r, s, DAY, 4);
    CHECK(r.n == WORKING_SET - first);   // the working set fills up, no further
    for (int i = 0; i < r.n; i++) CHECK(!before[r.card[i]] && isNew(s, r.card[i], DAY));
  }
}

// The pace: a running score of answers on cards already met sets how many new facts a round brings, never past the
// working set; the next table's first facts come while the last of the one before are still being learned.
static void streak(Save& s, Grade g) { for (int i = 0; i < 40; i++) notePace(s, g); }
static void pace() {
  Save s;
  setUp(s, 4, DAY);
  CHECK(paceOf(s) == PACE_START);
  streak(s, EASY);
  CHECK(paceOf(s) > 950 && newAllowed(s) == NEW_PER_ROUND);
  streak(s, WRONG);
  CHECK(paceOf(s) < 100 && newAllowed(s) == 1);   // struggling, but nothing being learned: still one new fact
  grade(s, cardOf(4, 1), WRONG, DAY);
  CHECK(learning(s) == 1 && newAllowed(s) == 0);   // struggling with one in hand: no more until it settles
  setUp(s, 4, DAY);
  streak(s, EASY);
  for (int n = 1; n <= 4; n++) grade(s, cardOf(4, n), EASY, DAY);
  CHECK(learning(s) == 4 && newAllowed(s) == WORKING_SET - 4);   // a fast kid: room in the set is the only limit
}
static void overlap() {   // 4x mostly known and the rest being learned, so the 5x facts start before 4x is done
  Save s;
  setUp(s, 4, DAY);
  streak(s, EASY);
  for (int n = 1; n <= FACTS; n++) grade(s, cardOf(4, n), EASY, DAY);
  for (int n = 1; n <= 7; n++) grade(s, cardOf(4, n), EASY, DAY + 1);
  CHECK(s.level == 4 && known(s, 4) == 7 && learning(s) == 3);
  Round r;
  build(r, s, DAY + 1, 5);
  CHECK(count(r, isNew, s, DAY + 1) == WORKING_SET - 3);
  for (int i = 0; i < r.n; i++) if (isNew(s, r.card[i], DAY + 1) && !isDiv(r.card[i])) CHECK(tableOf(r.card[i]) == 5);
}
static void score() {
  Save s;
  Round r;
  // The running score: an answer on a card already met counts, a first sight and a second look do not.
  setUp(s, 4, DAY);
  build(r, s, DAY + 30, 9);   // due known facts and new ones
  while (!r.done()) answer(r, s, isNew(s, r.current(), DAY + 30) ? WRONG : EASY, DAY + 30);
  CHECK(paceOf(s) > PACE_START);   // Wrong on every new card, Easy on the rest: the pace still went up
  // Struggling: practice leaves room for known facts, easy wins between the hard ones.
  setUp(s, 4, DAY);
  for (int n = 1; n <= WORKING_SET; n++) grade(s, cardOf(4, n), WRONG, DAY);
  streak(s, WRONG);
  build(r, s, DAY + 1, 5);
  int knownIn = 0;
  for (int i = 0; i < r.n; i++) knownIn += s.boxOf(r.card[i]) >= MASTER_BOX;
  CHECK(r.n == ROUND && count(r, isNew, s, DAY + 1) == 0 && knownIn >= KNOWN_MIX);
}

// Division: a known multiplication opens its two divisions (fact families, 1x last), once the kid is on DIV_FROM_LEVEL.
static void division() {
  CHECK(answerOf(divOf(8, 7)) == 7 && dividendOf(divOf(8, 7)) == 56 && isDiv(divOf(8, 7)) && !isDiv(cardOf(10, 10)));
  CHECK(divisorOf(divOf(10, 10)) == 10 && quotientOf(divOf(2, 1)) == 1 && divOf(10, 10) == ALL - 1);
  Save s;
  bool taken[ALL] = {};
  setUp(s, 3, DAY);   // a younger kid: 1x and 2x known, but division waits for 4x
  CHECK(nextDivision(s, taken) < 0);
  setUp(s, 4, DAY);
  const int order[] = {divOf(2, 1), divOf(2, 2), divOf(2, 3), divOf(3, 2), divOf(2, 4), divOf(4, 2)};
  for (int c : order) { CHECK(nextDivision(s, taken) == c); taken[c] = true; }   // 2 / 2, 4 / 2, 6 / 2, 6 / 3, 8 / 2, 8 / 4
  CHECK(!taken[divOf(4, 7)]);
  memset(taken, 0, sizeof taken);
  for (int c = CARDS; c < ALL; c++) if (c != divOf(4, 7) && c != divOf(7, 4)) s.boxOf(c) = KNOWN_BOX;
  CHECK(nextDivision(s, taken) < 0);   // 28 / 4 and 28 / 7 wait for 4x7
  grade(s, cardOf(4, 7), EASY, DAY);
  grade(s, cardOf(4, 7), EASY, DAY + 1);
  CHECK(nextDivision(s, taken) == divOf(4, 7));
  taken[divOf(4, 7)] = true;
  CHECK(nextDivision(s, taken) == divOf(7, 4));
  // A division card learns like any other, and never moves the level (the tables do).
  const int c = divOf(7, 4);
  grade(s, c, EASY, DAY + 1);
  grade(s, c, EASY, DAY + 2);
  CHECK(s.boxOf(c) == MASTER_BOX && s.level == 4 && learningOf(s, true) == 0);
  // Divisions take about a third of the set: two multiplications being learned and no division, so a division first.
  setUp(s, 4, DAY);
  grade(s, cardOf(4, 1), HARD, DAY);
  grade(s, cardOf(4, 2), HARD, DAY);
  int fresh[NEW_PER_ROUND];
  const int nf = newCards(s, fresh);
  CHECK(nf == 3 && isDiv(fresh[0]) && !isDiv(fresh[1]) && isDiv(fresh[2]));   // 2 / 2, 4x3, then 4 / 2
}

// The grading check counts first showings, the next later day after an Easy, and how fast cards are turned; a missed
// card's second look in the same round is left out.
static void gradingCheck() {
  Save s;
  Round r;
  setUp(s, 1, DAY);
  build(r, s, DAY, 7);   // a first-timer on 1x: first showings only
  const int n = r.n;
  noteTurn(s, 400, false);   // a quick turn...
  answer(r, s, EASY, DAY, true);   // ...graded Easy
  noteTurn(s, 3000, false);
  answer(r, s, WRONG, DAY);
  while (!r.done()) { noteTurn(s, 20000, r.redo[r.at]); answer(r, s, HARD, DAY); }   // the missed card's second look: not counted
  CHECK(s.checkFrom == DAY && s.firstSight[EASY] == 1 && s.firstSight[WRONG] == 1 && s.firstSight[HARD] == n - 2);
  CHECK(s.turns == n && s.quick == 1 && s.quickEasy == 1 && s.thinkMs == 400 + 3000 + (uint32_t)(n - 2) * THINK_CAP_MS);
  CHECK(s.nextDay[0] + s.nextDay[1] == 0);
  const int easy = cardOf(4, 1), hard = cardOf(4, 2);
  s.boxOf(easy) = 2; s.seenOf(easy) = (uint16_t)DAY; s.easyLast[easy / 8] |= (uint8_t)(1u << (easy % 8));
  s.boxOf(hard) = 1; s.seenOf(hard) = (uint16_t)DAY; s.easyLast[hard / 8] &= (uint8_t)~(1u << (hard % 8));
  noteGrade(s, easy, EASY, DAY, false);   // the same day: not a next day
  CHECK(s.nextDay[0] == 0 && easyLast(s, easy));
  noteGrade(s, easy, WRONG, DAY + 1, false);   // Easy, then missed the next day
  noteGrade(s, hard, EASY, DAY + 1, false);   // last grade Hard: not counted
  CHECK(s.nextDay[0] == 0 && s.nextDay[1] == 1 && !easyLast(s, easy) && easyLast(s, hard));
  s.seenOf(hard) = (uint16_t)(DAY + 1);
  noteGrade(s, hard, EASY, DAY + 3, false);
  CHECK(s.nextDay[0] == 1 && s.checkFrom == DAY);
}

static void practice() {   // a fact still being learned and not yet seen today; a known fact comes only when due
  Save s;
  Round r;
  // Practice is a fact still being learned and not yet seen today; a known fact comes only when due.
  setUp(s, 4, DAY - 1);
  for (int n = 1; n <= 6; n++) grade(s, cardOf(4, n), EASY, DAY - 1);
  grade(s, cardOf(4, 1), WRONG, DAY - 1);   // due today
  build(r, s, DAY, 5);   // 4x1 and some known older facts due, then 4x2..4x6 as practice; the set is full, no new
  auto has = [&](int c) { for (int i = 0; i < r.n; i++) if (r.card[i] == c) return true; return false; };
  CHECK(count(r, isNew, s, DAY) == 0 && r.n <= ROUND);
  for (int n = 1; n <= 6; n++) CHECK(has(cardOf(4, n)));
  for (int i = 0; i < r.n; i++) CHECK(tableOf(r.card[i]) == 4 || isDue(s, r.card[i], DAY));
  grade(s, cardOf(4, 3), HARD, DAY);   // seen today: not practised again today
  build(r, s, DAY, 5);
  CHECK(!has(cardOf(4, 3)) && has(cardOf(4, 4)));
  setUp(s, TABLES, DAY);   // every table known and nothing due: practice from all of them
  for (int n = 1; n <= FACTS; n++) { grade(s, cardOf(TABLES, n), EASY, DAY); grade(s, cardOf(TABLES, n), EASY, DAY + 1); }
  for (int c = 0; c < ALL; c++) {   // the divisions known too
    if (isDiv(c)) s.boxOf(c) = KNOWN_BOX;
    s.dueOf(c) = (uint16_t)(DAY + 3);
  }
  build(r, s, DAY + 1, 5);
  CHECK(allKnown(s) && r.n == ROUND);
}

static void putBack() {
  Save s;
  Round r;
  // A Wrong card comes back AGAIN_AFTER cards later, at most AGAIN_MAX times.
  setUp(s, 4, DAY);
  build(r, s, DAY + 30, 9);
  const int first = r.current();
  answer(r, s, WRONG, DAY + 30);
  CHECK(r.n == ROUND + 1 && r.card[1 + AGAIN_AFTER] == first && r.wrong == 1);
  CHECK(r.base == ROUND && r.redo[1 + AGAIN_AFTER] && !r.redo[AGAIN_AFTER] && r.number() == 2);   // "2 of 10" after one card
  int shown = 1;
  while (!r.done()) {
    const bool again = r.current() == first;
    shown += again;
    answer(r, s, again ? WRONG : EASY, DAY + 30);
  }
  CHECK(shown == 1 + AGAIN_MAX && r.n == ROUND + AGAIN_MAX && r.easy == ROUND - 1 && r.wrong == 1 + AGAIN_MAX);
  CHECK(s.boxOf(first) == 1 && s.dueOf(first) == DAY + 31);
  build(r, s, DAY + 30, 4);   // an all-Wrong round grows by AGAIN_ROOM cards at most
  while (!r.done()) answer(r, s, WRONG, DAY + 30);
  CHECK(r.n == MAX_QUEUE && r.wrong == MAX_QUEUE);
}

static void saves() {
  Save s;
  setUp(s, 3, DAY);
  grade(s, cardOf(3, 5), EASY, DAY);
  seal(s);
  Save back;
  CHECK(loadBlob(&s, sizeof s, back) && !memcmp(&back, &s, sizeof s));
  Save bad = s;
  bad.boxOf(0) ^= 1;
  CHECK(!loadBlob(&bad, sizeof bad, back));   // a damaged blob: a fresh deck instead
  CHECK(!loadBlob(&s, sizeof s - 4, back));
  {   // version 1, from before division: the same fields up to its crc, and no division met
    uint8_t v1[SAVE_V1_SIZE];
    Save old = s;
    old.version = 1; old.size = SAVE_V1_SIZE;
    memcpy(v1, &old, SAVE_V1_SIZE - 4);
    const uint32_t crc = os::crc32(v1, SAVE_V1_SIZE - 4);
    memcpy(v1 + SAVE_V1_SIZE - 4, &crc, 4);
    CHECK(loadBlob(v1, sizeof v1, back) && back.boxOf(cardOf(3, 5)) == s.boxOf(cardOf(3, 5)) && back.level == s.level);
    for (int c = CARDS; c < ALL; c++) CHECK(!back.boxOf(c));
    v1[20] ^= 1;
    CHECK(!loadBlob(v1, sizeof v1, back));
  }
  {   // version 2, from before the grading check: loads with nothing counted yet
    uint8_t v2[SAVE_V2_SIZE];
    Save old = s;
    old.version = 2; old.size = SAVE_V2_SIZE;
    memcpy(v2, &old, SAVE_V2_SIZE - 4);
    const uint32_t crc = os::crc32(v2, SAVE_V2_SIZE - 4);
    memcpy(v2 + SAVE_V2_SIZE - 4, &crc, 4);
    CHECK(loadBlob(v2, sizeof v2, back) && back.boxOf(cardOf(3, 5)) == s.boxOf(cardOf(3, 5)) && !back.checkFrom &&
          !back.turns);
    old.version = 1;   // a size that says one version and a field that says another: refused
    memcpy(v2, &old, SAVE_V2_SIZE - 4);
    const uint32_t crc1 = os::crc32(v2, SAVE_V2_SIZE - 4);
    memcpy(v2 + SAVE_V2_SIZE - 4, &crc1, 4);
    CHECK(!loadBlob(v2, sizeof v2, back));
  }
  s.dueOf(cardOf(3, 5)) = (uint16_t)(DAY + 900);   // a clock set back: no card waits past its box's interval
  clampDue(s, DAY);
  CHECK(s.dueOf(cardOf(3, 5)) == DAY + INTERVAL[2]);
}

static uint16_t panel[gfx565::W * gfx565::H];
static uint32_t ms = 1000;
static const uint32_t NOW = (uint32_t)DAY * 86400u + 3600;
static void frame(Game& g, Input in = Input{}) {
  g.update(NOW, ms, in);
  gfx565::target(panel);
  g.render();
  ms += 40;
}
static void tapBox(Game& g, int x, int y) {
  Input in; in.down = in.pressed = true; in.x = in.downX = x; in.y = in.downY = y;
  frame(g, in);
  in.down = in.pressed = false; in.released = in.tap = true;
  frame(g, in);
}
static void settle(Game& g) { for (int i = 0; i < 15; i++) frame(g); }
static bool on(const Game& g, const char* s) { return !strcmp(g.screenName(), s); }

// A grown-up holds the title for two seconds: the grading check; a short press does nothing.
static void holdTitle(Game& g) {
  Input in; in.down = in.pressed = true; in.x = in.downX = 80; in.y = in.downY = 30;
  for (int i = 0; i < 10; i++) { frame(g, in); in.pressed = false; }
  CHECK(on(g, "cards_home"));
  for (int i = 0; i < 50; i++) frame(g, in);
  CHECK(on(g, "cards_stats"));
  in.down = false; in.released = true;
  frame(g, in);
  settle(g);
  tapBox(g, 80, 12);   // the house: back to the deck
  CHECK(on(g, "cards_home"));
}

// ---- version 4: the deck stored bit-packed
static size_t g_most = 0;   // the largest packed deck seen
// Packs s and reads it back: true if that is byte for byte the deck that went in, and packing left it alone.
static bool packsExactly(Save& s, size_t* bytes = nullptr) {
  seal(s);
  const Save before = s;
  uint8_t blob[sizeof(Save)]; Save back{};
  const size_t n = pack(s, blob, sizeof blob);
  if (bytes) *bytes = n;
  if (n > g_most) g_most = n;
  return n && storedVersion(blob, n) == SAVE_VERSION && !memcmp(&s, &before, sizeof s) && loadBlob(blob, n, back) &&
         !memcmp(&back, &s, sizeof s);
}
// A real version 3 save: every byte of it survives version 4, and the game stores it packed as soon as it is opened.
static void versionThreeMigrates() {
  const size_t old = sizeof TIMES_CARDS_SAVE_V3;
  Save deck;
  CHECK(storedVersion(TIMES_CARDS_SAVE_V3, old) == SAVE_V3 && loadBlob(TIMES_CARDS_SAVE_V3, old, deck));
  CHECK(!memcmp(&deck, TIMES_CARDS_SAVE_V3, old) && deck.level == 10 && deck.answers == 1995 && deck.turns == 1819);
  size_t n = 0;
  CHECK(packsExactly(deck, &n) && !memcmp(&deck, TIMES_CARDS_SAVE_V3, old) && n < 420);
  Save fresh; setUp(fresh, 4, DAY); size_t small = 0;
  CHECK(packsExactly(fresh, &small) && small < 200);
  printf("times cards save: version 3 1020 bytes; version 4 %u new, %u four months in\n", (unsigned)small, (unsigned)n);

  Game g;
  Profile zoe = {1, "Zoe", 0, 8, 0};
  SaveSlot slot = {TIMES_CARDS_SAVE_V3, old};
  AppEnter e = {&zoe, &zoe, &slot, 1, 20830u * 86400u, ms, true};   // the day after the deck's last round
  uint32_t wallet = 0;
  e.stars = &wallet;
  g.enter(e);
  const void* data; size_t len;
  CHECK(g.takeSave(&data, &len, true) && len == n && storedVersion(data, len) == SAVE_VERSION);
  CHECK(!memcmp(&g.save(), TIMES_CARDS_SAVE_V3, old));   // opening moved nothing
  uint8_t stored[sizeof(Save)]; Save back;
  memcpy(stored, data, len);
  CHECK(loadBlob(stored, len, back) && !memcmp(&back, TIMES_CARDS_SAVE_V3, old));
  slot = {stored, len};   // opened again on the packed deck: the same deck, and nothing to write
  g.enter(e);
  CHECK(!g.takeSave(&data, &len, true) && g.save().answers == 1995 && !memcmp(g.save().box, back.box, sizeof back.box));
}
// One bit wrong anywhere, or a byte short or long: not read.
static void damagedPacked() {
  Save deck, back;
  CHECK(loadBlob(TIMES_CARDS_SAVE_V3, sizeof TIMES_CARDS_SAVE_V3, deck));
  uint8_t blob[sizeof(Save) + 1] = {};
  const size_t n = pack(deck, blob, sizeof(Save));
  bool read = false;
  for (size_t bit = 0; bit < n * 8; bit++) {
    blob[bit / 8] ^= (uint8_t)(1u << bit % 8);
    read |= loadBlob(blob, n, back);
    blob[bit / 8] ^= (uint8_t)(1u << bit % 8);
  }
  for (size_t cut = 0; cut < n; cut++) read |= loadBlob(blob, cut, back);
  CHECK(!read && !loadBlob(blob, n + 1, back) && loadBlob(blob, n, back));
  CHECK(!pack(deck, blob, 11) && !pack(deck, blob, n - 1) && pack(deck, blob, n) == n);   // too little room: nothing kept
  deck.boxOf(7) = MAX_BOX + 1;   // no loaded deck has one; it would not survive three bits, so it is stored whole
  CHECK(!pack(deck, blob, sizeof(Save)));
}
// Any deck at all, not only one play can make: every field at random, days close together or all over the calendar.
static void anyDeckPacks() {
  uint32_t r = 2026;
  for (int t = 0; t < 600; t++) {
    Save s{};
    const uint32_t span = 1u << (t % 17), from = nextRand(r);
    s.level = (uint8_t)(nextRand(r) % (TABLES + 1)); s.start = (uint8_t)nextRand(r);
    s.pace = (uint16_t)nextRand(r); s.answers = nextRand(r);
    for (int c = 0; c < ALL; c++) {
      if (nextRand(r) % 4) s.boxOf(c) = (uint8_t)(nextRand(r) % (MAX_BOX + 1));
      if (nextRand(r) % 3) s.seenOf(c) = (uint16_t)(from + nextRand(r) % span);
      if (nextRand(r) % 5) s.dueOf(c) = (uint16_t)(t % 2 ? s.seenOf(c) + nextRand(r) % 70 : from + nextRand(r) % span);
    }
    for (uint8_t* p = (uint8_t*)&s.thinkMs; p < (uint8_t*)&s.crc; p++) *p = (uint8_t)nextRand(r);
    if (t % 3) s.spare = s.spare3 = 0;
    else s.spare = (uint16_t)nextRand(r);
    CHECK(packsExactly(s));
  }
}
// Decks as kids make them: every day of half a year of play, for each starting table and a few kinds of kid.
static void playedDecksPack() {
  size_t most = 0;
  for (int kid = 0; kid < 12; kid++) {
    Save s; int32_t day = DAY + kid * 400; uint32_t r = 7 + (uint32_t)kid;
    setUp(s, 1 + kid % 4, day);
    for (int d = 0; d < 180; d++, day++) {
      if (d % 7 == kid % 7 || (d > 50 && d < 50 + kid * 3)) continue;   // days off, and weeks away
      clampDue(s, day);
      for (int round = 0; round < 1 + (d + kid) % 3; round++) {
        Round q; build(q, s, day, nextRand(r));
        while (!q.done()) {
          const uint32_t think = 300 + nextRand(r) % 9000, roll = nextRand(r) % 100;
          noteTurn(s, think, q.redo[q.at]);
          answer(q, s, roll < (uint32_t)(50 + kid * 4) ? EASY : roll < 92 ? HARD : WRONG, day, think < QUICK_MS);
        }
      }
      size_t n = 0;
      CHECK(packsExactly(s, &n));
      if (n > most) most = n;
    }
  }
  CHECK(most < 480);   // 17 NVS entries; host/test_nvs_budget.cpp holds the household's budget
  printf("times cards save: at most %u bytes in half a year of play, %u for any deck at all\n", (unsigned)most, (unsigned)g_most);
}

static void pages() {
  Game g;
  Profile zoe = {1, "Zoe", 0, 8, 0};
  SaveSlot none = {nullptr, 0};
  AppEnter e = {&zoe, &zoe, &none, 1, NOW, ms, true};
  uint32_t wallet = 7;
  e.stars = &wallet;
  g.enter(e);
  settle(g);
  CHECK(on(g, "cards_home") && g.save().level == 4);   // an 8-year-old starts on 4x
  const void* data; size_t len;
  CHECK(g.takeSave(&data, &len, true) && len < 200 && storedVersion(data, len) == SAVE_VERSION);   // the fresh deck is kept at once, packed
  tapBox(g, 80, 133);   // Go!
  CHECK(on(g, "cards_ask"));
  settle(g);
  const int first = MIN_ROUND;   // day one: the first new facts, topped up with known ones
  for (int i = 0; i < first; i++) {
    tapBox(g, 80, 70);   // the card: the answer shows
    CHECK(on(g, "cards_answer"));
    settle(g);
    tapBox(g, 41, 122);   // Easy
    settle(g);
  }
  CHECK(on(g, "cards_done") && g.save().answers == (uint32_t)first);
  CHECK(g.save().turns == first && g.save().firstSight[EASY] == 3 && g.save().checkFrom == NOW / 86400);   // three new facts; the top-up was known
  CHECK(wallet == 7 + ROUND_STARS);   // a finished round pays the kid's shared stars
  CHECK(g.takeSave(&data, &len, true));
  uint8_t saved[sizeof(Save)]; const size_t savedLen = len; memcpy(saved, data, len);
  tapBox(g, 80, 133);   // Again
  CHECK(on(g, "cards_ask"));
  settle(g);
  tapBox(g, 80, 12);   // home: back to the deck, then out
  CHECK(on(g, "cards_home"));
  settle(g);
  tapBox(g, 80, 70);   // the deck's card deals too
  CHECK(on(g, "cards_ask"));
  settle(g);
  tapBox(g, 80, 12);
  settle(g);
  tapBox(g, 80, 12);
  CHECK(g.wantsHome() && !g.wantsHome());
  holdTitle(g);
  g.debugCmd("level7");
  CHECK(g.save().level == 7 && g.save().answers == 0);
  // Back again: the saved deck, not a new one.
  SaveSlot slot = {saved, savedLen};
  e.saves = &slot;
  g.enter(e);
  CHECK(g.save().answers == (uint32_t)first && g.save().level == 4);
  g.debugPrint();
}

int main() {
  start();
  grades();
  levelUp();
  rounds();
  division();
  gradingCheck();
  pace();
  overlap();
  score();
  practice();
  putBack();
  saves();
  versionThreeMigrates();
  damagedPacked();
  anyDeckPacks();
  playedDecksPack();
  pages();
  printf("test_times_cards: %d checks ok\n", checks);
  return 0;
}
