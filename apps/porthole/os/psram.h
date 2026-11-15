// Long-lived objects kept out of static internal RAM. Every byte of .dram0 is gone from the heap a wireless update
// needs (see platformio.ini, custom_dram_static_max), so the games and the shell live in PSRAM on the board instead.
// They are a few KB each and stay in the data cache; a game's hottest buffer can still borrow internal RAM (fastmem.h).
#pragma once
#include <new>
#include <stdlib.h>
#include <string.h>
#ifdef ESP_PLATFORM
#include <esp_heap_caps.h>
#endif

namespace psram {
// Built once, before setup() (the IDF adds PSRAM to the heap before global constructors run), never freed.
template <class T> T& make() {
  void* p = nullptr;
#ifdef ESP_PLATFORM
  p = heap_caps_aligned_alloc(alignof(T) < 4 ? 4 : alignof(T), sizeof(T), MALLOC_CAP_SPIRAM | MALLOC_CAP_8BIT);
#endif
  if (!p) p = malloc(sizeof(T));   // no PSRAM (or the host): the plain heap
  if (!p) abort();
  memset(p, 0, sizeof(T));   // static storage started zeroed; a member without an initializer may rely on that
  return *new (p) T;
}
}  // namespace psram
