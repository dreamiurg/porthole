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
  int cover = -1;     // the spot in reach it can hide in (not the one a Check names): lit, and HOLD TO HIDE
  int coverDir = 1;   // FIND COVER's arrow, -1 left or 1 right
  uint32_t pageMs = 0, animationMs = 0;
  float beamX = 80;  // cone endpoint on y=99; same width (+/-20) as detection
  bool searching = false, warning = false, ready = false, starting = false;
  float exposure = 0, homeHold = 0;
  float candy[3] = {58, 102, 134};    // candy corn centers on the lane
  uint8_t got = 0;                    // bit i: candy i collected
  uint32_t sinceCandy = UINT32_MAX;   // ms since the last pickup
  bool doorOpen = false;              // all candy collected: the door at the left wall is open
  int checking = -1;                  // spot Grandma announces (warning) or checks (searching)
  bool shiver = false, phew = false;  // beam over a hidden pumpkin / a close search just ended
  bool refused = false;               // a finger asked the sign to hide away from cover: it says "no"
  bool ghostReady = true;             // the ghost at the window can still be tapped (box {22,30,24,24})
  uint32_t distract = 0;              // ms left of the crash Grandma goes to look at ("WHAT WAS THAT?")
  bool rustling = false;              // hidden and tilting hard: the spot shakes ("SHH!")
  bool heard = false;                 // Grandma heard it: her next move checks that spot
  uint32_t startle = 0;               // ms left of the scared face and "!": she named its spot, so it jumped out
  bool scared = false;                // in the open within reach of the spot she names: the scared face, no "!"
  uint8_t quip = 0;                   // which catch / pie / escape line, 0..QUIPS-1
};
constexpr int QUIPS = 4;
// The palette, RGB565; paint indexes it through its own table.
constexpr uint16_t INK = rgb888_to_565(0x201d32), WALL = rgb888_to_565(0x463451), PURPLE = rgb888_to_565(0x815573);
constexpr uint16_t LILAC = rgb888_to_565(0xb9809a), WOOD = rgb888_to_565(0x925743), FLOOR = rgb888_to_565(0x633e43);
constexpr uint16_t ORANGE = rgb888_to_565(0xf58b32), RUST = rgb888_to_565(0xb74f30), GOLD = rgb888_to_565(0xffc875);
constexpr uint16_t CREAM = rgb888_to_565(0xffedc2), SAGE = rgb888_to_565(0x87a878), GREEN = rgb888_to_565(0x49674d);
constexpr uint16_t NIGHT = rgb888_to_565(0x263449), BLUE = rgb888_to_565(0x53637a), SKIN = rgb888_to_565(0xe6b79d);
constexpr uint16_t GREY = rgb888_to_565(0xb6acb4), BEAM = rgb888_to_565(0x997752);
constexpr uint16_t SKY = rgb888_to_565(0x9ad4e4);  // sweat drops
namespace paint {
void draw(const View& view);
const gfx::Sprite& icon();
}  // namespace paint
}  // namespace pumpkin
