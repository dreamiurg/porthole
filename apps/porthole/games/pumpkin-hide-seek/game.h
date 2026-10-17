#pragma once
#include "app.h"
#include "render.h"
#include "rules.h"
#include "tilt.h"
#include "ui.h"

namespace pumpkin {
class Game : public App {
 public:
  const char* name() const override { return "Pumpkin"; }
  const gfx::Sprite& icon() const override { return paint::icon(); }
  const char* store() const override { return "pumpkin"; }
  void enter(const AppEnter& e) override;
  void update(uint32_t nowSec, uint32_t ms, const Input& in) override;
  void render() override;
  Surface surface() const override { return SURFACE_RGB565; }
  Tint tint() const override { return TINT_DAY; }
  bool asleep() const override { return false; }
  bool soundOn(uint32_t) override { return false; }
  bool takeSave(const void**, size_t*, bool) override { return false; }
  bool wantsHome() override { bool home = home_; home_ = false; return home; }
  void leave() override {}
  const char* screenName() const override;
  void debugPrint() override;
  void debugCmd(const char*) override {}
  const Round& round() const { return round_; }

 private:
  Page page_ = Page::Intro;
  Round round_;
  Input in_;
  ui::FreshGate gate_;
  ui::Hold hold_;
  tilt::Steady steady_;
  tilt::Grav neutral_ = {0, 0, -1000};
  uint32_t ms_ = 0, pageMs_ = 0, animationMs_ = 0, stepMs_ = 0;
  bool starting_ = false, home_ = false, actionPending_ = false;
  void go(Page page);
  void start();
  void calibrate();
  void play(uint32_t dt);
  bool ready() const;
  bool action() const;
  bool leaving();
};
}  // namespace pumpkin
