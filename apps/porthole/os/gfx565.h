// Full-resolution RGB565 surface: the whole 480x480 panel, one uint16_t per physical pixel, for apps whose
// surface() is SURFACE_RGB565 (os/app.h). The host owns the pixels: the firmware hands the panel's back buffer to
// target() before every render, the sim a static array. Coordinates here are physical px; hit tests stay logical
// (divide by 3), like every other app. Colors are native RGB565 words (rgb888_to_565 in palette.h).
#pragma once
#include <stdint.h>

namespace gfx565 {
constexpr int W = 480, H = 480, CX = 240, CY = 240, R = 240;
extern uint16_t* fb;             // the current target: W * H pixels, row-major
void target(uint16_t* pixels);   // must be set before an RGB565 render()

// Generated art (Biscuit's scenes and pictures) uses these layouts verbatim.
struct Image565 { uint16_t width, height; const uint16_t* pixels; };
struct RleImage { uint16_t width, height; const uint16_t* runs; uint32_t runCount; };  // runs = (count, color565) pairs; runCount = pairs

inline void pixel(int x, int y, uint16_t c) { if ((unsigned)x < W && (unsigned)y < H) fb[y * W + x] = c; }
void clear(uint16_t c);
void rect(int x, int y, int w, int h, uint16_t c);             // filled, clipped to the surface
void hline(int x, int y, int w, uint16_t c);
void frame(int x, int y, int w, int h, uint16_t c);            // 1 px outline inside the box
struct Rect { int x, y, w, h; };
void roundRect(const Rect& b, int r, uint16_t c);              // filled, corners of radius r (clamped to half the box)
void circle(int cx, int cy, int r, uint16_t c);                // filled disc
// Nearest-neighbor copy, each source pixel becoming a scale x scale block (1 = as is, 3 = a 160 px image on the panel).
void blit(const Image565& im, int x, int y, int scale);
// Same, decoding the runs straight into the target (no scratch buffer). Runs past width*height are ignored.
void blitRle(const RleImage& im, int x, int y, int scale);

// Is physical (x,y) inside the round glass (radius R - margin around the center)?
inline bool inCircle(int x, int y, int margin = 0) {
  int dx = x - CX, dy = y - CY, r = R - margin; return dx * dx + dy * dy <= r * r;
}
// RGB565 -> RGB888 by bit replication (for snapshots and the text audit).
inline uint32_t rgb888(uint16_t c) {
  uint32_t r = c >> 11, g = (c >> 5) & 63, b = c & 31;
  return (r << 3 | r >> 2) << 16 | (g << 2 | g >> 4) << 8 | (b << 3 | b >> 2);
}
}  // namespace gfx565
