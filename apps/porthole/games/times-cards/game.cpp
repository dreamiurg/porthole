#include "game.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "font.h"
#include "generated/big.h"
#include "generated/fonts.h"
#include "launcher_icon.h"
#include "palette.h"
#include "skills.h"
#include "stars.h"
#include "telemetry.h"

namespace cards {
namespace {
using canvas::Box;
// Times Cards' own look: an index card (cream, a red margin line, blue rules) on a periwinkle desk.
constexpr uint16_t rgb(uint32_t c) { return rgb888_to_565(c); }
constexpr uint16_t DESK = rgb(0xaebdf2), SHADOW = rgb(0x8796d6), CARD = rgb(0xfffaf0), RULE = rgb(0xcfdff5),
                   MARGIN = rgb(0xf09090), INK = rgb(0x2a2d4f), SOFT = rgb(0x3f4370), WHITE = rgb(0xffffff);
constexpr uint16_t GREEN = rgb(0x23804a), AMBER = rgb(0xa65a00), RED = rgb(0xc43c34);   // white text reads on each
constexpr uint32_t RIGHT_MS = 600, WRONG_MS = 2000;   // how long a pick shows before the next card: a miss shows the right answer

// Hit boxes are logical (160 px across, like every game); everything is drawn at 3x.
constexpr ui::Box HOME = {68, 0, 25, 25};            // ui::back's place, the way back on every page
constexpr ui::Box GO = {47, 122, 66, 22};            // Go! (home page) and Again (done page)
constexpr ui::Box BIG_CARD = {20, 34, 120, 73};      // the card a kid taps to see the answers
constexpr ui::Box ANSWER_CARD = {20, 34, 120, 88};   // the card turned: the fact on top, the four answers under it
constexpr ui::Box DECK = {33, 41, 94, 60};           // the deck's card on the home page: a tap deals, like Go!
constexpr ui::Box TITLE = {30, 22, 100, 18};       // "Times Cards": a grown-up holds it to see how it's going
constexpr uint32_t TITLE_HOLD_MS = 2000;
// Two by two; the gap between the columns is where the finger that turned the card was (80, 70 hits none).
constexpr ui::Box CHOICE[OPTIONS] = {{26, 62, 52, 24}, {82, 62, 52, 24}, {26, 92, 52, 24}, {82, 92, 52, 24}};
constexpr Box px(const ui::Box& b) { return {b.x * 3, b.y * 3, (b.x + b.w) * 3, (b.y + b.h) * 3}; }

bool down(const Input& in, const ui::Box& b) { return in.down && ui::pressing(in, b); }
void centered(const font::Font& f, const char* s, int cx, int cy, uint16_t c) {
  font::text(f, s, cx - font::textWidth(f, s) / 2, cy - f.lineHeight / 2, c);
}
void house(bool pressed, bool back) {   // the deck leaves for the launcher (a house); every other page goes back to it
  const int cx = 240, cy = 38;
  canvas::disc(cx, cy, 30, pressed ? SHADOW : INK);
  if (back) {
    const int head[] = {cx - 16, cy, cx - 1, cy - 15, cx - 1, cy + 15};
    canvas::poly(head, 3, WHITE);
    canvas::roundBox({cx - 3, cy - 5, cx + 15, cy + 5}, 0, WHITE);
    return;
  }
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
// "7 × 8" or "56 ÷ 8", with " =" after it on the answer; `tight` drops the spaces round the sign.
void fact(char* s, size_t n, int card, bool withAnswer, bool tight = false) {
  const bool d = isDiv(card);
  snprintf(s, n, tight ? "%d%s%d%s" : "%d %s %d%s", d ? dividendOf(card) : tableOf(card), d ? "\xC3\xB7" : "\xC3\x97",
           d ? divisorOf(card) : factorOf(card), withAnswer ? " =" : "");
}
}  // namespace

const gfx::Sprite& Game::icon() const { return LAUNCHER_ICON; }

void Game::enter(const AppEnter& e) {
  in_ = Input{};
  home_ = dirty_ = keep_ = false;
  ms_ = e.ms; now_ = e.nowSec; wallet_ = e.stars;
  book_ = e.book ? e.book : &own_;
  save_ = Save{};
  bool loaded = false;
  // Only this kid's deck. One this build cannot read (a newer build's after a rollback, or damaged) starts a fresh
  // deck on screen, but opening the game must not be what wipes it from storage: nothing is written until the kid
  // answers a card on the fresh one.
  for (int i = 0; i < e.n; i++)
    if (e.all[i].id == e.who->id && e.saves[i].len) {
      loaded = loadBlob(e.saves[i].data, e.saves[i].len, save_);
      dirty_ = loaded && storedVersion(e.saves[i].data, e.saves[i].len) != SAVE_VERSION;   // an older version: stored anew, packed
      keep_ = !loaded;
    }
  if (!loaded || !save_.level) { setUp(save_, startTable(e.who->age), half()); dirty_ = true; }
  // The engine's times facts start where this deck stands: its level is known, so no calibration (os/skills.h).
  skills::seed(*book_, skills::TIMES_FACTS, save_.level, true);
  clampDue(save_, half());
  startSession(save_);
  round_ = Round{};
  picked_ = -1;
  go(P_HOME);
}

// A new page ignores touches for a moment. The answer page ignores only the finger that turned the card, by place, so
// a double tap on the card acts once but a pick right after the answers show still counts.
void Game::go(Page p) {
  page_ = p;
  frames_.reset();
  if (p == P_ANSWER) gate_.shownAt(ms_, in_.x, in_.y);
  else gate_.shown(ms_);
  if (p == P_ASK) askedMs_ = ms_;
}

void Game::deal() {
  build(round_, save_, half());
  pay_ = skills::start(*book_, now_);
  go(round_.done() ? P_DONE : P_ASK);
}

// The card turns: four answers, in an order the kid can't learn.
void Game::turn() {
  face_ = round_.current(); faceNumber_ = round_.number(); faceRedo_ = round_.redo[round_.at];
  right_ = choices(face_, ms_ * 2654435761u ^ save_.answers, options_);
  picked_ = -1;
  go(P_ANSWER);
}

// Telemetry (os/telemetry.h): every answer, and each finished round. Nothing leaves a board where it is off.
void Game::tell(int card, Grade g, bool retry, uint32_t ms) const {
  static const char* const GRADES[] = {"easy", "hard", "wrong"};
  char fact[24];
  if (isDiv(card)) snprintf(fact, sizeof fact, "%d:%d", dividendOf(card), divisorOf(card));
  else snprintf(fact, sizeof fact, "%dx%d", tableOf(card), factorOf(card));
  using telemetry::P;
  telemetry::emit("times_cards_answer", {P("fact", fact), P("grade", GRADES[g]), P("correct", (long)(g != WRONG)),
                                         P("ms", (long)ms), P("box", (long)save_.boxOf(card)), P("retry", (long)retry)});
  if (!round_.done()) return;
  telemetry::emit("level_finished", {P("game", name()), P("what", "round"), P("easy", (long)round_.easy),
                                     P("hard", (long)round_.hard), P("wrong", (long)round_.wrong),
                                     P("level_up", (long)round_.leveledUp)});
}

// The kid picks: graded and stored at once (the time counts from the card showing), the next card waits for the
// green, or for the right answer to be seen.
void Game::pick(int i) {
  picked_ = i; pickedMs_ = ms_;
  const uint32_t ms = ms_ - askedMs_;
  const Grade g = answer(round_, save_, i == right_, ms, half());
  tell(face_, g, faceRedo_, ms);
  // The engine's rating and pay: a fact's difficulty is its table, give or take its size (x2, x5, squares are easier,
  // 6..9 pairs and divisions harder). The deck's own Easy and Hard say quick or slow.
  const int beta = (isDiv(face_) ? divisorOf(face_) - 1 : tableOf(face_) - 1) * skills::LEVEL + (int)lroundf((sizeOf(face_) - 1) * 200);
  skills::answer(*book_, pay_, {skills::TIMES_FACTS, beta, OPTIONS, READ_MS, (uint16_t)face_, g == HARD ? skills::SLOW : skills::QUICK},
                 i == right_, ms);
  dirty_ = true; keep_ = false;
  if (round_.done()) paid_ = skills::finish(*book_, pay_, now_, wallet_);
}

// The answer page: a tap picks; once picked, the next card after the green (or, after a miss, the right answer seen).
void Game::updateAnswer() {
  if (picked_ < 0) {
    for (int i = 0; i < OPTIONS; i++)
      if (ui::clicked(in_, CHOICE[i])) { pick(i); return; }
  } else if (ms_ - pickedMs_ >= (picked_ == right_ ? RIGHT_MS : WRONG_MS)) {
    go(round_.done() ? P_DONE : P_ASK);
  }
}

// A grown-up holds the deck's title for TITLE_HOLD_MS: the grading check. No tap target: not for kids.
bool Game::heldTitle() {
  const bool on = page_ == P_HOME && in_.down && in_.downX >= TITLE.x && in_.downX < TITLE.x + TITLE.w &&
                  in_.downY >= TITLE.y && in_.downY < TITLE.y + TITLE.h;
  title_.step(on, ms_);
  if (title_.progress(TITLE_HOLD_MS) < 1) return false;
  title_ = ui::Hold{};
  return true;
}

void Game::update(uint32_t nowSec, uint32_t ms, const Input& in) {
  ms_ = ms; now_ = nowSec;
  in_ = in;
  gate_.filter(in_, ms);
  if (ui::clicked(in_, HOME)) {   // from a card back to the deck; from the deck out to the launcher
    if (page_ == P_HOME) home_ = true; else go(P_HOME);
    return;
  }
  if (heldTitle()) { go(P_STATS); return; }
  switch (page_) {
    case P_HOME:
    case P_DONE:   // Go!, Again, or (home) the deck's card
      if (ui::clicked(in_, GO) || (page_ == P_HOME && ui::clicked(in_, DECK))) deal();
      break;
    case P_ASK:
      if (ui::clicked(in_, BIG_CARD)) turn();
      break;
    case P_ANSWER:
      updateAnswer();
      break;
    default: break;   // P_STATS: only the house, back to the deck
  }
}

// A dot per fact of the kid's table: green known, amber learning, pale new.
static void dots(const Save& s, int cx, int y) {
  for (int n = 1; n <= FACTS; n++) {
    const uint8_t b = s.boxOf(cardOf(s.level, n));
    canvas::disc(cx + (n * 2 - FACTS - 1) * 12, y, 9, b >= MASTER_BOX ? GREEN : b ? AMBER : RULE);
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
  button(GO, "Go!", GREEN, goDown, FONT40);
}

// The grown-ups' page: how the deck reads the kid (games/times-cards/AGENTS.md says how to read it). Plain numbers,
// nothing to press but the house.
static void statRow(const char* label, const char* value, int y, uint16_t c) {
  font::text(FONT28, label, 64, y - FONT28.lineHeight / 2, SOFT);
  font::text(FONT28, value, 416 - font::textWidth(FONT28, value), y - FONT28.lineHeight / 2, c);
}
static void tenths(char* t, size_t n, uint32_t ms) { snprintf(t, n, "%u.%u s", (unsigned)(ms / 1000), (unsigned)(ms / 100 % 10)); }
static void drawStats(const Save& s) {
  char t[40];
  centered(FONT40, "How it's going", 240, 100, INK);
  snprintf(t, sizeof t, "Step %+d, %u jump%s", s.step, (unsigned)s.jumps, s.jumps == 1 ? "" : "s");
  centered(FONT28, t, 240, 146, SOFT);
  canvas::roundBox({48, 172, 432, 366}, 20, CARD);   // the numbers on a plain card, where every color reads
  snprintf(t, sizeof t, "%u of %u", (unsigned)s.rights, (unsigned)s.tries);
  statRow("Right", t, 196, INK);
  tenths(t, sizeof t, usualMs(s));
  statRow("Usual time", t, 244, INK);
  tenths(t, sizeof t, fastMs(s, cardOf(3, 4)));
  statRow("Fast under", t, 292, INK);
  snprintf(t, sizeof t, "%d", learning(s));
  statRow("Learning now", t, 340, INK);
}

void Game::drawDone() const {
  char s[24];
  char t[32];
  if (round_.leveledUp) snprintf(t, sizeof t, "Next up: %d \xC3\x97!", save_.level);
  const char* title = round_.leveledUp ? t : !round_.wrong ? "Perfect round!"
                      : round_.wrong * 2 > round_.base ? "Nice try!" : "Great job!";
  centered(FONT40, title, 240, 100, INK);
  indexCard({80, 140, 400, 304});
  if (paid_) snprintf(s, sizeof s, paid_ == 1 ? "+1 star" : "+%d stars", paid_);
  else snprintf(s, sizeof s, "Almost a star!");
  centered(FONT28, s, 240, 154, AMBER);   // above the margin line, like a card's heading
  const int counts[2] = {round_.easy + round_.hard, round_.wrong};   // right, fast or slow; and missed
  const char* const word[2] = {"Right", "Missed"};
  for (int g = 0; g < 2; g++) {
    const int x = 184 + g * 112;
    canvas::disc(x, 218, 32, g ? RED : GREEN);
    snprintf(s, sizeof s, "%d", counts[g]);
    centered(FONT40, s, x, 218, WHITE);
    centered(FONT28, word[g], x, 272, SOFT);
  }
  snprintf(s, sizeof s, "%d \xC3\x97", save_.level);   // where the table stands now
  centered(FONT28, s, 108, 336, INK);
  dots(save_, 264, 336);
  button(GO, "Again", GREEN, down(in_, GO), FONT40);
}

void Game::draw() const {
  canvas::roundBox(canvas::FULL, 0, DESK);
  house(down(in_, HOME), page_ != P_HOME);
  char s[24];
  if (page_ == P_HOME) { drawHome(save_, down(in_, GO)); return; }
  if (page_ == P_STATS) { drawStats(save_); return; }
  if (page_ == P_DONE) { drawDone(); return; }
  if (page_ == P_ANSWER) { drawAnswer(); return; }
  snprintf(s, sizeof s, "%d of %d", round_.number(), round_.base);   // a card come back adds nothing to count
  centered(FONT28, round_.redo[round_.at] ? "One more try" : s, 240, 84, SOFT);
  const int dy = down(in_, BIG_CARD) ? 6 : 0;   // the card sinks under a finger
  indexCard(px(BIG_CARD), dy);
  const int c = round_.current();
  fact(s, sizeof s, c, false);
  if (font::textWidth(cardsbig::FONT110, s) > 330) fact(s, sizeof s, c, false, true);   // 100 ÷ 10: fits the card
  centered(cardsbig::FONT110, s, 240, 232 + dy, INK);
  centered(FONT28, "Say it out loud, then tap!", 240, 352, SOFT);
}

// The card turned: the fact, and four answers to pick from. Once picked, the right one turns green, and a wrong pick red.
void Game::drawAnswer() const {
  char s[24];
  snprintf(s, sizeof s, "%d of %d", faceNumber_, round_.base);
  centered(FONT28, faceRedo_ ? "One more try" : s, 240, 84, SOFT);
  indexCard(px(ANSWER_CARD));
  fact(s, sizeof s, face_, true);
  centered(FONT40, s, 240, 146, INK);
  for (int i = 0; i < OPTIONS; i++) {
    snprintf(s, sizeof s, "%d", options_[i]);
    const uint16_t c = picked_ < 0 ? INK : i == right_ ? GREEN : i == picked_ ? RED : SHADOW;
    button(CHOICE[i], s, c, picked_ < 0 && down(in_, CHOICE[i]), FONT40);
  }
}

void Game::render() {
  // Repaint only when what the page shows changed: the page, its card, the round's place and a button under the finger.
  struct Look { uint8_t page, card, level, at, n, held; int8_t picked; } look = {page_, 0, save_.level, (uint8_t)round_.at,
                                                                                (uint8_t)round_.n, 0, (int8_t)picked_};
  if (page_ == P_ASK) look.card = (uint8_t)round_.current();
  if (page_ == P_ANSWER) look.card = (uint8_t)face_;
  const ui::Box* hot[] = {&HOME, &GO, &CHOICE[0], &CHOICE[1], &CHOICE[2], &CHOICE[3], &BIG_CARD};   // what each page draws pressed
  const bool on[P_COUNT][7] = {{1, 1, 0, 0, 0, 0, 0}, {1, 0, 0, 0, 0, 0, 1}, {1, 0, 1, 1, 1, 1, 0}, {1, 1, 0, 0, 0, 0, 0},
                               {1, 0, 0, 0, 0, 0, 0}};
  for (int i = 0; i < 7; i++) if (on[page_][i] && in_.down && ui::pressing(in_, *hot[i])) look.held = (uint8_t)(i + 1);
  frames_.render(nullptr, 0, os::crc32(&look, sizeof look), [&] { draw(); });
}

bool Game::takeSave(const void** data, size_t* len, bool allowed) {
  if (!allowed || !dirty_ || keep_) return false;
  seal(save_);
  // Packed, unless the blob does not read back as exactly this deck (no deck the game makes: then whole, as version
  // 5). The shell copies it out before the next update.
  *data = packed_; *len = pack(save_, packed_, sizeof packed_);
  if (!*len || !loadBlob(packed_, *len, check_) || memcmp(&check_, &save_, sizeof save_)) { *data = &save_; *len = sizeof save_; }
  dirty_ = false;
  return true;
}

const char* Game::screenName() const {
  static const char* N[] = {"cards_home", "cards_ask", "cards_answer", "cards_done", "cards_stats"};
  static_assert(sizeof N / sizeof *N == P_COUNT, "a name per page");
  return N[page_];
}

void Game::debugPrint() {
  const bool onCard = page_ == P_ASK || page_ == P_ANSWER;
  const int c = page_ == P_ANSWER ? face_ : onCard ? round_.current() : 0;
  printf("[cards page=%s level=%d start=%d known=%d answers=%lu at=%d n=%d base=%d card=%d%c%d box=%d due=%d easy=%d "
         "hard=%d wrong=%d leveled=%d jumped=%d stars=%lu learning=%d step=%d streak=%u jumps=%u usual=%lu fast=%lu "
         "right=%d picked=%d tries=%u rights=%u]\n",
         screenName(), save_.level, save_.start, known(save_, save_.level), (unsigned long)save_.answers, round_.at,
         round_.n, round_.base, onCard ? (isDiv(c) ? dividendOf(c) : tableOf(c)) : 0, isDiv(c) ? '/' : 'x',
         onCard ? (isDiv(c) ? divisorOf(c) : factorOf(c)) : 0, onCard ? save_.boxOf(c) : 0,
         onCard ? (int)(save_.dueOf(c) - half()) : 0, round_.easy, round_.hard, round_.wrong, round_.leveledUp,
         round_.jumped, (unsigned long)stars::balance(wallet_), learning(save_), save_.step, save_.streak, save_.jumps,
         (unsigned long)usualMs(save_), (unsigned long)fastMs(save_, c), page_ == P_ANSWER ? right_ : -1, picked_,
         save_.tries, save_.rights);
  printf("screen=%s\n", screenName());
}

void Game::debugCmd(const char* cmd) {
  if (!strncmp(cmd, "card", 4)) {   // a round of one card, for screenshots: card<N>, N as in deck.h (0..ALL-1)
    const int c = atoi(cmd + 4);
    if (c < 0 || c >= ALL) return;
    round_ = Round{};
    round_.card[0] = (uint8_t)c;
    round_.n = round_.base = round_.dealt = 1;
    round_.used[c] = true;
    go(P_ASK);
    return;
  }
  const bool right = !strcmp(cmd, "right");   // the right answer (or, "wrong", the next one), as if tapped
  if ((right || !strcmp(cmd, "wrong")) && page_ == P_ANSWER && picked_ < 0) { pick(right ? right_ : (right_ + 1) % OPTIONS); return; }
  if (strncmp(cmd, "level", 5)) return;
  const int t = atoi(cmd + 5);
  if (t < 1 || t > TABLES) return;
  setUp(save_, t, half());
  dirty_ = true; keep_ = false;
  round_ = Round{};
  go(P_HOME);
}
}  // namespace cards
