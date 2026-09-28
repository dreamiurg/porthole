// A clipped span painter on the RGB565 surface (gfx565.h), for games drawn in code (Marble Kick, Tilt FC) that repaint
// only what moved. Every fill below goes through one span writer cut to the clip box (and clip() sets font::clip to the
// same box), so a game can repaint any rectangle alone: it draws its whole page, and only the box changes. The look
// (colors, what is drawn) stays in each game; this is only the painter and the two-buffer bookkeeping. Physical px,
// x1/y1 exclusive.
#pragma once
#include <stdint.h>
#include "gfx.h"
#include "gfx565.h"

namespace canvas {
struct Box { int x0, y0, x1, y1; };   // x1, y1 exclusive; empty when x1 <= x0
constexpr Box FULL = {0, 0, gfx565::W, gfx565::H};
constexpr Box NONE = {0, 0, 0, 0};
inline bool empty(const Box& b) { return b.x1 <= b.x0 || b.y1 <= b.y0; }
inline bool same(const Box& a, const Box& b) { return a.x0 == b.x0 && a.y0 == b.y0 && a.x1 == b.x1 && a.y1 == b.y1; }
inline int area(const Box& b) { return empty(b) ? 0 : (b.x1 - b.x0) * (b.y1 - b.y0); }
Box unite(const Box& a, const Box& b);
void clip(const Box& b);        // every draw below, text included (font::clip), touches only this box
const Box& clipBox();           // the box in force, cut to the panel

int isqrt(int n);               // floor(sqrt(n)), -1 for a negative n
void span(int y, int x0, int x1, uint16_t c);   // row y from x0 to x1 (exclusive), clipped
void rows(int& y0, int& y1);    // [y0, y1) cut to the clip's rows
struct Run { int l, r; };       // [l, r) of a row; empty when r <= l
Run discRun(int cx, int cy, int r, int y);      // a disc's run on row y
void disc(int cx, int cy, int r, uint16_t c);
// A convex polygon, vertices in order (x, y pairs): its rows in the clip, and each row's run [*l, *r).
void polyRows(const int* p, int n, int* y0, int* y1);
bool polyRun(const int* p, int n, int y, int* l, int* r);
void poly(const int* p, int n, uint16_t c);
void roundBox(const Box& b, int r, uint16_t c);   // corners of radius r (0: square)

// What each of the panel's two buffers (os/app.h) holds: the page, its movers as boxes with keys (a box and its key
// fully name the pixels in it), and the rest of the page's state as one `look` word. render() gives a buffer it does
// not know (after reset(), on a page change) or one with another look or mover count the whole page; one it knows
// gets each changed mover's old and new boxes (one pass over both when they overlap enough), and nothing when none
// changed. With the UI audit on, every frame is whole: the audit logs every glyph. N: the most movers a page has.
struct Mover { Box box; int key; };
template <int N>
class Frames {
 public:
  void reset() { held_[0].fb = held_[1].fb = nullptr; }
  template <class Draw>   // draw(): the whole page, as clipped by clip()
  void render(const Mover* now, int n, uint32_t look, Draw draw) {
    Held* h = held_[0].fb == gfx565::fb ? &held_[0] : held_[1].fb == gfx565::fb ? &held_[1] : nullptr;
    if (!h) { h = &held_[held_[0].fb ? 1 : 0]; h->fb = gfx565::fb; h->n = -1; }
    if (gfx::textLogEnabled || look != h->look || n != h->n) { clip(FULL); draw(); }
    else
      for (int i = 0; i < n; i++) {   // each thing that moved: where it was, and where it is
        const Mover& was = h->movers[i];
        if (same(now[i].box, was.box) && now[i].key == was.key) continue;
        const Box u = unite(was.box, now[i].box);
        if (area(u) <= area(was.box) + area(now[i].box)) { clip(u); draw(); continue; }
        clip(was.box);
        draw();
        clip(now[i].box);
        draw();
      }
    clip(FULL);   // font::clip back to the panel for whatever draws next
    for (int i = 0; i < n; i++) h->movers[i] = now[i];
    h->n = n;
    h->look = look;
  }

 private:
  struct Held { const uint16_t* fb; Mover movers[N]; int n; uint32_t look; };
  Held held_[2] = {};
};
}  // namespace canvas
