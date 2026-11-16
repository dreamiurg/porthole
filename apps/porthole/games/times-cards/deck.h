// Times Cards' rules: the times tables as flash cards, scheduled by spaced repetition (Leitner boxes), one deck per kid.
// Header-only and free of drawing, so host/test_times_cards.cpp checks it alone.
//
// A card is one fact, table x n (n 1..10), tables 1..10. Its box says how well the kid knows it: 0 never shown, 1
// learning, and each Easy on a day the card was due moves it one box up, to wait longer (INTERVAL) before it comes back.
// A card still being learned (below MASTER_BOX) also moves up on Easy any day after the one it was last seen, so a fact
// is known after Easy on two separate days. Hard keeps its box and brings it back tomorrow; Wrong puts it back in box 1
// and shows it again later in the same round. A known card answered before it was due never moves up: only a
// remembered card earns a longer wait.
//
// Pace: new facts come in table order (2x1, 2x2, ... 2x10, 3x1, ...), a few at a time, never all of a table at once. At
// most WORKING_SET facts are being learned (shown, not yet known) at once, so the next table's first facts arrive while
// the last ones of the table before are still settling: with six in the set, the next table starts once half of the
// one before is known. How many come in a round follows the kid's pace: a running score of their answers on cards
// already met, aimed at about 85% right; a kid who remembers well gets up to NEW_PER_ROUND, one who is missing a lot
// gets none until the set clears. So each kid moves through all the tables at their own speed. The deck's level is
// the first table not yet fully known: it is what the home page shows, and finishing it is the "Next up" moment.
//
// Division (3.OA.7): once the kid is on DIV_FROM_LEVEL, each known multiplication fact opens its division partners
// (6 x 7 known: 42 / 6 and 42 / 7) as new cards on the same boxes, schedule and pace. Divisions take about a third of
// the working set's places (a fact family's two together), so the tables keep moving while the divisions of the
// facts already known come in. Division by 1 is left out: it says nothing a kid doesn't know.
//
// Time is Porthole's local wall-clock seconds, so a day is now / 86400. Nothing punishes being away: a card that waited
// longer than planned is simply due, oldest first.
//
// Save: one blob per kid (timescards/s<id>, written by the shell). The Save struct below is how the game holds a deck
// and was what versions 1 to 3 stored, whole (1020 bytes by version 3). Version 4 stores the same deck bit-packed
// (pack, a few hundred bytes): the board's NVS holds 20 KB for the whole household, and four kids' decks did not fit.
// Every older version still loads. The struct stays append-only: a new field goes right before crc (its zero must be
// a sensible default), joins packDeck, and SAVE_VERSION goes up; nothing is reordered or resized.
#pragma once
#include <stddef.h>
#include <stdint.h>
#include <string.h>
#include "bits.h"
#include "crc32.h"

namespace cards {
constexpr const char* STORE = "timescards";   // NVS namespace: never renamed once shipped
constexpr uint32_t SAVE_MAGIC = 0x53445243;   // "CRDS"
constexpr uint16_t SAVE_V3 = 3;   // the struct, whole. 1: multiplication only (520 bytes); 2: division appended (972); 3: grading check
constexpr uint16_t SAVE_VERSION = 4;   // what is stored now: the version 3 struct, bit-packed

// ---- tuning knobs
constexpr int TABLES = 10, FACTS = 10, CARDS = TABLES * FACTS;   // 1x1 .. 10x10: cards 0..CARDS-1
constexpr int DIVS = (TABLES - 1) * FACTS;   // n / d for divisors 2..10 and quotients 1..10: cards CARDS..ALL-1
constexpr int ALL = CARDS + DIVS;
constexpr int DIV_FROM_LEVEL = 4;        // division joins once 1x to 3x are known (a third-grade standard)
constexpr int ROUND = 10;                // cards in a round (before Wrong ones come back)
constexpr int MIN_ROUND = 5;             // a shorter round is topped up with known facts: a one-card round feels broken
constexpr int NEW_PER_ROUND = 4;         // at most this many never-seen cards in one round
constexpr int WORKING_SET = 6;           // at most this many facts being learned (shown, below MASTER_BOX) at once
constexpr int PACE_START = 850;          // the pace (thousandths right) assumed before a kid has answered anything
constexpr int PACE_STEPS[NEW_PER_ROUND] = {600, 700, 800, 900};   // a pace this high lets 1, 2, 3, 4 new cards in
constexpr int KNOWN_MIX = 4;             // below the third pace step, this many known facts mixed into a round
constexpr int PACE_WEIGHT = 8;           // the pace moves 1/8 of the way toward each answer: about the last two rounds
constexpr int AGAIN_AFTER = 3;           // a Wrong card comes back after this many others...
constexpr int AGAIN_MAX = 1;             // ...at most this many times a round
constexpr int AGAIN_ROOM = 5;            // and a round grows by at most this many put-back cards
constexpr uint8_t MASTER_BOX = 3;        // a fact is known from here: Easy on two separate days (new -> 2 -> 3)
constexpr uint8_t KNOWN_BOX = 5;         // the tables below where a kid starts: known, reviewed now and then
constexpr uint8_t MAX_BOX = 7;
constexpr uint8_t INTERVAL[MAX_BOX + 1] = {0, 1, 2, 4, 7, 14, 30, 60};   // days until a card in box b is due again
constexpr int MAX_QUEUE = ROUND + AGAIN_ROOM;
constexpr uint32_t QUICK_MS = 1000;      // a card turned this soon after it showed was hardly recalled first
constexpr uint32_t THINK_CAP_MS = 10000; // one card's think time counts at most this much toward the average
constexpr uint32_t ROUND_STARS = 2;   // stars (os/stars.h) for every round finished, however it went: trying is the work

// The table a kid starts on, from the age on their profile: 6 starts on 2x, 8 on 4x (lower tables count as known).
inline int startTable(int age) { const int t = age - 4; return t < 1 ? 1 : t > 4 ? 4 : t; }
inline int tableOf(int card) { return card / FACTS + 1; }
inline int factorOf(int card) { return card % FACTS + 1; }
inline int cardOf(int table, int n) { return (table - 1) * FACTS + n - 1; }
inline bool isDiv(int card) { return card >= CARDS; }
inline int divOf(int divisor, int quotient) { return CARDS + (divisor - 2) * FACTS + quotient - 1; }
inline int divisorOf(int card) { return (card - CARDS) / FACTS + 2; }
inline int quotientOf(int card) { return (card - CARDS) % FACTS + 1; }
inline int dividendOf(int card) { return divisorOf(card) * quotientOf(card); }
inline int answerOf(int card) { return isDiv(card) ? quotientOf(card) : tableOf(card) * factorOf(card); }
inline int32_t dayOf(uint32_t now) { return (int32_t)(now / 86400u); }

struct Save {
  uint32_t magic;
  uint16_t version, size;
  uint8_t level;            // the first table not fully known (1..TABLES); 0: a fresh deck, set up from the kid's age
  uint8_t start;            // the table the kid started on
  uint16_t pace;            // thousandths right on cards already met, plus one; 0: none yet (PACE_START)
  uint32_t answers;         // cards answered, ever
  uint8_t box[CARDS];       // by card: 0 never shown, 1..MAX_BOX
  uint16_t due[CARDS];      // by card: the day (dayOf) it is due again; meaningless while box is 0
  uint16_t seen[CARDS];     // by card: the day it was last answered; 0 never (or set up as known)
  uint8_t divBox[DIVS];     // v2: the same three for the division cards (card - CARDS)
  uint16_t divDue[DIVS];
  uint16_t divSeen[DIVS];
  uint16_t spare;           // 0; keeps crc aligned
  // v3, the grading check: does a young kid grade themselves honestly? (docs/design/adaptive-practice.md) Counted
  // from checkFrom on, read on the grown-ups' page; a second look at a missed card in the same round never counts.
  uint32_t thinkMs;         // time from a card showing to its turn, summed, each at most THINK_CAP_MS
  uint16_t checkFrom;       // the day counting started; 0 not yet
  uint16_t firstSight[3];   // grades on a card's very first showing: Easy, Hard, Wrong
  uint16_t nextDay[2];      // a card last graded Easy, at its next showing on a later day: Easy again, or not
  uint16_t turns, quick;    // cards turned, and turned within QUICK_MS of showing
  uint16_t quickEasy;       // of those quick turns, graded Easy
  uint8_t easyLast[(ALL + 7) / 8];   // by card, one bit: its last grade was Easy
  uint16_t spare3;          // 0; keeps crc aligned
  uint32_t crc;
  // Any card, multiplication or division.
  uint8_t& boxOf(int c) { return c < CARDS ? box[c] : divBox[c - CARDS]; }
  uint16_t& dueOf(int c) { return c < CARDS ? due[c] : divDue[c - CARDS]; }
  uint16_t& seenOf(int c) { return c < CARDS ? seen[c] : divSeen[c - CARDS]; }
  uint8_t boxOf(int c) const { return c < CARDS ? box[c] : divBox[c - CARDS]; }
  uint16_t dueOf(int c) const { return c < CARDS ? due[c] : divDue[c - CARDS]; }
  uint16_t seenOf(int c) const { return c < CARDS ? seen[c] : divSeen[c - CARDS]; }
};
constexpr size_t SAVE_V1_SIZE = 520, SAVE_V2_SIZE = 972;   // older versions: their fields, then their crc
static_assert(offsetof(Save, divBox) == SAVE_V1_SIZE - 4, "version 1's fields are kept where they were");
static_assert(offsetof(Save, thinkMs) == SAVE_V2_SIZE - 4, "version 2's fields are kept where they were");
static_assert(sizeof(Save) == 1020, "Save is persisted: append before crc, never resize");

// Seals the struct as version 3: what pack() stores packed, and what is stored whole if a deck ever cannot be packed.
inline void seal(Save& s) {
  s.magic = SAVE_MAGIC; s.version = SAVE_V3; s.size = sizeof(Save);
  s.crc = os::crc32(&s, sizeof s - sizeof s.crc);
}

// ---- version 4: the deck, bit-packed. Stored: magic, version, total bytes (the struct's first eight bytes, so the
// version reads the same way in every version), the fields below, crc of all before it. One description writes and
// reads (os::Bits::io), so the two cannot drift apart; writing leaves the deck as it was.
// A day, or 0: one bit, then its distance from the deck's earliest day in `w` bits.
inline uint16_t packDay(os::Bits& b, uint16_t day, uint16_t base, int w) {
  return b.io(day != 0, 1) ? (uint16_t)(base + b.io((uint16_t)(day - base), w)) : 0;
}
// One card: its box; nothing more if it was never shown; the day it was last seen; and how its due day follows from
// those (the box's wait, tomorrow, another day within nine weeks) or, failing all three, the due day itself.
inline void packCard(os::Bits& b, Save& s, int c, uint16_t base, int w) {
  const uint8_t box = s.boxOf(c) = (uint8_t)b.io(s.boxOf(c), 3);
  if (!box && !b.io(s.dueOf(c) || s.seenOf(c), 1)) return;
  const uint16_t seen = s.seenOf(c) = packDay(b, s.seenOf(c), base, w);
  const uint16_t from = seen ? seen : base, gap = (uint16_t)(s.dueOf(c) - from), wait = INTERVAL[box & MAX_BOX];
  const uint32_t how = b.io(gap == wait ? 0 : gap == 1 ? 1 : gap < 64 ? 2 : 3, 2);
  s.dueOf(c) = how == 3 ? packDay(b, s.dueOf(c), base, w) : (uint16_t)(from + (how == 0 ? wait : how == 1 ? 1 : b.io(gap, 6)));
}
inline void packDeck(os::Bits& b, Save& s) {
  uint16_t first = 0xFFFF, last = 0;   // the deck's earliest and latest day; read back from the blob when unpacking
  auto span = [&](uint16_t day) { if (day) { first = day < first ? day : first; last = day > last ? day : last; } };
  for (int c = 0; c < ALL; c++) { span(s.dueOf(c)); span(s.seenOf(c)); }
  const uint16_t base = (uint16_t)b.io(last ? first : 0, 16);
  const int w = (int)b.io(last ? os::bitsFor((uint32_t)(last - first)) : 0, 5);
  if (w > 16) { b.bad = true; return; }
  s.level = (uint8_t)b.io(s.level, 4); s.start = (uint8_t)b.io(s.start, 8);
  s.pace = (uint16_t)b.io(s.pace, 16); s.answers = b.io(s.answers, 32);
  for (int c = 0; c < ALL; c++) packCard(b, s, c, base, w);
  s.spare = (uint16_t)b.sparse(s.spare, 16);
  s.thinkMs = b.io(s.thinkMs, 32); s.checkFrom = (uint16_t)b.io(s.checkFrom, 16);
  for (int i = 0; i < 3; i++) s.firstSight[i] = (uint16_t)b.io(s.firstSight[i], 16);
  for (int i = 0; i < 2; i++) s.nextDay[i] = (uint16_t)b.io(s.nextDay[i], 16);
  s.turns = (uint16_t)b.io(s.turns, 16); s.quick = (uint16_t)b.io(s.quick, 16); s.quickEasy = (uint16_t)b.io(s.quickEasy, 16);
  for (size_t i = 0; i < sizeof s.easyLast; i++) s.easyLast[i] = (uint8_t)b.io(s.easyLast[i], 8);
  s.spare3 = (uint16_t)b.sparse(s.spare3, 16);
}
// The deck as a version 4 blob in out (at most cap bytes). 0 if it cannot be packed (a box past MAX_BOX, or days so
// scattered it would not fit): the caller then stores the struct whole, as version 3.
inline size_t pack(Save& s, uint8_t* out, size_t cap) {
  if (cap < 12) return 0;
  memset(out, 0, cap);
  os::Bits b = os::Bits::writer(out, cap - 4);
  b.io(SAVE_MAGIC, 32); b.io(SAVE_VERSION, 16); b.io(0, 16);
  packDeck(b, s);
  if (!b.ok()) return 0;
  const uint16_t bytes = (uint16_t)(b.bytes() + 4);
  memcpy(out + 6, &bytes, sizeof bytes);
  const uint32_t crc = os::crc32(out, bytes - sizeof crc);
  memcpy(out + bytes - sizeof crc, &crc, sizeof crc);
  return bytes;
}
inline bool unpack(const uint8_t* data, size_t n, Save& zeroed) {
  uint32_t crc;
  memcpy(&crc, data + n - sizeof crc, sizeof crc);
  os::Bits b = os::Bits::reader(data, n - sizeof crc);
  if (b.io(0, 32) != SAVE_MAGIC || b.io(0, 16) != SAVE_VERSION || b.io(0, 16) != n) return false;
  if (crc != os::crc32(data, n - sizeof crc)) return false;
  packDeck(b, zeroed);
  if (!b.ok() || b.bytes() != n - sizeof crc || zeroed.level > TABLES) return false;
  seal(zeroed);
  return true;
}
// The version a stored blob says it is (in the same place in every version); 0 if it is too short to be a deck.
inline uint16_t storedVersion(const void* data, size_t n) {
  uint16_t version = 0;
  if (data && n >= 12) memcpy(&version, (const uint8_t*)data + offsetof(Save, version), sizeof version);
  return version;
}
// A blob is valid as version 4, or as an older version at exactly that version's size (kept, the newer fields zero: no
// division met, no grading counted yet); anything else (damaged, a newer firmware's) is not a deck this build reads.
inline bool loadBlob(const void* data, size_t n, Save& out) {
  Save s{};
  if (storedVersion(data, n) == SAVE_VERSION) {
    if (!unpack((const uint8_t*)data, n, s)) return false;
    out = s;
    return true;
  }
  const uint16_t version = n == SAVE_V1_SIZE ? 1 : n == SAVE_V2_SIZE ? 2 : n == sizeof(Save) ? SAVE_V3 : 0;
  if (!version) return false;
  const size_t body = n - sizeof s.crc;
  uint32_t crc;
  memcpy(&s, data, body);
  memcpy(&crc, (const uint8_t*)data + body, sizeof crc);
  if (s.magic != SAVE_MAGIC || s.version != version || s.size != n || s.level > TABLES ||
      crc != os::crc32(data, body)) return false;
  s.crc = crc;   // resealed at the next save
  for (int c = 0; c < ALL; c++) if (s.boxOf(c) > MAX_BOX) s.boxOf(c) = MAX_BOX;
  out = s;
  return true;
}

// A fresh deck for a kid starting on `table`: its facts and the higher tables' are new; every lower table is known,
// spread over the next two weeks so a few of them come back each day.
inline void setUp(Save& s, int table, int32_t today) {
  s = Save{};
  s.level = s.start = (uint8_t)table;
  for (int c = 0; c < cardOf(table, 1); c++) {
    s.boxOf(c) = KNOWN_BOX;
    s.dueOf(c) = (uint16_t)(today + 1 + c % INTERVAL[KNOWN_BOX]);
  }
}
// A clock set back (or a save from the future) leaves due days far ahead: none waits longer than its box says.
inline void clampDue(Save& s, int32_t today) {
  for (int c = 0; c < ALL; c++)
    if (s.boxOf(c) && (int32_t)s.dueOf(c) > today + INTERVAL[s.boxOf(c)]) s.dueOf(c) = (uint16_t)(today + INTERVAL[s.boxOf(c)]);
}
inline bool isDue(const Save& s, int card, int32_t today) { return s.boxOf(card) && (int32_t)s.dueOf(card) <= today; }
inline int known(const Save& s, int table) {   // facts of `table` in MASTER_BOX or higher
  int k = 0;
  for (int n = 1; n <= FACTS; n++) k += s.boxOf(cardOf(table, n)) >= MASTER_BOX;
  return k;
}
inline bool allKnown(const Save& s) { return s.level == TABLES && known(s, TABLES) == FACTS; }
enum Grade : uint8_t { EASY, HARD, WRONG };
inline int learning(const Save& s) {   // facts shown and not yet known
  int k = 0;
  for (int c = 0; c < ALL; c++) k += s.boxOf(c) && s.boxOf(c) < MASTER_BOX;
  return k;
}
inline int paceOf(const Save& s) { return s.pace ? s.pace - 1 : PACE_START; }
// One answer on a card already met: Easy counts as right, Hard as half, Wrong as nothing.
inline void notePace(Save& s, Grade g) {
  const int got = g == EASY ? 1000 : g == HARD ? 500 : 0;
  const int p = paceOf(s);
  s.pace = (uint16_t)(p + (got - p) / PACE_WEIGHT + 1);
}
// New cards this round may bring: by the kid's pace, never past the working set, and at least one when nothing is
// being learned (so a kid who knows everything met so far always moves on).
inline int newAllowed(const Save& s) {
  int n = 0;
  while (n < NEW_PER_ROUND && paceOf(s) >= PACE_STEPS[n]) n++;
  const int room = WORKING_SET - learning(s), floor = room == WORKING_SET ? 1 : 0;
  const int k = n > floor ? n : floor;
  return k < room ? k : room;
}

// Grades one card. Returns true when this answer finished the level's table (the level moves to the next unfinished one).
// A real recall, which may move a card up: new, due, or still being learned and last seen on another day.
inline bool recalls(const Save& s, int card, int32_t today) {
  return !s.boxOf(card) || isDue(s, card, today) || (s.boxOf(card) < MASTER_BOX && s.seenOf(card) != (uint16_t)today);
}
inline uint8_t nextBox(uint8_t b, Grade g, bool counts) {
  if (g == WRONG) return 1;
  if (g == EASY && counts) return (uint8_t)(b < 1 ? 2 : b < MAX_BOX ? b + 1 : MAX_BOX);   // a new card known at once: box 2
  return b ? b : 1;
}
inline bool grade(Save& s, int card, Grade g, int32_t today) {
  const bool counts = recalls(s, card, today);
  s.boxOf(card) = nextBox(s.boxOf(card), g, counts);
  if (counts || g == WRONG) s.dueOf(card) = (uint16_t)(today + (g == EASY ? INTERVAL[s.boxOf(card)] : 1));
  else if (g == HARD && (int32_t)s.dueOf(card) > today + 1) s.dueOf(card) = (uint16_t)(today + 1);   // practice: sooner, never later
  s.seenOf(card) = (uint16_t)today;
  s.answers++;
  bool up = false;
  while (s.level < TABLES && known(s, s.level) == FACTS) { s.level++; up = true; }
  return up;
}

// The grading check. A card turned `ms` after it showed (a missed card's second look is left out).
inline void noteTurn(Save& s, uint32_t ms, bool redo) {
  if (redo) return;
  s.turns++;
  s.quick += ms < QUICK_MS;
  s.thinkMs += ms < THINK_CAP_MS ? ms : THINK_CAP_MS;
}
inline bool easyLast(const Save& s, int card) { return s.easyLast[card / 8] >> (card % 8) & 1; }
// A grade (not a missed card's second look), before the card's box moves: on a first showing, or (last graded Easy) on its next later day.
inline void noteGrade(Save& s, int card, Grade g, int32_t today, bool quick) {
  if (!s.checkFrom) s.checkFrom = (uint16_t)today;
  if (!s.boxOf(card)) s.firstSight[g]++;
  else if (easyLast(s, card) && s.seenOf(card) != (uint16_t)today) s.nextDay[g == EASY ? 0 : 1]++;
  s.quickEasy += quick && g == EASY;
  const uint8_t bit = (uint8_t)(1u << (card % 8));
  s.easyLast[card / 8] = (uint8_t)(g == EASY ? s.easyLast[card / 8] | bit : s.easyLast[card / 8] & ~bit);
}

// One round's cards, in the order shown. A Wrong card is put back AGAIN_AFTER cards on (or last).
struct Round {
  uint8_t card[MAX_QUEUE];
  uint8_t again[ALL];     // times each card was put back this round
  bool redo[MAX_QUEUE];   // by place: a Wrong card put back, shown as "One more try", not counted in "k of base"
  int n = 0, at = 0, base = 0;   // base: the round's cards before any was put back
  int easy = 0, hard = 0, wrong = 0;
  bool leveledUp = false;
  bool done() const { return at >= n; }
  int current() const { return card[at]; }
  int number() const { int k = 0; for (int i = 0; i <= at && i < n; i++) k += !redo[i]; return k; }   // k of base
};

inline uint32_t nextRand(uint32_t& r) { r ^= r << 13; r ^= r >> 17; r ^= r << 5; return r; }

// Adds the best unused card already met by `better` (better(c, -1): c qualifies) while r.n < upTo.
template <class Better>
inline void pick(Round& r, bool* used, const Save& s, Better better, int upTo) {
  while (r.n < upTo) {
    int best = -1;
    for (int c = 0; c < ALL; c++)
      if (!used[c] && s.boxOf(c) && better(c, best)) best = c;
    if (best < 0) return;
    used[best] = true; r.card[r.n++] = (uint8_t)best;
  }
}
inline int nextTimes(const Save& s, const bool* taken) {   // the next never-shown multiplication fact, in table order
  for (int c = 0; c < CARDS; c++) if (!s.box[c] && !taken[c]) return c;
  return -1;
}
// The next never-shown division fact that is open: by the known multiplications in table order (1x last, its
// families being n / n and n / 1), each family's two divisions together: 2 x 3 known gives 6 / 2, then 6 / 3.
inline int nextDivision(const Save& s, const bool* taken) {
  if (s.level < DIV_FROM_LEVEL) return -1;
  for (int i = FACTS; i < CARDS + FACTS; i++) {
    const int m = i % CARDS, t = tableOf(m), n = factorOf(m);
    if (s.box[m] < MASTER_BOX) continue;
    const int pair[2][2] = {{t, n}, {n, t}};   // divisor, quotient
    for (const auto& p : pair)
      if (p[0] >= 2 && !s.boxOf(divOf(p[0], p[1])) && !taken[divOf(p[0], p[1])]) return divOf(p[0], p[1]);
  }
  return -1;
}
inline int learningOf(const Save& s, bool division) {   // facts of one kind shown and not yet known
  int k = 0;
  for (int c = division ? CARDS : 0; c < (division ? ALL : CARDS); c++) k += s.boxOf(c) && s.boxOf(c) < MASTER_BOX;
  return k;
}
// The never-shown facts this round brings, as many as the pace allows: a division whenever the divisions being learned
// are fewer than half the multiplications (about a third of the set), else the next multiplication; either, when the
// other kind has none left. A half share slowed the tables by half in simulation; a third keeps most of their pace.
inline int newCards(const Save& s, int* fresh) {
  const int most = newAllowed(s);
  bool taken[ALL] = {};
  int times = learningOf(s, false), divs = learningOf(s, true), nf = 0;
  while (nf < most) {
    int c = divs * 2 < times ? nextDivision(s, taken) : -1;
    if (c < 0) c = nextTimes(s, taken);
    if (c < 0) c = nextDivision(s, taken);
    if (c < 0) break;
    taken[c] = true;
    fresh[nf++] = c;
    (isDiv(c) ? divs : times)++;
  }
  return nf;
}
inline void shuffle(Round& r, uint32_t seed) {
  uint32_t x = seed | 1;
  for (int i = r.n - 1; i > 0; i--) { const int j = (int)(nextRand(x) % (uint32_t)(i + 1)); const uint8_t t = r.card[i]; r.card[i] = r.card[j]; r.card[j] = t; }
}
// Easy wins between the hard ones: known facts not seen today, the soonest due first (early, so they never move up).
inline void mixKnown(Round& r, bool* used, const Save& s, int32_t today, int upTo) {
  pick(r, used, s, [&](int a, int b) { return s.boxOf(a) >= MASTER_BOX && s.seenOf(a) != (uint16_t)today && (b < 0 || s.dueOf(a) < s.dueOf(b)); },
       upTo);
}
// Which cards come first: the due ones, oldest due first (a lower box breaks a tie), up to the round less the room kept
// for new cards; then the new facts the pace allows (newCards); then practice: facts still being
// learned and not yet seen today (a missed one gets its second look in its own round), least-known first. While the
// kid is struggling (pace below the third step) practice leaves KNOWN_MIX places for known facts, so a hard round still
// has easy wins in it (interspersal, as in incremental rehearsal). Otherwise a known fact comes only when due, but a
// round shorter than MIN_ROUND is topped up with known facts not seen today. Only when nothing at all qualifies is the round practice from every
// fact met so far, the least-known and longest-waiting first. Shuffled, so the order never gives the answer away.
inline void build(Round& r, const Save& s, int32_t today, uint32_t seed) {
  r = Round{};
  bool used[ALL] = {};
  int fresh[NEW_PER_ROUND];
  const int nf = newCards(s, fresh);
  pick(r, used, s, [&](int a, int b) { return isDue(s, a, today) && (b < 0 || s.dueOf(a) < s.dueOf(b) || (s.dueOf(a) == s.dueOf(b) && s.boxOf(a) < s.boxOf(b))); },
       ROUND - nf);
  for (int i = 0; i < nf; i++) { used[fresh[i]] = true; r.card[r.n++] = (uint8_t)fresh[i]; }
  auto least = [&](int a, int b) { return b < 0 || s.boxOf(a) < s.boxOf(b) || (s.boxOf(a) == s.boxOf(b) && s.dueOf(a) < s.dueOf(b)); };
  const bool struggling = paceOf(s) < PACE_STEPS[2];
  pick(r, used, s, [&](int a, int b) { return s.boxOf(a) < MASTER_BOX && s.seenOf(a) != (uint16_t)today && least(a, b); },
       struggling ? ROUND - KNOWN_MIX : ROUND);
  if (struggling) mixKnown(r, used, s, today, ROUND);
  if (!r.n) pick(r, used, s, least, ROUND);
  else if (r.n < MIN_ROUND) mixKnown(r, used, s, today, MIN_ROUND);
  r.base = r.n;
  shuffle(r, seed);
}

// The kid's answer to the round's current card: graded, counted, put back if Wrong, and the round moves on.
inline void answer(Round& r, Save& s, Grade g, int32_t today, bool quick = false) {   // quick: turned within QUICK_MS
  const int c = r.current();
  if (!r.redo[r.at]) noteGrade(s, c, g, today, quick);
  if (s.boxOf(c) && !r.redo[r.at]) notePace(s, g);   // a first sight, or a second look a moment later, says little
  if (grade(s, c, g, today)) r.leveledUp = true;
  (g == EASY ? r.easy : g == HARD ? r.hard : r.wrong)++;
  if (g == WRONG && r.again[c] < AGAIN_MAX && r.n < MAX_QUEUE) {
    r.again[c]++;
    const int to = r.at + 1 + AGAIN_AFTER < r.n ? r.at + 1 + AGAIN_AFTER : r.n;
    for (int i = r.n; i > to; i--) { r.card[i] = r.card[i - 1]; r.redo[i] = r.redo[i - 1]; }
    r.card[to] = (uint8_t)c;
    r.redo[to] = true;
    r.n++;
  }
  r.at++;
}
}  // namespace cards
