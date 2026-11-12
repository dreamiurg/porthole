// Stateless arcade run, entered afresh from the launcher. Rules and art remain independent of input handling.
#pragma once
#include "app.h"
#include "controls.h"
#include "rules.h"
#include "ui.h"

namespace starfighter {
class Game : public App {
 public:
  const char* name() const override { return "Star Fighter"; }
  const gfx::Sprite& icon() const override;
  const char* store() const override { return "starfighter"; }
  void enter(const AppEnter& e) override;
  void update(uint32_t nowSec, uint32_t ms, const Input& in) override;
  void render() override;
  Surface surface() const override { return SURFACE_RGB565; }
  Tint tint() const override { return daylight_ ? TINT_DAY : clockTint(now_); }
  bool asleep() const override { return false; }
  bool soundOn(uint32_t) override { return false; }
  bool takeSave(const void**, size_t*, bool) override { return false; }
  bool wantsHome() override { const bool wanted = home_; home_ = false; return wanted; }
  void leave() override {}
  const char* screenName() const override;
  void debugPrint() override;
  void debugCmd(const char* cmd) override;
  const Run& run() const { return run_; }
  float steering() const { return bank_; }
  enum class Screen : uint8_t { Title, Calibrate, Brief, Flight };

 private:
  Screen screen_ = Screen::Title;
  Run run_{};
  Input in_{};
  controls::Calibration calibration_;
  tilt::Grav neutral_ = {0, 1000, 0};
  ui::FreshGate gate_;
  ui::Hold hold_;
  uint32_t now_ = 0, rawMs_ = 0, ms_ = 0, stepMs_ = 0, taps_ = 0, seed_ = 1, runNo_ = 0;
  float bank_ = 0;
  bool home_ = false, daylight_ = false;

  void go(Screen screen);
  void startRun();
  void updateCalibrate();
  void updateFlight(uint32_t dt);
  void transition();
  bool terminal() const;
  bool leaving();
  void debugStage(Stage stage);
};
}  // namespace starfighter
