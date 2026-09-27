#include "game.h"
#include <math.h>
#include <stdio.h>
#include <string.h>
#include <initializer_list>
#include "generated/icon.h"
#include "gfx565.h"

#ifdef ESP_PLATFORM
#pragma GCC optimize("O2")   // the board builds -Os; every cell, every frame
#endif
namespace sandjar {
namespace {
constexpr float PI = 3.14159265f, F = 2.0f / 3;   // F: the prototype's speeds are in its 2 px grains
constexpr int HOME_CX = 80, HOME_CY = 12, HOME_HIT_R = 16;   // logical: ui::back's place and size, like every game
constexpr ui::Box MODE_BOX = {56, 132, 48, 22};              // the mode button: inside the glass at the bottom
constexpr ui::Box HOME_CELLS = {68, 0, 25, 26}, MODE_CELLS = {55, 131, 50, 24};   // cells the buttons may cover

struct Rgb { float r, g, b; };
constexpr uint16_t rgb(uint32_t c) { return (uint16_t)(((c >> 8) & 0xF800) | ((c >> 5) & 0x07E0) | ((c >> 3) & 0x001F)); }
constexpr Rgb hex(uint32_t c) { return {(float)(c >> 16 & 255), (float)(c >> 8 & 255), (float)(c & 255)}; }
int byte(float v) { const int i = (int)v; return i < 0 ? 0 : i > 255 ? 255 : i; }   // clamped as an int: no float compares
inline uint16_t pack(float r, float g, float b) { return (uint16_t)((byte(r) >> 3) << 11 | (byte(g) >> 2) << 5 | byte(b) >> 3); }
Rgb unpack(uint16_t c) { const uint32_t v = gfx565::rgb888(c); return hex(v); }
// lighten() for whole steps of light (the glass's reflection, Orbits' trails): no float on the board's slow path.
uint16_t brighten(uint16_t c, int r, int g, int b) {
  const uint32_t v = gfx565::rgb888(c);
  r += (int)(v >> 16); g += (int)(v >> 8 & 255); b += (int)(v & 255);
  return (uint16_t)((r > 255 ? 255 : r) >> 3 << 11 | (g > 255 ? 255 : g) >> 2 << 5 | (b > 255 ? 255 : b) >> 3);
}

// The prototype's colors. Sand Jar's warm sand, Rainbow Sand's six strata, Beach's water, Orbits' stars.
constexpr Rgb SANDS[6] = {hex(0xe9c27a), hex(0xd9a85b), hex(0xf2d39a), hex(0xc98f48), hex(0xe3b66c), hex(0xb97e3c)};
constexpr Rgb STRATA[6] = {hex(0xff6b9a), hex(0xffb547), hex(0xffe066), hex(0x5ee3b0), hex(0x4cc3ff), hex(0x9d7bff)};
constexpr Rgb STARS[5] = {hex(0xffd36e), hex(0x7fd8ff), hex(0xff8f6b), hex(0xc9a2ff), hex(0xffffff)};
constexpr Rgb WATER_DEEP = hex(0x0a4a73), WATER_MID = hex(0x1a86bd), WATER_TOP = hex(0x4fc0e6), FOAM = hex(0xd8f6ff);
constexpr uint32_t BG[MODE_COUNT][2] = {{0x2a2036, 0x08070c}, {0x241a2c, 0x07060b}, {0x12303a, 0x03090c}, {0x161338, 0x020207}};
// The mode button's four dots, each a tiny picture of its mode (horizontal bands, top first).
struct Dot { uint32_t band[3]; int bands; };
constexpr Dot DOTS[MODE_COUNT] = {{{0xe9c27a}, 1}, {{0xff6b9a, 0xffe066, 0x4cc3ff}, 3}, {{0x1c8fc4, 0xe9c27a}, 2}, {{0xc9a2ff}, 1}};

const char* const MODES[MODE_COUNT] = {"sand", "rainbow", "beach", "orbits"};
float SIN[256];   // the prototype's 256-step sine (Game::tables)
Rgb WATER_AT[8];  // water's color by depth under the surface, 0..7 cells (Game::tables)
float sine(int i) { return SIN[i & 255]; }
Rgb mix(const Rgb& a, const Rgb& b, float u) { return {a.r + (b.r - a.r) * u, a.g + (b.g - a.g) * u, a.b + (b.b - a.b) * u}; }
uint16_t lighten(uint16_t c, float r, float g, float b) { const Rgb p = unpack(c); return pack(p.r + r, p.g + g, p.b + b); }
void block(int c, uint16_t col) {   // a cell's 3x3 physical px
  uint16_t* p = gfx565::fb + (c / G) * 3 * gfx565::W + (c % G) * 3;
  for (int k = 0; k < 3; k++, p += gfx565::W) p[0] = p[1] = p[2] = col;
}
bool put(uint16_t* seen, bool full, int c, uint16_t col) {   // true when it wrote the cell
  if (!full && seen[c] == col) return false;
  seen[c] = col;
  block(c, col);
  return true;
}
bool inBox(const ui::Box& b, int c) { const int x = c % G, y = c / G; return x >= b.x && y >= b.y && x < b.x + b.w && y < b.y + b.h; }
bool underButtons(int c) { return inBox(HOME_CELLS, c) || inBox(MODE_CELLS, c); }
bool overHome(int x, int y) { const int dx = x - HOME_CX, dy = y - HOME_CY; return dx * dx + dy * dy <= HOME_HIT_R * HOME_HIT_R; }
bool overMode(int x, int y) { return x >= MODE_BOX.x && y >= MODE_BOX.y && x < MODE_BOX.x + MODE_BOX.w && y < MODE_BOX.y + MODE_BOX.h; }
int bodyCell(const Body& b) { return (int)(b.y * (1.0f / 3)) * G + (int)(b.x * (1.0f / 3)); }   // bodies are in panel px
}  // namespace

const gfx::Sprite& Game::icon() const { return SPR_SANDJAR_ICON; }

// The jar keeps its sand between visits, like a real one on a shelf; the touch, the shake and what the panel's
// buffers hold (the shell drew its own screens into them meanwhile) start over.
void Game::enter(const AppEnter& e) {
  ms_ = lastMs_ = e.ms; acc_ = 0;
  wantsHome_ = buttonTouch_ = sensed_ = false;
  moveX_ = moveY_ = 0;
  jar_.finger.on = false; jar_.shaking = 0;
  gate_.shown(ms_);
  for (Seen& s : seen_) {
    if (!s.cells) s.cells = new uint16_t[G * G];   // once: two 51 KB histories, the board may put them in PSRAM
    s.target = nullptr; s.valid = false;
  }
  if (!started_) { tables(); pour(MODE_SAND); started_ = true; }
}

void Game::pour(Mode m) {
  jar_.reset(m, 0x9E3779B9u * ++pours_);
  for (Seen& s : seen_) s.valid = false;   // a new background, also outside the glass, where a frame never looks again
  for (int y = 0; y < G; y++) {   // the glass's cells, row by row: nothing outside them ever changes
    int x0 = 0, x1 = G - 1;
    while (x0 < G && jar_.occ[y * G + x0] == WALL) x0++;
    while (x1 >= x0 && jar_.occ[y * G + x1] == WALL) x1--;
    span_[y][0] = (uint8_t)x0; span_[y][1] = (uint8_t)(x1 + 1);
  }
  if (m == MODE_ORBITS) memset(jar_.sky.glow, 0, sizeof jar_.sky.glow);   // the grains' memory until now
  const Rgb c0 = hex(BG[m][0]), c1 = hex(BG[m][1]);
  for (int k = 0; k < BG_N; k++) {   // a radial gradient, dithered the prototype's +-3.5 (in four steps)
    const float d = sqrtf(k * 32.0f + 16) / 2 / (G / 2), t = powf(d, 1.5f);
    for (int n = 0; n < 4; n++) {
      const float z = (n - 1.5f) * 2.3f;
      bg_[n][k] = d > 1 || k == BG_N - 1 ? 0 : pack(c0.r + (c1.r - c0.r) * t + z, c0.g + (c1.g - c0.g) * t + z, c0.b + (c1.b - c0.b) * t + z);
    }
  }
}

// Mode-independent: the vignette, the dither tile, and the glass's reflection (a soft arc near the upper-left rim).
void Game::tables() {
  for (int k = 0; k < BG_N; k++) {
    const float d = sqrtf(k * 32.0f + 16) / 2 / (G / 2 - 1.5f), t = fminf(1, fmaxf(0, (d - 0.6f) / 0.4f));
    vignette_[k] = (float)(uint8_t)(255 * (1 - 0.38f * t * t)) / 255;
  }
  for (int k = 0; k < 256; k++) SIN[k] = sinf(k / 256.0f * 2 * PI);
  for (int d = 0; d < 8; d++) {   // lighter near the surface, deep blue below
    const float t = d * (1.0f / 7);
    WATER_AT[d] = t < 0.5f ? mix(WATER_TOP, WATER_MID, t * 2) : mix(WATER_MID, WATER_DEEP, t * 2 - 1);
  }
  uint32_t s = 2463534242u;
  for (uint8_t& n : noise_) { s ^= s << 13; s ^= s >> 17; s ^= s << 5; n = (uint8_t)(s >> 30); }
  shines_ = 0;
  for (int c = 0; c < G * G && shines_ < (int)(sizeof shine_ / sizeof shine_[0]); c++) {
    const float dx = (c % G) * 3 + 1.5f - 240, dy = (c / G) * 3 + 1.5f - 240, d = sqrtf(dx * dx + dy * dy) / 240;
    const float da = fabsf(fmodf(atan2f(dy, dx) + 0.75f * PI + 3 * PI, 2 * PI) - PI);
    const float radial = fmaxf(0, 1 - fabsf(d - 0.885f) / 0.045f), along = fmaxf(0, 1 - da / 0.42f), a = 34 * radial * along * along;
    if (a > 1) shine_[shines_++] = {(uint16_t)c, (uint8_t)a};
  }
}

void Game::update(uint32_t, uint32_t ms, const Input& in) {
  ms_ = ms; in_ = in;
  gate_.filter(in_, ms_);
  sense(in);
  touch();
  tick();
}

// The motion sensor: in-plane gravity, eased against noise, flat under FLAT_G; a reading far off 1 g, or one that
// jumps, is a shake.
void Game::sense(const Input& in) {
  const float mag = sqrtf((float)in.gx * in.gx + (float)in.gy * in.gy + (float)in.gz * in.gz);
  const float dx = (float)(in.gx - lastG_[0]), dy = (float)(in.gy - lastG_[1]), dz = (float)(in.gz - lastG_[2]);
  if (sensed_ && (fabsf(mag - 1000) > SHAKE_MG || sqrtf(dx * dx + dy * dy + dz * dz) > JOLT_MG)) jar_.shake();
  lastG_[0] = in.gx; lastG_[1] = in.gy; lastG_[2] = in.gz; sensed_ = true;
  float tx = in.gx / 1000.0f, ty = in.gy / 1000.0f;
  const float m = sqrtf(tx * tx + ty * ty);
  if (m > 1) { tx /= m; ty /= m; }
  tiltX_ += (tx - tiltX_) * TILT_EASE; tiltY_ += (ty - tiltY_) * TILT_EASE;
  const bool flat = tiltX_ * tiltX_ + tiltY_ * tiltY_ < FLAT_G * FLAT_G;
  jar_.tiltX = flat ? 0 : tiltX_; jar_.tiltY = flat ? 0 : tiltY_;
}

// A touch that begins on a button belongs to it until lifted; any other is a fingertip in the jar.
void Game::touch() {
  const bool home = in_.tapInCircle(HOME_CX, HOME_CY, HOME_HIT_R), mode = in_.tapIn(MODE_BOX.x, MODE_BOX.y, MODE_BOX.w, MODE_BOX.h);
  if (in_.pressed) buttonTouch_ = overHome(in_.x, in_.y) || overMode(in_.x, in_.y);
  if (buttonTouch_ && home) wantsHome_ = true;
  if (buttonTouch_ && mode) pour((Mode)((jar_.mode + 1) % MODE_COUNT));
  Finger& f = jar_.finger;
  const bool on = in_.down && !buttonTouch_;
  const float x = in_.x + 0.5f, y = in_.y + 0.5f;
  if (on && f.on) { moveX_ += x - f.x; moveY_ += y - f.y; }
  f.on = on; f.x = x; f.y = y;
}

// Physics at STEP_MS a step, whatever the frame rate; the finger's travel is spread over this frame's steps.
void Game::tick() {
  acc_ += ms_ - lastMs_; lastMs_ = ms_;
  int steps = (int)(acc_ / STEP_MS);
  if (steps > MAX_STEPS) { steps = MAX_STEPS; acc_ = 0; } else acc_ -= (uint32_t)steps * STEP_MS;
  if (!steps) return;
  jar_.finger.vx = 0.5f * moveX_ / steps; jar_.finger.vy = 0.5f * moveY_ / steps;   // the prototype's half-a-move
  moveX_ = moveY_ = 0;
  for (int s = 0; s < steps; s++) {
    jar_.step();
    if (jar_.mode == MODE_ORBITS) glowStep();
  }
}

// Orbits' trails: every body leaves light in its cell and a little in the four around it; all of it fades a tenth a
// step. The prototype's 7x7 px gaussian peaks at ~16 along a body's path; a cell gets TRAIL_MID of a splat to match.
// The fade rounds up or down by an ordered dither: always rounding down would put out a faint trail within a few
// steps, where the prototype's float light lasts ~30. The fade visits every cell of the glass (nothing outside it is
// ever lit), so it runs every other step, by 0.9^2: a frame is two steps, and the panel shows the same light.
void Game::glowStep() {
  if (!(jar_.frame & 1)) {
    const uint32_t f = jar_.frame * 151;
    for (uint8_t* g : {jar_.sky.glow[0], jar_.sky.glow[1], jar_.sky.glow[2]})
      for (int y = 0; y < G; y++)
        for (int k = y * G + span_[y][0], end = y * G + span_[y][1]; k < end; k++)
          g[k] = (uint8_t)((g[k] * 207 + ((k * 73 + f) & 255)) >> 8);
  }
  for (int i = 0; i < jar_.n; i++) {
    const Body& b = jar_.sky.body[i];
    const int c = bodyCell(b);   // bodies stay 8 px inside the glass: their own cell is in it
    const float v2 = b.vx * b.vx + b.vy * b.vy, s = v2 * invSqrt(v2) * 0.2f, sp = s < 1.6f ? s : 1.6f;   // fminf is a call
    const float k = 0.05f * b.size * (0.7f + sp);
    const Rgb& p = STARS[b.shade];
    const float v[3] = {p.r * k, p.g * k, p.b * k};
    for (int ch = 0; ch < 3; ch++) {
      uint8_t* l = jar_.sky.glow[ch];
      const int mid = (int)(v[ch] * TRAIL_MID + 0.5f), side = (int)(v[ch] * TRAIL_SIDE + 0.5f);
      l[c] = (uint8_t)(l[c] + mid > 255 ? 255 : l[c] + mid);
      for (int off : {1, -1, G, -G}) {   // a side cell may be past the rim: only a full draw shows it, and would keep it lit
        if (jar_.occ[c + off] != WALL) l[c + off] = (uint8_t)(l[c + off] + side > 255 ? 255 : l[c + off] + side);
      }
    }
  }
}

// ---------------------------------------------------------------- rendering, at cell resolution
Game::Seen& Game::seenFor(const uint16_t* target) {
  for (Seen& s : seen_) if (s.target == target) return s;
  Seen& s = seen_[nextSeen_];   // a buffer not seen before: drawn in full
  nextSeen_ ^= 1;
  s.target = target; s.valid = false; s.buttons = -1;
  return s;
}

void Game::render() {
  Seen& s = seenFor(gfx565::fb);
  upStep_ = DY[jar_.upDir] * G + DX[jar_.upDir];
  under_ = !s.valid;
  drawCells(s);
  if (jar_.mode == MODE_ORBITS) drawCores(s);
  drawButtons(s);
  s.valid = true;
}

// Every cell: the background, a grain, or Orbits' sky; the glass's reflection over it; written only if it changed.
// Outside the glass nothing changes: only a full draw visits it.
void Game::drawCells(Seen& s) {
  const bool full = !s.valid, orbits = jar_.mode == MODE_ORBITS;
  int k = 0;
  for (int y = 0; y < G; y++) {
    const int dy = 2 * y - (G - 1), dy2 = dy * dy;
    const int x1 = full ? G : span_[y][1];
    for (int x = full ? 0 : span_[y][0], c = y * G + x; x < x1; x++, c++) {
      const int dx = 2 * x - (G - 1), r = (dx * dx + dy2) >> 5, ring = r < BG_N ? r : BG_N - 1;
      const uint16_t bg = bg_[noise_[(y & 15) * 16 + (x & 15)]][ring];
      const int o = jar_.occ[c];
      uint16_t col = orbits ? sky(c, bg) : o > 0 ? grain(o - 1, c, ring) : bg;
      while (k < shines_ && shine_[k].cell < c) k++;
      if (k < shines_ && shine_[k].cell == c) { const int a = shine_[k++].add; col = brighten(col, a, a, a); }
      if (put(s.cells, full, c, col) && underButtons(c)) under_ = true;
    }
  }
}

// Sand: its own shade and tone, lighter on the surface and while moving (and a glint now and then), darker when wet.
uint16_t Game::grain(int i, int c, int ring) const {
  const uint8_t look = jar_.g.look[i];
  if (kindOf(look) != SAND) return water(i, c, ring);
  const int vx = jar_.g.vx[i], vy = jar_.g.vy[i];
  const float v2 = (float)(vx * vx + vy * vy), sp = vx | vy ? v2 * invSqrt(v2) * (1 / (256 * F)) : 0;   // most sand is still
  if (sp > 0.5f && (i * 7 + jar_.frame) % 19 == 0) return rgb(0xfff6dc);
  const Rgb* palette = jar_.mode == MODE_RAINBOW ? STRATA : SANDS;
  const Rgb& p = palette[shadeOf(look)];
  const int up = jar_.occ[c + upStep_];
  float m = 0.89f + toneOf(look) * (0.22f / 7);
  if (up == EMPTY) m *= 1.2f;
  else if (up > 0 && kindOf(jar_.g.look[up - 1]) != SAND) m *= 0.68f;   // wet sand under water (Beach)
  m = (m + (sp < 2.5f ? sp * 0.12f : 0.3f)) * vignette_[ring];
  return pack(p.r * m, p.g * m, p.b * m);
}

// Water: foam on top, lighter near the surface and deep blue below, caustic glints, and white bubbles.
uint16_t Game::water(int i, int c, int ring) const {
  int d = 0;
  for (int m = c; d < 7; d++) { m += upStep_; if (jar_.occ[m] <= 0) break; }
  Rgb w = WATER_AT[d];
  const int cx = c % G, cy = c / G, f = (int)jar_.frame;
  if (d == 0) w = mix(w, FOAM, 0.65f + 0.35f * sine(cx * 9 + f * 5));
  const float s = d ? sine(cx * 5 + f * 3) + sine(cy * 4 - f * 2 + (cx >> 1)) : 0;
  const float glint = s > 1.25f ? (s - 1.25f) * 70 : 0;
  const int vx = jar_.g.vx[i], vy = jar_.g.vy[i];
  const float v2 = (float)(vx * vx + vy * vy), sp = vx | vy ? v2 * invSqrt(v2) * (22 / (256 * F)) : 0, lift = sp < 40 ? sp : 40;
  w = {w.r + glint + lift, w.g + glint + lift, w.b + glint * 0.8f + lift};
  if (kindOf(jar_.g.look[i]) == BUBBLE) w = {w.r * 0.35f + 180, w.g * 0.35f + 235, w.b * 0.35f + 245};
  const float v = vignette_[ring];
  return pack(w.r * v, w.g * v, w.b * v);
}

uint16_t Game::sky(int c, uint16_t bg) const {
  const uint8_t r = jar_.sky.glow[0][c], g = jar_.sky.glow[1][c], b = jar_.sky.glow[2][c];
  return r | g | b ? brighten(bg, r, g, b) : bg;
}

// Orbits: each body's own cell, bright over its glow (brighter and bigger bodies brighter).
void Game::drawCores(Seen& s) {
  for (int i = 0; i < jar_.n; i++) {
    const Body& b = jar_.sky.body[i];
    const int c = bodyCell(b);
    const Rgb& p = STARS[b.shade];
    const float k = b.size * 0.6f * CORE, h = 36 * b.size * CORE;
    if (put(s.cells, false, c, lighten(s.cells[c], p.r * k + h, p.g * k + h, p.b * k + h)) && underButtons(c)) under_ = true;
  }
}

// The buttons, on top of the jar, redrawn only when this buffer's copy is out of date: a cell under them changed, or
// they did (pressed, the mode). The cells under them are put back first: this buffer's copy of them is two frames old.
void Game::drawButtons(Seen& s) {
  const bool homeDown = in_.down && buttonTouch_ && overHome(in_.x, in_.y), modeDown = in_.down && buttonTouch_ && overMode(in_.x, in_.y);
  const int state = jar_.mode | homeDown << 2 | modeDown << 3;
  if (!under_ && s.buttons == state) return;
  s.buttons = state;
  for (const ui::Box& b : {HOME_CELLS, MODE_CELLS})
    for (int y = b.y; y < b.y + b.h; y++) for (int x = b.x; x < b.x + b.w; x++) block(y * G + x, s.cells[y * G + x]);
  drawHome(homeDown);
  drawModes(modeDown);
}
// ui::drawBack at 3x (as Biscuit draws it), sunk while pressed.
void Game::drawHome(bool homeDown) const {
  const int dy = homeDown ? 3 : 0;
  const uint16_t orange = rgb(PALETTE_RGB[C_ORANGE]);
  gfx565::circle(HOME_CX * 3 + 1, HOME_CY * 3 + 1, 34, rgb(PALETTE_RGB[C_DKBROWN]));
  gfx565::circle(HOME_CX * 3 + 1, HOME_CY * 3 - 2 + dy, 31, homeDown ? rgb(0xdf8e00) : orange);
  const gfx::Sprite& gl = ui::HOME_GLYPH;
  for (int y = 0; y < gl.h; y++) for (int x = 0; x < gl.w; x++)
    if (gl.px[y * gl.w + x] != C_T) gfx565::rect((HOME_CX - 4 + x) * 3, (HOME_CY - 5 + y) * 3 + dy, 3, 3, rgb(PALETTE_RGB[C_WHITE]));
}
// A dark pill with a dot per mode, the current one big and ringed.
void Game::drawModes(bool modeDown) const {
  const gfx565::Rect pill = {MODE_BOX.x * 3, MODE_BOX.y * 3, MODE_BOX.w * 3, MODE_BOX.h * 3};
  gfx565::roundRect(pill, 33, rgb(0x8a7fa6));
  gfx565::roundRect({pill.x + 3, pill.y + 3, pill.w - 6, pill.h - 6}, 30, modeDown ? rgb(0x3a3350) : rgb(0x1c1826));
  for (int m = 0; m < MODE_COUNT; m++) {
    const int cx = 190 + m * 33, cy = pill.y + pill.h / 2, r = m == jar_.mode ? 11 : 6;
    if (m == jar_.mode) gfx565::circle(cx, cy, r + 3, rgb(0xfff7e6));
    const Dot& d = DOTS[m];
    for (int yy = -r; yy <= r; yy++) {   // the disc in horizontal bands
      int w = 0; while ((w + 1) * (w + 1) + yy * yy <= r * r + r) w++;
      gfx565::rect(cx - w, cy + yy, 2 * w + 1, 1, rgb(d.band[(yy + r) * d.bands / (2 * r + 1)]));
    }
  }
}

// One name, but in a table all the same: tools/playtest.py learns every game's screen names from these N[] tables.
const char* Game::screenName() const {
  static const char* N[] = {"sandjar_jar"};
  return N[0];
}
void Game::debugPrint() {
  int kinds[3] = {};
  if (jar_.mode != MODE_ORBITS) for (int i = 0; i < jar_.n; i++) kinds[kindOf(jar_.g.look[i])]++;
  printf("[sandjar mode=%s grains=%d sand=%d water=%d bubbles=%d tilt=%d,%d finger=%d shaking=%d steps=%u]\n", MODES[jar_.mode],
         jar_.n, kinds[SAND], kinds[WATER], kinds[BUBBLE], (int)lroundf(jar_.tiltX * 1000), (int)lroundf(jar_.tiltY * 1000),
         jar_.finger.on, jar_.shaking > 0.01f, (unsigned)jar_.frame);
  printf("screen=%s\n", screenName());
}
void Game::debugCmd(const char* cmd) {
  for (int m = 0; m < MODE_COUNT; m++) if (!strcmp(cmd, MODES[m])) pour((Mode)m);
}
}  // namespace sandjar
