#include "packed_image.h"
#include "gfx565.h"
#include <algorithm>
#include <string.h>
#ifdef HOST_BUILD
#include <zlib.h>
#else
#include <miniz.h>
#endif

namespace packed565 {
namespace {
constexpr size_t WINDOW = 32768;
static uint8_t dictionary[WINDOW];
struct Origin { int x, y, scale; };

struct ImageSink {
  const Image& image;
  Origin at;
  uint32_t total = 0;
  size_t filled = 0;
  uint8_t row[gfx565::W * 2] = {};

  void drawRow(int sourceY) const {
    int top = std::max(0, at.y + sourceY * at.scale);
    int bottom = std::min(gfx565::H, at.y + (sourceY + 1) * at.scale);
    if (bottom <= top) return;
    int left = std::max(0, at.x), right = std::min(gfx565::W, at.x + image.width * at.scale);
    uint16_t* target = gfx565::fb + top * gfx565::W;
    int sourceX = (left - at.x) / at.scale;
    for (int x = left; x < right;) {
      uint16_t color = row[sourceX * 2] | (uint16_t)row[sourceX * 2 + 1] << 8;
      int end = std::min(right, at.x + (sourceX + 1) * at.scale);
      std::fill(target + x, target + end, color);
      x = end; ++sourceX;
    }
    for (int y = top + 1; y < bottom; ++y)
      memcpy(gfx565::fb + y * gfx565::W + left, target + left, (size_t)(right - left) * 2);
  }

  bool accept(const uint8_t* data, size_t count) {
    uint32_t expected = (uint32_t)image.width * image.height * 2;
    if (count > expected - total) return false;
    size_t stride = image.width * 2;
    while (count) {
      size_t n = std::min(count, stride - filled);
      memcpy(row + filled, data, n);
      filled += n; total += n; data += n; count -= n;
      if (filled == stride) {
        drawRow((int)(total / stride) - 1);
        filled = 0;
      }
    }
    return true;
  }
  bool complete() const { return total == (uint32_t)image.width * image.height * 2; }
};

#ifdef HOST_BUILD
bool inflateImage(const Image& image, ImageSink& sink) {
  z_stream stream = {};
  if (inflateInit(&stream) != Z_OK) return false;
  stream.next_in = const_cast<Bytef*>(image.data);
  stream.avail_in = image.byteCount;
  int status = Z_OK;
  bool valid = true;
  while (status == Z_OK && valid) {
    stream.next_out = dictionary;
    stream.avail_out = sizeof dictionary;
    uInt before = stream.avail_in;
    status = inflate(&stream, Z_NO_FLUSH);
    size_t produced = sizeof dictionary - stream.avail_out;
    valid = sink.accept(dictionary, produced);
    if (status == Z_OK && produced == 0 && before == stream.avail_in) valid = false;
  }
  bool complete = valid && status == Z_STREAM_END && stream.avail_in == 0 && sink.complete();
  inflateEnd(&stream);
  return complete;
}
#else
static tinfl_decompressor decoder;
static_assert(sizeof decoder + sizeof dictionary + sizeof(ImageSink) < 49152, "bounded decoder scratch");
bool inflateImage(const Image& image, ImageSink& sink) {
  tinfl_init(&decoder);
  size_t input = 0, output = 0;
  for (;;) {
    size_t available = image.byteCount - input, produced = sizeof dictionary - output;
    tinfl_status status = tinfl_decompress(&decoder, image.data + input, &available,
      dictionary, dictionary + output, &produced, TINFL_FLAG_PARSE_ZLIB_HEADER);
    input += available;
    if (!sink.accept(dictionary + output, produced)) return false;
    output = (output + produced) % sizeof dictionary;
    if (status == TINFL_STATUS_DONE) return input == image.byteCount && sink.complete();
    if (status < 0 || (available == 0 && produced == 0)) return false;
    if (status == TINFL_STATUS_NEEDS_MORE_INPUT && input == image.byteCount) return false;
  }
}
#endif
}  // namespace

bool draw(const Image& image, int x, int y, int scale) {
  if (!image.data || !image.byteCount || image.byteCount > 2000000 || !gfx565::fb) return false;
  if (!image.width || !image.height || image.width > gfx565::W || image.height > gfx565::H) return false;
  if (scale <= 0 || scale > gfx565::W) return false;
  int64_t right = (int64_t)x + image.width * scale, bottom = (int64_t)y + image.height * scale;
  if (x >= gfx565::W || y >= gfx565::H || right <= 0 || bottom <= 0) return true;
  ImageSink sink{image, {x, y, scale}};
  return inflateImage(image, sink);
}
}  // namespace packed565
