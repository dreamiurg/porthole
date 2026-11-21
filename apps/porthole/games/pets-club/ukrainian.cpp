// Ukrainian words, the Puzzles screen's fourth button (the design page "Українська в Porthole", 2026-11-21): rounds of
// five questions, the three exercises taking turns (ukrainian.h), at the kid's own level in the shared skills engine
// (os/skills.h, WORDS_UK), which calibrates a new kid from the easiest word up and pays the round's stars at its end. A
// missed word comes back as a meaning on a later day. No timer on screen: the time only tells a guess and a slow read.
// The frame, the buttons and the English are Pets Club's own 8x8 art on the indexed frame; this screen is RGB565 so
// the Ukrainian can be Andika (os/uk_fonts.h) drawn over it at native size. Telemetry (os/telemetry.h) gets
// every answer, so the steps and the calibration can be tuned (docs/superpowers/specs/2026-11-21-ukrainian-telemetry.md).
#include <stdio.h>
#include <string.h>
#include "font.h"
#include "game.h"
#include "gfx565.h"
#include "palette.h"
#include "sprites.h"
#include "telemetry.h"
#include "uk_fonts.h"

using namespace gfx;
namespace uk = learn::uk;
namespace puzzles = learn::puzzles;

namespace {
constexpr uint32_t UK_SUMMARY_MS = 1000;   // as the puzzles': a slow second tap on the last Next must not skip the end
constexpr uint32_t UK_TAP_GUARD_MS = 300;  // a double tap's second half lands on the next screen: it answers nothing
const uint8_t OPTION_COLS[uk::OPTIONS] = {C_GREEN, C_ORANGE, C_BLUE};
const char* const KIND_NAMES[uk::KIND_COUNT] = {"meaning", "lookalike", "syllables"};
const ui::Box UK_NEXT = {56, 122, 48, 22}, UK_MORE = {50, 118, 60, 22};
ui::Box optionBox(int i) { return {38, 78 + i * 24, 84, 22}; }   // the lowest inside the circle
constexpr int SLOT_Y = 66, TILE_Y = 92, TILE_H = 22, TILE_GAP = 4;
// A word asked into the ring of the last N, the oldest going.
template <int N> void remember(uint16_t (&ring)[N], uint8_t& n, uint16_t word) {
  if (n == N) memmove(ring, ring + 1, sizeof ring - sizeof *ring); else n++;
  ring[n - 1] = word;
}
bool tapBox(const Input& in, const ui::Box& b) { return in.tapIn(b.x, b.y, b.w, b.h); }
void syl(const uk::Tile& t, char* out) { uk::syllable(uk::WORDS[t.word], t.part, out, 16); }
// A syllable's tile: as wide as it is in Andika 48, with room around it.
int partW(const char* s) { const int w = (font::textWidth(ukfont::FONT48, s) + 2) / 3 + 8; return w < 26 ? 26 : w; }
// Box i of a centered row of n boxes this wide.
ui::Box inRow(const int* w, int n, int i, int y) {
  int all = (n - 1) * TILE_GAP, x = 0;
  for (int k = 0; k < n; k++) { all += w[k]; if (k < i) x += w[k] + TILE_GAP; }
  return {80 - all / 2 + x, y, w[i], TILE_H};
}
// Three tiles in a row; four in two rows of two, which the circle has room for.
ui::Box tileBox(const uk::Question& q, int i) {
  int w[uk::MAX_TILES]; char s[16];
  for (int k = 0; k < q.tileCount; k++) { syl(q.tiles[k], s); w[k] = partW(s); }
  if (q.tileCount < 4) return inRow(w, q.tileCount, i, TILE_Y + 8);
  return inRow(w + i / 2 * 2, 2, i % 2, TILE_Y + i / 2 * (TILE_H + TILE_GAP));
}
ui::Box slotBox(const uk::Question& q, int i) {
  const uk::Word& word = uk::WORDS[q.word];
  int w[uk::MAX_TILES]; char s[16];
  int widest = 0;   // all as wide: a slot's width would tell which syllable goes in it
  for (int k = 0; k < uk::parts(word); k++) { uk::syllable(word, k, s, sizeof s); widest = partW(s) > widest ? partW(s) : widest; }
  for (int k = 0; k < uk::parts(word); k++) w[k] = widest;
  return inRow(w, uk::parts(word), i, SLOT_Y);
}
// Native-size text centered on a logical point.
void ukCentered(const font::Font& f, int cx, int cy, const char* s, uint16_t fg) {
  font::text(f, s, cx * 3 + 1 - font::textWidth(f, s) / 2, cy * 3 + 1 - f.lineHeight / 2, fg);
}
void ukInBox(const font::Font& f, const ui::Box& b, bool pressed, const char* s, uint16_t fg) {
  ukCentered(f, b.x + b.w / 2, b.y + b.h / 2 + (pressed ? 1 : 0), s, fg);
}
// The indexed frame's palette as RGB565, tinted as the firmware tints an indexed frame (it leaves RGB565 alone).
const uint16_t* palette565(Tint t) {
  static uint16_t pal[TINT_COUNT][C_COUNT]; static bool made = false;
  if (!made) {
    for (int k = 0; k < TINT_COUNT; k++) {
      uint32_t rgb[C_COUNT]; palette_build((Tint)k, rgb);
      for (int i = 0; i < C_COUNT; i++) pal[k][i] = rgb888_to_565(rgb[i]);
    }
    made = true;
  }
  return pal[t < TINT_COUNT ? t : TINT_DAY];
}
// A line of Ukrainian as big as it fits: Andika 48, or 24 for a long one.
const font::Font& lineFont(const char* s) { return font::textWidth(ukfont::FONT48, s) <= 420 ? ukfont::FONT48 : ukfont::FONT24; }
// The word as big as it fits: Andika 88, or 48 for a long one.
const font::Font& bigFont(const char* s) { return font::textWidth(ukfont::FONT88, s) <= 400 ? ukfont::FONT88 : ukfont::FONT48; }
}  // namespace

// ---------------------------------------------------------------- the round
void Game::startUk() {
  if (!rng_) rng_ = pet::rnd(save_);
  skills::seed(*book_, skills::WORDS_UK, 0, false);   // a new kid calibrates from the easiest words
  pay_ = skills::start(*book_, now_);
  ukSeed_ = puzzles::next(rng_); asked_ = right_ = roundStars_ = 0; ukFound_ = false;
  const int due = uk::due(save_, (uint16_t)pet::dayIndex(now_));   // a missed word takes one place, as a meaning
  ukReview_ = (int16_t)due;
  if (due >= 0) remember(ukRecent_, ukRecentN_, (uint16_t)due);   // not picked fresh as well
  ukReviewAt_ = (int8_t)(due < 0 ? -1 : 1 + puzzles::below(rng_, uk::ROUND - 1));
  nextUk();
  go(SC_UK);
}
void Game::nextUk() {
  uk::Kind k = uk::kindAt(asked_, ukSeed_);
  int word = -1;
  if (asked_ == ukReviewAt_) { k = uk::MEANING; word = ukReview_; }
  if (word < 0) word = uk::pick(k, uk::target(*book_, k), rng_, ukRecent_, ukRecentN_);   // none asked lately
  if (word < 0) word = uk::pick(k, uk::target(*book_, k), rng_, ukAsked_, asked_);
  if (word < 0) { k = uk::MEANING; word = uk::pick(k, uk::target(*book_, k), rng_, ukAsked_, asked_); }
  ukAsked_[asked_] = (uint16_t)word;
  remember(ukRecent_, ukRecentN_, (uint16_t)word);
  ukQ_ = uk::build(k, word, rng_);
  ukBuilt_ = ukUsed_ = 0; answered_ = -1; screenMs_ = ms_;   // the time counts from here; a finger still down answers nothing
}
// An answer: the engine's rating and the round's points, the review queue, and the telemetry event that tunes them.
void Game::ukAnswer(bool right) {
  const uint32_t took = ms_ - screenMs_;
  const uk::Word& w = uk::WORDS[ukQ_.word];
  const skills::State before = skills::get(*book_, skills::WORDS_UK);
  skills::answer(*book_, pay_, uk::item(ukQ_.kind, ukQ_.word, took), right, took);
  const skills::State after = skills::get(*book_, skills::WORDS_UK);
  if (!right || took >= uk::READ_MS[ukQ_.kind]) uk::answered(save_, ukQ_.word, right, (uint16_t)pet::dayIndex(now_));   // a guess takes nothing off
  right_ += right; screenMs_ = ms_;
  pet::puzzleAnswered(save_, now_, right); markDirty();   // the counters and the bond; the stars are the engine's
  if (right) { sound(SND_YIP); spawn(80, 96, 1, 4); }
  using telemetry::P;
  telemetry::emit("uk_answer", {P("kind", KIND_NAMES[ukQ_.kind]), P("word", w.en), P("step", (long)w.step),
                                P("rating", (long)after.theta), P("correct", (long)right), P("ms", (long)took)});
  if (!before.calibrating || after.calibrating) return;
  ukFound_ = true;
  telemetry::emit("uk_calibrated", {P("rating", (long)after.theta), P("answers", (long)before.n + 1)});
}

void Game::updateUk() {
  if (backButton()) { go(SC_PUZZLES); return; }   // what was answered is kept; only the round's end is lost
  if (ms_ - screenMs_ < UK_TAP_GUARD_MS) return;
  if (asked_ >= uk::ROUND) {
    if (ms_ - roundEndMs_ >= UK_SUMMARY_MS && tapBox(in_, UK_MORE) && pressedAfter(0)) startUk();
    return;
  }
  if (answered_ >= 0) {
    if (!tapBox(in_, UK_NEXT) || !pressedAfter(0)) return;
    if (++asked_ < uk::ROUND) { nextUk(); return; }
    pet::learnRoundDone(save_); payRound(); markDirty();
    roundEndMs_ = screenMs_ = ms_; sound(SND_HAPPY); spawn(80, 60, 4, 10);
    const skills::State k = skills::get(*book_, skills::WORDS_UK);
    using telemetry::P;
    telemetry::emit("level_finished", {P("game", name()), P("what", "ukrainian"), P("right", (long)right_),
                                       P("asked", (long)uk::ROUND), P("rating", (long)k.theta),
                                       P("calibrating", (long)k.calibrating)});
    return;
  }
  if (ukQ_.kind == uk::SYLLABLES) { updateUkTiles(); return; }
  for (int i = 0; i < uk::OPTIONS; i++) {
    if (!tapBox(in_, optionBox(i)) || !pressedAfter(0)) continue;
    answered_ = i;
    ukAnswer(i == ukQ_.right);
    return;
  }
}
// Syllables, tapped in order: a tile that reads as the next syllable goes into its slot (two the same are the same);
// any other ends the question as a miss.
void Game::updateUkTiles() {
  const uk::Word& w = uk::WORDS[ukQ_.word];
  for (int i = 0; i < ukQ_.tileCount; i++) {
    if ((ukUsed_ >> i) & 1 || !tapBox(in_, tileBox(ukQ_, i)) || !pressedAfter(0)) continue;
    char want[16], got[16]; uk::syllable(w, ukBuilt_, want, sizeof want); syl(ukQ_.tiles[i], got);
    if (strcmp(want, got)) { answered_ = i; ukAnswer(false); return; }
    ukUsed_ |= (uint8_t)(1u << i); ukBuilt_++;
    if (ukBuilt_ == uk::parts(w)) { answered_ = i; ukAnswer(true); }
    return;
  }
}

// ---------------------------------------------------------------- drawing: the indexed frame, then the Ukrainian
bool Game::ukRight() const {
  if (ukQ_.kind == uk::SYLLABLES) return ukBuilt_ == uk::parts(uk::WORDS[ukQ_.word]);
  return answered_ == ukQ_.right;
}
void Game::drawUk() {
  if (asked_ >= uk::ROUND) { drawUkDone(); return; }
  const uk::Word& w = uk::WORDS[ukQ_.word];
  const bool done = answered_ >= 0, right = done && ukRight();
  clear(right ? C_MINT : C_WALL);   // green behind a right answer, as the puzzles
  char b[48], s[96];
  snprintf(b, sizeof b, "Ukrainian %d of %d", asked_ + 1, uk::ROUND); textCentered(80, 30, b, C_DKGRAY);
  drawUkPrompt(w, done);
  if (done) {   // what it was (drawUkText), then a cheer, the star's cheer or a nudge, and Next
    fillNames(right ? learn::CHEERS[asked_ % uk::ROUND] : learn::NUDGES[asked_ % uk::ROUND], s, sizeof s);
    if (right) learn::lines(104, s, C_DKBROWN, 2);   // a miss's own lines take its place
    ui::button(in_, {UK_NEXT, "Next", nullptr, C_GREEN});
  } else drawUkChoices(w);
  drawBackButton();
}
// The question's line under the title; after an answer, drawUkText's line says it with the word.
void Game::drawUkPrompt(const uk::Word& w, bool done) {
  if (ukQ_.kind == uk::MEANING) { textCentered(80, 40, "What does it mean?", C_PLUM); return; }
  if (done) return;
  char b[48];
  textCentered(80, 40, ukQ_.kind == uk::LOOKALIKE ? "Which word means" : "Build the word for", C_PLUM);
  snprintf(b, sizeof b, "\"%s\"", w.en);
  const int scale = textWidth(b, 2) <= 120 ? 2 : 1;
  textCentered(80, scale == 2 ? 50 : 53, b, C_NAVY, scale);
}
// The buttons before an answer: the syllables' slots and tiles, or the three options.
void Game::drawUkChoices(const uk::Word& w) {
  if (ukQ_.kind != uk::SYLLABLES) {
    for (int i = 0; i < uk::OPTIONS; i++)
      ui::button(in_, {optionBox(i), ukQ_.kind == uk::MEANING ? uk::WORDS[ukQ_.options[i]].en : nullptr, nullptr, OPTION_COLS[i]});
    return;
  }
  for (int i = 0; i < uk::parts(w); i++) ui::panel(slotBox(ukQ_, i), C_CREAM, C_DKBROWN);
  for (int i = 0; i < ukQ_.tileCount; i++) {
    const ui::Box t = tileBox(ukQ_, i);
    if ((ukUsed_ >> i) & 1) rect(t.x + 2, t.y + 2, t.w - 4, t.h - 4, C_PEACH);   // its place, now empty
    else ui::button(in_, {t, nullptr, nullptr, OPTION_COLS[i % uk::OPTIONS]});
  }
}
// The round's end, as the puzzles': how it went and the stars; the first round says the kid's level was found.
void Game::drawUkDone() {
  clear(C_WALL);
  ui::panel({18, 30, 124, 84}, C_CREAM, C_DKBROWN);
  char b[64], s[64];
  snprintf(b, sizeof b, "%d right this time!", right_); textCentered(80, 36, b, C_PLUM);
  if (roundStars_) snprintf(b, sizeof b, "+%d star%s!", roundStars_, roundStars_ > 1 ? "s" : "");
  else snprintf(b, sizeof b, "Almost a star!");
  int y = learn::lines(50, b, C_NAVY, 2, 112);
  fillNames(ukFound_ ? "Next time: words just right for you!" : "{pet} is so proud of you.", s, sizeof s);
  learn::lines(y + 2, s, C_DKBROWN, 2, 112);
  if (roundStars_) blit(SPR_STAR, 112, 50);
  ui::button(in_, {UK_MORE, "More", nullptr, C_GREEN});
  drawBackButton();
}
// The Ukrainian, at native size over the expanded frame: render() calls this last on SC_UK.
void Game::drawUkText() {
  const uint16_t* pal = palette565(tint());
  gfx565::expandIndexed3(gfx::fb, pal);
  if (asked_ >= uk::ROUND) return;
  const uk::Word& w = uk::WORDS[ukQ_.word];
  char s[80];
  if (answered_ >= 0) { drawUkAnswerText(w, pal); return; }   // the word and what it means
  if (ukQ_.kind == uk::MEANING) { ukCentered(bigFont(w.uk), 80, 56, w.uk, pal[C_DKBROWN]); return; }   // a descender clears the options
  if (ukQ_.kind == uk::LOOKALIKE) {
    for (int i = 0; i < uk::OPTIONS; i++) {
      const ui::Box o = optionBox(i);
      ukInBox(ukfont::FONT48, o, ui::pressing(in_, o), uk::WORDS[ukQ_.options[i]].uk, pal[ui::inkOn(OPTION_COLS[i])]);
    }
    return;
  }
  for (int i = 0; i < ukBuilt_; i++) { uk::syllable(w, i, s, sizeof s); ukInBox(ukfont::FONT48, slotBox(ukQ_, i), false, s, pal[C_DKBROWN]); }
  for (int i = 0; i < ukQ_.tileCount; i++) {
    if ((ukUsed_ >> i) & 1) continue;
    const ui::Box t = tileBox(ukQ_, i);
    syl(ukQ_.tiles[i], s); ukInBox(ukfont::FONT48, t, ui::pressing(in_, t), s, pal[ui::inkOn(OPTION_COLS[i % uk::OPTIONS])]);
  }
}

// After an answer: the word and what it means; after a lookalike missed, the one picked and what that means.
void Game::drawUkAnswerText(const uk::Word& w, const uint16_t* pal) {
  char s[80];
  const bool right = ukRight(), picked = !right && ukQ_.kind == uk::LOOKALIKE;
  if (ukQ_.kind == uk::MEANING) { drawUkMeaningText(w, right, pal); return; }   // the word stays; under it what it means
  snprintf(s, sizeof s, "%s = %s", ukQ_.kind == uk::SYLLABLES && !right ? w.syl : w.uk, w.en);
  ukCentered(lineFont(s), 80, picked ? 70 : 80, s, pal[C_NAVY]);
  if (!picked) {
    if (right) return;
    char want[16], got[16], prev[16];   // which syllable it wanted, and the one tapped
    uk::syllable(w, ukBuilt_, want, sizeof want); syl(ukQ_.tiles[answered_], got);
    if (ukBuilt_) { uk::syllable(w, ukBuilt_ - 1, prev, sizeof prev); snprintf(s, sizeof s, "After %s comes %s, not %s.", prev, want, got); }
    else snprintf(s, sizeof s, "It starts with %s, not %s.", want, got);
    ukCentered(ukfont::FONT24, 80, 95, s, pal[C_DKBROWN]);
    return;
  }
  const uk::Word& other = uk::WORDS[ukQ_.options[answered_]];
  snprintf(s, sizeof s, "%s = %s", other.uk, other.en);
  ukCentered(lineFont(s), 80, 86, s, pal[C_DKBROWN]);
  font::textBox({&ukfont::FONT24, 330, 0, 0, font::Align::CENTER}, uk::hint(ukQ_), 75, 288, pal[C_SLATE]);
}
// A meaning answered: the word stays; under it what it means and, after a miss, the one picked.
void Game::drawUkMeaningText(const uk::Word& w, bool right, const uint16_t* pal) {
  char s[80];
  const uk::Word& other = uk::WORDS[ukQ_.options[answered_]];
  ukCentered(bigFont(w.uk), 80, 56, w.uk, pal[C_DKBROWN]);
  snprintf(s, sizeof s, right ? "Yes! It means %s." : "It means %s.", w.en);
  ukCentered(ukfont::FONT24, 80, right ? 90 : 86, s, pal[C_NAVY]);
  snprintf(s, sizeof s, "%s is %s.", other.en, other.uk);
  if (!right) ukCentered(ukfont::FONT24, 80, 97, s, pal[C_DKBROWN]);
}

// For `debug` and the playtest's answer-uk: the question, the level, and where to tap for the right answer (uk_taps,
// in order) and for a wrong one (uk_wrong).
void Game::ukDebug() const {
  if (screen_ != SC_UK) return;
  const skills::State k = skills::get(*book_, skills::WORDS_UK);
  printf("uk_kind=%s uk_word=\"%s\" uk_step=%d asked=%d right=%d answered=%d uk_rating=%d uk_calibrating=%d uk_answers=%d",
         KIND_NAMES[ukQ_.kind], uk::WORDS[ukQ_.word].en, uk::WORDS[ukQ_.word].step, asked_, right_, answered_,
         k.theta, k.calibrating, k.n);
  auto mid = [](const ui::Box& b) { printf("%d,%d", b.x + b.w / 2, b.y + b.h / 2); };
  printf(" uk_taps=");
  if (ukQ_.kind != uk::SYLLABLES) { mid(optionBox(ukQ_.right)); printf(" uk_wrong="); mid(optionBox((ukQ_.right + 1) % uk::OPTIONS)); puts(""); return; }
  const uk::Word& w = uk::WORDS[ukQ_.word];
  uint8_t used = 0; int wrong = -1;
  for (int p = 0; p < uk::parts(w); p++) {
    char want[16], got[16]; uk::syllable(w, p, want, sizeof want);
    for (int i = 0; i < ukQ_.tileCount; i++) {
      if ((used >> i) & 1) continue;
      syl(ukQ_.tiles[i], got);
      if (strcmp(want, got)) continue;
      used |= (uint8_t)(1u << i); printf(p ? ";" : ""); mid(tileBox(ukQ_, i)); break;
    }
  }
  char first[16], got[16]; uk::syllable(w, 0, first, sizeof first);
  for (int i = 0; i < ukQ_.tileCount && wrong < 0; i++) { syl(ukQ_.tiles[i], got); if (strcmp(first, got)) wrong = i; }
  printf(" uk_wrong="); mid(tileBox(ukQ_, wrong < 0 ? 0 : wrong)); puts("");
}
// Test hook: "ukmeaning X", "uklook X" or "uksyl X" puts the word that means X on screen as that exercise (Ukrainian).
bool Game::ukDebugCmd(const char* cmd) {
  static const char* const VERBS[uk::KIND_COUNT] = {"ukmeaning ", "uklook ", "uksyl "};
  for (int k = 0; k < uk::KIND_COUNT; k++) {
    const size_t n = strlen(VERBS[k]);
    if (strncmp(cmd, VERBS[k], n)) continue;
    for (int i = 0; i < uk::WORD_COUNT; i++) {
      if (strcmp(uk::WORDS[i].en, cmd + n) || !uk::fits(uk::WORDS[i], (uk::Kind)k)) continue;
      if (screen_ != SC_UK) startUk();
      ukQ_ = uk::build((uk::Kind)k, i, rng_); ukBuilt_ = ukUsed_ = 0; answered_ = -1; screenMs_ = ms_;
      return true;
    }
    return false;
  }
  return false;
}
