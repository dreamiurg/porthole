// Biscuit's widgets on the RGB565 surface: the look of its LVGL build (paper, ink, soft purple/peach/sage buttons with
// a 1 px line and radius 7), as plain functions. Stateless like os/ui.h, but split in two: update() asks tapped(),
// render() draws (pressed while the finger is on it). Never draw from update(): on the device the RGB565 target is
// only handed over for render(), and between frames it is the buffer on the glass. Hit boxes are logical, so the UI
// audit sees every region; drawing is physical px.
// ponytail: game-local; move to os/ when a second RGB565 game wants the same widgets.
#pragma once
#include <stdint.h>
#include "input.h"
#include "layout.h"

namespace biscuit::ui565 {
constexpr uint16_t rgb(uint32_t c) { return (uint16_t)(((c >> 8) & 0xF800) | ((c >> 5) & 0x07E0) | ((c >> 3) & 0x001F)); }
constexpr uint16_t PAPER = rgb(0xfff7e6), INK = rgb(0x42355a), LINE = rgb(0x9c86ad), DETAIL = rgb(0x655575);
constexpr uint16_t PURPLE = rgb(0xe6dfef), PEACH = rgb(0xefdfce), SAGE = rgb(0xe6ebd6), DISABLED = rgb(0xe6d2d6);
constexpr uint16_t BOWL = rgb(0xc5965a), HEART = rgb(0xc58a94), MOON = rgb(0x9ca66b), BUBBLE_EDGE = rgb(0xbdaa84);

enum class Icon : uint8_t { Bowl, Heart, Moon, Book, Ball, Paw, Star };
void icon(Icon which, int x, int y, int scale, uint16_t color);   // 12x12 mask at (x, y), physical
void tennisBall(int x, int y);                                   // 30x30 at (x, y), physical

struct Button { Box box; const char* label; const font::Font* font; uint16_t fill; bool enabled; };
bool tapped(const Input& in, const Button& b);   // a disabled button never hits
void button(const Input& in, const Button& b);   // disabled: dimmed
void actionButton(const Input& in, const Button& b, Icon glyph);   // Home's actions: an icon over a label
// World's big tiles: icon and title, a detail line under them.
struct Tile { Box box; const char* title; const char* detail; Icon glyph; uint16_t fill; };
void tile(const Input& in, const Tile& t);
void hotspot(const Input& in, const Box& b);     // an invisible room target: lights up while pressed
void roundButton(const Input& in, const Box& b, const char* label);   // a disc in the box, FONT28 label
bool homeTapped(const Input& in);                // the shell's orange home button, top center
void home(const Input& in);

constexpr Button BACK_BUTTON = {BACK, "Back", &FONT20, PURPLE, true};
void top(const Input& in, const char* title, int stars);   // Back, the stars, a title
int navTapped(const Input& in, int page, int count);       // Previous / page / Next: -1, +1 or 0
void nav(const Input& in, int page, int count);
void need(int slot, Icon glyph, int value, uint16_t color); // one of Home's three need cards
void bubble(const char* text);                              // Home's speech bubble
void text(const Label& l, const char* s, uint16_t color);    // a label at its place (middle: centered in its h)
}  // namespace biscuit::ui565
