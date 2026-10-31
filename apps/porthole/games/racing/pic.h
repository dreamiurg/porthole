// The picture type every generated header shares (tools/pictures.py).
#pragma once
#include <stdint.h>
namespace racing::pic {
struct Pic { uint16_t w, h; const uint16_t* px; };   // RGB565, row-major
constexpr uint16_t CLEAR = 0xF81F;                   // transparent: the generator never emits this color
}  // namespace racing::pic
