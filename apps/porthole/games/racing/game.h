// Rally: a short, silent arcade race, stateless between runs. Every launcher entry starts at the track cards.
#pragma once
#include "app.h"
#include "controls.h"
#include "race.h"
#include "render.h"
#include "save.h"
#include "ui.h"

namespace racing {
class Game : public App {
 public:
  const char* name() const override { return "Rally"; }
  const gfx::Sprite& icon() const override { return paint::icon(); }
  const char* store() const override { return "racing"; }
  void enter(const AppEnter& e) override;
  void update(uint32_t nowSec, uint32_t ms, const Input& in) override;
  void render() override;
  Surface surface() const override { return SURFACE_RGB565; }
  Tint tint() const override { return daylight_ ? TINT_DAY : clockTint(now_); }
  bool asleep() const override { return false; }
  bool soundOn(uint32_t) override { return false; }
  bool takeSave(const void** data, size_t* len, bool allowed) override;
  bool wantsHome() override { const bool w = home_; home_ = false; return w; }
  void leave() override {}
  const char* screenName() const override;
  void debugPrint() override;
  void debugCmd(const char* cmd) override;
  const Race& race() const { return race_; }
  float steering() const { return steer_; }
  using Screen = paint::Page;   // one page per screen
  Screen screen() const { return screen_; }

 private:
  Screen screen_ = Screen::Tracks;
  Race race_{};
  Input in_{};
  controls::Calibration calibration_;
  tilt::Grav neutral_ = {0, 1000, 0}, grip_ = {0, 1000, 0};   // the race's straight ahead; the grip page's candidate
  ui::FreshGate gate_;
  ui::Hold hold_;
  paint::Leave leave_ = paint::Leave::Restart;
  uint32_t now_ = 0, rawMs_ = 0, ms_ = 0, stepMs_ = 0, resumeMs_ = 0, badGripMs_ = 0, finishMs_ = 0, seed_ = 1, runs_ = 0;
  int track_ = 0;
  float steer_ = 0;
  bool home_ = false, daylight_ = false, seenHowTo_ = false, recenter_ = false, gripLost_ = false, gripSet_ = false;
  bool boost_ = false, rocket_ = false;   // fresh presses waiting for the next simulation step
  // The leaderboard: every profile's best times (their saves, read on entry), this kid's own among them.
  struct Kid { char name[12]; Best best[TRACKS]; bool me; };
  Kid kids_[MAX_PROFILES] = {};
  int nKids_ = 0, me_ = -1;
  Save save_{};
  bool dirty_ = false, recorded_ = false;
  uint8_t newBest_ = 0;            // BEST_RACE / BEST_LAP bits from this finish
  uint32_t prevBestMs_ = 0;        // the kid's best race here before this finish (0: first time)

  tilt::Grav gravity() const { return {in_.gx, in_.gy, in_.gz}; }
  bool gripReady() const { return gripSet_ && controls::upright(gravity()); }   // a steady grip, still held like a wheel
  void go(Screen s);
  void startRace();
  void updateTracks();
  void updateHowTo();
  void updateCalibrate();
  void updateRace(uint32_t dt);
  void updatePause();
  void updateConfirm();
  void updateResults();
  bool gripOk(uint32_t dt);
  void simulate(uint32_t dt);
  void debugFixture(const char* cmd);
  void loadKids(const AppEnter& e);
  void recordFinish();
  void seedLevels();
  int board(paint::Row out[MAX_PROFILES]) const;
};
}  // namespace racing
