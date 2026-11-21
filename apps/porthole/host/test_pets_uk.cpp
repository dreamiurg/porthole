// Self-check for Pets Club's Ukrainian words (games/pets-club/ukrainian.h, content_uk.h): the engine's calibration
// finding every reader's step through these words, variety at the top, the review queue, every question each word can make, every letter in the
// font (os/uk_fonts.h), and every line measured where ukrainian.cpp draws it.
// Run: make test
#include "host/check.h"
#include <math.h>
#include <stdio.h>
#include <string.h>
#include "font.h"
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
    for (int k = 0; k < KIND_COUNT; k++) {
      const int w = pick((Kind)k, STEPS * skills::LEVEL, rng, ring, n);
      CHECK(w >= 0);
      lowest = WORDS[w].step < lowest ? WORDS[w].step : lowest;
      if (n == 16) memmove(ring, ring + 1, sizeof ring - sizeof *ring); else n++;
      ring[n - 1] = (uint16_t)w;
    }
  }
  CHECK(lowest < STEPS);
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

// A syllables question: its tiles in the font, a miss's line, the slots and the tiles in the circle.
static void tileChecks(int i, const Question& q) {
  const Word& w = WORDS[i];
  CHECK(q.tileCount == parts(w) + 1 && q.tileCount <= MAX_TILES);
  int mine = 0, tw[MAX_TILES];
  for (int t = 0; t < q.tileCount; t++) {
    char s[16]; syllable(WORDS[q.tiles[t].word], q.tiles[t].part, s, sizeof s);
    mine += q.tiles[t].word == i;
    CHECK(covered(ukfont::FONT48, s));
    char miss[80], first[16];   // a miss names the syllable wanted and the one tapped
    syllable(w, 0, first, sizeof first);
    snprintf(miss, sizeof miss, "After %s comes %s, not %s.", first, first, s);
    CHECK(font::textWidth(ukfont::FONT24, miss) <= 420);
    tw[t] = (font::textWidth(ukfont::FONT48, s) + 2) / 3 + 8;   // ukrainian.cpp partW (at least 26)
    tw[t] = tw[t] < 26 ? 26 : tw[t];
  }
  CHECK(mine == parts(w));
  int widest = 0;   // the slots, all as wide as the widest syllable, in a row at y 66 to 88
  for (int t = 0; t < q.tileCount; t++) if (q.tiles[t].word == i) widest = tw[t] > widest ? tw[t] : widest;
  CHECK(parts(w) * widest + (parts(w) - 1) * 4 <= 150);
  // inside the circle: three in a row (y 100 to 122), four as two rows of two (the lower one y 118 to 140)
  if (q.tileCount < 4) CHECK(tw[0] + tw[1] + tw[2] + 8 <= 134);
  else CHECK(tw[0] + tw[1] + 4 <= 130 && tw[2] + tw[3] + 4 <= 104);
}

// One word's own lines, measured where ukrainian.cpp draws them (physical px: 480 wide, a logical px is 3).
static void wordLines(const Word& w) {
  CHECK(covered(ukfont::FONT88, w.uk) && covered(ukfont::FONT48, w.syl) && covered(ukfont::FONT24, w.en));
  CHECK(font::textWidth(ukfont::FONT48, w.uk) <= 400);   // the big word falls back to 48 and still fits
  char line[64];   // the answer shown: Andika 48, or 24 when that is too wide (ukrainian.cpp lineFont)
  snprintf(line, sizeof line, "%s = %s", w.syl, w.en);
  CHECK(font::textWidth(ukfont::FONT48, line) <= 420 || font::textWidth(ukfont::FONT24, line) <= 420);
  snprintf(line, sizeof line, "Yes! It means %s.", w.en);   // a meaning's lines, one each (ukrainian.cpp drawUkMeaningText)
  CHECK(font::textWidth(ukfont::FONT24, line) <= 420);
  snprintf(line, sizeof line, "%s is %s.", w.en, w.uk);
  CHECK(font::textWidth(ukfont::FONT24, line) <= 420);
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
    for (int k = 0; k < KIND_COUNT; k++) {
      if (!fits(WORDS[i], (Kind)k)) continue;
      const Question q = build((Kind)k, i, rng);
      if (k == SYLLABLES) { tiles++; tileChecks(i, q); }
      else { looks += k == LOOKALIKE; optionChecks((Kind)k, i, q); }
    }
  }
  CHECK(tiles >= 60 && looks >= 60);
  // Every exercise has a word at every target a kid can have, none twice in a round.
  for (int t = 0; t <= STEPS * skills::LEVEL; t += 50)
    for (int k = 0; k < KIND_COUNT; k++) {
      uint16_t asked[ROUND];
      for (int n = 0; n < ROUND; n++) { const int w = pick((Kind)k, t, rng, asked, n); CHECK(w >= 0 && fits(WORDS[w], (Kind)k)); asked[n] = (uint16_t)w; }
    }
}

int main() {
  calibration(); variety(); reviews(); questions();
  printf("test_pets_uk: all checks passed (%d checks, %d words, %d lookalike sets)\n", checks, WORD_COUNT, LOOK_COUNT);
}
