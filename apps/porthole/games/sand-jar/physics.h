// Sand Jar's physics: sand, water and bubbles as grains on a 160x160 cell grid inside the round glass (a cell is 3
// physical px, one logical px, so a touch is a cell as is), and Orbits' free bodies. No drawing here: game.cpp renders
// what the Jar holds. Deterministic (its own xorshift RNG) and allocation-free: every buffer is sized at compile time
// and lives in the Jar, which the game allocates once. Ported from the approved browser prototype; the rules it keeps
// are listed in games/sand-jar/CLAUDE.md.
#pragma once
#include <stdint.h>

namespace sandjar {
constexpr int G = 160;   // cells per side
enum Kind : uint8_t { SAND, WATER, BUBBLE };
enum Mode : uint8_t { MODE_SAND, MODE_RAINBOW, MODE_BEACH, MODE_ORBITS, MODE_COUNT };
enum Cell : int16_t { EMPTY = 0, WALL = -1, FINGER = -2 };   // occ[]: else a grain's index + 1

// ---- tuning knobs: the board's frame budget decides the counts (FILL_PCT), the rest is how the sand feels
constexpr uint8_t FILL_PCT[MODE_COUNT] = {34, 42, 46, 3};   // grains per 100 cells of glass; Orbits: bodies
constexpr int MAX_GRAINS = 9000, MAX_BODIES = 600;          // hold 46% and 3% of the 19,3xx cells inside the glass
constexpr float FINGER_R = 14;       // cells: 42 physical px, a ~9 mm fingertip
constexpr float WAKE = 14.0f / 3;    // cells past the finger's rim that its motion drags along
constexpr float SAND_SLIDE = 0.55f;  // chance a stopped grain rolls down a diagonal: the angle of repose
constexpr float SAND_FRIC = 0.5f;    // share of its speed a grain keeps along a surface it hits
constexpr float ROLL_DROP = 0.5f;    // cells a roll over a neighbor must end lower: sets the steepest resting slope (~40 deg)
constexpr float SHAKE = 2.6f, SHAKE_FADE = 0.86f;   // a shake's random kick, and how much of it is left a step later
constexpr float BEACH_SAND = 0.45f;  // Beach: the share that is sand, the rest water
constexpr float ORB_SUN = 700, ORB_STAR = 3800;     // Orbits: the sun's and the finger's pull (panel px^3/step^2)

struct Finger { bool on; float x, y, vx, vy; };             // cells; v: cells a step
struct Body { float x, y, vx, vy, size; uint8_t shade; };   // Orbits: panel px; size also sets brightness

// A grain's look byte: kind (bits 6-7), palette shade (bits 0-2), tone (bits 3-5: its own brightness, 0..7).
inline Kind kindOf(uint8_t look) { return (Kind)(look >> 6); }
inline int shadeOf(uint8_t look) { return look & 7; }
inline int toneOf(uint8_t look) { return (look >> 3) & 7; }
inline uint8_t makeLook(Kind k, int shade, int tone) { return (uint8_t)(k << 6 | (tone & 7) << 3 | (shade & 7)); }

struct Jar {
  void reset(Mode m, uint32_t seed);   // the mode's grains (or bodies), heaped on the bottom edge of the current tilt
  void step();                         // one tick (the prototype's 60 Hz frame)
  void shake() { shaking = SHAKE; }
  static float pos(uint16_t v) { return v * (1.0f / 256); }   // grain coordinates are 8.8 fixed-point cells
  static float vel(int16_t v) { return v * (1.0f / 256); }

  // inputs, set before step()
  float tiltX = 0, tiltY = 1;   // in-plane gravity in g (magnitude 0..1): where things fall, and how hard
  Finger finger{};
  // state
  Mode mode = MODE_SAND;
  int n = 0;                    // grains, or Orbits' bodies
  int sand = 0;                 // grains [0, sand) are sand, the rest (Beach) water or bubbles: none ever turns into the other
  int inside = 0;               // cells inside the glass
  uint32_t frame = 0;           // steps since reset
  uint32_t rng = 1;
  float shaking = 0;
  float gxn = 0, gyn = 1;       // gravity's direction, kept while the jar lies flat
  uint8_t upDir = 6;            // the grid direction (0 = +x, clockwise on screen) closest to straight up
  int16_t occ[G * G];
  // The grains, or (Orbits has none) the bodies and their trail light. The two share memory: the board's internal RAM
  // holds the whole Jar (random access in PSRAM is 14x slower), and it is tight.
  struct Grains {
    uint16_t x[MAX_GRAINS], y[MAX_GRAINS];
    int16_t vx[MAX_GRAINS], vy[MAX_GRAINS];
    uint8_t look[MAX_GRAINS];
    uint16_t order[MAX_GRAINS];   // scratch: this step's grains, from the top of the pile down to the bottom
  };
  struct Sky {
    Body body[MAX_BODIES];
    uint8_t glow[3][G * G];       // the renderer's trail light (R, G, B per cell); physics never touches it
  };
  union { Grains g; Sky sky; };
  // scratch, kept here so a step never allocates
  static constexpr int KEYS = 4 * G + 4, FINGER_MAX = 720;
  uint16_t count[KEYS + 1];
  uint16_t fingerCells[FINGER_MAX]; int fingerCount = 0;
};

// 1 / sqrt(x) for x > 0 with no library call: the board's sqrtf and divide are each a ~100-cycle call, too slow per
// body, grain or cell. The classic bit trick and two Newton steps: within 5e-6 of the true value. At 0 it is finite,
// so v2 * invSqrt(v2) (a length) is 0 there too.
inline float invSqrt(float x) {
  union { float f; uint32_t i; } u = {x};   // GCC and clang define reading the other member; the board's memcpy is a call
  u.i = 0x5f3759df - (u.i >> 1);
  float y = u.f;
  y *= 1.5f - 0.5f * x * y * y;
  return y * (1.5f - 0.5f * x * y * y);
}

// The eight grid neighbors, clockwise on screen from +x.
constexpr int DX[8] = {1, 1, 0, -1, -1, -1, 0, 1}, DY[8] = {0, 1, 1, 1, 0, -1, -1, -1};
}  // namespace sandjar
