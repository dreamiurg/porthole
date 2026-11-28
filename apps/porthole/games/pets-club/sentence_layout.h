// Where a sentence goes on the round screen (ukrainian.cpp draws it, host/test_pets_uk.cpp checks every sentence fits).
// Logical 160x160, drawn x3; text in Andika 36 (os/uk_fonts.h) at native size. Top to bottom inside a circle of
// radius R: the round's dots, the sentence in up to two lines, one line of the translation built so far (the tail of
// it, after an ellipsis, once it is longer), then the tiles in up to three rows. Built, the translation takes the
// tiles' place in up to two lines over a check.
#pragma once
#include <math.h>
#include <stdint.h>
#include <stdio.h>
#include <string.h>
#include "font.h"
#include "ukrainian.h"
#include "uk_fonts.h"

namespace learn::uk::layout {
struct Rect { int x, y, w, h; };
constexpr int R = 78, CY = 80;                      // the circle everything stays in: the glass, less 6 px
constexpr int PROMPT_Y = 35, LINE_H = 14, PROMPT_LINES = 2;
constexpr int ANSWER_Y = PROMPT_Y + PROMPT_LINES * LINE_H, DONE_LINES = 2;
constexpr int BANK_Y = ANSWER_Y + LINE_H, TILE_H = 22, GAP = 2, ROWS = 3, PAD = 3, MIN_W = 24;   // a tap target is 24x22 (8 mm)
constexpr int CHECK_Y = 120, CHECK_R = 14;
inline const font::Font& textFont() { return ukfont::FONT36; }

// The circle's width (logical) across the band y0..y1.
inline int chord(int y0, int y1) {
  const int a = y0 - CY < 0 ? CY - y0 : y0 - CY, b = y1 - CY < 0 ? CY - y1 : y1 - CY, far = a > b ? a : b;
  return far >= R ? 0 : 2 * (int)sqrtf((float)(R * R - far * far));
}
// A line of text at logical y: native px across, its letters (not their line box) inside the circle.
inline int textRoom(int y) { return (chord(y + 2, y + LINE_H - 2) - 4) * 3; }
inline int textW(const char* s) { return font::textWidth(textFont(), s); }   // native px
inline int tileW(const char* s) { const int w = (textW(s) + 2) / 3 + 2 * PAD; return w < MIN_W ? MIN_W : w; }

// Tiles this wide, in rows from BANK_Y, centered: each row takes, in the bank's order, every tile still out that fits
// beside those it has. False if they need more than ROWS.
inline bool tiles(const int* w, int n, Rect* out) {
  uint16_t placed = 0;
  for (int row = 0; row < ROWS && placed != (1u << n) - 1; row++) {
    const int y = BANK_Y + row * (TILE_H + GAP), room = chord(y, y + TILE_H) - 2;
    int in[16], k = 0, used = 0;
    for (int i = 0; i < n; i++)
      if (!((placed >> i) & 1) && used + (k ? GAP : 0) + w[i] <= room) { used += (k ? GAP : 0) + w[i]; in[k++] = i; placed |= (uint16_t)(1u << i); }
    for (int j = 0, x = 80 - used / 2; j < k; x += w[in[j++]] + GAP) out[in[j]] = {x, y, w[in[j]], TILE_H};
  }
  return placed == (1u << n) - 1;
}

// The tiles of a bank, in its order, laid out: false if they need more than ROWS.
inline bool laysOut(const Sentence& s, const Bank& b, Rect* out) {
  int w[MAX_BANK]; char t[TILE_CHARS];
  for (int i = 0; i < b.n; i++) { bankText(s, b, i, t); w[i] = tileW(t); }
  return tiles(w, b.n, out);
}
// A bank's tiles widest first: the order the game falls back to when shuffles do not fit (the test checks it fits).
inline void widestFirst(const Sentence& s, Bank& b) {
  char x[TILE_CHARS], y[TILE_CHARS];
  for (int a = 0; a < b.n; a++)
    for (int c = a + 1; c < b.n; c++) {
      bankText(s, b, a, x); bankText(s, b, c, y);
      if (tileW(y) > tileW(x)) { const uint8_t t = b.tile[a]; b.tile[a] = b.tile[c]; b.tile[c] = t; }
    }
}

// Text in lines of at most `lines`, from logical y, each as wide as the circle allows there: the byte where each line
// starts (starts[lines] is the end). Returns the lines used, 0 if it does not fit. A line breaks at a space.
inline int wrap(const char* s, int y, int lines, int* starts) {
  const int len = (int)strlen(s);
  int at = 0, n = 0;
  char buf[160];
  while (at < len) {
    if (n == lines) return 0;
    const int room = textRoom(y + n * LINE_H);
    int end = at, fit = -1;
    while (end <= len) {   // the longest run of words from `at` that fits
      if (end == len || s[end] == ' ') {
        memcpy(buf, s + at, (size_t)(end - at)); buf[end - at] = 0;
        if (textW(buf) > room) break;
        fit = end;
      }
      if (end == len) break;
      end++;
    }
    if (fit < 0) return 0;
    starts[n++] = at;
    at = fit < len ? fit + 1 : len;
  }
  starts[n] = len + 1;
  return n;
}
// The built line: the whole of it, or "..." and as many of its last words as fit. out holds it.
inline void tail(const char* s, char* out, size_t cap) {
  const int room = textRoom(ANSWER_Y);
  const char* from = s;
  for (;;) {
    snprintf(out, cap, "%s%s", from == s ? "" : "...", from);
    const char* sp = strchr(from, ' ');
    if (textW(out) <= room || !sp) return;
    from = sp + 1;
  }
}
}  // namespace learn::uk::layout
