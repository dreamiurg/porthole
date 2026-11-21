// The skills engine: one rating per kid per skill, shared by every game that teaches it, and the stars a round of
// learning pays (os/stars.h holds the wallet). Pure logic, header-only; host/test_skills.cpp checks it.
//
// The design, approved by the parent on 2026-11-21 (the "Porthole Star Economy" page):
// - A skill is what a kid learns, not a game: Times Cards and Pets Club's times stories feed different skills, the
//   same plus-and-minus is one skill wherever it is asked. A rating theta is in levels x 100 (level 3 = 300).
// - An item has a difficulty beta on the same scale. The chance of a right answer is an Elo curve with the guess rate
//   of its answers (expect). A game asks for the level to serve next (target): about 80% right.
// - Every answer moves theta by K (s - p): s 1 right, 0.7 right but slow, 0 wrong or a guess faster than reading.
//   K = max(40, 160 / (1 + n/5)), doubled while three answers in a row miss the expectation the same way.
// - A new skill calibrates: from a starting level it jumps up 1, 2, 4 levels on right answers and halves the jump on a
//   miss, until the jump is one level and two answers were wrong (or 15 answers). It pays a flat rate meanwhile.
// - A right answer pays base x W x repeat points, W = clamp((1 - p) / 0.2, 0.25, 2.5): what the kid would almost surely
//   get right pays a quarter, what is hard for them up to two and a half. p for pay uses max(theta, peak - 150), so
//   missing on purpose does not make easy items dear. The same item again the same day pays half, a quarter, then 0.
//   A wrong answer faster than reading takes half a base off the round; the round never goes below 0, and the wallet
//   never goes down for a wrong answer.
// - A round's points become stars at its end (finish): 100 points a star, the rest carried. The first 6 learning stars
//   a day count twice (a fresh start), past 40 a day points count a quarter (stars are resting), a new best level is
//   +5. A weekly pace factor (tempo, 0.8..1.5) evens out stars per minute of learning between kids.
// - Games that teach nothing pay through earnForPlay: at most PLAY_DAY stars a day across all of them.
//
// Book is persisted inside the profile's record (shell/profiles.h, v4): append-only, never resized.
#pragma once
#include <math.h>
#include <stddef.h>
#include <stdint.h>
#include "stars.h"

namespace skills {
enum Skill : uint8_t { ADD_SUB, TIMES_FACTS, TIMES_STORY, SPELL_EN, READ_EN, WORDS_UK, READ_UK, SKILL_COUNT };
constexpr int MAX_SKILLS = 7;   // stored in the record; a new skill takes the next id (never renumber) and a v5 record
static_assert(SKILL_COUNT <= MAX_SKILLS, "Book stores MAX_SKILLS");
// Points a right answer at the kid's level pays: about the seconds an item takes, so a minute is worth the same in any
// game (a Times Cards card 4-5 s, a puzzle 8-10 s, a passage with its question about 25 s).
constexpr uint16_t BASE[SKILL_COUNT] = {40, 20, 40, 40, 100, 40, 100};
constexpr int LEVEL = 100;                  // rating points per level
constexpr int POINTS_PER_STAR = 100;
constexpr int REST_BONUS_STARS = 6;         // the day's first learning stars count twice
constexpr int SOFT_CAP_STARS = 40;          // past this many learning stars a day, points count a quarter
constexpr int MILESTONE_STARS = 5;          // a new best level in a skill
constexpr int PLAY_DAY = 10;                // stars a day from games that teach nothing, all of them together
constexpr int PEAK_DROP = 150, PEAK_COOL = 10;   // pay never reads theta lower than peak - 150; the peak cools 10 a day
constexpr float TARGET = 0.8f;
constexpr int THETA_MAX = 2047;             // 11 bits: 20 levels
constexpr int CALIB_ANSWERS = 15;           // a calibration ends here at the latest

// A skill as stored: 4 bytes, because a profile's record has to fit the board's NVS with four kids
// (host/test_nvs_budget.cpp). get() and put() unpack it.
//   word: theta (11 bits) | best level + 1 (4 bits, 0: none yet) | spare bit
//   peak: the highest the rating has been (cooling PEAK_COOL a day), in eighths, rounded up
//   meta: 0 never used. Calibrated: 0x80 | trend + 3 (3 bits, answers in a row above + or below - expectation) << 4 |
//         answers counted (4 bits, saturating at 15). Calibrating: an error so far 0x40 | jump code (1: 1 level,
//         2: 2, 3: 4) << 4 | answers (4 bits).
struct Stored { uint16_t word; uint8_t peak, meta; };
static_assert(sizeof(Stored) == 4, "a stored skill is 4 bytes");

struct Book {
  uint16_t day;          // the local day (seconds / 86400) the day's counters are for; its week is day / 7
  uint8_t learnStars;    // learning stars earned today, bonuses aside (saturating)
  uint8_t playStars;     // stars today from games that teach nothing
  uint8_t carry;         // points toward the next star
  uint8_t tempo;         // the pace factor x 100 (0: not set, 100)
  uint16_t weekStars;    // learning stars earned this week
  uint16_t weekSec;      // seconds spent answering this week
  Stored skill[MAX_SKILLS];
  uint16_t spare;
};
static_assert(sizeof(Book) == 40, "Book is persisted in the profile record: append, never resize");

// A skill, unpacked.
struct State {
  int theta = 0, peak = 0, n = 0, trend = 0, best = 0, jump = 1;
  bool started = false, calibrating = false, errored = false;
};
inline State get(const Book& b, Skill k) {
  const Stored& x = b.skill[k];
  State s;
  s.started = x.meta != 0;
  s.theta = x.word & 0x7FF; s.best = x.word >> 11 & 15; s.peak = x.peak * 8 > s.theta ? x.peak * 8 : s.theta;
  s.n = x.meta & 15;
  if (x.meta & 0x80) { s.trend = (x.meta >> 4 & 7) - 3; }
  else if (s.started) { s.calibrating = true; s.errored = x.meta & 0x40; const int c = x.meta >> 4 & 3; s.jump = c == 3 ? 4 : c; }
  return s;
}
inline void put(Book& b, Skill k, const State& s) {
  Stored& x = b.skill[k];
  const int theta = s.theta < 0 ? 0 : s.theta > THETA_MAX ? THETA_MAX : s.theta, peak = (s.peak + 7) / 8;
  x.word = (uint16_t)(theta | (s.best > 15 ? 15 : s.best) << 11);
  x.peak = (uint8_t)(peak < 0 ? 0 : peak > 255 ? 255 : peak);
  const int n = s.n > 15 ? 15 : s.n;
  if (!s.started) x.meta = 0;
  else if (s.calibrating) x.meta = (uint8_t)((s.errored ? 0x40 : 0) | (s.jump >= 4 ? 3 : s.jump) << 4 | n);
  else x.meta = (uint8_t)(0x80 | (s.trend < -3 ? 0 : s.trend > 3 ? 6 : s.trend + 3) << 4 | n);
}
inline bool calibrating(const Book& b, Skill k) { return get(b, k).calibrating; }
inline bool started(const Book& b, Skill k) { return b.skill[k].meta != 0; }

// The chance a kid at theta gets an item of difficulty beta right, with `options` answers to guess from.
inline float expect(int theta, int beta, int options) {
  const float g = options > 1 ? 1.0f / options : 0;
  return g + (1 - g) / (1 + expf(-(float)(theta - beta) / LEVEL));
}
// How far below theta an item is right about TARGET of the time.
inline int easeFor(int options) {
  const float g = options > 1 ? 1.0f / options : 0, c = (TARGET - g) / (1 - g);
  return (int)lroundf(LEVEL * logf(c / (1 - c)));
}

// A new day: the day's counters start over and peaks cool; a new week moves the pace factor toward 1.7 stars a minute
// (only after a week with at least 10 minutes of answers; a quarter of the way, within 0.8..1.5).
inline void today(Book& b, uint32_t nowSec) {
  const uint16_t day = (uint16_t)(nowSec / 86400u);
  if (!b.tempo) b.tempo = 100;
  if (day == b.day) return;
  if (day / 7 != b.day / 7) {
    if (b.weekSec >= 600) {
      const float perMin = b.weekStars * 60.0f / b.weekSec, ratio = perMin > 0.05f ? 1.7f / perMin : 4;
      const float t = b.tempo * powf(ratio, 0.25f);
      b.tempo = (uint8_t)(t < 80 ? 80 : t > 150 ? 150 : lroundf(t));
    }
    b.weekStars = 0; b.weekSec = 0;
  }
  const int gone = day > b.day && b.day ? day - b.day : 1;
  for (int k = 0; k < SKILL_COUNT; k++) {
    State s = get(b, (Skill)k);
    if (!s.started) continue;
    const int cooled = s.peak - PEAK_COOL * (gone > 30 ? 30 : gone);
    s.peak = cooled > s.theta ? cooled : s.theta;
    put(b, (Skill)k, s);
  }
  b.day = day; b.learnStars = 0; b.playStars = 0;
}

// A skill's first level: a game seeds it once, before its first answer. calibrated: the level is known already (a save
// from before this engine, a grown-up's setting), so no calibration; else it calibrates from there.
inline void seed(Book& b, Skill k, int level, bool calibrated) {
  if (started(b, k)) return;
  State s;
  level = level < 0 ? 0 : level;
  s.theta = s.peak = level * LEVEL; s.best = level + 1; s.started = true; s.calibrating = !calibrated;
  put(b, k, s);
}
// The level of the next item, 0..levels-1: while calibrating, the level the jumps reached; after, the level a kid gets
// right about TARGET of the time.
inline int target(const Book& b, Skill k, int levels, int options) {
  const State s = get(b, k);
  const int at = s.calibrating ? s.theta : s.theta - easeFor(options);
  const int level = (at + LEVEL / 2) / LEVEL;
  return level < 0 ? 0 : level >= levels ? levels - 1 : level;
}
inline int level(const Book& b, Skill k) { return get(b, k).theta / LEVEL; }

struct Round {
  int32_t points = 0;     // this round's, before the day's rules
  uint32_t ms = 0;        // time spent answering (each answer counts at most 30 s)
  int answers = 0;
  bool newBest = false;   // a skill reached a level it never had
  // finish() fills these, for the round's end page:
  int stars = 0, bonus = 0, milestone = 0;
  bool resting = false;   // the day's soft cap was reached
};

namespace detail {
// The same item the same day: a few RAM slots, a profile's book and a day telling one kid's from another's. A reboot
// forgets them (accepted: it only makes a repeat pay full once more).
struct Seen { const Book* who; uint16_t day, key; uint8_t skill, times; };
inline Seen seen[48];
inline uint8_t seenNext = 0;
inline int repeats(const Book& b, Skill k, uint16_t key) {   // times seen before today, and counts this one
  for (Seen& x : seen)
    if (x.who == &b && x.day == b.day && x.skill == k && x.key == key) { const int t = x.times; if (x.times < 255) x.times++; return t; }
  seen[seenNext] = {&b, b.day, key, (uint8_t)k, 1};
  seenNext = (uint8_t)((seenNext + 1) % (sizeof seen / sizeof *seen));
  return 0;
}
}  // namespace detail
inline void forgetRepeats() { for (detail::Seen& x : detail::seen) x = {}; detail::seenNext = 0; }   // tests

// Speed, when the game judges it (Times Cards' own Easy and Hard). A game that doesn't counts every right answer quick.
enum Speed : int8_t { QUICK = 0, SLOW = 1 };
// One answer. beta: the item's difficulty; options: answers it offered; ms: from the item showing to the pick; readMs:
// no one reads the item and its answers faster: a right pick sooner is a guess (pays nothing, counts as wrong), a
// wrong one sooner costs half a base in the round; key: which item, for repeats (the game's own numbering).
inline void answer(Book& b, Round& r, Skill k, int beta, int options, bool right, uint32_t ms, uint32_t readMs,
                   uint16_t key, Speed speed = QUICK) {
  if (!started(b, k)) seed(b, k, 0, false);
  State s = get(b, k);
  const bool wasCalibrating = s.calibrating, rushed = ms < readMs;
  const float sc = !right || rushed ? 0.f : speed == SLOW ? 0.7f : 1.f;
  const float p = expect(s.theta, beta, options);
  const int payTheta = s.peak - PEAK_DROP > s.theta ? s.peak - PEAK_DROP : s.theta;
  const int times = detail::repeats(b, k, key);
  if (right && !rushed) {
    float w = s.calibrating ? 1.f : (1 - expect(payTheta, beta, options)) / (1 - TARGET);
    w = w < 0.25f ? 0.25f : w > 2.5f ? 2.5f : w;
    r.points += times >= 3 ? 0 : (int32_t)lroundf(BASE[k] * w) >> times;
  } else if (!right && rushed) {
    r.points -= BASE[k] / 2;
  }
  if (r.points < 0) r.points = 0;
  r.ms += ms > 30000 ? 30000 : ms;
  r.answers++;
  if (s.calibrating) {
    if (sc > 0) { s.theta += s.jump * LEVEL; s.jump = s.jump >= 4 ? 4 : s.jump * 2; }
    else {
      s.theta -= s.jump * LEVEL / 2; s.jump = s.jump > 1 ? s.jump / 2 : 1;
      if (s.jump == 1 && s.errored) s.calibrating = false;   // two errors, and the jump is one level
      s.errored = true;
    }
    if (s.n + 1 >= CALIB_ANSWERS) s.calibrating = false;
    if (!s.calibrating) { s.n = 0; s.trend = 0; }
  } else {
    const float d = sc - p;
    s.trend = d > 0 ? (s.trend > 0 ? s.trend + 1 : 1) : (s.trend < 0 ? s.trend - 1 : -1);
    s.trend = s.trend > 3 ? 3 : s.trend < -3 ? -3 : s.trend;
    int kk = 160 / (1 + s.n / 5);
    kk = kk < 40 ? 40 : kk;
    if (s.trend >= 3 || s.trend <= -3) kk = kk * 2 > 160 ? 160 : kk * 2;
    s.theta += (int)lroundf(kk * d);
  }
  s.theta = s.theta < 0 ? 0 : s.theta > THETA_MAX ? THETA_MAX : s.theta;
  if (!wasCalibrating || s.calibrating) s.n++;
  if (s.theta > s.peak) s.peak = s.theta;
  const int lvl = s.theta / LEVEL;
  if (!s.calibrating && lvl + 1 > s.best) {
    if (!wasCalibrating) r.newBest = true;   // the level a calibration lands on is a start, not a milestone
    s.best = lvl + 1;
  }
  put(b, k, s);
}

// A round's start: the day's counters are today's (a repeat is the same item the same day).
inline Round start(Book& b, uint32_t nowSec) { today(b, nowSec); return Round{}; }

// A round's end: its points into stars by the day's rules, into the wallet. Returns the stars it paid in all.
inline int finish(Book& b, Round& r, uint32_t nowSec, uint32_t* wallet) {
  today(b, nowSec);
  int32_t pts = (int32_t)((int64_t)(r.points < 0 ? 0 : r.points) * (b.tempo ? b.tempo : 100) / 100);
  const int32_t room = (SOFT_CAP_STARS - (int)b.learnStars) * POINTS_PER_STAR - b.carry;
  if (pts > room) { pts = (room > 0 ? room : 0) + (pts - (room > 0 ? room : 0)) / 4; r.resting = true; }
  pts += b.carry;
  r.stars = pts / POINTS_PER_STAR;
  b.carry = (uint8_t)(pts % POINTS_PER_STAR);
  const int fresh = REST_BONUS_STARS - (int)b.learnStars;
  r.bonus = fresh > 0 ? (r.stars < fresh ? r.stars : fresh) : 0;
  r.milestone = r.newBest ? MILESTONE_STARS : 0;
  b.learnStars = (uint8_t)(b.learnStars + r.stars > 255 ? 255 : b.learnStars + r.stars);
  b.weekStars = (uint16_t)(b.weekStars + r.stars);
  const uint32_t sec = b.weekSec + r.ms / 1000;
  b.weekSec = (uint16_t)(sec > 65535 ? 65535 : sec);
  const int all = r.stars + r.bonus + r.milestone;
  stars::award(wallet, (uint32_t)all);
  return all;
}

// Stars from a game that teaches nothing (a race won, a level cleared): at most PLAY_DAY a day across all such games.
// Returns how many it paid. Without a book (a test), all of them.
inline uint32_t earnForPlay(Book* b, uint32_t* wallet, uint32_t nowSec, uint32_t n) {
  if (b) {
    today(*b, nowSec);
    const uint32_t left = b->playStars >= PLAY_DAY ? 0 : PLAY_DAY - b->playStars;
    n = n < left ? n : left;
    b->playStars = (uint8_t)(b->playStars + n);
  }
  stars::award(wallet, n);
  return n;
}
}  // namespace skills
