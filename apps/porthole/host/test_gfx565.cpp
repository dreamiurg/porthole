// Self-check for the RGB565 surface and its text. Run: make test
// The goldens come from Biscuit's LVGL 8.3 firmware (tag biscuit-v0.1.0): widths and page breaks from
// lv_txt_get_width and its paginate(), and checksums of two framebuffer captures from the panel (a story page and a
// discovery page) that the renderer reproduces pixel for pixel.
#include <assert.h>
#include <stdio.h>
#include <string.h>
#include "../games/biscuit/generated/fonts.h"
#include "crc32.h"
#include "font.h"
#include "gfx.h"
#include "gfx565.h"

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
  memcpy(moon, MOON, sizeof MOON);   // minLast: the last page (7 words) takes a word from the end of the one before
  assert(font::pageBreaks(story, moon, p, 4, 8) == 3 && !strcmp(p[2], "crumbs go everywhere. Do these mice carry rulers?\""));
  assert(!strcmp(p[1] + strlen(p[1]) - 9, "bites! My"));
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

static void images() {
  target(g_fb); clear(0);
  // 4x3 image: a run crossing a row end, one ending the image, then one past it (ignored)
  static const uint16_t runs[] = {5, 0x1111, 3, 0x2222, 4, 0x3333, 9, 0x4444};
  const RleImage rle = {4, 3, runs, 4};
  blitRle(rle, 10, 20, 1);
  const uint16_t want[12] = {0x1111, 0x1111, 0x1111, 0x1111, 0x1111, 0x2222, 0x2222, 0x2222, 0x3333, 0x3333, 0x3333, 0x3333};
  for (int i = 0; i < 12; i++) assert(fb[(20 + i / 4) * W + 10 + i % 4] == want[i]);
  assert(!fb[23 * W + 10] && !fb[20 * W + 14]);
  clear(0);
  blitRle(rle, -3, 0, 3);   // 3x blocks, clipped on the left
  assert(fb[0] == 0x1111 && fb[2 * W + 8] == 0x1111 && fb[3 * W + 0] == 0x2222 && fb[3 * W + 3] == 0x2222 && fb[5 * W + 8] == 0x2222);
  assert(fb[6 * W + 0] == 0x3333 && fb[8 * W + 8] == 0x3333 && !fb[9 * W]);
}

static void pictures() {
  static const uint16_t px[4] = {1, 2, 3, 4};
  target(g_fb); clear(0);
  blit({2, 2, px}, 0, 0, 3);
  assert(fb[0] == 1 && fb[2 * W + 5] == 2 && fb[3 * W + 2] == 3 && fb[5 * W + 5] == 4 && !fb[6 * W]);
  blit({2, 2, px}, 478, 478, 1);   // clipped at the corner
  assert(fb[478 * W + 478] == 1 && fb[479 * W + 479] == 4);
  clear(0);
  blit({2, 2, px}, -1, -1, 1);   // clipped at the origin: only the last pixel lands
  assert(fb[0] == 4 && !fb[1] && !fb[W]);
}

int main() {
  widths();
  pages();
  drawing();
  audit();
  roundRects();
  shapes();
  images();
  pictures();
  puts("test_gfx565: all checks passed");
  return 0;
}
