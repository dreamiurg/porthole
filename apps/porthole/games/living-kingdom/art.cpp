#include "art.h"
#include "gfx565.h"
#include "generated/fonts.h"
#ifdef HOST_BUILD
#include <zlib.h>
#else
#include <miniz.h>
#endif

namespace kingdom { namespace art {
struct PackedImage { uint16_t width, height; const uint8_t* data; uint32_t byteCount; };
}}
#include "generated/art_data.inc"
#include "generated/icon.h"
#include "generated/art_skins.inc"

namespace kingdom { namespace art {
namespace {
struct Origin { int x, y, scale; };
// Rendering is serial. The ROM inflater needs one fixed dictionary, never a full-image scratch copy.
static uint8_t inflateBuffer[32768];
struct ImageSink {
  const PackedImage& image;
  Origin at;
  uint32_t total = 0;
  uint16_t row[480] = {};
  bool accept(const uint8_t* data, size_t count) {
    uint32_t expected = (uint32_t)image.width * image.height * 2;
    if (count > expected - total) return false;
    uint32_t stride = image.width * 2;
    for (size_t i = 0; i < count; ++i) {
      uint32_t position = total % stride;
      if (position & 1) row[position / 2] |= (uint16_t)data[i] << 8;
      else row[position / 2] = data[i];
      ++total;
      if (total % stride == 0) {
        int y = (int)(total / stride) - 1;
        gfx565::blit({image.width, 1, row}, at.x, at.y + y * at.scale, at.scale);
      }
    }
    return true;
  }
  bool complete() const { return total == (uint32_t)image.width * image.height * 2; }
};
#ifdef HOST_BUILD
bool inflateImage(const PackedImage& image, ImageSink& sink) {
  z_stream stream = {};
  if (inflateInit(&stream) != Z_OK) return false;
  stream.next_in = const_cast<Bytef*>(image.data);
  stream.avail_in = image.byteCount;
  int status = Z_OK;
  bool valid = true;
  while (status == Z_OK && valid) {
    stream.next_out = inflateBuffer;
    stream.avail_out = sizeof inflateBuffer;
    uInt before = stream.avail_in;
    status = inflate(&stream, Z_NO_FLUSH);
    size_t produced = sizeof inflateBuffer - stream.avail_out;
    valid = sink.accept(inflateBuffer, produced);
    if (status == Z_OK && produced == 0 && before == stream.avail_in) valid = false;
  }
  bool complete = valid && status == Z_STREAM_END && stream.avail_in == 0 && sink.complete();
  inflateEnd(&stream);
  return complete;
}
#else
bool inflateImage(const PackedImage& image, ImageSink& sink) {
  static tinfl_decompressor decoder;
  tinfl_init(&decoder);
  size_t input = 0, output = 0;
  for (;;) {
    size_t available = image.byteCount - input, produced = sizeof inflateBuffer - output;
    tinfl_status status = tinfl_decompress(&decoder, image.data + input, &available,
      inflateBuffer, inflateBuffer + output, &produced, TINFL_FLAG_PARSE_ZLIB_HEADER);
    input += available;
    if (!sink.accept(inflateBuffer + output, produced)) return false;
    output = (output + produced) % sizeof inflateBuffer;
    if (status == TINFL_STATUS_DONE) return input == image.byteCount && sink.complete();
    if (status < 0 || (available == 0 && produced == 0)) return false;
    if (status == TINFL_STATUS_NEEDS_MORE_INPUT && input == image.byteCount) return false;
  }
}
#endif
bool drawImage(const PackedImage& image, const Origin& at) {
  if (!image.data || !image.byteCount || image.byteCount > 2000000) return false;
  if (!image.width || !image.height || image.width > gfx565::W || image.height > gfx565::H) return false;
  if (at.scale <= 0 || at.scale > gfx565::W) return false;
  int64_t right = (int64_t)at.x + image.width * at.scale;
  int64_t bottom = (int64_t)at.y + image.height * at.scale;
  if (at.x >= gfx565::W || at.y >= gfx565::H || right <= 0 || bottom <= 0) return true;
  ImageSink sink{image, at};
  return inflateImage(image, sink);
}
void backdrop(uint8_t id, const Origin& at, uint8_t changes) {
  uint8_t outcome = changes & 15;
  if (outcome > 4) outcome = 0;
  drawImage(*SCENES[id][outcome + ((changes & 128) ? 5 : 0)], at);
}
}  // namespace
void scene(uint8_t location, int x, int y, int scale, uint8_t changes) {
  if (scale > 0 && scale <= 240) backdrop(location < 6 ? location : 0, {x, y, scale * 2}, changes);
}
void map(int x, int y, int scale, uint8_t changes) {
  if (scale > 0 && scale <= 240) backdrop(6, {x, y, scale * 2}, changes);
}
void portrait(uint8_t character, int x, int y, int scale) {
  drawImage(*PORTRAITS[character < 3 ? character : 0], {x, y, scale});
}
void item(uint8_t id, int x, int y, int scale) {
  if (id >= 1 && id <= 6) drawImage(*ITEMS[id - 1], {x, y, scale});
}
void landscape(int x, int y, int scale) {
  if (scale > 0 && scale <= 240) drawImage(*LANDSCAPE, {x, y, scale * 2});
}
void camp(int x, int y, int scale) {
  if (scale > 0 && scale <= 240) drawImage(*CAMP, {x, y, scale * 2});
}
namespace {
int skinCoordinate(int position, int length) {
  constexpr int border = 6, size = 48;
  if (length < border * 2) return position * size / length;
  if (position < border) return position;
  if (position >= length - border) return size - (length - position);
  return border + (position - border) % (size - border * 2);
}
}
void panel(int x, int y, int width, int height, PanelStyle style) {
  if (width <= 0 || height <= 0 || width > 480 || height > 480) return;
  unsigned index = (unsigned)style;
  if (index >= 4) return;
  int64_t right = (int64_t)x + width, bottom = (int64_t)y + height;
  if (x >= 480 || y >= 480 || right <= 0 || bottom <= 0) return;
  for (int row = 0; row < height; ++row) {
    int sourceY = skinCoordinate(row, height);
    for (int col = 0; col < width; ++col) {
      int sourceX = skinCoordinate(col, width);
      gfx565::pixel(x + col, y + row, SKINS[index][sourceY * 48 + sourceX]);
    }
  }
}
const gfx::Sprite& icon() { return ICON; }
}}  // namespace kingdom::art
