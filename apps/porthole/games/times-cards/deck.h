// Times Cards' rules: the times tables as flash cards, scheduled by spaced repetition (Leitner boxes), one deck per kid.
// Header-only and free of drawing, so host/test_times_cards.cpp checks it alone. The design, with its sources, is
// docs/design/adaptive-practice.md (Times Cards: chosen answers, timed grades).
//
// A card is one fact, table x n (n 1..10), tables 1..10. The kid picks its answer from four (choices), and the game
// grades the pick itself from two signals: right or wrong, and how long the card was up (gradeOf). Right and faster
// than the kid's own usual time for a fact that size is Easy, right and slower is Hard, a wrong pick is Wrong.
//
// The usual time (fastMs) is the kid's, not a fixed one: a running mean and spread of log(ms / size) over their right
// answers on known facts, the newest weighing most (an exponentially weighted average, about the last ten answers).
// A fact's size scales it (x1, x2, x5, x10 and squares are quick; 6..9 x 6..9 and divisions take longer), and Easy
// means faster than mean + one spread, about the kid's quickest five in six, kept within 3..8 s. Kids grow, so the
// time follows them; one slow, distracted answer moves it little (each answer is clipped to 2.5 spreads).
//
// A card's box says how well the kid knows it: 0 never shown, 1 learning, and each Easy on a half-day the card was due
// moves it one box up, to wait longer (INTERVAL) before it comes back. Time runs in half-days (midnight to noon, noon
// to midnight), as a kid plays once or twice a day: box 1 waits a half, box 2 a day, then 4, 7, 14, 30 and 60 days. A
// card still being learned also moves up on Easy any time it is met again after enough of a gap: box 1 in a later
// half, box 2 on a later day (after a night's sleep). So a fact is known after Easy on two separate days. Hard keeps
// its box and brings it back the next half; Wrong puts it back in box 1. Both come back later in the same round too.
// A known card answered before it was due never moves up: only a remembered card earns a longer wait.
//
// The step (a staircase, -2..+2) sets what the round deals, card by card: two fast right answers in a row move it up,
// a wrong one moves it down, a slow right one starts the count again. That aims at about 85% right. -2 deals known
// facts (easy wins), -1 due and still-learning facts only, 0 a new fact about one card in three, +1 every other card,
// and +2 probes the next unseen table: its hardest facts (x6, x7, x8) first, and three fast right answers on them in
// a row mark the whole table known (a jump; due again over the next few days, so a lucky guess is soon found). The step
// carries to the kid's next session, at most +1. Due cards take at most half a round, so a long break never makes a
// round all review. At most WORKING_SET facts are being learned at once.
//
// The deck's level is the first table from the last level up not fully known: it is what the home page shows, and
// reaching a new one is the "Next up" moment. It only moves up: a known fact missed again is learned again, the level stays.
//
// Division (3.OA.7): once the kid is on DIV_FROM_LEVEL, each known multiplication fact opens its division partners
// (6 x 7 known: 42 / 6 and 42 / 7) as new cards on the same boxes, schedule and pace. Divisions take about a third of
// the working set's places (a fact family's two together), so the tables keep moving while the divisions of the
// facts already known come in. Division by 1 is left out: it says nothing a kid doesn't know.
//
// Time is Porthole's local wall-clock seconds, so a half-day is now / 43200. Nothing punishes being away: a card that
// waited longer than planned is simply due, oldest first.
//
// Save: one blob per kid (timescards/s<id>, written by the shell). The Save struct below is how the game holds a deck
// and was what versions 1 to 3 stored, whole (1020 bytes by version 3). Version 4 stores the same deck bit-packed
// (pack, a few hundred bytes): the board's NVS holds 20 KB for the whole household, and four kids' decks did not fit.
// Version 6 counts in half-days and keeps the timing and the step where version 3's grading check was; version 7 (this
// one) adds the best fast-round time; version 5 is the same struct whole, if a deck ever cannot be packed. Every
// older version still loads: its days become half-days. The struct stays the same size: a new field takes room from spare (its zero must be a sensible
// default), joins packDeck, and SAVE_VERSION goes up; nothing is reordered or resized.
#pragma once
#include <math.h>
#include <stddef.h>
#include <stdint.h>
#include <string.h>
#include "bits.h"
#include "crc32.h"

namespace cards {
constexpr const char* STORE = "timescards";   // NVS namespace: never renamed once shipped
constexpr uint32_t SAVE_MAGIC = 0x53445243;   // "CRDS"
// 1: multiplication only (520 bytes, whole); 2: division appended (972); 3: grading check (1020); 4: version 3
// bit-packed. All four count days.
constexpr uint16_t SAVE_V3 = 3, SAVE_V4 = 4;
constexpr uint16_t SAVE_WHOLE = 5;     // the struct whole, in half-days: only if a deck cannot be packed
constexpr uint16_t SAVE_V6 = 6;        // half-days and timed answers, bit-packed
constexpr uint16_t SAVE_VERSION = 7;   // what is stored now: version 6 and the best fast-round time

// ---- tuning knobs
constexpr int TABLES = 10, FACTS = 10, CARDS = TABLES * FACTS;   // 1x1 .. 10x10: cards 0..CARDS-1
constexpr int DIVS = (TABLES - 1) * FACTS;   // n / d for divisors 2..10 and quotients 1..10: cards CARDS..ALL-1
constexpr int ALL = CARDS + DIVS;
constexpr int DIV_FROM_LEVEL = 4;        // division joins once 1x to 3x are known (a third-grade standard)
constexpr int ROUND = 10;                // cards in a round (before missed and slow ones come back)
constexpr int NEW_PER_ROUND = 4;         // at most this many never-seen cards in one round (a jump's probe aside)
constexpr int WORKING_SET = 6;           // at most this many facts being learned (shown, below MASTER_BOX) at once
constexpr int DUE_MOST = ROUND / 2;      // due cards take at most this many of a round's places
constexpr int AGAIN_AFTER = 3;           // a Wrong card comes back after this many others...
constexpr int SLOW_AFTER = 5;            // ...a Hard one (right, but slow) after this many...
constexpr int AGAIN_MAX = 1;             // ...at most this many times a round
constexpr int AGAIN_ROOM = 5;            // and a round grows by at most this many put-back cards
constexpr int OPTIONS = 4;               // answers offered for each card
constexpr int STEP_MIN = -2, STEP_MAX = 2, STEP_CARRY = 1;   // the staircase; a session starts at most at STEP_CARRY
constexpr int STEP_UP = 2;               // fast right answers in a row that move the step up (2 up, 1 down: ~85% right)
constexpr int PROBE_FROM = 6, PROBE_TO = 8;   // a jump probes a table's x6, x7, x8
constexpr uint8_t MASTER_BOX = 3;        // a fact is known from here: Easy on two separate days (new -> 2 -> 3)
constexpr uint8_t KNOWN_BOX = 5;         // the tables below where a kid starts: known, reviewed now and then
constexpr uint8_t MAX_BOX = 7;
constexpr uint8_t INTERVAL[MAX_BOX + 1] = {0, 1, 2, 8, 14, 28, 60, 120};   // half-days until a card in box b is due again
constexpr uint8_t INTERVAL_DAYS[MAX_BOX + 1] = {0, 1, 2, 4, 7, 14, 30, 60};   // the same, in days (versions 1 to 4)
constexpr int MAX_QUEUE = ROUND + AGAIN_ROOM;
constexpr uint32_t TIME_CAP_MS = 10000;  // a card up longer than this counts as this long
constexpr uint32_t FAST_MIN_MS = 3000, FAST_MAX_MS = 8000;   // Easy's line is never stricter or looser than these
constexpr float TIME_WEIGHT = 0.1f;      // each right answer moves the usual time 1/10 of the way: about the last ten
constexpr float TIME_SPREAD = 0.35f;     // the spread of log time assumed before the kid's own is known
constexpr float TIME_CLIP = 2.5f;        // an answer counts at most this many spreads from the usual time
constexpr uint16_t RECENT = 200;         // the right-answer share counts about the last 100 to 200 answers
constexpr uint32_t READ_MS = 700;      // no one reads a card and its four answers faster: a right pick sooner is a guess
constexpr int FAST_ROUND = 8;            // cards in a fast round, always this many, so its times compare
constexpr int FAST_FROM = 3;             // slow known facts of 6x, 8x and 9x it takes for the deck to offer one

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
inline int32_t halfOf(uint32_t now) { return (int32_t)(now / 43200u); }   // the half-day: two a day, from midnight and noon
inline bool isProbe(int n) { return n >= PROBE_FROM && n <= PROBE_TO; }

struct Save {
  uint32_t magic;
  uint16_t version, size;
  uint8_t level;            // the first table not fully known, from the last level up (1..TABLES); 0: a fresh deck, set up from the kid's age
  uint8_t start;            // the table the kid started on
  uint16_t pace;            // up to version 4: the self-graded pace; 0 since
  uint32_t answers;         // cards answered, ever
  uint8_t box[CARDS];       // by card: 0 never shown, 1..MAX_BOX
  uint16_t due[CARDS];      // by card: the half-day (halfOf) it is due again; meaningless while box is 0
  uint16_t seen[CARDS];     // by card: the half-day it was last answered; 0 never (or set up as known)
  uint8_t divBox[DIVS];     // v2: the same three for the division cards (card - CARDS)
  uint16_t divDue[DIVS];
  uint16_t divSeen[DIVS];
  uint16_t spare;           // 0; keeps the fields after it aligned
  // v6, where version 3's grading check was (zeroed when an older deck loads): the kid's usual time and the step.
  int16_t logMu;            // ln(ms / size) of right answers on known facts, a running mean, in 1/1024
  uint16_t logVar;          // its running variance, in 1/4096; 0: none yet (the start table's guess)
  int8_t step;              // the staircase, STEP_MIN..STEP_MAX
  uint8_t streak;           // fast right answers in a row since the step last moved
  uint16_t jumps;           // tables marked known by a jump
  uint16_t tries, rights;   // recent answers on cards already met, and the right ones (both halved at RECENT)
  uint16_t probeMiss;       // a bit a table (1 << t): one of its probes was missed or slow, so it is never jumped
  uint16_t bestFast;        // v7: the best fast round, in tenths of a second; 0: none yet
  uint8_t spare6[32];       // 0
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
static_assert(offsetof(Save, logMu) == SAVE_V2_SIZE - 4, "version 2's fields are kept where they were");
static_assert(sizeof(Save) == 1020, "Save is persisted: take new fields from spare6, never resize");

// Seals the struct as version 5: what pack() stores packed, and what is stored whole if a deck ever cannot be packed.
inline void seal(Save& s) {
  s.magic = SAVE_MAGIC; s.version = SAVE_WHOLE; s.size = sizeof(Save);
  s.crc = os::crc32(&s, sizeof s - sizeof s.crc);
}
// A deck from versions 1 to 4, which counted days: its days become half-days (a day's first), and what version 3's
// grading check and version 4's pace kept is dropped.
inline void toHalves(Save& s) {
  for (int c = 0; c < ALL; c++) { s.dueOf(c) = (uint16_t)(s.dueOf(c) * 2); s.seenOf(c) = (uint16_t)(s.seenOf(c) * 2); }
  s.pace = 0;
  memset(&s.logMu, 0, offsetof(Save, crc) - offsetof(Save, logMu));
}

// ---- versions 4, 6 and 7: the deck, bit-packed. Stored: magic, version, total bytes (the struct's first eight bytes, so
// the version reads the same way in every version), the fields below, crc of all before it. One description writes
// and reads (os::Bits::io), so the two cannot drift apart; writing leaves the deck as it was.
// A day, or 0: one bit, then its distance from the deck's earliest day in `w` bits.
inline uint16_t packDay(os::Bits& b, uint16_t day, uint16_t base, int w) {
  return b.io(day != 0, 1) ? (uint16_t)(base + b.io((uint16_t)(day - base), w)) : 0;
}
// One card: its box; nothing more if it was never shown; the day it was last seen; and how its due day follows from
// those (the box's wait, the next day, another day within 64) or, failing all three, the due day itself.
struct Days { uint16_t base; int w; const uint8_t* waits; };   // the deck's earliest day, bits a day, a box's wait
inline void packCard(os::Bits& b, Save& s, int c, const Days& d) {
  const uint8_t box = s.boxOf(c) = (uint8_t)b.io(s.boxOf(c), 3);
  if (!box && !b.io(s.dueOf(c) || s.seenOf(c), 1)) return;
  const uint16_t seen = s.seenOf(c) = packDay(b, s.seenOf(c), d.base, d.w);
  const uint16_t from = seen ? seen : d.base, gap = (uint16_t)(s.dueOf(c) - from), wait = d.waits[box & MAX_BOX];
  const uint32_t how = b.io(gap == wait ? 0 : gap == 1 ? 1 : gap < 64 ? 2 : 3, 2);
  s.dueOf(c) = how == 3 ? packDay(b, s.dueOf(c), d.base, d.w) : (uint16_t)(from + (how == 0 ? wait : how == 1 ? 1 : b.io(gap, 6)));
}
inline void skipCheck(os::Bits& b) {   // version 3's grading check, as version 4 stored it: read and dropped
  b.io(0, 32);   // think time
  for (int i = 0; i < 9; i++) b.io(0, 16);   // its first day, eight counters
  for (int i = 0; i < (ALL + 7) / 8; i++) b.io(0, 8);   // an Easy bit a card
  b.sparse(0, 16);   // spare3
}
inline void packTiming(os::Bits& b, Save& s) {   // version 6: the usual time, the step and its counters
  if (s.step < STEP_MIN || s.step > STEP_MAX) b.bad = true;
  s.logMu = (int16_t)b.io((uint16_t)s.logMu, 16); s.logVar = (uint16_t)b.io(s.logVar, 16);
  s.step = (int8_t)((int)b.io((uint32_t)(s.step - STEP_MIN), 3) + STEP_MIN);
  s.streak = (uint8_t)b.io(s.streak, 8); s.jumps = (uint16_t)b.io(s.jumps, 16);
  s.tries = (uint16_t)b.io(s.tries, 16); s.rights = (uint16_t)b.io(s.rights, 16);
  s.probeMiss = (uint16_t)b.sparse(s.probeMiss, 16);
}
inline void packDeck(os::Bits& b, Save& s, uint16_t version = SAVE_VERSION) {
  const bool v4 = version == SAVE_V4;   // only ever read now
  uint16_t first = 0xFFFF, last = 0;   // the deck's earliest and latest day; read back from the blob when unpacking
  auto span = [&](uint16_t day) { if (day) { first = day < first ? day : first; last = day > last ? day : last; } };
  for (int c = 0; c < ALL; c++) { span(s.dueOf(c)); span(s.seenOf(c)); }
  const uint16_t base = (uint16_t)b.io(last ? first : 0, 16);
  const int w = (int)b.io(last ? os::bitsFor((uint32_t)(last - first)) : 0, 5);
  if (w > 16) { b.bad = true; return; }
  s.level = (uint8_t)b.io(s.level, 4); s.start = (uint8_t)b.io(s.start, 8);
  if (v4) s.pace = (uint16_t)b.io(s.pace, 16);
  s.answers = b.io(s.answers, 32);
  const Days d = {base, w, v4 ? INTERVAL_DAYS : INTERVAL};
  for (int c = 0; c < ALL; c++) packCard(b, s, c, d);
  s.spare = (uint16_t)b.sparse(s.spare, 16);
  if (v4) skipCheck(b); else packTiming(b, s);
  if (version >= 7) s.bestFast = (uint16_t)b.sparse(s.bestFast, 16);   // version 6: none yet
}
// The deck as a packed blob in out (at most cap bytes). 0 if it cannot be packed (a box past MAX_BOX, a step out of
// range, or days so scattered it would not fit): the caller then stores the struct whole, as version 5.
// `version` is SAVE_VERSION but in the tests, which make an older one to load.
inline size_t pack(Save& s, uint8_t* out, size_t cap, uint16_t version = SAVE_VERSION) {
  if (cap < 12) return 0;
  memset(out, 0, cap);
  os::Bits b = os::Bits::writer(out, cap - 4);
  b.io(SAVE_MAGIC, 32); b.io(version, 16); b.io(0, 16);
  packDeck(b, s, version);
  for (uint8_t x : s.spare6) b.bad |= x != 0;   // not stored: a deck with any of it set is stored whole
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
  const bool magic = b.io(0, 32) == SAVE_MAGIC;
  const uint16_t version = (uint16_t)b.io(0, 16);
  if (!magic || (version != SAVE_V4 && version != SAVE_V6 && version != SAVE_VERSION) || b.io(0, 16) != n) return false;
  if (crc != os::crc32(data, n - sizeof crc)) return false;
  packDeck(b, zeroed, version);
  if (!b.ok() || b.bytes() != n - sizeof crc || zeroed.level > TABLES || zeroed.step > STEP_MAX) return false;
  if (version == SAVE_V4) toHalves(zeroed);
  seal(zeroed);
  return true;
}
// The version a stored blob says it is (in the same place in every version); 0 if it is too short to be a deck.
inline uint16_t storedVersion(const void* data, size_t n) {
  uint16_t version = 0;
  if (data && n >= 12) memcpy(&version, (const uint8_t*)data + offsetof(Save, version), sizeof version);
  return version;
}
// A blob is valid packed (version 4, 6 or 7), or whole at exactly its version's size (1, 2, 3 or 5; the newer fields
// zero: no division met, nothing timed yet); anything else (damaged, a newer firmware's) is not a deck this build reads.
inline uint16_t wholeVersion(size_t n, uint16_t stored) {   // a whole struct's version from its size; 0: none
  const bool whole = stored == SAVE_V3 || stored == SAVE_WHOLE;
  return n == SAVE_V1_SIZE ? 1 : n == SAVE_V2_SIZE ? 2 : n == sizeof(Save) && whole ? stored : 0;
}
inline bool loadWhole(const void* data, size_t n, uint16_t stored, Save& s) {
  const uint16_t version = wholeVersion(n, stored);
  if (!version) return false;
  const size_t body = n - sizeof s.crc;
  uint32_t crc;
  memcpy(&s, data, body);
  memcpy(&crc, (const uint8_t*)data + body, sizeof crc);
  if (s.magic != SAVE_MAGIC || s.version != version || s.size != n || s.level > TABLES ||
      crc != os::crc32(data, body)) return false;
  if (version != SAVE_WHOLE) toHalves(s);
  for (int c = 0; c < ALL; c++) if (s.boxOf(c) > MAX_BOX) s.boxOf(c) = MAX_BOX;
  if (s.step < STEP_MIN || s.step > STEP_MAX) s.step = 0;
  seal(s);   // as this version holds it, the same whichever way it was stored
  return true;
}
inline bool loadBlob(const void* data, size_t n, Save& out) {
  Save s{};
  const uint16_t stored = storedVersion(data, n);
  const bool packed = stored == SAVE_V4 || stored == SAVE_V6 || stored == SAVE_VERSION;
  if (!(packed ? unpack((const uint8_t*)data, n, s) : loadWhole(data, n, stored, s))) return false;
  out = s;
  return true;
}

// A fresh deck for a kid starting on `table`: its facts and the higher tables' are new; every lower table is known,
// spread over the next two weeks so a few of them come back each day.
inline void setUp(Save& s, int table, int32_t now) {
  s = Save{};
  s.level = s.start = (uint8_t)table;
  for (int c = 0; c < cardOf(table, 1); c++) {
    s.boxOf(c) = KNOWN_BOX;
    s.dueOf(c) = (uint16_t)(now + 1 + c % INTERVAL[KNOWN_BOX]);
  }
}
// A clock set back (or a save from the future) leaves due days far ahead: none waits longer than its box says.
inline void clampDue(Save& s, int32_t now) {
  for (int c = 0; c < ALL; c++)
    if (s.boxOf(c) && (int32_t)s.dueOf(c) > now + INTERVAL[s.boxOf(c)]) s.dueOf(c) = (uint16_t)(now + INTERVAL[s.boxOf(c)]);
}
inline bool isDue(const Save& s, int card, int32_t now) { return s.boxOf(card) && (int32_t)s.dueOf(card) <= now; }
inline int known(const Save& s, int table) {   // facts of `table` in MASTER_BOX or higher
  int k = 0;
  for (int n = 1; n <= FACTS; n++) k += s.boxOf(cardOf(table, n)) >= MASTER_BOX;
  return k;
}
inline int levelOf(const Save& s) { int t = 1; while (t < TABLES && known(s, t) == FACTS) t++; return t; }
inline bool allKnown(const Save& s) { return s.level == TABLES && known(s, TABLES) == FACTS; }
enum Grade : uint8_t { EASY, HARD, WRONG };
inline int learningOf(const Save& s, bool division) {   // facts of one kind shown and not yet known
  int k = 0;
  for (int c = division ? CARDS : 0; c < (division ? ALL : CARDS); c++) k += s.boxOf(c) && s.boxOf(c) < MASTER_BOX;
  return k;
}
inline int learning(const Save& s) { return learningOf(s, false) + learningOf(s, true); }

// ---- the kid's usual time, and the grade it gives a right answer
// How long a fact usually takes against the rest: x1, x2, x5, x10 and squares come quick; two of 6..9 (not a square)
// and every division take longer (Zbrodoff & Logan 2005: the problem-size effect).
inline float sizeOf(int card) {
  if (isDiv(card)) return 1.25f;
  const int a = tableOf(card), b = factorOf(card);
  auto easy = [](int x) { return x == 1 || x == 2 || x == 5 || x == 10; };
  if (easy(a) || easy(b) || a == b) return 0.8f;
  return a >= 6 && b >= 6 && a <= 9 && b <= 9 ? 1.25f : 1.0f;
}
// Before the kid's own answers say otherwise: a younger starter (by start table) is given longer.
inline float startLog(const Save& s) { return logf(s.start <= 2 ? 5000.f : s.start == 3 ? 4500.f : 4000.f); }
inline float logMean(const Save& s) { return s.logVar ? s.logMu / 1024.f : startLog(s); }
inline float logSpread(const Save& s) { return s.logVar ? sqrtf(s.logVar / 4096.f) : TIME_SPREAD; }
inline uint32_t usualMs(const Save& s) { return (uint32_t)lroundf(expf(logMean(s))); }   // a mid-size fact's middle time
// Easy's line for this card: the kid's mean plus one spread, for the fact's size, within FAST_MIN_MS..FAST_MAX_MS.
inline uint32_t fastMs(const Save& s, int card) {
  const float ms = expf(logMean(s) + logSpread(s)) * sizeOf(card);
  return ms < FAST_MIN_MS ? FAST_MIN_MS : ms > FAST_MAX_MS ? FAST_MAX_MS : (uint32_t)ms;
}
inline Grade gradeOf(const Save& s, int card, bool right, uint32_t ms) {
  return !right ? WRONG : ms <= fastMs(s, card) ? EASY : HARD;
}
// A right answer on a known fact moves the usual time and its spread (an exponentially weighted mean and variance of
// log time, clipped so one long look away moves it little).
inline void noteTime(Save& s, int card, uint32_t ms) {
  const float mu = logMean(s), sd = logSpread(s), var = sd * sd;
  float x = logf((float)(ms < 100 ? 100 : ms > TIME_CAP_MS ? TIME_CAP_MS : ms) / sizeOf(card));
  x = x < mu - TIME_CLIP * sd ? mu - TIME_CLIP * sd : x > mu + TIME_CLIP * sd ? mu + TIME_CLIP * sd : x;
  const float d = x - mu, v = (1 - TIME_WEIGHT) * (var + TIME_WEIGHT * d * d);
  s.logMu = (int16_t)lroundf((mu + TIME_WEIGHT * d) * 1024);
  s.logVar = (uint16_t)lroundf((v < 0.01f ? 0.01f : v > 4 ? 4 : v) * 4096);   // a spread of 0.1..2; never 0 (none yet)
}

// ---- the step
inline void noteStep(Save& s, Grade g) {
  if (g == HARD) { s.streak = 0; return; }
  if (g == WRONG) { s.streak = 0; if (s.step > STEP_MIN) s.step--; return; }
  if (++s.streak < STEP_UP) return;
  s.streak = 0;
  if (s.step < STEP_MAX) s.step++;
}
inline void startSession(Save& s) { if (s.step > STEP_CARRY) s.step = STEP_CARRY; s.streak = 0; }
inline void noteRight(Save& s, Grade g) {
  s.tries++; s.rights += g != WRONG;
  if (s.tries >= RECENT) { s.tries /= 2; s.rights /= 2; }
}

// The four answers a card offers in out: the right one and three near misses (the neighbouring facts: 7 x 8 offers
// three of 49, 63, 48 and 64 beside 56; a division its quotient's neighbours), topped up with numbers just above or
// below, in an order drawn from seed. Returns where the right one is.
inline uint32_t nextRand(uint32_t& r) { r ^= r << 13; r ^= r >> 17; r ^= r << 5; return r; }
inline int choices(int card, uint32_t seed, int* out) {
  const int a = answerOf(card), t = tableOf(card), f = factorOf(card);
  int near[10] = {t * (f - 1), t * (f + 1), (t - 1) * f, (t + 1) * f, a + 1, a - 1, a + 2, a - 2, a + 3, a + 4};
  if (isDiv(card)) { const int d[4] = {a - 1, a + 1, a - 2, a + 2}; memcpy(near, d, sizeof d); }
  uint32_t r = (seed ^ 0x9E3779B9u) * 2654435761u | 1;   // close seeds, far apart
  nextRand(r); nextRand(r);
  for (int i = 3; i > 0; i--) { const int j = (int)(nextRand(r) % (uint32_t)(i + 1)), x = near[i]; near[i] = near[j]; near[j] = x; }
  int n = 0;
  out[n++] = a;
  for (int i = 0; i < 10 && n < OPTIONS; i++) {
    bool fresh = near[i] > 0;
    for (int k = 0; k < n; k++) fresh &= out[k] != near[i];
    if (fresh) out[n++] = near[i];
  }
  int right = 0;
  for (int i = OPTIONS - 1; i > 0; i--) {
    const int j = (int)(nextRand(r) % (uint32_t)(i + 1)), x = out[i];
    out[i] = out[j]; out[j] = x;
    right = right == i ? j : right == j ? i : right;
  }
  return right;
}

// Grades one card: its box and due day. Returns true when this answer reached a new level (a first not fully known
// table past the one before).
// A real recall, which may move a card up: new, due, or still being learned and met after enough of a gap (box 1: a
// later half-day; box 2: a later day).
inline bool recalls(const Save& s, int card, int32_t now) {
  const int b = s.boxOf(card);
  if (!b || isDue(s, card, now)) return true;
  return b < MASTER_BOX && (b == 1 ? s.seenOf(card) != (uint16_t)now : s.seenOf(card) / 2 != now / 2);
}
inline uint8_t nextBox(uint8_t b, Grade g, bool counts) {
  if (g == WRONG) return 1;
  if (g == EASY && counts) return (uint8_t)(b < 1 ? 2 : b < MAX_BOX ? b + 1 : MAX_BOX);   // a new card known at once: box 2
  return b ? b : 1;
}
// The level only moves up: a missed known fact is learned again from box 1, but one slip does not send the deck back a
// table (it would show 1x to a kid on 8x for two days).
inline bool newLevel(Save& s) {
  const int was = s.level;
  while (s.level < TABLES && known(s, s.level) == FACTS) s.level++;
  return s.level > was;
}
inline bool grade(Save& s, int card, Grade g, int32_t now) {
  const bool counts = recalls(s, card, now);
  s.boxOf(card) = nextBox(s.boxOf(card), g, counts);
  if (counts || g == WRONG) s.dueOf(card) = (uint16_t)(now + (g == EASY ? INTERVAL[s.boxOf(card)] : 1));
  else if (g == HARD && (int32_t)s.dueOf(card) > now + 1) s.dueOf(card) = (uint16_t)(now + 1);   // practice: sooner, never later
  s.seenOf(card) = (uint16_t)now;
  s.answers++;
  return newLevel(s);
}

// ---- the jump
// The next probe card: in the first table from the level up with no fact met but its probe ones, the next of x6, x7,
// x8 not yet met, while those met were all answered fast and right the first time (box 2). -1 when one was missed or
// slow (no jump past that table until it is learned), or when every table is under way.
inline int probeCard(const Save& s) {
  for (int t = s.level; t <= TABLES; t++) {
    bool untouched = true;
    for (int n = 1; n <= FACTS; n++) untouched &= isProbe(n) || !s.boxOf(cardOf(t, n));
    if (!untouched) continue;
    if (s.probeMiss >> t & 1) return -1;   // a probe missed or slow: learned the usual way, even once it is right
    for (int n = PROBE_FROM; n <= PROBE_TO; n++) {
      const uint8_t b = s.boxOf(cardOf(t, n));
      if (!b) return cardOf(t, n);
      if (b != 2) return -1;
    }
    return -1;
  }
  return -1;
}
// After a probe card's answer: if its table's three probe facts were all fast and right, the whole table is known,
// due again over the next one to four days (so a lucky streak is soon found out). Returns true when it jumped.
inline bool jump(Save& s, int card, int32_t now) {
  if (isDiv(card)) return false;
  const int t = tableOf(card);
  if (s.probeMiss >> t & 1) return false;
  for (int n = 1; n <= FACTS; n++) if (s.boxOf(cardOf(t, n)) != (isProbe(n) ? 2 : 0)) return false;
  for (int n = 1; n <= FACTS; n++) {
    const int c = cardOf(t, n);
    s.boxOf(c) = MASTER_BOX; s.seenOf(c) = (uint16_t)now; s.dueOf(c) = (uint16_t)(now + 2 * (1 + n % 4));
  }
  s.jumps++;
  return true;
}

// ---- a round, dealt a card at a time
struct Round {
  uint8_t card[MAX_QUEUE] = {};   // the cards dealt so far, in order; card[at] is the one up now
  bool redo[MAX_QUEUE] = {};    // by place: a missed or slow card come back, not counted in "k of base"
  uint8_t again[ALL] = {};      // times each card was put back this round
  bool used[ALL] = {};          // dealt this round
  uint8_t back[AGAIN_ROOM] = {}, backAt[AGAIN_ROOM] = {};   // cards put back, and the place each comes back at
  int backs = 0, puts = 0;
  int n = 0, at = 0, base = ROUND;   // base: the round's cards, not counting those come back
  int dealt = 0, dues = 0, news = 0;   // of base: dealt so far, and how many of them were due or new
  int easy = 0, hard = 0, wrong = 0;
  bool leveledUp = false, jumped = false;
  bool fast = false;           // a fast round (buildFast): its cards fixed, none put back, its time kept
  bool newBest = false;        // a fast round that beat the kid's best time (or set the first)
  uint16_t was = 0;            // the best before it, in tenths of a second (0: none)
  uint32_t took = 0;           // the round's time: each card's, from showing to the pick
  bool done() const { return at >= n; }
  int current() const { return card[at]; }
  int number() const { int k = 0; for (int i = 0; i <= at && i < n; i++) k += !redo[i]; return k; }   // k of base
};

// The best card not yet dealt this round by `better` (better(c, -1): c qualifies), or -1.
template <class Better>
inline int best(const Round& r, Better better) {
  int b = -1;
  for (int c = 0; c < ALL; c++) if (!r.used[c] && better(c, b)) b = c;
  return b;
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
// A never-shown fact, if the round and the working set have room: a division whenever the divisions being learned are
// fewer than half the multiplications (about a third of the set), else the next multiplication; either, when the
// other kind has none left. A half share slowed the tables by half in simulation; a third keeps most of their pace.
inline int newCard(const Round& r, const Save& s) {
  if (r.news >= NEW_PER_ROUND || learning(s) >= WORKING_SET) return -1;
  int c = learningOf(s, true) * 2 < learningOf(s, false) ? nextDivision(s, r.used) : -1;
  if (c < 0) c = nextTimes(s, r.used);
  return c < 0 ? nextDivision(s, r.used) : c;
}
inline int dueCard(const Round& r, const Save& s, int32_t now) {   // oldest due first, a lower box breaking a tie
  return best(r, [&](int a, int b) {
    return isDue(s, a, now) && (b < 0 || s.dueOf(a) < s.dueOf(b) || (s.dueOf(a) == s.dueOf(b) && s.boxOf(a) < s.boxOf(b)));
  });
}
inline int learningCard(const Round& r, const Save& s, int32_t now) {   // least known first, then the longest unseen
  auto key = [&](int c) { return s.boxOf(c) * 2 + (s.seenOf(c) == (uint16_t)now); };
  return best(r, [&](int a, int b) {
    return s.boxOf(a) && s.boxOf(a) < MASTER_BOX && (b < 0 || key(a) < key(b) || (key(a) == key(b) && s.dueOf(a) < s.dueOf(b)));
  });
}
inline int easyWin(const Round& r, const Save& s, int32_t now) {   // a known fact: due ones first, then unseen this half
  auto key = [&](int c) { return isDue(s, c, now) ? 0 : s.seenOf(c) != (uint16_t)now ? 1 : 2; };
  return best(r, [&](int a, int b) {
    return s.boxOf(a) >= MASTER_BOX && (b < 0 || key(a) < key(b) || (key(a) == key(b) && s.dueOf(a) < s.dueOf(b)));
  });
}
// The next card the step deals (see the top of this file). Every kind falls through to the next, so a round only ends
// short when nothing at all is left to deal.
inline bool wantsNew(const Round& r, int step) {   // +2 every card, +1 every other, 0 one in three
  return step >= 2 || (step == 1 && r.dealt % 2 == 1) || (step == 0 && r.dealt % 3 == 2);
}
inline int freshCard(Round& r, const Save& s) {   // at +2 a probe, else a new fact; counted as new
  int c = s.step >= 2 ? probeCard(s) : -1;
  if (c < 0 || r.used[c]) c = newCard(r, s);
  if (c >= 0) r.news++;
  return c;
}
inline int choose(Round& r, const Save& s, int32_t now) {
  int c = s.step <= -2 ? easyWin(r, s, now) : -1;
  if (c < 0 && wantsNew(r, s.step)) c = freshCard(r, s);
  if (c < 0 && r.dues < DUE_MOST && (c = dueCard(r, s, now)) >= 0) r.dues++;
  if (c < 0) c = learningCard(r, s, now);
  if (c < 0) c = easyWin(r, s, now);
  if (c < 0 && (c = newCard(r, s)) >= 0) r.news++;
  return c;
}
// Deals card[at] if it is not dealt yet: a card put back, once its place comes; else the next the step chooses; and
// once the round's own cards are all dealt (or none is left), the cards still put back.
inline void next(Round& r, const Save& s, int32_t now) {
  if (r.at < r.n || r.n >= MAX_QUEUE) return;
  int k = -1;
  for (int i = 0; i < r.backs; i++) if (k < 0 || r.backAt[i] < r.backAt[k]) k = i;
  int c = -1;
  if ((k < 0 || r.backAt[k] > r.at) && r.dealt < r.base) {
    c = choose(r, s, now);
    if (c < 0) r.base = r.dealt;
    else { r.used[c] = true; r.dealt++; r.card[r.n] = (uint8_t)c; r.redo[r.n++] = false; return; }
  }
  if (k < 0) return;
  r.card[r.n] = r.back[k]; r.redo[r.n++] = true;
  r.back[k] = r.back[--r.backs]; r.backAt[k] = r.backAt[r.backs];
}
inline void build(Round& r, const Save& s, int32_t now) {
  r = Round{};
  next(r, s, now);
}

// ---- the fast round: known facts with a 6, 8 or 9 in them (either order) that the kid still works out slowly
inline bool hardTable(int x) { return x == 6 || x == 8 || x == 9; }
inline bool fastPool(const Save& s, int c) {   // a known multiplication fact of 6x, 8x or 9x
  return !isDiv(c) && (hardTable(tableOf(c)) || hardTable(factorOf(c))) && s.boxOf(c) >= MASTER_BOX;
}
// Slow: its last answer was right but Hard, which leaves a known card due the very half after it was seen (grade).
inline bool slowFact(const Save& s, int c) {
  return fastPool(s, c) && s.seenOf(c) && s.dueOf(c) == (uint16_t)(s.seenOf(c) + 1);
}
// The deck page offers a fast round once FAST_FROM such facts are slow and FAST_ROUND of them are known.
inline bool offersFast(const Save& s) {
  int pool = 0, slow = 0;
  for (int c = 0; c < CARDS; c++) { pool += fastPool(s, c); slow += slowFact(s, c); }
  return slow >= FAST_FROM && pool >= FAST_ROUND;
}
// FAST_ROUND cards: the slow facts first, then other known ones, each kind in an order drawn from seed.
inline void buildFast(Round& r, const Save& s, uint32_t seed) {
  r = Round{};
  r.fast = true; r.was = s.bestFast;
  uint32_t key[CARDS], x = seed | 1;
  for (int c = 0; c < CARDS; c++) key[c] = (slowFact(s, c) ? 0 : 1u << 31) | nextRand(x) >> 1;
  while (r.n < FAST_ROUND) {
    const int c = best(r, [&](int a, int b) { return fastPool(s, a) && (b < 0 || key[a] < key[b]); });
    if (c < 0) break;
    r.used[c] = true; r.card[r.n++] = (uint8_t)c;
  }
  r.base = r.dealt = r.n;
}
// A finished fast round's time, against the kid's best: true when it is a new best (or the first).
inline bool noteFast(Save& s, uint32_t ms) {
  const uint32_t t = (ms + 50) / 100;
  const uint16_t tenths = (uint16_t)(t < 1 ? 1 : t > 0xFFFF ? 0xFFFF : t);
  if (s.bestFast && tenths >= s.bestFast) return false;
  s.bestFast = tenths;
  return true;
}
// A missed 1 x n or n x 1: the answer page shows it as groups of dots (n groups of 1, or 1 group of n).
inline bool showsOnes(int card, bool right) { return !right && !isDiv(card) && (tableOf(card) == 1 || factorOf(card) == 1); }

// Counts the grade, and puts a missed or slow card back a few cards on (once, while the round has room).
inline void putBack(Round& r, int c, Grade g) {
  (g == EASY ? r.easy : g == HARD ? r.hard : r.wrong)++;
  if (g == EASY || r.fast || r.again[c] >= AGAIN_MAX || r.puts >= AGAIN_ROOM) return;
  r.again[c]++; r.puts++;
  r.back[r.backs] = (uint8_t)c; r.backAt[r.backs++] = (uint8_t)(r.at + 1 + (g == WRONG ? AGAIN_AFTER : SLOW_AFTER));
}
// The kid's answer to the round's current card, picked `ms` after it showed: graded by the deck (gradeOf), counted,
// put back if missed or slow, and the next card dealt; a fast round's last answer keeps its time. Returns the grade.
// What an answer to card c tells the deck before it is graded: a probe's miss, the kid's step, right share and time.
// Returns whether c was the probe.
inline bool noteAnswer(Save& s, int c, Grade g, bool redo, uint32_t ms) {
  const bool probe = !redo && c == probeCard(s);
  if (probe && g != EASY) s.probeMiss |= (uint16_t)(1u << tableOf(c));
  if (!redo && (s.boxOf(c) || probe)) { noteStep(s, g); noteRight(s, g); }   // a first sight or a second look says little
  if (!redo && g != WRONG && s.boxOf(c) >= MASTER_BOX) noteTime(s, c, ms);
  return probe;
}
inline Grade answer(Round& r, Save& s, bool right, uint32_t ms, int32_t now) {
  const int c = r.current();
  const Grade g = gradeOf(s, c, right, ms);
  const bool probe = noteAnswer(s, c, g, r.redo[r.at], ms);
  if (grade(s, c, g, now)) r.leveledUp = true;
  if (probe && g == EASY && jump(s, c, now)) { r.jumped = true; r.leveledUp |= newLevel(s); }
  putBack(r, c, g);
  r.took += ms;
  r.at++;
  next(r, s, now);
  if (r.fast && r.done()) r.newBest = noteFast(s, r.took);
  return g;
}
}  // namespace cards
