#pragma once
#include "gfx.h"
#include <stdint.h>

namespace pumpkin {
enum class Page : uint8_t { Intro, Calibrate, Play, Catch, Icing, Lost, Won };
// Art uses logical 160px coordinates, expanded pixel-perfect to the RGB565 surface.
// Controls: Home {68,8,24,22}, action {40,119,80,24}; room lane y=99.
// Hiding centers 36,80,124. Pumpkin center x in [24,136].
struct View {
  Page page = Page::Intro;
  float x = 80, velocity = 0;
  int hidden = -1, nearby = -1;
  uint32_t elapsed = 0, pageMs = 0, animationMs = 0;
  float beamX = 80;  // cone endpoint on y=99; same width (+/-20) as detection
  bool searching = false, warning = false, ready = false, starting = false;
  float exposure = 0, homeHold = 0;
};
namespace paint {
void draw(const View& view);
const gfx::Sprite& icon();
}  // namespace paint
}  // namespace pumpkin
