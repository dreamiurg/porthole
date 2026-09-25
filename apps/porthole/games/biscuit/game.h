// Biscuit: a companion pup that grows through care, shared stories, curiosity and little tricks. Runs as an App in the
// Porthole shell (profiles, saves, the daily play cap, rest screens and the backlight are the shell's). Every screen
// draws on the full-resolution RGB565 surface except pet naming, which uses the shell's indexed name keyboard.
// The rules are pet.h; the screens live in screens_*.cpp, one file per group, the widgets in ui565.
#pragma once
#include <stdint.h>
#include "app.h"
#include "pet.h"
#include "ui.h"

namespace biscuit {
class Game : public App {
 public:
  const char* name() const override { return "Biscuit"; }
  const gfx::Sprite& icon() const override;
  const char* store() const override { return STORE; }
  void enter(const AppEnter& e) override;
  void update(uint32_t nowSec, uint32_t ms, const Input& in) override;
  void render() override;
  Surface surface() const override;
  Tint tint() const override;
  bool asleep() const override { return save_.named && save_.sleeping; }
  bool soundOn(uint32_t) override { return false; }   // Biscuit never drives the buzzer
  bool takeSave(const void** data, size_t* len, bool allowed) override;
  bool wantsHome() override { bool w = wantsHome_; wantsHome_ = false; return w; }
  void leave() override { if (save_.named) dirty_ = true; fetch_ = -1; }
  const char* screenName() const override;
  void debugPrint() override;
  void debugCmd(const char* cmd) override;   // "hungry" "unlock" "young" "grown" "tricks" "practiced": test states

  // shared by the screens (screens_*.cpp)
  enum Screen : uint8_t { SC_SETUP_PET, SC_HOME, SC_WORLD, SC_TRICKS, SC_TRAINING, SC_PROFILE, SC_RENAME_PET, SC_COUNT };

 private:
  Save save_{}, out_{};                       // the working pup, and the sealed copy takeSave hands out
  Profile who_{};                             // the kid playing: {name} in every line
  bool dirty_ = false, wantsHome_ = false;
  uint32_t now_ = 0, ms_ = 0, lastTickSec_ = 0, lastCheckpointSec_ = 0;
  Input in_{};
  ui::FreshGate gate_;                        // every screen change ignores touches for a moment (os/ui.h)
  Screen screen_ = SC_HOME;
  // Home: what the pup says and does. A line lasts 4 s, its activity as long (a room one-shot 800 ms), then the pup
  // goes back to idling.
  char speech_[256] = {0};                    // personalized: 256 holds the longest line with 12-letter names
  uint8_t activity_ = 0;                      // SceneActivity
  uint32_t sayUntilMs_ = 0, actUntilMs_ = 0, actStartMs_ = 0, lastSurpriseMs_ = 0;   // 0 = nothing pending
  int fetch_ = -1;                            // catches so far, -1 = not playing fetch
  int page_ = 0;                              // Tricks: the list page
  int trick_ = 0, step_ = 0; bool watching_ = true;   // Training
  char nameBuf_[ui::NAME_LEN + 1] = {0}; int nameLen_ = 0; uint8_t namePage_ = 0;   // naming

  void go(Screen s);
  void markDirty() { dirty_ = true; }
  void say(const char* line, uint8_t activity);   // personalized, with what the pup does meanwhile
  void animate(uint8_t activity);             // SCENE_IDLE: nothing
  void quiet();                               // the idle line, idling
  void tick();
  void surprise();
  void startNaming(const char* current);
  // screens
  void updateSetupPet(); void drawSetupPet();
  void updateHome(); void drawHome();
  void homeTaps(); void homeFetch(); void drawHomeTop(); void drawScene();
  void updateWorld(); void drawWorld();
  void updateTricks(); void drawTricks();
  void openTrick(int id);
  void updateTraining(); void drawTraining();
  void updateProfile(); void drawProfile();
  void updateRenamePet(); void drawRenamePet();
  void toggleNap();
};
}  // namespace biscuit
