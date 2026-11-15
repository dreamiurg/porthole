// Biscuit's doghouse content: what the things are called and what they cost, in DecorId order (generated/
// doghouse_art.h, Save::decor bits: append only). Prices are stars; a story is 3, five right puzzles 1.
#pragma once
#include <stdint.h>

namespace biscuit {
struct DecorItem { const char* name; uint16_t price; };
inline constexpr DecorItem DECOR[] = {
  {"Ball", 3},
  {"Food bowl", 4},
  {"Flags", 5},
  {"Our photo", 6},
  {"Plant", 8},
  {"Clock", 10},
  {"Star poster", 12},
  {"Toy shelf", 15},
  {"Comfy bed", 20},
};
}  // namespace biscuit
