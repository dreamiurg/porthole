#include "font.h"
#include <ctype.h>
#include <string.h>
#include "gfx.h"
#include "gfx565.h"

// Each helper names the LVGL 8.3 function it ports; LVGL's configuration here is Biscuit's old one: UTF-8, no recolor,
// letter space 0, LV_TXT_LINE_BREAK_LONG_LEN 0, LV_USE_FONT_PLACEHOLDER 1, LV_COLOR_DEPTH 16 without byte swap.
namespace font {
namespace {
constexpr uint32_t NO_BREAK = 0xFFFFFFFFu;
struct Pen { int x, y; int x0, y0, x1, y1; uint16_t fg; };   // the pen position, a half-open clip box and the color

uint32_t next(const char* s, uint32_t& i) {   // _lv_txt_utf8_next: a malformed sequence reads as 0 (zero width)
  uint8_t c = (uint8_t)s[i++];
  if (c < 0x80) return c;
  int n = (c & 0xE0) == 0xC0 ? 1 : (c & 0xF0) == 0xE0 ? 2 : (c & 0xF8) == 0xF0 ? 3 : 0;
  if (!n) return 0;
  uint32_t cp = c & (0x3Fu >> n);
  while (n--) {
    if (((uint8_t)s[i] & 0xC0) != 0x80) return 0;
    cp = cp << 6 | ((uint8_t)s[i++] & 0x3F);
  }
  return cp;
}
int kern(const Font& f, int left, int right) {   // get_kern_value, class tables
  int l = f.kernLeft[left], r = f.kernRight[right];
  return l && r ? f.kernValues[(l - 1) * f.kernRightClasses + r - 1] : 0;
}
int advance(const Font& f, uint32_t cp, uint32_t nextCp) {   // lv_font_get_glyph_width: kerned, rounded per glyph
  bool tab = cp == '\t';
  int g = glyphIndex(f, tab ? ' ' : cp);
  if (!g) return cp < 0x20 ? 0 : f.lineHeight / 2 + 2;   // the placeholder box
  int n = glyphIndex(f, nextCp);
  return (f.glyphs[g].advW * (tab ? 2 : 1) + (n ? kern(f, g, n) : 0) + 8) >> 4;
}
bool endsWord(uint32_t c) { return c == '\n' || c == '\r' || (c < 128 && c && strchr(" ,.;:-_", (int)c)); }

// lv_txt_get_next_word: bytes of the word (or single break character) at txt, 0 if it does not fit in maxW unless
// force (then the bytes that fit). *wordW gets the width of what fits.
uint32_t nextWord(const Font& f, const char* txt, int maxW, int* wordW, bool force) {
  if (!txt[0]) return 0;
  uint32_t i = 0, iNext = 0, brk = NO_BREAK;
  uint32_t letter = next(txt, iNext), letterNext = 0, iNextNext = iNext;
  int curW = 0, wordLen = 0;
  while (txt[i]) {
    letterNext = next(txt, iNextNext);
    wordLen++;
    curW += advance(f, letter, letterNext);
    if (brk == NO_BREAK && curW > maxW) brk = i;   // the first letter that does not fit
    if (endsWord(letter)) {
      if (i == 0 && brk == NO_BREAK) *wordW = curW;
      wordLen--;
      break;
    }
    if (brk == NO_BREAK) *wordW = curW;
    i = iNext; iNext = iNextNext; letter = letterNext;
  }
  if (brk == NO_BREAK) return wordLen == 0 || (letter == '\r' && letterNext == '\n') ? iNext : i;
  if (force) return brk;
  *wordW = 0;
  return 0;
}
uint32_t nextLine(const Font& f, const char* txt, int maxW) {   // _lv_txt_get_next_line: bytes of the line at txt
  if (!txt[0]) return 0;
  uint32_t i = 0;
  while (txt[i] && maxW > 0) {
    int wordW = 0;
    uint32_t adv = nextWord(f, txt + i, maxW, &wordW, i == 0);
    maxW -= wordW;
    if (!adv) break;
    i += adv;
    if (txt[0] == '\n' || txt[0] == '\r') break;
    if (txt[i] == '\n' || txt[i] == '\r') { i++; break; }
  }
  if (!i) next(txt, i);   // always step at least one letter
  return i;
}
int lineWidth(const Font& f, const char* s, uint32_t len) {   // lv_txt_get_width: kerns with the letter after len too
  int w = 0;
  for (uint32_t i = 0; i < len;) {
    uint32_t c = next(s, i), j = i;
    w += advance(f, c, next(s, j));
  }
  return w;
}

uint16_t mix(uint16_t fg, uint16_t bg, uint8_t opa) {   // lv_color_mix for 16-bit color: 5-bit mix, three channels at once
  uint32_t m = (opa + 4u) >> 3;
  uint32_t b = (bg | (uint32_t)bg << 16) & 0x7E0F81Fu, f = (fg | (uint32_t)fg << 16) & 0x7E0F81Fu;
  uint32_t r = ((((f - b) * m) >> 5) + b) & 0x7E0F81Fu;
  return (uint16_t)(r >> 16 | r);
}
// The clip of an LVGL label: its box grown by lineHeight / 4 on every side (the label's ext draw size, room for
// glyphs that overhang it), cut to the surface.
Pen pen(const Font& f, int x, int y, int w, int h) {
  int e = f.lineHeight / 4, x0 = x - e, y0 = y - e, x1 = x + w + e, y1 = y + h + e;
  return {x, y, x0 < 0 ? 0 : x0, y0 < 0 ? 0 : y0, x1 > gfx565::W ? gfx565::W : x1, y1 > gfx565::H ? gfx565::H : y1, 0};
}
void fill(const Pen& p, int x, int y, int w, int h) {
  int x0 = x > p.x0 ? x : p.x0, y0 = y > p.y0 ? y : p.y0, x1 = x + w < p.x1 ? x + w : p.x1, y1 = y + h < p.y1 ? y + h : p.y1;
  if (x1 > x0 && y1 > y0) gfx565::rect(x0, y0, x1 - x0, y1 - y0, p.fg);
}
void drawGlyph(const Font& f, uint32_t cp, const Pen& p) {   // lv_draw_sw_letter + draw_letter_normal
  int gi = glyphIndex(f, cp);
  if (!gi) {   // LVGL's placeholder: a 1 px outline, (lineHeight/2 + 1) x (lineHeight + 1)
    int w = f.lineHeight / 2 + 1, h = f.lineHeight + 1;
    if (cp >= 0x20) { fill(p, p.x, p.y, w, 1); fill(p, p.x, p.y + h - 1, w, 1); fill(p, p.x, p.y, 1, h); fill(p, p.x + w - 1, p.y, 1, h); }
    return;
  }
  const Glyph& g = f.glyphs[gi];
  const uint8_t* bm = f.bitmap + g.bitmapIndex;
  int gx = p.x + g.ofsX, gy = p.y + f.lineHeight - f.baseLine - g.boxH - g.ofsY;
  for (int row = 0; row < g.boxH; row++) {
    int py = gy + row;
    if (py < p.y0 || py >= p.y1) continue;
    for (int col = 0; col < g.boxW; col++) {
      uint32_t bit = (uint32_t)(row * g.boxW + col) * 4;
      uint8_t v = (bm[bit >> 3] >> ((bit & 4) ? 0 : 4)) & 15;
      int px = gx + col;
      if (!v || px < p.x0 || px >= p.x1) continue;
      uint16_t& d = gfx565::fb[py * gfx565::W + px];
      d = v == 15 ? p.fg : mix(p.fg, d, (uint8_t)(v * 17));   // _lv_bpp4_opa_table: coverage * 17
    }
  }
}
int drawLine(const Font& f, const char* s, uint32_t len, Pen& p) {   // lv_draw_label's letter loop
  for (uint32_t i = 0; i < len;) {
    uint32_t c = next(s, i), j = i;
    drawGlyph(f, c, p);
    p.x += advance(f, c, next(s, j));
  }
  return p.x;
}

int floor3(int v) { return v / 3 - (v % 3 < 0); }
// The audit's view of a line (gfx::textLog, logical px): its covering box, the colors, and the background under it,
// taken before the glyphs by majority vote. ponytail: a vote, not a histogram (no 128 KB table on the device); a
// line over a busy picture with no majority color reports an arbitrary pixel of it.
void logLine(const Font& f, int x, int y, int w, uint16_t fg) {
  if (!gfx::textLogEnabled || gfx::textLogCount >= 64 || w <= 0) return;
  uint16_t bg = fg; int votes = 0;
  for (int yy = y; yy < y + f.lineHeight; yy++) for (int xx = x; xx < x + w; xx++) {
    if ((unsigned)xx >= gfx565::W || (unsigned)yy >= gfx565::H) continue;
    uint16_t c = gfx565::fb[yy * gfx565::W + xx];
    if (!votes) { bg = c; votes = 1; } else votes += c == bg ? 1 : -1;
  }
  int x0 = floor3(x), y0 = floor3(y), x1 = -floor3(-(x + w)), y1 = -floor3(-(y + f.lineHeight));
  gfx::textLog[gfx::textLogCount++] = {(int16_t)x0, (int16_t)y0, (int16_t)(x1 - x0), (int16_t)(y1 - y0), gfx565::rgb888(fg), gfx565::rgb888(bg), true};
}
void squeeze(char* s) {   // paginate() split on isspace and joined with single spaces
  char* o = s;
  for (const char* p = s; *p; p++) {
    if (!isspace((unsigned char)*p)) *o++ = *p;
    else if (o > s && p[1] && !isspace((unsigned char)p[1])) *o++ = ' ';
  }
  *o = 0;
}
}  // namespace

int glyphIndex(const Font& f, uint32_t cp) {
  if (cp >= 32 && cp <= 126) return (int)cp - 31;
  for (int k = 0; k < f.extraCount; k++) if (f.extras[k] == cp) return 96 + k;
  return 0;
}
int textWidth(const Font& f, const char* s) { return lineWidth(f, s, (uint32_t)strlen(s)); }
int text(const Font& f, const char* s, int x, int y, uint16_t fg) {
  uint32_t len = (uint32_t)strlen(s);
  int w = lineWidth(f, s, len);
  Pen p = pen(f, x, y, w, f.lineHeight);
  p.fg = fg;
  logLine(f, x, y, w, fg);
  return drawLine(f, s, len, p);
}
int wrap(const Font& f, const char* s, int width, uint16_t* starts, int maxLines) {
  int n = 0;
  for (uint32_t at = 0, len; (len = nextLine(f, s + at, width)) != 0; at += len, n++) if (n < maxLines) starts[n] = (uint16_t)at;
  return n;
}
int textHeight(const Font& f, const char* s, int width, int spacing) {   // lv_txt_get_size
  int n = wrap(f, s, width, nullptr, 0);
  size_t len = strlen(s);
  if (len && (s[len - 1] == '\n' || s[len - 1] == '\r')) n++;   // a trailing newline opens one more line
  return n ? n * (f.lineHeight + spacing) - spacing : f.lineHeight;
}
int textBox(const Box& b, const char* s, int x, int y, uint16_t fg) {   // lv_draw_label into a LONG_WRAP label
  const Font& f = *b.font;
  int h = textHeight(f, s, b.w, b.spacing);
  Pen p = pen(f, x, y, b.w, h);
  p.fg = fg;
  for (uint32_t at = 0, len; (len = nextLine(f, s + at, b.w)) != 0; at += len) {
    int lw = lineWidth(f, s + at, len);
    p.x = x + (b.align == CENTER ? (b.w - lw) / 2 : b.align == RIGHT ? b.w - lw : 0);
    logLine(f, p.x, p.y, lw, fg);
    drawLine(f, s + at, len, p);
    p.y += f.lineHeight + b.spacing;
  }
  return h;
}
int pageBreaks(const Box& b, char* s, const char** pages, int maxPages) {   // the LVGL build's paginate()
  squeeze(s);
  int n = 0;
  char* page = s;
  for (char* p = s; *p;) {
    char* e = p;
    while (*e && *e != ' ') e++;
    char keep = *e;
    *e = 0;   // measure the page so far plus this word, as its own string
    bool over = textHeight(*b.font, page, b.w, b.spacing) > b.h;
    *e = keep;
    if (over && p != page) {   // this word starts the next page
      p[-1] = 0;
      if (n < maxPages) pages[n] = page;
      n++;
      page = p;
    }
    p = *e ? e + 1 : e;
  }
  if (n < maxPages) pages[n] = page;
  return n + 1;
}
}  // namespace font
