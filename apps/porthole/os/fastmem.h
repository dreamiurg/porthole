// Internal RAM for a game's hottest buffer while the game is open and the screen lit. On the board the ordinary heap
// puts big blocks in PSRAM, about 14x slower for random access. Internal RAM is what a wireless update needs, so a loan
// is made only when the update's share stays free, and every loan comes home when the screen goes dark (the daily
// update runs only then) and on leave(). One loan at a time. On the host: the heap.
#pragma once
#include <stddef.h>

namespace fastmem {
// Internal RAM a wireless update needs: its screen's 19 chunks of 3840 bytes (main.cpp static_asserts the sum), then
// wifi and TLS (on the board those took about 28 KB more, with 77 KB still free).
constexpr size_t UPDATE_PICTURE = 19 * 3840, UPDATE_HEADROOM = 48 * 1024, UPDATE_RAM = UPDATE_PICTURE + UPDATE_HEADROOM;
constexpr size_t TLS_BLOCK = 45000;   // one contiguous block a TLS handshake needs (net.cpp's MIN_HEAP)

using Recall = void (*)(void* owner);
// n bytes of internal RAM, not zeroed, or null (keep the buffer where it is): dark, a loan already out, or too little
// room (then the next RETRY_CALLS calls are refused without asking the heap, so a game may simply ask every frame).
// back(owner) is called when the block must come home: it stops using the block and calls give().
void* take(size_t n, Recall back, void* owner);
void give(void* p);
void setDark(bool dark);   // the loop, every frame, before the updater: dark recalls the loan and lends nothing
}  // namespace fastmem
