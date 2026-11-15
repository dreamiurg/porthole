// Times Cards' self-check: the deck's rules (games/times-cards/deck.h): where a kid starts, how each grade moves a card,
// that only a remembered due card moves up, a round's makeup and a Wrong card coming back, the level opening the next
// table, the save and a clock set back; then the pages through the App interface, each one drawn.
#include "host/check.h"
#include <stdio.h>
#include <string.h>
#include "games/times-cards/deck.h"
#include "games/times-cards/game.h"
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
    if (tableOf(c) < 4) CHECK(s.box[c] == KNOWN_BOX && s.due[c] > DAY && s.due[c] <= DAY + INTERVAL[KNOWN_BOX]);
    else CHECK(s.box[c] == 0);
  }
  CHECK(known(s, 3) == FACTS && known(s, 4) == 0);
  CHECK(answerOf(cardOf(7, 8)) == 56 && tableOf(cardOf(7, 8)) == 7 && factorOf(cardOf(7, 8)) == 8);
}

static void grades() {
  Save s;
  setUp(s, 2, DAY);
  const int c = cardOf(2, 3);
  grade(s, c, EASY, DAY);   // new and known at once: box 2, back in 2 days
  CHECK(s.box[c] == 2 && s.due[c] == DAY + 2);
  grade(s, c, EASY, DAY);   // the same day again (practice): no move
  CHECK(s.box[c] == 2 && s.due[c] == DAY + 2);
  grade(s, c, HARD, DAY);   // early Hard: no later than tomorrow, box kept
  CHECK(s.box[c] == 2 && s.due[c] == DAY + 1);
  grade(s, c, EASY, DAY + 1);   // still learning, another day: up to the mastery box
  CHECK(s.box[c] == MASTER_BOX && s.due[c] == DAY + 1 + INTERVAL[MASTER_BOX]);
  grade(s, c, EASY, DAY + 2);   // known and not due: no move
  CHECK(s.box[c] == MASTER_BOX && s.due[c] == DAY + 5);
  grade(s, c, HARD, DAY + 5);   // due Hard: kept, tomorrow
  CHECK(s.box[c] == MASTER_BOX && s.due[c] == DAY + 6);
  grade(s, c, WRONG, DAY + 5);   // Wrong: back to learning, tomorrow
  CHECK(s.box[c] == 1 && s.due[c] == DAY + 6);
  const int d = cardOf(2, 4);
  grade(s, d, HARD, DAY);
  CHECK(s.box[d] == 1 && s.due[d] == DAY + 1);
  for (int i = 0; i < 20; i++) grade(s, d, EASY, DAY + 1000 * (i + 1));   // never past the top box
  CHECK(s.box[d] == MAX_BOX);
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
static bool isNew(const Save& s, int c, int32_t) { return s.box[c] == 0; }
static bool due(const Save& s, int c, int32_t day) { return isDue(s, c, day); }

static void rounds() {
  Save s;
  setUp(s, 4, DAY);
  Round r;
  const int first = newAllowed(s);   // a fresh deck at the starting pace: three new facts
  CHECK(first == 3);
  build(r, s, DAY, 7);   // day one: nothing due yet and nothing to practise: only the first new 4x facts
  CHECK(r.n == first && count(r, isNew, s, DAY) == first);
  for (int i = 0; i < r.n; i++) CHECK(tableOf(r.card[i]) == 4 && factorOf(r.card[i]) <= first);   // in table order
  build(r, s, DAY + 30, 7);   // a month away: everything known is due; room is still kept for new cards
  CHECK(r.n == ROUND && count(r, isNew, s, DAY + 30) == first && count(r, due, s, DAY + 30) == ROUND - first);
  setUp(s, 1, DAY);
  build(r, s, DAY, 3);   // a first-timer on 1x: only its new cards exist, and only a few come at once
  CHECK(r.n == first);
  {
    bool before[CARDS] = {};
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
  for (int i = 0; i < r.n; i++) if (isNew(s, r.card[i], DAY + 1)) CHECK(tableOf(r.card[i]) == 5);
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
  for (int i = 0; i < r.n; i++) knownIn += s.box[r.card[i]] >= MASTER_BOX;
  CHECK(r.n == ROUND && count(r, isNew, s, DAY + 1) == 0 && knownIn >= KNOWN_MIX);
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
  for (int c = 0; c < CARDS; c++) s.due[c] = (uint16_t)(DAY + 3);
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
  CHECK(s.box[first] == 1 && s.due[first] == DAY + 31);
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
  bad.box[0] ^= 1;
  CHECK(!loadBlob(&bad, sizeof bad, back));   // a damaged blob: a fresh deck instead
  CHECK(!loadBlob(&s, sizeof s - 4, back));
  s.due[cardOf(3, 5)] = (uint16_t)(DAY + 900);   // a clock set back: no card waits past its box's interval
  clampDue(s, DAY);
  CHECK(s.due[cardOf(3, 5)] == DAY + INTERVAL[2]);
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
  CHECK(g.takeSave(&data, &len, true) && len == sizeof(Save));   // the fresh deck is kept at once
  tapBox(g, 80, 133);   // Go!
  CHECK(on(g, "cards_ask"));
  settle(g);
  const int first = newAllowed(g.save());
  for (int i = 0; i < first; i++) {   // day one: the first new facts
    tapBox(g, 80, 70);   // the card: the answer shows
    CHECK(on(g, "cards_answer"));
    settle(g);
    tapBox(g, 41, 122);   // Easy
    settle(g);
  }
  CHECK(on(g, "cards_done") && g.save().answers == (uint32_t)first);
  CHECK(wallet == 7 + ROUND_STARS);   // a finished round pays the kid's shared stars
  CHECK(g.takeSave(&data, &len, true));
  Save saved; memcpy(&saved, data, sizeof saved);
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
  g.debugCmd("level7");
  CHECK(g.save().level == 7 && g.save().answers == 0);
  // Back again: the saved deck, not a new one.
  SaveSlot slot = {&saved, sizeof saved};
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
  pace();
  overlap();
  score();
  practice();
  putBack();
  saves();
  pages();
  printf("test_times_cards: %d checks ok\n", checks);
  return 0;
}
