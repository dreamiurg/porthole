// The device side the kid barely sees: the "something new" card after an update, and the grown-ups' system menu
// behind a 5 s hold on the picker's title (Info, then Updates and Wifi). Shell palette only (root rule 7), no sound.
// Adult screens render at native resolution with an anti-aliased reading font; logical touch coordinates stay
// unchanged. The "something new" card keeps the children's indexed art and typography. Design: docs/superpowers/specs/2026-10-17-wireless-updates-design.md.
#include <math.h>
#include <stdio.h>
#include <string.h>
#include <time.h>
#include "shell.h"
#include "adult_fonts.h"
#include "canvas.h"
#include "timezone.h"

using namespace gfx;
using namespace shell;

bool shell::newRun(Store& st, const char* version) {
  char ran[16] = {0}, cur[16];
  snprintf(cur, sizeof cur, "%s", version);   // stored and compared at 15 bytes: a longer one still matches next boot
  st.load("sys", "ran", ran, sizeof ran - 1);   // 0 (missing, or too long to be a version): never ran
  if (!strcmp(ran, cur)) return false;
  st.save("sys", "ran", cur, strlen(cur));   // only on a change: no flash write on every boot
  return strcmp(cur, "dev") != 0;           // a local build is never news
}

// ---------------------------------------------------------------- the picker's title, held SYS_HOLD_MS
// Long enough that no kid gets in by accident; the rows' long press (delete) is below it, so neither sees the other.
static const uint32_t SYS_HOLD_MS = 5000;
static const ui::Box TITLE = {44, 5, 72, 22};   // ends 2 px above the first row with four profiles (y 29)
bool Shell::titleHeld() {
  if (!sys_) return false;
  const ui::Box& t = TITLE;   // hit() first, finger up or not: the audit frame sees the target
  bool began = in_.downX >= t.x && in_.downX < t.x + t.w && in_.downY >= t.y && in_.downY < t.y + t.h;
  return in_.hit(t.x, t.y, t.w, t.h) && in_.down && began && in_.heldMs >= SYS_HOLD_MS;
}

// ---------------------------------------------------------------- "something new" (once, on the boot after an update)
static const ui::Box NEW_OK = {45, 112, 70, 26};
void Shell::updateSysNew() { if (in_.tapIn(NEW_OK.x, NEW_OK.y, NEW_OK.w, NEW_OK.h)) land(); }
static void sparkle(int x, int y, uint8_t c) { hline(x - 3, y, 7, c); line(x, y - 3, x, y + 3, c); }
void Shell::drawSysNew() {
  clear(C_SKY);
  sparkle(34, 34, C_YELLOW); sparkle(128, 34, C_WHITE); sparkle(30, 96, C_WHITE); sparkle(130, 94, C_YELLOW);
  textCentered(80, 42, "Porthole has", C_NAVY);
  textCentered(80, 56, "something", C_NAVY, 2);
  textCentered(80, 76, "new!", C_NAVY, 2);
  ui::drawButton({NEW_OK, "OK", nullptr, C_GREEN}, in_.down && in_.hit(NEW_OK.x, NEW_OK.y, NEW_OK.w, NEW_OK.h));
}

// ---------------------------------------------------------------- the menu's font and widgets
// Geometry remains logical (160 x 160) so drawing and touch targets share one layout. Only this adult renderer
// scales it to physical pixels; font glyphs are rasterized at their final size, never enlarged pixel art.
static uint16_t scol(uint8_t c) { return rgb888_to_565(PALETTE_RGB[c]); }
static void sclear(uint8_t c) { canvas::clip(canvas::FULL); gfx565::clear(scol(c)); }
static void srect(int x, int y, int w, int h, uint8_t c) { gfx565::rect(x * 3, y * 3, w * 3, h * 3, scol(c)); }
static void sframe(int x, int y, int w, int h, uint8_t c) {
  srect(x, y, w, 1, c); srect(x, y + h - 1, w, 1, c); srect(x, y, 1, h, c); srect(x + w - 1, y, 1, h, c);
}
static void scircle(int x, int y, int r, uint8_t c) { gfx565::circle(x * 3, y * 3, r * 3, scol(c)); }
static void sround(int x, int y, int w, int h, uint8_t c) { gfx565::roundRect({x * 3, y * 3, w * 3, h * 3}, 9, scol(c)); }
static void spanel(const ui::Box& b, uint8_t fill, uint8_t border) {
  sround(b.x, b.y, b.w, b.h, border); sround(b.x + 1, b.y + 1, b.w - 2, b.h - 2, fill);
}
static int stextW(const char* s) { return (font::textWidth(adult::FONT24, s) + 2) / 3; }
static void stext(int x, int y, const char* s, uint8_t c) { font::text(adult::FONT24, s, x * 3, y * 3, scol(c)); }
static void stextC(int cx, int y, const char* s, uint8_t c) {
  font::text(adult::FONT24, s, cx * 3 - font::textWidth(adult::FONT24, s) / 2, y * 3, scol(c));
}
static void shead(int cx, int y, const char* s, uint8_t c) {
  font::text(adult::FONT28, s, cx * 3 - font::textWidth(adult::FONT28, s) / 2, y * 3, scol(c));
}
static void sback() {
  scircle(80, 12, 11, C_DKBROWN); scircle(80, 11, 10, C_ORANGE);
  const int roof[] = {225, 31, 240, 17, 255, 31}; canvas::poly(roof, 3, scol(C_WHITE));
  gfx565::rect(229, 30, 22, 15, scol(C_WHITE)); gfx565::rect(237, 36, 7, 9, scol(C_ORANGE));
}
// s cut to room px, its last two letters ".." when cut: "HomeNe..".
static void fit(char* out, size_t n, const char* s, int room) {
  snprintf(out, n, "%s", s);
  for (int k = (int)strlen(out); k > 0 && stextW(out) > room;) {
    out[--k] = 0;
    if (k > 2) out[k - 1] = out[k - 2] = '.';
  }
}
static bool down(const Input& in, const ui::Box& b) { return in.down && in.hit(b.x, b.y, b.w, b.h); }
static bool tapped(const Input& in, const ui::Box& b) { return in.tapIn(b.x, b.y, b.w, b.h); }
// ui::drawButton's look in the menu's font. A keyboard key is the same with a 1 px shadow, yellow while pressed.
static void sbutton(const ui::Box& b, const char* label, uint8_t col, bool pressed, int shadow = 2) {
  const int dy = pressed ? 1 : 0;
  if (!pressed) sround(b.x, b.y + shadow, b.w, b.h, C_DKBROWN);
  sround(b.x, b.y + dy, b.w, b.h, C_DKBROWN);
  const uint8_t fill = pressed && shadow == 1 ? (uint8_t)C_YELLOW : col;
  sround(b.x + 1, b.y + 1 + dy, b.w - 2, b.h - 2, fill);
  if (shadow == 2) srect(b.x + 2, b.y + 1 + dy, b.w - 4, 1, C_WHITE);
  stextC(b.x + b.w / 2, b.y + dy + (b.h - 8) / 2, label, ui::inkOn(fill));
}

// ---------------------------------------------------------------- Info: label | value rows on navy
static const int ROW = 9, COL = 76;   // labels end at COL, values start 5 px after it
// Widest a value starting at x may be on the row at y, inside a 78 px circle (the audit clips at 79).
static int valueRoom(int x, int y) {
  int d = y < 80 ? 80 - y : y + 8 - 80;
  return (int)sqrtf((float)(78 * 78 - d * d)) + 80 - x;
}
static void row(int y, const char* label, const char* value) {
  stext(COL - stextW(label), y, label, C_LTGRAY);
  char v[40]; fit(v, sizeof v, value, valueRoom(COL + 5, y));   // a long SSID: "HomeNe.."
  stext(COL + 5, y, v, C_WHITE);
}
static void ago(char* out, size_t n, uint32_t now, uint32_t then) {
  uint32_t d = now > then ? now - then : 0;
  if (!then) snprintf(out, n, "never");
  else if (d < 60) snprintf(out, n, "just now");
  else if (d < 3600) snprintf(out, n, "%lu min ago", (unsigned long)(d / 60));
  else if (d < 86400) snprintf(out, n, "%lu h ago", (unsigned long)(d / 3600));
  else snprintf(out, n, "%lu days ago", (unsigned long)(d / 86400));
}
// The status is RAM (IDLE after every boot) but lastCheck is persisted: an IDLE with a lastCheck tried before this boot.
static void wifiResult(char* out, size_t n, const UpdStatus& u, uint32_t now) {
  if (u.state == UP_IDLE && u.lastCheck) ago(out, n, now, u.lastCheck);
  else snprintf(out, n, "%s", u.state == UP_IDLE ? "not yet" : u.state == UP_NO_WIFI ? "no wifi" : u.state == UP_CHECKING ? "trying" : "ok");
}

static const ui::Box WIFI_BTN = {22, 98, 56, 20}, UPD_BTN = {82, 98, 56, 20};
static const ui::Box HOME_BTN = {30, 122, 48, 20}, ADMIN_BTN = {82, 122, 48, 20};
void Shell::updateSysInfo() {
  upd_ = sys_->status();
  if (ui::back(in_)) { go(SH_PICK); return; }
  if (tapped(in_, ADMIN_BTN)) { adminPage_ = 0; go(SH_SYS_ADMIN); return; }
  if (tapped(in_, UPD_BTN)) { go(SH_SYS_UPD); return; }
  if (sys_->hasHome() && tapped(in_, HOME_BTN)) { go(SH_SYS_HOME); return; }
  if (tapped(in_, WIFI_BTN)) { netPage_ = 0; scan(); go(SH_SYS_WIFI); }
}
void Shell::drawSysInfo() {
  sclear(C_NAVY);
  char heap[16], tried[24]; snprintf(heap, sizeof heap, "%lu KB", (unsigned long)(info_.heapFree / 1024));
  wifiResult(tried, sizeof tried, upd_, now_);
  int y = 27;
  row(y, "Version", info_.version); y += ROW;
  row(y, "Slot", info_.slot); y += ROW;
  row(y, "Reset", info_.reset); y += ROW;
  row(y, "Free heap", heap); y += ROW;
  row(y, "SD card", info_.sd ? "yes" : "no"); y += ROW;
  row(y, "Wifi", info_.wifiSaved ? info_.ssid : "none"); y += ROW;
  row(y, upd_.state == UP_IDLE && upd_.lastCheck ? "Checked" : "Last try", tried);   // before this boot: a time, no result
  sbutton(WIFI_BTN, "Wifi", C_SKY, down(in_, WIFI_BTN));
  sbutton(UPD_BTN, "Updates", C_SKY, down(in_, UPD_BTN));
  if (sys_->hasHome()) sbutton(HOME_BTN, "Home", C_SKY, down(in_, HOME_BTN));
  sbutton(ADMIN_BTN, "Admin", C_SKY, down(in_, ADMIN_BTN));
  sback();
}

// ---------------------------------------------------------------- optional admin code and everyone's play limits
// A separate key: existing devices stay unlocked, and no shipped profile/save layout changes. Leaving Info for
// the picker ends access; every new title hold asks again. The post-update news card never grants menu access.
void Shell::enterSystem() { askAdmin(SH_SYS_INFO); }
// The system menu and the picker's New and delete: behind the admin code when one is set, so a new profile or a
// deleted one cannot dodge the limits.
void Shell::askAdmin(Screen next) {
  resetPin(); firstPin_ = 0; afterAdmin_ = next;
  if (adminPin_) go(SH_SYS_PIN); else adminPassed();
}
void Shell::adminPassed() {
  if (afterAdmin_ == SH_NAME) startNew();
  else if (afterAdmin_ == SH_DELETE) authorized();
  else go(SH_SYS_INFO);
}
static const ui::Box ADMIN_CODE = {30, 60, 100, 20}, ADMIN_RESET = {30, 84, 100, 20}, ADMIN_REMOVE = {30, 108, 100, 20};
static const ui::Box ADMIN_PLAYERS = {42, 132, 76, 18};
static ui::Box adminPageArrow(bool next) { return {next ? 114 : 26, 26, 20, 18}; }
void Shell::updateSysAdmin() {
  if (ui::back(in_)) { go(SH_SYS_INFO); return; }
  if (tapped(in_, adminPageArrow(!adminPage_))) { adminPage_ = !adminPage_; return; }
  if (adminPage_) {
    if (tapped(in_, ADMIN_CODE)) go(SH_SYS_CLOCK);
    if (tapped(in_, ADMIN_RESET)) saveSound(!sound_);   // a failed write leaves the switch as it was
    return;
  }
  if (tapped(in_, ADMIN_CODE)) { resetPin(); firstPin_ = 0; go(SH_SYS_PIN_SET); return; }
  if (tapped(in_, ADMIN_RESET)) { go(SH_SYS_TIME); return; }
  if (tapped(in_, ADMIN_PLAYERS)) { target_ = -1; go(SH_SYS_PLAYERS); return; }
  if (adminPin_ && tapped(in_, ADMIN_REMOVE)) go(SH_SYS_REMOVE);
}
void Shell::drawSysAdmin() {
  sclear(C_NAVY);
  shead(80, 33, adminPage_ ? "Admin 2/2" : "Admin 1/2", C_WHITE);
  const ui::Box arrow = adminPageArrow(!adminPage_);
  sbutton(arrow, adminPage_ ? "<" : ">", C_SKY, down(in_, arrow));
  if (adminPage_) {
    stextC(80, 46, "Device settings", C_LTGRAY);
    sbutton(ADMIN_CODE, "Clock", C_SKY, down(in_, ADMIN_CODE));
    sbutton(ADMIN_RESET, sound_ ? "Sound: on" : "Sound: off", sound_ ? C_SKY : C_LTGRAY, down(in_, ADMIN_RESET));
    sback(); return;
  }
  stextC(80, 46, adminPin_ ? "Menu code: on" : "Menu code: off", C_LTGRAY);
  sbutton(ADMIN_CODE, adminPin_ ? "Change code" : "Set admin code", C_SKY, down(in_, ADMIN_CODE));
  sbutton(ADMIN_RESET, "Screen time", C_SKY, down(in_, ADMIN_RESET));
  if (adminPin_) sbutton(ADMIN_REMOVE, "Remove code", C_LTGRAY, down(in_, ADMIN_REMOVE));
  sbutton(ADMIN_PLAYERS, "Player PINs", C_SKY, down(in_, ADMIN_PLAYERS));
  sback();
}
// Read back, like the code below: a failed flash write must not claim the sound changed.
bool Shell::saveSound(bool on) {
  const uint8_t v = on ? 1 : 0;
  st_->save("sys", "sound", &v, 1);
  uint8_t saved = 2;
  if (st_->load("sys", "sound", &saved, 1) != 1 || saved != v) return false;
  sound_ = on;
  return true;
}
// A failed flash write must not claim the code changed.
bool Shell::saveAdminPin(uint16_t code) {
  if (!st_->save("sys", "admin", &code, sizeof code)) return false;
  adminPin_ = code;
  return true;
}
// Adult keypad: 0-9 and erase, four masked digits, automatic submission after the last digit has shown briefly.
// Kept separate from the kids' nine big keys so an admin code can include zero without changing their flow.
static ui::Box adminKey(int k) { return k == 9 ? ui::Box{65, 117, 28, 19} : ui::Box{35 + k % 3 * 30, 54 + k / 3 * 21, 28, 19}; }
static const ui::Box ADMIN_ERASE = {35, 117, 28, 19};
void Shell::updateSysPin() {
  if (ui::back(in_)) { resetPin(); firstPin_ = 0; go(screen_ == SH_SYS_PIN ? SH_PICK : SH_SYS_ADMIN); return; }
  if (pinLen_ == 4) {   // a key pressed while the fourth dot shows is the next screen's first (the repeat): never lost
    const char next = adminDigit();
    if (!next && ms_ - pinMs_ < 400) return;
    submitSysPin();
    if (next && screen_ >= SH_SYS_PIN && screen_ <= SH_SYS_PIN_AGAIN && !adminKeysHeld()) typeDigit(next);
    return;
  }
  if (adminKeysHeld()) return;
  if (ui::clicked(in_, ADMIN_ERASE)) { if (pinLen_) pin_[--pinLen_] = 0; return; }
  if (const char d = adminDigit()) typeDigit(d);
}
// A wrong code's shake, or too many wrong tries (pinlock.h), holds the keys.
bool Shell::adminKeysHeld() { return ms_ < pinWrongUntil_ || (screen_ == SH_SYS_PIN && pinWait(shell::ADMIN_LOCK)); }
char Shell::adminDigit() const {   // on release, however long the press
  for (int k = 0; k < 10; k++) if (ui::clicked(in_, adminKey(k))) return k == 9 ? '0' : (char)('1' + k);
  return 0;
}
void Shell::submitSysPin() {
  const uint16_t code = pinCode(pin_); resetPin();
  if (screen_ == SH_SYS_PIN_SET) { firstPin_ = code; go(SH_SYS_PIN_AGAIN); return; }
  if (screen_ == SH_SYS_PIN) {
    if (code == adminPin_) { pinClear(shell::ADMIN_LOCK); adminPassed(); return; }
    pinStrike(shell::ADMIN_LOCK); pinWrongUntil_ = ms_ + 900; toast(pinWait(shell::ADMIN_LOCK) ? "Wrong code" : "Wrong code. Try again."); return;
  }
  if (code != firstPin_) { firstPin_ = 0; go(SH_SYS_PIN_SET); toast("Codes did not match"); return; }
  const bool ok = saveAdminPin(code); firstPin_ = 0;
  go(SH_SYS_ADMIN); toast(ok ? "Admin code saved" : "Could not save code");
}
void Shell::drawSysPin() {
  sclear(C_NAVY);
  const char* prompt = screen_ == SH_SYS_PIN ? "Admin code?" : screen_ == SH_SYS_PIN_SET ? "Choose 4 digits" : "Repeat admin code";
  const uint32_t wait = screen_ == SH_SYS_PIN ? pinWait(shell::ADMIN_LOCK) : 0;
  shead(80, 32, ms_ < toastUntil_ ? toast_ : wait ? "Too many tries" : prompt, C_WHITE);
  for (int i = 0; i < 4; i++) {
    const int x = 58 + i * 12;
    sframe(x, 43, 10, 9, ms_ < pinWrongUntil_ ? (uint8_t)C_YELLOW : (uint8_t)C_WHITE);
    if (i < pinLen_) scircle(x + 5, 47, 2, C_WHITE);
  }
  if (wait) {   // the keys come back when the wait is over
    char line[32]; snprintf(line, sizeof line, "Try again in %u s", (unsigned)wait);
    stextC(80, 80, line, C_WHITE); sback(); return;
  }
  for (int k = 0; k < 10; k++) {
    const ui::Box b = adminKey(k); char digit[2] = {k == 9 ? '0' : (char)('1' + k), 0};
    sbutton(b, "", C_WHITE, down(in_, b), 1);
    font::text(adult::FONT32, digit, (b.x + b.w / 2) * 3 - font::textWidth(adult::FONT32, digit) / 2, b.y * 3 + (b.h * 3 - adult::FONT32.lineHeight) / 2 + (down(in_, b) ? 3 : 0), scol(C_DKBROWN));
  }
  sbutton(ADMIN_ERASE, "Del", C_SKY, down(in_, ADMIN_ERASE), 1);
  sback();
}
static const ui::Box CONFIRM_NO = {30, 84, 100, 20}, CONFIRM_YES = {30, 108, 100, 20};
void Shell::updateSysConfirm() {
  if (ui::back(in_) || tapped(in_, CONFIRM_NO)) { go(screen_ == SH_SYS_REMOVE ? SH_SYS_ADMIN : SH_SYS_TIME); return; }
  if (!tapped(in_, CONFIRM_YES)) return;
  if (screen_ == SH_SYS_REMOVE) {
    const bool ok = saveAdminPin(0);
    go(SH_SYS_ADMIN); toast(ok ? "Admin code removed" : "Could not remove code");
  } else {
    const bool ok = resetLimits(*st_, prof_);
    go(SH_SYS_TIME); toast(ok ? "Usage reset" : "Reset failed. Retry");
  }
}
void Shell::drawSysConfirm() {
  sclear(C_NAVY);
  const bool remove = screen_ == SH_SYS_REMOVE;
  shead(80, 34, remove ? "Remove admin code?" : "Reset everyone's usage?", C_WHITE);
  stextC(80, 49, remove ? "The system menu will open" : "Clear daily play and rest", C_LTGRAY);
  stextC(80, 60, remove ? "without a code." : "timers for every profile.", C_LTGRAY);
  if (!remove) stextC(80, 71, "Game progress is kept.", C_LTGRAY);
  sbutton(CONFIRM_NO, "Cancel", C_GREEN, down(in_, CONFIRM_NO));
  sbutton(CONFIRM_YES, remove ? "Remove code" : "Reset usage", C_SKY, down(in_, CONFIRM_YES));
  sback();
}

// ---------------------------------------------------------------- recover a selected player's forgotten PIN
// Rows map through nth(): a deleted profile leaves a stable-id hole. No profile PIN is asked here; the system
// menu already checked the adult's code. The selected player's details, budgets and games all stay intact.
static ui::Box playerRow(int i) { return {30, 46 + i * 22, 100, 20}; }
void Shell::updateSysPlayers() {
  if (ui::back(in_)) { go(SH_SYS_ADMIN); return; }
  for (int i = 0; i < prof_.count(); i++) {
    if (!tapped(in_, playerRow(i))) continue;
    target_ = prof_.nth(i);
    if (!prof_.rec[target_].pin) { toast("No PIN set"); return; }
    go(SH_SYS_PLAYER_PIN); return;
  }
}
void Shell::drawSysPlayers() {
  sclear(C_NAVY);
  shead(80, 33, "Reset a player's PIN", C_WHITE);
  if (!prof_.count()) stextC(80, 70, "No players yet", C_LTGRAY);
  for (int i = 0; i < prof_.count(); i++) {
    const Record& r = prof_.rec[prof_.nth(i)]; const ui::Box b = playerRow(i);
    sbutton(b, "", C_SKY, down(in_, b));
    char name[12]; fit(name, sizeof name, r.name, 54);
    const int dy = down(in_, b) ? 1 : 0;
    stext(b.x + 6, b.y + 6 + dy, name, C_DKBROWN);
    stext(b.x + 66, b.y + 6 + dy, r.pin ? "PIN" : "No PIN", C_DKBROWN);
  }
  sback();
}
void Shell::updateSysPlayerPin() {
  if (ui::back(in_) || tapped(in_, CONFIRM_NO)) { go(SH_SYS_PLAYERS); return; }
  if (!tapped(in_, CONFIRM_YES)) return;
  if (!clearPin(target_)) { toast("Could not reset PIN"); return; }
  go(SH_SYS_PLAYERS); toast("Player PIN reset");
}
void Shell::drawSysPlayerPin() {
  sclear(C_NAVY);
  shead(80, 34, "Reset player PIN?", C_WHITE);
  stextC(80, 48, prof_.rec[target_].name, C_YELLOW);
  stextC(80, 61, "Their next login will", C_LTGRAY);
  stextC(80, 72, "not need a PIN.", C_LTGRAY);
  sbutton(CONFIRM_NO, "Cancel", C_GREEN, down(in_, CONFIRM_NO));
  sbutton(CONFIRM_YES, "Reset PIN", C_SKY, down(in_, CONFIRM_YES));
  sback();
}

static bool typeInto(const Input& in, TextEntry& t);

// ---------------------------------------------------------------- Screen time: one home for limits, play hours and deliberate exceptions
static const ui::Box TIME_ROWS[3] = {{30, 42, 100, 22}, {30, 67, 100, 22}, {30, 92, 100, 22}};
static const ui::Box TIME_RESET = {26, 119, 52, 20}, TIME_ALLOW = {82, 119, 52, 20};
static void friendlyTime(char* out, size_t n, int minute) {
  const int hour = minute / 60;
  if (minute % 60) snprintf(out, n, "%d:%02d %s", hour % 12 ? hour % 12 : 12, minute % 60, hour < 12 ? "AM" : "PM");
  else snprintf(out, n, "%d %s", hour % 12 ? hour % 12 : 12, hour < 12 ? "AM" : "PM");
}
static void settingRow(const ui::Box& b, const char* label, const char* value, bool pressed) {
  sbutton(b, "", C_SKY, pressed);
  const int dy = pressed ? 1 : 0;
  stextC(80, b.y + 3 + dy, label, C_DKBROWN); stextC(80, b.y + 12 + dy, value, C_DKBROWN);
}
void Shell::updateSysTime() {
  if (ui::back(in_)) { go(SH_SYS_ADMIN); return; }
  for (int i = 0; i < 2; i++) if (tapped(in_, TIME_ROWS[i])) {
    limitEditor_ = (uint8_t)i; limitsDraft_ = limits_; go(SH_SYS_LIMITS); return;
  }
  if (tapped(in_, TIME_ROWS[2])) { windowDraft_ = window_; go(SH_SYS_WINDOW); return; }
  if (tapped(in_, TIME_RESET)) { go(SH_SYS_RESET); return; }
  if (!tapped(in_, TIME_ALLOW)) return;
  if (allowance_.active(ms_)) { allowance_.stop(); toast("Extra time ended"); }
  else go(SH_SYS_ALLOW);
}
void Shell::drawSysTime() {
  sclear(C_NAVY); shead(80, 28, "Screen time", C_WHITE);
  char value[40]; snprintf(value, sizeof value, "%u min", (unsigned)limits_.sessionMinutes);
  settingRow(TIME_ROWS[0], "Session", value, down(in_, TIME_ROWS[0]));
  snprintf(value, sizeof value, "%u min", (unsigned)limits_.dailyMinutes);
  settingRow(TIME_ROWS[1], "Daily total", value, down(in_, TIME_ROWS[1]));
  if (window_.enabled) {
    char start[16], end[16]; friendlyTime(start, sizeof start, window_.startMinute); friendlyTime(end, sizeof end, window_.endMinute);
    snprintf(value, sizeof value, "%s to %s", start, end);
  } else snprintf(value, sizeof value, "Any time");
  settingRow(TIME_ROWS[2], "Play window", value, down(in_, TIME_ROWS[2]));
  sbutton(TIME_RESET, "Reset usage", C_SKY, down(in_, TIME_RESET));
  if (allowance_.active(ms_)) snprintf(value, sizeof value, "End %lu min", (unsigned long)allowance_.minutesLeft(ms_));
  else snprintf(value, sizeof value, "Allow 30 min");
  sbutton(TIME_ALLOW, value, C_GREEN, down(in_, TIME_ALLOW)); sback();
}
static const ui::Box EDIT_MINUS = {26, 68, 24, 24}, EDIT_PLUS = {110, 68, 24, 24};
static const ui::Box EDIT_CANCEL = {26, 110, 52, 22}, EDIT_SAVE = {82, 110, 52, 22};
static int limitStep(const Input& in, const ui::Box& box, int value, int delta) {
  const bool tap = tapped(in, box), held = in.longPress && in.hit(box.x, box.y, box.w, box.h);
  if (!tap && !held) return value;
  int next = value + delta * (held ? 10 : 1);
  return next < 1 ? 1 : next > 1440 ? 1440 : next;
}
void Shell::updateSysLimits() {
  if (ui::back(in_) || tapped(in_, EDIT_CANCEL)) { go(SH_SYS_TIME); return; }
  uint16_t& value = limitEditor_ ? limitsDraft_.dailyMinutes : limitsDraft_.sessionMinutes;
  value = (uint16_t)limitStep(in_, EDIT_MINUS, value, -1);
  value = (uint16_t)limitStep(in_, EDIT_PLUS, value, 1);
  if (!tapped(in_, EDIT_SAVE)) return;
  if (!saveLimits(*st_, limitsDraft_)) { toast("Could not save limit"); return; }
  limits_ = limitsDraft_; go(SH_SYS_TIME); toast("Limit saved");
}
void Shell::drawSysLimits() {
  sclear(C_NAVY); shead(80, 32, limitEditor_ ? "Daily total" : "Session limit", C_WHITE);
  stextC(80, 49, limitEditor_ ? "Minutes per day" : "Minutes per session", C_LTGRAY);
  sbutton(EDIT_MINUS, "-", C_SKY, down(in_, EDIT_MINUS)); sbutton(EDIT_PLUS, "+", C_SKY, down(in_, EDIT_PLUS));
  char value[16]; snprintf(value, sizeof value, "%u min", (unsigned)(limitEditor_ ? limitsDraft_.dailyMinutes : limitsDraft_.sessionMinutes));
  shead(80, 76, value, C_WHITE);
  sbutton(EDIT_CANCEL, "Cancel", C_LTGRAY, down(in_, EDIT_CANCEL)); sbutton(EDIT_SAVE, "Save", C_GREEN, down(in_, EDIT_SAVE));
  stextC(80, 141, "Hold +/- for 10", C_LTGRAY); sback();
}
static const ui::Box WINDOW_TOGGLE = {42, 40, 76, 18}, WINDOW_CANCEL = {37, 125, 40, 20}, WINDOW_SAVE = {83, 125, 40, 20};
static ui::Box windowStep(int row, bool plus) { return {plus ? 110 : 26, 70 + row * 32, 24, 20}; }
static uint16_t stepTime(const Input& in, const ui::Box& b, uint16_t value, int sign) {
  if (tapped(in, b)) return (uint16_t)((value + 1440 + sign * 15) % 1440);
  if (in.longPress && in.hit(b.x, b.y, b.w, b.h)) return (uint16_t)((value + 1440 + sign * 60) % 1440);
  return value;
}
void Shell::updateSysWindow() {
  if (ui::back(in_) || tapped(in_, WINDOW_CANCEL)) { go(SH_SYS_TIME); return; }
  if (tapped(in_, WINDOW_TOGGLE)) windowDraft_.enabled = !windowDraft_.enabled;
  uint16_t* values[2] = {&windowDraft_.startMinute, &windowDraft_.endMinute};
  for (int i = 0; i < 2; i++) {
    *values[i] = stepTime(in_, windowStep(i, false), *values[i], -1);
    *values[i] = stepTime(in_, windowStep(i, true), *values[i], 1);
  }
  if (!tapped(in_, WINDOW_SAVE)) return;
  if (windowDraft_.startMinute == windowDraft_.endMinute) { toast("Choose different times"); return; }
  if (!savePlayWindow(*st_, windowDraft_)) { toast("Could not save hours"); return; }
  window_ = windowDraft_; go(SH_SYS_TIME); toast("Play window saved");
}
void Shell::drawSysWindow() {
  sclear(C_NAVY); shead(80, 28, "Play window", C_WHITE);
  sbutton(WINDOW_TOGGLE, windowDraft_.enabled ? "Enabled" : "Off: any time", C_SKY, down(in_, WINDOW_TOGGLE));
  for (int i = 0; i < 2; i++) {
    stextC(80, 61 + i * 32, i ? "Until" : "From", C_LTGRAY);
    const ui::Box minus = windowStep(i, false), plus = windowStep(i, true);
    sbutton(minus, "-", C_SKY, down(in_, minus)); sbutton(plus, "+", C_SKY, down(in_, plus));
    char time[16]; friendlyTime(time, sizeof time, i ? windowDraft_.endMinute : windowDraft_.startMinute);
    stextC(80, minus.y + 6, time, C_WHITE);
  }
  sbutton(WINDOW_CANCEL, "Cancel", C_LTGRAY, down(in_, WINDOW_CANCEL)); sbutton(WINDOW_SAVE, "Save", C_GREEN, down(in_, WINDOW_SAVE)); sback();
}
void Shell::updateSysAllowance() {
  if (ui::back(in_) || tapped(in_, CONFIRM_NO)) { go(SH_SYS_TIME); return; }
  if (!tapped(in_, CONFIRM_YES)) return;
  allowance_.start(ms_); go(SH_SYS_TIME); toast("Extra time: 30 minutes");
}
void Shell::drawSysAllowance() {
  sclear(C_NAVY); shead(80, 34, "Allow 30 minutes?", C_WHITE);
  stextC(80, 49, "Ignore limits and play hours", C_LTGRAY);
  stextC(80, 60, "for everyone, temporarily.", C_LTGRAY);
  stextC(80, 71, "Usage still counts.", C_LTGRAY);
  sbutton(CONFIRM_NO, "Cancel", C_LTGRAY, down(in_, CONFIRM_NO));
  sbutton(CONFIRM_YES, "Allow 30 min", C_GREEN, down(in_, CONFIRM_YES)); sback();
}

// ---------------------------------------------------------------- Clock: local IANA zone, automatic detection and ordinary clock sync
static const ui::Box CLOCK_ZONE = {30, 77, 100, 20}, CLOCK_AUTO = {30, 101, 100, 20};
static const ui::Box CLOCK_DETECT = {37, 125, 40, 20}, CLOCK_SYNC = {83, 125, 40, 20};
void Shell::updateSysClock() {
  if (ui::back(in_)) { go(SH_SYS_ADMIN); return; }
  const ClockStatus clock = sys_->clockStatus();
  if (tapped(in_, CLOCK_ZONE)) { zonePage_ = 0; zoneQuery_ = TextEntry{}; go(SH_SYS_ZONE); return; }
  if (tapped(in_, CLOCK_AUTO)) {
    toast(sys_->setTimeZone(clock.zone, !clock.automaticZone) ? "Time zone mode saved" : "Could not save mode"); return;
  }
  if (tapped(in_, CLOCK_DETECT)) {
    if (!clock.automaticZone && !sys_->setTimeZone(clock.zone, true)) { toast("Could not enable auto"); return; }
    if (clock.zoneState != ZONE_LOOKING && !sys_->detectTimeZone()) toast("Wifi busy. Try again");
  }
  if (clock.state != TIME_SYNCING && tapped(in_, CLOCK_SYNC) && !sys_->syncClock()) toast("Wifi busy. Try again");
}
void Shell::drawSysClock() {
  sclear(C_NAVY); shead(80, 28, "Clock", C_WHITE);
  const time_t epoch = now_; struct tm date{}; gmtime_r(&epoch, &date);
  char current[32]; strftime(current, sizeof current, "%b %d, %I:%M %p", &date); stextC(80, 43, current, C_WHITE);
  const ClockStatus clock = sys_->clockStatus();
  static const char* const STATES[] = {"Using device clock", "Syncing with wifi...", "Wifi time synced", "No wifi connection", "Time sync failed"};
  const char* status = clock.zoneState == ZONE_LOOKING ? "Detecting time zone..." : clock.zoneState == ZONE_FAILED ? "Zone lookup failed; saved zone" : STATES[clock.state];
  stextC(80, 55, status, clock.state == TIME_OK && clock.zoneState != ZONE_FAILED ? (uint8_t)C_GREEN : (uint8_t)C_YELLOW);
  stextC(80, 67, "Time zone", C_LTGRAY);
  const timezones::Zone* zone = timezones::find(clock.zone);
  char label[48]; fit(label, sizeof label, zone ? zone->label : clock.zone, 92);
  sbutton(CLOCK_ZONE, label, C_SKY, down(in_, CLOCK_ZONE));
  sbutton(CLOCK_AUTO, clock.automaticZone ? "Automatic: on" : "Automatic: off", C_SKY, down(in_, CLOCK_AUTO));
  if (clock.zoneState != ZONE_LOOKING) sbutton(CLOCK_DETECT, "Detect", C_SKY, down(in_, CLOCK_DETECT));
  if (clock.state != TIME_SYNCING) sbutton(CLOCK_SYNC, "Sync", C_SKY, down(in_, CLOCK_SYNC));
  sback();
}
static const char* const COMMON_ZONES[] = {"America/Los_Angeles", "America/Denver", "America/Chicago", "America/New_York", "Europe/London", "Etc/UTC"};
static const ui::Box ZONE_COMMON = {30, 125, 48, 20}, ZONE_SEARCH = {82, 125, 48, 20};
static ui::Box zoneRow(int i) { return {30, 45 + i * 24, 100, 20}; }
static const ui::Box ZONE_PREV = {26, 24, 20, 18}, ZONE_NEXT = {114, 24, 20, 18};
static int zoneCount(const TextEntry& query) {
  if (!query.len) return (int)(sizeof COMMON_ZONES / sizeof COMMON_ZONES[0]);
  int count = 0; for (int i = 0; i < timezones::count(); i++) count += timezones::matches(i, query.buf);
  return count;
}
static const timezones::Zone* zoneAt(const TextEntry& query, int match) {
  if (!query.len) return timezones::find(COMMON_ZONES[match]);
  for (int i = 0; i < timezones::count(); i++) if (timezones::matches(i, query.buf) && match-- == 0) return &timezones::at(i);
  return nullptr;
}
void Shell::updateSysZone() {
  if (ui::back(in_)) { go(SH_SYS_CLOCK); return; }
  if (tapped(in_, ZONE_COMMON)) { zoneQuery_ = TextEntry{}; zonePage_ = 0; return; }
  if (tapped(in_, ZONE_SEARCH)) { zoneSearch_ = zoneQuery_; go(SH_SYS_ZONE_SEARCH); return; }
  const int count = zoneCount(zoneQuery_), pages = (count + 2) / 3;
  if (zonePage_ > 0 && tapped(in_, ZONE_PREV)) zonePage_--;
  if (zonePage_ + 1 < pages && tapped(in_, ZONE_NEXT)) zonePage_++;
  for (int i = 0; i < 3 && zonePage_ * 3 + i < count; i++) if (tapped(in_, zoneRow(i))) {
    const timezones::Zone* zone = zoneAt(zoneQuery_, zonePage_ * 3 + i);
    if (!zone || !sys_->setTimeZone(zone->id, false)) { toast("Could not save zone"); return; }
    go(SH_SYS_CLOCK); toast("Time zone saved"); return;
  }
}
void Shell::drawSysZone() {
  sclear(C_NAVY); shead(80, 29, "Time zone", C_WHITE);
  const int count = zoneCount(zoneQuery_), pages = (count + 2) / 3;
  for (int i = 0; i < 3 && zonePage_ * 3 + i < count; i++) {
    const timezones::Zone* zone = zoneAt(zoneQuery_, zonePage_ * 3 + i); char label[48];
    fit(label, sizeof label, zone ? zone->label : "Unknown", 92);
    sbutton(zoneRow(i), label, C_SKY, down(in_, zoneRow(i)));
  }
  if (!count) { stextC(80, 65, "No matching zones", C_WHITE); stextC(80, 79, "Try a city or IANA name", C_LTGRAY); }
  if (zonePage_ > 0) sbutton(ZONE_PREV, "<", C_SKY, down(in_, ZONE_PREV));
  if (zonePage_ + 1 < pages) sbutton(ZONE_NEXT, ">", C_SKY, down(in_, ZONE_NEXT));
  char page[32]; snprintf(page, sizeof page, "%d / %d", count ? zonePage_ + 1 : 0, pages); stextC(80, 115, page, C_LTGRAY);
  sbutton(ZONE_COMMON, "Common", C_LTGRAY, down(in_, ZONE_COMMON)); sbutton(ZONE_SEARCH, "Search", C_SKY, down(in_, ZONE_SEARCH)); sback();
}
void Shell::updateSysZoneSearch() {
  if (ui::back(in_)) { go(SH_SYS_ZONE); return; }
  if (typeInto(in_, zoneSearch_)) { zoneQuery_ = zoneSearch_; zonePage_ = 0; go(SH_SYS_ZONE); }
}
void Shell::drawSysZoneSearch() {
  sclear(C_NAVY); spanel({28, 25, 104, 11}, C_WHITE, C_DKBROWN);
  const char* tail = zoneSearch_.buf; while (stextW(tail) > 98) tail++;
  stextC(80, 27, *tail ? tail : "City or IANA zone", *tail ? (uint8_t)C_NAVY : (uint8_t)C_DKGRAY);
  drawKeys(zoneSearch_); sback();
}

// ---------------------------------------------------------------- Updates: channel, versions, Check now / Restart now
static ui::Box chanSeg(int c) { return {28 + c * 54, 30, 50, 20}; }
static const ui::Box SYS_BTN = {40, 98, 80, 20};   // Check now, Restart now
static const char* const CHAN_NAMES[2] = {"Stable", "Beta"};
static bool busy(UpdState s) { return s == UP_CHECKING || s == UP_DOWNLOADING; }   // no button: it is already on it
// The channel stays put mid-check and with an image waiting: switching then would show one channel's result as another's.
static bool chanLocked(UpdState s) { return busy(s) || s == UP_READY; }
// A scan or a test join holds the radio: Check now would do nothing until it is done.
static bool radioBusy(System& s) {
  Home found{};
  return s.scanResult(nullptr, 0) < 0 || s.testResult() < 0 || s.lookupResult(found) < 0 || s.clockStatus().state == TIME_SYNCING || s.clockStatus().zoneState == ZONE_LOOKING;
}
void Shell::updateSysUpd() {
  upd_ = sys_->status();
  if (ui::back(in_)) { go(SH_SYS_INFO); return; }
  for (int c = 0; c < 2 && !chanLocked(upd_.state); c++)
    if (tapped(in_, chanSeg(c)) && c != sys_->channel()) sys_->setChannel((Channel)c);
  const bool ready = upd_.state == UP_READY;   // Restart now needs no radio; Check now waits for a scan or test to end
  if (busy(upd_.state) || (!ready && radioBusy(*sys_)) || !tapped(in_, SYS_BTN)) return;
  if (ready) sys_->restart(); else sys_->checkNow();
  upd_ = sys_->status();
}
// The status line. Two states read differently than their names: IDLE after a boot with a check on record (the daily
// check comes on its own), and CURRENT on a dev build (newer() never offers it anything, so it is not "up to date").
static void statusLine(char* out, size_t n, const UpdStatus& u, const char* running) {
  static const char* const STATE[] = {"Not checked yet", "No wifi", "Checking...", "Up to date", "", "New version ready", "Check failed"};
  static_assert(sizeof STATE / sizeof STATE[0] == UP_COUNT, "one label per UpdState");
  int v[3];
  if (u.state == UP_DOWNLOADING) snprintf(out, n, "Downloading %u%%", (unsigned)u.pct);
  else if ((u.state == UP_IDLE || u.state == UP_CURRENT) && !parseVersion(running, v)) snprintf(out, n, "Dev build: no updates");
  else if (u.state == UP_IDLE && u.lastCheck) snprintf(out, n, "Checks once a day");   // a dev build never does
  else snprintf(out, n, "%s", (size_t)u.state < sizeof STATE / sizeof STATE[0] ? STATE[u.state] : "");
}
void Shell::drawSysUpd() {
  sclear(C_NAVY);
  const Channel ch = sys_->channel();
  const bool locked = chanLocked(upd_.state);
  for (int c = 0; c < 2; c++) {
    ui::Box b = chanSeg(c);
    uint8_t col = c == ch ? C_GREEN : locked ? C_DKGRAY : C_LTGRAY;   // dimmed: not a choice right now
    sbutton(b, CHAN_NAMES[c], col, !locked && down(in_, b));
  }
  char when[24]; ago(when, sizeof when, now_, upd_.lastCheck);
  row(58, "Running", info_.version);
  row(67, "Newest", upd_.available[0] ? upd_.available : "?");
  row(76, "Checked", when);
  char st[24]; statusLine(st, sizeof st, upd_, info_.version);
  const bool radio = !busy(upd_.state) && upd_.state != UP_READY && radioBusy(*sys_);
  stextC(80, 87, radio ? "Wifi setup busy" : st, C_YELLOW);
  if (!radio) drawSysAction();
  sback();
}
void Shell::drawSysAction() {   // the bar while downloading, else the button that fits the state
  if (upd_.state == UP_DOWNLOADING) {
    sframe(30, 102, 100, 10, C_WHITE);
    srect(31, 103, (upd_.pct > 100 ? 100 : upd_.pct) * 98 / 100, 8, C_GREEN);
  }
  if (busy(upd_.state)) return;
  bool ready = upd_.state == UP_READY;
  sbutton(SYS_BTN, ready ? "Restart now" : "Check now", ready ? (uint8_t)C_GREEN : (uint8_t)C_SKY, down(in_, SYS_BTN));
}

// ---------------------------------------------------------------- Wifi: the networks a scan found, 4 a page
static const int NET_ROWS = 4;
static ui::Box netRow(int i) { return {22, 30 + i * 20, 116, 18}; }
static const ui::Box NET_PREV = {28, 114, 22, 18}, RESCAN = {54, 114, 52, 18}, NET_NEXT = {110, 114, 22, 18};
void Shell::scan() { scanRefused_ = !sys_->startScan(); nNets_ = scanRefused_ ? 0 : -1; }
void Shell::updateSysWifi() {
  upd_ = sys_->status();
  if (nNets_ < 0) nNets_ = sys_->scanResult(nets_, MAX_NETWORKS);
  if (ui::back(in_)) { go(SH_SYS_INFO); return; }
  const int pages = (nNets_ + NET_ROWS - 1) / NET_ROWS;
  if (nNets_ >= 0 && tapped(in_, RESCAN)) { netPage_ = 0; scan(); return; }
  if (pages > 1 && netPage_ > 0 && tapped(in_, NET_PREV)) netPage_--;
  if (pages > 1 && netPage_ + 1 < pages && tapped(in_, NET_NEXT)) netPage_++;
  for (int i = 0; i < NET_ROWS; i++) {
    const int k = netPage_ * NET_ROWS + i;
    if (k >= nNets_ || !tapped(in_, netRow(i))) continue;
    pick_ = nets_[k];
    pass_ = TextEntry{};
    if (pick_.secured) go(SH_SYS_PASS); else join();
    return;
  }
}
static void bars(int x, int y, int rssi) {   // 1-3 bars: under -75 dBm, under -60, the rest
  const int on = rssi >= -60 ? 3 : rssi >= -75 ? 2 : 1;
  for (int b = 0; b < 3; b++) srect(x + b * 3, y + 4 - 2 * b, 2, 3 + 2 * b, b < on ? (uint8_t)C_NAVY : (uint8_t)C_LTGRAY);
}
void Shell::drawNetRow(int i, const Network& n) {
  const ui::Box b = netRow(i);
  const bool pressed = down(in_, b);
  const int dy = pressed ? 1 : 0;
  sbutton(b, "", C_WHITE, pressed);
  bars(b.x + 5, b.y + 6 + dy, n.rssi);
  char name[40]; fit(name, sizeof name, n.ssid, b.w - 31);
  stext(b.x + 16, b.y + 6 + dy, name, C_NAVY);
  if (n.secured) {
    const int x = (b.x + b.w - 10) * 3, y = (b.y + 5 + dy) * 3;
    gfx565::roundRect({x + 3, y, 12, 17}, 6, scol(C_NAVY));
    gfx565::roundRect({x + 6, y + 3, 6, 11}, 3, scol(C_WHITE));
    gfx565::roundRect({x, y + 10, 18, 15}, 3, scol(C_NAVY));
  }
}
void Shell::drawSysWifi() {
  sclear(C_NAVY);
  const int pages = (nNets_ + NET_ROWS - 1) / NET_ROWS;
  if (nNets_ < 0) stextC(80, 66, "Looking for wifi...", C_WHITE);
  else if (scanRefused_ && busy(upd_.state)) { stextC(80, 60, "Busy with an update,", C_WHITE); stextC(80, 70, "Rescan when it is done", C_WHITE); }
  else if (scanRefused_) { stextC(80, 60, "Wifi is busy,", C_WHITE); stextC(80, 70, "Rescan in a moment", C_WHITE); }
  else if (!nNets_) stextC(80, 66, "No wifi found", C_WHITE);
  for (int i = 0; i < NET_ROWS && netPage_ * NET_ROWS + i < nNets_; i++) drawNetRow(i, nets_[netPage_ * NET_ROWS + i]);
  if (pages > 1) {
    sbutton(NET_PREV, "<", netPage_ > 0 ? (uint8_t)C_SKY : (uint8_t)C_DKGRAY, netPage_ > 0 && down(in_, NET_PREV));
    sbutton(NET_NEXT, ">", netPage_ + 1 < pages ? (uint8_t)C_SKY : (uint8_t)C_DKGRAY, netPage_ + 1 < pages && down(in_, NET_NEXT));
    char pg[24]; snprintf(pg, sizeof pg, "%d/%d", netPage_ + 1, pages);
    stextC(80, 138, pg, C_LTGRAY);
  }
  sbutton(RESCAN, "Rescan", nNets_ < 0 ? (uint8_t)C_DKGRAY : (uint8_t)C_SKY, nNets_ >= 0 && down(in_, RESCAN));
  sback();
}

// ---------------------------------------------------------------- the keyboard: full QWERTY shaped to the glass
// A user decision of 2026-10-18: full QWERTY for grown-ups, keys of 12x18 (4 x 6 mm, under the menu's 18 px floor:
// tools/playtest.py KEYBOARDS). Every tap goes to the nearest key (the 2 px gaps belong to their neighbours), and the
// key under the finger shows big above it. Rows of 10, 9, then shift + 7 + backspace, then page, space and OK; the
// widest row sits low, where the glass is widest. The 26 character keys type the page's characters (TK_SHIFT..TK_OK).
static const char* const PAGES[3] = {"qwertyuiopasdfghjklzxcvbnm", "1234567890-/:;()$&@\".,?!'#", "[]{}%^*+=_\\|~<>`"};
static const char* const NEXT_PAGE[3] = {"123", "#+=", "abc"};
// Character keys: rows from y 48, 20 apart, keys 12 wide every 14. The controls, in TK_SHIFT..TK_OK order: shift and
// backspace flank the third row, page, space and OK are the fourth.
static const ui::Box CONTROL_KEYS[TK_COUNT - TK_CHARS] = {
  {11, 88, 19, 18}, {21, 108, 26, 18}, {49, 108, 58, 18}, {130, 88, 19, 18}, {109, 108, 30, 18}};
static ui::Box textKeyBox(int key) {
  static const int N[3] = {10, 9, 7}, X0[3] = {11, 18, 32};
  if (key >= TK_CHARS) return CONTROL_KEYS[key - TK_CHARS];
  int r = 0;
  while (key >= N[r]) key -= N[r++];
  return {X0[r] + key * 14, 48 + r * 20, 12, 18};
}
static bool live(const TextEntry& t, int k) { return k >= TK_CHARS || textChar(t, k); }   // drawn and typing
// The key a finger at (x, y) means: the one whose box, grown by half a gap, holds it; -1 off the keyboard.
static int textKeyAt(int x, int y) {
  for (int k = 0; k < TK_COUNT; k++) {
    const ui::Box b = textKeyBox(k);
    if (x >= b.x - 1 && x < b.x + b.w + 1 && y >= b.y - 1 && y < b.y + b.h + 1) return k;
  }
  return -1;
}
char shell::textChar(const TextEntry& t, int key) {
  if (key < 0 || key >= TK_CHARS) return 0;
  const char* p = PAGES[t.page % 3];
  if (key >= (int)strlen(p)) return 0;
  return t.upper && p[key] >= 'a' && p[key] <= 'z' ? (char)(p[key] - 32) : p[key];
}
bool shell::textKey(TextEntry& t, int key) {
  char c = key == TK_SPACE ? ' ' : textChar(t, key);
  if (c && t.len < TEXT_MAX) { t.buf[t.len++] = c; t.buf[t.len] = 0; }
  if (key == TK_SHIFT) t.upper = !t.upper;
  if (key == TK_PAGE) t.page = (uint8_t)((t.page + 1) % 3);
  if (key == TK_DEL && t.len > 0) t.buf[--t.len] = 0;
  return key == TK_OK && t.len > 0;
}
// One frame of typing into t: true when OK was tapped with something typed.
static bool typeInto(const Input& in, TextEntry& t) {
  for (int k = 0; k < TK_COUNT; k++) {
    const ui::Box b = textKeyBox(k);
    if (live(t, k)) UiAudit::add(b.x, b.y, b.w, b.h);   // what tapped() would log, for the nearest-key rule
  }
  // The key under the finger when it lifts (the one the preview showed), however long it was held, for a press that
  // began on the keyboard: on 4 mm keys a finger often rolls onto the next one.
  if (!in.released || textKeyAt(in.downX, in.downY) < 0) return false;
  const int k = textKeyAt(in.x, in.y);
  return k >= 0 && textKey(t, k);   // an empty slot types nothing; OK needs something typed
}
void Shell::updateSysPass() {
  if (ui::back(in_)) { pass_ = TextEntry{}; go(SH_SYS_WIFI); return; }
  if (typeInto(in_, pass_)) join();
}
// The keys, and the one under the finger big above it (a finger hides the key it presses).
void Shell::drawKeys(const TextEntry& t) {
  const int held = in_.down ? textKeyAt(in_.x, in_.y) : -1;
  for (int k = 0; k < TK_CHARS; k++) {
    const char c[2] = {textChar(t, k), 0};
    if (live(t, k)) sbutton(textKeyBox(k), c, C_WHITE, k == held, 1);
  }
  const char* const labels[5] = {"Aa", NEXT_PAGE[t.page % 3], "space", "<", "OK"};
  const uint8_t cols[5] = {t.upper ? (uint8_t)C_YELLOW : (uint8_t)C_LTGRAY, C_SKY, C_WHITE, C_LTGRAY,
                           t.len ? (uint8_t)C_GREEN : (uint8_t)C_LTGRAY};
  for (int k = TK_CHARS; k < TK_COUNT; k++) sbutton(textKeyBox(k), labels[k - TK_CHARS], cols[k - TK_CHARS], k == held, 1);
  if (held < 0 || held >= TK_CHARS || !live(t, held)) return;
  const ui::Box b = textKeyBox(held);
  const char c[2] = {textChar(t, held), 0};
  const int keyCenter = b.x + b.w / 2;
  const int previewCenter = keyCenter < 40 ? 40 : keyCenter > 120 ? 120 : keyCenter;
  // Edge keys sit where the glass is wide; their raised preview must move inward to the narrower upper chord.
  spanel({previewCenter - 12, b.y - 27, 24, 24}, C_WHITE, C_DKBROWN);
  font::text(adult::FONT32, c, previewCenter * 3 - font::textWidth(adult::FONT32, c) / 2, (b.y - 22) * 3, scol(C_NAVY));
}
// What was typed, every letter but the last as '*' (a password is never shown whole), its tail when it is long.
static void masked(char* out, size_t n, const TextEntry& t, bool cursor) {
  int k = 0;
  for (int i = 0; i < t.len && k + 2 < (int)n; i++) out[k++] = i + 1 < t.len ? '*' : t.buf[i];
  if (cursor && k + 1 < (int)n) out[k++] = '_';
  out[k] = 0;
  const char* s = out;
  while (stextW(s) > 98) s++;
  memmove(out, s, strlen(s) + 1);
}
void Shell::drawSysPass() {
  sclear(C_NAVY);
  spanel({28, 25, 104, 11}, C_WHITE, C_DKBROWN);
  char shown[TEXT_MAX + 2]; masked(shown, sizeof shown, pass_, pass_.len < TEXT_MAX && (ms_ / 400) % 2);
  if (pass_.len) stextC(80, 27, shown, C_NAVY); else stextC(80, 27, "Password", C_DKGRAY);
  drawKeys(pass_);
  sback();
}

// ---------------------------------------------------------------- joining: a test with the new password, then saved
// The test comes first, so a mistyped password never replaces a network that works; Save anyway keeps it regardless
// (a board set up away from that wifi). While the radio is busy (an update, a scan, an earlier test the grown-up backed
// out of) it waits, then tests. A save that fails (NVS) says so and can be tried again.
enum : int8_t { WAITING = -2, JOINING = -1, JOIN_FAILED = 0, JOINED = 1, SAVE_FAILED = 2 };
static const ui::Box JOIN_OK = {50, 98, 60, 20}, JOIN_RETRY = {14, 98, 60, 20}, JOIN_SAVE = {78, 98, 68, 20};
void Shell::join() { join_ = WAITING; go(SH_SYS_JOIN); }
bool Shell::save() { return sys_->saveWifi(pick_.ssid, pick_.secured ? pass_.buf : ""); }
void Shell::saved() {   // Save anyway, or Try again after a failed save
  if (!save()) { join_ = SAVE_FAILED; return; }
  pass_ = TextEntry{};
  go(SH_SYS_INFO);
}
void Shell::pollJoin() {
  if (join_ == WAITING && sys_->startTest(pick_.ssid, pick_.secured ? pass_.buf : "")) join_ = JOINING;
  else if (join_ == JOINING && (join_ = (int8_t)sys_->testResult()) == JOINED && !save()) join_ = SAVE_FAILED;
}
void Shell::updateSysJoin() {
  upd_ = sys_->status();
  pollJoin();
  if (ui::back(in_)) {
    if (join_ == JOINED) { pass_ = TextEntry{}; go(SH_SYS_INFO); } else go(pick_.secured ? SH_SYS_PASS : SH_SYS_WIFI);
    return;
  }
  if (join_ == JOINED && tapped(in_, JOIN_OK)) { pass_ = TextEntry{}; go(SH_SYS_INFO); return; }
  if (join_ == SAVE_FAILED && tapped(in_, JOIN_OK)) { saved(); return; }
  if (join_ != JOIN_FAILED) return;
  if (tapped(in_, JOIN_SAVE)) saved();
  else if (tapped(in_, JOIN_RETRY)) { if (pick_.secured) go(SH_SYS_PASS); else join(); }
}
void Shell::drawSysJoin() {
  sclear(C_NAVY);
  char name[40]; fit(name, sizeof name, pick_.ssid, 120);
  static const char* const HEAD[] = {"Could not connect to", "Connected to", "Could not save"};
  static const char* const TAIL[] = {"Wrong password or no signal", "Saved", "Nothing was changed"};
  if (join_ < 0) {
    static const char* const DOTS[] = {".", "..", "..."};
    const bool updating = join_ == WAITING && busy(upd_.state);
    stextC(80, 48, updating ? "Waiting for the update" : "Connecting to", C_LTGRAY);
    stextC(80, 60, name, C_WHITE); stextC(80, 72, DOTS[ms_ / 400 % 3], C_WHITE);
  } else {
    stextC(80, 48, HEAD[join_], C_LTGRAY); stextC(80, 60, name, C_WHITE);
    stextC(80, 74, TAIL[join_], join_ == JOINED ? (uint8_t)C_GREEN : (uint8_t)C_YELLOW);
  }
  if (join_ == JOINED) sbutton(JOIN_OK, "OK", C_GREEN, down(in_, JOIN_OK));
  if (join_ == SAVE_FAILED) sbutton(JOIN_OK, "Try again", C_SKY, down(in_, JOIN_OK));
  if (join_ == JOIN_FAILED) {
    sbutton(JOIN_RETRY, "Try again", C_SKY, down(in_, JOIN_RETRY));
    sbutton(JOIN_SAVE, "Save anyway", C_LTGRAY, down(in_, JOIN_SAVE));
  }
  sback();
}

// ---------------------------------------------------------------- Home: where the radar centers, from a street address
// Home shows what is set; Set address types one on the keyboard (shown in full, not masked: an address is not a
// secret), OK looks it up (a radio job: it waits while another runs), and the place found is saved only after Yes.
enum : int8_t { L_WAITING = -2, L_LOOKING = -1, L_NONE = 0, L_FOUND = 1, L_SAVE_FAILED = 2 };
static const ui::Box SET_BTN = {40, 104, 80, 20}, FIND_YES = {26, 104, 52, 20}, FIND_NO = {82, 104, 52, 20};
static const ui::Box FIND_RETRY = {50, 104, 60, 20};
// s word-wrapped into up to `lines` centered rows of at most room px from y; returns the y after the last row.
static int wrapC(int y, const char* s, int room, int lines, uint8_t c) {
  char line[64];
  while (*s && lines-- > 0) {
    int n = 0, cut = 0;
    while (s[n] && n < 63) {
      memcpy(line, s, (size_t)n + 1); line[n + 1] = 0;
      if (stextW(line) > room) break;
      n++;
      if (!s[n] || s[n] == ' ') cut = n;
    }
    if (!cut || !lines) cut = n;   // one long word, or the last row: cut where it stops fitting
    memcpy(line, s, (size_t)cut); line[cut] = 0;
    stextC(80, y, line, c);
    y += ROW;
    s += cut;
    while (*s == ' ') s++;
  }
  return y;
}
void Shell::updateSysHome() {
  if (ui::back(in_)) { go(SH_SYS_INFO); return; }
  if (tapped(in_, SET_BTN)) { addr_ = TextEntry{}; go(SH_SYS_ADDR); }
}
void Shell::drawSysHome() {
  sclear(C_NAVY);
  stextC(80, 30, "The radar centers on", C_LTGRAY);
  Home h;
  if (sys_->home(h)) {
    const int y = wrapC(44, h.name[0] ? h.name : "a place with no name", 120, 3, C_WHITE);
    char at[32]; snprintf(at, sizeof at, "%.3f, %.3f", h.lat, h.lon);
    stextC(80, y + 3, at, C_LTGRAY);
  } else {
    stextC(80, 50, "nothing yet", C_YELLOW);
  }
  sbutton(SET_BTN, "Set address", C_SKY, down(in_, SET_BTN));
  sback();
}
void Shell::updateSysAddr() {
  if (ui::back(in_)) { addr_ = TextEntry{}; go(SH_SYS_HOME); return; }
  if (typeInto(in_, addr_)) { look_ = L_WAITING; go(SH_SYS_FIND); }
}
void Shell::drawSysAddr() {
  sclear(C_NAVY);
  spanel({28, 25, 104, 11}, C_WHITE, C_DKBROWN);
  char shown[TEXT_MAX + 2];   // its tail, as much as fits, and a cursor
  snprintf(shown, sizeof shown, "%s%s", addr_.buf, addr_.len < TEXT_MAX && (ms_ / 400) % 2 ? "_" : "");
  const char* tail = shown;
  while (stextW(tail) > 98) tail++;
  if (addr_.len) stextC(80, 27, tail, C_NAVY); else stextC(80, 27, "Street, city", C_DKGRAY);
  drawKeys(addr_);
  sback();
}
void Shell::pollLook() {
  if (look_ == L_WAITING && sys_->startLookup(addr_.buf)) look_ = L_LOOKING;
  else if (look_ == L_LOOKING) look_ = (int8_t)sys_->lookupResult(found_);
}
void Shell::updateSysFind() {
  upd_ = sys_->status();
  pollLook();
  if (ui::back(in_)) { go(SH_SYS_ADDR); return; }   // the typing is kept
  if (look_ == L_FOUND && tapped(in_, FIND_YES)) {
    if (sys_->saveHome(found_)) { addr_ = TextEntry{}; go(SH_SYS_HOME); } else look_ = L_SAVE_FAILED;
  } else if (look_ == L_FOUND && tapped(in_, FIND_NO)) go(SH_SYS_ADDR);
  else if ((look_ == L_NONE || look_ == L_SAVE_FAILED) && tapped(in_, FIND_RETRY)) {
    if (look_ == L_NONE) go(SH_SYS_ADDR); else if (sys_->saveHome(found_)) go(SH_SYS_HOME);
  }
}
void Shell::drawSysFind() {
  sclear(C_NAVY);
  static const char* const DOTS[] = {".", "..", "..."};
  if (look_ < 0) {
    stextC(80, 48, look_ == L_WAITING && busy(upd_.state) ? "Waiting for the update" : "Looking up", C_LTGRAY);
    wrapC(60, addr_.buf, 120, 2, C_WHITE);
    stextC(80, 82, DOTS[ms_ / 400 % 3], C_WHITE);
  } else if (look_ == L_FOUND) {
    stextC(80, 30, "Is this home?", C_LTGRAY);
    wrapC(44, found_.name, 120, 4, C_WHITE);
    sbutton(FIND_YES, "Yes", C_GREEN, down(in_, FIND_YES));
    sbutton(FIND_NO, "No", C_LTGRAY, down(in_, FIND_NO));
  } else {
    stextC(80, 48, look_ == L_NONE ? "Could not find that address" : "Could not save", C_YELLOW);
    wrapC(62, look_ == L_NONE ? "Check the spelling, or add the city" : "Nothing was changed", 120, 2, C_LTGRAY);
    sbutton(FIND_RETRY, "Try again", C_SKY, down(in_, FIND_RETRY));
  }
  sback();
}

void Shell::drawSysToast() {
  const font::Box box = {&adult::FONT24, 360, 80, 5, font::Align::CENTER};
  const int h = font::textHeight(adult::FONT24, toast_, box.w, box.spacing);
  const int y = 210 - h / 2;
  gfx565::roundRect({48, y - 12, 384, h + 24}, 12, scol(C_DKBROWN));
  gfx565::roundRect({51, y - 9, 378, h + 18}, 9, scol(C_WHITE));
  font::textBox(box, toast_, 60, y, scol(C_DKBROWN));
}
