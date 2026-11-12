// Shared widgets: the look every Porthole screen shares (shell and games). Stateless: each call draws and/or
// hit-tests against the Input it is given, so the UI audit sees every region through Input::hit/tapIn.
#pragma once
#include <stdint.h>
#include "gfx.h"
#include "input.h"

namespace ui {
struct Box { int x, y, w, h; };
struct Button { Box box; const char* label; const gfx::Sprite* icon; uint8_t col; };

uint8_t inkOn(uint8_t fill);   // readable text color for a fill: dark on bright, white on dark
void panel(const Box& b, uint8_t fill, uint8_t border);
void drawButton(const Button& b, bool pressed);
bool button(const Input& in, const Button& b);   // draws it (sunk while a press that began on it is on it); true on a tap
// Round button of radius 11 with a one-color icon mask drawn in white; the hit circle is 4 px wider.
bool iconButton(const Input& in, int cx, int cy, const gfx::Sprite& icon, uint8_t col);
void toast(const char* text);   // a one-line note in a white panel across the middle of the glass
// The orange home button at the top of the glass: every screen's way back. HOME_GLYPH is its house (a mask), for a
// game that draws the same button on another surface.
extern const gfx::Sprite HOME_GLYPH;
void drawBack();
bool back(const Input& in);

// Name keyboard: a field on top, then two pages of letters (A-M, N-Z) as a 4x4 grid of 24x22 keys with the page
// switch, backspace and OK on the last row. `page` is the caller's (0 or 1; reset it with the name). Names are up to
// 8 letters, first one capital.
constexpr int NAME_LEN = 8;
bool keyboard(const Input& in, char* buf, int& len, uint8_t& page);   // true when OK is tapped with at least one letter
void drawKeyboard(const Input& in, const char* buf, uint8_t page, const char* hint, uint32_t ms);   // hint shows while empty

// A press on a logical circle: the finger went down in it and is still in it.
bool pressing(const Input& in, int cx, int cy, int r);
bool pressing(const Input& in, const Box& b);   // the same for a box; logs b for the UI audit every frame
// The release counterpart: the finger went down in b and came up in it this frame, however long it was held, and
// however far it wandered inside b (Input::tap stops at an 8 px slide). Logs b for the UI audit.
bool clicked(const Input& in, const Box& b);
bool clicked(const Input& in, int cx, int cy, int r);   // the same for a circle (ui::back's, the paging arrows)
// During play a hand holding the case may brush the glass, so a game's way out there takes a hold, not a tap: HOLD_MS
// with the finger on the sign. The time counts from when the finger is on it (step's `on`: pressing, every frame), so
// one that drifts off and back starts over, and it runs on across page changes that do not hide the finger.
constexpr uint32_t HOLD_MS = 600;
class Hold {
 public:
  void step(bool on, uint32_t ms) { if (on && !on_) fromMs_ = ms; on_ = on; ms_ = ms; }
  float progress(uint32_t holdMs = HOLD_MS) const {   // 0..1 over holdMs: a weighty choice may ask longer
    if (!on_) return 0;
    const uint32_t d = ms_ - fromMs_;
    return d >= holdMs ? 1 : (float)d / holdMs;
  }

 private:
  bool on_ = false;
  uint32_t fromMs_ = 0, ms_ = 0;
};

// A fresh screen ignores touches for a moment, so the press that changed it (still down after a long press, or the
// next of a kid's mash) never acts on whatever now sits under the finger. Call filter() on every frame's input and one
// of these on every change; a finger held across any of them stays invisible until lifted, and its lift re-arms nothing.
// changed(): a new screen. One a tap made (a finger down or lifting as it changes) remembers where that press went
//   down: a mash happens on one spot, while deliberate navigation goes to a new target (screen-gate playtest M2: a
//   quick reader's taps looked exactly like a mash to a gate that waited out every press). A press beginning within
//   SPOT_R of the spot (one target: a kid's repeated taps scatter 3-6 mm) waits until FRESH_MS after the change and
//   FRESH_MS after the lift of each press so ignored ("again!" taps: 300-400 ms presses 150-750 ms apart, kid
//   playtests 9 and 10), but one begun MAX_MS after the change acts, so a kid retrying about once a second never meets
//   a dead screen for long; a press anywhere else acts at once (a keypad's quick first key, too). r narrows the spot
//   where targets sit closer than SPOT_R (the grown-ups' keypad). A change no finger made (a timer, a game event) is
//   shown().
// shown(): a change in place (a page turn, a +/-, a new cue) or a timer's: any press waits `wait` (LIGHT_MS), never
//   re-armed, so quick repeat taps on one control still count.
// waited(): a press near the spot beginning now would act.
struct FreshGate {
  static constexpr uint32_t LIGHT_MS = 450, FRESH_MS = 800, MAX_MS = 2500;
  static constexpr int SPOT_R = 24;
  uint32_t fromMs = 0, changedMs = 0, waitMs = 0; bool closed = false, spot = false, mashed = false;
  int spotX = 0, spotY = 0, spotR = SPOT_R;
  void shown(uint32_t now, uint32_t wait = LIGHT_MS) { arm(now, wait); }
  void changed(uint32_t now, const Input& in, int r = SPOT_R) {
    if (!in.down && !in.released) { shown(now); return; }
    arm(now, FRESH_MS); spot = true; spotR = r;
    spotX = in.released ? in.downX : in.x; spotY = in.released ? in.downY : in.y;   // a lift may land a few px off
  }
  bool waited(uint32_t now) const { return now - fromMs >= waitMs || now - changedMs >= MAX_MS; }
  void filter(Input& in, uint32_t now);

 private:
  void arm(uint32_t now, uint32_t wait) { fromMs = changedMs = now; waitMs = wait; closed = true; spot = mashed = false; }
};
}  // namespace ui
