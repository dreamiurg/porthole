#include "canvas.h"
#include <math.h>
#include "font.h"

namespace canvas {
namespace {
Box clip_ = FULL;
}  // namespace

Box unite(const Box& a, const Box& b) {
  if (empty(a)) return b;
  if (empty(b)) return a;
  return {a.x0 < b.x0 ? a.x0 : b.x0, a.y0 < b.y0 ? a.y0 : b.y0, a.x1 > b.x1 ? a.x1 : b.x1, a.y1 > b.y1 ? a.y1 : b.y1};
}
void clip(const Box& b) {
  clip_ = {b.x0 < 0 ? 0 : b.x0, b.y0 < 0 ? 0 : b.y0, b.x1 > gfx565::W ? gfx565::W : b.x1, b.y1 > gfx565::H ? gfx565::H : b.y1};
  font::clip(clip_.x0, clip_.y0, clip_.x1, clip_.y1);
}
const Box& clipBox() { return clip_; }

int isqrt(int n) {
  if (n < 0) return -1;
  int r = (int)sqrtf((float)n);
  while (r * r > n) r--;
  while ((r + 1) * (r + 1) <= n) r++;
  return r;
}
void span(int y, int x0, int x1, uint16_t c) {
  if (y < clip_.y0 || y >= clip_.y1) return;
  if (x0 < clip_.x0) x0 = clip_.x0;
  if (x1 > clip_.x1) x1 = clip_.x1;
  if (x1 > x0) gfx565::hline(x0, y, x1 - x0, c);
}
void rows(int& y0, int& y1) { if (y0 < clip_.y0) y0 = clip_.y0; if (y1 > clip_.y1) y1 = clip_.y1; }
Run discRun(int cx, int cy, int r, int y) {
  const int h = isqrt(r * r + r - (y - cy) * (y - cy));
  return h < 0 ? Run{0, 0} : Run{cx - h, cx + h + 1};
}
void disc(int cx, int cy, int r, uint16_t c) {
  int y0 = cy - r, y1 = cy + r + 1;
  rows(y0, y1);
  for (int y = y0; y < y1; y++) { const Run d = discRun(cx, cy, r, y); span(y, d.l, d.r, c); }
}
void polyRows(const int* p, int n, int* y0, int* y1) {
  *y0 = *y1 = p[1];
  for (int i = 1; i < n; i++) { if (p[2 * i + 1] < *y0) *y0 = p[2 * i + 1]; if (p[2 * i + 1] > *y1) *y1 = p[2 * i + 1]; }
  ++*y1;
  rows(*y0, *y1);
}
bool polyRun(const int* p, int n, int y, int* l, int* r) {
  float lo = 1e9f, hi = -1e9f;
  for (int i = 0; i < n; i++) {
    const int* a = p + 2 * i; const int* b = p + 2 * ((i + 1) % n);
    if (a[1] == b[1] || y < (a[1] < b[1] ? a[1] : b[1]) || y > (a[1] > b[1] ? a[1] : b[1])) continue;
    const float x = a[0] + (float)(y - a[1]) * (b[0] - a[0]) / (b[1] - a[1]);
    if (x < lo) lo = x;
    if (x > hi) hi = x;
  }
  *l = (int)lroundf(lo); *r = (int)lroundf(hi) + 1;
  return hi >= lo;
}
void poly(const int* p, int n, uint16_t c) {
  int y0, y1, l, r;
  polyRows(p, n, &y0, &y1);
  for (int y = y0; y < y1; y++) if (polyRun(p, n, y, &l, &r)) span(y, l, r, c);
}
void roundBox(const Box& b, int r, uint16_t c) {
  int y0 = b.y0, y1 = b.y1;
  rows(y0, y1);
  for (int y = y0; y < y1; y++) {
    const int k = y - b.y0 < r ? r - (y - b.y0) : y - (b.y1 - 1 - r) > 0 ? y - (b.y1 - 1 - r) : 0;   // rows into a corner
    const int in = k ? r - isqrt(r * r - k * k) : 0;
    span(y, b.x0 + in, b.x1 - in, c);
  }
}
}  // namespace canvas
