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
  gate_.closed = false;   // boot: no finger to guard against
}
uint32_t Shell::lastSeen() const {
  uint32_t t = 0;
  for (int id = 0; id < MAX_PROFILES; id++) if (prof_.used[id] && prof_.rec[id].lastPlayed > t) t = prof_.rec[id].lastPlayed;
  return t;
}
void Shell::go(Screen s) {
  screen_ = s; screenMs_ = ms_; toastUntil_ = pinWrongUntil_ = 0; in_.tap = in_.pressed = in_.longPress = false;   // a tap acts on one screen only
  gate_.shown(ms_);
  if (s == SH_PICK) active_ = -1;   // nobody is playing while the picker shows
}
void Shell::toast(const char* s) { snprintf(toast_, sizeof toast_, "%s", s); toastUntil_ = ms_ + 1500; }

using ScreenFn = void (Shell::*)();
void Shell::update(uint32_t nowSec, uint32_t ms, const Input& in) {
  static const ScreenFn UPDATE[] = {&Shell::updatePick, &Shell::updateName, &Shell::updateAvatar, &Shell::updateAge, &Shell::updatePin,
    &Shell::updatePin, &Shell::updatePin, &Shell::updateDelete, &Shell::updateLauncher, &Shell::updateRest, &Shell::updateApp};
  static_assert(sizeof UPDATE / sizeof UPDATE[0] == SH_APP + 1, "one update per screen, in Screen order");
  now_ = nowSec; ms_ = ms; in_ = in;
  gate_.filter(in_, ms_);   // the game's screens count as one: it gates its own
  const ScreenFn fn = UPDATE[screen_];   // never (this->*TABLE[i])(): gcc 13.3/14.2 -fsanitize=bounds on aarch64 miscompiles it
  (this->*fn)();
}
void Shell::render() {
  static const ScreenFn DRAW[] = {&Shell::drawPick, &Shell::drawName, &Shell::drawAvatar, &Shell::drawAge, &Shell::drawPin,
    &Shell::drawPin, &Shell::drawPin, &Shell::drawDelete, &Shell::drawLauncher, &Shell::drawRest, &Shell::drawApp};
  static_assert(sizeof DRAW / sizeof DRAW[0] == SH_APP + 1, "one draw per screen, in Screen order");
  const ScreenFn fn = DRAW[screen_];     // see update()
  (this->*fn)();
  bool pinScreen = screen_ >= SH_PIN_SET && screen_ <= SH_PIN;   // drawPin shows its messages in the prompt line
  if (ms_ < toastUntil_ && !pinScreen) ui::toast(toast_);
}
Tint Shell::tint() const {   // the game's light, or the clock's: a night game never hands back to a daylight launcher
  if (screen_ == SH_APP) return app_->tint();
  Tint t = clockTint(now_);
  return screen_ == SH_REST && t == TINT_DAY ? TINT_EVENING : t;   // resting is never broad daylight
}
bool Shell::soundOn(uint32_t ms) { return screen_ == SH_APP && app_->soundOn(ms) && !prof_.rec[active_].muted; }

// ---------------------------------------------------------------- flow
void Shell::startNew() {
  memset(&draft_, 0, sizeof draft_); nameLen_ = 0; name_[0] = 0; namePage_ = 0; creating_ = true;
  go(SH_NAME);
}
void Shell::choose(int id, bool del) {
  target_ = id; deleting_ = del;
  if (prof_.rec[id].pin) { resetPin(); go(SH_PIN); } else authorized();
}
void Shell::authorized() {
  if (!deleting_) { select(target_); return; }
  holdTop_ = in_.y >= 80; go(SH_DELETE);   // the finger (long press or the code's OK) is where "No" is safe
}
bool Shell::restingNow() const { return prof_.count() >= 2 && shell::resting(prof_.rec[active_], now_); }
void Shell::select(int id) {
  active_ = id;
  shell::recharge(rec(), now_);
  if (!rec().age) { creating_ = false; go(SH_AGE); return; }   // migrated from a v1 Pets Club save: ask once
  go(restingNow() ? SH_REST : SH_LAUNCHER);
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
  for (int k = 0; k < nApps_; k++) if (sameName(apps_[k]->name(), name)) { openIdx(k); gate_.closed = false; return true; }
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
static ui::Box pickRow(int i, int rows) { return {24, 80 - (rows * 26 - 2) / 2 + i * 26, 112, 24}; }   // an 8-letter name clears the lock
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
    int dy = in_.down && in_.hit(b.x, b.y, b.w, b.h) ? 1 : 0;
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
    bool pr = in_.down && in_.hit(b.x, b.y, b.w, b.h);
    uint8_t col = i < 2 ? C_GREEN : i < 4 ? C_BLUE : C_PLUM, ink = ui::inkOn(col); int y = b.y + 5 + (pr ? 1 : 0);
    ui::drawButton({b, nullptr, nullptr, col}, pr);
    if (i < 5) { textCentered(b.x + 20, y, l, ink, 2); continue; }
    int w = textWidth(l, 2) + 1 + textWidth("+");   // "10+" at full size fills the button edge to edge: a small +
    text(text(b.x + 20 - w / 2, y, l, ink, 2) - 1, y, "+", ink);
  }
  ui::drawBack();
}

// ---------------------------------------------------------------- secret code: set twice (or skip), or asked on the picker
// Keys are drawn at their real 24x22 size, 2 px apart: 1-9, then backspace, 0, OK. "skip" (no code, only while
// choosing one) sits to the right of the pad; messages replace the prompt so they never cover a key.
static ui::Box pinKey(int k) { return {40 + (k % 3) * 26, 48 + (k / 3) * 24, 24, 22}; }   // rows 48..142
static const ui::Box PIN_SKIP = {121, 72, 34, 22};
Shell::Key Shell::keypad(bool canSkip) {
  if (canSkip && in_.tapIn(PIN_SKIP.x, PIN_SKIP.y, PIN_SKIP.w, PIN_SKIP.h)) return KEY_SKIP;
  for (int k = 0; k < 12; k++) {
    ui::Box b = pinKey(k);
    if (!in_.tapIn(b.x, b.y, b.w, b.h)) continue;
    if (k == 9) { if (pinLen_ > 0) pin_[--pinLen_] = 0; return KEY_NONE; }
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
  if (key == KEY_SKIP) { finishCreate(0); return; }   // "skip": no code
  if (key != KEY_OK) return;
  uint16_t code = shell::pinCode(pin_);
  resetPin();
  if (screen_ == SH_PIN_SET) { firstPin_ = code; go(SH_PIN_AGAIN); }
  else if (screen_ == SH_PIN_AGAIN && code == firstPin_) finishCreate(code);
  else if (screen_ == SH_PIN_AGAIN) { go(SH_PIN_SET); toast("Not the same!"); }
  else if (code == prof_.rec[target_].pin) authorized();
  else { pinWrongUntil_ = ms_ + 900; toast("Hmm, that's not it"); }
}
void Shell::drawPin() {
  clear(C_WALL);
  char prompt[sizeof toast_];   // holds a whole toast: the message takes the prompt's place
  if (ms_ < toastUntil_) snprintf(prompt, sizeof prompt, "%s", toast_);   // the message takes the prompt's place
  else if (screen_ == SH_PIN) snprintf(prompt, sizeof prompt, "%s's code?", prof_.rec[target_].name);
  else snprintf(prompt, sizeof prompt, "%s", screen_ == SH_PIN_SET ? "Pick 4 numbers" : "Once more!");
  textCentered(80, 26, prompt, ms_ < toastUntil_ ? (uint8_t)C_PLUM : (uint8_t)C_DKBROWN);
  drawPinDigits();
  static const char* const LABELS[12] = {"1", "2", "3", "4", "5", "6", "7", "8", "9", "<", "0", "OK"};
  for (int k = 0; k < 12; k++) drawPinKey(pinKey(k), LABELS[k], k == 9 || (k == 11 && pinLen_ < 4) ? C_LTGRAY : k == 11 ? C_GREEN : C_WHITE);
  if (screen_ == SH_PIN_SET) drawPinKey(PIN_SKIP, "skip", C_SKY);
  ui::drawBack();
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
  int dy = in_.down && in_.hit(b.x, b.y, b.w, b.h) ? 1 : 0;
  roundRect(b.x, b.y + 1, b.w, b.h, C_DKBROWN); roundRect(b.x, b.y + dy, b.w, b.h, C_DKBROWN); roundRect(b.x + 1, b.y + 1 + dy, b.w - 2, b.h - 2, dy ? (uint8_t)C_YELLOW : col);
  textCentered(b.x + b.w / 2, b.y + 7 + dy, l, C_DKBROWN);
}

// ---------------------------------------------------------------- delete a profile (long press on the picker)
// "No" is big and central; deleting takes a deliberate HOLD_MS press on a button in the half the finger was not on,
// so neither the long press that got here nor one more tap can delete.
static const uint32_t HOLD_MS = 900;   // under the sim's 1 s `hold`, still a clear press-and-hold for a kid
static const ui::Box DEL_NO = {30, 66, 100, 28};
static ui::Box delHold(bool top) { return {32, top ? 30 : 104, 96, 24}; }
void Shell::updateDelete() {
  if (ui::button(in_, {DEL_NO, "No, keep!", nullptr, C_GREEN})) { go(SH_PICK); return; }
  ui::Box h = delHold(holdTop_);
  bool began = in_.downX >= h.x && in_.downX < h.x + h.w && in_.downY >= h.y && in_.downY < h.y + h.h;
  if (!(in_.down && in_.hit(h.x, h.y, h.w, h.h) && began && in_.heldMs >= HOLD_MS)) return;
  const char* stores[MAX_APPS]; int n = appStores(stores);
  shell::removeProfile(*st_, prof_, target_, stores, n);
  if (prof_.count()) go(SH_PICK); else startNew();
}
void Shell::drawDelete() {
  clear(C_NAVY);
  const Record& r = prof_.rec[target_];
  int ty = holdTop_ ? 98 : 30;
  char b[32]; snprintf(b, sizeof b, "Delete %s?", r.name); textCentered(80, ty, b, C_WHITE);
  textCentered(80, ty + 12, "Their pets and games", C_YELLOW);
  textCentered(80, ty + 22, "will be gone.", C_YELLOW);
  ui::drawButton({DEL_NO, "No, keep!", nullptr, C_GREEN}, in_.down && in_.hit(DEL_NO.x, DEL_NO.y, DEL_NO.w, DEL_NO.h));
  ui::Box h = delHold(holdTop_); bool pr = in_.down && in_.hit(h.x, h.y, h.w, h.h);
  ui::drawButton({h, nullptr, nullptr, C_PLUM}, pr);   // white on plum and on the brown fill: both >= 4.5:1
  int fill = pr ? (int)(in_.heldMs >= HOLD_MS ? h.w - 2 : in_.heldMs * (uint32_t)(h.w - 2) / HOLD_MS) : 0;
  if (fill > 0) roundRect(h.x + 1, h.y + 2, fill, h.h - 3, C_BROWN);
  textCentered(80, h.y + (h.h - 7) / 2 + (pr ? 1 : 0), "Hold to delete", C_WHITE);
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
  if (!restingNow()) { go(SH_PICK); return; }   // over while showing: whoever is here next picks
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
  static const char* const TINTS[TINT_COUNT] = {"day", "evening", "night"};
  printf("[shell profile=%d/%d age=%d muted=%d play=%lus rest=%lu tint=%s]\n", active_, prof_.count(), r ? r->age : 0, r ? r->muted : 0,
         (unsigned long)(r ? r->playSec : 0), (unsigned long)(r && restingNow() ? r->restUntil - now_ : 0), TINTS[tint()]);
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
