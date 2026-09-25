// Content copy names the child and the dog with tokens, never literally: {name} is the profile's name ("Friend" when
// the profile has none) and {pet} the dog's ("Biscuit" until it is named). Anything else in braces is left as written.
#pragma once
#include <stddef.h>

namespace biscuit {
// Writes `text` with the tokens filled into out (always NUL-terminated, cut at cap - 1). Returns the length written.
size_t personalize(const char* text, const char* name, const char* pet, char* out, size_t cap);
}  // namespace biscuit
