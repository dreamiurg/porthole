#include "game.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "font.h"
#include "generated/big.h"
#include "generated/fonts.h"
#include "launcher_icon.h"
#include "palette.h"
#include "stars.h"

namespace cards {
namespace {
using canvas::Box;
// Times Cards' own look: an index card (cream, a red margin line, blue rules) on a periwinkle desk.
constexpr uint16_t rgb(uint32_t c) { return rgb888_to_565(c); }
constexpr uint16_t DESK = rgb(0xaebdf2), SHADOW = rgb(0x8796d6), CARD = rgb(0xfffaf0), RULE = rgb(0xcfdff5),
                   MARGIN = rgb(0xf09090), INK = rgb(0x2a2d4f), SOFT = rgb(0x3f4370), WHITE = rgb(0xffffff);
constexpr uint16_t GRADE_C[3] = {rgb(0x23804a), rgb(0xa65a00), rgb(0xc43c34)};   // Easy, Hard, Wrong: white text reads on each
const char* const GRADE_WORD[3] = {"Easy", "Hard", "Wrong"};

// Hit boxes are logical (160 px across, like every game); everything is drawn at 3x.
constexpr ui::Box HOME = {68, 0, 25, 25};            // ui::back's place, the way back on every page
constexpr ui::Box GO = {47, 122, 66, 22};            // Go! (home page) and Again (done page)
constexpr ui::Box BIG_CARD = {20, 34, 120, 73};      // the card a kid taps to see the answer
constexpr ui::Box DECK = {33, 41, 94, 60};           // the deck's card on the home page: a tap deals, like Go!
constexpr ui::Box GRADE[3] = {{24, 110, 34, 22}, {63, 110, 34, 22}, {102, 110, 34, 22}};
constexpr Box px(const ui::Box& b) { return {b.x * 3, b.y * 3, (b.x + b.w) * 3, (b.y + b.h) * 3}; }

bool down(const Input& in, const ui::Box& b) { return in.down && ui::pressing(in, b); }
void centered(const font::Font& f, const char* s, int cx, int cy, uint16_t c) {
  font::text(f, s, cx - font::textWidth(f, s) / 2, cy - f.lineHeight / 2, c);
}
void house(bool pressed) {
  const int cx = 240, cy = 38;
  canvas::disc(cx, cy, 30, pressed ? SHADOW : INK);
  const int roof[] = {cx, cy - 15, cx + 15, cy - 1, cx - 15, cy - 1};
  canvas::poly(roof, 3, WHITE);
  canvas::roundBox({cx - 10, cy - 2, cx + 10, cy + 13}, 0, WHITE);
  canvas::roundBox({cx - 3, cy + 5, cx + 3, cy + 13}, 0, pressed ? SHADOW : INK);
}
void indexCard(Box b, int sink = 0) {   // a shadow, the card (pressed `sink` px toward it), its margin line and rules
  canvas::roundBox({b.x0 + 6, b.y0 + 8, b.x1 + 6, b.y1 + 8}, 20, SHADOW);
  b.y0 += sink; b.y1 += sink;
  canvas::roundBox(b, 20, CARD);
  for (int y = b.y0 + 72; y < b.y1 - 16; y += 40) canvas::roundBox({b.x0 + 14, y, b.x1 - 14, y + 2}, 0, RULE);
  canvas::roundBox({b.x0 + 14, b.y0 + 28, b.x1 - 14, b.y0 + 31}, 0, MARGIN);
}
void button(const ui::Box& lb, const char* word, uint16_t c, bool pressed, const font::Font& f) {
  const Box b = px(lb);
  if (!pressed) canvas::roundBox({b.x0, b.y0 + 5, b.x1, b.y1 + 5}, 18, SHADOW);
  const int dy = pressed ? 4 : 0;
  canvas::roundBox({b.x0, b.y0 + dy, b.x1, b.y1 + dy}, 18, c);
  centered(f, word, (b.x0 + b.x1) / 2, (b.y0 + b.y1) / 2 + dy, WHITE);
}
void fact(char* s, size_t n, int card, bool withAnswer) {   // "7 × 8", or "7 × 8 ="
  snprintf(s, n, withAnswer ? "%d \xC3\x97 %d =" : "%d \xC3\x97 %d", tableOf(card), factorOf(card));
}
}  // namespace

const gfx::Sprite& Game::icon() const { return LAUNCHER_ICON; }

void Game::enter(const AppEnter& e) {
  in_ = Input{};
  home_ = dirty_ = false;
  ms_ = e.ms; now_ = e.nowSec; wallet_ = e.stars;
  save_ = Save{};
  bool loaded = false;
  for (int i = 0; i < e.n; i++)   // only this kid's deck; a bad one starts fresh
    if (e.all[i].id == e.who->id && e.saves[i].len) loaded = loadBlob(e.saves[i].data, e.saves[i].len, save_);
  if (!loaded || !save_.level) { setUp(save_, startTable(e.who->age), today()); dirty_ = true; }
  clampDue(save_, today());
  round_ = Round{};
  go(P_HOME);
}

// A new page ignores touches for a moment. The answer page ignores only the finger that turned the card, by place, so
// a double tap on the card acts once but a grade right after the answer shows still counts.
void Game::go(Page p) {
  page_ = p;
  frames_.reset();
  if (p == P_ANSWER) gate_.shownAt(ms_, in_.x, in_.y);
  else gate_.shown(ms_);
}

void Game::deal() {
  build(round_, save_, today(), ms_ * 2654435761u ^ save_.answers);
  go(P_ASK);
}

void Game::grade(Grade g) {
  answer(round_, save_, g, today());
  dirty_ = true;
  if (round_.done()) stars::award(wallet_, ROUND_STARS);
  go(round_.done() ? P_DONE : P_ASK);
}

void Game::update(uint32_t nowSec, uint32_t ms, const Input& in) {
  ms_ = ms; now_ = nowSec;
  in_ = in;
  gate_.filter(in_, ms);
  if (ui::clicked(in_, HOME)) {   // from a card back to the deck; from the deck out to the launcher
    if (page_ == P_HOME) home_ = true; else go(P_HOME);
    return;
  }
  switch (page_) {
    case P_HOME:
    case P_DONE:   // Go!, Again, or (home) the deck's card
      if (ui::clicked(in_, GO) || (page_ == P_HOME && ui::clicked(in_, DECK))) deal();
      break;
    case P_ASK:
      if (ui::clicked(in_, BIG_CARD)) go(P_ANSWER);
      break;
    case P_ANSWER:
      for (int g = 0; g < 3; g++)
        if (ui::clicked(in_, GRADE[g])) { grade((Grade)g); break; }
      break;
    case P_COUNT: break;
  }
}

// A dot per fact of the kid's table: green known, amber learning, pale new.
static void dots(const Save& s, int cx, int y) {
  for (int n = 1; n <= FACTS; n++) {
    const uint8_t b = s.box[cardOf(s.level, n)];
    canvas::disc(cx + (n * 2 - FACTS - 1) * 12, y, 9, b >= MASTER_BOX ? GRADE_C[EASY] : b ? GRADE_C[HARD] : RULE);
  }
}

// The home page: the table the kid is on, big on its card, and its dots.
static void drawHome(const Save& s, bool goDown) {
  centered(FONT40, "Times Cards", 240, 92, INK);
  const Box card = {100, 124, 380, 304};
  indexCard(card);
  char t[16];
  snprintf(t, sizeof t, "%d \xC3\x97", s.level);
  centered(cardsbig::FONT110, t, 240, 214, INK);
  dots(s, 240, 280);
  centered(FONT28, allKnown(s) ? "Every table known!" : "Say it, then tap the card", 240, 336, SOFT);
  button(GO, "Go!", GRADE_C[EASY], goDown, FONT40);
}

void Game::draw() const {
  canvas::roundBox(canvas::FULL, 0, DESK);
  house(down(in_, HOME));
  char s[24];
  if (page_ == P_HOME) { drawHome(save_, down(in_, GO)); return; }
  if (page_ == P_DONE) {
    char t[32];
    if (round_.leveledUp) snprintf(t, sizeof t, "Next up: %d \xC3\x97!", save_.level);
    const char* title = round_.leveledUp ? t : !round_.hard && !round_.wrong ? "Perfect round!"
                        : round_.wrong * 2 > round_.base ? "Nice try!" : "Great job!";
    centered(FONT40, title, 240, 100, INK);
    indexCard({80, 140, 400, 304});
    snprintf(s, sizeof s, "+%u stars", (unsigned)ROUND_STARS);
    centered(FONT28, s, 240, 154, GRADE_C[HARD]);   // above the margin line, like a card's heading
    const int counts[3] = {round_.easy, round_.hard, round_.wrong};
    for (int g = 0; g < 3; g++) {
      const int x = 152 + g * 88;
      canvas::disc(x, 218, 32, GRADE_C[g]);
      snprintf(s, sizeof s, "%d", counts[g]);
      centered(FONT40, s, x, 218, WHITE);
      centered(FONT28, GRADE_WORD[g], x, 272, SOFT);
    }
    snprintf(s, sizeof s, "%d \xC3\x97", save_.level);   // where the table stands now
    centered(FONT28, s, 108, 336, INK);
    dots(save_, 264, 336);
    button(GO, "Again", GRADE_C[EASY], down(in_, GO), FONT40);
    return;
  }
  snprintf(s, sizeof s, "%d of %d", round_.number(), round_.base);   // a missed card's second look adds nothing to count
  centered(FONT28, round_.redo[round_.at] ? "One more try" : s, 240, 84, SOFT);
  const int dy = page_ == P_ASK && down(in_, BIG_CARD) ? 6 : 0;   // the card sinks under a finger
  indexCard(px(BIG_CARD), dy);
  const int c = round_.current();
  if (page_ == P_ASK) {
    fact(s, sizeof s, c, false);
    centered(cardsbig::FONT110, s, 240, 232 + dy, INK);
    centered(FONT28, "Say it out loud, then tap!", 240, 352, SOFT);
    return;
  }
  fact(s, sizeof s, c, true);
  centered(FONT40, s, 240, 166, SOFT);
  snprintf(s, sizeof s, "%d", answerOf(c));
  centered(cardsbig::FONT110, s, 240, 248, INK);
  for (int g = 0; g < 3; g++) button(GRADE[g], GRADE_WORD[g], GRADE_C[g], down(in_, GRADE[g]), FONT28);
}

void Game::render() {
  // Repaint only when what the page shows changed: the page, its card, the round's place and a button under the finger.
  struct Look { uint8_t page, card, level, at, n, held; } look = {page_, 0, save_.level, (uint8_t)round_.at,
                                                                   (uint8_t)round_.n, 0};
  if (page_ == P_ASK || page_ == P_ANSWER) look.card = (uint8_t)round_.current();
  const ui::Box* hot[] = {&HOME, &GO, &GRADE[0], &GRADE[1], &GRADE[2], &BIG_CARD};   // what each page draws pressed
  const bool on[P_COUNT][6] = {{1, 1, 0, 0, 0, 0}, {1, 0, 0, 0, 0, 1}, {1, 0, 1, 1, 1, 0}, {1, 1, 0, 0, 0, 0}};
  for (int i = 0; i < 6; i++) if (on[page_][i] && in_.down && ui::pressing(in_, *hot[i])) look.held = (uint8_t)(i + 1);
  frames_.render(nullptr, 0, os::crc32(&look, sizeof look), [&] { draw(); });
}

bool Game::takeSave(const void** data, size_t* len, bool allowed) {
  if (!allowed || !dirty_) return false;
  seal(save_);
  *data = &save_; *len = sizeof save_;   // the shell copies it out before the next update
  dirty_ = false;
  return true;
}

const char* Game::screenName() const {
  static const char* N[] = {"cards_home", "cards_ask", "cards_answer", "cards_done"};
  static_assert(sizeof N / sizeof *N == P_COUNT, "a name per page");
  return N[page_];
}

void Game::debugPrint() {
  const bool onCard = page_ == P_ASK || page_ == P_ANSWER;
  const int c = onCard ? round_.current() : 0;
  printf("[cards page=%s level=%d start=%d known=%d answers=%lu at=%d n=%d card=%dx%d box=%d due=%d easy=%d hard=%d "
         "wrong=%d leveled=%d stars=%lu pace=%d learning=%d]\n",
         screenName(), save_.level, save_.start, known(save_, save_.level), (unsigned long)save_.answers, round_.at,
         round_.n, onCard ? tableOf(c) : 0, onCard ? factorOf(c) : 0, onCard ? save_.box[c] : 0,
         onCard ? (int)(save_.due[c] - today()) : 0, round_.easy, round_.hard, round_.wrong, round_.leveledUp,
         (unsigned long)stars::balance(wallet_), paceOf(save_), learning(save_));
  printf("screen=%s\n", screenName());
}

void Game::debugCmd(const char* cmd) {
  if (strncmp(cmd, "level", 5)) return;
  const int t = atoi(cmd + 5);
  if (t < 1 || t > TABLES) return;
  setUp(save_, t, today());
  dirty_ = true;
  round_ = Round{};
  go(P_HOME);
}
}  // namespace cards
