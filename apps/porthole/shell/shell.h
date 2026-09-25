// The Porthole shell: "Who's playing?", new profiles, PIN gate, the launcher, the rest screen, and hosting the
// active game. Platform-agnostic: the firmware and the host sim both run it with their own Store and app array.
#pragma once
#include <stdint.h>
#include "app.h"
#include "profiles.h"
#include "ui.h"

constexpr int MAX_APPS = 8;

class Shell {
 public:
  void begin(shell::Store& st, App* const* apps, int nApps);   // loads (or migrates) profiles, lands on the picker
  void update(uint32_t nowSec, uint32_t ms, const Input& in);   // also writes saves: app saves at most every 5 s
  void render();
  Tint tint() const;
  bool asleep() const { return screen_ == SH_APP && app_->asleep(); }
  bool soundOn(uint32_t ms);          // the active game's buzzer, silenced by the profile's mute
  uint32_t lastSeen() const;          // newest play time on the device: restores the clock when the RTC lost it
  int profileCount() const { return prof_.count(); }
  // serial and sim hooks
  void clearPin(int id);              // parent escape hatch (serial "P<n>")
  int createProfile(const char* name, uint8_t age, const char* code);   // code "1234", "" = none; lands on the launcher
  bool openApp(const char* name);     // from the launcher; matches App::name() ignoring case, spaces and dashes
  const char* screenName() const;
  void debugPrint();
  void debugCmd(const char* cmd);     // "tired" "rested" "younger" "older", then passed on to the open game

 private:
  enum Screen : uint8_t { SH_PICK, SH_NAME, SH_AVATAR, SH_AGE, SH_PIN_SET, SH_PIN_AGAIN, SH_PIN, SH_DELETE, SH_LAUNCHER, SH_REST, SH_APP };
  enum Key : uint8_t { KEY_NONE, KEY_SKIP, KEY_OK };

  shell::Store* st_ = nullptr;
  App* const* apps_ = nullptr; int nApps_ = 0;
  App* app_ = nullptr;                             // the open game, or the last one (its stats stay readable)
  shell::Profiles prof_{};
  int active_ = -1;                                // the profile on the launcher / in a game
  int target_ = -1; bool deleting_ = false;        // the picker row a PIN or a delete is about
  bool holdTop_ = false;                           // the delete button goes to the half the finger was not on
  bool creating_ = false;                          // the age screen is a creation step (else: a migrated profile)
  shell::Record draft_{};                          // the profile being created
  char name_[12] = {0}; int nameLen_ = 0; uint8_t namePage_ = 0;
  char pin_[5] = {0}; int pinLen_ = 0; uint16_t firstPin_ = 0; uint32_t pinWrongUntil_ = 0;
  uint32_t now_ = 0, ms_ = 0, screenMs_ = 0, lastTick_ = 0, lastSaveMs_ = 0, lastRecordSave_ = 0;
  Screen screen_ = SH_PICK;
  Input in_{};
  ui::FreshGate gate_;                             // every screen change ignores touches for a moment (os/ui.h)
  char toast_[40] = {0}; uint32_t toastUntil_ = 0;
  uint8_t blobs_[MAX_PROFILES][shell::BLOB_MAX];   // every profile's save for the game being opened

  shell::Record& rec() { return prof_.rec[active_]; }
  bool restingNow() const;   // rest only applies while there are 2+ profiles
  void go(Screen s);
  void toast(const char* s);
  void resetPin() { pinLen_ = 0; pin_[0] = 0; }
  // flow
  void startNew();
  void choose(int id, bool del);
  void authorized();
  void select(int id);
  int appStores(const char** out) const;   // every app's save namespace, for delete and create
  void finishCreate(uint16_t pin);
  void openIdx(int k);
  void closeApp();
  void flushApp(bool allowed);
  void budgetTick();
  Key keypad(bool canSkip);
  // screens
  void updatePick(); void drawPick();
  void updateName(); void drawName();
  void updateAvatar(); void drawAvatar();
  void updateAge(); void drawAge();
  void updatePin(); void drawPin(); void drawPinKey(const ui::Box& b, const char* label, uint8_t col);
  void updateDelete(); void drawDelete();
  void updateLauncher(); void drawLauncher();
  void updateRest(); void drawRest();
  void updateApp(); void drawApp();
};
