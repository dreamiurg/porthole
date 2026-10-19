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

// The system menu's text entry (a wifi password): any printable ASCII, up to TEXT_MAX bytes. Keys 0..TK_CHARS-1 type
// the page's characters (letters, a-z or A-Z with the shift lock; digits and symbols; more symbols), then shift, the
// page switch, space, backspace and OK.
constexpr int TEXT_MAX = 64;
enum TextKey { TK_CHARS = 26, TK_SHIFT = TK_CHARS, TK_PAGE, TK_SPACE, TK_DEL, TK_OK, TK_COUNT };
struct TextEntry { char buf[TEXT_MAX + 1] = {0}; int len = 0; uint8_t page = 0; bool upper = false; };
char textChar(const TextEntry& t, int key);   // what key types right now; 0 for none (an empty slot, a control)
bool textKey(TextEntry& t, int key);           // applies one key; true for OK with at least one character typed

// The kids' brightness, picked on the launcher's sun button: one per device, one byte in "sys"/"lite" (missing or
// unknown: Auto). Auto is the light of the clock: evening and night tints, the games' night scenes, a full backlight.
// The others are always daylight at their backlight %. Only the active level: the firmware still dims to 30% after a
// minute idle and goes dark after five (a game asleep(): 40%, then dark), which the wireless updater relies on.
// The values are what "sys"/"lite" holds: append only, a new level goes right before LIGHT_COUNT.
enum Light : uint8_t { LIGHT_AUTO, LIGHT_BRIGHT, LIGHT_MEDIUM, LIGHT_SOFT, LIGHT_COUNT };
constexpr uint8_t LIGHT_BACKLIGHT[LIGHT_COUNT] = {100, 100, 65, 40};   // Soft stays above the idle 30%: dimming still shows

// The backlight % the firmware sets: `active` (Shell::backlight()) in use, 30 after IDLE_MS without activity, dark
// after 5 min; a game asleep(): 40, dark after 20 s. Dark at every level: the wireless updater writes flash only then.
constexpr uint32_t DARK_MS = 300000, ASLEEP_DARK_MS = 20000;
inline uint8_t backlightFor(bool asleep, uint32_t idleMs, uint8_t active) {
  if (asleep) return idleMs > ASLEEP_DARK_MS ? 0 : 40;
  return idleMs > DARK_MS ? 0 : idleMs > IDLE_MS ? 30 : active;
}
}

class Shell {
 public:
  // loads (or migrates) profiles, lands on the picker; sys (wireless updates, the system menu) may be null
  void begin(shell::Store& st, App* const* apps, int nApps, shell::System* sys = nullptr);
  void update(uint32_t nowSec, uint32_t ms, const Input& in);   // also writes saves: app saves at most every 5 s
  void render();
  Tint tint() const;
  uint8_t backlight() const { return shell::LIGHT_BACKLIGHT[light_]; }   // % while active (shell::Light)
  Surface surface() const { return screen_ == SH_APP ? app_->surface() : adultScreen() ? SURFACE_RGB565 : SURFACE_INDEXED; }
  bool asleep() const { return screen_ == SH_APP && app_->asleep(); }
  bool soundOn(uint32_t ms);          // the active game's buzzer, silenced by the profile's mute
  uint32_t lastSeen() const;          // newest play time on the device: restores the clock when the RTC lost it
  int profileCount() const { return prof_.count(); }
  // serial and sim hooks
  void clockRestored() { shell::liftCaps(*st_, prof_); }   // the RTC was lost and the clock guessed: no cap holds
  bool clearPin(int id);              // parent escape hatch (serial "P<n>" and Admin); false if save fails
  int createProfile(const char* name, uint8_t age, const char* code);   // code "1234", "" = none; lands on the launcher
  bool openApp(const char* name);     // from the launcher; matches App::name() ignoring case, spaces and dashes
  const char* screenName() const;
  void debugPrint();
  void debugCmd(const char* cmd);     // "tired" "bedtime" "rested" "younger" "older" "light" (the next level), then the open game

 private:
  enum Screen : uint8_t { SH_PICK, SH_NAME, SH_AVATAR, SH_AGE, SH_PIN_SET, SH_PIN_AGAIN, SH_PIN, SH_DELETE, SH_LAUNCHER, SH_REST,
                         SH_SYS_NEW, SH_SYS_INFO, SH_SYS_UPD, SH_SYS_WIFI, SH_SYS_PASS, SH_SYS_JOIN, SH_SYS_HOME, SH_SYS_ADDR, SH_SYS_FIND, SH_LIGHT,
                         SH_SYS_ADMIN, SH_SYS_PIN, SH_SYS_PIN_SET, SH_SYS_PIN_AGAIN, SH_SYS_RESET, SH_SYS_REMOVE,
                         SH_SYS_PLAYERS, SH_SYS_PLAYER_PIN,
                         SH_SYS_LIMITS, SH_SYS_CLOCK, SH_SYS_TIME, SH_SYS_WINDOW, SH_SYS_ALLOW, SH_SYS_ZONE, SH_SYS_ZONE_SEARCH,
                         SH_APP };   // SH_APP last: the tables in shell.cpp

  shell::Store* st_ = nullptr;
  shell::System* sys_ = nullptr;
  App* const* apps_ = nullptr; int nApps_ = 0;
  App* app_ = nullptr;                             // the open game, or the last one (its stats stay readable)
  bool opened_[MAX_APPS] = {};                     // games entered since boot: debugPrint shows their pets
  shell::Profiles prof_{};
  int active_ = -1;                                // the profile on the launcher / in a game
  shell::Light light_ = shell::LIGHT_AUTO;         // the device's brightness ("sys"/"lite")
  uint16_t adminPin_ = 0;                          // optional device-wide code, "sys"/"admin" (pinCode, 0 = off)
  uint8_t adminPage_ = 0;
  Screen afterAdmin_ = SH_SYS_INFO;                // where the admin code leads
  shell::Limits limits_{}, limitsDraft_{};
  shell::PlayWindow window_{}, windowDraft_{};
  shell::Allowance allowance_{};
  uint8_t limitEditor_ = 0;   // session or daily
  shell::TextEntry zoneQuery_{}, zoneSearch_{}; int zonePage_ = 0;
  int page_ = 0;                                   // the launcher's page of games: 0 for a new kid, else the last game's
  int target_ = -1; bool deleting_ = false;        // the picker row a PIN or a delete is about
  bool holdTop_ = false;                           // the delete button goes to the half the finger was not on
  bool creating_ = false;                          // the age screen is a creation step (else: a migrated profile)
  shell::Record draft_{};                          // the profile being created
  char name_[12] = {0}; int nameLen_ = 0; uint8_t namePage_ = 0;
  char pin_[5] = {0}; int pinLen_ = 0; uint16_t firstPin_ = 0; uint32_t pinWrongUntil_ = 0, pinMs_ = 0;   // pinMs_: the last digit
  uint32_t now_ = 0, ms_ = 0, screenMs_ = 0, lastTick_ = 0, lastSaveMs_ = 0, lastRecordSave_ = 0;
  uint32_t playTickMs_ = 0;                        // monotonic duration: network clock corrections are not extra play
  ActivityTracker activity_;                       // idle (shell::IDLE_MS without a touch or a move) is not play time
  Screen screen_ = SH_PICK;
  Input in_{};
  ui::FreshGate gate_;                             // every screen change ignores touches for a moment (os/ui.h)
  char toast_[40] = {0}; uint32_t toastUntil_ = 0;
  shell::SysInfo info_{}; shell::UpdStatus upd_{};   // the system menu's view of sys_, read when a menu screen opens
  shell::Network nets_[shell::MAX_NETWORKS] = {}; int nNets_ = 0;   // the last scan; -1 while it runs
  bool scanRefused_ = false; uint8_t netPage_ = 0;
  shell::Network pick_{}; shell::TextEntry pass_{}; int8_t join_ = 0;   // the network being joined (sysmenu.cpp)
  shell::TextEntry addr_{}; shell::Home found_{}; int8_t look_ = 0;    // the address being looked up (sysmenu.cpp)
  uint8_t blobs_[MAX_PROFILES][shell::BLOB_MAX];   // every profile's save for the game being opened
  uint8_t written_[shell::BLOB_MAX];               // the open game's save as last written (or loaded)
  size_t writtenLen_ = 0;
  bool writtenOk_ = true;                          // false: the last write failed; flushApp retries written_

  shell::Record& rec() { return prof_.rec[active_]; }
  bool restingNow() const;
  void enforceCurrentBudget();
  bool guardScreenTime();
  void go(Screen s);
  void land();   // the picker, or the first profile's name on a fresh device
  bool adultScreen() const { return (screen_ >= SH_SYS_INFO && screen_ <= SH_SYS_FIND) || (screen_ >= SH_SYS_ADMIN && screen_ <= SH_SYS_ZONE_SEARCH); }
  void toast(const char* s);
  void drawSysToast();
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
  void enterSystem();
  void askAdmin(Screen next); void adminPassed();   // next: SH_SYS_INFO, SH_NAME (New) or SH_DELETE
  void updateSysAdmin(); void drawSysAdmin();
  void updateSysPin(); void submitSysPin(); void drawSysPin();
  bool saveAdminPin(uint16_t code);
  void updateSysConfirm(); void drawSysConfirm();   // reset all limits, or remove the admin code
  void updateSysPlayers(); void drawSysPlayers();   // select whose profile PIN to remove
  void updateSysPlayerPin(); void drawSysPlayerPin();
  void updateSysLimits(); void drawSysLimits();
  void updateSysClock(); void drawSysClock();
  void updateSysTime(); void drawSysTime();
  void updateSysWindow(); void drawSysWindow();
  void updateSysAllowance(); void drawSysAllowance();
  void updateSysZone(); void drawSysZone();
  void updateSysZoneSearch(); void drawSysZoneSearch();
  void updateSysNew(); void drawSysNew();
  void updateSysInfo(); void drawSysInfo();
  void updateSysUpd(); void drawSysUpd(); void drawSysAction();
  void scan();
  void updateSysWifi(); void drawSysWifi(); void drawNetRow(int i, const shell::Network& n);
  void updateSysPass(); void drawSysPass(); void drawKeys(const shell::TextEntry& t);
  void join(); void pollJoin(); bool save(); void saved();
  void updateSysJoin(); void drawSysJoin();
  void updateSysHome(); void drawSysHome(); void updateSysAddr(); void drawSysAddr();
  void pollLook(); void updateSysFind(); void drawSysFind();
};
