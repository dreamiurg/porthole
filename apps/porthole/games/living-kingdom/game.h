#pragma once
#include "app.h"
#include "art.h"
#include "canvas.h"
#include "engine.h"
#include "reader.h"
#include "ui.h"

namespace kingdom {
class Game : public App {
 public:
  const char* name() const override { return "Kingdom"; }
  const gfx::Sprite& icon() const override { return art::icon(); }
  const char* store() const override { return "kingdom"; }
  void enter(const AppEnter& e) override;
  void update(uint32_t nowSec, uint32_t ms, const Input& in) override;
  void render() override;
  Surface surface() const override { return SURFACE_RGB565; }
  Tint tint() const override { return TINT_DAY; }
  bool asleep() const override { return false; }
  bool soundOn(uint32_t) override { return false; }
  bool takeSave(const void** data, size_t* len, bool allowed) override;
  bool saveUrgent() const override { return dirty_; }
  bool wantsHome() override { const bool result = home_; home_ = false; return result; }
  void leave() override { dirty_ = true; }
  const char* screenName() const override;
  void debugPrint() override;
  void debugCmd(const char*) override {}
  const Save& save() const { return save_; }
  int pages() const { return reader_.count(); }
 private:
  Save save_{};
  Input in_{};
  ui::FreshGate gate_;
  canvas::Frames<1> frames_;
  Reader reader_;
  char prose_[8192] = {}, heading_[96] = {};
  bool title_ = true, dirty_ = false, home_ = false, fresh_ = true;
  uint32_t ms_ = 0;
  void changed();
  void mode(uint8_t value);
  void prepare();
  void prepareStory(const Scene& scene);
  void prepareInfo();
  void prepareJournal();
  int infoCount() const;
  void infoStep(int delta);
  void openMenu();
  void closeInfo();
  void updateTitle();
  void updateMap();
  void updateRead();
  void updateChoice();
  void updateConfirm();
  void updateResult();
  void updateMenu();
  void updateInfo();
  void updateTravel();
  void updateRest();
  bool clicked(const ui::Box& box) const;
  void draw();
  void drawTitle();
  void drawMap();
  void drawRead();
  void drawChoice();
  void drawConfirm();
  void drawResult();
  void drawMenu();
  void drawInfo();
  void drawPartyOverview();
  void drawInventoryOverview();
  void drawTravel();
  void drawRest();
};
} // namespace kingdom
