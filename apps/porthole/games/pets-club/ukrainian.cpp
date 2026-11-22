// Ukrainian words, the Puzzles screen's fourth button (the design page "Українська в Porthole", 2026-11-21): rounds of
// five questions, the three exercises taking turns (ukrainian.h), at the kid's own level in the shared skills engine
// (os/skills.h, WORDS_UK), which calibrates a new kid from the easiest word up and pays the round's stars at its end. A
// missed word comes back as a meaning on a later day. No timer on screen: the time only tells a guess and a slow read.
// Colors say how it went, not words (the redesign of 2026-11-22): a right answer turns green and the next question
// comes by itself; a miss turns red and stays, the right one is not shown, and the kid tries again. Only the first
// answer rates the word. Five dots stand for the round, and the stars at its end are stars.
// The frame, the buttons and the English are Pets Club's own 8x8 art on the indexed frame; this screen is RGB565 so
// the Ukrainian can be Andika (os/uk_fonts.h) drawn over it at native size. Telemetry (os/telemetry.h) gets
// every answer, so the steps and the calibration can be tuned (docs/superpowers/specs/2026-11-21-ukrainian-telemetry.md).
#include <stdio.h>
#include <string.h>
#include "font.h"
#include "game.h"
#include "books.h"
#include "gfx565.h"
#include "palette.h"
#include "sprites.h"
#include "telemetry.h"
#include "uk_fonts.h"

using namespace gfx;
namespace uk = learn::uk;
namespace puzzles = learn::puzzles;

namespace {
constexpr uint32_t UK_SUMMARY_MS = 1000;   // a tap just after the last green pause must not start the next round
constexpr uint32_t UK_TAP_GUARD_MS = 300;  // a double tap's second half lands on the next screen: it answers nothing
constexpr uint32_t UK_NEXT_MS = 1000;      // a right answer stays green this long, then the next question comes by itself
constexpr uint32_t UK_FLASH_MS = 500;      // a miss turns the screen pink this long; the option missed stays red
const uint8_t OPTION_COLS[uk::OPTIONS] = {C_GREEN, C_ORANGE, C_BLUE};
const char* const KIND_NAMES[uk::KIND_COUNT] = {"meaning", "lookalike", "syllables"};
const ui::Box UK_MORE = {50, 118, 60, 22};
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
// An answer's mark on an option's right edge, half over it: a check on the right one, a cross on a miss.
void mark(const ui::Box& b, bool right) {
  const int cx = b.x + b.w - 1, cy = b.y + b.h / 2;
  circle(cx, cy, 6, C_DKBROWN); circle(cx, cy, 5, C_WHITE);
  if (right) blitTint(SPR_CHECK, cx - 4, cy - 3, C_DKGREEN); else blitTint(SPR_CROSS, cx - 3, cy - 3, C_RED);
}
// The word as big as it fits: Andika 88, or 48 for a long one.
const font::Font& bigFont(const char* s) { return font::textWidth(ukfont::FONT88, s) <= 400 ? ukfont::FONT88 : ukfont::FONT48; }
}  // namespace

// ---------------------------------------------------------------- the round
void Game::startUk() {
  if (screen_ != SC_UK) ukSeen_ = 0;   // a new visit shows each exercise's line again, once
  if (!rng_) rng_ = pet::rnd(save_);
  skills::seed(*book_, skills::WORDS_UK, 0, false);   // a new kid calibrates from the easiest words
  pay_ = skills::start(*book_, now_);
  ukSeed_ = puzzles::next(rng_); asked_ = right_ = roundStars_ = 0;
  memset(ukDots_, 0, sizeof ukDots_);
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
  ukFresh();
}
// A question just put up: nothing built, missed or answered, and the time counts from here (a finger still down
// answers nothing).
void Game::ukFresh() { ukBuilt_ = ukUsed_ = ukMissed_ = 0; ukFlash_ = -1; answered_ = -1; screenMs_ = ms_; }
// The first answer: the engine's rating and the round's points, the review queue, and the telemetry event that tunes them.
void Game::ukAnswer(bool right) {
  const uint32_t took = ms_ - screenMs_;
  const uk::Word& w = uk::WORDS[ukQ_.word];
  const skills::State before = skills::get(*book_, skills::WORDS_UK);
  skills::answer(*book_, pay_, uk::item(ukQ_.kind, ukQ_.word, took), right, took);
  const skills::State after = skills::get(*book_, skills::WORDS_UK);
  if (!right || took >= uk::READ_MS[ukQ_.kind]) uk::answered(save_, ukQ_.word, right, (uint16_t)pet::dayIndex(now_));   // a guess takes nothing off
  right_ += right;
  pet::puzzleAnswered(save_, now_, right); markDirty();   // the counters and the bond; the stars are the engine's
  using telemetry::P;
  telemetry::emit("uk_answer", {P("kind", KIND_NAMES[ukQ_.kind]), P("word", w.en), P("step", (long)w.step),
                                P("rating", (long)after.theta), P("correct", (long)right), P("ms", (long)took)});
  if (!before.calibrating || after.calibrating) return;
  telemetry::emit("uk_calibrated", {P("rating", (long)after.theta), P("answers", (long)before.n + 1)});
}
// A tap on option or tile i: the first one rates the word; a right one settles the question, a wrong one only says so.
void Game::ukTried(int i, bool right) {
  if (!ukDots_[asked_]) { ukDots_[asked_] = right ? 1 : 2; ukAnswer(right); }
  screenMs_ = ms_;
  if (right) { answered_ = i; ukFlash_ = -1; sound(SND_YIP); spawn(80, 96, 1, 4); return; }
  ukFlash_ = (int8_t)i;
}
// The question was green long enough: the next one, or the round's end.
void Game::ukAdvance() {
  ukSeen_ |= (uint8_t)(1u << ukQ_.kind);
  if (++asked_ < uk::ROUND) { nextUk(); return; }
  pet::learnRoundDone(save_); payRound(); markDirty();
  roundEndMs_ = screenMs_ = ms_; sound(SND_HAPPY); spawn(80, 60, 4, 10);
  const skills::State k = skills::get(*book_, skills::WORDS_UK);
  using telemetry::P;
  telemetry::emit("level_finished", {P("game", name()), P("what", "ukrainian"), P("right", (long)right_),
                                     P("asked", (long)uk::ROUND), P("rating", (long)k.theta),
                                     P("calibrating", (long)k.calibrating)});
}

void Game::updateUk() {
  if (backButton()) {   // what was answered is kept; a fifth right answer still in its green pause pays the round
    if (answered_ >= 0 && asked_ == uk::ROUND - 1) ukAdvance();
    go(SC_PUZZLES); return;
  }
  if (ukFlash_ < 0 && ms_ - screenMs_ < UK_TAP_GUARD_MS) return;   // after a miss the next try counts at once
  if (asked_ >= uk::ROUND) {
    if (ms_ - roundEndMs_ >= UK_SUMMARY_MS && tapBox(in_, UK_MORE) && pressedAfter(0)) startUk();
    return;
  }
  if (answered_ >= 0) { if (ms_ - screenMs_ >= UK_NEXT_MS) ukAdvance(); return; }
  if (ukQ_.kind == uk::SYLLABLES) updateUkTiles(); else updateUkOptions();
}
// Meaning and lookalike: a wrong option stays red and off; the right one settles the question.
void Game::updateUkOptions() {
  for (int i = 0; i < uk::OPTIONS; i++) {
    if (!ukOpen(i) || !tapBox(in_, optionBox(i)) || !pressedAfter(0)) continue;
    if (i != ukQ_.right) ukMissed_ |= (uint8_t)(1u << i);
    ukTried(i, i == ukQ_.right);
    return;
  }
}
// Syllables, tapped in order: a tile that reads as the next syllable goes into its slot (two the same are the same).
// Any other is a miss: one the word needs later only flashes; one it never needs stays red and off.
void Game::updateUkTiles() {
  const uk::Word& w = uk::WORDS[ukQ_.word];
  for (int i = 0; i < ukQ_.tileCount; i++) {
    if ((ukUsed_ >> i) & 1 || !ukOpen(i) || !tapBox(in_, tileBox(ukQ_, i)) || !pressedAfter(0)) continue;
    char want[16], got[16]; uk::syllable(w, ukBuilt_, want, sizeof want); syl(ukQ_.tiles[i], got);
    if (strcmp(want, got)) {
      bool later = false;
      for (int p = ukBuilt_ + 1; p < uk::parts(w) && !later; p++) { uk::syllable(w, p, want, sizeof want); later = !strcmp(want, got); }
      if (!later) ukMissed_ |= (uint8_t)(1u << i);
      ukTried(i, false);
      return;
    }
    ukUsed_ |= (uint8_t)(1u << i); ukBuilt_++; ukFlash_ = -1;
    if (ukBuilt_ == uk::parts(w)) ukTried(i, true);
    return;
  }
}

// ---------------------------------------------------------------- drawing: the indexed frame, then the Ukrainian
bool Game::ukFlashing() const { return ukFlash_ >= 0 && ms_ - screenMs_ < UK_FLASH_MS; }   // a miss's tap set screenMs_
// An option or tile still to tap: not missed, and the question not settled.
bool Game::ukOpen(int i) const { return answered_ < 0 && !((ukMissed_ >> i) & 1); }
// An option's or tile's fill: dark red once missed (a tile only out of order for a moment); once settled the right
// one green and the rest gray.
uint8_t Game::ukFill(int i) const {
  if ((ukMissed_ >> i) & 1 || (ukFlashing() && ukFlash_ == i)) return C_PLUM;
  if (answered_ >= 0) return i == answered_ ? C_GREEN : C_LTGRAY;
  return OPTION_COLS[i % uk::OPTIONS];
}
void Game::drawUk() {
  if (asked_ >= uk::ROUND) { drawUkDone(); return; }
  const uk::Word& w = uk::WORDS[ukQ_.word];
  clear(answered_ >= 0 ? C_MINT : ukFlashing() ? C_PINK : C_WALL);   // green once right, pink a moment on a miss
  drawUkDots(30);
  drawUkPrompt(w);
  drawUkChoices(w);
  drawBackButton();
}
// The round as five dots: green right the first time, orange found later, blue the one asked, cream to come.
void Game::drawUkDots(int y) {
  for (int i = 0; i < uk::ROUND; i++) {
    const uint8_t c = ukDots_[i] == 1 ? C_GREEN : ukDots_[i] ? C_ORANGE : i == asked_ ? C_BLUE : C_CREAM;
    circle(60 + i * 10, y, 3, C_DKBROWN); circle(60 + i * 10, y, 2, c);
  }
}
// The exercise's line, only the first time it comes this visit; under it the English to find (a meaning has none).
void Game::drawUkPrompt(const uk::Word& w) {
  static const char* const LINES[uk::KIND_COUNT] = {"What does it mean?", "Which word means", "Build the word for"};
  if (!((ukSeen_ >> ukQ_.kind) & 1)) textCentered(80, 40, LINES[ukQ_.kind], C_DKBROWN);
  if (ukQ_.kind == uk::MEANING) return;
  const int scale = textWidth(w.en, 2) <= 120 ? 2 : 1;
  textCentered(80, scale == 2 ? 50 : 53, w.en, C_NAVY, scale);
}
// The options with their marks, or the syllables' slots (green once the word is built) and tiles. One missed, or
// any once the question is settled, takes no tap.
void Game::drawUkChoices(const uk::Word& w) {
  const bool done = answered_ >= 0;
  if (ukQ_.kind != uk::SYLLABLES) {
    for (int i = 0; i < uk::OPTIONS; i++) {
      const bool missed = (ukMissed_ >> i) & 1;
      const ui::Button b = {optionBox(i), ukQ_.kind == uk::MEANING ? uk::WORDS[ukQ_.options[i]].en : nullptr, nullptr, ukFill(i)};
      if (ukOpen(i)) ui::button(in_, b); else ui::drawButton(b, false);
      if (missed || i == answered_) mark(b.box, !missed);
    }
    return;
  }
  for (int i = 0; i < uk::parts(w); i++) ui::panel(slotBox(ukQ_, i), done ? C_GREEN : C_CREAM, C_DKBROWN);
  for (int i = 0; i < ukQ_.tileCount; i++) {
    const ui::Button b = {tileBox(ukQ_, i), nullptr, nullptr, ukFill(i)};
    if ((ukUsed_ >> i) & 1) rect(b.box.x + 2, b.box.y + 2, b.box.w - 4, b.box.h - 4, C_PEACH);   // its place, now empty
    else if (ukOpen(i)) ui::button(in_, b);
    else ui::drawButton(b, false);
  }
}
// The round's end: its five dots and the stars it paid, drawn as stars (a heart when none); the arrow plays another round.
void Game::drawUkDone() {
  clear(C_WALL);
  drawUkDots(40);
  // shortcut: at most 12, two rows of 6; a round pays its stars, the same again as the day's first bonus, and 5 for a
  // new best, so more would need a third row (or a number)
  const int n = roundStars_ < 12 ? roundStars_ : 12, scale = n > 3 ? 2 : 3, step = 7 * scale + 4, rows = n > 6 ? 2 : 1;
  for (int r = 0, done = 0; r < rows; r++) {
    const int k = (n - done + rows - r - 1) / (rows - r), y = 62 - 7 * scale / 2 + (rows == 2 ? r * 22 - 7 : 0);
    for (int i = 0; i < k; i++) blitScaled(SPR_STAR, 80 - (k * step - 4) / 2 + i * step, y, scale);
    done += k;
  }
  if (!n) blitScaled(SPR_HEART, 80 - 7 * 3 / 2 - 1, 62 - 7 * 3 / 2, 3);   // no stars this time: the pup's heart, not a blank
  ui::button(in_, {UK_MORE, nullptr, nullptr, C_GREEN});
  blitTint(SPR_ARROW, UK_MORE.x + UK_MORE.w / 2 - 3, UK_MORE.y + UK_MORE.h / 2 - 4 + (ui::pressing(in_, UK_MORE) ? 1 : 0), C_DKBROWN);
  drawBackButton();
}
// The Ukrainian, at native size over the expanded frame: render() calls this last on SC_UK.
void Game::drawUkText() {
  const uint16_t* pal = palette565(tint());
  gfx565::expandIndexed3(gfx::fb, pal);
  if (asked_ >= uk::ROUND) return;
  const uk::Word& w = uk::WORDS[ukQ_.word];
  char s[16];
  if (ukQ_.kind == uk::MEANING) { ukCentered(bigFont(w.uk), 80, 56, w.uk, pal[C_DKBROWN]); return; }   // a descender clears the options
  if (ukQ_.kind == uk::LOOKALIKE) {
    for (int i = 0; i < uk::OPTIONS; i++) {
      const ui::Box o = optionBox(i);
      ukInBox(ukfont::FONT48, o, ukOpen(i) && ui::pressing(in_, o), uk::WORDS[ukQ_.options[i]].uk, pal[ui::inkOn(ukFill(i))]);
    }
    return;
  }
  for (int i = 0; i < ukBuilt_; i++) { uk::syllable(w, i, s, sizeof s); ukInBox(ukfont::FONT48, slotBox(ukQ_, i), false, s, pal[C_DKBROWN]); }
  for (int i = 0; i < ukQ_.tileCount; i++) {
    if ((ukUsed_ >> i) & 1) continue;
    const ui::Box t = tileBox(ukQ_, i);
    syl(ukQ_.tiles[i], s); ukInBox(ukfont::FONT48, t, ukOpen(i) && ui::pressing(in_, t), s, pal[ui::inkOn(ukFill(i))]);
  }
}

// For `debug` and the playtest's answer-uk: the question, the level, and where to tap for the right answer (uk_taps,
// in order, from the syllables already built) and for a wrong one not yet missed (uk_wrong).
void Game::ukDebug() const {
  if (screen_ != SC_UK) return;
  const skills::State k = skills::get(*book_, skills::WORDS_UK);
  printf("uk_kind=%s uk_word=\"%s\" uk_step=%d asked=%d right=%d answered=%d missed=%d uk_rating=%d uk_calibrating=%d uk_answers=%d",
         KIND_NAMES[ukQ_.kind], uk::WORDS[ukQ_.word].en, uk::WORDS[ukQ_.word].step, asked_, right_, answered_, ukMissed_,
         k.theta, k.calibrating, k.n);
  auto mid = [](const ui::Box& b) { printf("%d,%d", b.x + b.w / 2, b.y + b.h / 2); };
  printf(" uk_taps=");
  if (ukQ_.kind != uk::SYLLABLES) {
    const int a = (ukQ_.right + 1) % uk::OPTIONS, b = (ukQ_.right + 2) % uk::OPTIONS;
    mid(optionBox(ukQ_.right));
    if (ukOpen(a) || ukOpen(b)) { printf(" uk_wrong="); mid(optionBox(ukOpen(a) ? a : b)); }   // none once both are missed
    puts(""); return;
  }
  ukDebugTiles();
}
// The syllables' taps: the tiles in the word's order from the next slot, and a tile that is not next.
void Game::ukDebugTiles() const {
  auto mid = [](const ui::Box& b) { printf("%d,%d", b.x + b.w / 2, b.y + b.h / 2); };
  const uk::Word& w = uk::WORDS[ukQ_.word];
  uint8_t used = ukUsed_; int wrong = -1;
  for (int p = ukBuilt_; p < uk::parts(w); p++) {
    char want[16], got[16]; uk::syllable(w, p, want, sizeof want);
    for (int i = 0; i < ukQ_.tileCount; i++) {
      if ((used >> i) & 1) continue;
      syl(ukQ_.tiles[i], got);
      if (strcmp(want, got)) continue;
      used |= (uint8_t)(1u << i); printf(p > ukBuilt_ ? ";" : ""); mid(tileBox(ukQ_, i)); break;
    }
  }
  char next[16], got[16]; uk::syllable(w, ukBuilt_, next, sizeof next);
  for (int i = 0; i < ukQ_.tileCount && wrong < 0; i++) {
    syl(ukQ_.tiles[i], got);
    if (!((ukUsed_ >> i) & 1) && ukOpen(i) && strcmp(next, got)) wrong = i;
  }
  if (wrong >= 0) { printf(" uk_wrong="); mid(tileBox(ukQ_, wrong)); }
  puts("");
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
      ukQ_ = uk::build((uk::Kind)k, i, rng_); ukDots_[asked_] = 0; ukFresh();
      return true;
    }
    return false;
  }
  return false;
}

// A Ukrainian book's text (books.h) in Andika at native size over the expanded frame: render() calls this last on the
// cover picker and the reader for a Ukrainian book. The boxes are game.cpp's, in logical px times three, and
// books::ukbox's widths (which tools/check_books_uk.py holds every story to).
void Game::drawBookText() {
  namespace bx = books::ukbox;
  const uint16_t* pal = palette565(tint());
  gfx565::expandIndexed3(gfx::fb, pal);
  const Book& bk = curBook();
  auto centered = [](const font::Font& f, int w, int y, const char* s, uint16_t fg) {
    font::textBox({&f, (int16_t)w, 0, 0, font::Align::CENTER}, s, 241 - w / 2, y, fg);   // on the screen's middle
  };
  if (screen_ == SC_BOOKS) {
    centered(ukfont::FONT24, bx::COVER_W, 91 * 3, bk.title, pal[C_WHITE]);   // on drawBooks' plate
    return;
  }
  const int npages = books::pages(bk);
  centered(ukfont::FONT24, bx::TITLE_W, 24 * 3 - 4, bk.title, pal[C_PLUM]);
  if (page_ < npages) {
    font::textBox({&bx::pageFont(bk), bx::PAGE_W, 0, 0, font::Align::LEFT}, bk.pages[page_], 19 * 3, 38 * 3, pal[C_DKBROWN]);
    return;
  }
  centered(ukfont::FONT24, bx::QUESTION_W, 36 * 3 - 2, bk.question, pal[C_DKBROWN]);
  const int ay = bookAnswerTop(bk);
  for (int i = 0; i < books::answers(bk); i++) {
    const ui::Box b = {26, ay + i * 24, 108, 22};   // game.cpp's ANSWER_STEP and answer box
    const uint8_t col = answerPick_ < 0 ? C_BLUE : i == bk.correct ? C_GREEN : i == answerPick_ ? C_RED : C_LTGRAY;
    const int h = bx::lines(ukfont::FONT24, bk.answers[i], bx::ANSWER_W) * ukfont::FONT24.lineHeight;
    centered(ukfont::FONT24, bx::ANSWER_W, b.y * 3 + (b.h * 3 - h) / 2 + (ui::pressing(in_, b) ? 3 : 0), bk.answers[i], pal[ui::inkOn(col)]);
  }
}
