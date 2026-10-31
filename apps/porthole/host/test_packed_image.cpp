// Decoder contracts: exact RGB565 bytes, clipping/scaling, independent panel targets and bounded inputs.
// Fixtures come from the system zlib encoder, independently of the generated Biscuit payloads.
#include <assert.h>
#include <limits.h>
#include <initializer_list>
#include <stdio.h>
#include <string.h>
#include <zlib.h>
#include "gfx565.h"
#include "packed_image.h"

namespace {
constexpr uint16_t BACKGROUND = 0xA55A, GUARD = 0xDEAD;
struct Surface {
  uint16_t before[16], pixels[gfx565::W * gfx565::H], after[16];
  void reset() {
    for (uint16_t& v : before) v = GUARD;
    for (uint16_t& v : pixels) v = BACKGROUND;
    for (uint16_t& v : after) v = GUARD;
    gfx565::target(pixels);
  }
  void guards() const {
    for (uint16_t v : before) assert(v == GUARD);
    for (uint16_t v : after) assert(v == GUARD);
  }
};
static Surface first, second;
static uint8_t raw[gfx565::W * gfx565::H * 2], compressed[500000];

// A row/column-dependent pattern makes byte order, row stride and nearest-neighbor errors visible.
uint16_t color(int x, int y) { return (uint16_t)((x * 1583) ^ (y * 4051) ^ (x * y * 17)); }
packed565::Image fixture(int width, int height, int strategy = Z_DEFAULT_STRATEGY, int windowBits = 15) {
  for (int y = 0; y < height; y++)
    for (int x = 0; x < width; x++) {
      uint16_t c = color(x, y);
      raw[(y * width + x) * 2] = (uint8_t)c;
      raw[(y * width + x) * 2 + 1] = (uint8_t)(c >> 8);
    }
  z_stream encoder = {};
  assert(deflateInit2(&encoder, 9, Z_DEFLATED, windowBits, 8, strategy) == Z_OK);
  encoder.next_in = raw;
  encoder.avail_in = (uInt)(width * height * 2);
  encoder.next_out = compressed;
  encoder.avail_out = sizeof compressed;
  assert(deflate(&encoder, Z_FINISH) == Z_STREAM_END);
  uint32_t count = (uint32_t)encoder.total_out;
  assert(deflateEnd(&encoder) == Z_OK);
  return {(uint16_t)width, (uint16_t)height, compressed, count};
}

void expected(const Surface& surface, const packed565::Image& image, int ox, int oy, int scale) {
  surface.guards();
  for (int y = 0; y < gfx565::H; y++)
    for (int x = 0; x < gfx565::W; x++) {
      int64_t sx = (int64_t)x - ox, sy = (int64_t)y - oy;
      uint16_t c = BACKGROUND;
      if (sx >= 0 && sy >= 0 && sx < (int64_t)image.width * scale && sy < (int64_t)image.height * scale)
        c = color((int)(sx / scale), (int)(sy / scale));
      assert(surface.pixels[y * gfx565::W + x] == c);
    }
}

void render(const packed565::Image& image, int x, int y, int scale) {
  first.reset();
  assert(packed565::draw(image, x, y, scale));
  expected(first, image, x, y, scale);
  second.reset();
  assert(packed565::draw(image, x, y, scale));
  expected(second, image, x, y, scale);
  assert(memcmp(first.pixels, second.pixels, sizeof first.pixels) == 0);
}

void pixels() {
  const int positions[][2] = {{0, 0}, {17, 31}, {-1, -1}, {-8, 5}, {7, -4}, {477, 479},
                             {-2000, 0}, {0, -2000}, {480, 0}, {0, 480},
                             {INT_MIN, INT_MIN}, {INT_MAX, INT_MAX}};
  for (int strategy : {Z_DEFAULT_STRATEGY, Z_FIXED, Z_HUFFMAN_ONLY}) {
    packed565::Image image = fixture(7, 5, strategy);
    for (int scale : {1, 2, 3, 7, 480})
      for (const auto& at : positions) render(image, at[0], at[1], scale);
  }
  render(fixture(1, 1), 479, 479, 1);
  render(fixture(480, 480), 0, 0, 1);    // Maximum accepted dimensions; many dictionary-sized output chunks.
  render(fixture(257, 66, Z_DEFAULT_STRATEGY, 14), -53, -8, 3); // Row boundaries and dictionary boundaries deliberately disagree.
}

void rejected(const packed565::Image& image, int scale = 1) {
  first.reset();
  assert(!packed565::draw(image, 0, 0, scale));
  first.guards(); // Corrupt data may be partly rendered, but never reaches outside the target.
}

void metadata() {
  packed565::Image image = fixture(7, 5), bad = image;
  bad.data = nullptr; rejected(bad);
  bad = image; bad.byteCount = 0; rejected(bad);
  bad.byteCount = UINT32_MAX; rejected(bad);
  bad = image; bad.width = 0; rejected(bad);
  bad = image; bad.height = 0; rejected(bad);
  bad = image; bad.width = 481; rejected(bad);
  bad = image; bad.height = 481; rejected(bad);
  bad = image; bad.width = UINT16_MAX; rejected(bad);
  bad = image; bad.height = UINT16_MAX; rejected(bad);
  for (int scale : {0, -1, INT_MIN, 481, INT_MAX}) rejected(image, scale);
  gfx565::target(nullptr);
  assert(!packed565::draw(image, 0, 0, 1));
  first.reset();
  for (uint16_t v : first.pixels) assert(v == BACKGROUND);
}

void corruption() {
  packed565::Image image = fixture(7, 5), bad = image;
  // Every prefix is incomplete, including an empty/header-only stream and a partial checksum.
  for (uint32_t n = 0; n < image.byteCount; n++) {
    bad.byteCount = n;
    rejected(bad);
  }
  bad = image; bad.width--; rejected(bad);   // Decompressed output exceeds the declared image.
  bad = image; bad.width++; rejected(bad);   // Complete zlib stream, fewer pixels than declared.
  bad = image; bad.height++; rejected(bad);
  // Exact consumption is required: accepting a valid prefix would conceal a damaged asset table.
  bad = image; compressed[bad.byteCount++] = 0; rejected(bad);
  // A damaged header, payload or Adler checksum must fail even when its prefix rendered successfully.
  for (uint32_t at : {0u, 1u, image.byteCount / 2, image.byteCount - 1}) {
    compressed[at] ^= 0xFF;
    rejected(image);
    compressed[at] ^= 0xFF;
  }
  render(image, 3, 4, 3); // A failed decode must leave no state that poisons the next image.
}
} // namespace

int main() {
  pixels();
  metadata();
  corruption();
  puts("packed images: all checks passed");
}
