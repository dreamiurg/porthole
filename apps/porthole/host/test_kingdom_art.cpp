// Include Kingdom asset tables to verify the shared decoder against every original stream.
#include <algorithm>
#include "host/check.h"
#include <climits>
#include <stdio.h>
#include <random>
#include <vector>
#include <zlib.h>
#include "games/living-kingdom/art.cpp"

namespace {
namespace art = kingdom::art;
constexpr uint16_t CLEAR = 0x1357;
constexpr size_t PIXELS = gfx565::W * gfx565::H;
uint16_t guarded[PIXELS + 2];

void reset() {
  std::fill(guarded, guarded + PIXELS + 2, CLEAR);
  gfx565::target(guarded + 1);
}
void guards() { assert(guarded[0] == CLEAR && guarded[PIXELS + 1] == CLEAR); }
void unchanged() {
  guards();
  assert(std::all_of(guarded + 1, guarded + PIXELS + 1, [](uint16_t p) { return p == CLEAR; }));
}
std::vector<uint8_t> compressed(const std::vector<uint8_t>& raw) {
  uLongf size = compressBound(raw.size());
  std::vector<uint8_t> bytes(size);
  assert(compress2(bytes.data(), &size, raw.data(), raw.size(), 9) == Z_OK);
  bytes.resize(size); return bytes;
}
art::PackedImage image(const std::vector<uint8_t>& bytes) {
  return {2, 2, bytes.data(), static_cast<uint32_t>(bytes.size())};
}
void invalidArguments(const std::vector<uint8_t>& bytes) {
  const auto good = image(bytes);
  const art::PackedImage bad[] = {{2, 2, nullptr, 2}, {2, 2, bytes.data(), 0},
    {0, 2, bytes.data(), good.byteCount}, {2, 0, bytes.data(), good.byteCount},
    {481, 1, bytes.data(), good.byteCount}, {1, 481, bytes.data(), good.byteCount},
    {2, 2, bytes.data(), 2000001}};
  for (const auto& invalid : bad) { assert(!packed565::draw(invalid, 0, 0, 1)); unchanged(); }
  for (int scale : {0, -1, 481}) { assert(!packed565::draw(good, 0, 0, scale)); unchanged(); }
  for (int position : {INT_MIN, INT_MAX}) {
    assert(packed565::draw(good, position, position, 1)); unchanged();
  }
}
void malformedStreams(const std::vector<uint8_t>& bytes) {
  for (uint32_t size = 1; size < bytes.size(); size++) {
    reset(); assert(!packed565::draw({2, 2, bytes.data(), size}, 0, 0, 1)); guards();
  }
  const std::vector<uint8_t> invalid[] = {
    compressed({1, 2, 3, 4, 5, 6}), compressed({1, 2, 3, 4, 5, 6, 7, 8, 9})};
  for (const auto& data : invalid) { reset(); assert(!packed565::draw(image(data), 0, 0, 1)); guards(); }
  auto trailing = bytes; trailing.push_back(0);
  reset(); assert(!packed565::draw(image(trailing), 0, 0, 1)); guards();
  auto checksum = bytes; checksum.back() ^= 1;
  reset(); assert(!packed565::draw(image(checksum), 0, 0, 1)); guards();
}
void clippedPixels(const std::vector<uint8_t>& bytes, int x, int y, int scale) {
  const uint16_t source[] = {0x1234, 0xabcd, 0xf81f, 0x07e0};
  reset(); assert(packed565::draw(image(bytes), x, y, scale)); guards();
  for (int py = 0; py < gfx565::H; py++) for (int px = 0; px < gfx565::W; px++) {
    uint16_t expected = CLEAR;
    if (px >= x && py >= y && px < x + 2 * scale && py < y + 2 * scale)
      expected = source[(py - y) / scale * 2 + (px - x) / scale];
    assert(gfx565::fb[py * gfx565::W + px] == expected);
  }
}
void randomStreams() {
  std::mt19937 random(74363);
  for (int trial = 0; trial < 5000; trial++) {
    std::vector<uint8_t> fuzz(1 + random() % 512);
    for (auto& byte : fuzz) byte = static_cast<uint8_t>(random());
    const art::PackedImage invalid = {static_cast<uint16_t>(1 + random() % 16),
      static_cast<uint16_t>(1 + random() % 16), fuzz.data(), static_cast<uint32_t>(fuzz.size())};
    reset(); assert(!packed565::draw(invalid, 0, 0, 1)); guards();
  }
}
void assertSourcePixels(const art::PackedImage& source, int scale) {
  // The generator independently proves each packed stream equals its authoritative source crop.
  // A one-shot reference inflate checks the production row sink across its 32KiB boundaries.
  uLongf size = static_cast<uLongf>(source.width) * source.height * 2;
  std::vector<uint8_t> raw(size);
  assert(uncompress(raw.data(), &size, source.data, source.byteCount) == Z_OK && size == raw.size());
  guards();
  for (int y = 0; y < gfx565::H; y++) for (int x = 0; x < gfx565::W; x++) {
    uint16_t expected = CLEAR;
    if (x < source.width * scale && y < source.height * scale) {
      const size_t offset = static_cast<size_t>(y / scale * source.width + x / scale) * 2;
      expected = static_cast<uint16_t>(raw[offset] | raw[offset + 1] << 8);
    }
    assert(gfx565::fb[y * gfx565::W + x] == expected);
  }
}
void realAssets() {
  std::vector<const art::PackedImage*> sources = {art::LANDSCAPE, art::CAMP};
  for (const auto& states : art::SCENES) for (const auto* source : states)
    if (std::find(sources.begin(), sources.end(), source) == sources.end()) sources.push_back(source);
  for (const auto* source : art::PORTRAITS) sources.push_back(source);
  for (const auto* source : art::ITEMS) sources.push_back(source);
  assert(sources.size() >= 15);
  for (const auto* source : sources) {
    reset(); assert(packed565::draw(*source, 0, 0, 1)); assertSourcePixels(*source, 1);
  }
}
void publicPixels() {
  const uint8_t states[] = {0, 1, 2, 3, 4, 128, 129, 130, 131, 132};
  for (uint8_t location = 0; location < 7; location++) for (int i = 0; i < 10; i++) {
    reset();
    if (location == 6) art::map(0, 0, 1, states[i]);
    else art::scene(location, 0, 0, 1, states[i]);
    assert(art::SCENES[location][i]->width * 2 == (location == 6 ? art::MAP_W : art::SCENE_W));
    assert(art::SCENES[location][i]->height * 2 == (location == 6 ? art::MAP_H : art::SCENE_H));
    assertSourcePixels(*art::SCENES[location][i], 2);
  }
  reset(); art::landscape(); assertSourcePixels(*art::LANDSCAPE, 2);
  reset(); art::camp(); assertSourcePixels(*art::CAMP, 2);
  assert(art::LANDSCAPE->width * 2 == art::LANDSCAPE_W && art::LANDSCAPE->height * 2 == art::LANDSCAPE_H);
  assert(art::CAMP->width * 2 == art::SCENE_W && art::CAMP->height * 2 == art::SCENE_H);
  for (uint8_t i = 0; i < 3; i++) {
    reset(); art::portrait(i, 0, 0); assertSourcePixels(*art::PORTRAITS[i], 1);
    assert(art::PORTRAITS[i]->width == art::PORTRAIT_W && art::PORTRAITS[i]->height == art::PORTRAIT_H);
  }
  for (uint8_t i = 0; i < 6; i++) {
    reset(); art::item(i + 1, 0, 0); assertSourcePixels(*art::ITEMS[i], 1);
    assert(art::ITEMS[i]->width == art::ITEM_W && art::ITEMS[i]->height == art::ITEM_H);
  }
}
void nativeSkin(art::PanelStyle style, const uint16_t* source) {
  reset(); art::panel(10, 20, 48, 48, style); guards();
  for (int y = 0; y < gfx565::H; y++) for (int x = 0; x < gfx565::W; x++) {
    const bool inside = x >= 10 && y >= 20 && x < 58 && y < 68;
    assert(gfx565::fb[y * gfx565::W + x] == (inside ? source[(y - 20) * 48 + x - 10] : CLEAR));
  }
}
void resizedSkin(art::PanelStyle style, const uint16_t* source) {
  reset(); art::panel(10, 10, 120, 80, style); guards();
  for (int cornerY : {0, 1}) for (int cornerX : {0, 1})
    for (int y = 0; y < 6; y++) for (int x = 0; x < 6; x++)
      assert(gfx565::fb[(10 + cornerY * 74 + y) * gfx565::W + 10 + cornerX * 114 + x]
        == source[(cornerY * 42 + y) * 48 + cornerX * 42 + x]);
  assert(gfx565::fb[16 * gfx565::W + 16] == source[6 * 48 + 6]);
  assert(gfx565::fb[52 * gfx565::W + 52] == source[6 * 48 + 6]);
  const std::vector<uint16_t> whole(gfx565::fb, gfx565::fb + PIXELS);
  reset(); art::panel(-20, -10, 120, 80, style); guards();
  for (int y = 0; y < gfx565::H; y++) for (int x = 0; x < gfx565::W; x++) {
    const uint16_t expected = x < 100 && y < 70 ? whole[(y + 20) * gfx565::W + x + 30] : CLEAR;
    assert(gfx565::fb[y * gfx565::W + x] == expected);
  }
  reset(); art::panel(0, 0, 5, 4, style); guards();
  assert(gfx565::fb[gfx565::W + 2] == source[12 * 48 + 19]);
  assert(gfx565::fb[3 * gfx565::W + 4] == source[36 * 48 + 38]);
}
void skinArguments() {
  reset();
  for (int size : {0, -1, 481}) {
    art::panel(0, 0, size, 48, art::PanelStyle::PAPER);
    art::panel(0, 0, 48, size, art::PanelStyle::PAPER); unchanged();
  }
  art::panel(0, 0, 48, 48, static_cast<art::PanelStyle>(255)); unchanged();
  for (int position : {INT_MIN, INT_MAX}) {
    art::panel(position, position, 48, 48, art::PanelStyle::PAPER); unchanged();
  }
}
void panelSkins() {
  for (int i = 0; i < 4; i++) {
    const auto style = static_cast<art::PanelStyle>(i);
    nativeSkin(style, art::SKINS[i]); resizedSkin(style, art::SKINS[i]);
  }
  skinArguments();
}
}  // namespace

int main() {
  const auto bytes = compressed({0x34, 0x12, 0xcd, 0xab, 0x1f, 0xf8, 0xe0, 0x07});
  reset(); invalidArguments(bytes); malformedStreams(bytes);
  for (int scale : {1, 2, 3, 480}) for (int position : {-3, 0, 479, 480})
    clippedPixels(bytes, position, position, scale);
  clippedPixels(bytes, -1, 479, 2); clippedPixels(bytes, 479, -1, 2);
  randomStreams(); realAssets(); publicPixels(); panelSkins();
  puts("kingdom art: source pixels, public sizing, nine-slice skins, clipping, malformed streams and 5000 fuzz cases passed");
}
