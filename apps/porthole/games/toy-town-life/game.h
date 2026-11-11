#pragma once
#include "app.h"
#include "state.h"
#include "ui.h"
namespace toytown {
class Game : public App {
 public:
  const char* name() const override { return "Toy Town"; }
  const gfx::Sprite& icon() const override;
  const char* store() const override { return "toytown"; }
  void enter(const AppEnter& e) override;
  void update(uint32_t now, uint32_t ms, const Input& in) override;
  void render() override;
  Surface surface() const override { return SURFACE_RGB565; }
  Tint tint() const override { return TINT_DAY; }
  bool asleep() const override { return false; }
  bool soundOn(uint32_t) override { return false; }
  bool takeSave(const void** data, size_t* len, bool allowed) override;
  bool saveUrgent() const override { return dirty_; }
  bool wantsHome() override { bool h = home_; home_ = false; return h; }
  void leave() override { stopWalk(); dirty_ = true; }
  const char* screenName() const override;
  void debugPrint() override;
  void debugCmd(const char*) override {}
  const State& state() const { return s_; }
 private:
  State s_{};
  Input in_{};
  ui::FreshGate gate_;
  uint32_t ms_ = 0, look_ = 0;
  const uint16_t* buffers_[2]{};
  uint32_t painted_[2]{};
  bool dirty_ = false, home_ = false, title_ = true, pause_ = false;
  char heading_[96]{}, words_[384]{}, labels_[5][80]{};
  uint8_t destination_ = SCHOOL;
  bool walking_=false, displayTried_=false;
  uint32_t walkFrom_=0;
  int walkX_=80,walkY_=80,targetX_=80,targetY_=80;
  void sceneInput();
  bool overlayInput(int action);
  void actSchoolIntro(int i);
  bool sceneDepart();
  bool sceneNpc();
  bool sceneLandmark();
  void describeSceneSchool();
  void tickWalk();
  void stopWalk();
  void beginWalk(int x,int y);
  void drawScene();
  void drawDisplay();
  void displayInput();
  void actDisplay(int i);
  void actSchoolRepair(int i);
  void openDisplay(bool repair);
  void drawSceneCards();
  void drawSceneCard(int i,int count);
  void describeScene();
  void changed();
  void go(uint8_t p);
  void act(int i);
  void actHome(int i);
  void actDress(int i);
  bool dressInput();
  void dollInput(bool exit);
  void drawDress();
  void actNavigate(int i);
  void actPack(int i);
  void describeEntry(const Entry& e);
  void actSchoolPlan(int i);
  void actCafeTray(int i);
  void actParkGuess(int i);
  void describeSchool();
  void describeOverlay();
  void describeStyle();
  void actSchool(int i);
  void actCafe(int i);
  void actPark(int i);
  void actMemory(int i);
  void describe();
  void describeHome();
  void describeCafe();
  void describeCafeCart();
  void describePark();
  void draw();
  void drawHero();
  void drawButtons();
};
} // namespace toytown
