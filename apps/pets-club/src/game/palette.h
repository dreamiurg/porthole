// Pets Club palette: 32 fixed colors. Sprites and the framebuffer store palette
// indices; the display layer converts to RGB565 at flip time, so day/night
// tints are just alternate lookup tables.
#pragma once
#include <stdint.h>

enum Col : uint8_t {
  C_BLACK = 0, C_NAVY, C_PLUM, C_DKGREEN, C_BROWN, C_DKGRAY, C_LTGRAY, C_WHITE,
  C_RED, C_ORANGE, C_YELLOW, C_GREEN, C_BLUE, C_LAVENDER, C_PINK, C_PEACH,
  C_TAN, C_DKBROWN, C_CREAM, C_SKY, C_NIGHT, C_WOOD, C_DKWOOD, C_WALL,
  C_FLOOR, C_MINT, C_LEAF, C_GOLD, C_MUD, C_WATER, C_ROSE, C_SLATE,
  C_COUNT
};
constexpr uint8_t C_T = 255;  // transparent (sprites only)

static const uint32_t PALETTE_RGB[C_COUNT] = {
  0x000000, 0x1D2B53, 0x7E2553, 0x008751, 0xAB5236, 0x5F574F, 0xC2C3C7, 0xFFF1E8,
  0xFF004D, 0xFFA300, 0xFFEC27, 0x00E436, 0x29ADFF, 0x83769C, 0xFF77A8, 0xFFCCAA,
  0xE6B27A, 0x5C3A1E, 0xFFF7D6, 0xA8E0FF, 0x16213E, 0xC47A3A, 0x8A4B22, 0xF2D9B5,
  0xD9A066, 0x6EE7B7, 0x3BAA3B, 0xF5C400, 0x4A4A3C, 0x5FB8F5, 0xE0587A, 0x3A3F58,
};

// Tint modes applied at flip time.
enum Tint : uint8_t { TINT_DAY = 0, TINT_EVENING, TINT_NIGHT, TINT_COUNT };

// Build an RGB888 palette for a tint mode.
static inline void palette_build(Tint tint, uint32_t out[C_COUNT]) {
  for (int i = 0; i < C_COUNT; i++) {
    uint32_t c = PALETTE_RGB[i];
    int r = (c >> 16) & 255, g = (c >> 8) & 255, b = c & 255;
    if (tint == TINT_EVENING) { r = r * 230 / 256; g = g * 200 / 256; b = b * 180 / 256; }
    if (tint == TINT_NIGHT)   { r = r * 108 / 256; g = g * 118 / 256; b = b * 160 / 256 + 12; }
    if (b > 255) b = 255;
    out[i] = (uint32_t)(r << 16 | g << 8 | b);
  }
}
static inline uint16_t rgb888_to_565(uint32_t c) {
  return (uint16_t)(((c >> 8) & 0xF800) | ((c >> 5) & 0x07E0) | ((c >> 3) & 0x001F));
}
