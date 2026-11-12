#pragma once
#include "gfx.h"
#include "rules.h"

namespace starfighter::paint {
enum class Page : uint8_t { Title, Calibrate, Brief, Flight };
// Logical pixels. All four corners fit the round glass; targets are >=24x24.
struct Box { int x, y, w, h; };
constexpr Box MAIN{43,106,74,24}, EXIT{62,132,36,24}, HOLD{64,9,32,24};
struct View {
  Page page = Page::Title;
  const Run* run = nullptr;
  uint32_t ms = 0;
  float bank = 0;
  uint16_t calibrationPermille = 0, holdPermille = 0;
  bool gripReady = false, gripUpright = false;
};
void draw(const View& view);
const gfx::Sprite& icon();
}  // namespace starfighter::paint
