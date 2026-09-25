// Waveshare ESP32-S3-Touch-LCD-2.1 board layer.
#pragma once
#include <stdint.h>
#include <stddef.h>

namespace board {
constexpr int LCD_W = 480, LCD_H = 480;

void init();
// Upscale a 160x160 indexed frame 3x through pal565 into the back buffer and swap on vsync.
void present(const uint8_t* fb160, const uint16_t* pal565);

struct Touch { bool down; int x, y; };  // physical pixels
Touch readTouch();

// RTC: seconds since 1970-01-01 00:00 in *local* wall-clock terms (no timezone handling).
bool rtcValid();
uint32_t rtcNow();
void rtcSet(uint32_t localEpoch);

void setBacklight(uint8_t percent);
void buzzer(bool on);

// NVS blobs addressed by (namespace, key). Namespaces are at most 15 chars, keys at most 15.
bool saveBlob(const char* ns, const char* key, const void* data, size_t len);
size_t loadBlob(const char* ns, const char* key, void* data, size_t maxLen);  // 0 if missing or bigger than maxLen
void eraseBlob(const char* ns, const char* key);
void eraseAll();  // every namespace in NAMESPACES (board.cpp)

uint32_t freeHeap();
}  // namespace board
