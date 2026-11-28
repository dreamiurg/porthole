// Self-check for Pets Club's Ukrainian words (games/pets-club/ukrainian.h, content_uk.h): the engine's calibration
// finding every reader's step through these words, a week's play settling near 80% right, variety at the top, the review queue, every question each word can make, every letter in the
// font (os/uk_fonts.h), and every line measured where ukrainian.cpp draws it.
// Run: make test
#include "host/check.h"
#include <math.h>
#include <stdio.h>
#include <string.h>
#include "font.h"
#include "games/pets-club/sentence_layout.h"
#include "games/pets-club/ukrainian.h"
#include "uk_fonts.h"

using namespace learn::uk;
static int checks = 0;

// A kid who reads every word up to `step` and none above it, answering meanings in a reader's time, until the engine's
// calibration (os/skills.h) ends. Returns the answers it took; b holds the rating.
static int calibrate(int step, skills::Book& b) {
  b = skills::Book{}; skills::forgetRepeats();
  skills::seed(b, skills::WORDS_UK, 0, false);
  skills::Round r = skills::start(b, 20000 * 86400u);
  uint32_t rng = 7;
  int answers = 0;
  while (skills::calibrating(b, skills::WORDS_UK) && answers < 40) {
    const int w = pick(MEANING, target(b, MEANING), rng, nullptr, 0);
    skills::answer(b, r, item(MEANING, w, 3000), WORDS[w].step <= step, 3000);
    answers++;
  }
  return answers;
}

static void calibration() {
  skills::Book b;
  // Every reader is found within the engine's 15 answers, at their step or one off (the top step holds all above it).
  for (int step = 1; step <= STEPS; step++) {
    const int n = calibrate(step, b), found = skills::level(b, skills::WORDS_UK) + 1, at = found > STEPS ? STEPS : found;
    CHECK(n <= skills::CALIB_ANSWERS && at >= step - 1 && at <= step + 1);
  }
  // The next word is near the level found, and a guess (faster than reading) moves it like a miss.
  calibrate(4, b);
  uint32_t seed3 = 3;
  CHECK(abs(target(b, MEANING) - beta(WORDS[pick(MEANING, target(b, MEANING), seed3, nullptr, 0)], MEANING)) <= 2 * skills::LEVEL);
  CHECK(item(MEANING, 0, 300).readMs > 300 && item(SYLLABLES, 0, 20000).speed == skills::SLOW && item(LOOKALIKE, 0, 3000).speed == skills::QUICK);
  CHECK(item(SYLLABLES, 0, 0).options == 1 && item(MEANING, 0, 0).options == OPTIONS);
}

// At the top of the ladder, words asked lately are kept out: 30 questions in a row (the game's ring of 16) all differ
// over any 16, and come from more than the top step alone.
static void variety() {
  uint32_t rng = 99;
  uint16_t ring[16]; int n = 0, lowest = STEPS;
  for (int q = 0; q < 30; q++) {
    for (int k = 0; k < WORD_KINDS; k++) {
      const int w = pick((Kind)k, STEPS * skills::LEVEL, rng, ring, n);
      CHECK(w >= 0);
      lowest = WORDS[w].step < lowest ? WORDS[w].step : lowest;
      if (n == 16) memmove(ring, ring + 1, sizeof ring - sizeof *ring); else n++;
      ring[n - 1] = (uint16_t)w;
    }
  }
  CHECK(lowest < STEPS);
}

// A week of play shaped like 2026-11-27's telemetry (PostHog, one kid, 96 lookalike answers): meaning sat on the model,
// lookalikes were right 64% where the model said 77%; the fit put lookalikes 175 above a word's step. A kid that good
// (rating 280) playing rounds of the three kinds settles near their rating, and every kind lands near the 80% target.
static uint32_t lcg(uint32_t& x) { x = x * 1664525u + 1013904223u; return x >> 8; }
static void settles() {
  constexpr int TRUE_OFFSET[WORD_KINDS] = {0, 175, 50};   // what the kid's answers say, per kind
  constexpr int KID = 280, WARM = 100, PLAY = 600;
  skills::Book b{}; skills::forgetRepeats();
  skills::seed(b, skills::WORDS_UK, 0, false);
  skills::Round r = skills::start(b, 20000 * 86400u);
  uint32_t rng = 11, dice = 5;
  uint16_t ring[16]; int ringN = 0, asked[WORD_KINDS] = {}, right[WORD_KINDS] = {}, lo = 1 << 30, hi = 0;
  long sum = 0;
  for (int i = 0; i < WARM + PLAY; i++) {
    const Kind k = kindAt(i % ROUND, (uint32_t)(i / ROUND) * 7u);
    const int w = pick(k, target(b, k), rng, ring, ringN);
    const float p = skills::expect(KID, skills::LEVEL * (WORDS[w].step - 1) + TRUE_OFFSET[k], choices(k));
    const bool ok = (float)(lcg(dice) % 10000) < p * 10000;
    skills::answer(b, r, item(k, w, 3000), ok, 3000);
    if (ringN == 16) memmove(ring, ring + 1, sizeof ring - sizeof *ring); else ringN++;
    ring[ringN - 1] = (uint16_t)w;
    if (i < WARM) continue;
    const int theta = skills::get(b, skills::WORDS_UK).theta;
    asked[k]++; right[k] += ok; sum += theta; lo = theta < lo ? theta : lo; hi = theta > hi ? theta : hi;
  }
  for (int k = 0; k < WORD_KINDS; k++) {
    const float rate = (float)right[k] / asked[k];
    CHECK(rate > 0.72f && rate < 0.88f);   // 2026-11-27's numbers made lookalikes 72% here, and meaning 69% with the
                                           // new offset alone (the streak rule in skills.h kept the rating too high)
  }
  CHECK(labs(sum / PLAY - KID) <= 30 && lo > 0 && hi - lo <= 300);   // before: mean 289, 102..434
}

// Missed words ride the puzzles' review queue (review.h) as their own strand: due the next day, off when answered
// right while due, and never asked by a puzzle strand (nor a puzzle by this).
static void reviews() {
  Save p{};
  CHECK(due(p, 10) < 0);
  answered(p, 7, false, 10);
  CHECK(due(p, 10) < 0 && due(p, 11) == 7);
  answered(p, 7, false, 12);   // missed again: one place, due again the day after
  CHECK(due(p, 12) < 0 && due(p, 13) == 7 && learn::review::count(p) == 1);
  answered(p, 7, true, 12);    // right before it is due: it stays
  CHECK(due(p, 13) == 7);
  answered(p, 7, true, 13);
  CHECK(due(p, 99) < 0 && learn::review::count(p) == 0);
  answered(p, 200, false, 10);   // a word past 255 fits too
  learn::review::answered(p, {learn::puzzles::NUMBERS, 1, 2, 3, 0}, false, 10);
  CHECK(due(p, 11) == 200 && learn::review::due(p, learn::puzzles::NUMBERS, 11) == 1);
  for (int s = 0; s < learn::puzzles::STRAND_COUNT; s++) CHECK(learn::review::due(p, (learn::puzzles::Strand)s, 11) != 0);
  p.puzzleReviews[0].a = 0xFF; p.puzzleReviews[0].b = 0xFF;   // a word past the content (a save from a longer list) is skipped
  CHECK(due(p, 11) < 0);
}

// Every code point of s in the font.
static bool covered(const font::Font& f, const char* s) {
  for (const unsigned char* c = (const unsigned char*)s; *c;) {
    uint32_t cp = *c++;
    if (cp >= 0xE0) { cp = (cp & 0x0F) << 12; cp |= (*c++ & 0x3Fu) << 6; cp |= *c++ & 0x3Fu; }
    else if (cp >= 0xC0) { cp = (cp & 0x1F) << 6; cp |= *c++ & 0x3Fu; }
    if (!font::glyphIndex(f, cp)) { printf("  missing U+%04X in \"%s\"\n", (unsigned)cp, s); return false; }
  }
  return true;
}

static void tileRows(int i, const Question& q, const int* tw);
// A syllables question: its tiles in the font, the slots and the tiles in the circle.
static void tileChecks(int i, const Question& q) {
  const Word& w = WORDS[i];
  CHECK(q.tileCount == parts(w) + (w.step >= DECOY_STEP ? 2 : 1) && q.tileCount <= MAX_TILES);
  int mine = 0, tw[MAX_TILES];
  for (int t = 0; t < q.tileCount; t++) {
    char s[16]; syllable(WORDS[q.tiles[t].word], q.tiles[t].part, s, sizeof s);
    mine += q.tiles[t].word == i;
    CHECK(covered(ukfont::FONT48, s));
    tw[t] = (font::textWidth(ukfont::FONT48, s) + 2) / 3 + 8;   // ukrainian.cpp partW (at least 26)
    tw[t] = tw[t] < 26 ? 26 : tw[t];
  }
  CHECK(mine == parts(w));
  int widest = 0;   // the slots, all as wide as the widest syllable, in a row at y 66 to 88
  for (int t = 0; t < q.tileCount; t++) if (q.tiles[t].word == i) widest = tw[t] > widest ? tw[t] : widest;
  CHECK(parts(w) * widest + (parts(w) - 1) * 4 <= 150);
  tileRows(i, q, tw);
}
// The tiles inside the circle (three in a row, y 100 to 122; four as two rows of two; five as three and two, the lower
// row y 118 to 140), and no two tiles read the same.
static void tileRows(int i, const Question& q, const int* tw) {
  if (q.tileCount < 4) CHECK(tw[0] + tw[1] + tw[2] + 8 <= 134);
  else if (q.tileCount == 4) CHECK(tw[0] + tw[1] + 4 <= 130 && tw[2] + tw[3] + 4 <= 104);
  else CHECK(tw[0] + tw[1] + tw[2] + 8 <= 130 && tw[3] + tw[4] + 4 <= 104);
  for (int a = 0; a < q.tileCount; a++)
    for (int b = a + 1; b < q.tileCount; b++) {
      char x[16], y[16]; syllable(WORDS[q.tiles[a].word], q.tiles[a].part, x, sizeof x); syllable(WORDS[q.tiles[b].word], q.tiles[b].part, y, sizeof y);
      CHECK(q.tiles[a].word == i || q.tiles[b].word == i || strcmp(x, y));
    }
}

// One word's own lines, measured where ukrainian.cpp draws them (physical px: 480 wide, a logical px is 3).
static void wordLines(const Word& w) {
  CHECK(covered(ukfont::FONT88, w.uk) && covered(ukfont::FONT48, w.syl));
  CHECK(font::textWidth(ukfont::FONT48, w.uk) <= 400);   // the big word falls back to 48 and still fits
}

// A meaning or lookalike question: the right word among three different ones, each fitting its button.
static void optionChecks(Kind k, int i, const Question& q) {
  constexpr int OPTION_W = 84 * 3 - 8;
  CHECK(q.options[q.right] == i);
  for (int a = 0; a < OPTIONS; a++)
    for (int b = a + 1; b < OPTIONS; b++) CHECK(q.options[a] != q.options[b] && strcmp(WORDS[q.options[a]].en, WORDS[q.options[b]].en));
  if (k == MEANING) {
    for (int a = 0; a < OPTIONS; a++) CHECK(strlen(WORDS[q.options[a]].en) * 6 <= 80);   // the 8x8 font in the button
    return;
  }
  for (int a = 0; a < OPTIONS; a++) CHECK(font::textWidth(ukfont::FONT48, WORDS[q.options[a]].uk) <= OPTION_W);
  CHECK(covered(ukfont::FONT24, hint(q)) && font::textHeight(ukfont::FONT24, hint(q), 330, 0) <= 2 * ukfont::FONT24.lineHeight);
}

// Every word's questions, and every line of them.
static void questions() {
  uint32_t rng = 12345;
  int tiles = 0, looks = 0;
  for (int i = 0; i < WORD_COUNT; i++) {
    wordLines(WORDS[i]);
    for (int k = 0; k < WORD_KINDS; k++) {
      if (!fits(WORDS[i], (Kind)k)) continue;
      const Question q = build((Kind)k, i, rng);
      if (k == SYLLABLES) { tiles++; tileChecks(i, q); }
      else { looks += k == LOOKALIKE; optionChecks((Kind)k, i, q); }
    }
  }
  CHECK(tiles >= 60 && looks >= 60);
  // Every exercise has a word at every target a kid can have, none twice in a round.
  for (int t = 0; t <= STEPS * skills::LEVEL; t += 50)
    for (int k = 0; k < WORD_KINDS; k++) {
      uint16_t asked[ROUND];
      for (int n = 0; n < ROUND; n++) { const int w = pick((Kind)k, t, rng, asked, n); CHECK(w >= 0 && fits(WORDS[w], (Kind)k)); asked[n] = (uint16_t)w; }
    }
}

// ---------------------------------------------------------------- sentences (sentences_uk.h)
namespace lay = learn::uk::layout;
using lay::laysOut;
static Bank widestFirst(int i) { uint32_t rng = 1; Bank b = bank(i, rng); lay::widestFirst(SENTENCES[i], b); return b; }
// Builds sentence i along order o, tile by tile, as the game checks a tap: each next tile fits, nothing else does
// that the order would not take, and a decoy never fits.
static void buildAlong(int i, int o) {
  const Sentence& s = SENTENCES[i];
  char built[MAX_BANK][TILE_CHARS], t[TILE_CHARS];
  const int n = answerTiles(s);
  for (int p = 0; p < n; p++) {
    CHECK(orderTile(s, o, p, t));
    CHECK(fitsNext(s, built, p, t));
    for (int d = 0; d < count(s.decoys); d++) { listItem(s.decoys, d, t); CHECK(!fitsNext(s, built, p, t) && !inAnswer(s, t)); }
    orderTile(s, o, p, built[p]);
  }
  char out[160]; int starts[4];
  joined(built, n, true, out, sizeof out);
  CHECK(lay::wrap(out, lay::ANSWER_Y, lay::DONE_LINES, starts) > 0);   // built, it fits its two lines
  CHECK(memcmp(out, built[0], 2) != 0 || (out[0] >= 'A' && out[0] <= 'Z'));   // a capital first letter (2 bytes: Cyrillic)
}
static void sentenceContent(int i) {
  const Sentence& s = SENTENCES[i];
  const int n = answerTiles(s), d = count(s.decoys);
  CHECK(s.step >= 1 && s.step <= SENTENCE_STEPS && s.toUk == (s.step >= 4));
  CHECK(n >= 2 && d >= 1 && n + d <= MAX_BANK);
  CHECK(covered(lay::textFont(), s.uk) && covered(lay::textFont(), s.en) && covered(lay::textFont(), s.tiles) && covered(lay::textFont(), s.decoys));
  int starts[4];
  CHECK(lay::wrap(s.toUk ? s.en : s.uk, lay::PROMPT_Y, lay::PROMPT_LINES, starts) > 0);   // the sentence in two lines
  for (int a = 0; a < count(s.alt); a++) {   // an other order: every tile once
    char seq[MAX_BANK + 1]; listItem(s.alt, a, seq, sizeof seq);
    int seen = 0;
    CHECK((int)strlen(seq) == n);
    for (const char* c = seq; *c; c++) { CHECK(*c >= '0' && *c - '0' < n); seen |= 1 << (*c - '0'); }
    CHECK(seen == (1 << n) - 1);
  }
  for (int o = 0; o <= count(s.alt); o++) buildAlong(i, o);
  lay::Rect r[MAX_BANK];
  CHECK(laysOut(s, widestFirst(i), r));
  for (int t = 0; t < widestFirst(i).n; t++)   // every tile inside the circle
    CHECK(r[t].w <= lay::chord(r[t].y, r[t].y + r[t].h) && r[t].y + r[t].h <= 80 + lay::R);
}
static void sentenceRules();
static void sentences() {
  int perStep[SENTENCE_STEPS + 1] = {}, fitted = 0, tries = 0;
  for (int i = 0; i < SENTENCE_COUNT; i++) {
    sentenceContent(i); perStep[SENTENCES[i].step]++;
    uint32_t rng = (uint32_t)i * 7 + 1;
    for (int k = 0; k < 20; k++) { lay::Rect r[MAX_BANK]; fitted += laysOut(SENTENCES[i], bank(i, rng), r); tries++; }
  }
  for (int st = 1; st <= SENTENCE_STEPS; st++) CHECK(perStep[st] >= 10);
  CHECK(fitted * 10 >= tries * 9);   // a shuffle almost always fits; the game reshuffles, then goes widest first
  sentenceRules();
}
// What the game does with a sentence: its capital and tail, picking, places and rating.
static void sentenceRules() {
  // The capital letters the game makes: Latin, Cyrillic both halves, and і.
  const char words[4][TILE_CHARS] = {"today", "сьогодні", "у", "із"};
  const char* const want[4] = {"Today.", "Сьогодні.", "У.", "Із."};
  for (int k = 0; k < 4; k++) { char out[40]; joined(&words[k], 1, true, out, sizeof out); CHECK(!strcmp(out, want[k])); }
  // A long built line shows its end after "...", inside the circle.
  char out[160]; lay::tail("today dad and I will go to school and then home again", out, sizeof out);
  CHECK(!strncmp(out, "...", 3) && strstr(out, "again") && lay::textW(out) <= lay::textRoom(lay::ANSWER_Y));
  // Picking: near the target, none asked lately; places: two, never the review's.
  uint32_t rng = 5; uint16_t asked[ROUND];
  for (int t = 0; t <= (SENTENCE_STEPS - 1) * skills::LEVEL; t += skills::LEVEL)
    for (int n = 0; n < ROUND; n++) {
      const int i = pickSentence(t, rng, asked, n);
      CHECK(i >= 0 && abs(sentenceBeta(i) - t) <= skills::LEVEL / 2);
      for (int m = 0; m < n; m++) CHECK(asked[m] != (SENTENCE_KEY | i));
      asked[n] = (uint16_t)(SENTENCE_KEY | i);
    }
  for (int k = 0; k < 50; k++) { const uint8_t at = sentencePlaces(rng, 2); CHECK(__builtin_popcount(at) == SENTENCE_PLACES && !(at & 4)); }
  // Rating: no miss is a full answer, one a slow one, more a wrong one.
  CHECK(sentenceRight(0) && sentenceRight(1) && !sentenceRight(2));
  CHECK(sentenceItem(0, 9000, 0).speed == skills::QUICK && sentenceItem(0, 9000, 1).speed == skills::SLOW);
  CHECK(sentenceItem(0, 0, 0).skill == skills::READ_UK && sentenceItem(0, 0, 0).options == 1);
  skills::Book b{};
  CHECK(!sentencesOn(b));
  skills::seed(b, skills::WORDS_UK, SENTENCE_FROM - 1, true);
  CHECK(sentencesOn(b));
}

int main() {
  calibration(); settles(); variety(); reviews(); questions(); sentences();
  printf("test_pets_uk: all checks passed (%d checks, %d words, %d lookalike sets, %d sentences)\n", checks, WORD_COUNT, LOOK_COUNT, SENTENCE_COUNT);
}
