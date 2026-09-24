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

// One blob per house ("s0".."s2"); "save" is the pre-house key, read once for migration.
bool saveBlob(int slot, const void* data, size_t len);
size_t loadBlob(int slot, void* data, size_t maxLen);
size_t loadLegacyBlob(void* data, size_t maxLen);
void eraseBlob(int slot);
void eraseAll();

uint32_t freeHeap();
}  // namespace board
