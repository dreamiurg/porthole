// Discoveries (today's three, a topic's eight, or the notebook of the ones kept), Topics, a discovery (its cover, its
// pages, the wonder question) and its source; Today's adventure and its pocket word. Keeping a discovery counts as
// today's reading and stays in the notebook for good.
#include <stdio.h>
#include "content_discoveries.h"
#include "game.h"
#include "generated/scenes.h"
#include "gfx565.h"
#include "personalize.h"
#include "ui565.h"

namespace biscuit {
using namespace ui565;
namespace {
constexpr int FACT_ROWS = 2, TOPIC_PAGES = (TOPIC_COUNT + ROWS - 1) / ROWS;
constexpr Button TOPICS_BUTTON = {PAIR[0], "Topics", &FONT20, PURPLE, true};
constexpr Button THREE = {PAIR[1], "Today's three", &FONT20, PEACH, true};
constexpr Button FIND_OUT = {WIDE, "Let's find out", &FONT20, PURPLE, true};
constexpr Button SOURCE = {PREV, "Source", &FONT20, PURPLE, true}, KEEP = {NEXT, "Keep", &FONT20, SAGE, true};
constexpr Button BACK_TO_BOOK = {WIDE, "Back to our book", &FONT20, PURPLE, true};
constexpr Button WORD = {TODAY[3], "A lovely word", &FONT20, PEACH, true};
constexpr Button BACK_TO_TODAY = {WIDE, "Back to today", &FONT20, PURPLE, true};
constexpr Nav PAGES = {"Previous", "Next", true, true};   // a discovery's first Previous is its cover, its last Next the wonder
int mod(int n, int d) { return (n % d + d) % d; }
int nth(int topic, int n) {   // a topic's n-th discovery
  for (int i = 0; i < NUM_DISCOVERIES; i++) if (DISCOVERIES[i].topic == topic && n-- == 0) return i;
  return 0;
}
const Adventure& today(const Save& p) { return ADVENTURES[cycle(p.lastVisitDay, ADVENTURE_COUNT)]; }
// Today's activities in Action bit order, three a day, each marked once done.
Button activity(const Save& p, int slot, int bit, char (&label)[32]) {
  const bool done = p.dailyCompleted & (1u << bit);
  snprintf(label, sizeof label, "%s%s", done ? "* " : "", ACTIVITIES[bit]);
  return {TODAY[slot], label, &FONT20, done ? SAGE : PURPLE, true};
}
}  // namespace

// ---------------------------------------------------------------- Discoveries and Topics
void Game::showFacts(FactList which, int topic) { facts_ = which; topic_ = topic; page_ = 0; }
int Game::factList(uint8_t* ids) const {
  int n = 0;
  if (facts_ == TODAYS_THREE) {   // one from each third of the topics, three new ones every day (the old firmware's)
    const int day = save_.lastVisitDay;
    for (int family = 0; family < 3; family++) ids[n++] = (uint8_t)nth(family * 4 + mod(day, 4), mod(day / 4, 8));
    return n;
  }
  for (int i = 0; i < NUM_DISCOVERIES; i++)
    if (facts_ == TOPIC ? DISCOVERIES[i].topic == topic_ : discovered(save_, (uint8_t)i)) ids[n++] = (uint8_t)i;
  return n;
}
void Game::updateDiscoveries() {
  if (tapped(in_, BACK_BUTTON)) {   // a topic's list goes back to the topics, the others to the Library
    if (facts_ == TOPIC) { page_ = topic_ / ROWS; go(SC_TOPICS); } else { page_ = 0; go(SC_LIBRARY); }
    return;
  }
  if (tapped(in_, TOPICS_BUTTON)) {   // Topics' Back comes back to this list (a topic's list goes back to Topics)
    if (facts_ != TOPIC) topicsFrom_ = facts_;
    page_ = 0; go(SC_TOPICS);
    return;
  }
  if (tapped(in_, THREE)) { showFacts(TODAYS_THREE, 0); fresh(); return; }   // the rows change under the finger
  uint8_t ids[NUM_DISCOVERIES];
  const int n = factList(ids), pages = n ? (n + FACT_ROWS - 1) / FACT_ROWS : 1;
  const int turn = navTapped(in_, page_, pages);
  if (turn) { page_ += turn; return; }
  for (int row = 0; row < FACT_ROWS; row++) {
    const int i = page_ * FACT_ROWS + row;
    const Box& b = ROW[row + 1];
    if (i < n && in_.tapIn(b.x, b.y, b.w, b.h)) { openFact(ids[i]); return; }
  }
}
void Game::drawDiscoveries() {
  gfx565::clear(PAPER);
  top(in_, facts_ == NOTEBOOK ? "Our little notebook" : facts_ == TOPIC ? TOPICS[topic_].name : "Little discoveries", stars(save_));
  button(in_, TOPICS_BUTTON);
  button(in_, THREE);
  uint8_t ids[NUM_DISCOVERIES];
  const int n = factList(ids), pages = n ? (n + FACT_ROWS - 1) / FACT_ROWS : 1;
  if (!n) text(NOTEBOOK_EMPTY, "A place for all the things we find together.", INK);
  for (int row = 0; row < FACT_ROWS; row++) {
    const int i = page_ * FACT_ROWS + row;
    if (i < n) pictureRow(in_, ROW[row + 1], ids[i], DISCOVERIES[ids[i]].title);
  }
  nav(in_, page_, pages);
}
void Game::updateTopics() {
  if (tapped(in_, BACK_BUTTON)) { showFacts(topicsFrom_, 0); go(SC_DISCOVERIES); return; }
  const int turn = navTapped(in_, page_, TOPIC_PAGES);
  if (turn) { page_ += turn; return; }
  for (int row = 0; row < ROWS; row++) {
    const int t = page_ * ROWS + row;
    if (t < TOPIC_COUNT && in_.tapIn(ROW[row].x, ROW[row].y, ROW[row].w, ROW[row].h)) { showFacts(TOPIC, t); go(SC_DISCOVERIES); return; }
  }
}
void Game::drawTopics() {
  gfx565::clear(PAPER);
  top(in_, "So much to explore", stars(save_));
  for (int row = 0; row < ROWS; row++) {
    const int t = page_ * ROWS + row;
    if (t < TOPIC_COUNT) pictureRow(in_, ROW[row], nth(t, 0), TOPICS[t].name);
  }
  nav(in_, page_, TOPIC_PAGES);
}

// ---------------------------------------------------------------- a discovery: cover, pages, wonder; its source
void Game::openFact(int id) {
  if (id < 0 || id >= NUM_DISCOVERIES || !awake(save_)) return;
  fact_ = id;
  openPages(DISCOVERIES[id].pages, 2, 0);
  at_ = -1;   // the cover first
  go(SC_DISCOVERY);
}
// The cover, the pages and the wonder page each put other buttons where the last one was: every step waits for the
// fresh-screen pause, so a double tap on Next never keeps the discovery by itself.
void Game::updateDiscovery() {
  if (tapped(in_, BACK_BUTTON)) { go(SC_DISCOVERIES); return; }   // the list as it was
  if (at_ < 0) { if (tapped(in_, FIND_OUT)) { at_ = 0; fresh(); } return; }
  if (at_ >= total_) {
    if (tapped(in_, SOURCE)) { go(SC_SOURCE); return; }
    if (!tapped(in_, KEEP)) return;
    discover(save_, (uint8_t)fact_, now_); markDirty();
    showFacts(NOTEBOOK, 0);
    uint8_t ids[NUM_DISCOVERIES];
    const int n = factList(ids);
    for (int i = 0; i < n; i++) if (ids[i] == fact_) page_ = i / FACT_ROWS;   // the notebook page it is on
    go(SC_DISCOVERIES);
    return;
  }
  const int turn = navTapped(in_, PAGES);
  if (turn) { at_ += turn; fresh(); }
}
void Game::drawDiscovery() {
  const Discovery& d = DISCOVERIES[fact_];
  gfx565::clear(PAPER);
  top(in_, TOPICS[d.topic].name, stars(save_));
  if (at_ < 0) {
    text(coverTitle(d.title), d.title, INK);
    picture(fact_, COVER_X, COVER_Y, 3);
    button(in_, FIND_OUT);
  } else if (at_ < total_) {
    text(PAGE, pageText(), INK);
    nav(in_, PAGES, at_, total_);
  } else {
    char s[256];
    personalize(d.wonder, who_.name, save_.petName, s, sizeof s);
    text(WONDER_TITLE, "I wonder...", INK);
    text(WONDER, s, INK);
    button(in_, SOURCE);
    button(in_, KEEP);
  }
}
void Game::updateSource() {
  const bool back = tapped(in_, BACK_BUTTON), book = tapped(in_, BACK_TO_BOOK);   // both asked: the audit sees both
  if (back || book) go(SC_DISCOVERY);   // the wonder page again
}
void Game::drawSource() {
  const Discovery& d = DISCOVERIES[fact_];
  gfx565::clear(PAPER);
  top(in_, "Where we found it", stars(save_));
  text(SOURCE_NAME, d.sourceName, INK);
  text(SOURCE_URL, d.sourceUrl, INK);
  button(in_, BACK_TO_BOOK);
}

// ---------------------------------------------------------------- Today and its pocket word
void Game::updateToday() {
  if (tapped(in_, BACK_BUTTON)) { go(SC_WORLD); return; }
  if (tapped(in_, WORD)) { go(SC_WORD); return; }
  const uint8_t mask = dailyMask(save_.lastVisitDay);
  for (int bit = 0, slot = 0; bit < 6 && slot < 3; bit++) {
    if (!(mask & (1u << bit))) continue;
    char label[32];
    if (tapped(in_, activity(save_, slot++, bit, label))) { todayActivity(bit); return; }
  }
}
// The activity itself (a snack, fetch, a cuddle, a nap) or the place it happens (the Library, the tricks), as the
// old firmware did. Doing it is what counts toward today; this screen only marks it.
void Game::todayActivity(int bit) {
  const Action a = (Action)(1u << bit);
  if (a == Action::Rest) { toggleNap(); return; }
  if (!awake(save_)) { go(SC_HOME); return; }   // nothing but waking while the pup naps
  if (a == Action::Read) { openLibrary(SC_TODAY); return; }
  if (a == Action::Train) { openTricks(SC_TODAY); return; }
  go(SC_HOME);
  if (a == Action::Play) { fetch_ = 0; sayUntilMs_ = 0; animate(SCENE_PLAY); return; }
  biscuit::act(save_, a, now_); markDirty();
  say(a == Action::Feed ? SAY_FED : SAY_PETTED, a == Action::Feed ? SCENE_FEED : SCENE_PET);
}
void Game::drawToday() {
  const Adventure& a = today(save_);
  gfx565::clear(PAPER);
  top(in_, a.title, stars(save_));
  char s[256];
  personalize(a.description, who_.name, save_.petName, s, sizeof s);
  text(ADVENTURE, s, INK);
  const uint8_t mask = dailyMask(save_.lastVisitDay);
  for (int bit = 0, slot = 0; bit < 6 && slot < 3; bit++) {
    if (!(mask & (1u << bit))) continue;
    char label[32];
    button(in_, activity(save_, slot++, bit, label));
  }
  button(in_, WORD);
}
void Game::updateWord() {
  const bool back = tapped(in_, BACK_BUTTON), again = tapped(in_, BACK_TO_TODAY);
  if (back || again) go(SC_TODAY);
}
void Game::drawWord() {
  const Adventure& a = today(save_);
  gfx565::clear(PAPER);
  top(in_, a.word, stars(save_));
  char s[256];
  personalize(a.meaning, who_.name, save_.petName, s, sizeof s);
  text(MEANING, s, INK);
  button(in_, BACK_TO_TODAY);
}
}  // namespace biscuit
