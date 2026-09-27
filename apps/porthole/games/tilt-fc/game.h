// Tilt FC: top-down street football on the round glass. Tilt steers the kid's player (direction only, never speed); a
// tap anywhere passes or shoots with the ball and slides for it without. Two a side plus keepers, first to three or
// three minutes. Nothing punishes: a lost match just shows the score, and the next one is gentler.
// Runs as an App in the Porthole shell, all on the RGB565 surface.
// Rules: match.h, tune.h; save: save.h; look: render.h. Design: docs/superpowers/specs/2026-09-27-tilt-fc-design.md.
#pragma once
#include <stdint.h>
#include "app.h"
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
  // "kickoff": calibrate as held now and start a match; "score<T>-<C>": set the score (T, C 0-2); "clock<S>": S seconds
  // of the clock left.
  void debugCmd(const char* cmd) override;
  const Match& match() const { return match_; }   // for host/test_tiltfc.cpp

  enum Screen : uint8_t { SC_CALIBRATE, SC_KICKOFF, SC_MATCH, SC_GOAL, SC_FULLTIME, SC_COUNT };

 private:
  Save save_{};
  bool dirty_ = false, wantsHome_ = false;
  uint32_t now_ = 0, ms_ = 0, pageMs_ = 0;   // pageMs_: when this page appeared
  Input in_{};
  ui::FreshGate gate_;            // every page change ignores touches for a moment (os/ui.h)
  Screen screen_ = SC_CALIBRATE;
  tilt::Grav neutral_ = {0, 0, -1000};   // gravity as the kid held the device when the match started
  Vec tilt_{};                    // this frame's tilt away from it (milli-g)
  // Calibrate: the neutral is the steady average of the last frames; the arrow shows the tilt away from `ref_`, which
  // follows gravity over about a second, so holding still settles it.
  tilt::Steady steady_;
  float ref_[3] = {0, 0, -1000};
  bool refSet_ = false, starting_ = false;   // starting_: the go sign was pressed, the match starts once held steady
  Match match_{};
  uint32_t stepMs_ = 0;           // time not yet played, under one STEP_MS
  bool tapPending_ = false;       // a press not yet played (a frame too short for a step)
  uint16_t matchNo_ = 0;          // matches started this visit: the seed of the next one
  // What each of the panel's two buffers holds (os/app.h): this page, its movers (players, ball, the aim spot, the
  // clock; the Calibrate arrow) as boxes with keys, and the rest of its state as `look` (the score, the countdown, a
  // pressed sign, the hold). A buffer not listed (fb null after a page change) or with another look gets the whole
  // page; one listed gets each changed mover's old and new boxes, and nothing when none changed.
  static constexpr int MAX_MOVERS = PLAYERS + 3;
  struct Mover { paint::Box box; int key; };
  struct Held { const uint16_t* fb; Mover movers[MAX_MOVERS]; int n; uint32_t look; };
  Held held_[2] = {};

  void go(Screen s);
  void newMatch();
  void finish();                  // the Full time page, and the save: a win, the next level
  bool pressing(int cx, int cy, int r) const;   // the finger went down in this logical circle and is still in it
  bool onSign(int x, int y) const;
  bool leaving();
  float hold() const;             // how far holding the leave sign has got, 0..1 (Kickoff, Match, Goal)
  bool inPlay() const { return screen_ == SC_KICKOFF || screen_ == SC_MATCH || screen_ == SC_GOAL; }
  tilt::Vec calTilt() const;      // Calibrate: the tilt away from the slow reference
  bool ready() const;             // Calibrate: held still
  Kick kidAim() const;            // what a tap would do now (for the aim spot and the pass target)
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
