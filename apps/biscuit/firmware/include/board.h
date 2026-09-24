#pragma once
#include <stdint.h>

// Waveshare ESP32-S3-Touch-LCD-2.1. Call from the main Arduino/LVGL task.
bool boardBegin();
bool boardTouch(uint16_t& x, uint16_t& y);
// Coordinates are inclusive, matching LVGL 8's lv_area_t. RGB565 native byte order.
void boardFlush(int x1, int y1, int x2, int y2, const uint16_t* pixels);
void boardBrightness(uint8_t percent);
const uint16_t* boardFrameBuffer();
// UTC Unix seconds; read returns 0 for unavailable, stopped, or invalid RTC data.
// Stored years use 2000..2099. Existing clocks written by other firmware must be set.
uint64_t boardClockRead();
bool boardClockSet(uint64_t epochSeconds);
