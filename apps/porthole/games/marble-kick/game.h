// Marble Kick: a wooden labyrinth toy seen from above, a felt pitch inside. Tilt the device and the ball rolls; roll it
// past the pegs into the goal cut into the rim. Tilt is the only control: during play a brush of the glass does
// nothing, and leaving takes holding the home knob. Nothing punishes: no timer, no lives; a hole only sends the ball
// back to the start; a level is only not finished yet. Runs as an App in the Porthole shell, all on the RGB565 surface.
// Rules: physics.h, levels.h, tune.h; save: save.h; look: render.h.
// Design: docs/superpowers/specs/2026-09-27-marble-kick-design.md.
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
  const Ball& ball() const { return ball_; }   // for host/test_marble.cpp's render check

  enum Screen : uint8_t { SC_CALIBRATE, SC_PLAY, SC_GOAL, SC_DONE, SC_COUNT };

 private:
  Save save_{};                   // the level reached
  bool dirty_ = false, wantsHome_ = false;
  uint32_t now_ = 0, ms_ = 0, pageMs_ = 0;   // pageMs_: when this page appeared (the goal, the celebration)
  Input in_{};
  ui::FreshGate gate_;            // every page change ignores touches for a moment (os/ui.h)
  Screen screen_ = SC_CALIBRATE;
  int level_ = 0;                 // the level on the tray (index into LEVELS)
  Grav neutral_ = {0, 0, -1000};  // gravity as the kid held the device when play started
  Ball ball_{};
  uint32_t stepMs_ = 0;           // time not yet simulated, under one STEP_MS
  // Calibrate: the last frames' gravity (the neutral is their steady average), and the dish: a small ball rolling on
  // the tilt away from `ref_`, which follows gravity over about a second, so holding still settles it in the middle.
  tilt::Steady steady_;
  float ref_[3] = {0, 0, -1000};
  bool refSet_ = false, starting_ = false;   // starting_: the button was pressed, play starts once held steady
  Vec dish_{}, dishV_{};
  uint32_t dishMs_ = 0;           // the dish's time not yet simulated, under one STEP_MS
  // What each of the panel's two buffers holds (os/app.h): this page, its movers (the ball, the moving pegs, a moving
  // goal; the dish's ball) as boxes with keys, and the rest of its state as `look` (pressed buttons, the hold ring, the
  // dish's glow, stars picked up, the confetti's frame). A buffer not listed (fb null after a page change) or with
  // another look gets the whole page; one listed gets each changed mover's old and new boxes, and nothing when none
  // changed.
  static constexpr int MAX_MOVERS = 2 + MAX_PEGS;
  struct Mover { paint::Box box; int key; };
  struct Held { const uint16_t* fb; Mover movers[MAX_MOVERS]; int n; uint32_t look; };
  Held held_[2] = {};

  void go(Screen s);
  void play(int level);
  bool pressing(int cx, int cy, int r) const;   // the finger went down in this logical circle and is still in it
  bool pressingBox(int cx, int cy, int half) const;
  bool goPressed() const;
  bool leaving();
  float hold() const;             // Play: how far holding the knob has got to leaving, 0..1
  int pressedCoin() const;        // Done: the coin under the finger, or -1
  bool ready() const;             // Calibrate: the dish's ball has settled in the middle
  Mover ballMover() const;
  int movers(Mover* out) const;
  uint32_t look() const;
  void draw();
  void updateCalibrate(uint32_t dt); void drawCalibrate();
  void rollDish(uint32_t dt);
  void updatePlay(uint32_t dt); void drawPlay();
  void updateGoal(); void drawGoal();
  void updateDone(); void drawDone();
  void drawTray(bool things);     // the tray, the level's things (or none), the goalposts, the ball, the knob
  void drawShadows(bool things, const paint::Mouth& m) const;
  void drawThings() const;
  void drawBall() const;
  void drawFlags() const;
};
}  // namespace marble
