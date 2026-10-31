#include "scope.h"
#include <math.h>
#include <stdio.h>

namespace radar {

float screenDeg(float trueDeg, int up) {
  const float d = fmodf(trueDeg - up * 90.0f, 360.0f);
  return d < 0 ? d + 360 : d;
}

Placed place(float eastKm, float northKm, int up, int range) {
  const float a = up * 90 * DEG, c = cosf(a), s = sinf(a), px = RING / rangeKm(range);   // px a km
  const float x = (eastKm * c - northKm * s) * px, y = (northKm * c + eastKm * s) * px, r = hypotf(x, y);
  const bool in = r <= RING;
  const float k = in ? 1 : RIM / r;   // past the ring: same bearing, on the rim
  return {CX + (int)lroundf(x * k), CY - (int)lroundf(y * k), in};
}

namespace {
bool onGlass(int x, int y, int w, int h) {
  return gfx565::inCircle(x, y, 6) && gfx565::inCircle(x + w, y, 6) && gfx565::inCircle(x, y + h, 6) &&
         gfx565::inCircle(x + w, y + h, 6);
}
bool clear(const canvas::Box& r, const Boxes& taken) {
  for (int i = 0; i < taken.n; i++) if (overlap(r, taken.r[i])) return false;
  return true;
}
}  // namespace

Tag tagFor(const TagAsk& a, const Boxes& taken) {
  const int left = a.px - TAG_GAP - a.w, right = a.px + TAG_GAP, centered = a.px - a.w / 2;
  const int xs[] = {a.px < CX ? right : left, a.px < CX ? left : right, centered, centered};
  const int ys[] = {a.py - a.h / 2, a.py - a.h / 2, a.py - TAG_GAP - a.h, a.py + TAG_GAP};
  const int order[] = {a.prefer, 0, 1, 2, 3};
  for (int k : order) {
    if (k < 0 || k > 3) continue;
    const int x = xs[k], y = ys[k];
    if (onGlass(x, y, a.w, a.h) && clear({x - 3, y - 2, x + a.w + 3, y + a.h + 2}, taken)) return {x, y, k, true};
  }
  return {0, 0, -1, false};
}

void altText(int32_t altFt, char* buf, size_t n) {
  if (altFt == air::NO_ALT) { snprintf(buf, n, "%s", ""); return; }
  snprintf(buf, n, "%ld ft", lround(altFt / 100.0) * 100);   // to the nearest 100, below sea level too
}

void rangeText(int range, char* buf, size_t n) { snprintf(buf, n, "%d mi", RANGE_MI[range]); }

void spotText(bool set, double lat, double lon, char* buf, size_t n) {
  if (!set) snprintf(buf, n, "%s", "Home not set");
  else snprintf(buf, n, "Home %.2f,%.2f", lat, lon);
}

const char* stateText(air::State s) {
  static const char* const T[] = {"The radar is off. Try again soon.", "Ask a grown-up to set where home is.",
                                  "No wifi. Ask a grown-up to help.", "Looking for planes...", "",
                                  "Lost the planes. Trying again..."};
  static_assert(sizeof T / sizeof T[0] == air::S_COUNT, "a note per state");
  return T[s < air::S_COUNT ? s : air::S_OFF];
}

}  // namespace radar
