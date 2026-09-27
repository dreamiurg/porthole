// Sand Jar: a tilt toy. Hold the round board and tilt it: sand, water or stars move inside like a jar. A touch is a
// fingertip in the jar (Orbits: a heavy star everything orbits). One screen, four modes, a tap on the bottom button
// cycles them; nothing to win and nothing saved. The physics is physics.h; this is the App plumbing, the motion
// sensor, the touch, and the renderer (cell resolution, only the cells that changed reach the panel).
#pragma once
#include <stdint.h>
#include "app.h"
#include "physics.h"
#include "ui.h"

namespace sandjar {
// ---- tuning knobs
constexpr uint32_t STEP_MS = 16;     // physics ticks at the prototype's ~60 Hz...
constexpr int MAX_STEPS = 2;         // ...at most this many a frame: a slow frame slows the jar down, never piles up
constexpr float TILT_EASE = 0.35f;   // share of the way to a new sensor reading the tilt moves each frame (noise)
constexpr float FLAT_G = 0.05f;      // in-plane gravity under this is the jar lying flat: 0 g, nothing avalanches
constexpr int SHAKE_MG = 600;        // a reading this far off 1 g (milli-g) is a shake...
constexpr int JOLT_MG = 900;         // ...and so is a jump this big from one frame's reading to the next
constexpr float TRAIL_MID = 1.5f, TRAIL_SIDE = 0.25f;   // Orbits: light a body leaves in its cell and the four around
constexpr float CORE = 0.4f;         // Orbits: a body's own cell, as a share of the prototype's one-pixel core

class Game : public App {
 public:
  const char* name() const override { return "Sand Jar"; }
  const gfx::Sprite& icon() const override;
  const char* store() const override { return "sandjar"; }   // nothing is saved, but the namespace is taken
  void enter(const AppEnter& e) override;
  void update(uint32_t nowSec, uint32_t ms, const Input& in) override;
  void render() override;
  Surface surface() const override { return SURFACE_RGB565; }
  Tint tint() const override { return TINT_DAY; }
  bool asleep() const override { return false; }
  bool soundOn(uint32_t) override { return false; }   // a toy: no buzzer
  bool takeSave(const void**, size_t*, bool) override { return false; }
  bool wantsHome() override { bool w = wantsHome_; wantsHome_ = false; return w; }
  void leave() override { jar_.finger.on = false; }
  const char* screenName() const override;
  void debugPrint() override;
  void debugCmd(const char* cmd) override;   // "sand" "rainbow" "beach" "orbits": that mode, freshly poured

 private:
  // What one of the panel's buffers holds, per cell: render() writes a cell's 3x3 block only when it changed since it
  // last drew into that buffer (which holds the frame from two renders ago). Kept out of the internal RAM the jar
  // needs: allocated once, and only ever scanned in order.
  struct Seen { const uint16_t* target; uint16_t* cells; bool valid; int buttons; };   // buttons: the state they were drawn in
  static constexpr int BG_N = 802;   // background rings: (2dx)^2 + (2dy)^2 >> 5 from the middle, the last one black
  struct Shine { uint16_t cell; uint8_t add; };

  Jar jar_;                       // ~153 KB: the game object is static, so this stays in internal RAM
  Seen seen_[2] = {};
  int nextSeen_ = 0;
  bool started_ = false, wantsHome_ = false;
  bool buttonTouch_ = false;      // this touch began on a button: it is not a finger in the jar
  bool sensed_ = false;           // lastG_ holds a reading
  int16_t lastG_[3] = {};
  float tiltX_ = 0, tiltY_ = 1;   // the eased sensor reading
  float moveX_ = 0, moveY_ = 0;   // the finger's travel (cells) since the last physics step
  uint32_t ms_ = 0, lastMs_ = 0, acc_ = 0, pours_ = 0;
  Input in_{};
  ui::FreshGate gate_;
  // renderer tables, rebuilt when the mode changes
  uint16_t bg_[4][BG_N] = {};     // the mode's background per ring, in four dither levels
  float vignette_[BG_N] = {};     // grains darken toward the rim (0..1)
  uint8_t noise_[256] = {};       // dither level of each cell in a 16x16 tile
  Shine shine_[640] = {}; int shines_ = 0;   // the glass's reflection, in cell order
  int upStep_ = 0;                // cell offset of "up" this frame
  uint8_t span_[G][2] = {};       // each row's cells inside the glass: [first, last + 1)
  bool under_ = false;            // this render changed a cell under a button

  void pour(Mode m);
  void tables();
  void sense(const Input& in);
  void touch();
  void tick();
  void glowStep();
  Seen& seenFor(const uint16_t* target);
  uint16_t grain(int i, int c, int ring) const;
  uint16_t water(int i, int c, int ring) const;
  uint16_t sky(int c, uint16_t bg) const;
  void drawCells(Seen& s);
  void drawCores(Seen& s);
  void drawButtons(Seen& s);
  void drawHome(bool down) const;
  void drawModes(bool down) const;
};
}  // namespace sandjar
