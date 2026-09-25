// Biscuit: a companion pup that grows through care, shared stories, curiosity and little tricks. Runs as an App in the
// Porthole shell (profiles, saves, the daily play cap, rest screens and the backlight are the shell's). Every screen
// draws on the full-resolution RGB565 surface except pet naming, which uses the shell's indexed name keyboard.
// The rules are pet.h; the screens live in screens_*.cpp, one file per group, the widgets in ui565.
#pragma once
#include <stdint.h>
#include "app.h"
#include "content_daily.h"
#include "content_stories.h"
#include "layout.h"
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
  void leave() override { if (save_.named) dirty_ = true; fetch_ = -1; shelfAtMs_ = 0; }
  const char* screenName() const override;
  void debugPrint() override;
  // test states: "hungry" "unlock" "young" "grown" "tricks" "practiced" "read" (every story and discovery done)
  void debugCmd(const char* cmd) override;

  // shared by the screens (screens_*.cpp)
  enum Screen : uint8_t {
    SC_SETUP_PET, SC_HOME, SC_WORLD, SC_TRICKS, SC_TRAINING, SC_PROFILE, SC_RENAME_PET, SC_LIBRARY, SC_STORY, SC_CHOICE,
    SC_ENDING, SC_DISCOVERIES, SC_TOPICS, SC_DISCOVERY, SC_SOURCE, SC_TODAY, SC_WORD, SC_STICKERS, SC_COUNT
  };

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
  uint8_t said_ = SAY_IDLE;                   // the last line said (debug output)
  uint32_t sayUntilMs_ = 0, actUntilMs_ = 0, actStartMs_ = 0, lastSurpriseMs_ = 0;   // 0 = nothing pending
  int fetch_ = -1;                            // catches so far, -1 = not playing fetch
  uint32_t shelfAtMs_ = 0;                    // the shelf was tapped: the Library opens then (0 = not pending)
  Screen libraryBack_ = SC_HOME, tricksBack_ = SC_WORLD;   // where Back goes: Home and World, or Today's adventure
  bool claimed_ = false, stickerNews_ = false;   // today's sticker as last seen; just earned, not yet told at home
  int page_ = 0;                              // Tricks, Library, Discoveries, Topics, Stickers: the list page
  int trick_ = 0, step_ = 0; bool watching_ = true;   // Training
  char nameBuf_[ui::NAME_LEN + 1] = {0}; int nameLen_ = 0; uint8_t namePage_ = 0;   // naming
  // Reading (a story's pages, an ending's, a discovery's): each content page is split into at most PAGE_SCREENS
  // screens, counted across all of them. at_ is the screen shown; a discovery also has its cover (-1) and its wonder
  // page (total_).
  const char* const* pages_ = nullptr; int pageCount_ = 0, total_ = 0, at_ = 0;
  uint8_t screens_[STORY_PAGES] = {0};
  char text_[256] = {0}; const char* shown_[PAGE_SCREENS] = {nullptr}; int textPage_ = -1;   // at_'s page, filled
  int story_ = 0, choice_ = 0;
  enum FactList : uint8_t { TODAYS_THREE, TOPIC, NOTEBOOK };
  FactList facts_ = TODAYS_THREE, topicsFrom_ = TODAYS_THREE; int topic_ = 0, fact_ = 0;   // topicsFrom_: Topics' Back

  void go(Screen s);
  void fresh();                               // the layout under the finger changed
  void markDirty() { dirty_ = true; }
  void say(Say line, uint8_t activity);   // personalized, with what the pup does meanwhile
  void animate(uint8_t activity);             // SCENE_IDLE: nothing
  void quiet();                               // the idle line, idling
  void tick();
  void surprise();
  void startNaming(const char* current);
  int timeOfDay() const { return (int)tint(); }   // the scenes' light: 0 day, 1 evening, 2 night
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
  void updateLibrary(); void drawLibrary();
  void openStory(int id);
  void updateStory(); void drawStory();       // the story's pages and, as SC_ENDING, the chosen ending's
  void updateChoice(); void drawChoice();
  void openPages(const char* const* pages, int count, int at);   // at < 0: from the end
  const char* pageText();                     // at_'s screen, personalized
  void updateDiscoveries(); void drawDiscoveries();
  void showFacts(FactList which, int topic);
  int factList(uint8_t* ids) const;           // the discoveries the list shows, in order
  void updateTopics(); void drawTopics();
  void openFact(int id);
  void updateDiscovery(); void drawDiscovery();
  void updateSource(); void drawSource();
  void updateToday(); void drawToday();
  void todayActivity(int bit);
  void updateWord(); void drawWord();
  void updateStickers(); void drawStickers();
  void openLibrary(Screen back); void openTricks(Screen back);
  void stickerNews();
};
}  // namespace biscuit
