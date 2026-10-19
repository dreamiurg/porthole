// Waveshare ESP32-S3-Touch-LCD-2.1 board layer.
#pragma once
#include <stdint.h>
#include <stddef.h>
#include "console.h"   // Serial on both USB-C ports

namespace board {
constexpr int LCD_W = 480, LCD_H = 480;

void init();
// Upscale a 160x160 indexed frame 3x through pal565 and wait until the driver releases the old buffer.
void present(const uint8_t* fb160, const uint16_t* pal565);
// RGB565 apps render into backBuffer(); presentHires() waits for the driver's SRAM-copy fence before buffer reuse.
uint16_t* backBuffer();
void presentHires();
const uint16_t* frontBuffer();   // the frame on the glass (the serial frame dump)
// Explicit updates keep a small SRAM-backed progress display scanning while flash is unavailable.
bool beginUpdateDisplay();   // false: leave normal display intact and do not start the update
void drawUpdateProgress(const char* label, bool cancellable);
bool hitUpdateCancel(int logicalX, int logicalY);
void endUpdateDisplay();

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
// After flash writes, wait for the DMA restart and a full fresh scan before lighting a dark screen. False on timeout.
bool settlePanel();
void buzzer(bool on);

// NVS blobs addressed by (namespace, key). Namespaces are at most 15 chars, keys at most 15.
bool saveBlob(const char* ns, const char* key, const void* data, size_t len);
size_t loadBlob(const char* ns, const char* key, void* data, size_t maxLen);  // 0 if missing or bigger than maxLen
void eraseBlob(const char* ns, const char* key);
void eraseNamespace(const char* ns);  // every key in ns

// The optional microSD card (1-bit SDMMC), mounted at SD_MMC's "/sdcard". False with no card. Only after init().
bool mountSd();

uint32_t freeHeap();
}  // namespace board
