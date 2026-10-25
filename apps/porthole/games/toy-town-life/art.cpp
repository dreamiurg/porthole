#include "art.h"
#include <algorithm>
#include "gfx565.h"

namespace toytown { namespace art {
#include "generated/art_data.inc"
namespace {
uint8_t unpack(const uint8_t* data, int index, int bits) {
  int bit = index * bits, offset = bit & 7, byte = bit >> 3;
  unsigned value = static_cast<unsigned>(data[byte]) << 8;
  if (offset + bits > 8) value |= data[byte + 1];
  return (value >> (16 - offset - bits)) & ((1u << bits) - 1);
}
uint16_t shade(uint32_t color, int light) {
  uint32_t r = ((color >> 16) & 255) * light / 255;
  uint32_t g = ((color >> 8) & 255) * light / 255;
  uint32_t b = (color & 255) * light / 255;
  return rgb888_to_565((r << 16) | (g << 8) | b);
}
int pattern(const State& s, int pixel) {
  int x = pixel % 96, y = pixel / 96;
  if (s.outfit == 1 && y % 7 < 2) return 205;
  if (s.outfit == 2 && (x % 9 < 2 || y % 9 < 2)) return 180;
  return 255;
}
uint16_t recolor(uint16_t source, int material, const State& s, int pixel) {
  static const uint32_t skin[] = {0xffd7b8, 0xffd7b8, 0xba7c55, 0x805238};
  static const uint32_t cloth[] = {0xef9fa8, 0x72bec9, 0xb5a2d8, 0xe7be6d};
  static const uint32_t shoe[] = {0x487ba1, 0xb65f62, 0x765344};
  uint32_t color = gfx565::rgb888(source);
  int light = 140 + static_cast<int>(std::max({(color >> 16) & 255, (color >> 8) & 255, color & 255})) * 115 / 255;
  if (material == 2) return s.skin ? shade(skin[s.skin % 4], light) : source;
  if (material == 4) return s.shoes ? shade(shoe[s.shoes % 3], light) : source;
  if (material != 3) return source;
  int textile = pattern(s, pixel);
  if (s.color) return shade(cloth[s.color % 4], light * textile / 255);
  return shade(color, textile);
}
void dot(int x, int y, int radius, uint16_t color, int scale) {
  gfx565::circle(x, y, radius * scale, color);
}
void hairShape(const State& s, int x, int y, int scale) {
  if (!s.hair) return;
  static const uint32_t hair[] = {0x9b5d42, 0xbc9ac9, 0x34485f, 0x5790b5};
  uint16_t dark = shade(hair[s.doll % 4], 110), fill = shade(hair[s.doll % 4], 230);
  int cy = y + (s.hair == 1 ? 30 : 62) * scale;
  int radius = s.hair == 1 ? 12 : 18;
  for (int side = 0; side < 2; ++side) {
    int cx = x + (side ? 77 : 19) * scale;
    dot(cx, cy, radius, dark, scale);
    dot(cx, cy - scale, radius - 2, fill, scale);
    dot(cx - 3 * scale, cy - 5 * scale, 3, shade(hair[s.doll % 4], 255), scale);
  }
}
void accessories(const State& s, int x, int y, int scale) {
  uint16_t ink = rgb888_to_565(0x4b3e58), gold = rgb888_to_565(0xf2c36c);
  if (s.accessory == 1) {
    static const int eyes[] = {60, 78, 59, 65};
    int eyeY = y + eyes[s.doll % 4] * scale;
    for (int dx : {35, 61}) {
      gfx565::frame(x + (dx - 10) * scale, eyeY - 8 * scale, 20 * scale, 16 * scale, ink);
      gfx565::frame(x + (dx - 9) * scale, eyeY - 7 * scale, 18 * scale, 14 * scale, ink);
    }
    gfx565::hline(x + 45 * scale, eyeY, 7 * scale, ink);
  }
  if (s.accessory == 2) {
    for (int dx : {-4, 4}) for (int dy : {-4, 4})
      dot(x + (72 + dx) * scale, y + (36 + dy) * scale, 4, rgb888_to_565(0xf6bdcd), scale);
    dot(x + 72 * scale, y + 36 * scale, 3, gold, scale);
  }
}
}  // namespace

void background(int scene) {
  scene = std::max(0, std::min(4, scene));
  const uint8_t* pixels = SCENES[scene];
  const uint16_t* palette = SCENE_PALETTES[scene];
  for (int y = 0; y < 240; ++y) {
    uint16_t* row = gfx565::fb + y * 960;
    for (int x = 0; x < 240; ++x) {
      uint16_t color = palette[unpack(pixels, y * 240 + x, 6)];
      row[x * 2] = row[x * 2 + 1] = color;
      row[480 + x * 2] = row[481 + x * 2] = color;
    }
  }
}
void doll(const State& s, int x, int y, int scale) {
  scale = std::max(1, std::min(3, scale));
  int id = s.doll % 4;
  hairShape(s, x, y, scale);
  for (int pixel = 0; pixel < 96 * 160; ++pixel) {
    int index = unpack(DOLLS[id], pixel, 6);
    if (index == 63) continue;
    uint16_t color = recolor(DOLL_PALETTES[id][index], unpack(MATERIALS[id], pixel, 3), s, pixel);
    gfx565::rect(x + (pixel % 96) * scale, y + (pixel / 96) * scale, scale, scale, color);
  }
  accessories(s, x, y, scale);
}
const gfx::Sprite& icon() { return ICON; }
}}  // namespace toytown::art
