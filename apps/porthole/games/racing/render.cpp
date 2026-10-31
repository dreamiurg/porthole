#include "launcher_icon.h"
#include "render.h"
#include <math.h>
#include <new>
#include <stdio.h>
#include <string.h>
#include "assets.h"
#include "font.h"
#include "fonts.h"
#include "gfx565.h"
#include "palette.h"
#include "pictures.h"
#include "save.h"

// The look follows the approved reference boards (docs/superpowers/specs/porthole-racing/assets/reference/): Style 2
// panels 2A-2D and the four-track showcase. Pictures come from tools/pictures.py, the font and road colors from
// tools/art.py; everything is a flat fill or a pre-made picture, no runtime blending.
namespace racing::paint {
namespace {
// Rally's own interface colors: a navy-black plate, white and gold numerals with a black outline, a race-orange accent.
constexpr uint32_t INK = 0x141623, PLATE = 0x1F2438, PLATE_HI = 0x343C5C, RIM = 0x07080C, CREAM = 0xFFF6E4;
constexpr uint32_t DIM = 0x8A8FA3, ORANGE = 0xFF8A1E, CYAN = 0x45E3FF, GOLD = 0xFFD23A, GREEN = 0x7BE07B;
constexpr uint32_t HELD = 0xFF5A36;   // the speed held back by a trap
constexpr uint32_t RING = 0xDCE2EC, STEEL = 0xB4BCC6, STEEL_DARK = 0x5C6470;
constexpr float MPH_AT_TOP = 90;
constexpr int HORIZON = 228, CAR_ROW = 402, CAR_DEPTH = CAR_ROW - HORIZON, PIC_SCALE = 2;
constexpr float DEPTH_K = 6.0f * CAR_DEPTH;   // z (m ahead of the camera) = DEPTH_K / rows below the horizon
constexpr float CAMERA_BACK = 6, ROAD_SLOPE = 1.25f, CURVE_K = 0.0042f;
constexpr int ROWS = 480 - HORIZON;
float scroll = 0, scrollDist = 0;          // the skyline slides the way the road bends

void fill(int x, int y, int w, int h, uint32_t c) { gfx565::rect(x, y, w, h, rgb888_to_565(c)); }

// ---- text: Russo One (fonts.h), four sizes picked by the old scale numbers so layouts read 2 = body ... 8 = countdown.
// y is the top of the capitals, as before, so a line sits where its letters start.
const font::Font& face(int scale) { return scale <= 2 ? fonts::SMALL : scale == 3 ? fonts::LABEL : scale <= 5 ? fonts::BIG : fonts::GIANT; }
int capTop(const font::Font& f) { return f.lineHeight - f.baseLine - f.glyphs['H' - 31].boxH; }   // line top to cap top
int textWidth(const char* s, int scale) { return font::textWidth(face(scale), s); }
void ink(int x, int y, const char* s, int scale, uint16_t c) { font::text(face(scale), s, x, y - capTop(face(scale)), c); }
void text(int x, int y, const char* s, int scale, uint32_t color) {   // font::text logs the line for the UI audit
  ink(x, y, s, scale, rgb888_to_565(color));
}
void centered(int cx, int y, const char* s, int scale, uint32_t color) { text(cx - textWidth(s, scale) / 2, y, s, scale, color); }
// The reference HUD's numerals: bright letters in a thick black outline, straight on the scene. Only the fill is
// logged, against the outline it sits on.
void outlined(int x, int y, const char* s, int scale, uint32_t color) {
  const int t = scale > 3 ? 3 : 2;
  const bool logging = gfx::textLogEnabled;
  gfx::textLogEnabled = false;
  for (int dy = -t; dy <= t; dy += t) for (int dx = -t; dx <= t; dx += t) if (dx || dy) ink(x + dx, y + dy, s, scale, rgb888_to_565(RIM));
  ink(x, y, s, scale, rgb888_to_565(color));
  gfx::textLogEnabled = logging;
  const font::Font& f = face(scale);
  if (logging && gfx::textLogCount < 64)
    gfx::textLog[gfx::textLogCount++] = {(int16_t)((x - t) / 3), (int16_t)((y - t) / 3), (int16_t)((textWidth(s, scale) + 2 * t + 2) / 3),
                                         (int16_t)((f.lineHeight - f.baseLine - capTop(f) + 2 * t + 2) / 3), color, RIM, true};
}

// ---- pictures
void pic(const pic::Pic& p, int x, int y, int scale, bool dim = false) {
  for (int sy = 0; sy < p.h; sy++)
    for (int sx = 0; sx < p.w; sx++) {
      const uint16_t c = p.px[sy * p.w + sx];
      if (c != pic::CLEAR && (!dim || ((sx + sy) & 1))) gfx565::rect(x + sx * scale, y + sy * scale, scale, scale, c);
    }
}
// Nearest-neighbor at any size, anchored at its bottom center: how far things shrink toward the horizon.
void scaled(const pic::Pic& p, int cx, int bottom, float scale) {
  const int w = (int)(p.w * scale + 0.5f), h = (int)(p.h * scale + 0.5f);
  if (w < 1 || h < 1) return;
  const int x0 = cx - w / 2, y0 = bottom - h;
  const int from = x0 < 0 ? -x0 : 0, to = x0 + w > 480 ? 480 - x0 : w;   // only the columns on the panel
  if (from >= to) return;
  for (int y = y0 < 0 ? -y0 : 0; y < h && y0 + y < 480; y++) {
    const uint16_t* src = p.px + (y * p.h / h) * p.w;
    uint16_t* out = gfx565::fb + (y0 + y) * 480 + x0;
    for (int x = from; x < to; x++) {
      const uint16_t c = src[x * p.w / w];
      if (c != pic::CLEAR) out[x] = c;
    }
  }
}

// ---- interface pieces
void plate(const ui::Box& b, uint32_t face, uint32_t rim) {
  const gfx565::Rect r = {b.x * 3, b.y * 3, b.w * 3, b.h * 3};
  gfx565::roundRect(r, 14, rgb888_to_565(rim));
  gfx565::roundRect({r.x + 4, r.y + 4, r.w - 8, r.h - 8}, 11, rgb888_to_565(face));
}
void button(const ui::Box& b, const char* label, bool on) {
  UiAudit::add(b.x, b.y, b.w, b.h);
  plate(b, on ? PLATE_HI : INK, on ? ORANGE : PLATE);
  centered(b.x * 3 + b.w * 3 / 2, b.y * 3 + b.h * 3 / 2 - 10, label, 2, on ? CREAM : DIM);
}
void heading(int y, const char* s, int scale, uint32_t color) { centered(240, y, s, scale, color); }
void sky(int track, int bottom) {   // flat bands from the theme's two sky colors, then its clouds
  const art::Theme& t = art::THEME[track];
  for (int i = 0; i < 6; i++) {
    auto mix = [&](int s) { return (((t.skyTop >> s & 255) * (5 - i) + (t.skyLow >> s & 255) * i) / 5) << s; };
    fill(0, bottom * i / 6, 480, bottom / 6 + 1, mix(16) | mix(8) | mix(0));
  }
}
void clouds(int track, int offset) {
  if (track == 1 || track == 3) return;   // Industrial's sunset haze and Night Run's indigo stay clear
  const pic::Pic& c = pic::CLOUDS;
  for (int x = -((offset / 2) % 360 + 360) % 360 - 60; x < 480; x += 360) pic(c, x, 34, 2);
}
// The skyline at 2x, mirrored every other tile so it repeats without a seam, its bottom row on the horizon.
void skyline(int track, int bottom, int offset) {
  const pic::Pic& b = *pic::BACKDROP[track];
  const int top = bottom - b.h * 2, start = (offset % (2 * b.w) + 2 * b.w) % (2 * b.w);
  for (int y = 0; y < b.h; y++) {
    const int py = top + y * 2;
    if (py < 0 || py + 1 >= 480) continue;
    const uint16_t* row = b.px + y * b.w;
    uint16_t* out = gfx565::fb + py * 480;
    for (int x = 0, u = start; x < 240; x++, u = u + 1 == 2 * b.w ? 0 : u + 1) {
      const uint16_t c = row[u < b.w ? u : 2 * b.w - 1 - u];
      if (c != pic::CLEAR) out[2 * x] = out[2 * x + 1] = c;
    }
    memcpy(out + 480, out, 480 * sizeof(uint16_t));   // each art row covers two panel rows
  }
}

// ---- the road. Its per-row tables and the list of things on it live on the heap (PSRAM on the board), allocated
// at the first race frame, never at static init: internal RAM stays free for the wireless update's screen and TLS.
struct Thing { float z; const pic::Pic* p; float x, lift = 0; bool shadow = false; float size = 1; };   // size: drawn bigger than life
constexpr int MAX_THINGS = 48;
// The speed arc hugs the top rim: SEGMENTS slots across ARC_DEG either side of 12 o'clock, between ARC_IN and ARC_OUT.
// Which slot each of its pixels belongs to (or its dark rim) is worked out once, so a frame only looks it up.
constexpr int SEGMENTS = 24, ARC_IN = 210, ARC_OUT = 228, ARC_MAX = 9000, ARC_RIM = 255;
constexpr float ARC_DEG = 56;
struct Scratch {
  int16_t center[ROWS], half[ROWS];   // road center and half-width for each row below the horizon
  float z[ROWS];
  Thing things[MAX_THINGS];
  int n;
  uint32_t arc[ARC_MAX];              // panel offset << 8 | slot (ARC_RIM: the dark edge and the gaps)
  int arcCount;
};
Scratch* sc = nullptr;
void buildArc();   // with the HUD, below
struct Camera { const Race* r; int track; float dist, x; };
float rowZ(int y) { return DEPTH_K / (float)(y - HORIZON); }
void project(const Camera& cam) {   // bottom row up: the bend accumulates with distance
  const Track& t = *cam.r->track;
  float offset = 0, slope = 0, lastZ = rowZ(479);
  for (int y = 479; y > HORIZON; y--) {
    const int i = y - HORIZON - 1;
    const float z = rowZ(y), dz = z - lastZ;
    slope += curve(t, cam.dist + z) * CURVE_K * dz;
    offset += slope * dz;
    lastZ = z;
    sc->z[i] = z;
    sc->half[i] = (int16_t)(ROAD_SLOPE * (y - HORIZON));
    sc->center[i] = (int16_t)(240 + (offset - cam.x) * sc->half[i]);
  }
}
bool patchHere(const Track& t, const Feature& o, float z) {
  const float at = featureAt(t, o);
  return o.kind == Kind::Ramp ? (z >= at - 5 && z <= at) : (z >= at && z <= at + featureLen(t, o));
}
void patches(const Camera& cam, int y, float world) {   // ramps, mud, oil, slush, ice, puddles on this row
  const Track& t = *cam.r->track;
  const art::Theme& th = art::THEME[cam.track];
  const int i = y - HORIZON - 1;
  const float z = lapPos(t, world);
  for (int f = 0; f < t.nFeatures; f++) {
    const Feature& o = t.features[f];
    if ((o.kind != Kind::Ramp && o.kind != Kind::Slow && o.kind != Kind::Ice) || !patchHere(t, o, z)) continue;
    const int cx = sc->center[i] + (int)(o.x / 100.0f * sc->half[i]), hw = (int)(o.w / 100.0f * sc->half[i]);
    const float at = featureAt(t, o), end = o.kind == Kind::Ramp ? at : at + featureLen(t, o);
    const bool edge = o.kind != Kind::Ramp && (z < at + 1.2f || z > end - 1.2f);
    uint32_t c = o.kind == Kind::Ice ? th.ice : th.hazard, ring = o.kind == Kind::Ice ? th.iceEdge : th.hazardEdge;
    if (o.kind == Kind::Ramp) {   // the showcase's ramp: orange chevrons on a cream face, dark sides
      const float u = (z - (at - 5)) * 1.6f;
      c = ((int)u & 1) ? ORANGE : CREAM;
      ring = RIM;
    }
    fill(cx - hw, y, 2 * hw, 1, edge ? ring : c);
    fill(cx - hw - 3, y, 3, 1, ring); fill(cx + hw, y, 3, 1, ring);
  }
}
void rails(int y, int cx, int hw, float world) {   // guardrails on both shoulders, as in the reference
  const int h = hw / 8 + 1, w = hw / 60 + 1;
  const bool post = (int)floorf(world / 3) != (int)floorf((world + 0.6f) / 3);
  for (int k = -1; k <= 1; k += 2) {
    const int x = cx + k * (int)(hw * (RAIL + CAR_HALF_W));   // where the car's side stops: the rail it leans on
    fill(x - w, y - h, 2 * w, h / 3 + 1, STEEL);
    fill(x - w, y - h + h / 3 + 1, 2 * w, h / 5 + 1, STEEL_DARK);
    if (post) fill(x - w / 2, y - h, w + 1, h, STEEL_DARK);
  }
}
void roadRow(const Camera& cam, int y) {
  const art::Theme& th = art::THEME[cam.track];
  const int i = y - HORIZON - 1, cx = sc->center[i], hw = sc->half[i];
  const float world = cam.dist + sc->z[i];
  const bool band = (int)floorf(world / 6) & 1;
  const int rumble = hw / 7 + 2;   // five spans that never overlap: every pixel of the row written once
  const uint32_t grass = band ? th.grassA : th.grassB, edge = band ? th.rumbleA : th.rumbleB;
  fill(0, y, cx - hw - rumble, 1, grass);
  fill(cx - hw - rumble, y, rumble, 1, edge);
  fill(cx - hw, y, 2 * hw, 1, band ? th.roadA : th.roadB);
  fill(cx + hw, y, rumble, 1, edge);
  fill(cx + hw + rumble, y, 480 - (cx + hw + rumble), 1, grass);
  const int line = hw / 40 + 1;
  fill(cx - hw + rumble / 2, y, line, 1, CREAM); fill(cx + hw - rumble / 2 - line, y, line, 1, CREAM);
  if ((int)floorf(world / 4) % 3 == 0) {   // dashed lane guides: visual only, steering stays continuous
    fill(cx - line / 2 - line, y, 2 * line, 1, th.lane);
    for (int k = -1; k <= 1; k += 2) fill(cx + k * hw / 2 - line / 2, y, line, 1, CREAM);
  }
  const float lap = lapPos(*cam.r->track, world);
  if (lap < 2 || lap > cam.r->track->length - 2)   // the start/finish line: a checkered band
    for (int x = -hw; x < hw; x += 12) fill(cx + x, y, 6, 1, ((x / 12 + (int)(lap * 2)) & 1) ? CREAM : RIM);
  patches(cam, y, world);
}

// ---- things beside and on the road, far to near
void add(const Thing& t) {
  if (t.z < 1.5f || t.z > rowZ(HORIZON + 3) || sc->n >= MAX_THINGS) return;
  sc->things[sc->n++] = t;
}
void placeTrack(const Camera& cam) {
  const Track& t = *cam.r->track;
  for (int f = 0; f < t.nFeatures; f++) {
    const Feature& o = t.features[f];
    const float ahead = featureAt(t, o) - lapPos(t, cam.dist);
    const float z = ahead < -10 ? ahead + t.length : ahead;
    if (o.kind == Kind::Obstacle) add({z + featureLen(t, o) / 2, pic::OBSTACLE[cam.track], o.x / 100.0f});
    if (o.kind == Kind::Pickup && !(cam.r->taken & (1u << f)))
      add({z, pic::PICKUP[o.item], o.x / 100.0f, o.raised ? 0.9f : 0.15f, o.raised});
  }
  const float first = ceilf(cam.dist / 18) * 18;   // roadside scenery past the rails, alternating sides
  for (float d = first; d < cam.dist + 220; d += 18) {
    const int n = (int)(d / 18);
    add({d - cam.dist, pic::PROP[cam.track][(n / 2) & 1], (n & 1) ? 1.95f : -1.95f});
  }
}
float hop(const Car& c) { return airborne(c) ? sinf((float)(AIR_MS - c.airMs) / AIR_MS * 3.14159f) : 0; }   // 0..1..0
void placeCars(const Camera& cam) {
  const Race& r = *cam.r;
  for (int i = 1; i < CARS; i++) {
    const Car& c = r.car[i];
    if (c.finished) continue;   // home rivals leave the road
    const float lift = hop(c) * 0.9f;
    add({c.dist - cam.dist, pic::CAR[i], c.x, lift, true});
  }
  for (const Rocket& k : r.rocket) if (k.live) add({k.dist - cam.dist, &pic::MISSILE, k.x, 0.15f, false, 2.6f});   // easy to follow
  for (const Puff& p : r.puff) if (p.ms) add({p.dist - cam.dist + 0.5f, &pic::PUFF, p.x, 0.2f, false, 1.8f});
}
void drawThing(const Thing& t) {
  const int y = HORIZON + (int)(DEPTH_K / t.z);
  if (y <= HORIZON || y >= 480) return;
  const int i = y - HORIZON - 1;
  const float scale = t.size * PIC_SCALE * sc->half[i] / (ROAD_SLOPE * CAR_DEPTH);
  const int x = sc->center[i] + (int)(t.x * sc->half[i]);
  const int lift = (int)(t.lift * 60 * scale / PIC_SCALE);
  if (t.shadow) fill(x - (int)(t.p->w * scale * 0.42f), y - 2, (int)(t.p->w * scale * 0.84f), (int)(3 * scale) + 1, RIM);
  scaled(*t.p, x, y - lift, scale);
}
void drawThings() {
  for (int i = 1; i < sc->n; i++)   // far first
    for (int j = i; j > 0 && sc->things[j].z > sc->things[j - 1].z; j--) { const Thing t = sc->things[j]; sc->things[j] = sc->things[j - 1]; sc->things[j - 1] = t; }
  for (int i = 0; i < sc->n; i++) drawThing(sc->things[i]);
}
void player(const Camera& cam, float steer, uint32_t ms) {
  const Car& p = cam.r->car[PLAYER];
  const int i = CAR_ROW - HORIZON - 1;
  int x = sc->center[i] + (int)((p.x) * sc->half[i]) + (int)(steer * 3);
  if (p.wobbleMs) x += ((ms / 40) & 1) ? 3 : -3;
  const pic::Pic& s = *pic::CAR[0];
  const int lift = (int)(hop(p) * 44);
  fill(x - s.w + 4, CAR_ROW - 6, 2 * s.w - 8, 7, RIM);   // the shadow stays on the road
  if (p.boostMs)   // short exhaust flames, bounded and baked: no glow
    for (int k = -1; k <= 1; k += 2) fill(x + k * 14 - 3, CAR_ROW - lift - 4, 7, 10 + (int)((ms / 30) % 3) * 5, (ms / 60) & 1 ? ORANGE : GOLD);
  pic(s, x - s.w, CAR_ROW - lift - 2 * s.h, PIC_SCALE);
}
void scene(const Race& r, int track, float steer, uint32_t ms) {
  if (!sc) { sc = new (std::nothrow) Scratch(); if (sc) buildArc(); }
  if (!sc) return;
  const Car& p = r.car[PLAYER];
  Camera cam{&r, track, p.dist - CAMERA_BACK, p.x * 0.7f};
  if (fabsf(p.dist - scrollDist) < 50) scroll = fmodf(scroll + curve(*r.track, p.dist) * (p.dist - scrollDist) * 0.9f, 3600);
  else scroll = 0;   // a new race starts the skyline afresh
  scrollDist = p.dist;
  project(cam);
  sky(track, HORIZON);
  clouds(track, (int)scroll);
  skyline(track, HORIZON + 1, (int)scroll);
  for (int y = HORIZON + 1; y < 480; y++) roadRow(cam, y);
  for (int y = HORIZON + 6; y < 480; y++) rails(y, sc->center[y - HORIZON - 1], sc->half[y - HORIZON - 1], cam.dist + sc->z[y - HORIZON - 1]);
  sc->n = 0;
  placeTrack(cam);
  placeCars(cam);
  drawThings();
  player(cam, steer, ms);
}

// ---- the race HUD, after the reference: lap and place numerals on the scene, two round action buttons
const char* ordinal(int n) { static const char* O[] = {"1st", "2nd", "3rd", "4th"}; return O[(n < 1 ? 1 : n > 4 ? 4 : n) - 1]; }
void action(const ui::Box& b, int item, int count, bool lit, bool full) {
  UiAudit::add(b.x, b.y, b.w, b.h);
  const int cx = b.x * 3 + b.w * 3 / 2, cy = b.y * 3 + b.h * 3 / 2, inward = item == ROCKET ? 1 : -1;
  gfx565::circle(cx, cy, 39, rgb888_to_565(RIM));
  gfx565::circle(cx, cy, 36, rgb888_to_565(lit ? (item == BOOST ? CYAN : ORANGE) : count ? RING : DIM));
  gfx565::circle(cx, cy, 32, rgb888_to_565(count ? PLATE : INK));
  const pic::Pic& icon = *pic::PICKUP[item];
  if (count) scaled(icon, cx, cy + 22, 2.2f);
  else pic(icon, cx - icon.w, cy - icon.h, 2, true);   // spent: a dotted icon
  char n[2] = {(char)('0' + count), 0};
  outlined(cx + inward * 20 - 12, cy + 12, n, 3, count ? CREAM : DIM);   // the count, toward the middle
  if (full) { const int w = textWidth("Full", 2); outlined(cx + inward * 8 - w / 2, cy - 70, "Full", 2, GOLD); }
}
void buildArc() {
  sc->arcCount = 0;
  for (int y = 240 - ARC_OUT; y < 240; y++)
    for (int x = 0; x < 480; x++) {
      const float dx = (float)(x - 240), dy = (float)(240 - y), r = sqrtf(dx * dx + dy * dy);
      const float deg = atan2f(dx, dy) * (180 / 3.14159265f);
      if (r < ARC_IN || r > ARC_OUT || fabsf(deg) > ARC_DEG || sc->arcCount >= ARC_MAX) continue;
      const float at = (deg + ARC_DEG) / (2 * ARC_DEG) * SEGMENTS, inSlot = at - floorf(at);
      const bool edge = r < ARC_IN + 2 || r > ARC_OUT - 2 || inSlot < 0.12f || inSlot > 0.88f;
      sc->arc[sc->arcCount++] = (uint32_t)(y * 480 + x) << 8 | (edge ? ARC_RIM : (uint32_t)at);
    }
}
// Slot colors: the road range runs green to gold to orange; the slots past top speed are the boost's cyan.
uint16_t slotColor(int i) {
  constexpr int ROAD = (int)(SEGMENTS / BOOST_CAP + 0.5f);
  if (i >= ROAD) return rgb888_to_565(CYAN);
  const float t = (float)i / (ROAD - 1);
  const uint32_t a = t < 0.5f ? GREEN : GOLD, b = t < 0.5f ? GOLD : ORANGE;
  const float u = t < 0.5f ? t * 2 : t * 2 - 1;
  auto mix = [&](int s) { return (uint32_t)((a >> s & 255) * (1 - u) + (b >> s & 255) * u) << s; };
  return rgb888_to_565(mix(16) | mix(8) | mix(0));
}
// The speed: an arc of slots along the top rim lighting left to right, and big digits under it. Held back (a slow
// patch, off the road, the moment after a hit) the lit slots and digits turn red-orange; boosting, the digits are
// cyan as the boost slots fill. 1.0 (dry-road top speed) reads 90 mph.
void speedo(const Race& r) {
  const Car& p = r.car[PLAYER];
  bool ice = false;
  const bool slowed = p.protectMs || (!airborne(p) && terrainCap(*r.track, p.dist, p.x, &ice) < 1);
  const int lit = (int)lroundf(fminf(p.speed / BOOST_CAP, 1) * SEGMENTS);
  const uint16_t rim = rgb888_to_565(RIM), off = rgb888_to_565(PLATE), held = rgb888_to_565(HELD);
  for (int i = 0; i < sc->arcCount; i++) {
    const uint32_t slot = sc->arc[i] & 255;
    gfx565::fb[sc->arc[i] >> 8] = slot == ARC_RIM ? rim : (int)slot >= lit ? off : slowed ? held : slotColor((int)slot);
  }
  const uint32_t color = p.boostMs ? CYAN : slowed ? HELD : CREAM;
  char n[8];
  snprintf(n, sizeof n, "%d", (int)lroundf(p.speed * MPH_AT_TOP));
  outlined(262 - textWidth(n, 5), 40, n, 5, color);
  outlined(268, 62, "mph", 2, color);
}
void hud(const View& v) {
  const Race& r = *v.race;
  speedo(r);
  UiAudit::add(LAP.x, LAP.y, LAP.w, LAP.h);
  char lap[8];
  snprintf(lap, sizeof lap, "%d/%d", lapOf(r, PLAYER), LAPS);
  const int lx = LAP.x * 3, ly = LAP.y * 3, lw = LAP.w * 3;
  outlined(lx + (lw - textWidth(lap, 4)) / 2, ly + 6, lap, 4, CREAM);
  const int hw = textWidth("Hold", 2), mx = lx + (lw - hw - 16) / 2;
  fill(mx - 2, ly + 46, 8, 18, RIM); fill(mx + 5, ly + 46, 8, 18, RIM);
  fill(mx, ly + 48, 4, 14, CREAM); fill(mx + 7, ly + 48, 4, 14, CREAM);   // a pause mark, then the word
  outlined(mx + 16, ly + 46, "Hold", 2, CREAM);
  if (v.hold) { fill(lx + 6, ly + 70, lw - 12, 6, RIM); fill(lx + 8, ly + 72, (lw - 16) * v.hold / 1000, 2, GOLD); }
  const char* place = ordinal(r.place);
  const char num[2] = {place[0], 0};
  const int px = PLACE.x * 3 + 4, py = PLACE.y * 3 + 2;
  outlined(px, py, num, 6, GOLD);   // the reference's big place digit and its small suffix
  outlined(px + textWidth(num, 6) + 6, py + 4, place + 1, 3, GOLD);
  action(ROCKET_BTN, ROCKET, r.inventory[ROCKET], r.cooldownMs > 0, r.fullMs[ROCKET] > 0);
  action(BOOST_BTN, BOOST, r.inventory[BOOST], r.car[PLAYER].boostMs > 0, r.fullMs[BOOST] > 0);
}
void banner(int y, const char* s, int scale, uint32_t color) {
  outlined(240 - textWidth(s, scale) / 2, y, s, scale, color);
}
void clock(char* out, size_t n, uint32_t ms) {   // m:ss.t
  snprintf(out, n, "%u:%02u.%u", (unsigned)(ms / 60000), (unsigned)(ms / 1000 % 60), (unsigned)(ms / 100 % 10));
}
// The time to beat on this track: the record holder's, or the kid's own; false when nobody has a time yet.
bool toBeat(const View& v, char* line, size_t n) {
  if (!v.boardRows) return false;
  char t[16];
  clock(t, sizeof t, v.board[0].raceMs);
  if (v.board[0].me) snprintf(line, n, "Your record %s", t);
  else snprintf(line, n, "To beat: %s %s", v.board[0].name, t);
  return true;
}
void callouts(const View& v) {
  const Race& r = *v.race;
  if (r.phase == Phase::Countdown) {
    static const char* C[] = {"3", "2", "1"};
    const uint32_t i = r.ms / 1000;
    banner(150, C[i < 3 ? i : 2], 8, GOLD);
    char line[48];   // the time to beat again on a Replay, which skips the grip page
    if (toBeat(v, line, sizeof line)) outlined(240 - textWidth(line, 2) / 2, 232, line, 2, GOLD);
  } else if (r.raceMs < 700) banner(150, "GO!", 8, GREEN);
  if (r.calloutMs) banner(170, r.playerLap >= LAPS ? "Final lap" : "Lap 2", 4, CREAM);
  if (v.resumeMs) banner(160, "Ready", 5, GOLD);
  if (r.hitMs) banner(196, "Hit!", 4, ORANGE);
  if (r.phase == Phase::Finished) banner(186, "Finish!", 6, GOLD);   // under the lap chip
}

// ---- pages
void track(int i, int x, int y, int w, int h) {   // a track's own sky and skyline in a box
  const art::Theme& t = art::THEME[i];
  fill(x, y, w, h, t.skyLow);
  const pic::Pic& b = *pic::BACKDROP[i];
  const int top = y + h - b.h;
  for (int sy = top < y ? y - top : 0; sy < b.h; sy++)
    for (int sx = 0; sx < w; sx++) {
      const uint16_t c = b.px[sy * b.w + (sx + (b.w - w) / 2)];
      if (c != pic::CLEAR) gfx565::pixel(x + sx, top + sy, c);
    }
}
void wheelPreview(float steer, int trackId, int y) {   // a strip of road and the car, moving with the wheel
  const art::Theme& th = art::THEME[trackId];
  fill(110, y, 260, 72, th.roadA);
  fill(110, y, 260, 6, th.rumbleA); fill(110, y + 66, 260, 6, th.rumbleA);
  const pic::Pic& s = *pic::CAR[0];
  pic(s, 240 + (int)(steer * 90) - s.w, y + 6, 2);
}
void tracksPage() {
  gfx565::clear(rgb888_to_565(INK));
  banner(38, "RALLY", 4, ORANGE);
  heading(78, "Pick a track", 2, CREAM);
  for (int i = 0; i < TRACKS; i++) {
    const ui::Box& b = CARD[i];
    UiAudit::add(b.x, b.y, b.w, b.h);
    plate(b, PLATE, PLATE_HI);
    track(i, b.x * 3 + 6, b.y * 3 + 6, b.w * 3 - 12, 52);
    centered(b.x * 3 + b.w * 3 / 2, b.y * 3 + 62, TRACK[i].line1, 2, CREAM);
    centered(b.x * 3 + b.w * 3 / 2, b.y * 3 + 84, TRACK[i].line2, 2, CREAM);
  }
  button(HOME, "Home", true);
}
void howToPage(const View& v) {
  gfx565::clear(rgb888_to_565(INK));
  heading(54, "How to race", 3, ORANGE);
  pic(*pic::CAR[0], 92 + (int)(sinf(v.ms / 300.0f) * 6), 100, 2);
  text(176, 118, "Turn to steer", 2, CREAM);
  pic(pic::BOOST, 86, 176, 2); pic(pic::ROCKET, 126, 176, 2);
  text(176, 182, "Grab boosts", 2, CREAM);
  text(176, 206, "and rockets", 2, CREAM);
  centered(240, 246, "Finish 3 laps", 2, CREAM);
  centered(240, 280, "Hold 1/3 at the top to pause", 2, GOLD);
  button(BACK, "Back", true);
  button(NEXT, "Next", true);
}
void calibratePage(const View& v) {
  gfx565::clear(rgb888_to_565(INK));
  heading(60, "Hold like a wheel", 2, ORANGE);
  const char* status = v.gripReady ? "Ready! Turn to test" : v.gripUpright ? "Hold it steady" : "Lift it up";
  heading(96, status, 2, v.gripReady ? GREEN : CREAM);
  fill(150, 134, 180, 8, PLATE);
  fill(150, 134, 180 * v.calibration / 1000, 8, v.gripReady ? GREEN : GOLD);
  wheelPreview(v.steer, v.track, 162);
  centered(240, 252, "Turn right: car goes right", 2, DIM);
  char line[48];   // the time to beat on this track, before the kid starts
  if (toBeat(v, line, sizeof line)) centered(240, 288, line, 2, GOLD);
  button(BACK, "Back", true);
  button(NEXT, "Go", v.gripReady);
}
void pausePage(const View& v) {
  gfx565::clear(rgb888_to_565(INK));
  if (v.gripLost && !v.gripHeld) heading(46, "Hold like a wheel", 2, GOLD);
  else heading(48, "Paused", 3, ORANGE);
  button(RESUME, "Resume", v.gripHeld);
  button(RECENTER, "New grip", true);
  button(RESTART, "Restart", true);
  button(TRACKS_BTN, "Tracks", true);
  button(HOME_BTN, "Home", true);
}
void confirmPage(const View& v) {
  gfx565::clear(rgb888_to_565(INK));
  static const char* Q[] = {"Start over?", "Pick a new track?", "Go home?"};
  heading(150, Q[(int)v.leave], 2, ORANGE);
  heading(188, "This race ends.", 2, CREAM);
  button(YES, "Yes", true);
  button(NO, "No", true);
}
// The finish, then this track's leaderboard: every kid's best race and best lap, fastest first, this kid in gold.
void resultsPage(const View& v) {
  const Race& r = *v.race;
  gfx565::clear(rgb888_to_565(INK));
  heading(40, "Finished!", 3, ORANGE);
  banner(80, ordinal(r.place), 5, GOLD);
  char t[16], line[64];
  clock(t, sizeof t, r.car[PLAYER].finishMs);
  // Said plainly for a six-year-old: a best of their own is not beating a sibling's record.
  const uint32_t mine = r.car[PLAYER].finishMs;
  const bool top = v.boardRows && v.board[0].me && v.board[0].raceMs == mine;
  if (v.newBest & BEST_RACE) snprintf(line, sizeof line, "%s  %s", t, top && v.boardRows > 1 ? "Track record!" : "Your best yet!");
  else if (v.newBest & BEST_LAP) snprintf(line, sizeof line, "%s  Best lap yet!", t);
  else if (v.prevBestMs && mine > v.prevBestMs)
    snprintf(line, sizeof line, "%s  (+%u.%u)", t, (unsigned)((mine - v.prevBestMs) / 1000), (unsigned)((mine - v.prevBestMs) / 100 % 10));
  else snprintf(line, sizeof line, "Your time %s", t);
  centered(240, 136, line, 2, v.newBest ? GREEN : CREAM);
  centered(240, 166, "Best times", 2, DIM);
  for (int i = 0; i < v.boardRows; i++) {
    char lap[16];
    clock(t, sizeof t, v.board[i].raceMs);
    const uint32_t lm = v.board[i].lapMs;
    if (!lm) snprintf(lap, sizeof lap, "-");
    else if (lm < 60000) snprintf(lap, sizeof lap, "%u.%u", (unsigned)(lm / 1000), (unsigned)(lm / 100 % 10));   // "29.8"
    else clock(lap, sizeof lap, lm);
    snprintf(line, sizeof line, "%d %.11s %s  lap %s", i + 1, v.board[i].name, t, lap);
    centered(240, 194 + i * 26, line, 2, v.board[i].me ? GOLD : CREAM);
  }
  button(REPLAY, "Replay", true);
  button(RESULT_TRACKS, "Tracks", true);
  button(RESULT_HOME, "Home", true);
}

}  // namespace

void draw(const View& v) {
  if (!gfx565::fb) return;
  switch (v.page) {
    case Page::Tracks: tracksPage(); break;
    case Page::HowTo: howToPage(v); break;
    case Page::Calibrate: calibratePage(v); break;
    case Page::Race: scene(*v.race, v.track, v.steer, v.ms); hud(v); callouts(v); break;
    case Page::Pause: pausePage(v); break;
    case Page::Confirm: confirmPage(v); break;
    case Page::Results: resultsPage(v); break;
  }
}

const gfx::Sprite& icon() { return LAUNCHER_ICON; }
}  // namespace racing::paint
