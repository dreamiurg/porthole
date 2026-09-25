#include "shell.h"
#include <ctype.h>
#include <stdio.h>
#include <string.h>
#include "shell_sprites.h"

using namespace gfx;
using shell::Record;

void Shell::begin(shell::Store& st, App* const* apps, int nApps) {
  st_ = &st; apps_ = apps; nApps_ = nApps; app_ = nullptr; active_ = target_ = -1;
  shell::loadAll(st, prof_);
  if (prof_.count()) go(SH_PICK); else startNew();   // a fresh device goes straight to making the first profile
}
uint32_t Shell::lastSeen() const {
  uint32_t t = 0;
  for (int id = 0; id < MAX_PROFILES; id++) if (prof_.used[id] && prof_.rec[id].lastPlayed > t) t = prof_.rec[id].lastPlayed;
  return t;
}
void Shell::go(Screen s) {
  screen_ = s; screenMs_ = ms_; toastUntil_ = pinWrongUntil_ = 0; in_.tap = in_.pressed = in_.longPress = false;   // a tap acts on one screen only
  if (s == SH_PICK) active_ = -1;   // nobody is playing while the picker shows
}
void Shell::toast(const char* s) { snprintf(toast_, sizeof toast_, "%s", s); toastUntil_ = ms_ + 1500; }

using ScreenFn = void (Shell::*)();
void Shell::update(uint32_t nowSec, uint32_t ms, const Input& in) {
  static const ScreenFn UPDATE[] = {&Shell::updatePick, &Shell::updateName, &Shell::updateAvatar, &Shell::updateAge, &Shell::updatePin,
    &Shell::updatePin, &Shell::updatePin, &Shell::updateDelete, &Shell::updateLauncher, &Shell::updateRest, &Shell::updateApp};
  static_assert(sizeof UPDATE / sizeof UPDATE[0] == SH_APP + 1, "one update per screen, in Screen order");
  now_ = nowSec; ms_ = ms; in_ = in;
  tapGuard_.filter(in_, ms_, screen_);   // the game's screens count as one: it guards its own
  (this->*UPDATE[screen_])();
}
void Shell::render() {
  static const ScreenFn DRAW[] = {&Shell::drawPick, &Shell::drawName, &Shell::drawAvatar, &Shell::drawAge, &Shell::drawPin,
    &Shell::drawPin, &Shell::drawPin, &Shell::drawDelete, &Shell::drawLauncher, &Shell::drawRest, &Shell::drawApp};
  static_assert(sizeof DRAW / sizeof DRAW[0] == SH_APP + 1, "one draw per screen, in Screen order");
  (this->*DRAW[screen_])();
  if (ms_ < toastUntil_) ui::toast(toast_);
}
Tint Shell::tint() const { return screen_ == SH_APP ? app_->tint() : screen_ == SH_REST ? TINT_EVENING : TINT_DAY; }
bool Shell::soundOn(uint32_t ms) { return screen_ == SH_APP && app_->soundOn(ms) && !prof_.rec[active_].muted; }

// ---------------------------------------------------------------- flow
void Shell::startNew() {
  memset(&draft_, 0, sizeof draft_); nameLen_ = 0; name_[0] = 0; creating_ = true;
  go(SH_NAME);
}
void Shell::choose(int id, bool del) {
  target_ = id; deleting_ = del;
  if (prof_.rec[id].pin) { resetPin(); go(SH_PIN); } else authorized();
}
void Shell::authorized() { if (deleting_) go(SH_DELETE); else select(target_); }
void Shell::select(int id) {
  active_ = id;
  shell::recharge(rec(), now_);
  if (!rec().age) { creating_ = false; go(SH_AGE); return; }   // migrated from a v1 Pets Club save: ask once
  go(shell::resting(rec(), now_) ? SH_REST : SH_LAUNCHER);
}
void Shell::finishCreate(uint16_t pin) {
  draft_.pin = pin;
  const char* stores[MAX_APPS]; int n = appStores(stores);
  int id = shell::create(*st_, prof_, draft_, stores, n);
  if (id < 0) { go(SH_PICK); return; }   // full: the picker hides "+" at four, so only a sim script gets here
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
  for (int id = 0; id < MAX_PROFILES; id++) if (!prof_.used[id]) { draft_.avatar = (uint8_t)(id % NUM_AVATARS); break; }
  finishCreate(shell::pinCode(code));
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
  for (int k = 0; k < nApps_; k++) if (sameName(apps_[k]->name(), name)) { openIdx(k); return true; }
  return false;
}
void Shell::openIdx(int k) {
  app_ = apps_[k];
  Profile all[MAX_PROFILES]; SaveSlot saves[MAX_PROFILES]; int n = 0, who = 0;
  for (int id; (id = prof_.nth(n)) >= 0; n++) {
    all[n] = shell::toProfile(prof_, id);
    saves[n] = {blobs_[n], st_->load(app_->store(), shell::key('s', id), blobs_[n], shell::BLOB_MAX)};
    if (id == active_) who = n;
  }
  app_->enter({&all[who], all, saves, n, now_, ms_});
  lastTick_ = lastRecordSave_ = now_; lastSaveMs_ = ms_;
  go(SH_APP);
}
void Shell::flushApp(bool allowed) {
  const void* data; size_t len;
  if (!app_->takeSave(&data, &len, allowed)) return;
  if (len) st_->save(app_->store(), shell::key('s', active_), data, len); else st_->erase(app_->store(), shell::key('s', active_));
  lastSaveMs_ = ms_;
}
void Shell::closeApp() {
  app_->leave();
  flushApp(true);
  shell::saveRecord(*st_, prof_, active_);
}
void Shell::budgetTick() {   // per profile, across every game; only counts with someone to hand over to
  uint32_t dt = now_ - lastTick_; lastTick_ = now_;
  if (shell::play(rec(), now_, dt, prof_.count())) { closeApp(); go(SH_REST); return; }
  if (now_ - lastRecordSave_ >= 60) { shell::saveRecord(*st_, prof_, active_); lastRecordSave_ = now_; }   // checkpoint
}

// ---------------------------------------------------------------- Who's playing? (one row per profile, "+" below)
static ui::Box pickRow(int i, int rows) { return {28, 80 - (rows * 26 - 2) / 2 + i * 26, 104, 24}; }
void Shell::updatePick() {
  int n = prof_.count(), rows = n < MAX_PROFILES ? n + 1 : n;
  for (int i = 0; i < rows; i++) {
    ui::Box b = pickRow(i, rows);
    bool tap = in_.tapIn(b.x, b.y, b.w, b.h);
    if (i == n) { if (tap) startNew(); }
    else if (tap) choose(prof_.nth(i), false);
    else if (in_.longPress && in_.hit(b.x, b.y, b.w, b.h)) choose(prof_.nth(i), true);   // long press: delete
    if (screen_ != SH_PICK) return;
  }
}
void Shell::drawPick() {
  clear(C_SKY);
  textCentered(80, 18, "Who's playing?", C_NAVY);
  int n = prof_.count(), rows = n < MAX_PROFILES ? n + 1 : n;
  for (int i = 0; i < rows; i++) {
    ui::Box b = pickRow(i, rows);
    int dy = in_.down && in_.hit(b.x, b.y, b.w, b.h) ? 1 : 0;
    ui::drawButton({b, nullptr, nullptr, i == n ? (uint8_t)C_MINT : (uint8_t)C_CREAM}, dy);
    if (i == n) { blitTint(SPR_PLUS, 58, b.y + 8 + dy, C_DKBROWN); text(72, b.y + 9 + dy, "New", C_DKBROWN); continue; }
    const Record& r = prof_.rec[prof_.nth(i)];
    blit(SPR_AVATARS[r.avatar % NUM_AVATARS], b.x + 4, b.y + 2 + dy);
    text(b.x + 30, b.y + 9 + dy, r.name, C_DKBROWN);
    if (r.pin) blitTint(SPR_LOCK, b.x + b.w - 12, b.y + 8 + dy, C_DKGRAY);
  }
}

// ---------------------------------------------------------------- new profile: name -> face -> age -> code
void Shell::updateName() {
  if (ui::back(in_)) { go(SH_PICK); return; }
  if (ui::keyboard(in_, name_, nameLen_)) { snprintf(draft_.name, sizeof draft_.name, "%s", name_); go(SH_AVATAR); }
}
void Shell::drawName() { clear(C_WALL); ui::drawKeyboard(in_, name_, "Your name?", ms_); ui::drawBack(); }

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
    int dy = in_.down && in_.hit(b.x, b.y, b.w, b.h) ? 1 : 0;
    ui::drawButton({{b.x + 1, b.y + 1, b.w - 2, b.h - 4}, nullptr, nullptr, dy ? (uint8_t)C_YELLOW : (uint8_t)C_WHITE}, dy);
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
    ui::Box b = ageButton(i); char l[4]; snprintf(l, sizeof l, i == 5 ? "10+" : "%d", AGES[i]);
    bool pr = in_.down && in_.hit(b.x, b.y, b.w, b.h);
    uint8_t col = i < 2 ? C_GREEN : i < 4 ? C_BLUE : C_PLUM;
    ui::drawButton({b, nullptr, nullptr, col}, pr);
    textCentered(b.x + 20, b.y + 5 + (pr ? 1 : 0), l, ui::inkOn(col), 2);
  }
  ui::drawBack();
}

// ---------------------------------------------------------------- secret code: set twice (or skip), or asked on the picker
static const int PIN_KX[3] = {44, 68, 92}, PIN_KY[4] = {52, 72, 92, 112};
static ui::Box pinKey(int k) {   // the bottom-left key (skip / backspace) is wider: "skip" does not fit 20 px
  return k == 9 ? ui::Box{32, PIN_KY[3], 32, 18} : ui::Box{PIN_KX[k % 3], PIN_KY[k / 3], 20, 18};
}
Shell::Key Shell::keypad(bool leftIsSkip) {
  for (int k = 0; k < 12; k++) {
    ui::Box b = pinKey(k);
    if (!in_.tapIn(b.x - 2, b.y - 2, b.w + 4, b.h + 4)) continue;
    if (k == 9) { if (leftIsSkip) return KEY_LEFT; if (pinLen_ > 0) pin_[--pinLen_] = 0; return KEY_NONE; }
    if (k == 11) return pinLen_ == 4 ? KEY_OK : KEY_NONE;
    if (pinLen_ < 4) { pin_[pinLen_++] = (char)(k == 10 ? '0' : '1' + k); pin_[pinLen_] = 0; }
    return KEY_NONE;
  }
  return KEY_NONE;
}
void Shell::updatePin() {
  static const Screen BACK[3] = {SH_AGE, SH_PIN_SET, SH_PICK};   // from SH_PIN_SET, SH_PIN_AGAIN, SH_PIN
  if (ui::back(in_)) { resetPin(); go(BACK[screen_ - SH_PIN_SET]); return; }
  Key key = keypad(screen_ == SH_PIN_SET);
  if (key == KEY_LEFT) { finishCreate(0); return; }   // "skip": no code
  if (key != KEY_OK) return;
  uint16_t code = shell::pinCode(pin_);
  resetPin();
  if (screen_ == SH_PIN_SET) { firstPin_ = code; go(SH_PIN_AGAIN); }
  else if (screen_ == SH_PIN_AGAIN && code == firstPin_) finishCreate(code);
  else if (screen_ == SH_PIN_AGAIN) { go(SH_PIN_SET); toast("Not the same. Again!"); }
  else if (code == prof_.rec[target_].pin) authorized();
  else { pinWrongUntil_ = ms_ + 900; toast("Hmm, that's not it"); }
}
void Shell::drawPin() {
  clear(C_WALL);
  char prompt[32];
  if (screen_ == SH_PIN) snprintf(prompt, sizeof prompt, "%s's code?", prof_.rec[target_].name);
  else snprintf(prompt, sizeof prompt, "%s", screen_ == SH_PIN_SET ? "Pick 4 numbers" : "Once more!");
  textCentered(80, 28, prompt, C_DKBROWN);
  bool wrong = ms_ < pinWrongUntil_; int shake = wrong ? ((ms_ / 50) % 2 ? 1 : -1) : 0;
  for (int i = 0; i < 4; i++) {
    int x = 58 + i * 12 + shake; rect(x, 38, 10, 12, C_WHITE); frame(x, 38, 10, 12, wrong ? C_RED : C_DKBROWN);
    if (i >= pinLen_) continue;
    if (screen_ == SH_PIN) circle(x + 5, 44, 2, C_NAVY); else { char l[2] = {pin_[i], 0}; textCentered(x + 5, 40, l, C_NAVY); }
  }
  for (int k = 0; k < 12; k++) {
    char l[8]; uint8_t col = C_WHITE;
    if (k < 9) snprintf(l, sizeof l, "%d", k + 1); else if (k == 10) snprintf(l, sizeof l, "0");
    else if (k == 9) { snprintf(l, sizeof l, "%s", screen_ == SH_PIN_SET ? "skip" : "<"); col = C_LTGRAY; }
    else { snprintf(l, sizeof l, "OK"); col = pinLen_ == 4 ? C_GREEN : C_LTGRAY; }
    drawPinKey(k, l, col);
  }
  ui::drawBack();
}
void Shell::drawPinKey(int k, const char* l, uint8_t col) {
  ui::Box b = pinKey(k); int dy = in_.down && in_.hit(b.x - 2, b.y - 2, b.w + 4, b.h + 4) ? 1 : 0;
  roundRect(b.x, b.y + 1, b.w, b.h, C_DKBROWN); roundRect(b.x, b.y + dy, b.w, b.h, C_DKBROWN); roundRect(b.x + 1, b.y + 1 + dy, b.w - 2, b.h - 2, dy ? (uint8_t)C_YELLOW : col);
  textCentered(b.x + b.w / 2, b.y + 5 + dy, l, C_DKBROWN);
}

// ---------------------------------------------------------------- delete a profile (long press on the picker)
void Shell::updateDelete() {
  if (ui::button(in_, {{20, 90, 50, 24}, "No!", nullptr, C_GREEN})) { go(SH_PICK); return; }
  if (!ui::button(in_, {{90, 90, 50, 24}, "Yes", nullptr, C_RED})) return;
  const char* stores[MAX_APPS]; int n = appStores(stores);
  shell::removeProfile(*st_, prof_, target_, stores, n);
  if (prof_.count()) go(SH_PICK); else startNew();
}
void Shell::drawDelete() {
  clear(C_PLUM);
  const Record& r = prof_.rec[target_];
  char b[32]; snprintf(b, sizeof b, "Delete %s?", r.name); textCentered(80, 40, b, C_WHITE);
  textCentered(80, 56, "Their pets and games", C_YELLOW);
  textCentered(80, 66, "will be gone.", C_YELLOW);
  ui::drawButton({{20, 90, 50, 24}, "No!", nullptr, C_GREEN}, in_.down && in_.hit(20, 90, 50, 24));
  ui::drawButton({{90, 90, 50, 24}, "Yes", nullptr, C_RED}, in_.down && in_.hit(90, 90, 50, 24));
  blit(SPR_AVATARS[r.avatar % NUM_AVATARS], 70, 124);
}

// ---------------------------------------------------------------- launcher: back or who is playing (tap: switch), the games, mute
static const ui::Box HEADER = {30, 30, 100, 24};   // 2 px below the home button's hit box (rows -4..27)
static ui::Box appButton(int k, int n) { return {56 - (n - 1) * 28 + k * 56, 57, 48, 44}; }
void Shell::updateLauncher() {
  if (ui::back(in_) || in_.tapIn(HEADER.x, HEADER.y, HEADER.w, HEADER.h)) { go(SH_PICK); return; }
  if (ui::iconButton(in_, 80, 128, rec().muted ? SPR_SOUND_OFF : SPR_SOUND_ON, rec().muted ? C_DKGRAY : C_GREEN)) {
    rec().muted = !rec().muted; shell::saveRecord(*st_, prof_, active_);
  }
  for (int k = 0; k < nApps_; k++) {
    ui::Box b = appButton(k, nApps_);
    if (in_.tapIn(b.x, b.y, b.w, b.h)) { openIdx(k); return; }
  }
}
void Shell::drawLauncher() {
  clear(C_SKY);
  rect(0, 116, 160, 44, C_LEAF);
  ui::drawButton({HEADER, nullptr, nullptr, C_CREAM}, in_.down && in_.hit(HEADER.x, HEADER.y, HEADER.w, HEADER.h));
  blit(SPR_AVATARS[rec().avatar % NUM_AVATARS], HEADER.x + 4, HEADER.y + 2);
  text(HEADER.x + 28, HEADER.y + 8, rec().name, C_DKBROWN);
  for (int k = 0; k < nApps_; k++) {
    ui::Box b = appButton(k, nApps_);
    int dy = in_.down && in_.hit(b.x, b.y, b.w, b.h) ? 1 : 0;
    ui::drawButton({b, nullptr, nullptr, C_WHITE}, dy);
    const Sprite& ic = apps_[k]->icon(); blit(ic, b.x + (b.w - ic.w) / 2, b.y + (b.h - ic.h) / 2 + dy);
    textCenteredShadow(b.x + b.w / 2, b.y + b.h + 4, apps_[k]->name(), C_NAVY, C_WHITE);
  }
  ui::iconButton(in_, 80, 128, rec().muted ? SPR_SOUND_OFF : SPR_SOUND_ON, rec().muted ? C_DKGRAY : C_GREEN);
  ui::drawBack();
}

// ---------------------------------------------------------------- resting: the play budget ran out
void Shell::updateRest() {
  if (!shell::resting(rec(), now_)) { go(SH_LAUNCHER); return; }
  if (ui::button(in_, {{50, 120, 60, 24}, "OK", nullptr, C_GREEN})) go(SH_PICK);
}
void Shell::drawRest() {
  clear(C_NAVY);
  for (int i = 0; i < 20; i++) { int x = (i * 41) % 160, y = (i * 23) % 70; if (((ms_ / 600) + i) % 3) pixel(x, y, C_WHITE); }
  blitScaled(SPR_AVATARS[rec().avatar % NUM_AVATARS], 60, 24, 2);
  textCentered(106, 20 - (int)((ms_ / 400) % 3), "z", C_YELLOW);
  textCentered(80, 70, rec().name, C_YELLOW);
  textCentered(80, 80, "is resting.", C_WHITE);
  uint32_t left = rec().restUntil > now_ ? rec().restUntil - now_ : 0;
  char b[32]; snprintf(b, sizeof b, "Back in %lu min", (unsigned long)((left + 59) / 60)); textCentered(80, 94, b, C_LTGRAY);
  textCentered(80, 106, "Let a friend play!", C_WHITE);
  ui::drawButton({{50, 120, 60, 24}, "OK", nullptr, C_GREEN}, in_.down && in_.hit(50, 120, 60, 24));
}

// ---------------------------------------------------------------- a game is open
void Shell::updateApp() {
  app_->update(now_, ms_, in_);
  if (app_->wantsHome()) { closeApp(); go(SH_LAUNCHER); return; }
  if (now_ != lastTick_) budgetTick();
  if (screen_ == SH_APP) flushApp(ms_ - lastSaveMs_ > 5000);
}
void Shell::drawApp() { app_->render(); }

// ---------------------------------------------------------------- serial and sim hooks
void Shell::clearPin(int id) {
  if (id < 0 || id >= MAX_PROFILES || !prof_.used[id]) return;
  prof_.rec[id].pin = 0; shell::saveRecord(*st_, prof_, id);
}
const char* Shell::screenName() const {
  static const char* N[] = {"picker", "new_name", "new_avatar", "age", "pin_set", "pin_again", "pin", "delete", "launcher", "rest"};
  return screen_ == SH_APP ? app_->screenName() : N[screen_];
}
void Shell::debugPrint() {
  // The open game first (or the last one: its pet's stats stay checkable from the shell), then the profile, whose
  // keys (age, screen) win over a game's stale ones.
  if (app_) app_->debugPrint();
  const Record* r = active_ >= 0 ? &prof_.rec[active_] : nullptr;
  printf("[shell profile=%d/%d age=%d muted=%d play=%lus rest=%lu]\n", active_, prof_.count(), r ? r->age : 0, r ? r->muted : 0,
         (unsigned long)(r ? r->playSec : 0), (unsigned long)(r && shell::resting(*r, now_) ? r->restUntil - now_ : 0));
  if (screen_ != SH_APP) printf("screen=%s\n", screenName());
}
void Shell::debugCmd(const char* cmd) {
  if (active_ >= 0) {
    if (!strcmp(cmd, "tired")) rec().playSec = shell::SESSION_SEC;
    else if (!strcmp(cmd, "rested")) { rec().restUntil = 0; rec().playSec = 0; }
    else if (!strcmp(cmd, "younger")) rec().age = 6;
    else if (!strcmp(cmd, "older")) rec().age = 9;
  }
  if (screen_ == SH_APP) app_->debugCmd(cmd);
}
