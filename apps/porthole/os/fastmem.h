// Internal RAM for a game's hottest buffer while the game is open. On the board the ordinary heap puts big blocks in
// PSRAM, about 14x slower for random access. Internal RAM is what a wireless update needs, so take() lends a block only
// when the update's share (UPDATE_RAM) stays free, and the game gives it back on leave(). On the host: the heap.
#pragma once
#include <stddef.h>
#include <stdlib.h>
#ifdef ESP_PLATFORM
#include "esp_heap_caps.h"
#endif

namespace fastmem {
// Internal RAM a wireless update needs: its screen's 19 chunks of 3840 bytes, plus 48 KB for wifi and TLS. main.cpp
// checks the same sum at boot and static_asserts that the two agree.
constexpr size_t UPDATE_RAM = 19 * 3840 + 48 * 1024;
// n bytes of internal RAM, or null (then keep the buffer where it is).
inline void* take(size_t n) {
#ifdef ESP_PLATFORM
  if (heap_caps_get_free_size(MALLOC_CAP_INTERNAL | MALLOC_CAP_DMA) < n + UPDATE_RAM) return nullptr;
  return heap_caps_malloc(n, MALLOC_CAP_INTERNAL | MALLOC_CAP_8BIT);
#else
  return malloc(n);
#endif
}
inline void give(void* p) {
#ifdef ESP_PLATFORM
  heap_caps_free(p);
#else
  free(p);
#endif
}
}  // namespace fastmem
