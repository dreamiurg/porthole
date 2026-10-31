// Stable, bounded pagination over immutable prose. No campaign effects live in the reader.
#pragma once
#include <stdint.h>
#include "font.h"

namespace kingdom {
class Reader {
 public:
  static constexpr int MAX_PAGES = 64, PAGE_BYTES = 1024;
  // A smaller first page may leave room for an illustration; later pages use the regular box.
  bool open(const char* text, const font::Box& box, const font::Box* firstBox = nullptr);
  int count() const { return count_; }
  const char* page(int index);
 private:
  const char* text_ = "";
  uint16_t starts_[MAX_PAGES] = {}, ends_[MAX_PAGES] = {};
  int count_ = 1;
  char scratch_[PAGE_BYTES] = {};
  int fit(int start, const font::Box& box);
};
} // namespace kingdom
