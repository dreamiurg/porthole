// Original board08 crops and GPT reference edits. Sizes are physical pixels.
#pragma once
#include <stdint.h>
#include "font.h"
#include "gfx.h"

namespace kingdom { namespace art {
constexpr uint16_t NAVY = rgb888_to_565(0x0D0D1E);
constexpr uint16_t INK = rgb888_to_565(0x221C32);
constexpr uint16_t PARCHMENT = rgb888_to_565(0xFFEFC9);
constexpr uint16_t CREAM = rgb888_to_565(0xFFF3D4);
constexpr uint16_t GOLD = rgb888_to_565(0xEEC65C);
constexpr uint16_t GREEN = rgb888_to_565(0x60A463);
constexpr uint16_t RIVER = rgb888_to_565(0x5EA9D6);
constexpr uint16_t RED = rgb888_to_565(0xC9564E);
constexpr uint16_t BORDER = rgb888_to_565(0x71545A);
extern const font::Font BODY;     // Atkinson Hyperlegible 24px, line 30.
extern const font::Font SMALL;    // 22px, line 26.
extern const font::Font TITLE;    // Bold 32px, line 36.
extern const font::Font DETAIL;   // 20px, line 24.
extern const font::Font HEADING;  // Bold 24px, line 28.
extern const font::Font LABEL;    // Bold 22px, line 26.
constexpr int SCENE_W = 480, SCENE_H = 240;
constexpr int MAP_W = 480, MAP_H = 480;
constexpr int PORTRAIT_W = 80, PORTRAIT_H = 112;
constexpr int LANDSCAPE_W = 480, LANDSCAPE_H = 350;
constexpr int ITEM_W = 48, ITEM_H = 48;
// 0 village, 1 bridge, 2 mill, 3 woods, 4 marsh, 5 tower. Invalid ids draw village.
// changes low nibble: 1 mill priority, 2 marsh priority, 3 timed share, 4 shared channel.
// Bit 0x80: repaired bridge. Pass changes only once the consequence has happened.
void scene(uint8_t location, int x = 0, int y = 0, int scale = 1, uint8_t changes = 0);
void map(int x = 0, int y = 0, int scale = 1, uint8_t changes = 0);
// 0 Princess, 1 Lucas, 2 Dribble. Opaque NAVY background. Invalid ids draw Princess.
void portrait(uint8_t character, int x, int y, int scale = 1);
// 1 bread, 2 rope, 3 timber, 4 thyme, 5 flour, 6 channel map. Opaque navy.
void item(uint8_t id, int x, int y, int scale = 1);
void landscape(int x = 0, int y = 0, int scale = 1);
void camp(int x = 0, int y = 0, int scale = 1);
enum class PanelStyle : uint8_t { PAPER, GOLD, VIOLET, DARK };
// Nine-slice sampling of generated blank texture pixels, with clipped bounds.
void panel(int x, int y, int width, int height, PanelStyle style);
const gfx::Sprite& icon();
}}  // namespace kingdom::art
