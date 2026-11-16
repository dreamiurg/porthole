// Times Cards' self-check: the deck's rules (games/times-cards/deck.h): where a kid starts, how each grade moves a card
// in half-days, the grade a pick earns from the kid's own usual time, the four answers offered, the step and what each
// step deals, the jump, a card put back, division, the save in every version and a clock set back; then the pages
// through the App interface, each one drawn.
#include "host/check.h"
#include <stdio.h>
#include <string.h>
#include "games/times-cards/deck.h"
#include "games/times-cards/game.h"
#include "host/fixtures/times_cards_save_v3.h"
#include "host/fixtures/times_cards_save_v4.h"
#include "gfx565.h"

using namespace cards;
static int checks = 0;
constexpr int32_t H = 40000;   // a morning: half-day H, day H / 2
constexpr uint32_t FAST = 1500, SLOW = 9000;   // a quick pick and a slow one, for any kid and any fact

static void start() {
  CHECK(startTable(6) == 2 && startTable(8) == 4 && startTable(5) == 1 && startTable(0) == 1 && startTable(11) == 4);
  Save s;
  setUp(s, 4, H);
  CHECK(s.level == 4 && s.start == 4 && s.step == 0);
  for (int c = 0; c < CARDS; c++) {
    if (tableOf(c) < 4) CHECK(s.boxOf(c) == KNOWN_BOX && s.dueOf(c) > H && s.dueOf(c) <= H + INTERVAL[KNOWN_BOX]);
    else CHECK(s.boxOf(c) == 0);
  }
  CHECK(known(s, 3) == FACTS && known(s, 4) == 0 && levelOf(s) == 4);
}
static void facts() {
  CHECK(answerOf(cardOf(7, 8)) == 56 && tableOf(cardOf(7, 8)) == 7 && factorOf(cardOf(7, 8)) == 8);
  CHECK(halfOf(20000u * 86400u) == 40000 && halfOf(20000u * 86400u + 43199) == 40000 && halfOf(20000u * 86400u + 43200) == 40001);
}

// Half-days: box 1 waits a half, box 2 a day; a learning card moves up after a later half (box 1) or a later day (box 2).
static void grades() {
  Save s;
  setUp(s, 2, H);
  const int c = cardOf(2, 3);
  grade(s, c, EASY, H);   // new and right fast at once: box 2, back tomorrow
  CHECK(s.boxOf(c) == 2 && s.dueOf(c) == H + 2);
  grade(s, c, EASY, H + 1);   // the same evening: box 2 waits for a night
  CHECK(s.boxOf(c) == 2 && s.dueOf(c) == H + 2);
  grade(s, c, HARD, H + 1);   // slow: no later than the next half
  CHECK(s.boxOf(c) == 2 && s.dueOf(c) == H + 2);
  grade(s, c, EASY, H + 2);   // the next morning: known
  CHECK(s.boxOf(c) == MASTER_BOX && s.dueOf(c) == H + 2 + INTERVAL[MASTER_BOX] && INTERVAL[MASTER_BOX] == 8);
  grade(s, c, EASY, H + 3);   // known and not due: no move
  CHECK(s.boxOf(c) == MASTER_BOX && s.dueOf(c) == H + 10);
  grade(s, c, HARD, H + 10);   // due and slow: kept, the next half
  CHECK(s.boxOf(c) == MASTER_BOX && s.dueOf(c) == H + 11);
  grade(s, c, WRONG, H + 10);   // missed: back to learning, the next half
  CHECK(s.boxOf(c) == 1 && s.dueOf(c) == H + 11);
  const int d = cardOf(2, 4);
  grade(s, d, HARD, H);   // box 1
  grade(s, d, EASY, H);   // the same half: no move
  CHECK(s.boxOf(d) == 1 && s.dueOf(d) == H + 1);
  grade(s, d, EASY, H + 1);   // that evening: box 2
  CHECK(s.boxOf(d) == 2 && s.dueOf(d) == H + 3);
  for (int i = 0; i < 20; i++) grade(s, d, EASY, H + 1000 * (i + 1));   // never past the top box
  CHECK(s.boxOf(d) == MAX_BOX && s.answers == 30);
}

// The level is the first table from the last level up not fully known: it moves up when the last fact is known, and stays
// when a known one is missed.
static void levels() {
  Save s;
  setUp(s, 2, H);
  for (int n = 1; n <= FACTS; n++) CHECK(!grade(s, cardOf(2, n), EASY, H));
  CHECK(s.level == 2 && known(s, 2) == 0);
  for (int n = 1; n < FACTS; n++) CHECK(!grade(s, cardOf(2, n), EASY, H + 2));
  CHECK(grade(s, cardOf(2, FACTS), EASY, H + 2) && s.level == 3);   // the last fact known: 3x next
  for (int n = 1; n <= FACTS; n++) { grade(s, cardOf(4, n), EASY, H); grade(s, cardOf(4, n), EASY, H + 2); }
  CHECK(s.level == 3);   // 4x known first: the level waits on 3x...
  for (int n = 1; n <= FACTS; n++) { grade(s, cardOf(3, n), EASY, H); grade(s, cardOf(3, n), EASY, H + 2); }
  CHECK(s.level == 5);   // ...then skips the known 4x
  CHECK(!grade(s, cardOf(2, 7), WRONG, H + 20) && s.level == 5 && s.boxOf(cardOf(2, 7)) == 1);   // 2x7 missed: learned again
  CHECK(!grade(s, cardOf(2, 7), EASY, H + 21) && s.level == 5 && learning(s) == 1);   // the next half: box 2, the level stays
  setUp(s, TABLES, H);
  for (int n = 1; n <= FACTS; n++) { grade(s, cardOf(TABLES, n), EASY, H); grade(s, cardOf(TABLES, n), EASY, H + 4); }
  CHECK(s.level == TABLES && allKnown(s));   // nothing past the last table
}

// The grade from the time: Easy under the kid's mean plus one spread for the fact's size; the mean follows the kid.
static void timing() {
  CHECK(sizeOf(cardOf(7, 8)) > 1.2f && sizeOf(divOf(7, 8)) > 1.2f && sizeOf(cardOf(3, 4)) == 1.0f);
  CHECK(sizeOf(cardOf(2, 7)) < 0.9f && sizeOf(cardOf(7, 5)) < 0.9f && sizeOf(cardOf(6, 6)) < 0.9f && sizeOf(cardOf(10, 9)) < 0.9f);
  Save s;
  setUp(s, 4, H);   // an 8-year-old: 4 s usual before anything is timed
  const int mid = cardOf(3, 4), hard = cardOf(7, 8), easy = cardOf(2, 6);
  CHECK(usualMs(s) == 4000 && fastMs(s, mid) > 5600 && fastMs(s, mid) < 5750);   // 4 s x e^0.35
  CHECK(fastMs(s, hard) > fastMs(s, mid) && fastMs(s, easy) < fastMs(s, mid));
  setUp(s, 2, H);   // a 6-year-old: 5 s, and a hard fact gets the most there is
  CHECK(usualMs(s) == 5000 && fastMs(s, hard) == FAST_MAX_MS);
  CHECK(gradeOf(s, mid, true, 6000) == EASY && gradeOf(s, mid, true, 7500) == HARD && gradeOf(s, mid, false, 500) == WRONG);
}
// The usual time follows the kid: down when quicker, a little for one long look away, up again in a slower spell.
static void timeFollows() {
  Save s;
  setUp(s, 2, H);
  const int mid = cardOf(3, 4), hard = cardOf(7, 8);
  for (int i = 0; i < 60; i++) noteTime(s, mid, 2000);   // a quick kid: the line comes down to its floor
  CHECK(usualMs(s) > 1900 && usualMs(s) < 2100 && fastMs(s, mid) == FAST_MIN_MS && gradeOf(s, mid, true, 3500) == HARD);
  const uint32_t before = usualMs(s);
  noteTime(s, mid, TIME_CAP_MS);   // one long look away moves it a little, not to 10 s
  CHECK(usualMs(s) > before && usualMs(s) < before * 13 / 10);
  for (int i = 0; i < 60; i++) noteTime(s, mid, 6000);   // a slower spell: it follows
  CHECK(usualMs(s) > 5500 && usualMs(s) < 6100 && fastMs(s, mid) < FAST_MAX_MS);
  for (int i = 0; i < 60; i++) noteTime(s, hard, 7500);   // a hard fact's 7.5 s is a mid one's 6 s
  CHECK(usualMs(s) > 5800 && usualMs(s) < 6200);
}

// Four answers: the right one and three near misses, all different and above zero, the right one anywhere.
static void options() {
  int seen[OPTIONS] = {};
  for (int c = 0; c < ALL; c++)
    for (uint32_t seed = 1; seed < 40; seed += 3) {
      int o[OPTIONS];
      const int r = choices(c, (uint32_t)c * 40 + seed, o);
      CHECK(r >= 0 && r < OPTIONS && o[r] == answerOf(c));
      seen[r]++;
      for (int i = 0; i < OPTIONS; i++) {
        CHECK(o[i] > 0);
        for (int k = 0; k < i; k++) CHECK(o[i] != o[k]);
      }
    }
  for (int n : seen) CHECK(n > 400);
}
static void nearMisses() {
  int o[OPTIONS];
  choices(cardOf(7, 8), 5, o);
  for (int x : o) CHECK(x == 56 || x == 49 || x == 63 || x == 48 || x == 64);   // the neighbouring facts
  choices(cardOf(1, 1), 5, o);
  for (int x : o) CHECK(x >= 1 && x <= 4);
  choices(divOf(8, 7), 5, o);
  for (int x : o) CHECK(x >= 5 && x <= 9);
}

// Two fast right answers in a row step up, a miss steps down, a slow one starts the count again; -2..+2.
static void staircase() {
  Save s{};
  noteStep(s, EASY); CHECK(s.step == 0 && s.streak == 1);
  noteStep(s, HARD); CHECK(s.step == 0 && s.streak == 0);
  noteStep(s, EASY); noteStep(s, EASY); CHECK(s.step == 1 && s.streak == 0);
  for (int i = 0; i < 9; i++) noteStep(s, EASY);
  CHECK(s.step == STEP_MAX);
  startSession(s);   // the next session starts a step lower
  CHECK(s.step == STEP_CARRY && s.streak == 0);
  for (int i = 0; i < 9; i++) noteStep(s, WRONG);
  CHECK(s.step == STEP_MIN);
  startSession(s);
  CHECK(s.step == STEP_MIN);
  for (int i = 0; i < RECENT + 20; i++) noteRight(s, i % 5 ? EASY : WRONG);   // the share keeps to the last answers
  CHECK(s.tries < RECENT && s.tries >= RECENT / 2 && s.rights * 10 > s.tries * 7 && s.rights * 10 < s.tries * 9);
}

// A round dealt and answered (all right, at `ms`): how many of its own cards (not those come back) were `f`.
static int dealt(Round& r, Save& s, int32_t now, bool (*f)(const Save&, int, int32_t), uint32_t ms = FAST) {
  build(r, s, now);
  int k = 0;
  while (!r.done()) { k += !r.redo[r.at] && f(s, r.current(), now); answer(r, s, true, ms, now); }
  return k;
}
static bool isNew(const Save& s, int c, int32_t) { return !s.boxOf(c); }
static bool isKnown(const Save& s, int c, int32_t) { return s.boxOf(c) >= MASTER_BOX; }
static bool due(const Save& s, int c, int32_t now) { return isDue(s, c, now); }

// A round, card by card: what each step deals.
static void rounds() {
  Save s;
  Round r;
  // Day one for an 8-year-old who knows it all: known facts and a new one; two fast in a row step up, and on +2 the
  // next untouched table is probed, and jumped.
  setUp(s, 4, H);
  CHECK(dealt(r, s, H, isNew) == 6 && r.n == ROUND && r.easy == ROUND && r.jumped && known(s, 5) == FACTS);
  CHECK(s.step == STEP_MAX && s.boxOf(cardOf(4, 1)) == 2 && s.level == 4);
  setUp(s, 1, H);   // a first-timer on 1x, nothing known to fall back on: the new facts a round allows, a short round
  CHECK(dealt(r, s, H, isNew) == NEW_PER_ROUND && r.n == NEW_PER_ROUND && r.base == NEW_PER_ROUND);
  // A month away: due cards take half the round; the rest is still-learning ones.
  setUp(s, 4, H);
  for (int n = 1; n <= WORKING_SET; n++) grade(s, cardOf(4, n), HARD, H + 60);   // seen this morning: not due
  CHECK(dealt(r, s, H + 60, isKnown, SLOW) == DUE_MOST && r.dues == DUE_MOST && r.base == ROUND && s.step == 0);
  setUp(s, 4, H);
  for (int n = 1; n <= WORKING_SET; n++) grade(s, cardOf(4, n), HARD, H);   // all due by now: all of it review
  CHECK(dealt(r, s, H + 60, due, SLOW) == ROUND && r.dues == DUE_MOST);
}
// -2: only known facts, and a miss keeps it there; -1: no new ones; +1: more new ones than 0.
static void steps() {
  Save s;
  Round r;
  setUp(s, 4, H); s.step = -2;
  build(r, s, H);
  CHECK(isKnown(s, r.current(), H));
  answer(r, s, false, FAST, H);
  for (int i = 0; i < 4; i++) { CHECK(isKnown(s, r.current(), H) || r.redo[r.at]); answer(r, s, true, SLOW, H); }
  CHECK(s.step == -2);
  setUp(s, 4, H); s.step = -1;
  grade(s, cardOf(4, 1), HARD, H - 1);
  build(r, s, H);
  for (int i = 0; i < ROUND; i++) { CHECK(!isNew(s, r.current(), H)); answer(r, s, true, SLOW, H); }
  int news[2];
  for (int k = 0; k < 2; k++) {   // six slow right answers (the step stays put): on 0, then on +1
    setUp(s, 4, H); s.step = (int8_t)k;
    for (int n = 1; n <= 2; n++) grade(s, cardOf(4, n), HARD, H - 1);   // something to practise between new ones
    for (int c = 0; c < cardOf(4, 1); c++) s.dueOf(c) = H + 30;   // and nothing else due
    build(r, s, H);
    news[k] = 0;
    for (int i = 0; i < 6; i++) { news[k] += isNew(s, r.current(), H); answer(r, s, true, SLOW, H); }
    CHECK(s.step == k);
  }
  CHECK(news[0] == 2 && news[1] == 3);
}

// +2 probes the next unseen table: three fast right answers on its x6, x7, x8 and the table is known.
static void jumps() {
  Save s;
  Round r;
  setUp(s, 4, H); s.step = 2;
  build(r, s, H);
  for (int n = PROBE_FROM; n <= PROBE_TO; n++) {
    CHECK(r.current() == cardOf(4, n) && probeCard(s) == cardOf(4, n));
    answer(r, s, true, FAST, H);
  }
  CHECK(r.jumped && r.leveledUp && s.jumps == 1 && known(s, 4) == FACTS && s.level == 5);
  for (int n = 1; n <= FACTS; n++) {   // back over the next days, so a lucky streak is soon found out
    const int c = cardOf(4, n);
    CHECK(s.dueOf(c) > H + 1 && s.dueOf(c) <= H + 8 && !isDue(s, c, H));
  }
  CHECK(r.current() == cardOf(5, PROBE_FROM));   // still on +2: 5x is probed next
  answer(r, s, true, SLOW, H);   // slow: no jump past 5x for now
  CHECK(s.boxOf(cardOf(5, PROBE_FROM)) == 1 && probeCard(s) < 0 && s.step == 2);
  setUp(s, 4, H); s.step = 2;   // a miss: the same, and a step down
  build(r, s, H);
  answer(r, s, true, FAST, H);
  answer(r, s, false, FAST, H);
  CHECK(probeCard(s) < 0 && s.step == 1 && !r.jumped && known(s, 4) == 0);
}
// A slow probe learned later the usual way (box 2 again) still never counts toward a jump; a table under way is never
// probed at all.
static void noJump() {
  Save s;
  Round r;
  setUp(s, 4, H); s.step = 2;
  build(r, s, H);
  answer(r, s, true, SLOW, H);
  CHECK(s.boxOf(cardOf(4, PROBE_FROM)) == 1 && (s.probeMiss >> 4 & 1));
  grade(s, cardOf(4, PROBE_FROM), EASY, H + 1);
  CHECK(s.boxOf(cardOf(4, PROBE_FROM)) == 2 && probeCard(s) < 0 && !jump(s, cardOf(4, PROBE_FROM), H + 1));
  setUp(s, 4, H);   // a table under way is never probed: the next untouched one is
  grade(s, cardOf(4, 1), EASY, H);
  CHECK(probeCard(s) == cardOf(5, PROBE_FROM));
}

// A missed card comes back AGAIN_AFTER cards later, a slow one SLOW_AFTER later; once each, five at most a round.
static void putBack() {
  Save s;
  Round r;
  setUp(s, 4, H);
  build(r, s, H);
  const int first = r.current();
  answer(r, s, false, FAST, H);
  for (int i = 0; i < AGAIN_AFTER; i++) { CHECK(r.current() != first && !r.redo[r.at]); answer(r, s, true, FAST, H); }
  CHECK(r.current() == first && r.redo[r.at] && r.number() == 1 + AGAIN_AFTER && r.wrong == 1);   // "One more try"
  answer(r, s, false, FAST, H);   // missed again: not put back twice
  const int slow = r.current();
  answer(r, s, true, SLOW, H);
  for (int i = 0; i < SLOW_AFTER; i++) { CHECK(r.current() != slow); answer(r, s, true, FAST, H); }
  CHECK(r.current() == slow && r.redo[r.at]);
  while (!r.done()) answer(r, s, true, FAST, H);
  CHECK(r.n == ROUND + 2 && r.base == ROUND && r.wrong == 2 && s.boxOf(first) == 1 && s.dueOf(first) == H + 1);
  build(r, s, H);   // an all-missed round grows by AGAIN_ROOM cards at most
  while (!r.done()) answer(r, s, false, FAST, H);
  CHECK(r.n == MAX_QUEUE && r.wrong == MAX_QUEUE);
}

// Division: a known multiplication opens its two divisions (fact families, 1x last), once the kid is on DIV_FROM_LEVEL.
static void division() {
  CHECK(answerOf(divOf(8, 7)) == 7 && dividendOf(divOf(8, 7)) == 56 && isDiv(divOf(8, 7)) && !isDiv(cardOf(10, 10)));
  CHECK(divisorOf(divOf(10, 10)) == 10 && quotientOf(divOf(2, 1)) == 1 && divOf(10, 10) == ALL - 1);
  Save s;
  bool taken[ALL] = {};
  setUp(s, 3, H);   // a younger kid: 1x and 2x known, but division waits for 4x
  CHECK(nextDivision(s, taken) < 0);
  setUp(s, 4, H);
  const int order[] = {divOf(2, 1), divOf(2, 2), divOf(2, 3), divOf(3, 2), divOf(2, 4), divOf(4, 2)};
  for (int c : order) { CHECK(nextDivision(s, taken) == c); taken[c] = true; }   // 2 / 2, 4 / 2, 6 / 2, 6 / 3, 8 / 2, 8 / 4
  memset(taken, 0, sizeof taken);
  for (int c = CARDS; c < ALL; c++) if (c != divOf(4, 7) && c != divOf(7, 4)) s.boxOf(c) = KNOWN_BOX;
  CHECK(nextDivision(s, taken) < 0);   // 28 / 4 and 28 / 7 wait for 4x7
  grade(s, cardOf(4, 7), EASY, H);
  grade(s, cardOf(4, 7), EASY, H + 2);
  CHECK(nextDivision(s, taken) == divOf(4, 7));
  const int c = divOf(7, 4);   // a division card learns like any other, and never moves the level (the tables do)
  grade(s, c, EASY, H + 2);
  grade(s, c, EASY, H + 4);
  CHECK(s.boxOf(c) == MASTER_BOX && s.level == 4 && learningOf(s, true) == 0);
  // Divisions take about a third of the set: two multiplications being learned and no division, so a division first.
  setUp(s, 4, H);
  grade(s, cardOf(4, 1), HARD, H);
  grade(s, cardOf(4, 2), HARD, H);
  Round r;
  CHECK(newCard(r, s) == divOf(2, 1));
  grade(s, divOf(2, 1), HARD, H);
  CHECK(newCard(r, s) == cardOf(4, 3));
}

// ---- the save
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
// An older deck's cards as this version holds them: the same boxes, its days as half-days, nothing timed yet.
static bool sameInHalves(const Save& now, const Save& old) {
  bool same = now.level == old.level && now.start == old.start && now.answers == old.answers && !now.pace &&
              !now.step && !now.logVar && !now.jumps && !now.tries;
  for (int c = 0; c < ALL; c++)
    same &= now.boxOf(c) == old.boxOf(c) && now.dueOf(c) == old.dueOf(c) * 2 && now.seenOf(c) == old.seenOf(c) * 2;
  return same;
}
static void saves() {
  Save s;
  setUp(s, 3, H);
  grade(s, cardOf(3, 5), EASY, H);
  noteTime(s, cardOf(2, 2), 2500); s.step = -1; s.jumps = 3;
  seal(s);   // whole, as version 5: read back as it is
  Save back;
  CHECK(s.version == SAVE_WHOLE && loadBlob(&s, sizeof s, back) && !memcmp(&back, &s, sizeof s));
  Save bad = s;
  bad.boxOf(0) ^= 1;
  CHECK(!loadBlob(&bad, sizeof bad, back));   // a damaged blob: a fresh deck instead
  CHECK(!loadBlob(&s, sizeof s - 4, back));
  Save days;   // an older deck, in days
  setUp(days, 3, H / 2);
  grade(days, cardOf(3, 5), EASY, H / 2);
  for (uint16_t version = 1; version <= SAVE_V3; version++) {   // whole: 1 from before division, 2 before the grading check
    const size_t size = version == 1 ? SAVE_V1_SIZE : version == 2 ? SAVE_V2_SIZE : sizeof(Save);
    uint8_t old[sizeof(Save)];
    Save o = days;
    o.magic = SAVE_MAGIC; o.version = version; o.size = (uint16_t)size;
    memset(&o.logMu, 0xA5, offsetof(Save, crc) - offsetof(Save, logMu));   // version 3's grading check, counting
    memcpy(old, &o, size - 4);
    uint32_t crc = os::crc32(old, size - 4);
    memcpy(old + size - 4, &crc, 4);
    CHECK(loadBlob(old, size, back) && sameInHalves(back, days));
    for (int c = CARDS; c < ALL && version == 1; c++) CHECK(!back.boxOf(c));
    old[20] ^= 1;
    CHECK(!loadBlob(old, size, back));
    old[20] ^= 1;
    o.version = (uint16_t)(version % 3 + 1);   // a size that says one version and a field that says another: refused
    memcpy(old, &o, size - 4);
    crc = os::crc32(old, size - 4);
    memcpy(old + size - 4, &crc, 4);
    CHECK(!loadBlob(old, size, back));
  }
  s.dueOf(cardOf(3, 5)) = (uint16_t)(H + 900);   // a clock set back: no card waits past its box's interval
  clampDue(s, H);
  CHECK(s.dueOf(cardOf(3, 5)) == H + INTERVAL[2]);
}

static uint16_t panel[gfx565::W * gfx565::H];
static uint32_t ms = 1000;
static const uint32_t NOW = (uint32_t)H * 43200u + 3600;
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
static void settle(Game& g, int frames = 15) { for (int i = 0; i < frames; i++) frame(g); }
static bool on(const Game& g, const char* s) { return !strcmp(g.screenName(), s); }

// A grown-up holds the title for two seconds: how it's going; a short press does nothing.
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

// The real decks in the field: a version 3 one (whole) and the same deck as version 4 (packed) load as the same
// version 6 deck in half-days, and the game stores it anew, packed, as soon as it is opened.
static void fieldDecksMove() {
  Save raw;
  memcpy(&raw, TIMES_CARDS_SAVE_V3, sizeof raw);
  Save three, four;
  CHECK(storedVersion(TIMES_CARDS_SAVE_V3, sizeof TIMES_CARDS_SAVE_V3) == SAVE_V3 &&
        loadBlob(TIMES_CARDS_SAVE_V3, sizeof TIMES_CARDS_SAVE_V3, three));
  CHECK(storedVersion(TIMES_CARDS_SAVE_V4, sizeof TIMES_CARDS_SAVE_V4) == SAVE_V4 &&
        loadBlob(TIMES_CARDS_SAVE_V4, sizeof TIMES_CARDS_SAVE_V4, four));
  CHECK(sameInHalves(three, raw) && !memcmp(&three, &four, sizeof three) && three.level == 10 && three.answers == 1995);
  size_t n = 0;
  CHECK(packsExactly(three, &n) && n < 420);
  Save fresh; setUp(fresh, 4, H); size_t small = 0;
  CHECK(packsExactly(fresh, &small) && small < 200);
  printf("times cards save: version 3 1020 bytes, version 4 %u; version 6 %u new, %u four months in\n",
         (unsigned)sizeof TIMES_CARDS_SAVE_V4, (unsigned)small, (unsigned)n);

  Game g;
  Profile zoe = {1, "Zoe", 0, 8, 0};
  SaveSlot slot = {TIMES_CARDS_SAVE_V4, sizeof TIMES_CARDS_SAVE_V4};
  AppEnter e = {&zoe, &zoe, &slot, 1, 20830u * 86400u, ms, true};   // the day after the deck's last round
  uint32_t wallet = 0;
  e.stars = &wallet;
  g.enter(e);
  const void* data; size_t len;
  CHECK(g.takeSave(&data, &len, true) && len == n && storedVersion(data, len) == SAVE_VERSION);
  CHECK(!memcmp(&g.save(), &three, sizeof three));   // opening moved nothing
  uint8_t stored[sizeof(Save)]; Save back;
  memcpy(stored, data, len);
  CHECK(loadBlob(stored, len, back) && !memcmp(&back, &three, sizeof three));
  slot = {stored, len};   // opened again on the packed deck: the same deck, and nothing to write
  g.enter(e);
  CHECK(!g.takeSave(&data, &len, true) && g.save().answers == 1995 && !memcmp(g.save().box, back.box, sizeof back.box));
}
// One bit wrong anywhere, or a byte short or long: not read.
static void damagedPacked() {
  Save deck, back;
  CHECK(loadBlob(TIMES_CARDS_SAVE_V3, sizeof TIMES_CARDS_SAVE_V3, deck));
  noteTime(deck, cardOf(3, 4), 3000); deck.step = 1; deck.tries = 40; deck.rights = 33;
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
  uint8_t old[sizeof TIMES_CARDS_SAVE_V4];   // the version 4 deck, damaged anywhere: not read either
  memcpy(old, TIMES_CARDS_SAVE_V4, sizeof old);
  for (size_t bit = 0; bit < sizeof old * 8; bit++) {
    old[bit / 8] ^= (uint8_t)(1u << bit % 8);
    read |= loadBlob(old, sizeof old, back);
    old[bit / 8] ^= (uint8_t)(1u << bit % 8);
  }
  CHECK(!read);
  CHECK(!pack(deck, blob, 11) && !pack(deck, blob, n - 1) && pack(deck, blob, n) == n);   // too little room: nothing kept
  Save odd = deck;
  odd.boxOf(7) = MAX_BOX + 1;   // no loaded deck has one; it would not survive three bits, so it is stored whole
  CHECK(!pack(odd, blob, sizeof(Save)));
  odd = deck; odd.step = STEP_MAX + 1;
  CHECK(!pack(odd, blob, sizeof(Save)));
  odd = deck; odd.spare6[3] = 1;
  CHECK(!pack(odd, blob, sizeof(Save)));
}
// Any deck at all, not only one play can make: every field at random, days close together or all over the calendar.
static void anyDeckPacks() {
  uint32_t r = 2026;
  for (int t = 0; t < 600; t++) {
    Save s{};
    const uint32_t span = 1u << (t % 17), from = nextRand(r);
    s.level = (uint8_t)(nextRand(r) % (TABLES + 1)); s.start = (uint8_t)nextRand(r); s.answers = nextRand(r);
    for (int c = 0; c < ALL; c++) {
      if (nextRand(r) % 4) s.boxOf(c) = (uint8_t)(nextRand(r) % (MAX_BOX + 1));
      if (nextRand(r) % 3) s.seenOf(c) = (uint16_t)(from + nextRand(r) % span);
      if (nextRand(r) % 5) s.dueOf(c) = (uint16_t)(t % 2 ? s.seenOf(c) + nextRand(r) % 140 : from + nextRand(r) % span);
    }
    s.logMu = (int16_t)nextRand(r); s.logVar = (uint16_t)nextRand(r); s.probeMiss = (uint16_t)nextRand(r);
    s.step = (int8_t)((int)(nextRand(r) % 5) + STEP_MIN); s.streak = (uint8_t)nextRand(r);
    s.jumps = (uint16_t)nextRand(r); s.tries = (uint16_t)nextRand(r); s.rights = (uint16_t)nextRand(r);
    s.spare = t % 3 ? 0 : (uint16_t)nextRand(r);
    CHECK(packsExactly(s));
  }
}
// Decks as kids make them: once or twice a day for half a year, for each starting table and a few kinds of kid.
static void playedDecksPack() {
  size_t most = 0;
  for (int kid = 0; kid < 12; kid++) {
    Save s; int32_t half = H + kid * 800; uint32_t r = 7 + (uint32_t)kid;
    setUp(s, 1 + kid % 4, half);
    for (int d = 0; d < 360; d++, half++) {
      if (d % 14 == kid % 14 || (d > 100 && d < 100 + kid * 6) || (d + kid) % 3 == 0) continue;   // halves off, weeks away
      clampDue(s, half);
      startSession(s);
      for (int round = 0; round < 1 + (d + kid) % 3; round++) {
        Round q; build(q, s, half);
        while (!q.done()) {
          const uint32_t pickMs = 800 + nextRand(r) % (2000 + 700 * (uint32_t)kid);
          answer(q, s, nextRand(r) % 100 < (uint32_t)(70 + kid * 2), pickMs, half);
        }
      }
      size_t n = 0;
      CHECK(packsExactly(s, &n) && s.level >= 1 && s.level <= TABLES);
      if (n > most) most = n;
    }
  }
  CHECK(most < 480);   // 17 NVS entries; host/test_nvs_budget.cpp holds the household's budget
  printf("times cards save: at most %u bytes in half a year of play, %u for any deck at all\n", (unsigned)most, (unsigned)g_most);
}

// A stored deck this build cannot read (a newer build's after a rollback, or damaged) is not replaced by a fresh one
// just because the game was opened, looked at and left; only once the kid answers a card on the fresh deck.
static void unreadableDeckKept() {
  uint8_t newer[sizeof TIMES_CARDS_SAVE_V3];
  memcpy(newer, TIMES_CARDS_SAVE_V3, sizeof newer);
  newer[4] = 9;   // a version this build does not know
  Game g;
  Profile zoe = {1, "Zoe", 0, 8, 0};
  SaveSlot slot = {newer, sizeof newer};
  AppEnter e = {&zoe, &zoe, &slot, 1, NOW, ms, true};
  uint32_t wallet = 0;
  e.stars = &wallet;
  g.enter(e);
  settle(g);
  const void* data; size_t len;
  CHECK(on(g, "cards_home") && g.save().level == 4 && !g.takeSave(&data, &len, true));   // a fresh deck to play, not stored
  tapBox(g, 80, 133);   // Go!
  settle(g);
  tapBox(g, 80, 70);    // the card turned: still nothing of the kid's to keep
  CHECK(on(g, "cards_answer") && !g.takeSave(&data, &len, true));
  g.leave();
  CHECK(!g.takeSave(&data, &len, true));
  g.enter(e);           // opened again: the same, however often
  settle(g);
  CHECK(!g.takeSave(&data, &len, true));
  tapBox(g, 80, 133);
  settle(g);
  tapBox(g, 80, 70);
  settle(g);
  tapBox(g, 52, 74);   // an answer: the kid's first on the new deck, which is now theirs
  CHECK(g.save().answers == 1 && g.takeSave(&data, &len, true) && storedVersion(data, len) == SAVE_VERSION);
}

static void answerRight(Game& g) {   // turn the card and pick the right answer
  settle(g);
  tapBox(g, 80, 70);
  CHECK(on(g, "cards_answer"));
  settle(g, 5);
  g.debugCmd("right");
  settle(g);
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
  tapBox(g, 80, 70);   // the card: four answers
  CHECK(on(g, "cards_answer"));
  settle(g, 5);
  tapBox(g, 80, 74);   // between the answers: nothing
  CHECK(on(g, "cards_answer") && g.save().answers == 0);
  g.debugCmd("wrong");   // a miss: the right answer shows for two seconds, then the next card
  CHECK(g.save().answers == 1 && g.save().step == -1);
  settle(g, 20);
  CHECK(on(g, "cards_answer"));
  for (int i = 0; i < 6; i++) tapBox(g, 52, 74);   // taps while it shows do nothing
  CHECK(g.save().answers == 1);
  settle(g, 40);
  CHECK(on(g, "cards_ask"));
  for (int i = 1; i < ROUND + 1; i++) answerRight(g);   // the rest, and the missed card's second look
  CHECK(on(g, "cards_done") && g.save().answers == ROUND + 1 && g.save().step >= 1);
  CHECK(wallet == 7 + ROUND_STARS);   // a finished round pays the kid's shared stars
  CHECK(g.takeSave(&data, &len, true));
  uint8_t saved[sizeof(Save)]; const size_t savedLen = len; memcpy(saved, data, len);
  settle(g);
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
  g.debugCmd("card150");
  CHECK(on(g, "cards_ask"));
  // Back again: the saved deck, not a new one.
  SaveSlot slot = {saved, savedLen};
  e.saves = &slot;
  g.enter(e);
  CHECK(g.save().answers == ROUND + 1 && g.save().level == 4 && g.save().step <= STEP_CARRY);   // a missed fact: the level stays
  g.debugPrint();
}

int main() {
  start();
  facts();
  grades();
  levels();
  timing();
  timeFollows();
  options();
  nearMisses();
  staircase();
  rounds();
  steps();
  jumps();
  noJump();
  putBack();
  division();
  saves();
  fieldDecksMove();
  damagedPacked();
  anyDeckPacks();
  playedDecksPack();
  unreadableDeckKept();
  pages();
  printf("test_times_cards: %d checks ok\n", checks);
  return 0;
}
