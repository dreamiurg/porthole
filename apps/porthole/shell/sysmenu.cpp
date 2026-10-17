// The device side the kid barely sees: the "something new" card after an update, and the grown-ups' system menu
// behind a 5 s hold on the picker's title (Info, then Updates). Shell widgets and palette only (root rule 7), no
// sound. Design: docs/superpowers/specs/2026-10-17-wireless-updates-design.md.
#include <math.h>
#include <stdio.h>
#include <string.h>
#include "shell.h"

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

// ---------------------------------------------------------------- the menu: label | value rows on navy
static const int ROW = 11, COL = 78;   // labels end at COL, values start 6 px after it
// Widest a value starting at x may be on the 8 px row at y, inside a 78 px circle (the audit clips at 79).
static int valueRoom(int x, int y) {
  int d = y < 80 ? 80 - y : y + 8 - 80;
  return (int)sqrtf((float)(78 * 78 - d * d)) + 80 - x;
}
static void row(int y, const char* label, const char* value) {
  text(COL - textWidth(label), y, label, C_LTGRAY);
  char v[40]; snprintf(v, sizeof v, "%s", value);
  for (int n = (int)strlen(v), room = valueRoom(COL + 6, y); n > 0 && textWidth(v) > room;) {   // a long SSID: "HomeNe.."
    v[--n] = 0;
    if (n > 2) v[n - 1] = v[n - 2] = '.';
  }
  text(COL + 6, y, v, C_WHITE);
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

static const ui::Box SYS_BTN = {36, 116, 88, 24};   // the one button under the rows: Updates, Check now, Restart now
void Shell::updateSysInfo() {
  upd_ = sys_->status();
  if (ui::back(in_)) { go(SH_PICK); return; }
  if (in_.tapIn(SYS_BTN.x, SYS_BTN.y, SYS_BTN.w, SYS_BTN.h)) go(SH_SYS_UPD);
}
void Shell::drawSysInfo() {
  clear(C_NAVY);
  char heap[16], tried[24]; snprintf(heap, sizeof heap, "%lu KB", (unsigned long)(info_.heapFree / 1024));
  wifiResult(tried, sizeof tried, upd_, now_);
  int y = 30;
  row(y, "Version", info_.version); y += ROW;
  row(y, "Slot", info_.slot); y += ROW;
  row(y, "Reset", info_.reset); y += ROW;
  row(y, "Free heap", heap); y += ROW;
  row(y, "SD card", info_.sd ? "yes" : "no"); y += ROW;
  row(y, "Wifi", info_.wifiSaved ? info_.ssid : "none"); y += ROW;
  row(y, upd_.state == UP_IDLE && upd_.lastCheck ? "Checked" : "Last try", tried);   // before this boot: a time, no result
  ui::drawButton({SYS_BTN, "Updates", nullptr, C_SKY}, in_.down && in_.hit(SYS_BTN.x, SYS_BTN.y, SYS_BTN.w, SYS_BTN.h));
  ui::drawBack();
}

// ---------------------------------------------------------------- Updates: channel, versions, Check now / Restart now
static ui::Box chanSeg(int c) { return {26 + c * 56, 30, 52, 24}; }
static const char* const CHAN_NAMES[2] = {"Stable", "Beta"};
static bool busy(UpdState s) { return s == UP_CHECKING || s == UP_DOWNLOADING; }   // no button: it is already on it
// The channel stays put mid-check and with an image waiting: switching then would show one channel's result as another's.
static bool chanLocked(UpdState s) { return busy(s) || s == UP_READY; }
void Shell::updateSysUpd() {
  upd_ = sys_->status();
  if (ui::back(in_)) { go(SH_SYS_INFO); return; }
  for (int c = 0; c < 2 && !chanLocked(upd_.state); c++) {
    ui::Box b = chanSeg(c);
    if (in_.tapIn(b.x, b.y, b.w, b.h) && c != sys_->channel()) sys_->setChannel((Channel)c);
  }
  if (busy(upd_.state) || !in_.tapIn(SYS_BTN.x, SYS_BTN.y, SYS_BTN.w, SYS_BTN.h)) return;
  if (upd_.state == UP_READY) sys_->restart(); else sys_->checkNow();
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
  clear(C_NAVY);
  const Channel ch = sys_->channel();
  const bool locked = chanLocked(upd_.state);
  for (int c = 0; c < 2; c++) {
    ui::Box b = chanSeg(c);
    uint8_t col = c == ch ? C_GREEN : locked ? C_DKGRAY : C_LTGRAY;   // dimmed: not a choice right now
    ui::drawButton({b, CHAN_NAMES[c], nullptr, col}, !locked && in_.down && in_.hit(b.x, b.y, b.w, b.h));
  }
  char when[24]; ago(when, sizeof when, now_, upd_.lastCheck);
  row(60, "Running", info_.version);
  row(71, "Newest", upd_.available[0] ? upd_.available : "?");
  row(82, "Checked", when);
  char st[24]; statusLine(st, sizeof st, upd_, info_.version);
  textCentered(80, 95, st, C_YELLOW);
  drawSysAction();
  ui::drawBack();
}
void Shell::drawSysAction() {   // the bar while downloading, else the button that fits the state
  if (upd_.state == UP_DOWNLOADING) {
    frame(30, 118, 100, 10, C_WHITE);
    rect(31, 119, (upd_.pct > 100 ? 100 : upd_.pct) * 98 / 100, 8, C_GREEN);
  }
  if (busy(upd_.state)) return;
  bool ready = upd_.state == UP_READY;
  ui::drawButton({SYS_BTN, ready ? "Restart now" : "Check now", nullptr, ready ? (uint8_t)C_GREEN : (uint8_t)C_SKY},
                 in_.down && in_.hit(SYS_BTN.x, SYS_BTN.y, SYS_BTN.w, SYS_BTN.h));
}
