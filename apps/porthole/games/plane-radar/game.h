#pragma once
#include "air.h"
#include "app.h"
#include "canvas.h"
#include "render.h"
#include "save.h"
#include "ui.h"

namespace radar {
// The live scope around the device's home (air::Feed), or a note while there is nothing to show. A tap on the scope
// steps the range; the middle button opens the menu (which way is up, range, what the labels show). Each kid's
// settings are saved; home is the device's, set by a grown-up and kept by the feed.
class Game : public App {
 public:
  const char* name() const override { return "Radar"; }
  const gfx::Sprite& icon() const override { return paint::icon(); }
  const char* store() const override { return "radar"; }   // NVS namespace: never renamed once shipped
  void enter(const AppEnter& e) override;
  void update(uint32_t nowSec, uint32_t ms, const Input& in) override;
  void render() override;
  Surface surface() const override { return SURFACE_RGB565; }
  Tint tint() const override { return TINT_DAY; }
  bool asleep() const override { return false; }
  bool soundOn(uint32_t) override { return false; }
  bool takeSave(const void** data, size_t* len, bool allowed) override;
  bool wantsHome() override { bool h = home_; home_ = false; return h; }
  void leave() override;
  const char* screenName() const override;
  void debugPrint() override;
  void debugCmd(const char*) override {}
  const Save& save() const { return save_; }
  int ageSec() const;   // whole seconds since the planes were fetched: they are drawn flown on that far

 private:
  air::Snapshot snap_;
  Save save_{};
  Input in_;
  ui::FreshGate gate_;
  canvas::Frames<1> frames_;
  paint::Page page_ = paint::P_SCOPE;
  char spot_[32] = "";   // the menu's line about home
  uint32_t ms_ = 0, fetchMs_ = 0, gen_ = 0;   // when snap_.gen last changed
  bool home_ = false, dirty_ = false;
  void go(paint::Page p);
  void setRange(int r);
  void scope();
  void menu();
  bool clicked(const ui::Box& b) const;
};
}  // namespace radar
