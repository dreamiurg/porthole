// Pets Club's two bookshelves (the design page "Книжная полка на двух языках", approved 2026-11-22): English books
// (content.h BOOKS, levels 2..8) and Ukrainian ones (content_books_uk.h UK_BOOKS, levels 1..5), each language's own
// shelf picked for the kid's reading level in the shared skills engine (os/skills.h: READ_EN, which Little reads also
// move, and READ_UK). The question at a book's end is one answer to the engine, weighted by the book's length; a book
// paged through faster than anyone reads counts as read but rates nothing. Pure logic, no rendering;
// host/test_pets_books.cpp exercises it.
#pragma once
#include <initializer_list>
#include <stdint.h>
#include "content.h"
#include "content_books_uk.h"
#include "font.h"
#include "pet.h"
#include "skills.h"
#include "uk_fonts.h"

static const int NUM_UK_BOOKS = (int)(sizeof(UK_BOOKS) / sizeof(UK_BOOKS[0]));
static_assert(NUM_BOOKS <= 64 && NUM_UK_BOOKS <= 64, "Save::booksEn/booksUk are 64 bits: widen before adding more books");

namespace books {
enum Lang : uint8_t { EN = 0, UK = 1 };
constexpr int SHELF_MAX = 64;           // covers on one shelf: every book of the language (Save keeps 64 bits)
constexpr int LEVELS[2] = {8, 5};       // English 1..8 (Little reads are 1..5), Ukrainian 1..5
constexpr uint32_t MS_PER_WORD = 150;   // 400 words a minute: paging faster is not reading
constexpr int WORDS_PER_ITEM = 50;      // a book's answer pays one Little read's worth per 50 words, up to MAX_WEIGHT
constexpr int MAX_WEIGHT = 6;

inline int count(Lang l) { return l == EN ? NUM_BOOKS : NUM_UK_BOOKS; }
inline const Book& at(Lang l, int i) { return l == EN ? BOOKS[i] : UK_BOOKS[i]; }
inline skills::Skill skill(Lang l) { return l == EN ? skills::READ_EN : skills::READ_UK; }
inline const uint8_t* bits(const Save& s, Lang l) { return l == EN ? s.booksEn : s.booksUk; }
inline bool read(const Save& s, Lang l, int i) { return bits(s, l)[i / 8] >> (i % 8) & 1; }
inline void markRead(Save& s, Lang l, int i) {
  uint8_t* m = l == EN ? s.booksEn : s.booksUk;
  const unsigned u = (unsigned)i;
  m[u >> 3] = (uint8_t)(m[u >> 3] | 1u << (u & 7));
}
inline int pages(const Book& b) { int n = 0; while (n < 11 && b.pages[n]) n++; return n; }
inline int answers(const Book& b) { int n = 0; while (n < 3 && b.answers[n]) n++; return n; }
// Words: runs of non-space bytes holding a letter (any byte >= 0x80 is part of a Ukrainian letter).
inline int words(const Book& b) {
  int n = 0;
  for (int p = 0; p < pages(b); p++) {
    bool in = false, letter = false;
    for (const char* c = b.pages[p];; c++) {
      if (!*c || *c == ' ') { n += in && letter; in = letter = false; if (!*c) break; continue; }
      in = true;
      letter |= (*c >= 'A' && *c <= 'Z') || (*c >= 'a' && *c <= 'z') || (uint8_t)*c >= 0x80;
    }
  }
  return n;
}

// The engine's reading level for this kid in l, 1..LEVELS[l]: about TARGET of the questions at it come out right.
// Seeds the skill first: English where Little reads stand (known), Ukrainian from the first level (calibrating).
inline int target(skills::Book& b, const Save& s, Lang l) {
  if (l == EN) skills::seed(b, skills::READ_EN, pet::readLevel(s) - 1, true);
  else skills::seed(b, skills::READ_UK, 0, false);
  return skills::target(b, skill(l), LEVELS[l], 3) + 1;
}
// How hard a book is for this kid, as paws on its cover: 1 easy, 2 just right, 3 a stretch.
inline int paws(int level, int t) { return level < t ? 1 : level == t ? 2 : 3; }

namespace detail {
inline bool has(const uint8_t* out, int n, int i) { for (int k = 0; k < n; k++) if (out[k] == i) return true; return false; }
// The first unread book at level, not on the shelf yet; -1 when there is none.
inline int firstUnread(const Save& s, Lang l, int level, const uint8_t* out, int n) {
  for (int i = 0; i < count(l); i++) if (at(l, i).level == level && !read(s, l, i) && !has(out, n, i)) return i;
  return -1;
}
// Books already read, from one level above t to two below, the newest first, until the shelf is full.
inline int addRereads(const Save& s, Lang l, int t, uint8_t* out, int n) {
  for (int level = t + 1; level >= t - 2 && n < SHELF_MAX; level--)
    for (int i = count(l) - 1; i >= 0 && n < SHELF_MAX; i--)
      if (at(l, i).level == level && read(s, l, i) && !has(out, n, i)) out[n++] = (uint8_t)i;
  return n;
}
// Every book not on the shelf yet: the read ones, newest first, then the unread ones, nearest level t first.
inline int addRest(const Save& s, Lang l, int t, uint8_t* out, int n) {
  for (int i = count(l) - 1; i >= 0; i--) if (read(s, l, i) && !has(out, n, i)) out[n++] = (uint8_t)i;
  for (int d = 0; d < LEVELS[l]; d++)
    for (int level : {t - d, t + d}) {
      if (d == 0 && level != t) continue;
      for (int i = 0; i < count(l); i++) if (at(l, i).level == level && !has(out, n, i)) out[n++] = (uint8_t)i;
    }
  return n;
}
}  // namespace detail

// The shelf for level t: the next unread book just right (or the nearest unread level, above first), one unread a
// stretch above it and one unread easier, then books already read from two below to one above, to read again, then
// every other book, so no book ever leaves the shelf (Дмитро, 2026-11-24: a kid's old favorites must stay): the rest
// already read, newest first, then the rest unread, nearest the level first. Returns how many indices went into out,
// the new ones first: every book of the language.
inline int shelf(const Save& s, Lang l, int t, uint8_t* out) {
  int n = 0;
  int right = detail::firstUnread(s, l, t, out, n);
  for (int d = 1; right < 0 && d < LEVELS[l]; d++) {
    right = detail::firstUnread(s, l, t + d, out, n);
    if (right < 0) right = detail::firstUnread(s, l, t - d, out, n);
  }
  if (right >= 0) out[n++] = (uint8_t)right;
  const int lv = right >= 0 ? at(l, right).level : t;
  for (int level : {lv + 1, lv - 1}) { const int i = detail::firstUnread(s, l, level, out, n); if (i >= 0) out[n++] = (uint8_t)i; }
  n = detail::addRereads(s, l, t, out, n);
  return detail::addRest(s, l, t, out, n);
}

// Paged through in ms (from its first page to its question): fast enough that it was not read.
inline bool skimmed(const Book& b, uint32_t ms) { return ms < (uint32_t)words(b) * MS_PER_WORD; }
// The question's answer, for skills::answer: the book's level, its answers to guess from, its length as weight; the
// skim check is the reader's, so no read time here. key: which book (a reread the same day pays less).
inline skills::Item item(Lang l, int i) {
  const Book& b = at(l, i);
  int w = words(b) / WORDS_PER_ITEM; w = w < 1 ? 1 : w > MAX_WEIGHT ? MAX_WEIGHT : w;
  return {skill(l), (b.level - 1) * skills::LEVEL, answers(b), 0, (uint16_t)(l << 8 | i), skills::QUICK, (uint8_t)w};
}

// A Ukrainian book is drawn in Andika (os/uk_fonts.h) at native size: its boxes, in physical px. The reader
// (game.cpp, ukrainian.cpp) lays out by these, tools/check_books_uk.py holds the stories to them and
// host/test_pets_books.cpp measures every string against them with os/font.cpp.
namespace ukbox {
constexpr int PAGE_W = 366, TITLE_W = 324, COVER_W = 330, QUESTION_W = 348, ANSWER_W = 306;
constexpr int COVER_LINES = 2, QUESTION_LINES = 3, ANSWER_LINES = 2;
inline const font::Font& pageFont(const Book& b) { return b.level <= 2 ? ukfont::FONT48 : ukfont::FONT24; }
inline int pageLines(const Book& b) { return b.level <= 2 ? 4 : 8; }
inline int lines(const font::Font& f, const char* s, int w) { uint16_t st[16]; return font::wrap(f, s, w, st, 16); }
}  // namespace ukbox
}  // namespace books
