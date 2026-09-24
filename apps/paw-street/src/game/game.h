// The game: screens, animation, and glue around the pet simulation. Platform-agnostic.
#pragma once
#include <stdint.h>
#include "gfx.h"
#include "input.h"
#include "pet.h"

enum Sound : uint8_t { SND_NONE = 0, SND_TAP, SND_YIP, SND_HAPPY, SND_SAD, SND_CHIME, SND_BARK, SND_FANFARE };

class Game {
 public:
  void begin(uint32_t nowSec, uint32_t ms, const Save* houses, int count);
  void update(uint32_t nowSec, uint32_t ms, const Input& in);
  void render();
  Tint tint() const;
  bool asleep() const { return save_.asleep != 0; }
  // Save handling: hands out one dirty house at a time (slot = index) when allowed.
  bool takeSave(Save* out, int* slot, bool allowed);
  bool takeErase(int* slot);          // a slot that should be wiped after a house was deleted
  void clearPin(int slot);            // parent escape hatch (serial "P<n>")
  int houseCount() const { return nHouses_; }
  // Buzzer pattern player: true while the buzzer should be on.
  bool soundOn(uint32_t ms);
  void platformSoundStart(int id) { sound((Sound)id); }
  void debugPrint();
  const char* screenName() const;
  void debugCmd(const char* cmd);  // test hook: "dirty" "poop" "hungry" "hearts" "books" "tricks" "hats" "grown" "dog" "sleepy"

 private:
  enum Screen : uint8_t { SC_SPLASH, SC_INTRO, SC_NAME_KID, SC_NAME_PET, SC_HOME, SC_FEED, SC_PLAYMENU, SC_FETCH, SC_WORDS,
                          SC_LIBRARY, SC_READ, SC_TRICKS, SC_TRAIN, SC_BATH, SC_STATS, SC_STICKERS, SC_GIFT, SC_CELEBRATE, SC_CONFIRM_RESET, SC_HATS,
                          SC_STREET, SC_AGE, SC_THEME, SC_PIN_SET, SC_PIN_ENTER, SC_REST };
  struct Particle { int16_t x, y; int8_t vx, vy; uint8_t kind, life; };

  // ---- state
  Save save_{};                                    // working copy of the active house
  Save houses_[MAX_HOUSES]; int nHouses_ = 0, active_ = -1;
  bool dirtyHouse_[MAX_HOUSES] = {false, false, false}; int eraseSlot_ = -1;
  bool dirty_ = false, haveSave_ = false;
  Screen ageReturn_ = SC_HOME;                     // where the age question returns to
  int pinSlot_ = -1; char pinBuf_[5] = {0}; int pinLen_ = 0; uint32_t pinWrongUntil_ = 0;
  uint8_t bookList_[32]; int nBooks_ = 0;          // stories this kid may read, by age
  int wordCols_ = 3;
  uint32_t now_ = 0, ms_ = 0, screenMs_ = 0, lastTickSec_ = 0;
  Screen screen_ = SC_SPLASH, nextAfterCelebrate_ = SC_HOME;
  Input in_{};
  Sound sound_ = SND_NONE; uint32_t soundStartMs_ = 0;
  char toast_[40] = {0}; uint32_t toastUntil_ = 0;
  Particle parts_[24] = {};
  // home / dog
  int dogX_ = 64, dogTargetX_ = 64; bool dogFlip_ = false; int8_t dogAct_ = 0; uint32_t dogActUntil_ = 0; int trickShow_ = -1;
  uint32_t nextIdleMs_ = 0, nextEventMs_ = 0; uint8_t event_ = 0; int eventX_ = 0, eventY_ = 0; uint32_t eventUntil_ = 0;
  uint8_t weather_ = 0;  // 0 clear 1 rain
  // keyboard
  char nameBuf_[12] = {0}; int nameLen_ = 0;
  // feed
  int feedAnimFood_ = -1; uint32_t feedAnimUntil_ = 0;
  // fetch minigame
  struct Falling { int16_t x, y, vy; uint8_t kind, alive; } falling_[6];
  int fetchScore_ = 0, fetchMiss_ = 0; uint32_t fetchEndMs_ = 0, fetchNextSpawn_ = 0; int fetchDogX_ = 64; bool fetchDone_ = false;
  // words minigame
  int wordIdx_[5] = {0}; int wordRound_ = 0, wordTyped_ = 0, wordCorrect_ = 0; char tiles_[12] = {0}; bool tileUsed_[12] = {false};
  int wordWrongTile_ = -1; uint32_t wordWrongUntil_ = 0; uint32_t wordSolvedUntil_ = 0; bool wordsDone_ = false;
  // library / reading
  int bookSel_ = 0, page_ = 0, answerPick_ = -1; uint32_t answerUntil_ = 0;
  // tricks / training
  int trickSel_ = 0; uint8_t seq_[6] = {0}; int seqLen_ = 2, seqShow_ = -1, seqInput_ = 0, trainRound_ = 0; uint32_t seqStepMs_ = 0;
  uint8_t trainPhase_ = 0;  // 0 intro 1 showing 2 input 3 round ok 4 fail 5 passed
  // bath
  struct Spot { int8_t x, y; bool on; } spots_[6]; int spotsLeft_ = 0; uint32_t bathDoneMs_ = 0;
  // gift / celebrate
  pet::Gift gift_{}; bool giftOpened_ = false; char celebTitle_[24] = {0}, celebText_[48] = {0}; int celebIcon_ = 0;
  uint8_t pendingStickers_ = 0; uint16_t stickersSeen_ = 0;
  int statsPage_ = 0; int hatSel_ = 0;

  // ---- helpers
  void go(Screen s);
  void sound(Sound s) { sound_ = s; soundStartMs_ = ms_; }
  void toast(const char* s, uint32_t ms = 2200);
  void spawn(int x, int y, int kind, int n);
  void updateParticles();
  void drawParticles();
  void markDirty() { dirty_ = true; }
  int dogSize() const;
  const gfx::Sprite& dogFrame(int pose) const;
  void drawDog(int x, int y, int pose, bool flip, int scale = 1);
  void drawHat(int x, int y, int pose, bool flip, int scale);
  void drawButton(int x, int y, int w, int h, const char* label, const gfx::Sprite* icon, uint8_t col, bool pressed = false);
  bool button(int x, int y, int w, int h, const char* label, const gfx::Sprite* icon, uint8_t col);
  bool iconButton(int cx, int cy, int r, const gfx::Sprite& icon, uint8_t col, uint8_t iconCol = 0);
  void drawBackButton();
  bool backButton();
  void drawToast();
  void drawPanel(int x, int y, int w, int h, uint8_t fill, uint8_t border);
  void drawHearts(int cx, int y, int hearts);
  void drawStatPips(int x, int y, int value, const gfx::Sprite& icon, uint8_t col);
  void drawRoom();
  void drawWindow(int x, int y, int w, int h);
  void drawShelf(int x, int y);
  int  idlePose();
  void startTrickShow(int trick);
  void checkStickers();
  void celebrate(const char* title, const char* text, int icon, Screen next);
  void newDayCheck();

  // ---- screens
  void updateSplash(); void drawSplash();
  void updateIntro(); void drawIntro();
  void updateKeyboard(); void drawKeyboard(const char* prompt);
  void updateHome(); void drawHome();
  void updateFeed(); void drawFeed();
  void updatePlayMenu(); void drawPlayMenu();
  void startFetch(); void updateFetch(); void drawFetch();
  void startWords(); void nextWord(); void updateWords(); void drawWords();
  void updateLibrary(); void drawLibrary();
  void updateRead(); void drawRead();
  void updateTricks(); void drawTricks();
  void startTrain(); void updateTrain(); void drawTrain();
  void trainZones(int zx[3], int zy[3], int& scale, int& dx, int& dy) const;
  int trainHitZone(int x, int y) const;
  void startBath(); void updateBath(); void drawBath();
  void updateStats(); void drawStats();
  void updateStickers(); void drawStickers();
  void updateGift(); void drawGift();
  void updateCelebrate(); void drawCelebrate();
  void updateConfirmReset(); void drawConfirmReset();
  void updateHats(); void drawHats();
  void updateStreet(); void drawStreet();
  void updateAge(); void drawAge();
  void updateTheme(); void drawTheme();
  void updatePin(); void drawPin(const char* prompt);
  void updateRest(); void drawRest();
  // houses
  void enterHouse(int i);
  void leaveHouse();
  void finishAdoption();
  void refreshBookList() { nBooks_ = pet::bookListFor(save_, bookList_); }
  int tileX(int i) const { return (wordCols_ == 3 ? 41 : 28) + (i % wordCols_) * 26; }
  int tileY(int i) const { return (wordCols_ == 3 ? 66 : 60) + (i / wordCols_) * 26; }
  const struct Book& curBook() const;
};
