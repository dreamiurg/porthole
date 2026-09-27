// Converts raw touch samples into edges/taps/long-presses. Shared by firmware and host sim.
#pragma once
#include <stdint.h>

// Audit registry: every hit test the game performs is recorded so a checker can measure tap
// targets (size, spacing, overlap, distance from the round edge) without knowing the screens.
struct UiAudit {
  struct Rect { int16_t x, y, w, h; };
  static constexpr int MAX = 96;
  static inline Rect regions[MAX]; static inline int count = 0; static inline bool enabled = false;
  static void add(int x, int y, int w, int h) {
    if (!enabled || count >= MAX) return;
    for (int i = 0; i < count; i++) if (regions[i].x == x && regions[i].y == y && regions[i].w == w && regions[i].h == h) return;
    regions[count++] = {(int16_t)x, (int16_t)y, (int16_t)w, (int16_t)h};
  }
  static void reset() { count = 0; }
};

struct Input {
  bool down = false;        // finger currently on screen
  bool pressed = false;     // went down this frame
  bool released = false;    // lifted this frame
  bool tap = false;         // short press+release with little movement (fires on release)
  bool longPress = false;   // fires once when held >= 600ms
  int x = 0, y = 0;         // current (or last) logical position
  int downX = 0, downY = 0; // where the press started
  int px = 0, py = 0;       // previous frame position (for drags)
  uint32_t heldMs = 0;
  // Gravity from the motion sensor (where things fall), milli-g, in screen terms: +x toward the right edge, +y toward
  // the bottom edge, +z out of the glass toward the viewer. Held upright: (0, 1000, 0); lying face up: (0, 0, -1000).
  // The host fills it every frame (firmware: board::readAccel; sim: its `tilt` command); InputTracker leaves it alone.
  int16_t gx = 0, gy = 0, gz = -1000;
  bool hit(int rx, int ry, int rw, int rh) const { UiAudit::add(rx, ry, rw, rh); return x >= rx && y >= ry && x < rx + rw && y < ry + rh; }
  bool tapIn(int rx, int ry, int rw, int rh) const { bool h = hit(rx, ry, rw, rh); return tap && h; }
  bool tapInCircle(int cx, int cy, int r) const { UiAudit::add(cx - r, cy - r, 2 * r, 2 * r); int dx = x - cx, dy = y - cy; return tap && dx * dx + dy * dy <= r * r; }
};

// Activity for the idle rules (the shell's play time, the firmware's dimming): a touch, or the board moved. A tilt game
// is played hands-off, so touch alone would call it idle. Moved = gravity turned more than MOVE_MG away from where it
// was at the last activity: sensor noise at rest is 10-20 mg and a hand holding still wobbles a few tens, while 200 mg
// is about a 12 degree tilt or any shake. The reference only moves on activity, so a board on a table never counts,
// and neither does drift far below 200 mg; a slow tilt counts once it adds up.
class ActivityTracker {
 public:
  static constexpr int MOVE_MG = 200;
  void step(const Input& in, bool touching, uint32_t ms) {   // touching: the raw finger (the firmware swallows some)
    int dx = in.gx - ref_[0], dy = in.gy - ref_[1], dz = in.gz - ref_[2];
    if (!touching && dx * dx + dy * dy + dz * dz <= MOVE_MG * MOVE_MG) return;
    ref_[0] = in.gx; ref_[1] = in.gy; ref_[2] = in.gz; lastMs_ = ms;
  }
  uint32_t idleMs(uint32_t ms) const { return ms - lastMs_; }
 private:
  int16_t ref_[3] = {0, 0, -1000};
  uint32_t lastMs_ = 0;
};

class InputTracker {
 public:
  Input step(bool rawDown, int rx, int ry, uint32_t ms) {
    Input in = last_;
    in.pressed = in.released = in.tap = in.longPress = false;
    in.px = in.x; in.py = in.y;
    if (rawDown) { in.x = rx; in.y = ry; }
    if (rawDown && !in.down) { in.pressed = true; in.downX = rx; in.downY = ry; downMs_ = ms; longFired_ = false; moved_ = false; }
    if (rawDown) {
      in.heldMs = ms - downMs_;
      int dx = in.x - in.downX, dy = in.y - in.downY;
      if (dx * dx + dy * dy > 8 * 8) moved_ = true;
      if (!longFired_ && in.heldMs >= 600 && !moved_) { in.longPress = true; longFired_ = true; }
    }
    if (!rawDown && in.down) {
      in.released = true;
      if (!moved_ && !longFired_ && in.heldMs < 600) in.tap = true;
      in.heldMs = 0;
    }
    in.down = rawDown;
    last_ = in;
    return in;
  }
 private:
  Input last_;
  uint32_t downMs_ = 0;
  bool longFired_ = false, moved_ = false;
};
