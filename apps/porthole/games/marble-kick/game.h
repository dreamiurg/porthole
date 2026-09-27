// Marble Kick: a wooden labyrinth toy seen from above, a felt pitch inside. Tilt the device and the ball rolls; roll it
// past the pegs into the goal cut into the rim. Tilt is the only control: taps do nothing but Back (and move on from
// the Calibrate, Goal and Done pages). Nothing punishes: no timer, no holes, no lives; a level is only not finished yet.
// Runs as an App in the Porthole shell, all on the RGB565 surface. Rules: physics.h, levels.h, tune.h; save: save.h;
// look: render.h. Design: docs/superpowers/specs/2026-09-27-marble-kick-design.md.
#pragma once
#include <stdint.h>
#include "app.h"
#include "physics.h"
#include "render.h"
#include "save.h"
#include "ui.h"

namespace marble {
class Game : public App {
 public:
  const char* name() const override { return "Marble Kick"; }
  const gfx::Sprite& icon() const override;
  const char* store() const override { return STORE; }
  void enter(const AppEnter& e) override;
  void update(uint32_t nowSec, uint32_t ms, const Input& in) override;
  void render() override;
  Surface surface() const override { return SURFACE_RGB565; }
  Tint tint() const override { return clockTint(now_); }   // the launcher's light: handing back never changes it
  bool asleep() const override { return false; }
  bool soundOn(uint32_t) override { return false; }        // no sound in slice 1
  bool takeSave(const void** data, size_t* len, bool allowed) override;
  bool wantsHome() override { bool w = wantsHome_; wantsHome_ = false; return w; }
  void leave() override {}
  const char* screenName() const override;
  void debugPrint() override;
  void debugCmd(const char* cmd) override;   // "level<N>": calibrate as held now and play level N (1-based)

  enum Screen : uint8_t { SC_CALIBRATE, SC_PLAY, SC_GOAL, SC_DONE, SC_COUNT };

 private:
  Save save_{}, out_{};           // the level reached, and the sealed copy takeSave hands out
  bool dirty_ = false, wantsHome_ = false;
  uint32_t now_ = 0, ms_ = 0, goalMs_ = 0;
  Input in_{};
  ui::FreshGate gate_;            // every page change ignores touches for a moment (os/ui.h)
  Screen screen_ = SC_CALIBRATE;
  int level_ = 0;                 // the level on the tray (index into LEVELS)
  int neutralX_ = 0, neutralY_ = 1000;   // gravity as the kid held the device on the Calibrate page
  Ball ball_{};
  uint32_t stepMs_ = 0;           // time not yet simulated, under one STEP_MS
  // What each of the panel's two buffers holds (os/app.h): this page, with its moving part drawn at `moving`. A
  // buffer not listed (fb null after a page change) gets the whole page; one listed gets only the moving part's old
  // and new boxes, and nothing at all when they are the same.
  struct Held { const uint16_t* fb; paint::Box moving; };
  Held held_[2] = {};

  void go(Screen s);
  void play(int level);
  paint::Box moving() const;      // the part of the page that moves: the ball, the level's bubble
  int bubble(int axis) const;     // the bubble's offset in the vial, from the gravity now
  void updateCalibrate(); void drawCalibrate();
  void updatePlay(uint32_t dt); void drawPlay();
  void updateGoal(); void drawGoal();
  void updateDone(); void drawDone();
  void drawTray(bool pegs);       // the tray, the level's pegs (or none), the goalposts, the knob
};
}  // namespace marble
