// Anti-aliased bitmap text on the RGB565 surface (os/gfx565.h), from fonts converted out of lv_font_conv 1.5.3
// output (4 bpp, class kerning). Measuring, wrapping and drawing follow LVGL 8.3 (lv_font_fmt_txt, lv_txt,
// lv_draw_label) step for step, so a text laid out here breaks and renders exactly as it did on an LVGL build:
// break characters " ,.;:-_", per-glyph rounding of kerned advances, glyphs clipped like a label's (its box plus
// lineHeight / 4 around it).
// Text is UTF-8; a code point the font lacks draws as an outlined box (LVGL's placeholder). Physical px throughout.
#pragma once
#include <stdint.h>

namespace font {
struct Glyph { uint32_t bitmapIndex; uint16_t advW; uint8_t boxW, boxH; int8_t ofsX, ofsY; };   // advW in 1/16 px
struct Font {
  const uint8_t* bitmap;            // 4 bpp coverage, each glyph's rows packed back to back, high nibble first
  const Glyph* glyphs;              // [0] unused, [1..95] ASCII 32..126, then one per `extras` code point
  const uint16_t* extras; uint8_t extraCount;   // ascending
  const uint8_t* kernLeft; const uint8_t* kernRight;   // glyph -> kerning class, 0 = none
  const int8_t* kernValues; uint8_t kernRightClasses;  // [(left - 1) * kernRightClasses + right - 1], 1/16 px; all required
  uint8_t lineHeight, baseLine;     // baseLine: from the bottom of the line
};
enum class Align : uint8_t { LEFT, CENTER, RIGHT };
// A label: `w` wide, lines `spacing` px apart (LVGL line_space). `h` is only the page height for pageBreaks;
// textBox is as tall as its text.
struct Box { const Font* font; int16_t w, h; int8_t spacing; Align align; };

int glyphIndex(const Font& f, uint32_t cp);   // 0 = not in the font
int textWidth(const Font& f, const char* s);  // as one line
// One line with its top-left at (x, y): an LVGL label of content size (textWidth x lineHeight); '\n' draws nothing,
// use textBox for several lines. Returns the end x.
int text(const Font& f, const char* s, int x, int y, uint16_t fg);
// Word-wraps s at `width`: returns the line count and stores up to maxLines line starts (byte offsets, so s is
// under 64 KB).
int wrap(const Font& f, const char* s, int width, uint16_t* starts, int maxLines);
int textHeight(const Font& f, const char* s, int width, int spacing);   // lv_txt_get_size's height
// Wrapped and aligned in the box (x, y, b.w, its text height), an LVGL LONG_WRAP label. Returns that height.
int textBox(const Box& b, const char* s, int x, int y, uint16_t fg);
// Splits s into pages that fit b.w x b.h, word by word, the way the LVGL build's paginate() did: s is rewritten in
// place (whitespace runs become one space, ends trimmed, each page NUL-terminated) and pages[] points into it.
// Returns the page count (at least 1) and stores up to maxPages.
int pageBreaks(const Box& b, char* s, const char** pages, int maxPages);
}  // namespace font
