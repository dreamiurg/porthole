#include "reader.h"
#include <ctype.h>
#include <string.h>

namespace kingdom {
namespace {
bool sentenceEnd(const char* text, int pos) {
  while (pos > 0 && (text[pos - 1] == '"' || text[pos - 1] == '\'')) pos--;
  return pos > 0 && strchr(".!?", text[pos - 1]);
}
int skipSpace(const char* text, int pos) {
  while (text[pos] && isspace((unsigned char)text[pos])) pos++;
  return pos;
}
} // namespace

// Prefer a complete sentence or paragraph when it fills at least half a screen.
// A very long sentence can flow across screens; no words or paragraphs disappear.
int Reader::fit(int start, const font::Box& box) {
  int end = start, last = start, natural = start;
  while (text_[end]) {
    int candidate = skipSpace(text_, end);
    while (text_[candidate] && !isspace((unsigned char)text_[candidate])) candidate++;
    const int n = candidate - start;
    if (n >= PAGE_BYTES) break;
    memcpy(scratch_, text_ + start, n); scratch_[n] = 0;
    const int height = font::textHeight(*box.font, scratch_, box.w, box.spacing);
    if (height > box.h) break;
    last = candidate;
    const bool paragraph = text_[candidate] == '\n';
    if ((paragraph || sentenceEnd(text_, candidate)) && height >= box.h / 2) natural = candidate;
    end = candidate;
  }
  if (!text_[end]) return last;
  return natural > start ? natural : last;
}

bool Reader::open(const char* text, const font::Box& box, const font::Box* firstBox) {
  text_ = text ? text : ""; count_ = 0;
  if (!text || !box.font || (firstBox && !firstBox->font) || strlen(text) > 65500) { count_ = 1; return false; }
  int start = skipSpace(text_, 0);
  do {
    if (count_ == MAX_PAGES) return false;
    const int end = fit(start, !count_ && firstBox ? *firstBox : box);
    if (end == start && text_[start]) return false;
    starts_[count_] = (uint16_t)start; ends_[count_++] = (uint16_t)end;
    start = skipSpace(text_, end);
  } while (text_[start]);
  return true;
}

const char* Reader::page(int index) {
  if (index < 0) index = 0;
  if (index >= count_) index = count_ - 1;
  const int n = ends_[index] - starts_[index];
  memcpy(scratch_, text_ + starts_[index], n); scratch_[n] = 0;
  return scratch_;
}
} // namespace kingdom
