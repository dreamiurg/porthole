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
  grade(s, c, EASY, DAY + 1);   // early (practice): no move
  CHECK(s.box[c] == 2 && s.due[c] == DAY + 2);
  grade(s, c, HARD, DAY + 1);   // early Hard: no later than tomorrow, box kept
  CHECK(s.box[c] == 2 && s.due[c] == DAY + 2);
  grade(s, c, EASY, DAY + 2);   // due: up to the mastery box
  CHECK(s.box[c] == MASTER_BOX && s.due[c] == DAY + 2 + INTERVAL[MASTER_BOX]);
  grade(s, c, HARD, DAY + 6);   // due Hard: kept, tomorrow
  CHECK(s.box[c] == MASTER_BOX && s.due[c] == DAY + 7);
  grade(s, c, WRONG, DAY + 6);   // Wrong: back to learning, tomorrow
  CHECK(s.box[c] == 1 && s.due[c] == DAY + 7);
  const int d = cardOf(2, 4);
  grade(s, d, HARD, DAY);
  CHECK(s.box[d] == 1 && s.due[d] == DAY + 1);
  for (int i = 0; i < 20; i++) grade(s, d, EASY, DAY + 1000 * (i + 1));   // never past the top box
  CHECK(s.box[d] == MAX_BOX);
  CHECK(s.answers == 27);
}

static void levelUp() {
  Save s;
  setUp(s, 2, DAY);
  int32_t day = DAY;
  for (int n = 1; n <= FACTS; n++) CHECK(!grade(s, cardOf(2, n), EASY, day));   // box 2: not known yet
  CHECK(s.level == 2 && known(s, 2) == 0);
  day += 2;
  for (int n = 1; n < FACTS; n++) CHECK(!grade(s, cardOf(2, n), EASY, day));
  CHECK(grade(s, cardOf(2, FACTS), EASY, day));   // the last fact known: the 3x table opens
  CHECK(s.level == 3 && known(s, 2) == FACTS);
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
  build(r, s, DAY, 7);   // day one: nothing due yet, four new 4x facts, and practice from the known tables
  CHECK(r.n == ROUND && count(r, isNew, s, DAY) == NEW_PER_ROUND);
  bool seen[CARDS] = {};
  for (int i = 0; i < r.n; i++) { CHECK(!seen[r.card[i]] && tableOf(r.card[i]) <= 4); seen[r.card[i]] = true; }
  build(r, s, DAY + 30, 7);   // a month away: everything known is due; room is still kept for new cards
  CHECK(r.n == ROUND && count(r, isNew, s, DAY + 30) == NEW_PER_ROUND && count(r, due, s, DAY + 30) == ROUND - NEW_PER_ROUND);
  setUp(s, 1, DAY);
  build(r, s, DAY, 3);   // a first-timer on 1x: only its new cards exist, and only four of them come at once
  CHECK(r.n == NEW_PER_ROUND);
  // A Wrong card comes back AGAIN_AFTER cards later, at most AGAIN_MAX times.
  setUp(s, 4, DAY);
  build(r, s, DAY, 9);
  const int first = r.current();
  answer(r, s, WRONG, DAY);
  CHECK(r.n == ROUND + 1 && r.card[1 + AGAIN_AFTER] == first && r.wrong == 1);
  int shown = 1;
  while (!r.done()) {
    const bool again = r.current() == first;
    shown += again;
    answer(r, s, again ? WRONG : EASY, DAY);
  }
  CHECK(shown == 1 + AGAIN_MAX && r.n == ROUND + AGAIN_MAX && r.easy == ROUND - 1 && r.wrong == 1 + AGAIN_MAX);
  CHECK(s.box[first] == 1 && s.due[first] == DAY + 1);
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
  g.enter(e);
  settle(g);
  CHECK(on(g, "cards_home") && g.save().level == 4);   // an 8-year-old starts on 4x
  const void* data; size_t len;
  CHECK(g.takeSave(&data, &len, true) && len == sizeof(Save));   // the fresh deck is kept at once
  tapBox(g, 80, 133);   // Go!
  CHECK(on(g, "cards_ask"));
  settle(g);
  for (int i = 0; i < ROUND; i++) {
    tapBox(g, 80, 70);   // the card: the answer shows
    CHECK(on(g, "cards_answer"));
    settle(g);
    tapBox(g, 41, 122);   // Easy
    settle(g);
  }
  CHECK(on(g, "cards_done") && g.save().answers == ROUND);
  CHECK(g.takeSave(&data, &len, true));
  Save saved; memcpy(&saved, data, sizeof saved);
  tapBox(g, 80, 133);   // Again
  CHECK(on(g, "cards_ask"));
  settle(g);
  tapBox(g, 80, 12);   // home: back to the deck, then out
  CHECK(on(g, "cards_home"));
  settle(g);
  tapBox(g, 80, 12);
  CHECK(g.wantsHome() && !g.wantsHome());
  g.debugCmd("level7");
  CHECK(g.save().level == 7 && g.save().answers == 0);
  // Back again: the saved deck, not a new one.
  SaveSlot slot = {&saved, sizeof saved};
  e.saves = &slot;
  g.enter(e);
  CHECK(g.save().answers == ROUND && g.save().level == 4);
  g.debugPrint();
}

int main() {
  start();
  grades();
  levelUp();
  rounds();
  saves();
  pages();
  printf("test_times_cards: %d checks ok\n", checks);
  return 0;
}
