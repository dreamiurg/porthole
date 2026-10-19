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
  static constexpr int BACK_X = 50, BACK_TOP = 98, BACK_W = 60, BACK_H = 22;

  alignas(64) uint16_t black[W * CHUNK_ROWS];
  alignas(64) uint16_t progress[2][W * TEXT_ROWS];
  alignas(64) uint16_t back[W * TEXT_ROWS];

  // y is the first physical row of a four-row DMA chunk.
  uint16_t* chunk(int y, int bank) {
    if (y >= PROGRESS_Y && y < PROGRESS_Y + TEXT_ROWS) return progress[bank] + (y - PROGRESS_Y) * W;
    if (y >= BACK_Y && y < BACK_Y + TEXT_ROWS) return back + (y - BACK_Y) * W;
    return black;
  }

  void drawProgress(int bank, const char* label) { drawText(progress[bank], label); }
  void drawBack() { drawText(back, "Cancel"); }

  static bool hitBack(int x, int y) {
    return x >= BACK_X && x < BACK_X + BACK_W && y >= BACK_TOP && y < BACK_TOP + BACK_H;
  }

  static void drawGlyph(uint16_t* stripe, int x, unsigned char ch) {
    if (ch >= 128) ch = '?';
    for (int row = 0; row < 8; row++) {
      for (int col = 0; col < 8; col++) {
        if (!(font8x8_basic[ch][row] & (1u << col))) continue;
        for (int dy = 0; dy < 3; dy++) {
          for (int dx = 0; dx < 3; dx++) stripe[(row * 3 + dy) * W + x + col * 3 + dx] = 0xffff;
        }
      }
    }
  }

  static void drawText(uint16_t* stripe, const char* label) {
    memset(stripe, 0, W * TEXT_ROWS * sizeof(uint16_t));
    int length = 0;
    while (length < W / 24 && label[length]) length++;
    int x = (W - length * 24) / 2;
    for (int i = 0; i < length; i++) drawGlyph(stripe, x + i * 24, (unsigned char)label[i]);
  }
};

static_assert(UpdatePicture::CHUNK_BYTES <= 4092 && UpdatePicture::CHUNK_BYTES % 64 == 0,
              "Each sparse DMA chunk must fit one aligned descriptor");
static_assert(UpdatePicture::PROGRESS_Y % UpdatePicture::CHUNK_ROWS == 0 &&
              UpdatePicture::BACK_Y % UpdatePicture::CHUNK_ROWS == 0 &&
              UpdatePicture::TEXT_ROWS % UpdatePicture::CHUNK_ROWS == 0,
              "Text stripes must start and end at DMA chunk boundaries");
}  // namespace board
