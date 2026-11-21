// Self-check for Pets Club's Ukrainian words (games/pets-club/ukrainian.h, content_uk.h): the calibration on the design
// page's two examples, the rating after it, the review queue, every question each word can make, every letter in the
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

// A kid who reads every word up to `level` and none above it, answering in a reader's time.
static int calibrate(int level, UkWords& p) {
  p = UkWords{}; start(p);
  int answers = 0;
  while (calibrating(p)) {
    const bool right = p.at <= level;
    answer(p, (int16_t)(STEP * p.at), right, MEANING, 3000);
    answers++;
  }
  return answers;
}

static void calibration() {
  UkWords p;
  // The page's examples: Zoe (reads to step 6) is found in 8 answers, 1 2 4 8 6 7 6 7; Zach (step 3) in 7.
  CHECK(calibrate(6, p) == 8 && p.rating == 600 && stepOf(p) == 6);
  CHECK(calibrate(3, p) == 7 && p.rating == 300 && stepOf(p) == 3);
  // Every level is found within CALIBRATION_MAX, at that level or one under it (two misses can come a step early).
  for (int level = 0; level <= STEPS; level++) {
    const int n = calibrate(level, p);
    CHECK(n <= CALIBRATION_MAX && !calibrating(p));
    CHECK(stepOf(p) <= (level < 1 ? 1 : level) && stepOf(p) >= (level < 2 ? 1 : level - 1));
  }
  // A guess (too fast to have read) or a right answer moves the calibration like a miss and like a right answer.
  p = UkWords{}; start(p);
  answer(p, STEP, true, MEANING, 300);
  CHECK(p.at == 1 && p.searching == 1);
  p = UkWords{}; start(p);
  answer(p, STEP, true, SYLLABLES, 2000);
  CHECK(p.at == 2 && p.jump == 2);
  // A grown-up's start step is kept, and a started kid is never reset.
  p = UkWords{}; start(p, 4); CHECK(p.at == 4 && p.rating == 400);
  start(p, 9); CHECK(p.at == 4);
}

static void rating() {
  UkWords p; calibrate(5, p);
  const int16_t r = p.rating, b = (int16_t)(r - TARGET_BELOW);
  UkWords up = p; answer(up, b, true, MEANING, 3000); CHECK(up.rating > r);
  // Right but slow scores 0.7, under the 80% expected at the target: it holds the kid about where they are.
  UkWords slow = p; answer(slow, b, true, MEANING, 20000); CHECK(slow.rating < up.rating && abs(slow.rating - r) < 10);
  UkWords guess = p; answer(guess, b, true, MEANING, 200); CHECK(guess.rating < r);
  UkWords down = p; answer(down, b, false, MEANING, 3000); CHECK(down.rating < r && r - down.rating > up.rating - r);
  // At the target, about 80% right keeps the rating where it is; all right climbs a step in a few rounds.
  CHECK(fabsf(0.8f - expected(r, b)) < 0.02f);
  UkWords climb = p;
  for (int i = 0; i < 15; i++) answer(climb, target(climb), true, MEANING, 3000);
  CHECK(climb.rating >= r + STEP);
  UkWords low = p;
  for (int i = 0; i < 200; i++) answer(low, STEP, false, MEANING, 3000);
  CHECK(low.rating == RATING_MIN);
}

static void reviews() {
  UkWords p{}; start(p);
  CHECK(due(p, 10) < 0);
  missed(p, 7, 10);
  CHECK(due(p, 10) < 0 && due(p, 11) == 7);
  missed(p, 7, 12);   // missed again: one place, due again the day after
  CHECK(due(p, 12) < 0 && due(p, 13) == 7 && p.reviewWord[1] == 0);
  reviewed(p, 7);
  CHECK(due(p, 99) < 0);
  for (int w = 0; w < UK_REVIEWS + 2; w++) missed(p, (uint16_t)w, 10);   // a full queue lets its oldest go
  CHECK(due(p, 11) == 2);
  p.reviewWord[0] = WORD_COUNT + 5;   // a word past the content (a save from a longer list) is skipped
  CHECK(due(p, 11) == 3);
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

// Every word's questions, and every line of them as ukrainian.cpp draws it (physical px: 480 wide, a logical px is 3).
static void questions() {
  constexpr int OPTION_W = 96 * 3 - 8, BIG_W = 400;
  uint32_t rng = 12345;
  int tiles = 0, looks = 0;
  for (int i = 0; i < WORD_COUNT; i++) {
    const Word& w = WORDS[i];
    CHECK(covered(ukfont::FONT88, w.uk) && covered(ukfont::FONT48, w.syl) && covered(ukfont::FONT24, w.en));
    CHECK(font::textWidth(ukfont::FONT48, w.uk) <= BIG_W);   // the big word falls back to 48 and still fits
    char line[64];   // the answer shown: Andika 48, or 24 when that is too wide (ukrainian.cpp lineFont)
    snprintf(line, sizeof line, "%s = %s", w.syl, w.en);
    CHECK(font::textWidth(ukfont::FONT48, line) <= 420 || font::textWidth(ukfont::FONT24, line) <= 420);
    snprintf(line, sizeof line, "It means %s, not %s.", w.en, w.en);   // the longest meaning twice is the widest
    CHECK(font::textHeight(ukfont::FONT24, line, 400, 0) <= 2 * ukfont::FONT24.lineHeight);
    for (int k = 0; k < KIND_COUNT; k++) {
      if (!fits(w, (Kind)k)) continue;
      const Question q = build((Kind)k, i, rng);
      if (k == SYLLABLES) {
        tiles++;
        CHECK(q.tileCount == parts(w) + 1 && q.tileCount <= MAX_TILES);
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
        // inside the circle: three in a row (y 100 to 122), four as two rows of two (the lower one y 118 to 140)
        if (q.tileCount < 4) CHECK(tw[0] + tw[1] + tw[2] + 8 <= 134);
        else CHECK(tw[0] + tw[1] + 4 <= 130 && tw[2] + tw[3] + 4 <= 104);
        continue;
      }
      CHECK(q.options[q.right] == i);
      for (int a = 0; a < OPTIONS; a++)
        for (int b = a + 1; b < OPTIONS; b++) CHECK(q.options[a] != q.options[b] && strcmp(WORDS[q.options[a]].en, WORDS[q.options[b]].en));
      if (k == MEANING) {
        for (int a = 0; a < OPTIONS; a++) CHECK(strlen(WORDS[q.options[a]].en) * 6 <= 92);   // the 8x8 font in the button
        continue;
      }
      looks++;
      for (int a = 0; a < OPTIONS; a++) CHECK(font::textWidth(ukfont::FONT48, WORDS[q.options[a]].uk) <= OPTION_W);
      CHECK(covered(ukfont::FONT24, hint(q)) && font::textHeight(ukfont::FONT24, hint(q), 330, 0) <= 2 * ukfont::FONT24.lineHeight);
    }
  }
  CHECK(tiles >= 60 && looks >= 60);
  // Every exercise has a word at every target a kid can have, none twice in a round.
  for (int t = 0; t <= RATING_MAX; t += 50)
    for (int k = 0; k < KIND_COUNT; k++) {
      uint16_t asked[ROUND];
      for (int n = 0; n < ROUND; n++) { const int w = pick((Kind)k, (int16_t)t, rng, asked, n); CHECK(w >= 0 && fits(WORDS[w], (Kind)k)); asked[n] = (uint16_t)w; }
    }
}

int main() {
  calibration(); rating(); reviews(); questions();
  printf("test_pets_uk: all checks passed (%d checks, %d words, %d lookalike sets)\n", checks, WORD_COUNT, LOOK_COUNT);
}
