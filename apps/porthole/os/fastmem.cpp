#include "fastmem.h"
#include <stdlib.h>
#ifdef ESP_PLATFORM
#include "esp_heap_caps.h"
#endif

namespace fastmem {
namespace {
bool dark_ = false;
void* block_ = nullptr;
Recall back_ = nullptr;
void* owner_ = nullptr;

#ifdef ESP_PLATFORM
constexpr uint32_t CAPS = MALLOC_CAP_INTERNAL | MALLOC_CAP_DMA;   // the pool the update's check counts
void* grab(size_t n) {
  if (heap_caps_get_free_size(CAPS) < n + UPDATE_RAM || heap_caps_get_largest_free_block(CAPS) < n + TLS_BLOCK) return nullptr;
  void* p = heap_caps_malloc(n, CAPS);
  if (p && heap_caps_get_free_size(CAPS) < UPDATE_RAM) { heap_caps_free(p); return nullptr; }   // the radio's core got there first
  return p;
}
void drop(void* p) { heap_caps_free(p); }
#else
void* grab(size_t n) { return malloc(n); }
void drop(void* p) { free(p); }
#endif
}  // namespace

void* take(size_t n, Recall back, void* owner) {
  if (dark_ || block_) return nullptr;
  block_ = grab(n);
  if (block_) { back_ = back; owner_ = owner; }
  return block_;
}
void give(void* p) {
  if (!p || p != block_) return;
  drop(p);
  block_ = nullptr; back_ = nullptr; owner_ = nullptr;
}
void setDark(bool dark) {
  dark_ = dark;
  if (dark && block_) back_(owner_);
}
}  // namespace fastmem
