#include "host/check.h"
#include <stdio.h>
#include <string.h>
#include "games/living-kingdom/reader.h"

// A deliberately wide test font makes even a short paragraph span several screens.
static const uint8_t BITMAP[] = {255};
static font::Glyph glyphs[96];
static const uint8_t KERN[96] = {};
static const int8_t VALUES[] = {0};
static font::Font face = {BITMAP, glyphs, nullptr, 0, KERN, KERN, VALUES, 0, 20, 0};

static void illustratedFirstPage() {
  const font::Box regular = {&face, 120, 80, 0, font::Align::LEFT};
  const font::Box first = {&face, 120, 40, 0, font::Align::LEFT};
  const char* text = "A cart lost its wheel. Lucas kept the axle.\n\nDribble wanted to roast the wheel. Serena wanted to find it first. Everyone had a useful idea.";
  kingdom::Reader ordinary, illustrated;
  assert(ordinary.open(text, regular) && illustrated.open(text, regular, &first));
  assert(strlen(illustrated.page(0)) < strlen(ordinary.page(0)));
  assert(illustrated.count() >= ordinary.count()); size_t consumed = 0;
  for (int i = 0; i < illustrated.count(); i++) {
    const char* page = illustrated.page(i); const size_t length = strlen(page);
    assert(font::textHeight(face, page, regular.w, 0) <= (i ? regular.h : first.h));
    while (text[consumed] == ' ' || text[consumed] == '\n') consumed++;
    assert(!strncmp(text + consumed, page, length)); consumed += length;
  }
  assert(consumed == strlen(text));
  char firstPage[kingdom::Reader::PAGE_BYTES]; snprintf(firstPage, sizeof firstPage, "%s", illustrated.page(0));
  illustrated.page(illustrated.count() - 1); assert(!strcmp(firstPage, illustrated.page(0)));
  const font::Box invalid = {nullptr, 120, 40, 0, font::Align::LEFT};
  assert(!illustrated.open(text, regular, &invalid));
}

int main() {
  for (auto& g : glyphs) g.advW = 10 * 16;
  const font::Box box = {&face, 120, 80, 0, font::Align::LEFT};
  kingdom::Reader reader;
  const char* text = "A cart lost its wheel. Lucas kept the axle.\n\nDribble wanted to roast the wheel. Serena wanted to find it first. Everyone had a useful idea.";
  assert(reader.open(text, box));
  assert(reader.count() > 2);
  size_t consumed = 0;
  for (int i = 0; i < reader.count(); i++) {
    const char* page = reader.page(i);
    assert(font::textHeight(face, page, box.w, box.spacing) <= box.h);
    while (text[consumed] == ' ' || text[consumed] == '\n') consumed++;
    assert(!strncmp(text + consumed, page, strlen(page)));
    consumed += strlen(page);
  }
  assert(consumed == strlen(text));
  char first[kingdom::Reader::PAGE_BYTES];
  snprintf(first, sizeof first, "%s", reader.page(0));
  reader.page(reader.count() - 1);
  assert(!strcmp(first, reader.page(0))); // backwards and rereading are stable
  assert(!strcmp(reader.page(-1), first));
  assert(reader.open("", box) && reader.count() == 1 && !*reader.page(0));
  assert(reader.open("One.\n\nTwo.\n\nThree.", box));
  assert(strstr(reader.page(0), "\n\n")); // paragraph breaks survive
  assert(!reader.open(nullptr, box));
  illustratedFirstPage();
  puts("kingdom reader: stable, complete, paragraph-aware pagination passed");
}
