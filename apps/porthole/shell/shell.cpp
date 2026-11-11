#include "shell.h"
#include <ctype.h>
#include <stdio.h>
#include <string.h>
#include "launcher.h"
#include "shell_sprites.h"

using namespace gfx;
using shell::Record;

void Shell::begin(shell::Store& st, App* const* apps, int nApps, shell::System* sys, const uint8_t* ids) {
  st_ = &st; sys_ = sys; apps_ = apps; nApps_ = nApps < MAX_APPS ? nApps : MAX_APPS; app_ = nullptr; active_ = target_ = -1;
  for (bool& o : opened_) o = false;
  for (int k = 0; k < nApps_; k++) ids_[k] = ids ? ids[k] : (uint8_t)(k + 1);
  gamesOff_ = shell::loadGamesOff(st); starMode_ = false;
  shell::loadAll(st, prof_);
  limits_ = shell::loadLimits(st); window_ = shell::loadPlayWindow(st); allowance_.stop(); adminPage_ = 0;
  uint8_t l = shell::LIGHT_AUTO; st.load("sys", "lite", &l, 1);   // nothing saved yet (or a stray value): Auto
  light_ = l < shell::LIGHT_COUNT ? (shell::Light)l : shell::LIGHT_AUTO;
  adminPin_ = 0; st.load("sys", "admin", &adminPin_, sizeof adminPin_);
  uint8_t sound = 1; sound_ = st.load("sys", "sound", &sound, 1) != 1 || sound != 0;
  if (adminPin_ > 10000) adminPin_ = 0;
  locks_ = shell::loadPinLocks(st);
  if (sys_) sys_->info(info_);
  if (sys_ && shell::newRun(st, info_.version)) go(SH_SYS_NEW); else land();
  gate_.closed = false;   // boot: no finger to guard against
}
void Shell::land() { if (prof_.count()) go(SH_PICK); else startNew(); }   // a fresh device goes straight to making the first profile
uint32_t Shell::lastSeen() const {
  uint32_t t = 0;
  for (int id = 0; id < MAX_PROFILES; id++) if (prof_.used[id] && prof_.rec[id].lastPlayed > t) t = prof_.rec[id].lastPlayed;
  return t;
}
void Shell::go(Screen s) {
  screen_ = s; screenMs_ = ms_; toastUntil_ = pinWrongUntil_ = pinCalmUntil_ = pinNudgeUntil_ = 0; in_.tap = in_.pressed = in_.longPress = false;   // a tap acts on one screen only
  const bool pad = (s >= SH_PIN_SET && s <= SH_PIN) || (s >= SH_SYS_PIN && s <= SH_SYS_PIN_AGAIN);
  if (pad) gate_.shownAt(ms_, in_.x, in_.y); else gate_.shown(ms_);   // a keypad takes a quick first key (os/ui.h)
  if (s == SH_PICK) active_ = -1;   // nobody is playing while the picker shows
  if (s == SH_SYS_INFO || s == SH_SYS_UPD) { sys_->info(info_); upd_ = sys_->status(); }   // drawn before their first update
}
void Shell::toast(const char* s) { snprintf(toast_, sizeof toast_, "%s", s); toastUntil_ = ms_ + 1500; }

using ScreenFn = void (Shell::*)();
void Shell::update(uint32_t nowSec, uint32_t ms, const Input& in) {
  static const ScreenFn UPDATE[] = {&Shell::updatePick, &Shell::updateName, &Shell::updateAvatar, &Shell::updateAge, &Shell::updatePin,
    &Shell::updatePin, &Shell::updatePin, &Shell::updateDelete, &Shell::updateNoDelete, &Shell::updateLauncher, &Shell::updateRest,
    &Shell::updateSysNew, &Shell::updateSysInfo, &Shell::updateSysUpd, &Shell::updateSysWifi, &Shell::updateSysPass,
    &Shell::updateSysJoin, &Shell::updateSysHome, &Shell::updateSysAddr, &Shell::updateSysFind, &Shell::updateLight, &Shell::updateLibrary, &Shell::updateSysAdmin, &Shell::updateSysPin, &Shell::updateSysPin,
    &Shell::updateSysPin, &Shell::updateSysConfirm, &Shell::updateSysConfirm, &Shell::updateSysPlayers, &Shell::updateSysPlayerPin,
    &Shell::updateSysLimits, &Shell::updateSysClock, &Shell::updateSysTime, &Shell::updateSysWindow, &Shell::updateSysAllowance,
    &Shell::updateSysZone, &Shell::updateSysZoneSearch, &Shell::updateSysGames, &Shell::updateApp};
  static_assert(sizeof UPDATE / sizeof UPDATE[0] == SH_APP + 1, "one update per screen, in Screen order");
  now_ = nowSec; ms_ = ms; in_ = in; allowance_.tick(ms_);
  activity_.step(in, in.down, ms);
  gate_.filter(in_, ms_);   // the game's screens count as one: it gates its own
  if (guardScreenTime()) return;
  const ScreenFn fn = UPDATE[screen_];   // never (this->*TABLE[i])(): gcc 13.3/14.2 -fsanitize=bounds on aarch64 miscompiles it
  (this->*fn)();
}
void Shell::render() {
  static const ScreenFn DRAW[] = {&Shell::drawPick, &Shell::drawName, &Shell::drawAvatar, &Shell::drawAge, &Shell::drawPin,
    &Shell::drawPin, &Shell::drawPin, &Shell::drawDelete, &Shell::drawNoDelete, &Shell::drawLauncher, &Shell::drawRest,
    &Shell::drawSysNew, &Shell::drawSysInfo, &Shell::drawSysUpd, &Shell::drawSysWifi, &Shell::drawSysPass,
    &Shell::drawSysJoin, &Shell::drawSysHome, &Shell::drawSysAddr, &Shell::drawSysFind, &Shell::drawLight, &Shell::drawLibrary, &Shell::drawSysAdmin, &Shell::drawSysPin, &Shell::drawSysPin,
    &Shell::drawSysPin, &Shell::drawSysConfirm, &Shell::drawSysConfirm, &Shell::drawSysPlayers, &Shell::drawSysPlayerPin,
    &Shell::drawSysLimits, &Shell::drawSysClock, &Shell::drawSysTime, &Shell::drawSysWindow, &Shell::drawSysAllowance,
    &Shell::drawSysZone, &Shell::drawSysZoneSearch, &Shell::drawSysGames, &Shell::drawApp};
  static_assert(sizeof DRAW / sizeof DRAW[0] == SH_APP + 1, "one draw per screen, in Screen order");
  const ScreenFn fn = DRAW[screen_];     // see update()
  (this->*fn)();
  bool pinScreen = (screen_ >= SH_PIN_SET && screen_ <= SH_PIN) || (screen_ >= SH_SYS_PIN && screen_ <= SH_SYS_PIN_AGAIN);
  if (ms_ < toastUntil_ && !pinScreen && adultScreen()) drawSysToast();
  else if (ms_ < toastUntil_ && !pinScreen && surface() == SURFACE_INDEXED) ui::toast(toast_);   // gfx::fb is not on the glass under an RGB565 game
}
Tint Shell::tint() const {   // the game's light, or the clock's: a night game never hands back to a daylight launcher
  if (screen_ == SH_APP) return app_->tint();
  if (light_ != shell::LIGHT_AUTO) return TINT_DAY;   // a picked brightness is daylight on every screen, resting too
  Tint t = clockTint(now_);
  return screen_ == SH_REST && t == TINT_DAY ? TINT_EVENING : t;   // resting is never broad daylight
}
// One switch for the whole device, in the grown-ups' Admin menu: kids never reach for the buzzer, so the launcher has no
// mute button. A profile's old `muted` byte stays in its record (append-only) and is no longer read.
bool Shell::soundOn(uint32_t ms) { return screen_ == SH_APP && app_->soundOn(ms) && sound_; }

// ---------------------------------------------------------------- flow
void Shell::startNew() {
  memset(&draft_, 0, sizeof draft_); nameLen_ = 0; name_[0] = 0; namePage_ = 0; creating_ = true;
  go(SH_NAME);
}
// Deleting always takes a code (a parent decision of 2026-11-09: two long presses by a small kid must not delete a
// profile): the kid's own, or the grown-up's from "Admin" on that pad; with no kid code, the grown-up's; with
// neither, nobody can, and a card says who can fix that.
void Shell::choose(int id, bool del) {
  target_ = id; deleting_ = del;
  if (prof_.rec[id].pin) { resetPin(); go(SH_PIN); }
  else if (!del) select(id);
  else if (adminPin_) askAdmin(SH_DELETE);
  else go(SH_NO_DELETE);
}
void Shell::authorized() {
  if (!deleting_) { select(target_); return; }
  holdTop_ = in_.y >= 80; go(SH_DELETE);   // the finger (long press or the code's last digit) is where "No" is safe
}
bool Shell::restingNow() const {
  return !allowance_.active(ms_) && (!shell::windowAllows(window_, now_) || shell::restLeft(prof_.rec[active_], now_, limits_) > 0);
}
void Shell::enforceCurrentBudget() {
  if (allowance_.active(ms_)) return;
  const uint32_t before = rec().restUntil;
  shell::enforceBudget(rec(), now_, limits_);
  if (rec().restUntil != before) shell::saveRecord(*st_, prof_, active_);
}
bool Shell::guardScreenTime() {
  if (active_ < 0 || (screen_ != SH_APP && screen_ != SH_LAUNCHER && screen_ != SH_LIGHT && screen_ != SH_LIBRARY)) return false;
  enforceCurrentBudget();
  if (!restingNow()) return false;
  if (screen_ == SH_APP) closeApp();
  leaveStars(false);   // only the check saves stars
  go(SH_REST); return true;
}
void Shell::select(int id) {
  active_ = id; page_ = 0; starMode_ = false;
  if (!shell::loadFavorites(*st_, id, favs_)) {   // a kid's first Home: no game here now is new to them
    for (int k = 0; k < nApps_; k++) favs_.seen |= shell::gameBit(ids_[k]);
    // Written only when missing: a damaged one, or a newer version's after a USB rollback, stays for that firmware.
    if (!st_->load(shell::NS, shell::key('f', id), blobs_[0], sizeof blobs_[0])) shell::saveFavorites(*st_, id, favs_);
  }
  shell::recharge(rec(), now_); enforceCurrentBudget();
  if (!rec().age) { creating_ = false; go(SH_AGE); return; }   // migrated from a v1 Pets Club save: ask once
  go(restingNow() ? SH_REST : SH_LAUNCHER);
}
void Shell::finishCreate(uint16_t pin) {
  draft_.pin = pin;
  const char* stores[MAX_APPS]; int n = appStores(stores);
  int id = shell::create(*st_, prof_, draft_, stores, n);
  if (id < 0) { go(SH_PICK); return; }   // full: the picker hides "+" at four, so only a sim script gets here
  pinClear(id);   // a new kid never inherits a deleted one's strikes, even with the same code
  select(id);
}
int Shell::appStores(const char** out) const {
  int n = 0;
  for (int k = 0; k < nApps_ && n < MAX_APPS; k++) out[n++] = apps_[k]->store();
  return n;
}
int Shell::createProfile(const char* name, uint8_t age, const char* code) {
  memset(&draft_, 0, sizeof draft_);
  snprintf(draft_.name, sizeof draft_.name, "%s", name); draft_.age = age;
  const int id = prof_.firstFree();
  if (id >= 0) draft_.avatar = (uint8_t)(id % NUM_AVATARS);
  finishCreate(shell::pinCode(code));
  gate_.closed = false;   // a hook, not a touch
  return screen_ == SH_PICK ? -1 : active_;
}
static bool sameName(const char* a, const char* b) {   // "pets-club" matches "Pets Club"
  for (;; a++, b++) {
    while (*a == ' ' || *a == '-') a++;
    while (*b == ' ' || *b == '-') b++;
    if (tolower((unsigned char)*a) != tolower((unsigned char)*b)) return false;
    if (!*a) return true;
  }
}
bool Shell::openApp(const char* name) {
  if (active_ < 0 || screen_ == SH_APP) return false;
  int on[MAX_APPS]; const int n = shell::enabledGames(ids_, nApps_, gamesOff_, on);   // a game switched off is not there
  for (int i = 0; i < n; i++) {   // by its label or its store (which never changes: scripts outlive a renamed label)
    const int k = on[i];
    if (sameName(apps_[k]->name(), name) || sameName(apps_[k]->store(), name)) { openIdx(k); gate_.closed = false; return screen_ == SH_APP; }
  }
  return false;
}
void Shell::openIdx(int k) {
  leaveStars(false);   // the sim's `app` opens a game from star mode: its edits were never confirmed
  enforceCurrentBudget();
  if (restingNow()) { go(SH_REST); return; }
  app_ = apps_[k]; opened_[k] = true; fromLibrary_ = screen_ == SH_LIBRARY;
  int on[MAX_APPS]; const int m = shell::enabledGames(ids_, nApps_, gamesOff_, on);
  for (int i = 0; i < m; i++) if (on[i] == k) page_ = launcher::pageOf(m, i);   // back from a game: its Library page
  if (unseen(k)) { favs_.seen |= shell::gameBit(ids_[k]); shell::saveFavorites(*st_, active_, favs_); }   // no longer new
  Profile all[MAX_PROFILES]; SaveSlot saves[MAX_PROFILES]; int n = 0, who = 0;
  for (int id; (id = prof_.nth(n)) >= 0; n++) {
    all[n] = shell::toProfile(prof_, id);
    saves[n] = {blobs_[n], st_->load(app_->store(), shell::key('s', id), blobs_[n], shell::BLOB_MAX)};
    if (id == active_) who = n;
  }
  memcpy(written_, blobs_[who], saves[who].len); writtenLen_ = saves[who].len; writtenOk_ = true;   // on flash already
  app_->enter({&all[who], all, saves, n, now_, ms_, light_ != shell::LIGHT_AUTO});
  lastTick_ = lastRecordSave_ = now_; lastSaveMs_ = playTickMs_ = ms_;
  go(SH_APP);
}
void Shell::flushApp(bool allowed) {
  // The shell keeps what it last wrote: the same bytes again are not rewritten (a story reader's view change), and a
  // write that failed is retried from this copy at the save cadence, not every frame.
  const void* data; size_t len;
  if (app_->takeSave(&data, &len, allowed) && len <= shell::BLOB_MAX) {
    if (writtenOk_ && len == writtenLen_ && !memcmp(data, written_, len)) return;   // keeps the cadence: a page turn next is still prompt
    memcpy(written_, data, len); writtenLen_ = len;
  } else if (writtenOk_ || !allowed) return;
  const char* k = shell::key('s', active_);
  if (writtenLen_) writtenOk_ = st_->save(app_->store(), k, written_, writtenLen_);
  else { st_->erase(app_->store(), k); writtenOk_ = true; }   // App contract: a 0-byte save erases; no game uses it today
  lastSaveMs_ = ms_;
}
void Shell::closeApp() {
  app_->leave();
  flushApp(true);
  shell::saveRecord(*st_, prof_, active_);
}
void Shell::budgetTick() {   // per profile, across every game
  uint32_t dt = (ms_ - playTickMs_) / 1000; playTickMs_ += dt * 1000; lastTick_ = now_;
  const bool idle = activity_.idleMs(ms_) >= shell::IDLE_MS;
  if (allowance_.active(ms_)) shell::playUnrestricted(rec(), now_, dt, idle);
  else if (shell::play(rec(), now_, dt, idle, limits_)) { closeApp(); go(SH_REST); return; }
  if (now_ - lastRecordSave_ >= 60) { shell::saveRecord(*st_, prof_, active_); lastRecordSave_ = now_; }   // checkpoint
}

// ---------------------------------------------------------------- Who's playing? (one row per profile, "+" below)
static ui::Box pickRow(int i, int rows) { return {24, 80 - (rows * 26 - 2) / 2 + i * 26, 112, 24}; }   // an 8-letter name clears the lock
// A 6-year-old holds their own name 1-3 s to play (playtest 8): any shorter press opens it, and only a hold this long
// asks to delete. A bar fills under the name from DELETE_HINT_MS, so the longer meaning shows before it acts.
static const uint32_t DELETE_HOLD_MS = 3000, DELETE_HINT_MS = 1000;
void Shell::updatePick() {
  if (titleHeld()) { enterSystem(); return; }
  int n = prof_.count(), rows = prof_.firstFree() < 0 ? n : n + 1;
  for (int i = 0; i < rows; i++) {
    ui::Box b = pickRow(i, rows);
    bool tap = in_.tapIn(b.x, b.y, b.w, b.h);
    if (i == n) { if (tap) askAdmin(SH_NAME); }
    else if (tap) choose(prof_.nth(i), false);
    else if (ui::pressing(in_, b) && in_.heldMs >= DELETE_HOLD_MS) choose(prof_.nth(i), true);
    if (screen_ != SH_PICK) return;
  }
}
void Shell::drawPick() {
  clear(C_SKY);
  textCentered(80, 18, "Who's playing?", C_NAVY);
  int n = prof_.count(), rows = prof_.firstFree() < 0 ? n : n + 1;
  for (int i = 0; i < rows; i++) {
    ui::Box b = pickRow(i, rows);
    int dy = ui::pressing(in_, b) ? 1 : 0;
    ui::drawButton({b, nullptr, nullptr, i == n ? (uint8_t)C_MINT : (uint8_t)C_CREAM}, dy);
    if (i == n) { blitTint(SPR_PLUS, 58, b.y + 8 + dy, C_DKBROWN); text(72, b.y + 9 + dy, "New", C_DKBROWN); continue; }
    const Record& r = prof_.rec[prof_.nth(i)];
    blit(SPR_AVATARS[r.avatar % NUM_AVATARS], b.x + 4, b.y + 2 + dy);
    text(b.x + 30, b.y + 9 + dy, r.name, C_DKBROWN);
    if (r.pin) blitTint(SPR_LOCK, b.x + b.w - 12, b.y + 8 + dy, C_DKGRAY);
    if (dy && in_.heldMs > DELETE_HINT_MS)   // under the name, clear of the face and the lock
      rect(b.x + 30, b.y + 19 + dy, (int)((b.w - 46) * (in_.heldMs - DELETE_HINT_MS) / (DELETE_HOLD_MS - DELETE_HINT_MS)), 2, C_PLUM);
  }
  if (!sys_) return;   // the running firmware, small, under the rows: "v0.16.2", or "dev" for a local build
  int v[3]; char ver[16];   // at most 10 characters: 80 px, inside the glass's 84 px chord at the text's bottom row
  snprintf(ver, sizeof ver, "%s%.9s", shell::parseVersion(info_.version, v) ? "v" : "", info_.version);
  textCentered(80, 140, ver, C_DKGRAY);
}

// ---------------------------------------------------------------- new profile: name -> face -> age -> code
void Shell::updateName() {
  if (ui::back(in_)) { go(SH_PICK); return; }
  if (ui::keyboard(in_, name_, nameLen_, namePage_)) { snprintf(draft_.name, sizeof draft_.name, "%s", name_); go(SH_AVATAR); }
}
void Shell::drawName() { clear(C_WALL); ui::drawKeyboard(in_, name_, namePage_, "Your name?", ms_); ui::drawBack(); }

static ui::Box avatarCell(int i) { return {20 + (i % 4) * 30, 42 + (i / 4) * 32, 28, 30}; }
void Shell::updateAvatar() {
  if (ui::back(in_)) { go(SH_NAME); return; }
  for (int i = 0; i < NUM_AVATARS; i++) {
    ui::Box b = avatarCell(i);
    if (in_.tapIn(b.x, b.y, b.w, b.h)) { draft_.avatar = (uint8_t)i; go(SH_AGE); return; }
  }
}
void Shell::drawAvatar() {
  clear(C_WALL);
  textCentered(80, 28, "Pick a face!", C_DKBROWN);
  for (int i = 0; i < NUM_AVATARS; i++) {
    ui::Box b = avatarCell(i);
    int dy = ui::pressing(in_, b) ? 1 : 0;
    bool taken = false;   // another profile's face: a gray tile (still allowed, faces may repeat)
    for (int id = 0; id < MAX_PROFILES; id++) taken |= prof_.used[id] && prof_.rec[id].avatar % NUM_AVATARS == i;
    ui::drawButton({{b.x + 1, b.y + 1, b.w - 2, b.h - 4}, nullptr, nullptr, dy ? (uint8_t)C_YELLOW : taken ? (uint8_t)C_LTGRAY : (uint8_t)C_WHITE}, dy);
    blit(SPR_AVATARS[i], b.x + 4, b.y + 4 + dy);
  }
  textCentered(80, 112, draft_.name, C_PLUM);
  ui::drawBack();
}

static const int AGES[6] = {5, 6, 7, 8, 9, 10};
static ui::Box ageButton(int i) { return {17 + (i % 3) * 42, 56 + (i / 3) * 32, 40, 26}; }
void Shell::updateAge() {
  if (ui::back(in_)) { go(creating_ ? SH_AVATAR : SH_PICK); return; }
  for (int i = 0; i < 6; i++) {
    ui::Box b = ageButton(i);
    if (!in_.tapIn(b.x, b.y, b.w, b.h)) continue;
    if (creating_) { draft_.age = (uint8_t)AGES[i]; resetPin(); go(SH_PIN_SET); return; }
    rec().age = (uint8_t)AGES[i]; shell::saveRecord(*st_, prof_, active_);
    select(active_);
    return;
  }
}
void Shell::drawAge() {
  clear(C_WALL);
  textCentered(80, 30, "How old are you?", C_DKBROWN);
  for (int i = 0; i < 6; i++) {
    ui::Box b = ageButton(i); char l[4]; snprintf(l, sizeof l, "%d", AGES[i]);
    bool pr = ui::pressing(in_, b);
    uint8_t col = i < 2 ? C_GREEN : i < 4 ? C_BLUE : C_PLUM, ink = ui::inkOn(col); int y = b.y + 5 + (pr ? 1 : 0);
    ui::drawButton({b, nullptr, nullptr, col}, pr);
    if (i < 5) { textCentered(b.x + 20, y, l, ink, 2); continue; }
    int w = textWidth(l, 2) + 1 + textWidth("+");   // "10+" at full size fills the button edge to edge: a small +
    text(text(b.x + 20 - w / 2, y, l, ink, 2) - 1, y, "+", ink);
  }
  ui::drawBack();
}

// ---------------------------------------------------------------- secret code: set twice (or skip), or asked on the picker
// Nine keys, 1-9, as big as the glass holds under the prompt and the four boxes: no 0, no erase, no OK, because fewer
// and bigger keys are what a small finger hits on this screen. The fourth digit submits by itself once its box has
// shown for PIN_SHOW_MS (touches wait meanwhile), so a kid sees what they typed. A slip is fixed by typing on: a wrong
// or mismatched code just starts over. "skip" (no code, only while choosing one) sits under the pad.
static const uint32_t PIN_SHOW_MS = 400, PIN_CALM_MS = 1000;
static ui::Box pinKey(int k) { return {23 + (k % 3) * 39, 50 + (k / 3) * 28, 36, 25}; }   // rows 50..131
static const ui::Box PIN_SKIP = {60, 134, 40, 22};
void Shell::updatePin() {
  static const Screen BACK[3] = {SH_AGE, SH_PIN_SET, SH_PICK};   // from SH_PIN_SET, SH_PIN_AGAIN, SH_PIN
  if (pinLen_ == 4) { fourthShown(); return; }
  if (ui::back(in_)) { resetPin(); go(BACK[screen_ - SH_PIN_SET]); return; }
  if (adminSwitch() && ui::clicked(in_, PIN_SKIP)) { askAdmin(SH_DELETE); return; }   // its own lock
  if (kidKeysHidden()) {   // too many wrong tries: the keys come back once the wait is over and the finger rests
    if (in_.down) pinCalmUntil_ = ms_ + PIN_CALM_MS;   // a kid tapping the hourglass never types into the keys' return
    if (in_.pressed) pinNudgeUntil_ = ms_ + 300;
    return;
  }
  if (screen_ == SH_PIN_SET && in_.tapIn(PIN_SKIP.x, PIN_SKIP.y, PIN_SKIP.w, PIN_SKIP.h)) { finishCreate(0); return; }   // no code
  if (const char d = kidKey()) typeDigit(d);
}
void Shell::fourthShown() {   // a key pressed while the fourth digit shows is already the next screen's first: never lost
  const char next = kidKey();
  if (!next && (ms_ - pinMs_ < PIN_SHOW_MS || in_.down)) return;   // a finger down waits: its lift is that key
  submitPin();
  if (next && screen_ >= SH_PIN_SET && screen_ <= SH_PIN && !kidKeysHidden()) typeDigit(next);
}
char Shell::kidKey() const {   // on release, however long: a slow, deliberate press counts too
  for (int k = 0; k < 9; k++) if (ui::clicked(in_, pinKey(k))) return (char)('1' + k);
  return 0;
}
// A failed write keeps the strikes in RAM until a reboot.
void Shell::pinStrike(int lock) { shell::pinWrong(locks_.lock[lock], now_, lockCode(lock)); shell::savePinLocks(*st_, locks_); }
void Shell::pinClear(int lock) {
  if (!locks_.lock[lock].strikes) return;   // the usual right code: no write
  locks_.lock[lock] = shell::PinLock{};
  shell::savePinLocks(*st_, locks_);
}
bool Shell::kidKeysHidden() { return screen_ == SH_PIN && (pinWait(target_) || (int32_t)(pinCalmUntil_ - ms_) > 0); }
void Shell::submitPin() {
  uint16_t code = shell::pinCode(pin_);
  resetPin();
  if (screen_ == SH_PIN_SET) { firstPin_ = code; go(SH_PIN_AGAIN); }
  else if (screen_ == SH_PIN_AGAIN && code == firstPin_) finishCreate(code);
  else if (screen_ == SH_PIN_AGAIN) { go(SH_PIN_SET); toast("Not the same!"); }
  else if (code == prof_.rec[target_].pin) { pinClear(target_); authorized(); }
  else { pinStrike(target_); pinWrongUntil_ = ms_ + 900; toast("Hmm, that's not it"); }
}
// A delete's pad is navy like "Delete <name>?" and says so: a kid who types their code by habit on a pad that looked
// like signing in landed on the delete question.
void Shell::drawPin() {
  const bool del = screen_ == SH_PIN && deleting_;
  clear(del ? C_NAVY : C_WALL);
  const bool hidden = kidKeysHidden();
  const uint32_t wait = hidden ? pinWait(target_) : 0;
  drawPinPrompt(hidden, del);
  drawPinDigits();
  if (adminSwitch()) drawPinKey(PIN_SKIP, "Admin", C_SKY);   // in the wait too: the grown-up's code is not slowed
  if (hidden) { drawPinWait(wait, del); ui::drawBack(); return; }
  for (int k = 0; k < 9; k++) { char l[2] = {(char)('1' + k), 0}; drawPinKey(pinKey(k), l, C_WHITE); }
  if (screen_ == SH_PIN_SET) drawPinKey(PIN_SKIP, "skip", C_SKY);
  ui::drawBack();
}
void Shell::drawPinPrompt(bool hidden, bool del) {
  char prompt[sizeof toast_];   // holds a whole toast: the message takes the prompt's place
  const bool msg = ms_ < toastUntil_;
  if (msg) snprintf(prompt, sizeof prompt, "%s", toast_);
  else if (hidden) snprintf(prompt, sizeof prompt, "Wait a little");
  else if (del) snprintf(prompt, sizeof prompt, "Delete %s?", prof_.rec[target_].name);
  else if (screen_ == SH_PIN) snprintf(prompt, sizeof prompt, "%s's code?", prof_.rec[target_].name);
  else snprintf(prompt, sizeof prompt, "%s", screen_ == SH_PIN_SET ? "Pick 4 numbers" : "Once more!");
  const uint8_t ink = del ? (uint8_t)C_WHITE : (uint8_t)C_DKBROWN, alert = del ? (uint8_t)C_YELLOW : (uint8_t)C_PLUM;
  textCentered(80, 26, prompt, msg ? alert : ink);
}
// While a wait runs, the keys give way to an hourglass whose sand runs down, its seconds and a bar that empties: a
// kid who cannot read the number still sees the wait end, and nothing looks like a key. A tap wiggles it, silently.
// On a delete's navy pad its edges, the seconds and the bar's frame are white: dark brown on navy barely shows.
void Shell::drawPinWait(uint32_t wait, bool del) {
  const uint32_t of = shell::pinWaitFor(locks_.lock[target_].strikes);
  const int top = of ? (int)(24 * wait / of) : 0;   // rows of sand still in the top bulb, of 24
  const int dx = (int32_t)(pinNudgeUntil_ - ms_) > 0 ? ((ms_ / 50) % 2 ? 2 : -2) : 0, cx = 80 + dx;
  const uint8_t edge = del ? (uint8_t)C_WHITE : (uint8_t)C_DKBROWN;
  rect(cx - 18, 50, 37, 4, edge); rect(cx - 18, 103, 37, 4, edge);   // the caps
  for (int r = 0; r < 24; r++) {   // r = 0 at a cap, 23 at the waist; the top bulb above y 78, the bottom below
    const int hw = 15 - r * 14 / 23, yt = 54 + r, yb = 102 - r;
    hline(cx - hw, yt, 2 * hw + 1, r >= 24 - top ? (uint8_t)C_ORANGE : (uint8_t)C_CREAM);
    hline(cx - hw, yb, 2 * hw + 1, r < 24 - top ? (uint8_t)C_ORANGE : (uint8_t)C_CREAM);
    hline(cx - hw - 1, yt, 1, edge); hline(cx + hw + 1, yt, 1, edge);
    hline(cx - hw - 1, yb, 1, edge); hline(cx + hw + 1, yb, 1, edge);
  }
  if (wait) line(cx, 78, cx, 101 - (24 - top), C_ORANGE);   // the falling stream
  char n[4]; snprintf(n, sizeof n, "%u", (unsigned)wait);
  textCentered(80, 110, n, edge, 2);
  const int w = of ? (int)(80 * wait / of) : 0;
  rect(40, 128, 80, 6, edge); rect(41, 129, w > 2 ? w - 2 : 0, 4, C_SKY);
}
void Shell::drawPinDigits() {   // four boxes: the digits while choosing a code, dots while asked for one
  bool wrong = ms_ < pinWrongUntil_; int shake = wrong ? ((ms_ / 50) % 2 ? 1 : -1) : 0;
  for (int i = 0; i < 4; i++) {
    int x = 58 + i * 12 + shake; rect(x, 36, 10, 11, C_WHITE); frame(x, 36, 10, 11, wrong ? C_RED : C_DKBROWN);
    if (i >= pinLen_) continue;
    if (screen_ == SH_PIN) circle(x + 5, 41, 2, C_NAVY); else { char l[2] = {pin_[i], 0}; textCentered(x + 5, 38, l, C_NAVY); }
  }
}
void Shell::drawPinKey(const ui::Box& b, const char* l, uint8_t col) {
  int dy = pinLen_ < 4 && ui::pressing(in_, b) ? 1 : 0;   // no press look while the last digit shows: taps wait
  roundRect(b.x, b.y + 1, b.w, b.h, C_DKBROWN); roundRect(b.x, b.y + dy, b.w, b.h, C_DKBROWN); roundRect(b.x + 1, b.y + 1 + dy, b.w - 2, b.h - 2, dy ? (uint8_t)C_YELLOW : col);
  int s = l[1] ? 1 : 2;   // a digit at double size; "skip" fits only at one
  textCentered(b.x + b.w / 2, b.y + (b.h - 8 * s) / 2 + dy, l, C_DKBROWN, s);
}

// ---------------------------------------------------------------- delete a profile (long press on the picker)
// "No" is big and central; deleting takes a deliberate HOLD_MS press on a button in the half the finger was not on,
// so neither the long press that got here nor one more tap can delete.
static const uint32_t HOLD_MS = 900;   // under the sim's 1 s `hold`, still a clear press-and-hold for a kid
static const ui::Box DEL_NO = {30, 66, 100, 28};
static ui::Box delHold(bool top) { return {32, top ? 30 : 104, 96, 24}; }
void Shell::updateDelete() {
  if (ui::button(in_, {DEL_NO, "No, keep!", nullptr, C_GREEN})) { go(SH_PICK); return; }
  if (!(ui::pressing(in_, delHold(holdTop_)) && in_.heldMs >= HOLD_MS)) return;
  const char* stores[MAX_APPS]; int n = appStores(stores);
  shell::removeProfile(*st_, prof_, target_, stores, n);
  land();
}
// No code anywhere, so nobody may delete: a kid-safe card, the profile's face with a lock beside it (a pre-reader's
// "it stays"), and the ways back.
static const ui::Box NO_DELETE_OK = {50, 120, 60, 24};
void Shell::updateNoDelete() {
  if (ui::back(in_) || ui::button(in_, {NO_DELETE_OK, "OK", nullptr, C_GREEN})) go(SH_PICK);
}
void Shell::drawNoDelete() {
  clear(C_SKY);
  const Record& r = prof_.rec[target_];
  blitScaled(SPR_AVATARS[r.avatar % NUM_AVATARS], 50, 28, 2);
  for (int y = 0; y < SPR_LOCK.h; y++)   // the picker's lock, twice the size, in its dark gray
    for (int x = 0; x < SPR_LOCK.w; x++) if (SPR_LOCK.px[y * SPR_LOCK.w + x] != C_T) rect(96 + 2 * x, 39 + 2 * y, 2, 2, C_DKGRAY);
  char b[32]; snprintf(b, sizeof b, "To delete %s,", r.name); textCentered(80, 74, b, C_NAVY);
  textCentered(80, 86, "ask a grown-up", C_NAVY);
  textCentered(80, 96, "to set a code", C_NAVY);
  textCentered(80, 106, "in Admin.", C_NAVY);
  ui::drawButton({NO_DELETE_OK, "OK", nullptr, C_GREEN}, ui::pressing(in_, NO_DELETE_OK));
  ui::drawBack();
}
void Shell::drawDelete() {
  clear(C_NAVY);
  const Record& r = prof_.rec[target_];
  int ty = holdTop_ ? 98 : 30;
  char b[32]; snprintf(b, sizeof b, "Delete %s?", r.name); textCentered(80, ty, b, C_WHITE);
  textCentered(80, ty + 12, "Their pets and games", C_YELLOW);
  textCentered(80, ty + 22, "will be gone.", C_YELLOW);
  ui::drawButton({DEL_NO, "No, keep!", nullptr, C_GREEN}, ui::pressing(in_, DEL_NO));
  ui::Box h = delHold(holdTop_); bool pr = ui::pressing(in_, h);
  ui::drawButton({h, nullptr, nullptr, C_PLUM}, pr);   // white on plum and on the brown fill: both >= 4.5:1
  int fill = pr ? (int)(in_.heldMs >= HOLD_MS ? h.w - 2 : in_.heldMs * (uint32_t)(h.w - 2) / HOLD_MS) : 0;
  if (fill > 0) roundRect(h.x + 1, h.y + 2, fill, h.h - 3, C_BROWN);
  textCentered(80, h.y + (h.h - 7) / 2 + (pr ? 1 : 0), "Hold to delete", C_WHITE);
}

// ---------------------------------------------------------------- Home: back or who is playing (tap: switch), the games
// The kid's favorites, the newest star first, then "All games" (the Library), in the grid of shell/launcher.h.
// With six games or fewer switched on there is no Library: Home shows them all.
using launcher::HEADER;
using launcher::LIGHT;
static_assert(sizeof SPR_LIGHTS / sizeof SPR_LIGHTS[0] == shell::LIGHT_COUNT, "one icon per shell::Light, in its order");
int Shell::homeTiles(int* games) const {
  int m = shell::homeGames(favs_, ids_, nApps_, gamesOff_, games);
  if (shell::libraryOn(ids_, nApps_, gamesOff_)) games[m++] = -1;   // "All games" is always the last tile
  return m;
}
void Shell::updateLauncher() {
  if (ui::back(in_) || in_.tapIn(HEADER.x, HEADER.y, HEADER.w, HEADER.h)) { go(SH_PICK); return; }
  if (in_.tapIn(LIGHT.x, LIGHT.y, LIGHT.w, LIGHT.h)) { go(SH_LIGHT); return; }
  int games[MAX_APPS + 1]; const int m = homeTiles(games);
  for (int i = 0; i < m; i++) {
    const ui::Box b = launcher::tile(i, m);
    if (!in_.tapIn(b.x, b.y, b.w, b.h)) continue;
    if (games[i] >= 0) openIdx(games[i]); else { page_ = 0; go(SH_LIBRARY); }
    return;
  }
}
void Shell::drawLauncher() {
  clear(C_SKY);
  rect(0, launcher::GRASS_Y, 160, 160 - launcher::GRASS_Y, C_LEAF);
  drawHeader();
  int games[MAX_APPS + 1]; ui::Box tiles[MAX_APPS + 1]; const int m = homeTiles(games);
  for (int i = 0; i < m; i++) { tiles[i] = launcher::tile(i, m); drawAppTile(games[i], tiles[i]); }
  drawCaption(tiles, games, m);
  ui::drawBack();
}
void Shell::drawHeader() {   // who is playing, and the sun showing the level picked now
  ui::drawButton({HEADER, nullptr, nullptr, C_CREAM}, ui::pressing(in_, HEADER));
  blit(SPR_AVATARS[rec().avatar % NUM_AVATARS], HEADER.x + 3, HEADER.y + 2);
  text(HEADER.x + 26, HEADER.y + 8, rec().name, C_DKBROWN);
  const int dy = ui::pressing(in_, LIGHT) ? 1 : 0;
  ui::drawButton({LIGHT, nullptr, nullptr, C_WHITE}, dy);
  const Sprite& sun = SPR_LIGHTS[light_]; blit(sun, LIGHT.x + (LIGHT.w - sun.w) / 2, LIGHT.y + (LIGHT.h - sun.h) / 2 + dy);
}
static void caption(const ui::Box& c, const char* s) {   // yellow over the header row
  ui::drawButton({c, nullptr, nullptr, C_YELLOW}, 0);
  textCentered(c.x + (c.w + 1) / 2, c.y + (c.h - 7) / 2, s, C_DKBROWN);   // rounds up: CAPTION (odd width) centers at x 80
}
void Shell::drawCaption(const ui::Box* tiles, const int* games, int m) {   // a finger on a tile names it over the header row
  for (int i = 0; i < m; i++)   // only a press that started on it
    if (ui::pressing(in_, tiles[i])) caption(launcher::CAPTION, games[i] >= 0 ? apps_[games[i]]->name() : "All games");
}
uint32_t Shell::homeMask() const {   // the games on Home (the Library's stars), a bit each
  int home[MAX_APPS]; uint32_t mask = 0;
  for (int i = shell::homeGames(favs_, ids_, nApps_, gamesOff_, home) - 1; i >= 0; i--) mask |= shell::gameBit(ids_[home[i]]);
  return mask;
}
// A game's picture, with a red "new" dot until the kid first opens it; k < 0 is "All games". dx: star mode's jiggle,
// drawn only (b stays the target).
void Shell::drawAppTile(int k, const ui::Box& t, int dx) {
  const int dy = ui::pressing(in_, t) ? 1 : 0; const ui::Box b = {t.x + dx, t.y, t.w, t.h};
  ui::drawButton({b, nullptr, nullptr, k >= 0 ? (uint8_t)C_WHITE : (uint8_t)C_CREAM}, dy);   // a picture fills its tile inside the border
  bool dot = k >= 0 && unseen(k);
  if (k >= 0) { const Sprite& ic = apps_[k]->icon(); blit(ic, b.x + (b.w - ic.w) / 2, b.y + (b.h - ic.h) / 2 + dy); }
  else dot = drawMoreGames(b, dy);
  if (dot) { circle(b.x + 5, b.y + 5 + dy, 3, C_WHITE); circle(b.x + 5, b.y + 5 + dy, 2, C_RED); }
}
// "All games": six tiny game pictures on cream, the games not on Home first, so it reads as "more games" (a grid of
// colors read as a painting game). True while one of the games it leads to (not on Home) is new.
bool Shell::drawMoreGames(const ui::Box& b, int dy) {
  constexpr int MW = 11, MH = 8;   // a picture at about 1/3.5, nearest neighbor
  int on[MAX_APPS], pick[launcher::PER_PAGE], n = 0; bool dot = false;
  const int m = shell::enabledGames(ids_, nApps_, gamesOff_, on); const uint32_t home = homeMask();
  for (int i = 0; i < m; i++) dot |= unseen(on[i]) && !(home & shell::gameBit(ids_[on[i]]));
  for (int pass = 0; pass < 2; pass++)   // the games not on Home, then the ones on it
    for (int i = 0; i < m && n < launcher::PER_PAGE; i++)
      if (((home & shell::gameBit(ids_[on[i]])) != 0) == (pass == 1)) pick[n++] = on[i];
  for (int j = 0; j < n; j++) {
    const Sprite& ic = apps_[pick[j]]->icon(); const int x0 = b.x + 3 + (j % 3) * (MW + 1), y0 = b.y + 6 + (j / 3) * (MH + 2) + dy;
    for (int r = 0; r < MH; r++)
      for (int c = 0; c < MW; c++) {
        const uint8_t px = ic.px[((2 * r + 1) * ic.h / (2 * MH)) * ic.w + (2 * c + 1) * ic.w / (2 * MW)];
        if (px != C_T) pixel(x0 + c, y0 + r, px);
      }
  }
  return dot;
}
void Shell::drawPageArrow(bool right, int nudge) {   // a yellow button with a brown triangle pointing the way
  const ui::Box b = launcher::arrow(right);
  const int dy = ui::pressing(in_, b) ? 1 : 0, cy = b.y + b.h / 2 + dy, x0 = b.x + (right ? nudge : -nudge);
  ui::drawButton({{x0, b.y, b.w, b.h}, nullptr, nullptr, C_YELLOW}, dy);
  for (int i = 0; i < 6; i++) { const int x = right ? x0 + 9 + i : x0 + 14 - i; line(x, cy - 5 + i, x, cy + 5 - i, C_DKBROWN); }
}

// ---------------------------------------------------------------- the Library: every game switched on, six a page
// A gold star marks a favorite. The star button (the sun's place) starts star mode: a tap stars or unstars a game
// instead of opening it, up to MAX_FAVS, kept only by the green check; the home button (or the rest screen, or a
// restart) puts Home back as it was, since a kid who wandered in by accident (kid playtest 2) never meant to edit it.
void Shell::updateLibrary() {
  if (ui::back(in_)) { const bool stars = starMode_; leaveStars(false); go(stars ? SH_LIBRARY : SH_LAUNCHER); return; }
  if (in_.tapIn(LIGHT.x, LIGHT.y, LIGHT.w, LIGHT.h)) {
    if (starMode_) leaveStars(true);
    else { starMode_ = true; starsBefore_ = favs_; fullUntil_ = ms_; lastStar_ = -1; }
    go(SH_LIBRARY); return;
  }
  int on[MAX_APPS]; const int n = shell::enabledGames(ids_, nApps_, gamesOff_, on);
  if (page_ >= launcher::pages(n)) page_ = 0;
  const ui::Box l = launcher::arrow(false), r = launcher::arrow(true);   // only where there is a page that way
  if (page_ > 0 && in_.tapIn(l.x, l.y, l.w, l.h)) page_--;
  else if (page_ + 1 < launcher::pages(n) && in_.tapIn(r.x, r.y, r.w, r.h)) page_++;
  const int k0 = launcher::first(n, page_), m = launcher::count(n, page_);
  for (int i = 0; i < m; i++) {
    const ui::Box b = launcher::tile(i, m);
    if (in_.tapIn(b.x, b.y, b.w, b.h)) { tapLibrary(on[k0 + i]); return; }
  }
}
void Shell::tapLibrary(int k) {
  if (!starMode_) { openIdx(k); return; }
  // A second tap on the same tile soon after is a double tap (or mashing): one star, not a star and its undo.
  const bool again = k == lastStar_ && ms_ - lastStarMs_ < 400;
  lastStar_ = k; lastStarMs_ = ms_;
  if (again) return;
  if (shell::toggleStar(favs_, ids_[k], ids_, nApps_, gamesOff_)) fullUntil_ = ms_;   // in RAM until the check
  else fullUntil_ = ms_ + 2000;   // a sixth star: nothing changes, "Home full!" and the stars hop
}
void Shell::leaveStars(bool keep) {
  if (!starMode_) return;
  starMode_ = false;
  if (!keep) { favs_ = starsBefore_; return; }
  int home[MAX_APPS];   // no star at all is a non-reader's mashing (kid playtest 3), not a wish: the default comes back
  if (!shell::homeGames(favs_, ids_, nApps_, gamesOff_, home)) { favs_.picked = favs_.count = 0; memset(favs_.ids, 0, sizeof favs_.ids); }
  if (memcmp(&favs_, &starsBefore_, sizeof favs_)) shell::saveFavorites(*st_, active_, favs_);   // a failed write keeps it for this visit
}
// A 9x9 star, outlined: the Library's corner mark (s 1) and its button (s 2); white fill is an empty star.
static void star(int x, int y, int s, uint8_t fill = C_GOLD) {
  static const char* const ROWS[9] = {"....#....", "...###...", "...###...", "#########", ".#######.", "..#####..", "..#####..", ".###.###.", ".##...##."};
  for (int pass = 0; pass < 2; pass++)
    for (int r = 0; r < 9; r++)
      for (int c = 0; c < 9; c++)
        if (ROWS[r][c] == '#') { if (pass) rect(x + c * s, y + r * s, s, s, fill); else rect(x + c * s - 1, y + r * s - 1, s + 2, s + 2, C_DKBROWN); }
}
void Shell::drawStarButton() {   // the star that starts star mode, or the green check that ends it
  const int dy = ui::pressing(in_, LIGHT) ? 1 : 0;
  ui::drawButton({LIGHT, nullptr, nullptr, starMode_ ? (uint8_t)C_GREEN : (uint8_t)C_WHITE}, dy);
  if (!starMode_) { star(LIGHT.x + 3, LIGHT.y + 3 + dy, 2); return; }
  const uint8_t ink = ui::inkOn(C_GREEN);
  for (int t = 0; t < 3; t++) {
    line(LIGHT.x + 5, LIGHT.y + 11 + t + dy, LIGHT.x + 9, LIGHT.y + 15 + t + dy, ink);
    line(LIGHT.x + 9, LIGHT.y + 15 + t + dy, LIGHT.x + 18, LIGHT.y + 6 + t + dy, ink);
  }
}
// The title: a plain label on the sky, not a button. In star mode Home's five places as stars (gold taken, white
// free: no number to read), or for 2 s after a refused star "Home full!" in yellow, then "Tap a star", or the page
// holding them (starPage, 1-based) when this one has none; a finger on the star button names it.
void Shell::drawLibraryTitle(int starred, int starPage) {
  const int cx = HEADER.x + HEADER.w / 2;
  if (!starMode_) textCentered(cx, HEADER.y + 8, "All games", C_NAVY);
  else if (homeFull()) {
    char where[16] = "Tap a star"; if (starPage) snprintf(where, sizeof where, "Stars: page %d", starPage);
    ui::drawButton({HEADER, nullptr, nullptr, C_YELLOW}, 0);
    textCentered(cx, HEADER.y + 3, "Home full!", C_DKBROWN); textCentered(cx, HEADER.y + 13, where, C_DKBROWN);
  } else {
    for (int i = 0; i < shell::MAX_FAVS; i++) star(cx - 28 + i * 12, HEADER.y + 7, 1, i < starred ? C_GOLD : C_WHITE);
  }
  drawStarButton();
  if (ui::pressing(in_, LIGHT)) caption(HEADER, starMode_ ? "Done" : "Pick games");
}
bool Shell::drawLibraryTile(int i, int m, int k, uint32_t home, int hop) {   // true: k is on Home
  const ui::Box b = launcher::tile(i, m);
  const int dx = starMode_ ? (int)((ms_ / 200 + i) % 2) : 0;   // neighbors in turn, never left of the target
  drawAppTile(k, b, dx);
  const bool gold = home & shell::gameBit(ids_[k]);   // y + 3: a hop stays in the tile
  if (gold || starMode_) star(b.x + b.w - 11 + dx, b.y + 3 - (gold ? hop : 0), 1, gold ? C_GOLD : C_WHITE);
  return gold;
}
// In star mode every game wears a star: gold on Home, empty to pick, on a lavender sky with every tile jiggling (a
// phone's edit mode), so a kid who cannot read sees something is different. While Home is full the gold ones hop,
// and so do the arrows toward a page that has one, so they also see where to tap.
void Shell::drawLibrary() {
  clear(starMode_ ? C_LAVENDER : C_SKY);
  rect(0, launcher::GRASS_Y, 160, 160 - launcher::GRASS_Y, C_LEAF);
  int on[MAX_APPS]; const int n = shell::enabledGames(ids_, nApps_, gamesOff_, on);
  const uint32_t home = homeMask(); const int hop = homeFull() && (ms_ / 150) % 2 ? 2 : 0;
  if (page_ >= launcher::pages(n)) page_ = 0;
  const int k0 = launcher::first(n, page_), m = launcher::count(n, page_);
  int games[launcher::PER_PAGE] = {}, starred = 0; ui::Box tiles[launcher::PER_PAGE]; bool before = false, after = false;
  for (int j = 0; j < n; j++) {
    const bool gold = home & shell::gameBit(ids_[on[j]]);
    starred += gold; before |= gold && j < k0; after |= gold && j >= k0 + m;
  }
  bool here = false;   // a gold star on this page: "Tap a star", else the next page that way (1-based; MAX_APPS makes two pages at most)
  for (int i = 0; i < m; i++) { tiles[i] = launcher::tile(i, m); games[i] = on[k0 + i]; here |= drawLibraryTile(i, m, games[i], home, hop); }
  drawLibraryTitle(starred, here ? 0 : before ? page_ : page_ + 2);
  drawCaption(tiles, games, m);
  if (page_ > 0) drawPageArrow(false, before ? hop : 0);
  if (page_ + 1 < launcher::pages(n)) drawPageArrow(true, after ? hop : 0);
  ui::drawBack();
}

// ---------------------------------------------------------------- brightness (the launcher's sun): three suns, big to small,
// and Auto (a sun and a moon) under them. A tap picks, saves and goes back to the launcher, already in the new light;
// the level picked now is the mint one.
static ui::Box lightBox(int l) { return l == shell::LIGHT_AUTO ? launcher::LIGHT_AUTO : launcher::sun(l - 1); }
void Shell::setLight(shell::Light l) {
  if (l == light_) return;   // no flash write for the same level
  light_ = l; const uint8_t v = l; st_->save("sys", "lite", &v, 1);
}
void Shell::updateLight() {
  if (ui::back(in_)) { go(SH_LAUNCHER); return; }
  for (int l = 0; l < shell::LIGHT_COUNT; l++) {
    const ui::Box b = lightBox(l);
    if (in_.tapIn(b.x, b.y, b.w, b.h)) { setLight((shell::Light)l); go(SH_LAUNCHER); return; }
  }
}
void Shell::drawLight() {
  clear(C_SKY);
  textCentered(80, 40, "How bright?", C_NAVY);
  for (int l = 0; l < shell::LIGHT_COUNT; l++) {
    const ui::Box b = lightBox(l);
    const int dy = ui::pressing(in_, b) ? 1 : 0;
    ui::drawButton({b, nullptr, nullptr, l == light_ ? (uint8_t)C_MINT : (uint8_t)C_WHITE}, dy);
    const Sprite& sun = SPR_LIGHTS[l];
    if (l != shell::LIGHT_AUTO) { blitScaled(sun, b.x + (b.w - 2 * sun.w) / 2, b.y + (b.h - 2 * sun.h) / 2 + dy, 2); continue; }
    const int w = sun.w + 3 + textWidth("Auto"), x = b.x + (b.w - w) / 2;   // the icon, then its word
    blit(sun, x, b.y + (b.h - sun.h) / 2 + dy);
    text(x + sun.w + 3, b.y + (b.h - 8) / 2 + dy, "Auto", C_DKBROWN);
  }
  ui::drawBack();
}

// ---------------------------------------------------------------- resting: the play budget ran out (a turn, or the day)
void Shell::updateRest() {
  if (!restingNow()) { go(SH_PICK); return; }   // over while showing: whoever is here next picks
  if (ui::button(in_, {{50, 120, 60, 24}, "OK", nullptr, C_GREEN})) go(SH_PICK);
}
void Shell::drawRest() {
  clear(C_NAVY);
  for (int i = 0; i < 20; i++) { int x = (i * 41) % 160, y = (i * 23) % 70; if (((ms_ / 600) + i) % 3) pixel(x, y, C_WHITE); }
  blitScaled(SPR_AVATARS[rec().avatar % NUM_AVATARS], 60, 24, 2);
  textCentered(106, 20 - (int)((ms_ / 400) % 3), "z", C_YELLOW);
  textCentered(80, 70, rec().name, C_YELLOW);
  const shell::RestHint hint = shell::restHint(rec(), window_, now_, limits_);
  if (hint == shell::REST_DAY) {
    textCentered(80, 80, "played today.", C_WHITE);
    textCentered(80, 94, "Back tomorrow!", C_LTGRAY);
  } else if (hint == shell::REST_WINDOW) {
    textCentered(80, 80, "is taking a break.", C_WHITE);
    textCentered(80, 94, "Come back at", C_LTGRAY);
    const int hour = window_.startMinute / 60, minute = window_.startMinute % 60;
    char when[20];
    if (minute) snprintf(when, sizeof when, "%d:%02d %s", hour % 12 ? hour % 12 : 12, minute, hour < 12 ? "AM" : "PM");
    else snprintf(when, sizeof when, "%d %s", hour % 12 ? hour % 12 : 12, hour < 12 ? "AM" : "PM");
    textCentered(80, 106, when, C_WHITE);
  } else {
    textCentered(80, 80, "is resting.", C_WHITE);
    uint32_t left = shell::restLeft(rec(), now_, limits_);
    char b[32]; snprintf(b, sizeof b, "Back in %lu min", (unsigned long)((left + 59) / 60)); textCentered(80, 94, b, C_LTGRAY);
    textCentered(80, 106, prof_.count() >= 2 ? "Let a friend play!" : "Time for a break!", C_WHITE);
  }
  ui::drawButton({{50, 120, 60, 24}, "OK", nullptr, C_GREEN}, ui::pressing(in_, {50, 120, 60, 24}));
}

// ---------------------------------------------------------------- a game is open
void Shell::updateApp() {
  app_->update(now_, ms_, in_);
  if (app_->wantsHome()) { closeApp(); go(fromLibrary_ ? SH_LIBRARY : SH_LAUNCHER); return; }
  if (now_ != lastTick_ || ms_ - playTickMs_ >= 1000) budgetTick();
  const uint32_t sinceSave = ms_ - lastSaveMs_;   // an urgent checkpoint still waits a second: one flash write per tap at most
  if (screen_ == SH_APP) flushApp(sinceSave > 5000 || (app_->saveUrgent() && sinceSave >= 1000));
}
void Shell::drawApp() { app_->render(); }

// ---------------------------------------------------------------- serial and sim hooks
bool Shell::clearPin(int id) {
  if (id < 0 || id >= MAX_PROFILES || !prof_.used[id]) return false;
  if (!prof_.rec[id].pin) return true;
  const Record before = prof_.rec[id];
  prof_.rec[id].pin = 0;
  if (shell::saveRecord(*st_, prof_, id)) return true;
  prof_.rec[id] = before; return false;
}
const char* Shell::screenName() const {
  static const char* N[] = {"picker", "new_name", "new_avatar", "age", "pin_set", "pin_again", "pin", "delete", "no_delete", "launcher", "rest", "sys_new", "sys_info", "sys_updates",
                              "sys_wifi", "sys_pass", "sys_join", "sys_home", "sys_addr", "sys_find", "light", "library", "sys_admin", "sys_pin", "sys_pin_set", "sys_pin_again",
                              "sys_reset", "sys_remove", "sys_players", "sys_player_pin", "sys_limits", "sys_clock", "sys_time", "sys_window", "sys_allow", "sys_zone", "sys_zone_search",
                              "sys_games"};
  static_assert(sizeof N / sizeof N[0] == SH_APP, "one name per shell screen, in Screen order");
  return screen_ == SH_APP ? app_->screenName() : N[screen_];
}
void Shell::debugPrint() {
  // Every game's pet (a kid may have wandered from one game into another), the open or last one last so its keys win
  // where two games share a name, then the profile, whose keys (age, screen) win over a game's stale ones.
  for (int k = 0; k < nApps_; k++) if (opened_[k] && apps_[k] != app_) apps_[k]->debugPrint();
  if (app_) app_->debugPrint();
  const Record* r = active_ >= 0 ? &prof_.rec[active_] : nullptr;
  static const char* const TINTS[TINT_COUNT] = {"day", "evening", "night"};
  static const char* const LIGHTS[shell::LIGHT_COUNT] = {"auto", "bright", "medium", "soft"};
  printf("[shell profile=%d/%d age=%d sound=%d play=%lus today=%lu rest=%lu tint=%s bright=%s backlight=%u]\n", active_, prof_.count(),
         r ? r->age : 0, sound_ ? 1 : 0, (unsigned long)(r ? r->playSec : 0),
         (unsigned long)(r && r->playDay == now_ / shell::DAY_SEC ? r->dayPlaySec : 0),
         (unsigned long)(r ? shell::restLeft(*r, now_, limits_) : 0), TINTS[tint()], LIGHTS[light_], (unsigned)backlight());
  printf("[limits session_min=%u daily_min=%u]\n", (unsigned)limits_.sessionMinutes, (unsigned)limits_.dailyMinutes);
  printf("[hours window_on=%u window_start=%u window_end=%u allow_min=%lu]\n", (unsigned)window_.enabled, (unsigned)window_.startMinute, (unsigned)window_.endMinute, (unsigned long)allowance_.minutesLeft(ms_));
  printf("[codes pin_wait=%lu admin_wait=%lu]\n", (unsigned long)(target_ >= 0 ? pinWait(target_) : 0),
         (unsigned long)pinWait(shell::ADMIN_LOCK));
  debugHome();
  if (screen_ != SH_APP) printf("screen=%s\n", screenName());
}
void Shell::debugHome() const {   // home_first: the first tile's game by its store ("-" for "All games" or none)
  int games[MAX_APPS + 1]; const int tiles = homeTiles(games);
  printf("[home home_tiles=%d home_first=%s home_library=%d star_mode=%d home_full=%d games_off=%lu]\n", tiles,
         tiles && games[0] >= 0 ? apps_[games[0]]->store() : "-", (int)shell::libraryOn(ids_, nApps_, gamesOff_), (int)starMode_,
         (int)homeFull(), (unsigned long)gamesOff_);
}
void Shell::debugCmd(const char* cmd) {
  if (!strcmp(cmd, "anytime")) { window_.enabled = 0; shell::savePlayWindow(*st_, window_); }   // declared host test fixture
  if (!strcmp(cmd, "light")) setLight((shell::Light)((light_ + 1) % shell::LIGHT_COUNT));   // the sim's `b` key
  if (!strcmp(cmd, "unseen")) favs_.seen = 0;   // every game new to this kid until opened (RAM only): the "new" dots
  if (active_ >= 0) {
    if (!strcmp(cmd, "tired")) rec().playSec = (uint32_t)limits_.sessionMinutes * 60;
    else if (!strcmp(cmd, "bedtime")) { rec().playDay = now_ / shell::DAY_SEC; rec().dayPlaySec = (uint32_t)limits_.dailyMinutes * 60; }
    else if (!strcmp(cmd, "rested")) { rec().restUntil = 0; rec().playSec = 0; rec().dayPlaySec = 0; }
    else if (!strcmp(cmd, "younger")) rec().age = 6;
    else if (!strcmp(cmd, "older")) rec().age = 9;
  }
  if (screen_ == SH_APP) app_->debugCmd(cmd);
}
