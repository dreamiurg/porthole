#include "personalize.h"
#include <string.h>

namespace biscuit {
size_t personalize(const char* text, const char* name, const char* pet, char* out, size_t cap) {
  if (!cap) return 0;
  const char* fill[2] = {name && *name ? name : "friend", pet && *pet ? pet : "Biscuit"};
  static const char* const TOKEN[2] = {"{name}", "{pet}"};
  size_t n = 0;
  auto put = [&](char c) { if (n + 1 < cap) out[n++] = c; };
  while (*text) {
    int t = strncmp(text, TOKEN[0], 6) == 0 ? 0 : strncmp(text, TOKEN[1], 5) == 0 ? 1 : -1;
    if (t < 0) { put(*text++); continue; }
    for (const char* s = fill[t]; *s; ++s) put(*s);
    text += strlen(TOKEN[t]);
  }
  out[n] = 0;
  return n;
}
}  // namespace biscuit
