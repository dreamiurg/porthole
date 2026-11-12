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
  uint32_t pageMs = 0, animationMs = 0;
  float beamX = 80;  // cone endpoint on y=99; same width (+/-20) as detection
  bool searching = false, warning = false, ready = false, starting = false;
  float exposure = 0, homeHold = 0;
  float candy[3] = {58, 102, 134};    // candy corn centers on the lane
  uint8_t got = 0;                    // bit i: candy i collected
  uint32_t sinceCandy = UINT32_MAX;   // ms since the last pickup
  bool doorOpen = false;              // all candy collected: the door at the left wall is open
  int checking = -1;                  // spot Grandma announces (warning) or checks (searching)
  bool shiver = false, phew = false;  // beam over a hidden pumpkin / it just passed
  bool ghostReady = true;             // the ghost at the window can still be tapped (box {22,30,24,24})
  uint32_t distract = 0;              // ms left of the crash Grandma goes to look at ("WHAT WAS THAT?")
  bool rustling = false;              // hidden and tilting hard: the spot shakes ("SHH!")
  bool heard = false;                 // Grandma heard it: her next move checks that spot
  uint8_t quip = 0;                   // which catch / pie / escape line, 0..QUIPS-1
};
constexpr int QUIPS = 4;
namespace paint {
void draw(const View& view);
const gfx::Sprite& icon();
}  // namespace paint
}  // namespace pumpkin
