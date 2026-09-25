// TODO(rebase): temporary copy from feat/biscuit-content so Biscuit's generated art compiles before the RGB565 surface
// lands. The RGB565 surface PR owns this file: on rebase, take its version (these two structs are identical there).
#pragma once
#include <stdint.h>

namespace gfx565 {
struct Image565 { uint16_t width, height; const uint16_t* pixels; };
// runs alternate (count, color565) pairs; runCount = number of pairs.
struct RleImage { uint16_t width, height; const uint16_t* runs; uint32_t runCount; };
}  // namespace gfx565
