// Self-check for the RGB565 surface and its text. Run: make test
// The goldens come from Biscuit's LVGL 8.3 firmware (tag biscuit-v0.1.0): widths and page breaks from
// lv_txt_get_width and its paginate(), and checksums of two framebuffer captures from the panel (a story page and a
// discovery page) that the renderer reproduces pixel for pixel.
#include "host/check.h"
#include <stdio.h>
#include <string.h>
#include "../games/biscuit/generated/fonts.h"
#include "canvas.h"
#include "crc32.h"
#include "font.h"
#include "gfx.h"
#include "gfx565.h"
#include "ui.h"
#include "utf8.h"

using namespace gfx565;
static uint16_t g_fb[W * H];
static const uint16_t PAPER = 0xFFBC, INK = 0x59E8;   // 0xfff7e6 and that build's ink 0x5a3d42, as LVGL makes them 565

static uint32_t regionCrc(int x0, int y0, int x1, int y1) {   // row-major little-endian words, as the capture checksum
  static uint16_t buf[W * H]; int n = 0;
  for (int y = y0; y < y1; y++) for (int x = x0; x < x1; x++) buf[n++] = fb[y * W + x];
  return os::crc32(buf, (size_t)n * 2);
}

static void widths() {
  const font::Font* F[4] = {&biscuit::FONT16, &biscuit::FONT20, &biscuit::FONT24, &biscuit::FONT28};
  const int kerned[4] = {77, 95, 115, 133}, utf8[4] = {94, 119, 142, 167}, curly[4] = {42, 54, 62, 72};
  for (int f = 0; f < 4; f++) {
    assert(font::textWidth(*F[f], "\"47.\" 7/4, 1.") == kerned[f]);       // lv_txt_get_width on the device
    assert(font::textWidth(*F[f], "caf\xc3\xa9 \xc2\xb7 sch\xc3\xb6n") == utf8[f]);
    assert(font::textWidth(*F[f], "odd\xe2\x80\x99") == curly[f]);       // U+2019 is not in the font: a placeholder
  }
  // The same width from the tables by hand: per glyph, (advance + class kerning with the next glyph) / 16, rounded.
  // This conversion kerns digits and punctuation only (letters have no class), so that is what the string holds.
  const font::Font& f = biscuit::FONT24;
  const char* s = "\"47.\" 7/4, 1.";
  int sum = 0, kernedPairs = 0;
  for (const char* p = s; *p; p++) {
    int g = *p - 31, n = p[1] ? p[1] - 31 : 0, l = f.kernLeft[g], r = n ? f.kernRight[n] : 0;
    int k = l && r ? f.kernValues[(l - 1) * f.kernRightClasses + r - 1] : 0;
    kernedPairs += k != 0;
    sum += (f.glyphs[g].advW + k + 8) >> 4;
  }
  assert(kernedPairs >= 4 && sum == font::textWidth(f, s));
  assert(font::glyphIndex(f, 'A') == 34 && font::glyphIndex(f, 0xB7) == 96 && font::glyphIndex(f, 0xF6) == 98 && font::glyphIndex(f, 0x2019) == 0);
  assert(font::textWidth(f, "\x01") == 0 && font::textWidth(f, "\xff") == 0);   // control and malformed bytes: no width
  assert(font::textWidth(f, "\xef\xa3\xbf\xe2\x80\x8c") == 0);                   // nor LVGL's symbol dummy and the ZWNJ
  assert(font::textWidth(f, "\t") == (2 * f.glyphs[1].advW + 8) >> 4);         // a tab: one space glyph, twice as wide
}

static void pages() {
  // The Moon Biscuit's opening, cut before the first line that names the child.
  static const char MOON[] = "At midnight, a perfect wedge vanished from every moon biscuit in Pepper's bakery window. The glass was locked. "
                "By morning, golden crumbs lay outside, on the wrong side of the glass.\n\nPepper had opened the shop only yesterday. "
                "\"Mice,\" she declared. Biscuit tilted his head at a crescent. \"Such tidy bites! My crumbs go everywhere. Do these mice "
                "carry rulers?\"\n\n";
  char moon[sizeof MOON];
  memcpy(moon, MOON, sizeof MOON);
  const font::Box story = {&biscuit::FONT24, 352, 176, 4, font::Align::LEFT};
  assert(font::textHeight(biscuit::FONT24, moon, 352, 4) == 536);   // lv_txt_get_size, trailing newlines included
  const char* p[4];
  assert(font::pageBreaks(story, moon, p, 4) == 3);
  assert(!strcmp(p[0], "At midnight, a perfect wedge vanished from every moon biscuit in Pepper's bakery window. The glass was locked. "
                       "By morning, golden crumbs lay outside,"));
  assert(!strcmp(p[1], "on the wrong side of the glass. Pepper had opened the shop only yesterday. \"Mice,\" she declared. Biscuit "
                       "tilted his head at a crescent. \"Such tidy bites! My crumbs"));
  assert(!strcmp(p[2], "go everywhere. Do these mice carry rulers?\""));
  uint16_t starts[8];
  assert(font::wrap(biscuit::FONT24, p[0], 352, starts, 8) == 6 && font::textHeight(biscuit::FONT24, p[0], 352, 4) == 176);
  assert(!strncmp(p[0] + starts[1], "wedge vanished", 14));   // the device's second line

  target(g_fb); clear(PAPER);   // page 1 exactly as the panel showed it
  assert(font::textBox(story, p[0], 64, 146, INK) == 176);
  assert(regionCrc(50, 136, 430, 336) == 0xf0b20278u);

  char sun[] = "The Sun is not a giant campfire. Deep inside it, hydrogen nuclei combine to make helium. This process, nuclear fusion, "
               "releases energy.\n\nOrdinary fire needs fuel and oxygen. Fusion works differently: the Sun's intensely hot, compressed "
               "core makes it possible. A star is its own kind of furnace.";
  assert(font::pageBreaks(story, sun, p, 4) == 2);
  clear(PAPER);
  font::textBox(story, p[0], 64, 146, INK);
  font::textBox({&biscuit::FONT16, 120, 0, 4, font::Align::CENTER}, "1 / 2", 180, 337, INK);
  assert(regionCrc(50, 136, 430, 336) == 0x57929d01u);

  char empty[] = " \n ";
  assert(font::pageBreaks(story, empty, p, 4) == 1 && !*p[0]);
  memcpy(moon, MOON, sizeof MOON);
  assert(font::pageBreaks(story, moon, p, 1) == 3 && p[0] == moon);   // stores what fits, still counts every page
  memcpy(moon, MOON, sizeof MOON);   // minLast: the last page (7 words) takes 3 from the end of the one before
  assert(font::pageBreaks(story, moon, p, 4, 10) == 3 && !strcmp(p[2], "bites! My crumbs go everywhere. Do these mice carry rulers?\""));
  assert(!strcmp(p[1] + strlen(p[1]) - 10, "\"Such tidy"));
  memcpy(moon, MOON, sizeof MOON);   // the same when only the first pages are stored
  assert(font::pageBreaks(story, moon, p, 2, 10) == 3 && !strcmp(p[1] + strlen(p[1]) - 10, "\"Such tidy"));
  memcpy(moon, MOON, sizeof MOON);   // but not when the page before would be left with fewer than minLast
  assert(font::pageBreaks(story, moon, p, 4, 40) == 3 && !strcmp(p[2], "go everywhere. Do these mice carry rulers?\""));
  const font::Box line = {&biscuit::FONT24, (int16_t)font::textWidth(biscuit::FONT24, "Supercalifragilistic"),
                          biscuit::FONT24.lineHeight, 4, font::Align::LEFT};
  char tail[] = "a a a Supercalifragilistic";   // nor when the last page would no longer fit
  assert(font::pageBreaks(line, tail, p, 4, 2) == 2 && !strcmp(p[0], "a a a") && !strcmp(p[1], "Supercalifragilistic"));
  assert(font::textHeight(biscuit::FONT28, "Supercalifragilisticexpialidocious", 150, 4) == 128);   // a word longer than the box splits
  assert(font::textHeight(biscuit::FONT16, "", 100, 4) == biscuit::FONT16.lineHeight);
}

static bool blank(int x0, int y0, int x1, int y1) {   // nothing drawn in [x0, x1) x [y0, y1)
  for (int y = y0; y < y1; y++) for (int x = x0; x < x1; x++) if (fb[y * W + x]) return false;
  return true;
}
static int leftmostInk(int y0, int y1) {
  for (int x = 0; x < W; x++) for (int y = y0; y < y1; y++) if (fb[y * W + x]) return x;
  return W;
}

static void drawing() {
  target(g_fb); clear(0);
  const font::Font& f = biscuit::FONT20;
  int w = font::textWidth(f, "Next"), solid = 0, edge = 0;
  assert(font::text(f, "Next", 100, 100, 0xFFFF) == 100 + w);
  for (int y = 100; y < 100 + f.lineHeight; y++) for (int x = 100; x < 100 + w; x++) {
    uint16_t c = fb[y * W + x];
    solid += c == 0xFFFF;
    edge += c && c != 0xFFFF;
  }
  assert(solid > 40);   // solid strokes
  assert(edge > 20);    // and anti-aliased edges
  assert(blank(0, 0, W, 95) && blank(0, 105 + f.lineHeight, W, H) && blank(0, 0, 95, H) && blank(106 + w, 0, W, H));

  clear(0);   // alignment: the same word lands left, centered and right in a 200 px box
  int ww = font::textWidth(f, "Hi"), x0[3];
  for (int a = 0; a < 3; a++) {
    font::textBox({&f, 200, 0, 0, (font::Align)a}, "Hi", 0, 50 * a, 0xFFFF);
    x0[a] = leftmostInk(50 * a, 50 * a + f.lineHeight);
  }
  assert(x0[1] - x0[0] == (200 - ww) / 2);
  assert(x0[2] - x0[0] == 200 - ww);
  assert(font::textBox({&f, 200, 0, 4, font::Align::LEFT}, "one\ntwo", 0, 200, 0xFFFF) == 2 * f.lineHeight + 4);

  clear(0);   // a missing glyph is LVGL's outlined placeholder box
  font::text(f, "\xe2\x80\x99", 10, 10, 0xFFFF);
  assert(fb[10 * W + 10] == 0xFFFF);
  assert(fb[(10 + f.lineHeight) * W + 10 + f.lineHeight / 2] == 0xFFFF);
  assert(!fb[12 * W + 12]);

  // font::clip: a label drawn in two halves, each cut to its half, is the label drawn whole
  static uint16_t whole[W * H];
  clear(0);
  font::text(f, "Goal", 100, 100, 0xFFFF);
  memcpy(whole, g_fb, sizeof whole);
  clear(0);
  font::clip(0, 0, 120, H);
  font::text(f, "Goal", 100, 100, 0xFFFF);
  assert(blank(120, 0, W, H));
  font::clip(120, 0, W, H);
  font::text(f, "Goal", 100, 100, 0xFFFF);
  font::clip(0, 0, W, H);
  assert(!memcmp(whole, g_fb, sizeof whole));
}

// font::clip, any box, for labels that hang off the panel's corners too: inside it the label's own pixels, outside it
// nothing.
static void clipped() {
  static uint16_t whole[W * H];
  target(g_fb);
  const font::Font& f = biscuit::FONT20;
  const int at[2][2] = {{-9, -7}, {452, 466}};   // ink in 0..34 x 0..10 and in 453..478 x 469..479
  const int cut[][4] = {{0, 0, 12, 9}, {5, 3, 6, 4}, {0, 6, W, 7}, {3, 0, 4, H}, {461, 470, W, H}, {0, 474, W, 475}, {470, 0, 471, H},
                        {-50, -50, 900, 900}, {10, 10, 10, 20}, {20, 9, 10, 8}};
  for (const auto& a : at) {
    clear(0);
    font::text(f, "Goal", a[0], a[1], 0xFFFF);
    memcpy(whole, g_fb, sizeof whole);
    int crossed = 0;
    for (const auto& c : cut) {
      clear(0);
      font::clip(c[0], c[1], c[2], c[3]);
      assert(font::text(f, "Goal", a[0], a[1], 0xFFFF) == a[0] + font::textWidth(f, "Goal"));
      font::clip(0, 0, W, H);
      int inked = 0;
      for (int y = 0; y < H; y++) for (int x = 0; x < W; x++) {
        const bool in = x >= c[0] && x < c[2] && y >= c[1] && y < c[3];
        assert(fb[y * W + x] == (in ? whole[y * W + x] : 0));
        inked += fb[y * W + x] != 0;
      }
      crossed += inked > 0;
    }
    assert(crossed >= 4);   // the cuts do cross each label: a corner of it, one row, one column, all of it
  }
}

static void audit() {   // the text log: the logical box and RGB colors the playtest audit reads
  target(g_fb); clear(PAPER);
  const font::Font& f = biscuit::FONT20;
  gfx::textLogEnabled = true; gfx::textLogReset();
  font::text(f, "Back", 100, 60, INK);
  gfx::textLogEnabled = false;
  const gfx::TextBox& t = gfx::textLog[0];
  assert(gfx::textLogCount == 1 && t.rgb);
  assert(t.x == 33 && t.y == 20 && t.h == 8);   // 100,60 physical, 22 px tall
  assert(t.w == (100 + font::textWidth(f, "Back") + 2) / 3 - 33);
  assert(t.color == rgb888(INK) && t.bg == rgb888(PAPER));
}

static void roundRects() {
  target(g_fb); clear(0);
  roundRect({10, 10, 40, 30}, 7, 1);
  assert(!fb[10 * W + 10] && !fb[10 * W + 13] && fb[10 * W + 14]);   // the top row is cut by 4 at each end
  assert(fb[10 * W + 45] && !fb[10 * W + 46]);
  assert(fb[17 * W + 10] && fb[17 * W + 49] && !fb[17 * W + 50] && !fb[17 * W + 9]);   // straight sides
  assert(!fb[39 * W + 10] && fb[39 * W + 14] && !fb[40 * W + 20]);                    // the bottom mirrors the top
  clear(0);
  roundRect({0, 0, 10, 4}, 99, 1);   // the radius clamps to half the box
  assert(fb[0 * W + 4] && !fb[0 * W + 0] && fb[2 * W + 0]);
  clear(0);
  roundRect({-5, -5, 10, 10}, 0, 2);   // radius 0 is a plain rectangle, clipped at the surface edge
  assert(fb[0] == 2 && fb[4 * W + 4] == 2 && !fb[5 * W + 5]);
}

static void shapes() {
  target(g_fb); clear(0);
  frame(100, 100, 5, 4, 3);
  hline(0, 479, 480, 4);
  assert(fb[100 * W + 100] == 3 && fb[103 * W + 104] == 3 && !fb[101 * W + 101] && fb[479 * W + 479] == 4);
  clear(0);
  circle(240, 240, 3, 5);
  assert(fb[240 * W + 243] == 5 && fb[237 * W + 240] == 5);
  assert(!fb[237 * W + 237] && !fb[240 * W + 244]);
  assert(inCircle(240, 0) && !inCircle(0, 0) && !inCircle(240, 2, 3));
  assert(rgb888(0xFFFF) == 0xFFFFFF && rgb888(0xF800) == 0xFF0000 && rgb888(0x07E0) == 0x00FF00);
}

static void indexedExpansion() {
  struct Guarded { uint16_t before, pixels[W * H], after; };
  static Guarded buffers[2];
  static uint8_t logical[160 * 160];
  uint16_t palette[256];
  for (int i = 0; i < 256; i++) palette[i] = (uint16_t)(i * 257);   // all bits, including black and white
  for (int i = 0; i < 160 * 160; i++) logical[i] = (uint8_t)(i * 37 + i / 160);
  uint32_t previous = 0;
  for (int frame = 0; frame < 3; frame++) {
    Guarded& out = buffers[frame & 1];
    out.before = 0x1234; out.after = 0xABCD;
    target(out.pixels);
    expandIndexed3(logical, palette);
    assert(out.before == 0x1234 && out.after == 0xABCD);
    for (int y = 0; y < H; y++) for (int x = 0; x < W; x++)
      assert(out.pixels[y * W + x] == palette[logical[(y / 3) * 160 + x / 3]]);
    if (frame) assert(os::crc32(buffers[(frame - 1) & 1].pixels, sizeof out.pixels) == previous);
    previous = os::crc32(out.pixels, sizeof out.pixels);
    for (uint8_t& c : logical) c++;   // a different frame must land only in the next target
  }
}
// With what the buffer holds: only the rows that changed are rewritten, and every row after a palette change.
static void heldExpansion() {
  static uint8_t logical[160 * 160];
  uint16_t palette[256];
  for (int i = 0; i < 256; i++) palette[i] = (uint16_t)(i * 257);
  for (int i = 0; i < 160 * 160; i++) logical[i] = (uint8_t)(i * 37 + i / 160);
  Expanded held = {};
  uint16_t other[256];
  for (int i = 0; i < 256; i++) other[i] = (uint16_t)~palette[i];
  target(g_fb);
  expandIndexed3(logical, palette, &held);
  for (int i = 0; i < W * H; i++) assert(g_fb[i] == palette[logical[(i / W / 3) * 160 + i % W / 3]]);
  for (uint16_t& p : g_fb) p = 0x5A5A;   // a sentinel: whatever is not rewritten keeps it
  logical[70 * 160 + 9]++;
  expandIndexed3(logical, palette, &held);
  for (int y = 0; y < H; y++)
    for (int x = 0; x < W; x++) assert(g_fb[y * W + x] == (y / 3 == 70 ? palette[logical[70 * 160 + x / 3]] : 0x5A5A));
  expandIndexed3(logical, other, &held);
  for (int i = 0; i < W * H; i++) assert(g_fb[i] == other[logical[(i / W / 3) * 160 + i % W / 3]]);
  held.palette = nullptr;   // the host drew something else into it
  for (uint16_t& p : g_fb) p = 0x5A5A;
  expandIndexed3(logical, other, &held);
  assert(g_fb[0] == other[logical[0]] && g_fb[W * H - 1] == other[logical[160 * 160 - 1]]);
}

// canvas::Frames holds at most N movers: a page with more is drawn whole, every frame, never past its buffer.
static void tooManyMovers() {
  target(g_fb);
  canvas::Frames<1> frames;
  const canvas::Mover two[2] = {{{0, 0, 10, 10}, 1}, {{20, 20, 30, 30}, 2}};
  int whole = 0;
  for (int i = 0; i < 2; i++) frames.render(two, 2, 7, [&] { whole += canvas::same(canvas::clipBox(), canvas::FULL); });
  assert(whole == 2);
  const canvas::Mover one[1] = {{{0, 0, 10, 10}, 1}};
  int calls = 0;
  for (int i = 0; i < 2; i++) frames.render(one, 1, 7, [&] { calls++; });
  assert(calls == 1);   // back within N: one whole frame, then nothing changed
}

// The indexed surface's 8x8 font (os/gfx.h): glyphs trimmed to their ink, a 3 px space, '?' for a byte past ASCII.
static void indexedFont() {
  uint8_t widths[256];
  for (int c = 0; c < 256; c++) widths[c] = (uint8_t)gfx::glyphWidth((char)c);
  assert(os::crc32(widths, sizeof widths) == 0x386df87au);   // every glyph's width
  assert(widths[' '] == 3 && widths['i'] == 4 && widths['W'] == 7 && widths['.'] == 2 && widths[0x80] == widths['?']);
  assert(gfx::textWidth("Hello, world!") == 79 && gfx::textWidth("Hello, world!", 2) == 158 && gfx::textWidth("") == 0);
  memset(gfx::fb, 0, sizeof gfx::fb);
  assert(gfx::text(3, 5, "Wag it! (9)", 7) == 68);   // the end x
  gfx::text(2, 40, "Big", 9, 2);
  assert(os::crc32(gfx::fb, sizeof gfx::fb) == 0x686e30ebu);
}

// Names in Ukrainian: the 8x8 font draws the alphabet, the keyboard types it as UTF-8 within the save's bytes, and
// nothing cuts a letter in half.
static Input tapAt(int x, int y) { Input in; in.tap = in.released = true; in.x = in.downX = x; in.y = in.downY = y; return in; }
static void typeKey(char* buf, int& len, uint8_t& page, size_t cap, int i) {   // key i of the 4x4 grid
  ui::keyboard(tapAt(29 + (i % 4) * 26 + 12, 42 + (i / 4) * 24 + 11), buf, len, page, cap);
}
static void ukrainianNames() {
  auto ink = [](const char* t) { memset(gfx::fb, 0, sizeof gfx::fb); gfx::text(0, 0, t, 7); return os::crc32(gfx::fb, sizeof gfx::fb); };
  assert(ink("\xd0\x91") != ink("?") && ink("\xd2\x91") != ink("?") && ink("\xd0\x91") != ink("\xd2\x91"));   // Б, ґ
  assert(gfx::textWidth("\xc3\xa9") == gfx::textWidth("?") && gfx::textWidth("A\x80") == gfx::textWidth("A"));
  assert(utf8::count("\xd0\x91\xd1\x96\xd0\xbc") == 3 && utf8::ukrainian(0x491) && !utf8::ukrainian(0x42B));
  char caps[] = "\xd1\x97\xd0\xb6\xd0\xb0\xd0\xba \xd2\x91\xd1\x8f\xd1\x94x";   // їжак ґяєx -> ЇЖАК ҐЯЄX
  utf8::upper(caps); assert(!strcmp(caps, "\xd0\x87\xd0\x96\xd0\x90\xd0\x9a \xd2\x90\xd0\xaf\xd0\x84X"));
  char cut[6]; utf8::copy(cut, sizeof cut, "\xd0\x91\xd1\x96\xd0\xbc");   // 6 bytes into 5: two letters, not 2.5
  assert(!strcmp(cut, "\xd0\x91\xd1\x96"));
  char buf[12] = {}; int len = 0; uint8_t page = 0;
  ui::keyboard(tapAt(80, 147), buf, len, page, sizeof buf); assert(page == 2);   // the UA key
  typeKey(buf, len, page, sizeof buf, 1); typeKey(buf, len, page, sizeof buf, 11);   // Б, then і (small after the first)
  assert(!strcmp(buf, "\xd0\x91\xd1\x96") && len == 4);
  for (int i = 0; i < 6; i++) typeKey(buf, len, page, sizeof buf, 0);   // а until the 11 bytes are full
  assert(utf8::count(buf) == 5 && len == 10 && strlen(buf) == 10);
  ui::keyboard(tapAt(29 + 2 * 26 + 12, 42 + 3 * 24 + 11), buf, len, page, sizeof buf);   // backspace: one whole letter
  assert(len == 8 && utf8::count(buf) == 4);
  typeKey(buf, len, page, sizeof buf, 12); assert(utf8::count(buf) == 5);   // ї on the last row
  ui::keyboard(tapAt(80, 147), buf, len, page, sizeof buf); assert(page == 0);   // back to English: a fits one byte
  typeKey(buf, len, page, sizeof buf, 0); assert(len == 11 && !strcmp(buf + 10, "a"));
  char ap[12] = {}; int al = 0; uint8_t pg = 4;   // Ц Ч Ш Щ Ь Ю Я ': an apostrophe joins two letters, once
  typeKey(ap, al, pg, sizeof ap, 7); typeKey(ap, al, pg, sizeof ap, 0); typeKey(ap, al, pg, sizeof ap, 7);
  typeKey(ap, al, pg, sizeof ap, 7); assert(!strcmp(ap, "\xd0\xa6'"));
  assert(!ui::keyboard(tapAt(29 + 3 * 26 + 12, 42 + 3 * 24 + 11), ap, al, pg, sizeof ap));   // OK waits for a letter
  typeKey(ap, al, pg, sizeof ap, 6);
  assert(ui::keyboard(tapAt(29 + 3 * 26 + 12, 42 + 3 * 24 + 11), ap, al, pg, sizeof ap));
  char en[12] = {}; int n = 0; uint8_t p = 0;
  for (int i = 0; i < 10; i++) typeKey(en, n, p, sizeof en, 0);   // English stops at ui::NAME_LEN letters
  assert(!strcmp(en, "Aaaaaaaa"));
}

int main() {
  widths();
  indexedFont();
  ukrainianNames();
  pages();
  drawing();
  clipped();
  audit();
  roundRects();
  shapes();
  indexedExpansion();
  heldExpansion();
  tooManyMovers();
  puts("test_gfx565: all checks passed");
  return 0;
}
