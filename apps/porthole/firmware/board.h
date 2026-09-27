// Waveshare ESP32-S3-Touch-LCD-2.1 board layer.
#pragma once
#include <stdint.h>
#include <stddef.h>

namespace board {
constexpr int LCD_W = 480, LCD_H = 480;

void init();
// Upscale a 160x160 indexed frame 3x through pal565 into the back buffer and swap on vsync.
void present(const uint8_t* fb160, const uint16_t* pal565);
// RGB565 apps draw straight into the panel: render into backBuffer(), then presentHires() swaps it in on vsync.
uint16_t* backBuffer();
void presentHires();
const uint16_t* frontBuffer();   // the frame on the glass (the serial frame dump)

struct Touch { bool down; int x, y; };  // physical pixels
Touch readTouch();

// RTC: seconds since 1970-01-01 00:00 in *local* wall-clock terms (no timezone handling).
bool rtcValid();
uint32_t rtcNow();
void rtcSet(uint32_t localEpoch);

// Motion sensor (QMI8658 accelerometer). Gravity -- where things fall -- in milli-g in screen terms: +x toward the right
// edge, +y toward the bottom edge, +z out of the glass toward the viewer (upright: 0,1000,0; face up: 0,0,-1000).
// False, outputs untouched, when the chip is absent or the read fails. `raw`, if given, gets the chip's own axes (LSB).
bool readAccel(int16_t& gx, int16_t& gy, int16_t& gz, int16_t* raw = nullptr);

void setBacklight(uint8_t percent);
void buzzer(bool on);

// NVS blobs addressed by (namespace, key). Namespaces are at most 15 chars, keys at most 15.
bool saveBlob(const char* ns, const char* key, const void* data, size_t len);
size_t loadBlob(const char* ns, const char* key, void* data, size_t maxLen);  // 0 if missing or bigger than maxLen
void eraseBlob(const char* ns, const char* key);
void eraseNamespace(const char* ns);  // every key in ns

uint32_t freeHeap();
}  // namespace board
