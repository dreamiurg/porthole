#include "fastmem.h"
#include <stdlib.h>
#include <string.h>
#ifdef ESP_PLATFORM
#include "esp_heap_caps.h"
#endif

namespace fastmem {
namespace {
constexpr int RETRY_CALLS = 60;   // after a refusal for want of room, ask the heap again only every second or so
bool dark_ = false;
int wait_ = 0;
void* block_ = nullptr;
Recall back_ = nullptr;
void* owner_ = nullptr;

#ifdef ESP_PLATFORM
constexpr uint32_t CAPS = MALLOC_CAP_INTERNAL | MALLOC_CAP_DMA;   // the pool the update's check counts
void* grab(size_t n) {
  if (heap_caps_get_free_size(CAPS) < n + UPDATE_RAM) return nullptr;
  void* p = heap_caps_malloc(n, CAPS);
  // Checked after: the radio's core may have allocated meanwhile, and the block may have split the one TLS needs.
  if (p && (heap_caps_get_free_size(CAPS) < UPDATE_RAM || heap_caps_get_largest_free_block(CAPS) < TLS_BLOCK)) {
    heap_caps_free(p);
    return nullptr;
  }
  return p;
}
void drop(void* p) { heap_caps_free(p); }
#else
// Poisoned: a game must not count on a fresh loan being zeroed (the board's internal heap is reused).
void* grab(size_t n) { void* p = malloc(n); if (p) memset(p, 0xA5, n); return p; }
void drop(void* p) { free(p); }
#endif
}  // namespace

void* take(size_t n, Recall back, void* owner) {
  if (dark_ || block_) return nullptr;
  if (wait_ > 0) { wait_--; return nullptr; }
  block_ = grab(n);
  if (!block_) { wait_ = RETRY_CALLS; return nullptr; }
  back_ = back; owner_ = owner;
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
