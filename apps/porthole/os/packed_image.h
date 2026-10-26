// Lossless zlib RGB565 little-endian images, with deflate windows up to 32 KiB.
#pragma once
#include <stdint.h>

namespace packed565 {
struct Image { uint16_t width, height; const uint8_t* data; uint32_t byteCount; };
// Serial rendering only: one shared fixed decoder dictionary, no image-sized allocation.
// Clips to the current gfx565 target and draws every visible pixel on every call.
// Returns false for invalid metadata or a corrupt/incomplete stream; a failed draw may be partial.
bool draw(const Image& image, int x, int y, int scale);
}  // namespace packed565
