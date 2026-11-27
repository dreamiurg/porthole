// Self-check for Pets Club's two bookshelves (games/pets-club/books.h): every Ukrainian string measured in Andika with
// os/font.cpp in the box the reader draws it in, the shelf for a level, the paws, the skim check, and a book's answer
// weighed by its length in the skills engine (os/skills.h).
// Run: make test
#include "host/check.h"
#include <stdio.h>
#include <string.h>
#include "font.h"
#include "games/pets-club/books.h"
#include "uk_fonts.h"

using namespace books;
static int checks = 0;

// Fits in w px and at most maxLines lines, with no word wider than the box.
static bool fits(const font::Font& f, const char* s, int w, int maxLines) {
  uint16_t st[16];
  return font::wrap(f, s, w, st, 16) <= maxLines && font::textWidth(f, s) > 0 && strlen(s) > 0 &&
         [&] { for (const char* p = s; *p;) {   // each word alone
                 char word[64]; int n = 0;
                 while (*p == ' ') p++;
                 while (*p && *p != ' ' && n < 63) word[n++] = *p++;
                 word[n] = 0;
                 if (n && font::textWidth(f, word) > w) return false;
               }
               return true; }();
}

// Every letter of s has a glyph in both Andika sizes.
static void glyphs(const char* s) {
  for (const char* c = s; *c; c++) {
    if ((uint8_t)*c < 0x80 || ((uint8_t)*c & 0xC0) == 0x80) continue;   // only a letter's lead byte
    uint32_t cp = ((uint8_t)c[0] & 0x1F) << 6 | ((uint8_t)c[1] & 0x3F);
    if (((uint8_t)*c & 0xF0) == 0xE0) cp = ((uint8_t)c[0] & 0x0F) << 12 | ((uint8_t)c[1] & 0x3F) << 6 | ((uint8_t)c[2] & 0x3F);
    CHECK(font::glyphIndex(ukfont::FONT24, cp) && font::glyphIndex(ukfont::FONT48, cp));
  }
}

static void ukrainianBook(const Book& b) {
  CHECK(b.level >= 1 && b.level <= LEVELS[UK] && pages(b) >= 3 && answers(b) >= 2 && b.correct < answers(b));
  CHECK(fits(ukfont::FONT24, b.title, ukbox::TITLE_W, 1) && fits(ukfont::FONT24, b.title, ukbox::COVER_W, ukbox::COVER_LINES));
  CHECK(fits(ukfont::FONT24, b.question, ukbox::QUESTION_W, ukbox::QUESTION_LINES));
  for (int a = 0; a < answers(b); a++) CHECK(fits(ukfont::FONT24, b.answers[a], ukbox::ANSWER_W, ukbox::ANSWER_LINES));
  for (int p = 0; p < pages(b); p++) {
    const bool ok = fits(ukbox::pageFont(b), b.pages[p], ukbox::PAGE_W, ukbox::pageLines(b));
    if (!ok) printf("too long: %s p%d\n", b.title, p + 1);
    CHECK(ok);
    glyphs(b.pages[p]);
  }
}

// Every level from lo to the language's top has books, so a shelf never waits for one.
static void everyLevel(Lang l, int lo) {
  for (int lv = lo; lv <= LEVELS[l]; lv++) { int n = 0; for (int i = 0; i < count(l); i++) n += at(l, i).level == lv; CHECK(n >= 1); }
}

static void ukrainianFits() {
  CHECK(NUM_UK_BOOKS >= 10);
  for (int i = 0; i < NUM_UK_BOOKS; i++) ukrainianBook(UK_BOOKS[i]);
  everyLevel(UK, 1); everyLevel(EN, 2);
}

// A fresh kid at every level: an unread book just right first, no book twice.
static void shelfPerLevel() {
  Save s{};
  for (Lang l : {EN, UK}) {
    for (int t = 1; t <= LEVELS[l]; t++) {
      uint8_t out[SHELF_MAX]; const int n = shelf(s, l, t, out);
      CHECK(n >= 1 && n <= SHELF_MAX && !read(s, l, out[0]));
      bool any = false; for (int i = 0; i < count(l); i++) any |= at(l, i).level == t;
      if (any) CHECK(at(l, out[0]).level == t);   // just right is the level itself when it has an unread book
      for (int k = 0; k < n; k++) for (int j = k + 1; j < n; j++) CHECK(out[k] != out[j]);
    }
  }
}

static void shelves() {
  shelfPerLevel();
  Save s{};
  // Ukrainian level 1 read through: the shelf moves on to an unread book and offers level 1 again to reread.
  for (int i = 0; i < NUM_UK_BOOKS; i++) if (UK_BOOKS[i].level == 1) markRead(s, UK, i);
  uint8_t out[SHELF_MAX]; int n = shelf(s, UK, 1, out);
  CHECK(!read(s, UK, out[0]) && UK_BOOKS[out[0]].level == 2);
  bool again = false; for (int k = 0; k < n; k++) again |= read(s, UK, out[k]) && UK_BOOKS[out[k]].level == 1;
  CHECK(again);
  // Everything read: still a shelf, of books to read again.
  for (int i = 0; i < NUM_UK_BOOKS; i++) markRead(s, UK, i);
  n = shelf(s, UK, 3, out); CHECK(n >= 1 && read(s, UK, out[0]));
  CHECK(read(s, UK, 0) && !read(s, EN, 0));   // the languages keep their own marks
  markRead(s, EN, 63); CHECK(s.booksEn[7] == 0x80);
  markRead(s, EN, 64); markRead(s, EN, 127); CHECK(s.booksEnMore[0] == 0x01 && s.booksEnMore[7] == 0x80);
  CHECK(read(s, EN, 64) && read(s, EN, 127) && !read(s, EN, 65) && !s.reserved4[0]);
  CHECK(paws(2, 3) == 1 && paws(3, 3) == 2 && paws(4, 3) == 3);
}

static int expectedWeight(const Book& b) {
  const int w = words(b) / WORDS_PER_ITEM;
  return w > MAX_WEIGHT ? MAX_WEIGHT : w < 1 ? 1 : w;
}

// The longest Ukrainian book: the skim check and its answer's item.
static void bookItem() {
  int longest = 0; for (int i = 1; i < NUM_UK_BOOKS; i++) if (words(UK_BOOKS[i]) > words(UK_BOOKS[longest])) longest = i;
  const Book& b = UK_BOOKS[longest];
  CHECK(words(b) >= 200 && skimmed(b, 1000) && !skimmed(b, words(b) * MS_PER_WORD));
  const skills::Item it = item(UK, longest);
  CHECK(it.skill == skills::READ_UK && it.beta == (b.level - 1) * skills::LEVEL && it.weight == expectedWeight(b));
  CHECK(item(UK, 0).weight == 1 && item(UK, 0).key == (1 << 8 | 0) && item(EN, 3).key == 3);
}

static void answers() {
  bookItem();
  // A weight of three pays three answers' points and counts three answers' time.
  skills::Book k1{}, k3{}; skills::forgetRepeats();
  skills::seed(k1, skills::READ_UK, 2, true); skills::seed(k3, skills::READ_UK, 2, true);
  skills::Round r1 = skills::start(k1, 20000 * 86400u), r3 = skills::start(k3, 20000 * 86400u);
  skills::Item one = {skills::READ_UK, 2 * skills::LEVEL, 3, 0, 1}, three = one; three.weight = 3;
  skills::answer(k1, r1, one, true, 100000); skills::answer(k3, r3, three, true, 100000);
  CHECK(r1.points > 0 && r3.points >= 3 * r1.points - 2 && r3.points <= 3 * r1.points + 2);   // rounding
  CHECK(r1.ms == 30000 && r3.ms == 90000);
  CHECK(skills::get(k1, skills::READ_UK).theta == skills::get(k3, skills::READ_UK).theta);   // one answer, one rating step
  // Ukrainian starts calibrating from the first level; English starts where Little reads stand.
  skills::Book kb{}; Save s{}; s.readLevel = 4;
  CHECK(target(kb, s, UK) == 1 && skills::calibrating(kb, skills::READ_UK) && target(kb, s, EN) >= 3);
}

int main() {
  ukrainianFits();
  shelves();
  answers();
  printf("pets books: %d English, %d Ukrainian; %d checks\n", NUM_BOOKS, NUM_UK_BOOKS, checks);
  return 0;
}
