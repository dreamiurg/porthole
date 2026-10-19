// Physical sparse scanout and the visible progress/back contract, without ESP32 headers.
#include <assert.h>
#include <stdio.h>
#include <string.h>
#include <type_traits>
#include <vector>
#include "firmware/update_picture.h"

using board::UpdatePicture;

static bool inCircle(int x, int y, int center, int radius) {
  int dx = x - center, dy = y - center;
  return dx * dx + dy * dy <= radius * radius;
}

static void checkInk(const uint16_t* stripe, int top) {
  int ink = 0;
  for (int y = 0; y < UpdatePicture::TEXT_ROWS; y++) {
    for (int x = 0; x < UpdatePicture::W; x++) {
      uint16_t pixel = stripe[y * UpdatePicture::W + x];
      assert(pixel == 0 || pixel == 0xffff);
      if (!pixel) continue;
      assert(inCircle(x, top + y, 240, 240));
      ink++;
    }
  }
  assert(ink > 0);
}

static void checkLabels() {
  UpdatePicture picture{};
  picture.drawBack();
  std::vector<uint16_t> back(picture.back, picture.back + UpdatePicture::W * UpdatePicture::TEXT_ROWS);
  for (int percent = 0; percent <= 100; percent++) {
    char label[32];
    snprintf(label, sizeof label, "Downloading %d%%", percent);
    picture.drawProgress(0, label);
    checkInk(picture.progress[0], UpdatePicture::PROGRESS_Y);
    for (uint16_t pixel : picture.progress[1]) assert(pixel == 0);
    assert(memcmp(picture.back, back.data(), sizeof picture.back) == 0);
  }
  const char* labels[] = {"Checking...", "Verifying...", "Stopping..."};
  std::vector<uint16_t> saved(picture.progress[0], picture.progress[0] + UpdatePicture::W * UpdatePicture::TEXT_ROWS);
  for (const char* label : labels) {
    picture.drawProgress(1, label);
    checkInk(picture.progress[1], UpdatePicture::PROGRESS_Y);
    assert(memcmp(picture.progress[0], saved.data(), sizeof picture.progress[0]) == 0);
  }
  picture.drawProgress(1, "");
  for (uint16_t pixel : picture.progress[1]) assert(pixel == 0);
  checkInk(picture.back, UpdatePicture::BACK_Y);
}

static void checkBack() {
  assert(UpdatePicture::BACK_W >= 24 && UpdatePicture::BACK_H >= 22);
  for (int y : {UpdatePicture::BACK_TOP, UpdatePicture::BACK_TOP + UpdatePicture::BACK_H - 1}) {
    for (int x : {UpdatePicture::BACK_X, UpdatePicture::BACK_X + UpdatePicture::BACK_W - 1}) {
      assert(UpdatePicture::hitBack(x, y));
      assert(inCircle(x, y, 80, 80));
    }
  }
  assert(!UpdatePicture::hitBack(UpdatePicture::BACK_X - 1, UpdatePicture::BACK_TOP));
  assert(!UpdatePicture::hitBack(UpdatePicture::BACK_X + UpdatePicture::BACK_W, UpdatePicture::BACK_TOP));
  assert(!UpdatePicture::hitBack(UpdatePicture::BACK_X, UpdatePicture::BACK_TOP - 1));
  assert(!UpdatePicture::hitBack(UpdatePicture::BACK_X, UpdatePicture::BACK_TOP + UpdatePicture::BACK_H));
  UpdatePicture picture{};
  picture.drawBack();
  for (int y = 0; y < UpdatePicture::TEXT_ROWS; y++) {
    for (int x = 0; x < UpdatePicture::W; x++) {
      if (picture.back[y * UpdatePicture::W + x]) assert(UpdatePicture::hitBack(x / 3, (UpdatePicture::BACK_Y + y) / 3));
    }
  }
}

static std::vector<uint16_t> frame(UpdatePicture& picture, int bank) {
  std::vector<uint16_t> pixels(UpdatePicture::W * UpdatePicture::H);
  int chunks = 0;
  for (int y = 0; y < UpdatePicture::H; y += UpdatePicture::CHUNK_ROWS) {
    uint16_t* chunk = picture.chunk(y, bank);
    assert((uintptr_t)chunk % 64 == 0);
    memcpy(pixels.data() + y * UpdatePicture::W, chunk, UpdatePicture::CHUNK_BYTES);
    chunks++;
  }
  assert(chunks == 120 && chunks * UpdatePicture::CHUNK_BYTES == 480 * 480 * 2);
  return pixels;
}

static void checkChunkPointers(UpdatePicture& picture, int bank) {
  for (int y = 0; y < UpdatePicture::H; y += UpdatePicture::CHUNK_ROWS) {
    uint16_t* expected = picture.black;
    if (y >= UpdatePicture::PROGRESS_Y && y < UpdatePicture::PROGRESS_Y + UpdatePicture::TEXT_ROWS)
      expected = picture.progress[bank] + (y - UpdatePicture::PROGRESS_Y) * UpdatePicture::W;
    if (y >= UpdatePicture::BACK_Y && y < UpdatePicture::BACK_Y + UpdatePicture::TEXT_ROWS)
      expected = picture.back + (y - UpdatePicture::BACK_Y) * UpdatePicture::W;
    assert(picture.chunk(y, bank) == expected);
  }
  assert(picture.chunk(0, bank) == picture.black);
  assert(picture.chunk(476, bank) == picture.black);
  assert(picture.chunk(UpdatePicture::PROGRESS_Y, bank) == picture.progress[bank]);
  assert(picture.chunk(UpdatePicture::BACK_Y, bank) == picture.back);
}

static void checkFramePixels(UpdatePicture& picture, int bank) {
  auto pixels = frame(picture, bank);
  for (int y = 0; y < UpdatePicture::H; y++) {
    for (int x = 0; x < UpdatePicture::W; x++) {
      uint16_t expected = 0;
      if (y >= UpdatePicture::PROGRESS_Y && y < UpdatePicture::PROGRESS_Y + UpdatePicture::TEXT_ROWS)
        expected = picture.progress[bank][(y - UpdatePicture::PROGRESS_Y) * UpdatePicture::W + x];
      if (y >= UpdatePicture::BACK_Y && y < UpdatePicture::BACK_Y + UpdatePicture::TEXT_ROWS)
        expected = picture.back[(y - UpdatePicture::BACK_Y) * UpdatePicture::W + x];
      assert(pixels[y * UpdatePicture::W + x] == expected);
    }
  }
}

static void checkScanout() {
  UpdatePicture picture{};
  for (uint16_t pixel : frame(picture, 0)) assert(pixel == 0);
  for (uint16_t pixel : frame(picture, 1)) assert(pixel == 0);
  for (int i = 0; i < UpdatePicture::W * UpdatePicture::TEXT_ROWS; i++) {
    picture.progress[0][i] = (uint16_t)(i + 1);
    picture.progress[1][i] = (uint16_t)(i + 20000);
    picture.back[i] = (uint16_t)(i + 40000);
  }
  for (int bank = 0; bank < 2; bank++) {
    checkChunkPointers(picture, bank);
    checkFramePixels(picture, bank);
  }
}

static void preview(const char* path) {
  UpdatePicture picture{};
  picture.drawProgress(0, "Downloading 42%");
  picture.drawBack();
  auto pixels = frame(picture, 0);
  FILE* output = fopen(path, "wb");
  assert(output);
  fprintf(output, "P6\n480 480\n255\n");
  for (int y = 0; y < 480; y++) {
    for (int x = 0; x < 480; x++) {
      unsigned char value = pixels[y * 480 + x] ? 255 : 0;
      unsigned char rgb[3] = {value, value, value};
      if (!inCircle(x, y, 240, 240)) rgb[0] = rgb[1] = rgb[2] = 32;
      assert(fwrite(rgb, sizeof rgb, 1, output) == 1);
    }
  }
  assert(fclose(output) == 0);
}

int main(int argc, char** argv) {
  static_assert(std::is_trivial<UpdatePicture>::value && std::is_aggregate<UpdatePicture>::value,
                "The board must be able to value-initialize its DMA allocation");
  checkLabels();
  checkBack();
  checkScanout();
  if (argc == 3 && strcmp(argv[1], "--preview") == 0) preview(argv[2]);
  puts("update picture: progress, back target and full sparse scanout passed");
}
