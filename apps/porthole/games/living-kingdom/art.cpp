#include "art.h"
#include "gfx565.h"
#include "generated/fonts.h"
#include "packed_image.h"

namespace kingdom { namespace art {
using PackedImage = packed565::Image;
}}
#include "generated/art_data.inc"
#include "generated/icon.h"
#include "generated/art_skins.inc"

namespace kingdom { namespace art {
namespace {
struct Origin { int x, y, scale; };
void backdrop(uint8_t id, const Origin& at, uint8_t changes) {
  uint8_t outcome = changes & 15;
  if (outcome > 4) outcome = 0;
  packed565::draw(*SCENES[id][outcome + ((changes & 128) ? 5 : 0)], at.x, at.y, at.scale);
}
}  // namespace
void scene(uint8_t location, int x, int y, int scale, uint8_t changes) {
  if (scale > 0 && scale <= 240) backdrop(location < 6 ? location : 0, {x, y, scale * 2}, changes);
}
void map(int x, int y, int scale, uint8_t changes) {
  if (scale > 0 && scale <= 240) backdrop(6, {x, y, scale * 2}, changes);
}
void portrait(uint8_t character, int x, int y, int scale) {
  packed565::draw(*PORTRAITS[character < 3 ? character : 0], x, y, scale);
}
void item(uint8_t id, int x, int y, int scale) {
  if (id >= 1 && id <= 6) packed565::draw(*ITEMS[id - 1], x, y, scale);
}
void landscape(int x, int y, int scale) {
  if (scale > 0 && scale <= 240) packed565::draw(*LANDSCAPE, x, y, scale * 2);
}
void camp(int x, int y, int scale) {
  if (scale > 0 && scale <= 240) packed565::draw(*CAMP, x, y, scale * 2);
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
