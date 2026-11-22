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

static void ukrainianFits() {
  CHECK(NUM_UK_BOOKS >= 10);
  for (int i = 0; i < NUM_UK_BOOKS; i++) {
    const Book& b = UK_BOOKS[i];
    CHECK(b.level >= 1 && b.level <= LEVELS[UK] && pages(b) >= 3 && answers(b) >= 2 && b.correct < answers(b));
    CHECK(fits(ukfont::FONT24, b.title, ukbox::TITLE_W, 1) && fits(ukfont::FONT24, b.title, ukbox::COVER_W, ukbox::COVER_LINES));
    CHECK(fits(ukfont::FONT24, b.question, ukbox::QUESTION_W, ukbox::QUESTION_LINES));
    for (int a = 0; a < answers(b); a++) CHECK(fits(ukfont::FONT24, b.answers[a], ukbox::ANSWER_W, ukbox::ANSWER_LINES));
    for (int p = 0; p < pages(b); p++) {
      if (!fits(ukbox::pageFont(b), b.pages[p], ukbox::PAGE_W, ukbox::pageLines(b))) printf("too long: %s p%d\n", b.title, p + 1);
      CHECK(fits(ukbox::pageFont(b), b.pages[p], ukbox::PAGE_W, ukbox::pageLines(b)));
      for (const char* c = b.pages[p]; *c; c++) if ((uint8_t)*c >= 0x80 && ((uint8_t)*c & 0xC0) != 0x80) {   // a letter's lead byte
        uint32_t cp = ((uint8_t)c[0] & 0x1F) << 6 | ((uint8_t)c[1] & 0x3F);
        if (((uint8_t)*c & 0xF0) == 0xE0) cp = ((uint8_t)c[0] & 0x0F) << 12 | ((uint8_t)c[1] & 0x3F) << 6 | ((uint8_t)c[2] & 0x3F);
        CHECK(font::glyphIndex(ukfont::FONT24, cp) && font::glyphIndex(ukfont::FONT48, cp));
      }
    }
  }
  // No gap under the highest level with books, in both languages, so a shelf reaches for the nearest one (the top
  // levels, Ukrainian 3-5 and English 8, are still being written).
  auto noGaps = [](Lang l, int lo) {
    int top = 0; for (int i = 0; i < count(l); i++) top = at(l, i).level > top ? at(l, i).level : top;
    for (int lv = lo; lv <= top; lv++) { int n = 0; for (int i = 0; i < count(l); i++) n += at(l, i).level == lv; CHECK(n >= 1); }
  };
  noGaps(UK, 1); noGaps(EN, 2);
}

static void shelves() {
  Save s{};
  for (Lang l : {EN, UK}) {
    for (int t = 1; t <= LEVELS[l]; t++) {
      uint8_t out[SHELF_MAX]; const int n = shelf(s, l, t, out);
      CHECK(n >= 1 && n <= SHELF_MAX && !read(s, l, out[0]));
      // just right is the level itself when it has an unread book, and nothing on the shelf is far off
      bool any = false; for (int i = 0; i < count(l); i++) any |= at(l, i).level == t;
      if (any) CHECK(at(l, out[0]).level == t);
      for (int k = 0; k < n; k++) for (int j = k + 1; j < n; j++) CHECK(out[k] != out[j]);
    }
  }
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
  CHECK(paws(2, 3) == 1 && paws(3, 3) == 2 && paws(4, 3) == 3);
}

static void answers() {
  int longest = 0; for (int i = 1; i < NUM_UK_BOOKS; i++) if (words(UK_BOOKS[i]) > words(UK_BOOKS[longest])) longest = i;
  const Book& b = UK_BOOKS[longest];
  CHECK(words(b) >= 40 && skimmed(b, 1000) && !skimmed(b, words(b) * MS_PER_WORD));
  const skills::Item it = item(UK, longest);
  CHECK(it.skill == skills::READ_UK && it.beta == (b.level - 1) * skills::LEVEL && it.weight == (words(b) / WORDS_PER_ITEM > MAX_WEIGHT ? MAX_WEIGHT : words(b) < WORDS_PER_ITEM ? 1 : words(b) / WORDS_PER_ITEM));
  CHECK(item(UK, 0).weight == 1 && item(UK, 0).key == (1 << 8 | 0) && item(EN, 3).key == 3);
  // A weight of three pays three answers' points and counts three answers' time.
  skills::Book k1{}, k3{}; skills::forgetRepeats();
  skills::seed(k1, skills::READ_UK, 2, true); skills::seed(k3, skills::READ_UK, 2, true);
  skills::Round r1 = skills::start(k1, 20000 * 86400u), r3 = skills::start(k3, 20000 * 86400u);
  skills::Item one = {skills::READ_UK, 2 * skills::LEVEL, 3, 0, 1}, three = one; three.weight = 3;
  skills::answer(k1, r1, one, true, 100000); skills::answer(k3, r3, three, true, 100000);
  CHECK(r1.points > 0 && r3.points >= 3 * r1.points - 2 && r3.points <= 3 * r1.points + 2 && r1.ms == 30000 && r3.ms == 90000);   // rounding
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
