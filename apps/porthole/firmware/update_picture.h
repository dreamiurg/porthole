// Sparse RGB565 picture: DMA reuses black rows around the update text and its way back.
#pragma once
#include <stdint.h>
#include <string.h>
#include "os/font8x8_basic.h"

namespace board {
struct UpdatePicture {
  static constexpr int W = 480, H = 480, CHUNK_ROWS = 4, TEXT_ROWS = 24;
  static constexpr int PROGRESS_Y = 216, BACK_Y = 312;
  static constexpr int CHUNK_BYTES = W * CHUNK_ROWS * sizeof(uint16_t), CHUNKS = H / CHUNK_ROWS;
  static constexpr int STRIP_CHUNKS = TEXT_ROWS / CHUNK_ROWS, PIECES = 1 + 3 * STRIP_CHUNKS;
  static constexpr int BACK_X = 50, BACK_TOP = 98, BACK_W = 60, BACK_H = 22;
  using Strip = uint16_t* [STRIP_CHUNKS];   // a text strip, one DMA chunk per four rows

  // Every chunk is its own CHUNK_BYTES allocation: a fragmented heap long since out of one 73 KB internal block
  // still has 19 of 3.8 KB, and the update screen must come up or no update ever installs.
  uint16_t* black;
  Strip progress[2];
  Strip back;

  // f(uint16_t*&) on each of the PIECES chunk pointers: allocate or free them.
  template <class F> void eachChunk(F f) {
    f(black);
    for (Strip* s : {&progress[0], &progress[1], &back})
      for (uint16_t*& c : *s) f(c);
  }

  static uint16_t* row(const Strip strip, int y) { return strip[y / CHUNK_ROWS] + (y % CHUNK_ROWS) * W; }

  // y is the first physical row of a four-row DMA chunk.
  uint16_t* chunk(int y, int bank) {
    if (y >= PROGRESS_Y && y < PROGRESS_Y + TEXT_ROWS) return row(progress[bank], y - PROGRESS_Y);
    if (y >= BACK_Y && y < BACK_Y + TEXT_ROWS) return row(back, y - BACK_Y);
    return black;
  }

  void drawProgress(int bank, const char* label) { drawText(progress[bank], label); }
  void drawBack() { drawText(back, "Cancel"); }

  static bool hitBack(int x, int y) {
    return x >= BACK_X && x < BACK_X + BACK_W && y >= BACK_TOP && y < BACK_TOP + BACK_H;
  }

  static void drawGlyph(const Strip strip, int x, unsigned char ch) {
    if (ch >= 128) ch = '?';
    for (int r = 0; r < 8; r++) {
      for (int col = 0; col < 8; col++) {
        if (!(font8x8_basic[ch][r] & (1u << col))) continue;
        for (int dy = 0; dy < 3; dy++) {
          for (int dx = 0; dx < 3; dx++) row(strip, r * 3 + dy)[x + col * 3 + dx] = 0xffff;
        }
      }
    }
  }

  static void drawText(const Strip strip, const char* label) {
    for (int i = 0; i < STRIP_CHUNKS; i++) memset(strip[i], 0, CHUNK_BYTES);
    int length = 0;
    while (length < W / 24 && label[length]) length++;
    int x = (W - length * 24) / 2;
    for (int i = 0; i < length; i++) drawGlyph(strip, x + i * 24, (unsigned char)label[i]);
  }
};

static_assert(UpdatePicture::CHUNK_BYTES <= 4092 && UpdatePicture::CHUNK_BYTES % 64 == 0,
              "Each sparse DMA chunk must fit one aligned descriptor");
static_assert(UpdatePicture::PROGRESS_Y % UpdatePicture::CHUNK_ROWS == 0 &&
              UpdatePicture::BACK_Y % UpdatePicture::CHUNK_ROWS == 0 &&
              UpdatePicture::TEXT_ROWS % UpdatePicture::CHUNK_ROWS == 0,
              "Text stripes must start and end at DMA chunk boundaries");
}  // namespace board
