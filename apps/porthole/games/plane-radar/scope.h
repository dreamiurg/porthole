// Plane Radar's geometry, with no drawing: where a plane sits on the scope, the range steps, where a plane's tag fits
// inside the round glass, and the words the scope shows. Physical px (480x480), host-tested in host/test_radar.cpp.
#pragma once
#include <stddef.h>
#include <stdint.h>
#include "air.h"
#include "canvas.h"
#include "gfx565.h"

namespace radar {

using gfx565::CX;
using gfx565::CY;
constexpr float DEG = 3.14159265f / 180;
constexpr int RING = 196;     // the outer ring: the range's distance
constexpr int RIM = 212;      // a plane past the ring shows as a dot here, at its bearing
constexpr int CARD_R = 216;   // N / E / S / W, their letters wholly on the glass
constexpr int RANGES = 4;
constexpr int RANGE_MI[RANGES] = {3, 6, 12, 25};      // the outer ring's distance in miles; a tap steps through them
constexpr float rangeKm(int range) { return RANGE_MI[range] * 1.609344f; }
constexpr int START_RANGE = 2;

// Planes are asked for out to the glass's edge (past the ring, for the rim dots), plus a margin: the request goes out
// with the spot rounded to about a kilometre (firmware/radar.cpp).
inline float fetchKm(int range) { return rangeKm(range) * 240 / RING + 2; }

// up: which way is at the top of the screen (Save::up: 0 N, 1 E, 2 S, 3 W). A true heading as degrees clockwise from
// the top of the screen.
float screenDeg(float trueDeg, int up);

struct Placed { int x, y; bool inside; };   // inside the ring: a symbol at (x, y); past it: a rim dot at (x, y)
Placed place(float eastKm, float northKm, int up, int range);

// A plane's tag (w x h) goes beside its symbol: toward the middle, else the other side, else above, else below; the
// first place wholly on the glass and clear of every box in `taken` (other tags, the labels and buttons). ok is false
// when none is: that plane goes without a tag. (x, y) is the box's top-left.
inline bool overlap(const canvas::Box& a, const canvas::Box& b) {
  return a.x0 < b.x1 && b.x0 < a.x1 && a.y0 < b.y1 && b.y0 < a.y1;
}
struct Tag { int x, y, side; bool ok; };   // side: which of the four places it took (0 toward the middle .. 3 below)
constexpr int TAG_GAP = 18;   // from the plane's middle to the tag
// A label w x h for the plane at (px, py). prefer: the side its label had last repaint, tried first so a label does
// not hop as planes move; -1 for none.
struct TagAsk { int px, py, w, h, prefer; };
struct Boxes { const canvas::Box* r; int n; };
Tag tagFor(const TagAsk& a, const Boxes& taken);

void altText(int32_t altFt, char* buf, size_t n);   // "12300 ft" (to the nearest 100), "" when unknown
void rangeText(int range, char* buf, size_t n);     // "12 mi"
const char* stateText(air::State s);                // the note a non-live scope shows; "" for S_LIVE
void spotText(bool set, double lat, double lon, char* buf, size_t n);   // "Home 47.61,-122.33" or "Home not set"

}  // namespace radar
