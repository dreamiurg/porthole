#pragma once
#include "font.h"
#include "gfx.h"
#include "gfx565.h"
#include "state.h"

namespace toytown { namespace art {
extern const font::Font BODY, TITLE;
// Original scenes: street, home, school, cafe, park; each fills the 480px panel.
void background(int scene);
// Original anime/plush/clay/keychain dolls, transparent 96x160 base, integer scale.
// All saved cosmetic fields change visible materials, shapes or accessories.
void doll(const State& state, int x, int y, int scale);
// Cropped or scaled views of the same production doll: full, head, fabric, shoes.
void detail(const State& state, const gfx565::Rect& box, int part);
uint16_t swatch(const State& state, int category);
const gfx::Sprite& icon();
}}  // namespace toytown::art
