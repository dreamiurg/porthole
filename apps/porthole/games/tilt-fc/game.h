// Tilt FC: top-down street football on the round glass. Tilt steers the kid's player (direction only, never speed); a
// tap anywhere passes or shoots with the ball and slides for it without. Two a side plus keepers, first to three or
// three minutes. Nothing punishes: a lost match just shows the score, and the next one is gentler.
// Runs as an App in the Porthole shell, all on the RGB565 surface.
// Rules: match.h, tune.h; save: save.h; look: render.h. Design: docs/superpowers/specs/2026-09-27-tilt-fc-design.md.
#pragma once
#include <stdint.h>
#include "app.h"
#include "canvas.h"
#include "match.h"
#include "render.h"
#include "save.h"
#include "tilt.h"
#include "ui.h"

namespace fc {
class Game : public App {
 public:
  const char* name() const override { return "Tilt FC"; }
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
  // "kickoff": calibrate as held now and start a match (seeded by its number alone); "score<T>-<C>": set the score (T,
  // C 0-2); "clock<S>": S seconds of the clock left (negative: that far into the extra time).
  void debugCmd(const char* cmd) override;
  const Match& match() const { return match_; }   // for host/test_tiltfc.cpp

  enum Screen : uint8_t { SC_CALIBRATE, SC_KICKOFF, SC_MATCH, SC_GOAL, SC_FULLTIME, SC_COUNT };

 private:
  Save save_{};
  bool dirty_ = false, wantsHome_ = false;
  uint32_t now_ = 0, ms_ = 0, pageMs_ = 0;   // pageMs_: when this page appeared
  uint32_t pressMs_ = 0;          // when the finger last went down (Input::heldMs is 0 by the release)
  Input in_{};
  ui::FreshGate gate_;            // stepping onto the court or off it ignores touches for a moment (os/ui.h)
  ui::Hold hold_;                 // the leave sign held during play
  Screen screen_ = SC_CALIBRATE;
  tilt::Grav neutral_ = {0, 0, -1000};   // gravity as the kid held the device when the match started
  Vec tilt_{};                    // this frame's tilt away from it (milli-g)
  // Calibrate: the neutral is the steady average of the last frames; the arrow shows the tilt away from `ref_`, which
  // follows gravity over about a second, so holding still settles it.
  tilt::Steady steady_;
  tilt::Follow ref_;
  bool starting_ = false;         // the go sign was pressed, the match starts once held steady
  Match match_{};
  uint32_t stepMs_ = 0;           // time not yet played, under one STEP_MS
  bool tapPending_ = false;       // a press not yet played (a frame too short for a step)
  uint32_t seed_ = 0;             // the visit's: when it began
  uint16_t matchNo_ = 0;          // matches started this visit: with seed_, the seed of the next one
  // What each of the panel's two buffers holds (os/canvas.h): this page, its movers (players, ball, the aim spot, the
  // clock; the Calibrate arrow) as boxes with keys, and the rest of its state as `look` (the score, the countdown, a
  // pressed sign, the hold).
  static constexpr int MAX_MOVERS = PLAYERS + 3;
  using Mover = canvas::Mover;
  canvas::Frames<MAX_MOVERS> frames_;

  void go(Screen s);
  void newMatch();
  void decide();                  // the match is decided: the save gets a win, the next level
  bool pressing(int cx, int cy, int r) const { return ui::pressing(in_, cx, cy, r); }
  bool released(int cx, int cy, int r) const;   // a tap, or a long press let go, on this logical circle
  bool onSign(int x, int y) const;
  bool leaving();
  bool goPressed() const;
  bool ftReady() const;           // Full time: shown long enough for the go sign to play on
  bool ftPress() const;           // Full time: the finger went down once the go sign was lit
  float hold() const;             // how far holding the leave sign has got, 0..1 (Kickoff, Match, Goal)
  bool inPlay() const { return screen_ == SC_KICKOFF || screen_ == SC_MATCH || screen_ == SC_GOAL; }
  tilt::Vec calTilt() const;      // Calibrate: the tilt away from the slow reference
  bool ready() const;             // Calibrate: held still
  Kick kidAim() const;            // what a tap would do now (for the aim spot and the pass target)
  int clockSteps() const;         // the clock dial's steps filled (its mover key, and what it draws)
  int countdown() const;          // Kickoff: 3, 2, 1
  int movers(Mover* out) const;
  uint32_t look() const;
  void updateCalibrate(uint32_t dt);
  void updateKickoff();
  void updateMatch(uint32_t dt);
  void updateGoal();
  void updateFullTime();
  void draw();
  void drawCalibrate();
  void drawPitch();               // the court, the players, the ball, the scoreboard, the leave sign
  void drawKickoff();
  void drawGoal();
  void drawFullTime();
};
}  // namespace fc
