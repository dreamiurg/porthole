// The Porthole shell: "Who's playing?", new profiles, PIN gate, the launcher, the rest screen, and hosting the
// active game. Platform-agnostic: the firmware and the host sim both run it with their own Store and app array.
#pragma once
#include <stdint.h>
#include "app.h"
#include "profiles.h"
#include "system.h"
#include "ui.h"

constexpr int MAX_APPS = 8;

namespace shell {
// "sys"/"ran" holds the version that booted last (its bytes, no NUL). True when version is not "dev" and differs from
// it: an update (or a USB flash) since the last boot, so the "something new" card shows once. Stores version if new.
bool newRun(Store& st, const char* version);

// The kids' brightness, picked on the launcher's sun button: one per device, one byte in "sys"/"lite" (missing or
// unknown: Auto). Auto is the light of the clock: evening and night tints, the games' night scenes, a full backlight.
// The others are always daylight at their backlight %. Only the active level: the firmware still dims to 30% after a
// minute idle and goes dark after five (a game asleep(): 40%, then dark), which the wireless updater relies on.
enum Light : uint8_t { LIGHT_AUTO, LIGHT_BRIGHT, LIGHT_MEDIUM, LIGHT_SOFT, LIGHT_COUNT };
constexpr uint8_t LIGHT_BACKLIGHT[LIGHT_COUNT] = {100, 100, 65, 40};   // Soft stays above the idle 30%: dimming still shows
}

class Shell {
 public:
  // loads (or migrates) profiles, lands on the picker; sys (wireless updates, the system menu) may be null
  void begin(shell::Store& st, App* const* apps, int nApps, shell::System* sys = nullptr);
  void update(uint32_t nowSec, uint32_t ms, const Input& in);   // also writes saves: app saves at most every 5 s
  void render();
  Tint tint() const;
  uint8_t backlight() const { return shell::LIGHT_BACKLIGHT[light_]; }   // % while active (shell::Light)
  Surface surface() const { return screen_ == SH_APP ? app_->surface() : SURFACE_INDEXED; }   // the shell's own screens are indexed
  bool asleep() const { return screen_ == SH_APP && app_->asleep(); }
  bool soundOn(uint32_t ms);          // the active game's buzzer, silenced by the profile's mute
  uint32_t lastSeen() const;          // newest play time on the device: restores the clock when the RTC lost it
  int profileCount() const { return prof_.count(); }
  // serial and sim hooks
  void clockRestored() { shell::liftCaps(*st_, prof_); }   // the RTC was lost and the clock guessed: no cap holds
  void clearPin(int id);              // parent escape hatch (serial "P<n>")
  int createProfile(const char* name, uint8_t age, const char* code);   // code "1234", "" = none; lands on the launcher
  bool openApp(const char* name);     // from the launcher; matches App::name() ignoring case, spaces and dashes
  const char* screenName() const;
  void debugPrint();
  void debugCmd(const char* cmd);     // "tired" "bedtime" "rested" "younger" "older" "light" (the next level), then the open game

 private:
  enum Screen : uint8_t { SH_PICK, SH_NAME, SH_AVATAR, SH_AGE, SH_PIN_SET, SH_PIN_AGAIN, SH_PIN, SH_DELETE, SH_LAUNCHER, SH_REST,
                         SH_SYS_NEW, SH_SYS_INFO, SH_SYS_UPD, SH_LIGHT, SH_APP };   // SH_APP last: the tables in shell.cpp

  shell::Store* st_ = nullptr;
  shell::System* sys_ = nullptr;
  App* const* apps_ = nullptr; int nApps_ = 0;
  App* app_ = nullptr;                             // the open game, or the last one (its stats stay readable)
  bool opened_[MAX_APPS] = {};                     // games entered since boot: debugPrint shows their pets
  shell::Profiles prof_{};
  int active_ = -1;                                // the profile on the launcher / in a game
  shell::Light light_ = shell::LIGHT_AUTO;         // the device's brightness ("sys"/"lite")
  int page_ = 0;                                   // the launcher's page of games: 0 for a new kid, else the last game's
  int target_ = -1; bool deleting_ = false;        // the picker row a PIN or a delete is about
  bool holdTop_ = false;                           // the delete button goes to the half the finger was not on
  bool creating_ = false;                          // the age screen is a creation step (else: a migrated profile)
  shell::Record draft_{};                          // the profile being created
  char name_[12] = {0}; int nameLen_ = 0; uint8_t namePage_ = 0;
  char pin_[5] = {0}; int pinLen_ = 0; uint16_t firstPin_ = 0; uint32_t pinWrongUntil_ = 0, pinMs_ = 0;   // pinMs_: the last digit
  uint32_t now_ = 0, ms_ = 0, screenMs_ = 0, lastTick_ = 0, lastSaveMs_ = 0, lastRecordSave_ = 0;
  ActivityTracker activity_;                       // idle (shell::IDLE_MS without a touch or a move) is not play time
  Screen screen_ = SH_PICK;
  Input in_{};
  ui::FreshGate gate_;                             // every screen change ignores touches for a moment (os/ui.h)
  char toast_[40] = {0}; uint32_t toastUntil_ = 0;
  shell::SysInfo info_{}; shell::UpdStatus upd_{};   // the system menu's view of sys_, read when a menu screen opens
  uint8_t blobs_[MAX_PROFILES][shell::BLOB_MAX];   // every profile's save for the game being opened

  shell::Record& rec() { return prof_.rec[active_]; }
  bool restingNow() const;   // the day's play is used up, or (with 2+ profiles) a turn rest
  void go(Screen s);
  void land();   // the picker, or the first profile's name on a fresh device
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
  void submitPin();
  // screens
  void updatePick(); void drawPick();
  void updateName(); void drawName();
  void updateAvatar(); void drawAvatar();
  void updateAge(); void drawAge();
  void updatePin(); void drawPin(); void drawPinDigits(); void drawPinKey(const ui::Box& b, const char* label, uint8_t col);
  void updateDelete(); void drawDelete();
  void updateLauncher(); void drawLauncher(); void drawAppTile(int k, int i, int m); void drawPageArrow(bool right);
  bool muteButton();   // draws the launcher's mute button; true on a tap
  void setLight(shell::Light l);
  void updateLight(); void drawLight();
  void updateRest(); void drawRest();
  void updateApp(); void drawApp();
  // sysmenu.cpp: the "something new" card and the hidden system menu (hold the picker's title)
  bool titleHeld();
  void updateSysNew(); void drawSysNew();
  void updateSysInfo(); void drawSysInfo();
  void updateSysUpd(); void drawSysUpd(); void drawSysAction();
};
