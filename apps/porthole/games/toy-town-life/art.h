#pragma once
#include "font.h"
#include "gfx.h"
#include "state.h"

namespace toytown { namespace art {
extern const font::Font BODY, TITLE;
// Original scenes: street, home, school, cafe, park; each fills the 480px panel.
void background(int scene);
// Original anime/plush/clay/keychain dolls, transparent 96x160 base, integer scale.
// All saved cosmetic fields change visible materials, shapes or accessories.
void doll(const State& state, int x, int y, int scale);
const gfx::Sprite& icon();
}}  // namespace toytown::art
