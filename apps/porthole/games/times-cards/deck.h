// Times Cards' rules: the times tables as flash cards, scheduled by spaced repetition (Leitner boxes), one deck per kid.
// Header-only and free of drawing, so host/test_times_cards.cpp checks it alone.
//
// A card is one fact, table x n (n 1..10), tables 1..10. Its box says how well the kid knows it: 0 never shown, 1
// learning, and each Easy on a day the card was due moves it one box up, to wait longer (INTERVAL) before it comes back.
// A card still being learned (below MASTER_BOX) also moves up on Easy any day after the one it was last seen, so a fact
// is known after Easy on two separate days. Hard keeps its box and brings it back tomorrow; Wrong puts it back in box 1
// and shows it again later in the same round. A known card answered before it was due never moves up: only a
// remembered card earns a longer wait. The deck's level is the newest table: once every fact of it is in MASTER_BOX or
// higher, the next table's facts start coming in as new cards, and the old ones keep coming back for review.
//
// Time is Porthole's local wall-clock seconds, so a day is now / 86400. Nothing punishes being away: a card that waited
// longer than planned is simply due, oldest first.
//
// Save: one blob per kid (timescards/s<id>, written by the shell). Append-only from this first version: a new field goes
// right before crc (its zero must be a sensible default), SAVE_VERSION goes up, nothing is reordered or resized.
#pragma once
#include <stddef.h>
#include <stdint.h>
#include <string.h>
#include "crc32.h"

namespace cards {
constexpr const char* STORE = "timescards";   // NVS namespace: never renamed once shipped
constexpr uint32_t SAVE_MAGIC = 0x53445243;   // "CRDS"
constexpr uint16_t SAVE_VERSION = 1;

// ---- tuning knobs
constexpr int TABLES = 10, FACTS = 10, CARDS = TABLES * FACTS;   // 1x1 .. 10x10
constexpr int ROUND = 10;                // cards in a round (before Wrong ones come back)
constexpr int NEW_PER_ROUND = 4;         // at most this many never-seen cards in one round
constexpr int AGAIN_AFTER = 3;           // a Wrong card comes back after this many others...
constexpr int AGAIN_MAX = 1;             // ...at most this many times a round
constexpr int AGAIN_ROOM = 5;            // and a round grows by at most this many put-back cards
constexpr uint8_t MASTER_BOX = 3;        // a fact is known from here: Easy on two separate days (new -> 2 -> 3)
constexpr uint8_t KNOWN_BOX = 5;         // the tables below where a kid starts: known, reviewed now and then
constexpr uint8_t MAX_BOX = 7;
constexpr uint8_t INTERVAL[MAX_BOX + 1] = {0, 1, 2, 4, 7, 14, 30, 60};   // days until a card in box b is due again
constexpr int MAX_QUEUE = ROUND + AGAIN_ROOM;
constexpr uint32_t ROUND_STARS = 2;   // stars (os/stars.h) for every round finished, however it went: trying is the work

// The table a kid starts on, from the age on their profile: 6 starts on 2x, 8 on 4x (lower tables count as known).
inline int startTable(int age) { const int t = age - 4; return t < 1 ? 1 : t > 4 ? 4 : t; }
inline int tableOf(int card) { return card / FACTS + 1; }
inline int factorOf(int card) { return card % FACTS + 1; }
inline int cardOf(int table, int n) { return (table - 1) * FACTS + n - 1; }
inline int answerOf(int card) { return tableOf(card) * factorOf(card); }
inline int32_t dayOf(uint32_t now) { return (int32_t)(now / 86400u); }

struct Save {
  uint32_t magic;
  uint16_t version, size;
  uint8_t level;            // the newest table (1..TABLES); 0: a fresh deck, set up from the kid's age on the first visit
  uint8_t start;            // the table the kid started on
  uint8_t reserved[2];      // zero; explicit so the layout has no hidden padding
  uint32_t answers;         // cards answered, ever
  uint8_t box[CARDS];       // by card: 0 never shown, 1..MAX_BOX
  uint16_t due[CARDS];      // by card: the day (dayOf) it is due again; meaningless while box is 0
  uint16_t seen[CARDS];     // by card: the day it was last answered; 0 never (or set up as known)
  uint32_t crc;
};
static_assert(sizeof(Save) == 520, "Save is persisted: append before crc, never resize");

inline void seal(Save& s) {
  s.magic = SAVE_MAGIC; s.version = SAVE_VERSION; s.size = sizeof(Save); memset(s.reserved, 0, sizeof s.reserved);
  s.crc = os::crc32(&s, sizeof s - sizeof s.crc);
}
// A blob is valid at exactly this version's size; anything else (damaged, a newer firmware's) starts a fresh deck.
inline bool loadBlob(const void* data, size_t n, Save& out) {
  if (n != sizeof(Save)) return false;
  Save s;
  memcpy(&s, data, n);
  if (s.magic != SAVE_MAGIC || s.version != SAVE_VERSION || s.size != n || s.level > TABLES ||
      s.crc != os::crc32(&s, sizeof s - sizeof s.crc)) return false;
  for (uint8_t& b : s.box) if (b > MAX_BOX) b = MAX_BOX;
  out = s;
  return true;
}

// A fresh deck for a kid starting on `table`: its facts and the higher tables' are new; every lower table is known,
// spread over the next two weeks so a few of them come back each day.
inline void setUp(Save& s, int table, int32_t today) {
  s = Save{};
  s.level = s.start = (uint8_t)table;
  for (int c = 0; c < cardOf(table, 1); c++) {
    s.box[c] = KNOWN_BOX;
    s.due[c] = (uint16_t)(today + 1 + c % INTERVAL[KNOWN_BOX]);
  }
}
// A clock set back (or a save from the future) leaves due days far ahead: none waits longer than its box says.
inline void clampDue(Save& s, int32_t today) {
  for (int c = 0; c < CARDS; c++)
    if (s.box[c] && (int32_t)s.due[c] > today + INTERVAL[s.box[c]]) s.due[c] = (uint16_t)(today + INTERVAL[s.box[c]]);
}
inline bool isDue(const Save& s, int card, int32_t today) { return s.box[card] && (int32_t)s.due[card] <= today; }
inline int known(const Save& s, int table) {   // facts of `table` in MASTER_BOX or higher
  int k = 0;
  for (int n = 1; n <= FACTS; n++) k += s.box[cardOf(table, n)] >= MASTER_BOX;
  return k;
}
inline bool allKnown(const Save& s) { return s.level == TABLES && known(s, TABLES) == FACTS; }

enum Grade : uint8_t { EASY, HARD, WRONG };
// Grades one card. Returns true when this answer finished the level's table and opened the next one.
inline bool grade(Save& s, int card, Grade g, int32_t today) {
  // A real recall, which may move it up: new, due, or still being learned and last seen on another day.
  const bool counts = !s.box[card] || isDue(s, card, today) || (s.box[card] < MASTER_BOX && s.seen[card] != (uint16_t)today);
  uint8_t& b = s.box[card];
  if (g == WRONG) b = 1;
  else if (g == EASY && counts) b = (uint8_t)(b < 1 ? 2 : b < MAX_BOX ? b + 1 : MAX_BOX);   // a new card known at once: box 2
  else if (!b) b = 1;
  if (counts || g == WRONG) s.due[card] = (uint16_t)(today + (g == EASY ? INTERVAL[b] : 1));
  else if (g == HARD && (int32_t)s.due[card] > today + 1) s.due[card] = (uint16_t)(today + 1);   // practice: sooner, never later
  s.seen[card] = (uint16_t)today;
  s.answers++;
  if (s.level < TABLES && known(s, s.level) == FACTS) { s.level++; return true; }
  return false;
}

// One round's cards, in the order shown. A Wrong card is put back AGAIN_AFTER cards on (or last).
struct Round {
  uint8_t card[MAX_QUEUE];
  uint8_t again[CARDS];   // times each card was put back this round
  bool redo[MAX_QUEUE];   // by place: a Wrong card put back, shown as "One more try", not counted in "k of base"
  int n = 0, at = 0, base = 0;   // base: the round's cards before any was put back
  int easy = 0, hard = 0, wrong = 0;
  bool leveledUp = false;
  bool done() const { return at >= n; }
  int current() const { return card[at]; }
  int number() const { int k = 0; for (int i = 0; i <= at && i < n; i++) k += !redo[i]; return k; }   // k of base
};

inline uint32_t nextRand(uint32_t& r) { r ^= r << 13; r ^= r >> 17; r ^= r << 5; return r; }

// Which cards come first: the due ones, oldest due first (a lower box breaks a tie), up to the round less the room kept
// for new cards; then up to NEW_PER_ROUND of the level's new facts, in order; then practice: facts still being learned
// and not yet seen today (a missed one gets its second look in its own round), least-known first. A known
// fact comes only when due, so a round can be short. Only when nothing at all qualifies is the round practice from every
// unlocked table, the least-known and longest-waiting first. Shuffled, so the order never gives the answer away.
inline void build(Round& r, const Save& s, int32_t today, uint32_t seed) {
  r = Round{};
  bool used[CARDS] = {};
  int fresh[NEW_PER_ROUND], nf = 0;
  for (int t = 1; t <= s.level && nf < NEW_PER_ROUND; t++)
    for (int n = 1; n <= FACTS && nf < NEW_PER_ROUND; n++)
      if (!s.box[cardOf(t, n)]) fresh[nf++] = cardOf(t, n);
  auto pick = [&](auto better, int upTo) {   // the best unused card by `better` (better(c, -1): c qualifies), while r.n < upTo
    while (r.n < upTo) {
      int best = -1;
      for (int c = 0; c < s.level * FACTS; c++)   // the unlocked tables' cards
        if (!used[c] && s.box[c] && better(c, best)) best = c;
      if (best < 0) return;
      used[best] = true; r.card[r.n++] = (uint8_t)best;
    }
  };
  pick([&](int a, int b) { return isDue(s, a, today) && (b < 0 || s.due[a] < s.due[b] || (s.due[a] == s.due[b] && s.box[a] < s.box[b])); },
       ROUND - nf);
  for (int i = 0; i < nf; i++) { used[fresh[i]] = true; r.card[r.n++] = (uint8_t)fresh[i]; }
  auto least = [&](int a, int b) { return b < 0 || s.box[a] < s.box[b] || (s.box[a] == s.box[b] && s.due[a] < s.due[b]); };
  pick([&](int a, int b) { return s.box[a] < MASTER_BOX && s.seen[a] != (uint16_t)today && least(a, b); }, ROUND);
  if (!r.n) pick(least, ROUND);
  r.base = r.n;
  uint32_t x = seed | 1;
  for (int i = r.n - 1; i > 0; i--) { const int j = (int)(nextRand(x) % (uint32_t)(i + 1)); const uint8_t t = r.card[i]; r.card[i] = r.card[j]; r.card[j] = t; }
}

// The kid's answer to the round's current card: graded, counted, put back if Wrong, and the round moves on.
inline void answer(Round& r, Save& s, Grade g, int32_t today) {
  const int c = r.current();
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
