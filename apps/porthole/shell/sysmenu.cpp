// The device side the kid barely sees: the "something new" card after an update, and the grown-ups' system menu
// behind a 5 s hold on the picker's title (Info, then Updates and Wifi). Shell palette only (root rule 7), no sound.
// The menu is for grown-ups (a user decision of 2026-10-18): it draws in the shell's compact font (shell_font.h)
// with 18 px targets, denser than the kids' 8x8 font and 24x22 buttons; the "something new" card is the kids' and
// keeps their rules. Design: docs/superpowers/specs/2026-10-17-wireless-updates-design.md.
#include <math.h>
#include <stdio.h>
#include <string.h>
#include "shell.h"
#include "shell_font.h"
#include "shell_sprites.h"

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
static int glyphOf(char ch) { unsigned char u = (unsigned char)ch; return (u < 32 || u > 126 ? '?' : u) - 32; }   // a non-ASCII SSID byte: "?"
static int stextW(const char* s) {
  int w = 0;
  for (; *s; s++) w += SYS_FONT_W[glyphOf(*s)] + 1;
  return w ? w - 1 : 0;
}
static void stext(int x, int y, const char* s, uint8_t c) {
  logText(x, y, stextW(s), SYS_FONT_H, c);
  for (; *s; s++) {
    const int g = glyphOf(*s);
    for (int row = 0; row < SYS_FONT_H; row++)
      for (int col = 0; col < SYS_FONT_W[g]; col++) if (SYS_FONT[g][row] >> col & 1) pixel(x + col, y + row, c);
    x += SYS_FONT_W[g] + 1;
  }
}
static void stextC(int cx, int y, const char* s, uint8_t c) { stext(cx - stextW(s) / 2, y, s, c); }
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
  if (!pressed) roundRect(b.x, b.y + shadow, b.w, b.h, C_DKBROWN);
  roundRect(b.x, b.y + dy, b.w, b.h, C_DKBROWN);
  const uint8_t fill = pressed && shadow == 1 ? (uint8_t)C_YELLOW : col;
  roundRect(b.x + 1, b.y + 1 + dy, b.w - 2, b.h - 2, fill);
  if (shadow == 2) hline(b.x + 2, b.y + 1 + dy, b.w - 4, C_WHITE);
  stextC(b.x + b.w / 2, b.y + dy + (b.h - 5) / 2, label, ui::inkOn(fill));
}

// ---------------------------------------------------------------- Info: label | value rows on navy
static const int ROW = 9, COL = 76;   // labels end at COL, values start 5 px after it
// Widest a value starting at x may be on the row at y, inside a 78 px circle (the audit clips at 79).
static int valueRoom(int x, int y) {
  int d = y < 80 ? 80 - y : y + SYS_FONT_H - 80;
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
void Shell::updateSysInfo() {
  upd_ = sys_->status();
  if (ui::back(in_)) { go(SH_PICK); return; }
  if (tapped(in_, UPD_BTN)) { go(SH_SYS_UPD); return; }
  if (tapped(in_, WIFI_BTN)) { netPage_ = 0; scan(); go(SH_SYS_WIFI); }
}
void Shell::drawSysInfo() {
  clear(C_NAVY);
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
  ui::drawBack();
}

// ---------------------------------------------------------------- Updates: channel, versions, Check now / Restart now
static ui::Box chanSeg(int c) { return {28 + c * 54, 30, 50, 20}; }
static const ui::Box SYS_BTN = {40, 98, 80, 20};   // Check now, Restart now
static const char* const CHAN_NAMES[2] = {"Stable", "Beta"};
static bool busy(UpdState s) { return s == UP_CHECKING || s == UP_DOWNLOADING; }   // no button: it is already on it
// The channel stays put mid-check and with an image waiting: switching then would show one channel's result as another's.
static bool chanLocked(UpdState s) { return busy(s) || s == UP_READY; }
// A scan or a test join holds the radio: Check now would do nothing until it is done.
static bool radioBusy(System& s) { return s.scanResult(nullptr, 0) < 0 || s.testResult() < 0; }
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
  clear(C_NAVY);
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
  ui::drawBack();
}
void Shell::drawSysAction() {   // the bar while downloading, else the button that fits the state
  if (upd_.state == UP_DOWNLOADING) {
    frame(30, 102, 100, 10, C_WHITE);
    rect(31, 103, (upd_.pct > 100 ? 100 : upd_.pct) * 98 / 100, 8, C_GREEN);
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
  for (int b = 0; b < 3; b++) rect(x + b * 3, y + 4 - 2 * b, 2, 3 + 2 * b, b < on ? (uint8_t)C_NAVY : (uint8_t)C_LTGRAY);
}
void Shell::drawNetRow(int i, const Network& n) {
  const ui::Box b = netRow(i);
  const bool pressed = down(in_, b);
  const int dy = pressed ? 1 : 0;
  sbutton(b, "", C_WHITE, pressed);
  bars(b.x + 5, b.y + 6 + dy, n.rssi);
  char name[40]; fit(name, sizeof name, n.ssid, b.w - 31);
  stext(b.x + 16, b.y + 6 + dy, name, C_NAVY);
  if (n.secured) blitTint(SPR_LOCK, b.x + b.w - 11, b.y + 4 + dy, C_NAVY);
}
void Shell::drawSysWifi() {
  clear(C_NAVY);
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
  ui::drawBack();
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
  ui::panel({b.x + b.w / 2 - 12, b.y - 27, 24, 24}, C_WHITE, C_DKBROWN);
  gfx::textCentered(b.x + b.w / 2, b.y - 23, c, C_NAVY, 2);
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
  clear(C_NAVY);
  ui::panel({28, 25, 104, 11}, C_WHITE, C_DKBROWN);
  char shown[TEXT_MAX + 2]; masked(shown, sizeof shown, pass_, pass_.len < TEXT_MAX && (ms_ / 400) % 2);
  if (pass_.len) stextC(80, 27, shown, C_NAVY); else stextC(80, 27, "Password", C_DKGRAY);
  drawKeys(pass_);
  ui::drawBack();
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
  clear(C_NAVY);
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
  ui::drawBack();
}
