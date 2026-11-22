// UTF-8 helpers for names a kid can type in Ukrainian (ui::keyboard): a Cyrillic letter is 2 bytes, so a name's buffer
// fills before its letter count, and a cut must never split a letter.
#pragma once
#include <stddef.h>
#include <stdint.h>
#include <string.h>

namespace utf8 {
inline bool cont(char c) { return ((uint8_t)c & 0xC0) == 0x80; }   // a byte inside a letter, not its start
inline int count(const char* s) { int n = 0; for (; *s; s++) n += !cont(*s); return n; }   // letters
// Copies s into out (n bytes with the terminator): at most maxLetters letters, cut only between letters.
inline void copy(char* out, size_t n, const char* s, int maxLetters = 1 << 30) {
  if (!n) return;
  size_t k = 0;
  for (int letters = 0; s[k] && k + 1 < n; k++) if (!cont(s[k]) && ++letters > maxLetters) break;
  while (k && cont(s[k])) k--;   // the byte after the cut starts no letter: back off the half letter
  memmove(out, s, k); out[k] = 0;
}
// Appends code point cp (ASCII or 2-byte, U+0080..U+07FF) at buf[len] if it fits in cap bytes with the terminator.
inline bool append(char* buf, int& len, size_t cap, uint16_t cp) {
  const int need = cp < 0x80 ? 1 : 2;
  if ((size_t)(len + need) >= cap) return false;
  if (need == 1) buf[len++] = (char)cp;
  else { buf[len++] = (char)(0xC0 | cp >> 6); buf[len++] = (char)(0x80 | (cp & 0x3F)); }
  buf[len] = 0;
  return true;
}
// The Ukrainian alphabet, both cases, as the UA keyboard types it (the apostrophe is ASCII ').
inline bool ukrainian(uint16_t cp) {
  return (cp >= 0x410 && cp <= 0x44F && cp != 0x42A && cp != 0x42B && cp != 0x42D && cp != 0x44A && cp != 0x44B &&
          cp != 0x44D) || cp == 0x404 || cp == 0x406 || cp == 0x407 || cp == 0x454 || cp == 0x456 || cp == 0x457 ||
         cp == 0x490 || cp == 0x491;
}
// s's first letter if it is a 2-byte UTF-8 one, else 0.
inline uint16_t letter2(const char* s) {
  const uint8_t a = s[0], b = a ? s[1] : 0;
  return (a & 0xE0) == 0xC0 && (b & 0xC0) == 0x80 ? (uint16_t)((a & 0x1F) << 6 | (b & 0x3F)) : 0;
}
// s in capitals, in place: ASCII and the Ukrainian alphabet (every Ukrainian pair is 2 bytes in both cases).
inline void upper(char* s) {
  for (; *s; s++) {
    if (*s >= 'a' && *s <= 'z') { *s = (char)(*s - 32); continue; }
    uint16_t cp = letter2(s);
    if (!cp) continue;
    cp = cp >= 0x430 && cp <= 0x44F ? cp - 0x20 : cp >= 0x450 && cp <= 0x45F ? cp - 0x50 : cp == 0x491 ? 0x490 : cp;
    *s = (char)(0xC0 | cp >> 6); s[1] = (char)(0x80 | (cp & 0x3F)); s++;
  }
}
inline void popLetter(char* buf, int& len) {   // backspace: the whole last letter
  if (!len) return;
  while (len > 0 && cont(buf[--len])) {}
  buf[len] = 0;
}
}  // namespace utf8
