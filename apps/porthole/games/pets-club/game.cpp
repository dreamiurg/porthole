#include "game.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "content.h"
#include "sprites.h"

using namespace gfx;

// ---------------------------------------------------------------- small helpers
static int clampi(int v, int lo, int hi) { return v < lo ? lo : v > hi ? hi : v; }
static int absi(int v) { return v < 0 ? -v : v; }
static const int HOME_FLOOR_Y = 92, DOG_BASE_Y = 116;
static const int BTN_Y = 118, BTN_W = 24, BTN_H = 22;
static const int BTN_X[4] = {28, 54, 80, 106};

void Game::begin(uint32_t nowSec, uint32_t ms, const Save* houses, int count) {
  now_ = nowSec; ms_ = ms;
  nHouses_ = count > MAX_HOUSES ? MAX_HOUSES : count; active_ = -1; haveSave_ = false; dirty_ = false; eraseSlot_ = -1;
  for (int i = 0; i < nHouses_; i++) { houses_[i] = houses[i]; dirtyHouse_[i] = false; }
  memset(&save_, 0, sizeof save_);
  lastTickSec_ = now_; dogX_ = dogTargetX_ = 60; dogAct_ = 0;
  go(SC_SPLASH);
}

// Themes: one color scheme per house, chosen at adoption.
struct Theme { uint8_t wall, dots, floor, plank, rug, rugAlt, rugEdge, roof; };
static const Theme THEMES[4] = {
  {C_WALL, C_PEACH, C_FLOOR, C_WOOD, C_ROSE, C_PINK, C_PLUM, C_RED},
  {C_MINT, C_WHITE, C_TAN, C_WOOD, C_BLUE, C_SKY, C_NAVY, C_DKGREEN},
  {C_LAVENDER, C_WHITE, C_TAN, C_WOOD, C_GOLD, C_YELLOW, C_ORANGE, C_PLUM},
  {C_SKY, C_WHITE, C_WOOD, C_DKWOOD, C_ORANGE, C_YELLOW, C_RED, C_BLUE},
};
static const Theme& themeOf(const Save& s) { return THEMES[s.theme & 3]; }
static void drawHouseIcon(int cx, int groundY, int w, const Theme& th, bool lit) {
  int h = w * 7 / 8, top = groundY - h;
  rect(cx - w / 2, top, w, h, th.wall); frame(cx - w / 2, top, w, h, C_DKBROWN);
  int rh = w / 2 + 2;
  for (int r = 0; r < rh; r++) hline(cx - w / 2 - 3 + r * (w + 6) / (2 * rh), top - rh + r, (w + 6) - 2 * (r * (w + 6) / (2 * rh)), th.roof);
  int dw = w / 4; rect(cx - dw / 2, groundY - h / 2, dw, h / 2, C_DKWOOD); pixel(cx + dw / 2 - 2, groundY - h / 4, C_GOLD);
  int ww = w / 4; rect(cx + w / 2 - ww - 3, top + 4, ww, ww, lit ? C_YELLOW : C_SKY); frame(cx + w / 2 - ww - 3, top + 4, ww, ww, C_DKWOOD);
}

void Game::enterHouse(int i) {
  if (active_ >= 0) leaveHouse();
  active_ = i; save_ = houses_[i]; haveSave_ = true;
  uint32_t away = now_ > save_.lastSeen ? now_ - save_.lastSeen : 0;
  uint32_t ev = pet::simulate(save_, now_, false);
  if (away >= pet::REST_SEC) save_.playSec = 0;  // a proper break recharges the play budget
  lastTickSec_ = now_; stickersSeen_ = save_.stickersMask; refreshBookList();
  dogX_ = dogTargetX_ = 60; dogAct_ = 0; event_ = 0; trickShow_ = -1;
  markDirty();
  go(pet::resting(save_, now_) ? SC_REST : SC_HOME);
  if (ev & pet::EV_LONG_AWAY) {
    char buf[40];
    if (save_.asleep) snprintf(buf, sizeof buf, "%s is napping", save_.petName); else snprintf(buf, sizeof buf, "%s missed you!", save_.petName);
    toast(buf, 3000);
  }
}
void Game::leaveHouse() {
  if (active_ < 0) return;
  houses_[active_] = save_; if (dirty_) dirtyHouse_[active_] = true;
  dirty_ = false; active_ = -1; haveSave_ = false;
}
void Game::finishAdoption() {
  char kid[12], name[12]; strncpy(kid, save_.kidName, 12); strncpy(name, save_.petName, 12);
  uint8_t age = save_.kidAge, theme = save_.theme; uint16_t pin = save_.pin;
  Save s; pet::adopt(s, now_, kid, name); s.kidAge = age; s.theme = theme; s.pin = pin; pet::seal(s);
  int i = nHouses_++; houses_[i] = s; dirtyHouse_[i] = true;
  enterHouse(i);
  char t[24], b[48]; snprintf(t, sizeof t, "Welcome home!"); snprintf(b, sizeof b, "%s and %s, best friends.", s.kidName, s.petName);
  celebrate(t, b, 0, SC_HOME);
}
const Book& Game::curBook() const { return BOOKS[bookList_[bookSel_ < nBooks_ ? bookSel_ : 0]]; }
bool Game::takeErase(int* slot) { if (eraseSlot_ < 0) return false; *slot = eraseSlot_; eraseSlot_ = -1; return true; }
void Game::clearPin(int slot) {
  if (slot == active_) { save_.pin = 0; markDirty(); }
  else if (slot >= 0 && slot < nHouses_) { houses_[slot].pin = 0; dirtyHouse_[slot] = true; }
}

void Game::go(Screen s) { screen_ = s; screenMs_ = ms_; toastUntil_ = 0; in_.tap = in_.pressed = in_.longPress = false; }  // a tap acts on one screen only
void Game::toast(const char* s, uint32_t ms) { strncpy(toast_, s, sizeof toast_ - 1); toast_[sizeof toast_ - 1] = 0; toastUntil_ = ms_ + ms; }

Tint Game::tint() const {
  if (screen_ <= SC_NAME_PET || screen_ == SC_AGE || screen_ == SC_THEME || screen_ == SC_PIN_SET || screen_ == SC_PIN_ENTER) return TINT_DAY;
  if (screen_ == SC_REST) return TINT_EVENING;
  if (save_.asleep && screen_ == SC_HOME) return TINT_NIGHT;
  int h = pet::hourOf(now_);
  if (h >= 21 || h < 6) return TINT_NIGHT;
  if (h >= 18 || h < 7) return TINT_EVENING;
  return TINT_DAY;
}

bool Game::takeSave(Save* out, int* slot, bool allowed) {
  if (!allowed) return false;
  if (dirty_ && active_ >= 0) { pet::seal(save_); houses_[active_] = save_; *out = save_; *slot = active_; dirty_ = false; return true; }
  for (int i = 0; i < nHouses_; i++) if (dirtyHouse_[i]) { pet::seal(houses_[i]); *out = houses_[i]; *slot = i; dirtyHouse_[i] = false; return true; }
  return false;
}

bool Game::soundOn(uint32_t ms) {
  if (sound_ == SND_NONE || save_.muted) return false;
  uint32_t t = ms - soundStartMs_;
  // patterns: pairs of (on, off) ms
  // The on-board buzzer is a harsh single-tone piezo: keep every pattern short and rare.
  static const uint16_t TAP[] = {15, 0};
  static const uint16_t YIP[] = {20, 40, 20, 0};
  static const uint16_t HAPPY[] = {25, 50, 25, 0};
  static const uint16_t SAD[] = {60, 0};
  static const uint16_t CHIME[] = {20, 40, 20, 40, 40, 0};
  static const uint16_t BARK[] = {30, 0};
  static const uint16_t FANFARE[] = {25, 40, 25, 40, 25, 40, 70, 0};
  const uint16_t* p = TAP;
  switch (sound_) { case SND_YIP: p = YIP; break; case SND_HAPPY: p = HAPPY; break; case SND_SAD: p = SAD; break;
    case SND_CHIME: p = CHIME; break; case SND_BARK: p = BARK; break; case SND_FANFARE: p = FANFARE; break; default: break; }
  uint32_t acc = 0;
  for (int i = 0; ; i += 2) {
    if (p[i] == 0) { sound_ = SND_NONE; return false; }
    if (t < acc + p[i]) return true;
    acc += p[i];
    if (t < acc + p[i + 1]) return false;
    acc += p[i + 1];
  }
}

void Game::debugCmd(const char* cmd) {
  if (!strcmp(cmd, "dirty")) save_.dirty = 1;
  else if (!strcmp(cmd, "poop")) { save_.poop = 1; save_.poopSince = now_; }
  else if (!strcmp(cmd, "hungry")) { save_.food = 20; save_.fun = 25; save_.energy = 22; }
  else if (!strcmp(cmd, "hearts")) save_.bond = 700;
  else if (!strcmp(cmd, "books")) { save_.booksUnlocked = NUM_BOOKS; save_.booksDoneMask = 0x3F; save_.booksRead = 6; }
  else if (!strcmp(cmd, "tricks")) { for (int i = 0; i < 4; i++) save_.trickProgress[i] = 3; }
  else if (!strcmp(cmd, "hats")) save_.hatsMask = 0xFE;
  else if (!strcmp(cmd, "grown")) save_.adoptedAt -= 10 * 86400;
  else if (!strcmp(cmd, "dog")) save_.adoptedAt -= 4 * 86400;
  else if (!strcmp(cmd, "sleepy")) save_.energy = 15;
  else if (!strcmp(cmd, "tired")) save_.playSec = pet::SESSION_SEC;
  else if (!strcmp(cmd, "rested")) { save_.restUntil = 0; save_.playSec = 0; }
  else if (!strcmp(cmd, "younger")) { save_.kidAge = 6; refreshBookList(); }
  else if (!strcmp(cmd, "older")) { save_.kidAge = 9; refreshBookList(); }
  markDirty();
}
const char* Game::screenName() const {
  static const char* N[] = {"splash", "intro", "name_kid", "name_pet", "home", "feed", "playmenu", "fetch", "words", "library", "read",
                            "tricks", "train", "bath", "stats", "stickers", "gift", "celebrate", "confirm_reset", "hats", "street", "age",
                            "theme", "pin_set", "pin_enter", "rest"};
  return N[screen_];
}
void Game::debugPrint() {
  if (screen_ == SC_TRAIN) printf("train: trick=%d phase=%d round=%d len=%d seq=%d%d%d%d%d%d input=%d\n", trickSel_, trainPhase_, trainRound_, seqLen_, seq_[0], seq_[1], seq_[2], seq_[3], seq_[4], seq_[5], seqInput_);
  printf("[%s/%s age=%d house=%d/%d play=%lus rest=%lu] day=%d stage=%d food=%d fun=%d energy=%d clean=%d bond=%d hearts=%d streak=%d books=%d/%d asleep=%d poop=%d dirty=%d gift=%d screen=%d\n",
         save_.kidName, save_.petName, save_.kidAge, active_, nHouses_, (unsigned long)save_.playSec, (unsigned long)(pet::resting(save_, now_) ? save_.restUntil - now_ : 0), pet::ageDays(save_, now_), save_.stage, save_.food, save_.fun, save_.energy, save_.clean,
         save_.bond, pet::hearts(save_), save_.streak, save_.booksRead, save_.booksUnlocked, save_.asleep, save_.poop, save_.dirty,
         pet::giftReady(save_, now_), (int)screen_);
  printf("screen=%s tricks=%d%d%d%d%d%d%d%d hat=%d stickers=%u\n", screenName(), save_.trickProgress[0], save_.trickProgress[1], save_.trickProgress[2], save_.trickProgress[3],
         save_.trickProgress[4], save_.trickProgress[5], save_.trickProgress[6], save_.trickProgress[7], save_.hat, (unsigned)save_.stickersMask);
  if (screen_ == SC_WORDS && !wordsDone_) printf("word=%s cols=%d typed=%d tiles=%.*s\n", WORDS[wordIdx_[wordRound_]].word, wordCols_, wordTyped_, wordCols_ * 3, tiles_);
  if (screen_ == SC_LIBRARY || screen_ == SC_READ) { const Book& bk = curBook(); int np = 0; while (bk.pages[np]) np++; printf("book=%d title=\"%s\" pages=%d page=%d correct=%d\n", bookList_[bookSel_], bk.title, np, page_, bk.correct); }
  if (screen_ == SC_FETCH) printf("fetch score=%d left=%d\n", fetchScore_, fetchDone_ ? 0 : (int)((fetchEndMs_ - ms_) / 1000));
  if (screen_ == SC_BATH) printf("bath spots=%d\n", spotsLeft_);
}

// ---------------------------------------------------------------- particles
void Game::spawn(int x, int y, int kind, int n) {
  for (int i = 0; i < 24 && n > 0; i++) if (!parts_[i].life) {
    Particle& p = parts_[i]; p.x = (int16_t)x; p.y = (int16_t)y; p.kind = (uint8_t)kind;
    int r = (int)(pet::rnd(save_) % 7) - 3;
    p.vx = (int8_t)r; p.vy = (int8_t)(kind == 2 ? 1 + (int)(pet::rnd(save_) % 3) : -(1 + (int)(pet::rnd(save_) % 3)));
    p.life = (uint8_t)(18 + (pet::rnd(save_) % 12)); n--;
  }
}
void Game::updateParticles() {
  for (auto& p : parts_) if (p.life) { p.x = (int16_t)(p.x + p.vx / 2); p.y = (int16_t)(p.y + p.vy); if (p.kind == 0) p.vy++; if ((p.life & 3) == 0 && p.vx) p.vx += p.vx > 0 ? -1 : 1; p.life--; }
}
void Game::drawParticles() {
  for (auto& p : parts_) if (p.life) {
    switch (p.kind) {
      case 0: blit(SPR_HEART, p.x - 3, p.y - 3); break;            // hearts
      case 1: blit(SPR_SPARKLE, p.x - 2, p.y - 2); break;          // sparkles
      case 2: blit(SPR_DROP, p.x - 1, p.y - 2); break;             // water
      case 3: blit(SPR_BUBBLE, p.x - 2, p.y - 2); break;           // bubbles
      case 4: rect(p.x, p.y, 2, 2, (uint8_t)(C_RED + (p.life % 6))); break;  // confetti
      case 5: blit(SPR_NOTE, p.x - 3, p.y - 4); break;             // music
      case 6: pixel(p.x, p.y, C_BROWN); pixel(p.x + 1, p.y, C_TAN); break;  // crumbs
    }
  }
}

// ---------------------------------------------------------------- dog drawing
int Game::dogSize() const { return save_.stage == STAGE_GROWN ? SZ_BIG : save_.stage == STAGE_DOG ? SZ_DOG : SZ_PUP; }
const Sprite& Game::dogFrame(int pose) const { return DOG_FRAMES[dogSize()][pose]; }
void Game::drawDog(int x, int y, int pose, bool flip, int scale) {
  // x,y = bottom-left of the sprite's bounding box (feet on y)
  const Sprite& s = dogFrame(pose);
  int top = y - s.h * scale;
  if (scale == 1) blit(s, x, top, flip); else blitScaled(s, x, top, scale, flip);
  if (save_.dirty) {  // mud spots
    const DogParts& dp = DOG_PARTS[dogSize()][pose];
    int bx = flip ? s.w - dp.bodyX : dp.bodyX;
    blit(SPR_MUD, x + (bx - 3) * scale, top + (dp.bodyY - 1) * scale);
    blit(SPR_MUD, x + (bx + 2) * scale, top + (dp.bodyY + 2) * scale);
  }
  drawHat(x, top, pose, flip, scale);
}
void Game::drawHat(int x, int top, int pose, bool flip, int scale) {
  int hat = save_.hat;
  if (!hat) {  // personality accessory once grown
    if (save_.stage >= STAGE_DOG && save_.variant == VAR_BOOKWORM) hat = 3;
    else if (save_.stage >= STAGE_DOG && save_.variant == VAR_SPORTY) hat = 4;
  }
  if (!hat || pose == P_BACK || pose == P_DEAD) return;
  const DogParts& dp = DOG_PARTS[dogSize()][pose];
  const Sprite& s = dogFrame(pose);
  const Sprite& h = SPR_HATS[hat];
  if (hat == 3) {  // glasses sit on the eye
    int ex = flip ? s.w - 1 - dp.eyeX : dp.eyeX;
    int gx = x + ex * scale - 1 * scale, gy = top + dp.eyeY * scale - 1 * scale;
    if (scale == 1) blit(h, flip ? gx - 3 : gx, gy, flip); else blitScaled(h, flip ? gx - 3 * scale : gx, gy, scale, flip);
    return;
  }
  if (hat == 4) {  // bandana hangs from the neck
    int nx = flip ? s.w - dp.headX : dp.headX;
    int bx = x + (nx - h.w / 2 - (flip ? 3 : -3)) * scale, by = top + (dp.headY + dp.headR - 2) * scale;
    if (scale == 1) blit(h, bx, by, flip); else blitScaled(h, bx, by, scale, flip);
    return;
  }
  int hx = flip ? s.w - 1 - dp.headX : dp.headX;
  int px = x + (hx - h.w / 2 + (flip ? -1 : 1)) * scale;
  int py = top + (dp.headY - dp.headR - h.h + 2) * scale;
  if (scale == 1) blit(h, px, py, flip); else blitScaled(h, px, py, scale, flip);
}

// ---------------------------------------------------------------- UI widgets
void Game::drawPanel(int x, int y, int w, int h, uint8_t fill, uint8_t border) {
  roundRect(x, y, w, h, border); roundRect(x + 1, y + 1, w - 2, h - 2, fill);
}
static uint8_t inkOn(uint8_t fill) {  // readable text color for a fill: dark on bright, white on dark
  uint32_t c = PALETTE_RGB[fill & 31];
  int lum = (((c >> 16) & 255) * 299 + ((c >> 8) & 255) * 587 + (c & 255) * 114) / 1000;
  return lum > 120 ? C_DKBROWN : C_WHITE;
}
void Game::drawButton(int x, int y, int w, int h, const char* label, const Sprite* icon, uint8_t col, bool pressed) {
  int dy = pressed ? 1 : 0;
  if (!pressed) roundRect(x, y + 2, w, h, C_DKBROWN);   // shadow
  roundRect(x, y + dy, w, h, C_DKBROWN);
  roundRect(x + 1, y + 1 + dy, w - 2, h - 2, col);
  hline(x + 2, y + 1 + dy, w - 4, C_WHITE);
  int cy = y + dy + 2;
  if (icon) { blit(*icon, x + (w - icon->w) / 2, cy); cy += icon->h + 1; }
  if (label) textCentered(x + w / 2, label && icon ? y + dy + h - 9 : y + dy + (h - 7) / 2, label, inkOn(col));
}
bool Game::button(int x, int y, int w, int h, const char* label, const Sprite* icon, uint8_t col) {
  bool pressed = in_.down && in_.hit(x, y, w, h);
  drawButton(x, y, w, h, label, icon, col, pressed);
  return in_.tapIn(x, y, w, h);
}
bool Game::iconButton(int cx, int cy, int r, const Sprite& icon, uint8_t col, uint8_t iconCol) {
  circle(cx, cy + 1, r, C_DKBROWN); circle(cx, cy, r, C_DKBROWN); circle(cx, cy, r - 1, col);
  if (iconCol) blitTint(icon, cx - icon.w / 2, cy - icon.h / 2, iconCol); else blit(icon, cx - icon.w / 2, cy - icon.h / 2);
  return in_.tapInCircle(cx, cy, r + 4);
}
void Game::drawBackButton() { circle(80, 12, 11, C_DKBROWN); circle(80, 11, 10, C_ORANGE); blitTint(SPR_HOME, 76, 7, C_WHITE); }
bool Game::backButton() { drawBackButton(); return in_.tapInCircle(80, 12, 16); }
void Game::drawToast() {
  if (ms_ >= toastUntil_ || !toast_[0]) return;
  int w = textWidth(toast_) + 10;
  drawPanel(80 - w / 2, 62, w, 14, C_WHITE, C_DKBROWN);
  textCentered(80, 65, toast_, C_DKBROWN);
}
void Game::drawHearts(int cx, int y, int hearts) {
  int x = cx - (10 * 8) / 2;
  for (int i = 0; i < 10; i++) blit(i < hearts ? SPR_HEART : SPR_HEART_EMPTY, x + i * 8, y);
}
void Game::drawStatPips(int x, int y, int value, const Sprite& icon, uint8_t col) {
  blit(icon, x, y - 1);
  int pips = (value + 12) / 25; if (pips > 4) pips = 4;
  for (int i = 0; i < 4; i++) rect(x + icon.w + 2 + i * 4, y + 1, 3, 5, i < pips ? col : (uint8_t)C_LTGRAY);
  if (value < 30 && (ms_ / 400) % 2 == 0) frame(x + icon.w + 1, y, 17, 7, C_RED);
}

// ---------------------------------------------------------------- update / render dispatch
void Game::update(uint32_t nowSec, uint32_t ms, const Input& in) {
  now_ = nowSec; ms_ = ms; in_ = in;
  // A double tap whose first tap changed the screen must not act on whatever now sits under the finger.
  static uint32_t lastTapMs = 0; static int lastTapX = 0, lastTapY = 0; static Screen lastTapScreen = SC_SPLASH;
  if (in_.tap) {
    bool sameSpot = absi(in_.x - lastTapX) < 12 && absi(in_.y - lastTapY) < 12;
    if (sameSpot && ms_ - lastTapMs < 300 && lastTapScreen != screen_) in_.tap = false;
    else { lastTapMs = ms_; lastTapX = in_.x; lastTapY = in_.y; lastTapScreen = screen_; }
  }
  if (haveSave_ && now_ != lastTickSec_) {
    uint32_t ev = pet::simulate(save_, now_, true);
    if (ev & pet::EV_POOPED) { }
    if (ev & pet::EV_STAGE_UP) {
      char t[24], b[48];
      snprintf(t, sizeof t, "%s grew up!", save_.petName);
      snprintf(b, sizeof b, save_.stage == STAGE_GROWN ? "All grown up. What a good dog!" : "Not a puppy anymore!");
      celebrate(t, b, 1, SC_HOME);
    }
    if ((now_ % 300) == 0) markDirty();  // periodic checkpoint; actions save on their own
    // turn taking: the play budget only counts inside a house, and only when there is someone to hand over to
    if (nHouses_ >= 2 && screen_ >= SC_HOME && screen_ <= SC_HATS && !pet::resting(save_, now_)) {
      uint32_t dt = now_ - lastTickSec_; if (dt > 5) dt = 5;
      save_.playSec += dt;
    }
    lastTickSec_ = now_;
  }
  if (in_.pressed && haveSave_ && screen_ != SC_SPLASH) pet::touchDay(save_, now_);
  updateParticles();
  switch (screen_) {
    case SC_SPLASH: updateSplash(); break;
    case SC_INTRO: updateIntro(); break;
    case SC_NAME_KID: case SC_NAME_PET: updateKeyboard(); break;
    case SC_HOME: updateHome(); break;
    case SC_FEED: updateFeed(); break;
    case SC_PLAYMENU: updatePlayMenu(); break;
    case SC_FETCH: updateFetch(); break;
    case SC_WORDS: updateWords(); break;
    case SC_LIBRARY: updateLibrary(); break;
    case SC_READ: updateRead(); break;
    case SC_TRICKS: updateTricks(); break;
    case SC_TRAIN: updateTrain(); break;
    case SC_BATH: updateBath(); break;
    case SC_STATS: updateStats(); break;
    case SC_STICKERS: updateStickers(); break;
    case SC_GIFT: updateGift(); break;
    case SC_CELEBRATE: updateCelebrate(); break;
    case SC_CONFIRM_RESET: updateConfirmReset(); break;
    case SC_HATS: updateHats(); break;
    case SC_STREET: updateStreet(); break;
    case SC_AGE: updateAge(); break;
    case SC_THEME: updateTheme(); break;
    case SC_PIN_SET: case SC_PIN_ENTER: updatePin(); break;
    case SC_REST: updateRest(); break;
  }
  if (screen_ == SC_HOME) checkStickers();
}

void Game::render() {
  switch (screen_) {
    case SC_SPLASH: drawSplash(); break;
    case SC_INTRO: drawIntro(); break;
    case SC_NAME_KID: drawKeyboard("Your name?"); break;
    case SC_NAME_PET: drawKeyboard("Puppy's name?"); break;
    case SC_HOME: drawHome(); break;
    case SC_FEED: drawFeed(); break;
    case SC_PLAYMENU: drawPlayMenu(); break;
    case SC_FETCH: drawFetch(); break;
    case SC_WORDS: drawWords(); break;
    case SC_LIBRARY: drawLibrary(); break;
    case SC_READ: drawRead(); break;
    case SC_TRICKS: drawTricks(); break;
    case SC_TRAIN: drawTrain(); break;
    case SC_BATH: drawBath(); break;
    case SC_STATS: drawStats(); break;
    case SC_STICKERS: drawStickers(); break;
    case SC_GIFT: drawGift(); break;
    case SC_CELEBRATE: drawCelebrate(); break;
    case SC_CONFIRM_RESET: drawConfirmReset(); break;
    case SC_HATS: drawHats(); break;
    case SC_STREET: drawStreet(); break;
    case SC_AGE: drawAge(); break;
    case SC_THEME: drawTheme(); break;
    case SC_PIN_SET: drawPin("Secret code?"); break;
    case SC_PIN_ENTER: { static char pr[32]; snprintf(pr, sizeof pr, "%s's code?", pinSlot_ >= 0 ? houses_[pinSlot_].kidName : ""); drawPin(pr); break; }
    case SC_REST: drawRest(); break;
  }
  drawParticles();
  drawToast();
}

void Game::checkStickers() {
  uint16_t fresh = (uint16_t)(save_.stickersMask & ~stickersSeen_);
  if (!fresh) return;
  for (int i = 0; i < NUM_STICKERS; i++) if (fresh & (1u << i)) {
    stickersSeen_ |= (uint16_t)(1u << i);
    char t[48]; snprintf(t, sizeof t, "You earned the %s sticker!", STICKER_NAMES[i]);
    celebrate("New sticker!", t, 2, screen_);
    markDirty();
    return;
  }
}
void Game::celebrate(const char* title, const char* text, int icon, Screen next) {
  strncpy(celebTitle_, title, sizeof celebTitle_ - 1); strncpy(celebText_, text, sizeof celebText_ - 1);
  celebIcon_ = icon; nextAfterCelebrate_ = next == SC_CELEBRATE ? SC_HOME : next;
  toastUntil_ = 0;
  sound(SND_FANFARE); spawn(80, 60, 4, 16); go(SC_CELEBRATE);
}

// ---------------------------------------------------------------- splash
void Game::updateSplash() {
  if (ms_ - screenMs_ > 1800 || in_.tap) {
    if (nHouses_ == 0) { memset(&save_, 0, sizeof save_); go(SC_INTRO); }
    else if (nHouses_ == 1 && !houses_[0].pin) enterHouse(0);
    else go(SC_STREET);
  }
}
void Game::drawSplash() {
  clear(C_NAVY);
  for (int i = 0; i < 24; i++) { int x = (i * 53) % 160, y = (i * 29) % 160; if (((ms_ / 300) + i) % 4) pixel(x, y, C_WHITE); }
  blitTint(SPR_PAW, 36, 44, C_PEACH); blitTint(SPR_PAW, 116, 44, C_PEACH);
  // two stacked lines read as a logo; paws sit outside "PETS" (58 px at scale 2)
  textCenteredShadow(80, 38, "PETS", C_YELLOW, C_PLUM, 2);
  textCenteredShadow(80, 56, "CLUB", C_YELLOW, C_PLUM, 2);
  textCentered(80, 80, "a pixel puppy", C_LTGRAY);
  blit(DOG_FRAMES[SZ_DOG][(ms_ / 500) % 2 ? P_IDLE1 : P_IDLE0], 64, 92);
  if ((ms_ / 500) % 2) textCentered(80, 126, "tap to start", C_WHITE);
}

// ---------------------------------------------------------------- intro (adoption)
void Game::updateIntro() {
  // phase encoded in dogAct_: 0 box wobbling, 1 opened
  if (dogAct_ == 0 && in_.tap) { dogAct_ = 1; screenMs_ = ms_; spawn(80, 96, 1, 12); }
  else if (dogAct_ == 1 && ms_ - screenMs_ > 900 && in_.tap) { dogAct_ = 0; nameLen_ = 0; nameBuf_[0] = 0; memset(&save_, 0, sizeof save_); go(SC_NAME_KID); }
}
void Game::drawIntro() {
  clear(C_SKY);
  rect(0, 100, 160, 60, C_LEAF); rect(0, 100, 160, 2, C_DKGREEN);
  blit(SPR_CLOUD, 20 + (ms_ / 200) % 60, 30); blit(SPR_SUN, 112, 20);
  if (dogAct_ == 0) {
    int wob = ((ms_ / 250) % 2) ? 1 : -1;
    blitScaled(SPR_PARCEL, 66 + wob, 76, 2);
    textCentered(80, 40, "Knock knock!", C_NAVY);
    textCentered(80, 52, "A package for you.", C_NAVY);
    if ((ms_ / 500) % 2) textCentered(80, 136, "tap to open", C_NAVY);
  } else {
    rect(64, 96, 32, 8, C_WOOD); rect(60, 88, 40, 6, C_DKWOOD);
    int hop = ms_ - screenMs_ < 600 ? -(int)((600 - (ms_ - screenMs_)) / 60) : 0;
    blit(DOG_FRAMES[SZ_PUP][(ms_ / 300) % 2 ? P_JUMP : P_IDLE1], 68, 72 + hop);
    textCentered(80, 40, "A puppy!", C_NAVY);
    textCentered(80, 52, "It needs a home.", C_NAVY);
    if (ms_ - screenMs_ > 900 && (ms_ / 500) % 2) textCentered(80, 136, "tap to adopt", C_NAVY);
  }
}

// ---------------------------------------------------------------- keyboard
static const char* KB_ROWS[4] = {"ABCDEFG", "HIJKLMN", "OPQRSTU", "VWXYZ"};
void Game::updateKeyboard() {
  bool petScreen = screen_ == SC_NAME_PET;
  for (int r = 0; r < 4; r++) {
    for (int c = 0; KB_ROWS[r][c]; c++) {
      int x = 17 + c * 18, y = 54 + r * 18;
      if (in_.tapIn(x - 1, y - 1, 18, 18) && nameLen_ < 8) { char ch = KB_ROWS[r][c]; nameBuf_[nameLen_] = nameLen_ ? (char)(ch + 32) : ch; nameBuf_[++nameLen_] = 0; }
    }
  }
  if (in_.tapIn(107 - 1, 108 - 1, 18, 18) && nameLen_ > 0) { nameBuf_[--nameLen_] = 0; }          // backspace
  if (petScreen && in_.tapIn(122, 30, 20, 20)) {  // random name
    const char* n = PET_NAME_IDEAS[pet::rnd(save_) % 9]; strncpy(nameBuf_, n, 11); nameLen_ = (int)strlen(nameBuf_);
  }
  if (in_.tapIn(125 - 1, 108 - 1, 20, 18) && nameLen_ > 0) {  // OK

    if (!petScreen) { strncpy(save_.kidName, nameBuf_, 11); nameLen_ = 0; nameBuf_[0] = 0; ageReturn_ = SC_NAME_PET; go(SC_AGE); }
    else { strncpy(save_.petName, nameBuf_, 11); go(SC_THEME); }
  }
}
void Game::drawKeyboard(const char* prompt) {
  clear(C_WALL);
  textCentered(80, 20, prompt, C_DKBROWN);
  drawPanel(40, 32, 80, 16, C_WHITE, C_DKBROWN);
  char shown[14]; snprintf(shown, sizeof shown, "%s%s", nameBuf_, (ms_ / 400) % 2 ? "_" : " ");
  text(45, 36, shown, C_NAVY);
  if (screen_ == SC_NAME_PET) { circle(132, 40, 9, C_DKBROWN); circle(132, 39, 8, C_PINK); textCentered(132, 36, "?", C_DKBROWN); }
  for (int r = 0; r < 4; r++) {
    for (int c = 0; KB_ROWS[r][c]; c++) {
      int x = 17 + c * 18, y = 54 + r * 18; char l[2] = {KB_ROWS[r][c], 0};
      bool pr = in_.down && in_.hit(x - 1, y - 1, 18, 18);
      roundRect(x, y + (pr ? 1 : 0), 16, 16, C_DKBROWN); roundRect(x + 1, y + 1 + (pr ? 1 : 0), 14, 14, pr ? C_YELLOW : C_WHITE);
      textCentered(x + 8, y + 4 + (pr ? 1 : 0), l, C_DKBROWN);
    }
  }
  roundRect(107, 108, 16, 16, C_DKBROWN); roundRect(108, 109, 14, 14, C_LTGRAY); blitTint(SPR_ARROW, 113, 111, C_DKBROWN, true);
  roundRect(125, 108, 18, 16, C_DKBROWN); roundRect(126, 109, 16, 14, nameLen_ ? C_GREEN : C_LTGRAY); textCentered(134, 112, "OK", C_DKBROWN);
}

// ---------------------------------------------------------------- home
void Game::drawWindow(int x, int y, int w, int h) {
  int hr = pet::hourOf(now_); bool night = hr >= 20 || hr < 6;
  rect(x - 2, y - 2, w + 4, h + 4, C_DKWOOD);
  rect(x, y, w, h, night ? C_NIGHT : C_SKY);
  if (night) { for (int i = 0; i < 8; i++) { int sx = x + 3 + (i * 11) % (w - 6), sy = y + 3 + (i * 7) % (h - 8); if (((ms_ / 700) + i) % 3) pixel(sx, sy, C_WHITE); } blit(SPR_MOON, x + w - 12, y + 3); }
  else {
    blit(SPR_SUN, x + 3, y + 3);
    int cx = x + (int)((ms_ / 300) % (uint32_t)(w + 12)) - 12;
    for (int i = 0; i < SPR_CLOUD.h; i++) for (int j = 0; j < SPR_CLOUD.w; j++) { uint8_t c = SPR_CLOUD.px[i * SPR_CLOUD.w + j]; int px = cx + j; if (c != C_T && px >= x && px < x + w) pixel(px, y + 14 + i, c); }
  }
  rect(x, y + h - 6, w, 6, night ? C_DKGREEN : C_LEAF);
  if (weather_ == 1) for (int i = 0; i < 10; i++) { int rx = x + 2 + (i * 7) % (w - 4), ry = y + (int)((ms_ / 40 + i * 13) % (uint32_t)(h - 6)); rect(rx, ry, 1, 3, C_WATER); }
  if (event_ == 2) blit(SPR_SQUIRREL, x + w - 16, y + h - 15);
  rect(x + w / 2 - 1, y, 2, h, C_DKWOOD); rect(x, y + h / 2 - 1, w, 2, C_DKWOOD);
}
void Game::drawShelf(int x, int y) {
  rect(x, y, 36, 52, C_WOOD); frame(x, y, 36, 52, C_DKWOOD);
  for (int s = 0; s < 3; s++) {
    int sy = y + 4 + s * 16; rect(x + 2, sy + 12, 32, 2, C_DKWOOD);
    for (int b = 0; b < 5; b++) {
      int idx = s * 5 + b; if (idx >= save_.booksUnlocked || idx >= nBooks_) break;
      uint8_t col = BOOKS[bookList_[idx]].cover; int h = 10 + (idx % 3);
      rect(x + 3 + b * 6, sy + 12 - h, 5, h, col); pixel(x + 5 + b * 6, sy + 12 - h + 2, C_WHITE);
    }
  }
  blit(SPR_HEART, x + 27, y + 2);  // a little decoration on top shelf
}
void Game::drawRoom() {
  bool night = save_.asleep;
  const Theme& th = themeOf(save_);
  rect(0, 0, 160, HOME_FLOOR_Y, th.wall);
  for (int y = 6; y < HOME_FLOOR_Y; y += 12) for (int x = (y / 12) % 2 ? 6 : 0; x < 160; x += 12) pixel(x, y, th.dots);  // wallpaper dots
  rect(0, HOME_FLOOR_Y, 160, 160 - HOME_FLOOR_Y, th.floor);
  rect(0, HOME_FLOOR_Y, 160, 2, C_DKWOOD);
  for (int y = HOME_FLOOR_Y + 8; y < 160; y += 8) hline(0, y, 160, th.plank);
  // rug
  rect(46, 108, 68, 12, th.rug); frame(46, 108, 68, 12, th.rugEdge); checker(50, 111, 60, 6, th.rug, th.rugAlt);
  // front door (to the street)
  rect(20, 40, 14, 26, C_DKWOOD); rect(22, 42, 10, 24, C_WOOD); rect(24, 44, 6, 8, C_DKWOOD); pixel(30, 55, C_GOLD);
  drawWindow(58, 30, 44, 34);
  drawShelf(112, 40);
  blitScaled(night ? SPR_LAMP_OFF : SPR_LAMP_ON, 14, 68, 2);
  if (!night) { for (int i = 0; i < 3; i++) pixel(32 + i * 2, 70 + i, C_YELLOW); }
  blitScaled(SPR_BOWL, 20, 100, 2);
  blitScaled(SPR_BALL, 124, 98, 2);
}
int Game::idlePose() {
  if (save_.asleep) return (ms_ / 1000) % 2 ? P_SLEEP1 : P_SLEEP0;
  Want w = pet::computeWant(save_, now_);
  if (dogAct_ == 1) return (ms_ / 150) % 2 ? P_WALK0 : P_WALK1;          // walking
  if (dogAct_ == 2) return P_SIT;
  if (dogAct_ == 3) return (ms_ / 250) % 2 ? P_JUMP : P_IDLE1;           // excited
  if (dogAct_ == 4) return P_BARK;
  if (dogAct_ == 5) return P_LISTEN;
  if (w == WANT_FOOD || w == WANT_POOP || save_.fun < 35) return (ms_ / 900) % 3 == 0 ? P_IDLE0 : P_SAD;
  if (w == WANT_SLEEP) return (ms_ / 1200) % 2 ? P_SIT : P_LISTEN;
  int wag = save_.fun > 60 ? 350 : 800;
  return (ms_ / wag) % 2 ? P_IDLE1 : P_IDLE0;
}
void Game::startTrickShow(int trick) { trickShow_ = trick; dogAct_ = 10; dogActUntil_ = ms_ + 2600; }

void Game::updateHome() {
  if (save_.kidAge == 0) { ageReturn_ = SC_HOME; go(SC_AGE); return; }   // houses from before the age question
  if (pet::resting(save_, now_)) { go(SC_REST); return; }
  if (nHouses_ >= 2 && pet::sessionExpired(save_)) { save_.restUntil = now_ + pet::REST_SEC; save_.playSec = 0; markDirty(); go(SC_REST); return; }
  if (in_.tapIn(16, 36, 22, 32)) { leaveHouse(); go(SC_STREET); return; }   // the front door
  Want want = pet::computeWant(save_, now_);
  int dogW = dogFrame(P_IDLE0).w;
  // idle brain
  if (!save_.asleep && dogAct_ != 10 && ms_ >= nextIdleMs_) {
    int r = (int)(pet::rnd(save_) % 100);
    if (r < 35) { dogAct_ = 1; dogTargetX_ = 16 + (int)(pet::rnd(save_) % (uint32_t)(112 - dogW)); dogFlip_ = dogTargetX_ < dogX_; }
    else if (r < 50) dogAct_ = 2;
    else if (r < 58 && pet::learnedTrickCount(save_) > 0) { int t; do { t = (int)(pet::rnd(save_) % NUM_TRICKS); } while (!pet::trickLearned(save_, t)); startTrickShow(t); }
    else dogAct_ = 0;
    nextIdleMs_ = ms_ + 2500 + pet::rnd(save_) % 4000;
    if (dogAct_ == 1) nextIdleMs_ = ms_ + 6000;
  }
  if (dogAct_ == 1) {
    if (dogX_ < dogTargetX_) dogX_++; else if (dogX_ > dogTargetX_) dogX_--; else { dogAct_ = 0; }
    if ((ms_ / 33) % 2) { if (dogX_ < dogTargetX_) dogX_++; else if (dogX_ > dogTargetX_) dogX_--; }
  }
  if (dogAct_ == 10 && ms_ >= dogActUntil_) { dogAct_ = 0; trickShow_ = -1; if (!save_.asleep) { save_.tricksShown++; pet::addBond(save_, now_, 1); markDirty(); } }
  if ((dogAct_ == 3 || dogAct_ == 4 || dogAct_ == 5) && ms_ >= dogActUntil_) dogAct_ = 0;
  // random events
  if (!save_.asleep && ms_ >= nextEventMs_) {
    int r = (int)(pet::rnd(save_) % 100);
    if (event_ == 0) {
      if (r < 40) { event_ = 1; eventX_ = -10; eventY_ = 40 + (int)(pet::rnd(save_) % 30); eventUntil_ = ms_ + 9000; }
      else if (r < 65) { event_ = 2; eventUntil_ = ms_ + 4500; dogAct_ = 4; dogActUntil_ = ms_ + 1500; dogFlip_ = false; }
      else if (r < 80) { weather_ = weather_ ? 0 : 1; }
    }
    nextEventMs_ = ms_ + 25000 + pet::rnd(save_) % 40000;
  }
  if (event_ == 1) { eventX_ += (ms_ / 33) % 3 ? 1 : 0; if (eventX_ > 170 || ms_ > eventUntil_) event_ = 0; }
  if (event_ == 2 && ms_ > eventUntil_) event_ = 0;

  // touches: every hotspot is tested each frame (the UI audit relies on it); only a tap acts
  {
    int dogTop = DOG_BASE_Y - dogFrame(P_IDLE0).h;
    bool onDog = in_.tapIn(dogX_ - 4, dogTop - 6, dogW + 8, dogFrame(P_IDLE0).h + 8);
    if (event_ == 1 && in_.tapIn(eventX_ - 8, eventY_ - 8, 24, 22)) {  // caught the butterfly's attention
      event_ = 0; dogAct_ = 3; dogActUntil_ = ms_ + 1500; spawn(in_.x, in_.y, 1, 6);
      save_.fun = (uint8_t)clampi(save_.fun + 5, 0, 100); pet::addBond(save_, now_, 2); markDirty();
    } else if (save_.poop && in_.tapIn(dogX_ > 70 ? 30 : 100, 98, 22, 20)) {
      pet::cleanPoop(save_, now_); spawn(in_.x, in_.y, 1, 8); toast("All clean!"); markDirty();
    } else if (pet::giftReady(save_, now_) && in_.tapIn(76, 46, 28, 22)) {
      giftOpened_ = false; go(SC_GIFT);
    } else if (in_.tapIn(10, 64, 26, 30)) {  // lamp
      if (save_.asleep) {
        if (pet::canWake(save_)) { pet::setAsleep(save_, now_, false); toast("Good morning!"); }
        else { char b[40]; snprintf(b, sizeof b, "%s is too sleepy", save_.petName); toast(b); }
      } else { pet::setAsleep(save_, now_, true); toast("Sweet dreams..."); pet::addBond(save_, now_, pet::isNightHour(pet::hourOf(now_)) ? 5 : 1); }
      markDirty();
    } else if (save_.asleep && in_.tap) {
      toast("Shh... sleeping");
    } else if (onDog) {
      if (save_.dirty) { startBath(); }
      else { pet::petDog(save_, now_); spawn(in_.x, in_.y - 6, 0, 3); dogAct_ = 3; dogActUntil_ = ms_ + 900; markDirty(); }
    } else if (in_.tapIn(16, 96, 32, 20)) { go(SC_FEED); feedAnimFood_ = -1; }
    else if (in_.tapIn(118, 92, 28, 22)) { go(SC_PLAYMENU); }
    else if (in_.tapIn(110, 38, 40, 56)) { go(SC_LIBRARY); }
    else if (in_.tapIn(30, 12, 100, 18)) { statsPage_ = 0; go(SC_STATS); }
  }
  if (in_.longPress && !save_.asleep) {
    int dogTop = DOG_BASE_Y - dogFrame(P_IDLE0).h;
    if (in_.hit(dogX_ - 4, dogTop - 6, dogW + 8, dogFrame(P_IDLE0).h + 8) && !save_.dirty) { pet::petDog(save_, now_); spawn(in_.x, in_.y - 6, 0, 6); dogAct_ = 2; markDirty(); }
  }
  (void)want;
}
void Game::drawHome() {
  drawRoom();
  // stats row
  drawStatPips(34, 16, save_.food, SPR_BONE, C_ORANGE);
  drawStatPips(64, 16, save_.fun, SPR_BALL, C_RED);
  drawStatPips(92, 16, save_.energy, SPR_MOON, C_BLUE);
  drawStatPips(120, 16, save_.clean, SPR_BUBBLE, C_WATER);
  // gift parcel
  if (pet::giftReady(save_, now_)) {  // the mail arrives on the window sill
    int b = (ms_ / 300) % 2; blit(SPR_PARCEL, 83, 52 - b);
    textCentered(90, 38 - b * 2, "!", C_RED);
  }
  // poop
  if (save_.poop) blit(SPR_POOP, dogX_ > 70 ? 36 : 106, 104);
  // dog
  int pose = idlePose();
  bool flip = dogFlip_;
  if (dogAct_ == 10 && trickShow_ >= 0) {
    uint32_t t = ms_ % 600;
    switch (trickShow_) {
      case 0: pose = P_SIT; break;
      case 1: pose = t < 300 ? P_SIT : P_SIT_PAW; break;
      case 2: pose = t < 300 ? P_BARK : P_SIT; if (t < 300) textCentered(dogX_ + 16, DOG_BASE_Y - 34, "Woof!", C_DKBROWN); break;
      case 3: pose = (ms_ / 120) % 2 ? P_WALK0 : P_WALK1; flip = (ms_ / 240) % 2; blit(SPR_DUST, dogX_ + 12, DOG_BASE_Y - 3); break;
      case 4: pose = (ms_ / 300) % 2 ? P_BACK : P_IDLE0; flip = (ms_ / 300) % 4 >= 2; break;
      case 5: pose = P_DEAD; break;
      case 6: pose = P_BEG; break;
      case 7: pose = (ms_ / 200) % 2 ? P_JUMP : P_BEG; flip = (ms_ / 400) % 2; if ((ms_ / 200) % 3 == 0) blit(SPR_NOTE, dogX_ + 30, DOG_BASE_Y - 34); break;
    }
  }
  drawDog(dogX_, DOG_BASE_Y, pose, flip);
  // thought bubble
  Want w = pet::computeWant(save_, now_);
  if (w != WANT_NONE && !save_.asleep && dogAct_ != 10 && (ms_ / 3000) % 2 == 0) {
    int bx = dogX_ + (dogFlip_ ? -14 : dogFrame(P_IDLE0).w - 6), by = DOG_BASE_Y - dogFrame(P_IDLE0).h - 18;
    bx = clampi(bx, 8, 134);
    roundRect(bx, by, 18, 14, C_WHITE); frame(bx, by, 18, 14, C_DKGRAY); pixel(bx + (dogFlip_ ? 15 : 2), by + 15, C_WHITE); pixel(bx + (dogFlip_ ? 17 : 0), by + 17, C_WHITE);
    const Sprite* ic = &SPR_BONE;
    switch (w) { case WANT_PLAY: ic = &SPR_BALL; break; case WANT_SLEEP: ic = &SPR_MOON; break; case WANT_BATH: ic = &SPR_TUB; break; case WANT_POOP: ic = &SPR_POOP; break; case WANT_STORY: ic = &SPR_BOOK; break; default: break; }
    if (ic == &SPR_TUB || ic == &SPR_BOOK) blitScaled(*ic, bx + (18 - ic->w) / 2, by + (14 - ic->h) / 2, 1);
    else blit(*ic, bx + (18 - ic->w) / 2, by + (14 - ic->h) / 2);
  }
  if (save_.asleep) { textCentered(dogX_ + 20, DOG_BASE_Y - 40 - (int)((ms_ / 400) % 4), "z", C_NAVY); textCentered(dogX_ + 26, DOG_BASE_Y - 46 - (int)((ms_ / 400) % 4), "Z", C_NAVY); }
  // butterfly
  if (event_ == 1) blit((ms_ / 150) % 2 ? SPR_BUTTERFLY0 : SPR_BUTTERFLY1, eventX_, eventY_ + (int)((ms_ / 200) % 3));
  // action buttons
  static const Sprite* BTN_ICONS[4] = {&SPR_BOWL, &SPR_BALL, &SPR_BOOK, &SPR_PAW};
  static const uint8_t BTN_COLS[4] = {C_ORANGE, C_GREEN, C_BLUE, C_PLUM};
  for (int i = 0; i < 4; i++) {
    bool pr = in_.down && in_.hit(BTN_X[i] - 2, BTN_Y - 2, BTN_W + 4, BTN_H + 4);
    drawButton(BTN_X[i], BTN_Y, BTN_W, BTN_H, nullptr, nullptr, BTN_COLS[i], pr);
    const Sprite& ic = *BTN_ICONS[i]; int ix = BTN_X[i] + (BTN_W - ic.w * 2) / 2, iy = BTN_Y + (BTN_H - ic.h * 2) / 2 + (pr ? 1 : 0);
    if (i == 3) { for (int sy = 0; sy < ic.h; sy++) for (int sx = 0; sx < ic.w; sx++) if (ic.px[sy * ic.w + sx] != C_T) rect(ix + sx * 2, iy + sy * 2, 2, 2, C_WHITE); }
    else blitScaled(ic, ix, iy, 2);
    if (in_.tapIn(BTN_X[i] - 3, BTN_Y - 3, BTN_W + 6, BTN_H + 6)) {

      if (i == 0) { go(SC_FEED); feedAnimFood_ = -1; } else if (i == 1) go(SC_PLAYMENU); else if (i == 2) go(SC_LIBRARY); else go(SC_TRICKS);
    }
  }
}

// ---------------------------------------------------------------- feed
void Game::updateFeed() {
  if (feedAnimFood_ >= 0) {
    if (ms_ >= feedAnimUntil_) { feedAnimFood_ = -1; go(SC_HOME); }
    else if ((ms_ / 300) % 2 == 0 && (ms_ / 33) % 9 == 0) spawn(78, 98, 6, 1);
    return;
  }
  if (backButton()) go(SC_HOME);
  static const Food foods[3] = {FOOD_KIBBLE, FOOD_BONE, FOOD_COOKIE};
  for (int i = 0; i < 3; i++) {
    int x = 22 + i * 40;
    if (in_.tapIn(x - 3, 46 - 3, 42, 42)) {
      int d = 0;
      if (save_.asleep) { toast("Shh... sleeping"); }
      else if (save_.food >= 100) { char b[40]; snprintf(b, sizeof b, "%s is full!", save_.petName); toast(b); }
      else if (!pet::feed(save_, now_, foods[i], &d)) { toast("No more cookies today"); }
      else { feedAnimFood_ = i; feedAnimUntil_ = ms_ + 2200; markDirty(); char b[24]; snprintf(b, sizeof b, "Yum! +%d", d); toast(b, 2200); }
    }
  }
}
void Game::drawFeed() {
  drawRoom();
  if (feedAnimFood_ >= 0) {
    static const Sprite* icons[3] = {&SPR_BOWL, &SPR_BONE, &SPR_COOKIE};
    blitScaled(*icons[feedAnimFood_], 90, 100, 2);
    drawDog(56, DOG_BASE_Y, (ms_ / 250) % 2 ? P_EAT0 : P_EAT1, false);
    return;
  }
  drawPanel(14, 40, 132, 62, C_CREAM, C_DKBROWN);
  textCentered(80, 44, "What's for dinner?", C_DKBROWN);
  static const char* names[3] = {"Kibble", "Bone", "Cookie"};
  static const Sprite* icons[3] = {&SPR_BOWL, &SPR_BONE, &SPR_COOKIE};
  for (int i = 0; i < 3; i++) {
    int x = 22 + i * 40; bool pr = in_.down && in_.hit(x - 3, 43, 42, 42);
    bool out = i == 2 && save_.treatsToday >= 3;   // cookies are rationed: three a day
    roundRect(x, 56 + (pr ? 1 : 0), 36, 30, C_DKBROWN); roundRect(x + 1, 57 + (pr ? 1 : 0), 34, 28, out ? C_LTGRAY : pr ? C_YELLOW : C_WHITE);
    blitScaled(*icons[i], x + 18 - icons[i]->w, 60 + (pr ? 1 : 0) + (8 - icons[i]->h), 2);
    textCentered(x + 18, 90, names[i], C_DKBROWN);
  }

  drawDog(64, DOG_BASE_Y + 20, save_.food >= 100 ? P_SIT : P_BEG, false);
  drawBackButton();
}

// ---------------------------------------------------------------- play menu
void Game::updatePlayMenu() {
  if (backButton()) go(SC_HOME);
  if (save_.asleep) { if (in_.tap) toast("Shh... sleeping"); return; }
  if (in_.tapIn(30, 40, 100, 40)) { if (save_.energy < 20) { toast("Too sleepy to play"); } else startFetch(); }
  if (in_.tapIn(30, 88, 100, 40)) startWords();
}
void Game::drawPlayMenu() {
  drawRoom();
  drawPanel(24, 34, 112, 100, C_CREAM, C_DKBROWN);
  bool p1 = in_.down && in_.hit(30, 40, 100, 40), p2 = in_.down && in_.hit(30, 88, 100, 40);
  drawButton(34, 42, 92, 36, "Fetch!", nullptr, C_GREEN, p1); blitScaled(SPR_BALL, 40, 52, 2); blitScaled(SPR_BONE, 96, 55, 2);
  drawButton(34, 90, 92, 36, "Word Fetch", nullptr, C_BLUE, p2); blitScaled(SPR_BOOK, 40, 96, 1); text(106, 96, "A", C_DKBROWN, 2);
  drawBackButton();
}

// ---------------------------------------------------------------- fetch minigame
void Game::startFetch() {
  for (auto& f : falling_) f.alive = 0;
  fetchScore_ = 0; fetchMiss_ = 0; fetchEndMs_ = ms_ + 30000; fetchNextSpawn_ = ms_ + 800; fetchDogX_ = 64; fetchDone_ = false;
  go(SC_FETCH);
}
void Game::updateFetch() {
  if (fetchDone_) {
    if (in_.tap && ms_ - screenMs_ > 600) {
      bool muddy = (pet::rnd(save_) % 100) < 40;
      pet::playResult(save_, now_, fetchScore_, muddy); markDirty();
      if (muddy) { toast("Uh oh, muddy paws!", 3000); }
      go(SC_HOME); dogAct_ = 3; dogActUntil_ = ms_ + 1500;
    }
    return;
  }
  if (backButton()) { fetchDone_ = true; screenMs_ = ms_; return; }   // quitting early keeps the score so far
  int dogW = dogFrame(P_IDLE0).w;
  if (in_.down && in_.y > 26) fetchDogX_ = clampi(in_.x - dogW / 2, 6, 154 - dogW);
  if (ms_ >= fetchNextSpawn_) {
    for (auto& f : falling_) if (!f.alive) {
      f.alive = 1; f.x = (int16_t)(20 + pet::rnd(save_) % 120); f.y = -8;
      uint32_t elapsed = 30000 - (fetchEndMs_ - ms_);
      f.vy = (int16_t)(1 + (elapsed > 15000 ? 1 : 0)); f.kind = (pet::rnd(save_) % 100) < 18 ? 1 : 0; break;
    }
    fetchNextSpawn_ = ms_ + 650 + pet::rnd(save_) % 500;
  }
  int catchY = DOG_BASE_Y + 6 - dogFrame(P_IDLE0).h;
  for (auto& f : falling_) if (f.alive) {
    f.y = (int16_t)(f.y + f.vy + ((ms_ / 33) % 2 ? 1 : 0) * (f.vy > 1 ? 1 : 0));
    if (f.y + 6 >= catchY && f.y <= DOG_BASE_Y && absi(f.x + 4 - (fetchDogX_ + dogW / 2)) < dogW / 2 + 2) {
      f.alive = 0;
      if (f.kind == 0) { fetchScore_++; spawn(f.x, f.y, 0, 2); dogAct_ = 3; dogActUntil_ = ms_ + 250; }
      else { fetchScore_ = fetchScore_ > 0 ? fetchScore_ - 1 : 0; toast("Ouch, a bee!", 900); dogAct_ = 4; dogActUntil_ = ms_ + 500; }
    } else if (f.y > 150) { f.alive = 0; if (f.kind == 0) fetchMiss_++; }
  }
  if (dogAct_ && ms_ >= dogActUntil_) dogAct_ = 0;
  if (ms_ >= fetchEndMs_) { fetchDone_ = true; screenMs_ = ms_; sound(SND_HAPPY); spawn(80, 70, 4, 12); }
}
void Game::drawFetch() {
  clear(C_SKY);
  blit(SPR_CLOUD, 20 + (ms_ / 250) % 80, 22); blit(SPR_CLOUD, 90 + (ms_ / 400) % 50, 44); blit(SPR_SUN, 112, 14);
  rect(0, DOG_BASE_Y, 160, 44, C_LEAF); rect(0, DOG_BASE_Y, 160, 2, C_DKGREEN);
  for (int x = 4; x < 160; x += 9) pixel(x, DOG_BASE_Y + 6 + (x % 5), C_DKGREEN);
  for (auto& f : falling_) if (f.alive) blit(f.kind ? SPR_BEE : SPR_BONE, f.x, f.y);
  int pose = dogAct_ == 3 ? P_JUMP : dogAct_ == 4 ? P_SAD : (in_.down && absi(in_.x - (fetchDogX_ + 16)) > 6) ? ((ms_ / 120) % 2 ? P_WALK0 : P_WALK1) : P_IDLE1;
  bool flip = in_.down && in_.x < fetchDogX_ + dogFrame(P_IDLE0).w / 2 - 6;
  drawDog(fetchDogX_, DOG_BASE_Y, pose, flip);
  // hud
  int left = fetchDone_ ? 0 : (int)((fetchEndMs_ - ms_) / 1000);
  drawPanel(30, 14, 36, 13, C_WHITE, C_DKBROWN);
  blit(SPR_BONE, 33, 17); char b[16]; snprintf(b, sizeof b, "%d", fetchScore_); text(46, 17, b, C_DKBROWN);
  drawPanel(94, 14, 36, 13, C_WHITE, C_DKBROWN);
  snprintf(b, sizeof b, "%ds", left); textCentered(112, 17, b, left <= 5 ? C_RED : C_DKBROWN);
  if (!fetchDone_ && ms_ - screenMs_ < 2000) textCenteredShadow(80, 60, "Catch the bones!", C_NAVY, C_WHITE);
  if (!fetchDone_ && ms_ - screenMs_ < 2000) textCentered(80, 72, "(slide your finger)", C_NAVY);
  if (!fetchDone_) drawBackButton();
  if (fetchDone_) {
    drawPanel(30, 46, 100, 50, C_CREAM, C_DKBROWN);
    textCentered(80, 52, "Time's up!", C_DKBROWN);
    snprintf(b, sizeof b, "%d bones!", fetchScore_); textCentered(80, 66, b, C_PLUM, 1);
    textCentered(80, 80, fetchScore_ >= 12 ? "Amazing!" : fetchScore_ >= 6 ? "Great job!" : "Good try!", C_DKBROWN);
  }
}

// ---------------------------------------------------------------- word fetch
void Game::startWords() {
  int lo, hi; pet::wordLenRange(save_.kidAge ? save_.kidAge : 7, lo, hi);
  static int cand[NUM_WORDS]; int nc = 0;
  for (;;) {
    nc = 0;
    for (int i = 0; i < NUM_WORDS; i++) { int L = (int)strlen(WORDS[i].word); if (L >= lo && L <= hi) cand[nc++] = i; }
    if (nc >= 8 || lo <= 3) break;
    lo--;
  }
  int maxLen = 0;
  for (int i = 0; i < 5; i++) {
    int pick; bool dup;
    do { pick = cand[pet::rnd(save_) % (uint32_t)nc]; dup = false; for (int j = 0; j < i; j++) if (wordIdx_[j] == pick) dup = true; } while (dup && nc > 5);
    wordIdx_[i] = pick; int L = (int)strlen(WORDS[pick].word); if (L > maxLen) maxLen = L;
  }
  wordCols_ = maxLen > 6 ? 4 : 3;
  wordRound_ = 0; wordCorrect_ = 0; wordsDone_ = false; nextWord(); go(SC_WORDS);
}
void Game::nextWord() {
  const char* w = WORDS[wordIdx_[wordRound_]].word; int n = (int)strlen(w);
  int nt = wordCols_ * 3; if (n > nt) n = nt;
  char pool[12]; int k = 0;
  for (int i = 0; i < n; i++) pool[k++] = w[i];
  while (k < nt) { char c = (char)('A' + pet::rnd(save_) % 26); pool[k++] = c; }
  for (int i = nt - 1; i > 0; i--) { int j = (int)(pet::rnd(save_) % (uint32_t)(i + 1)); char t = pool[i]; pool[i] = pool[j]; pool[j] = t; }
  memcpy(tiles_, pool, (size_t)nt); for (auto& u : tileUsed_) u = false;
  wordTyped_ = 0; wordWrongTile_ = -1; wordSolvedUntil_ = 0;
}
void Game::updateWords() {
  if (wordsDone_) {
    if (in_.tap && ms_ - screenMs_ > 600) { pet::wordsResult(save_, now_, wordCorrect_); markDirty(); go(SC_HOME); dogAct_ = 3; dogActUntil_ = ms_ + 1500; }
    return;
  }
  if (backButton()) { go(SC_PLAYMENU); return; }
  if (wordSolvedUntil_) {
    if (ms_ >= wordSolvedUntil_) {
      wordRound_++;
      if (wordRound_ >= 5) { wordsDone_ = true; screenMs_ = ms_; sound(SND_HAPPY); spawn(80, 60, 4, 12); }
      else nextWord();
    }
    return;
  }
  const char* w = WORDS[wordIdx_[wordRound_]].word; int n = (int)strlen(w);
  for (int i = 0; i < wordCols_ * 3; i++) {
    int x = tileX(i), y = tileY(i);
    if (!tileUsed_[i] && in_.tapIn(x - 2, y - 2, 28, 28)) {
      if (tiles_[i] == w[wordTyped_]) {
        tileUsed_[i] = true; wordTyped_++; spawn(x + 12, y + 12, 1, 2);
        if (wordTyped_ >= n) { wordCorrect_++; wordSolvedUntil_ = ms_ + 1600; spawn(80, 50, 0, 5); }
      } else { wordWrongTile_ = i; wordWrongUntil_ = ms_ + 500; }
    }
  }
}
void Game::drawWords() {
  clear(C_WALL);
  for (int y = 6; y < 160; y += 12) for (int x = (y / 12) % 2 ? 6 : 0; x < 160; x += 12) pixel(x, y, C_PEACH);
  if (wordsDone_) {
    drawPanel(26, 40, 108, 60, C_CREAM, C_DKBROWN);
    textCentered(80, 46, "Word Fetch done!", C_DKBROWN);
    char b[32]; snprintf(b, sizeof b, "%d of 5 words", wordCorrect_); textCentered(80, 62, b, C_NAVY);
    textCentered(80, 78, "Super speller!", C_DKBROWN);
    drawDog(64, 140, P_JUMP, false);
    return;
  }
  const WordClue& wc = WORDS[wordIdx_[wordRound_]]; int n = (int)strlen(wc.word);
  char b[24];
  char lines[2][40]; int nl = wrap(wc.clue, 118, lines, 2);
  for (int i = 0; i < nl; i++) textCentered(80, 28 + i * 9, lines[i], C_NAVY);
  // blanks
  int by = wordCols_ == 4 ? 47 : 52;
  int bw = n > 8 ? 10 : 12, bx = 80 - (n * bw) / 2;
  for (int i = 0; i < n; i++) {
    rect(bx + i * bw, by, bw - 2, 11, i < wordTyped_ ? C_YELLOW : C_WHITE); frame(bx + i * bw, by, bw - 2, 11, C_DKBROWN);
    if (i < wordTyped_) { char l[2] = {wc.word[i], 0}; textCentered(bx + i * bw + bw / 2 - 1, by + 2, l, C_DKBROWN); }
  }

  for (int i = 0; i < wordCols_ * 3; i++) {
    int x = tileX(i), y = tileY(i);
    if (tileUsed_[i]) { roundRect(x + 2, y + 4, 20, 18, C_LTGRAY); continue; }
    if (wordSolvedUntil_) continue;
    bool wrong = wordWrongTile_ == i && ms_ < wordWrongUntil_;
    int sh = wrong ? ((ms_ / 50) % 2 ? 1 : -1) : 0;
    bool pr = in_.down && in_.hit(x - 2, y - 2, 28, 28);
    roundRect(x + sh, y + 2, 24, 24, C_DKBROWN); roundRect(x + sh, y + (pr ? 1 : 0), 24, 24, C_DKBROWN);
    roundRect(x + 1 + sh, y + 1 + (pr ? 1 : 0), 22, 22, wrong ? C_RED : pr ? C_YELLOW : C_WHITE);
    char l[2] = {tiles_[i], 0}; textCentered(x + 12 + sh, y + 5 + (pr ? 1 : 0), l, C_DKBROWN, 2);
  }
  if (wordSolvedUntil_) textCenteredShadow(80, 96, "WOOF! Got it!", C_PLUM, C_WHITE);
  snprintf(b, sizeof b, "word %d/5", wordRound_ + 1); textCentered(80, wordCols_ == 4 ? 140 : 146, b, C_DKGRAY);
  drawBackButton();
}

// ---------------------------------------------------------------- library & reading
void Game::updateLibrary() {
  if (backButton()) { go(SC_HOME); return; }
  int n = save_.booksUnlocked < nBooks_ ? save_.booksUnlocked : nBooks_; if (n < 1) n = 1;
  if (bookSel_ >= n) bookSel_ = 0;
  if (in_.tapInCircle(26, 70, 16)) { bookSel_ = (bookSel_ + n - 1) % n; }
  if (in_.tapInCircle(134, 70, 16)) { bookSel_ = (bookSel_ + 1) % n; }
  if (in_.tapIn(44, 118, 72, 30)) { page_ = 0; answerPick_ = -1; go(SC_READ); }
  if (in_.tapIn(56, 30, 48, 64)) { page_ = 0; answerPick_ = -1; go(SC_READ); }
}
static void drawCover(int x, int y, int w, int h, const Book& bk, uint32_t ms) {
  rect(x + 2, y + 2, w, h, C_DKBROWN);
  rect(x, y, w, h, bk.cover); frame(x, y, w, h, C_DKBROWN);
  rect(x + 3, y, 2, h, C_CREAM); rect(x + 8, y + 5, w - 12, h - 10, C_CREAM); frame(x + 8, y + 5, w - 12, h - 10, C_DKBROWN);
  int cx = x + 8 + (w - 12) / 2, cy = y + h / 2;
  const Sprite* ic = &SPR_HEART;
  switch (bk.icon) { case 0: ic = nullptr; break; case 1: ic = &SPR_MOON; break; case 2: ic = &SPR_STAR; break; case 3: ic = &SPR_HEART; break;
    case 4: ic = &SPR_CLOUD; break; case 5: ic = &SPR_BONE; break; case 6: ic = &SPR_BOOK; break; case 7: ic = &SPR_PAW; break; }
  if (!ic) blit(DOG_FRAMES[SZ_PUP][(ms / 600) % 2 ? P_IDLE1 : P_IDLE0], cx - 12, cy - 9);
  else if (bk.icon == 7) blitTint(*ic, cx - ic->w / 2, cy - ic->h / 2, C_DKBROWN);
  else blit(*ic, cx - ic->w / 2, cy - ic->h / 2);
}
void Game::drawLibrary() {
  clear(C_WOOD);
  for (int y = 0; y < 160; y += 16) hline(0, y, 160, C_DKWOOD);
  const Book& bk = curBook();
  drawCover(56, 30, 48, 60, bk, ms_);
  bool done = save_.booksDoneMask & (1u << bookList_[bookSel_]);
  if (!done) { blit(SPR_SPARKLE, 108, 30 + (int)((ms_ / 400) % 2)); textCentered(122, 40, "NEW!", C_YELLOW); }
  else blitTint(SPR_CHECK, 96, 84, C_GREEN);
  char lines[2][40]; int nl = wrap(bk.title, 110, lines, 2);
  for (int i = 0; i < nl; i++) textCenteredShadow(80, 96 + i * 9, lines[i], C_WHITE, C_DKWOOD);
  circle(26, 71, 12, C_DKBROWN); circle(26, 70, 11, C_ORANGE); blitTint(SPR_ARROW, 22, 66, C_WHITE, true);
  circle(134, 71, 12, C_DKBROWN); circle(134, 70, 11, C_ORANGE); blitTint(SPR_ARROW, 132, 66, C_WHITE);
  button(44, 118, 72, 24, done ? "Read again" : "Read to me!", nullptr, C_GREEN);
  int shown = save_.booksUnlocked < nBooks_ ? save_.booksUnlocked : nBooks_;
  char b[32]; snprintf(b, sizeof b, "book %d of %d", bookSel_ + 1, shown); textCentered(80, 24, b, C_CREAM);
  drawBackButton();
}
void Game::updateRead() {
  const Book& bk = curBook();
  int npages = 0; while (bk.pages[npages]) npages++;
  if (backButton()) { go(SC_LIBRARY); return; }
  if (page_ < npages) {
    if (in_.tap && in_.y > 26) {
      if (in_.x >= 64) { page_++; if (page_ == npages) { spawn(40, 120, 0, 3); } }
      else if (page_ > 0) { page_--; }
    }
    return;
  }
  // question page
  if (answerPick_ >= 0) {
    if (ms_ >= answerUntil_) {
      bool right = answerPick_ == bk.correct;
      pet::bookFinished(save_, now_, bookList_[bookSel_], right); markDirty();
      char t[24], b[48]; snprintf(t, sizeof t, "The End!");
      if (right) snprintf(b, sizeof b, "%s loved that story!", save_.petName); else snprintf(b, sizeof b, "It was: %s", bk.answers[bk.correct]);
      celebrate(t, b, right ? 0 : 3, SC_HOME);
    }
    return;
  }
  int na = 0; while (na < 3 && bk.answers[na]) na++;
  char tl[2][40]; int tn = wrap(bk.title, 108, tl, 2); char ql[3][40]; int qn = wrap(bk.question, 116, ql, 3);
  int ay = (tn == 2 ? 44 : 36) + qn * 9 + 4;
  for (int i = 0; i < na; i++) if (in_.tapIn(26, ay + i * 22, 108, 21)) {
    answerPick_ = i; answerUntil_ = ms_ + 1400;
    if (i == bk.correct) spawn(80, 84 + i * 22, 1, 8);
  }
}
void Game::drawRead() {
  const Book& bk = curBook();
  int npages = 0; while (bk.pages[npages]) npages++;
  clear(C_CREAM);
  char tl[2][40]; int tn = wrap(bk.title, 108, tl, 2);           // long titles take two lines
  int top = tn == 2 ? 46 : 38;
  for (int y = top + 2; y < 160; y += 10) hline(24, y, 112, C_PEACH);  // ruled paper
  for (int i = 0; i < tn; i++) textCentered(80, (tn == 2 ? 16 : 24) + i * 9, tl[i], C_PLUM);
  hline(30, top - 5, 100, C_PLUM);
  if (page_ < npages) {
    char lines[8][40]; int nl = wrap(bk.pages[page_], 122, lines, tn == 2 ? 7 : 8);
    for (int i = 0; i < nl; i++) text(19, top + i * 10, lines[i], C_DKBROWN);
    for (int i = 0; i < npages; i++) circle(80 - npages * 3 + i * 6, 130, i == page_ ? 2 : 1, i == page_ ? C_PLUM : C_LTGRAY);
    if (page_ < npages - 1 || true) blitTint(SPR_ARROW, 118, 124, C_ORANGE);
    if (page_ > 0) blitTint(SPR_ARROW, 36, 124, C_ORANGE, true);
    // listening dog
    int pose = (ms_ / 700) % 5 == 0 ? P_LISTEN : P_SIT;
    drawDog(58, 152, pose, false);
    if ((ms_ / 5000) % 3 == 1) blit(SPR_HEART, 84, 128);
  } else {
    int qTop = tn == 2 ? 44 : 36;
    char lines[3][40]; int nl = wrap(bk.question, 116, lines, 3);
    for (int i = 0; i < nl; i++) textCentered(80, qTop + i * 9, lines[i], C_DKBROWN);
    int ay = qTop + nl * 9 + 4;
    int na = 0; while (na < 3 && bk.answers[na]) na++;
    for (int i = 0; i < na; i++) {
      uint8_t col = C_BLUE;
      if (answerPick_ >= 0) { if (i == bk.correct) col = C_GREEN; else if (i == answerPick_) col = C_RED; else col = C_LTGRAY; }
      bool pr = in_.down && in_.hit(26, ay + i * 22, 108, 21);
      int by = ay + i * 22 + (pr ? 1 : 0);
      drawButton(26, ay + i * 22, 108, 20, nullptr, nullptr, col, pr);
      char al[2][40]; int an = wrap(bk.answers[i], 102, al, 2);
      for (int k = 0; k < an; k++) textCentered(80, by + (an == 2 ? 2 + k * 9 : 6), al[k], inkOn(col));
    }
    if (answerPick_ >= 0) drawDog(64, 158, answerPick_ == bk.correct ? P_JUMP : P_SAD, false);
  }
  drawBackButton();
}

// ---------------------------------------------------------------- tricks
void Game::updateTricks() {
  if (backButton()) { go(SC_HOME); return; }
  for (int i = 0; i < NUM_TRICKS; i++) {
    int x = i % 2 ? 82 : 20, y = 34 + (i / 2) * 24;
    if (!in_.tapIn(x - 2, y - 2, 64, 24)) continue;
    if (pet::trickLearned(save_, i)) { go(SC_HOME); startTrickShow(i); }
    else if (!pet::trickAvailable(save_, i)) { char b[32]; snprintf(b, sizeof b, "Needs %d hearts", pet::TRICK_UNLOCK_HEARTS[i]); toast(b); }
    else if (save_.asleep) { toast("Shh... sleeping"); }
    else if (pet::trickLessonToday(save_, now_, i)) { toast("More practice tomorrow!"); }
    else { trickSel_ = i; startTrain(); }
  }
}
void Game::drawTricks() {
  drawRoom();
  drawPanel(18, 28, 124, 110, C_CREAM, C_DKBROWN);
  for (int i = 0; i < NUM_TRICKS; i++) {
    int x = i % 2 ? 82 : 20, y = 34 + (i / 2) * 24;
    bool learned = pet::trickLearned(save_, i), avail = pet::trickAvailable(save_, i);
    uint8_t col = learned ? C_GREEN : avail ? C_BLUE : C_DKGRAY, ink = inkOn(col);
    bool pr = in_.down && in_.hit(x - 2, y - 2, 64, 24);
    int dy = pr ? 1 : 0;
    drawButton(x, y, 60, 21, nullptr, nullptr, col, pr);
    char nl[2][40]; int nn = wrap(TRICK_NAMES[i], 36, nl, 2);   // "Roll Over" and "Play Dead" take two lines
    for (int k = 0; k < nn; k++) text(x + 3, y + 2 + k * 9 + dy, nl[k], ink);
    if (learned) blitTint(SPR_CHECK, x + 49, y + 12 + dy, ink);
    else if (!avail) { blit(SPR_HEART, x + 42, y + 12 + dy); char b[4]; snprintf(b, sizeof b, "%d", pet::TRICK_UNLOCK_HEARTS[i]); text(x + 51, y + 12 + dy, b, ink); }
    else { for (int k = 0; k < 3; k++) rect(x + 44 + k * 5, y + 13 + dy, 4, 4, k < save_.trickProgress[i] ? C_YELLOW : C_NAVY); }
  }
  textCentered(80, 129, "tap to learn", C_DKBROWN);
  drawBackButton();
}

// ---------------------------------------------------------------- training (simon says on the dog)
static const int ZONE_N = 3;  // 0 head, 1 belly, 2 tail
void Game::trainZones(int zx[3], int zy[3], int& scale, int& dx, int& dy) const {
  scale = dogSize() == SZ_PUP ? 3 : 2;
  const Sprite& s = dogFrame(P_SIT); const DogParts& dp = DOG_PARTS[dogSize()][P_SIT];
  dx = 80 - s.w * scale / 2; dy = 118;
  int top = dy - s.h * scale;
  zx[0] = dx + dp.headX * scale; zy[0] = top + (dp.headY - dp.headR / 2) * scale;   // top of the head
  zx[1] = dx + (dp.bodyX - 1) * scale; zy[1] = top + (dp.bodyY + dp.bodyRY / 2) * scale;  // belly
  zx[2] = dx + dp.tailX * scale; zy[2] = top + dp.tailY * scale;                      // tail tip
}
int Game::trainHitZone(int x, int y) const {
  int zx[3], zy[3], sc, dx, dy; trainZones(zx, zy, sc, dx, dy);
  int best = -1, bestD = 22 * 22;  // nearest anchor wins; anything farther than 22px is not a zone tap
  for (int z = 0; z < ZONE_N; z++) { int d = (x - zx[z]) * (x - zx[z]) + (y - zy[z]) * (y - zy[z]); if (d < bestD) { bestD = d; best = z; } }
  return best;
}
void Game::startTrain() {
  seqLen_ = 2; trainRound_ = 0; trainPhase_ = 0; seqInput_ = 0; seqShow_ = -1; go(SC_TRAIN);
}
void Game::updateTrain() {
  int zx[ZONE_N], zy[ZONE_N], scale, DX, DY; trainZones(zx, zy, scale, DX, DY);
  if (backButton()) { go(SC_TRICKS); return; }
  switch (trainPhase_) {
    case 0:  // intro: tap anywhere to start
      if (in_.tap && ms_ - screenMs_ > 400) {
        for (int i = 0; i < 6; i++) seq_[i] = (uint8_t)(pet::rnd(save_) % ZONE_N);
        for (int i = 1; i < 6; i++) if (seq_[i] == seq_[i - 1]) seq_[i] = (uint8_t)((seq_[i] + 1) % ZONE_N);
        trainPhase_ = 1; seqShow_ = -1; seqStepMs_ = ms_ + 500;
      }
      break;
    case 1:  // showing
      if (ms_ >= seqStepMs_) {
        seqShow_++;
        if (seqShow_ >= seqLen_) { trainPhase_ = 2; seqInput_ = 0; seqShow_ = -1; }
        else { seqStepMs_ = ms_ + 700; }
      }
      break;
    case 2:  // player input
      for (int z = 0; z < ZONE_N; z++) UiAudit::add(zx[z] - 12, zy[z] - 12, 24, 24);   // anchors; taps resolve to the nearest one
      if (in_.tap) {
        int hitZone = trainHitZone(in_.x, in_.y);
        if (hitZone < 0) break;
        if (seqInput_ > 0 && hitZone == seq_[seqInput_ - 1] && ms_ - seqStepMs_ < 350) break;  // double tap on the same spot
        if (hitZone == seq_[seqInput_]) {
          spawn(zx[hitZone], zy[hitZone], 0, 2); seqInput_++; seqStepMs_ = ms_;
          if (seqInput_ >= seqLen_) { trainPhase_ = 3; seqStepMs_ = ms_ + 1200; spawn(80, 60, 1, 8); }
        } else { trainPhase_ = 4; seqStepMs_ = ms_ + 1400; }
      }
      break;
    case 3:  // round passed
      if (ms_ >= seqStepMs_) {
        trainRound_++;
        if (trainRound_ >= 3) {
          pet::trickLessonPassed(save_, now_, trickSel_); markDirty(); trainPhase_ = 5;
          char t[24], b[48];
          if (pet::trickLearned(save_, trickSel_)) { snprintf(t, sizeof t, "%s learned", save_.petName); snprintf(b, sizeof b, "%s! Tap the tricks button to see it.", TRICK_NAMES[trickSel_]); celebrate(t, b, 0, SC_HOME); go(SC_CELEBRATE); startTrickShow(trickSel_); }
          else { snprintf(t, sizeof t, "Lesson %d of 3", save_.trickProgress[trickSel_]); snprintf(b, sizeof b, "Practice %s again tomorrow!", TRICK_NAMES[trickSel_]); celebrate(t, b, 2, SC_HOME); }
        } else { seqLen_++; trainPhase_ = 1; seqShow_ = -1; seqStepMs_ = ms_ + 600; }
      }
      break;
    case 4:  // wrong: show again
      if (ms_ >= seqStepMs_) { trainPhase_ = 1; seqShow_ = -1; seqStepMs_ = ms_ + 500; }
      break;
  }
}
void Game::drawTrain() {
  int zx[ZONE_N], zy[ZONE_N], scale, DX, DY; trainZones(zx, zy, scale, DX, DY);
  static const char* ZONE_NAMES[ZONE_N] = {"head", "tummy", "tail"};
  clear(C_MINT);
  rect(0, DY - 2, 160, 42, C_LEAF); rect(0, DY - 2, 160, 2, C_DKGREEN);
  char b[40]; snprintf(b, sizeof b, "Learn: %s", TRICK_NAMES[trickSel_]); textCentered(80, 24, b, C_DKBROWN);
  snprintf(b, sizeof b, "round %d of 3", trainRound_ + 1); textCentered(80, 34, b, C_DKGRAY);
  int pose = trainPhase_ == 4 ? P_SAD : trainPhase_ == 3 || trainPhase_ == 5 ? P_JUMP : P_SIT;
  if (pose == P_SAD || pose == P_JUMP) { const Sprite& s = dogFrame(pose); blitScaled(s, DX, DY - s.h * scale, scale); }
  else drawDog(DX, DY, P_SIT, false, scale);
  int activeZone = -1;
  if (trainPhase_ == 1 && seqShow_ >= 0 && ms_ < seqStepMs_ - 200) activeZone = seq_[seqShow_];
  for (int z = 0; z < ZONE_N; z++) {
    uint8_t col = z == activeZone ? C_YELLOW : C_WHITE;
    if (trainPhase_ == 2 || z == activeZone) { ring(zx[z], zy[z], 10 + (z == activeZone ? (int)((ms_ / 100) % 3) : 0), col); ring(zx[z], zy[z], 9, z == activeZone ? C_ORANGE : C_DKBROWN); }
    if (z == activeZone) { blit(SPR_SPARKLE, zx[z] - 2, zy[z] - 18); textCentered(zx[z], zy[z] + 16, ZONE_NAMES[z], C_DKBROWN); }
  }
  switch (trainPhase_) {
    case 0: textCentered(80, 46, "Watch me glow,", C_NAVY); textCentered(80, 56, "then tap the spots!", C_NAVY); if ((ms_ / 500) % 2) textCentered(80, 138, "tap to start", C_DKBROWN); break;
    case 1: textCentered(80, 46, "Watch...", C_NAVY); break;
    case 2: snprintf(b, sizeof b, "Your turn! %d of %d", seqInput_, seqLen_); textCentered(80, 46, b, C_NAVY); break;
    case 3: textCentered(80, 46, "Good dog!", C_DKGREEN); break;
    case 4: textCentered(80, 46, "Oops! Watch again.", C_RED); break;
  }
  drawBackButton();
}

// ---------------------------------------------------------------- bath
void Game::startBath() {
  const int scale = 2; const Sprite& s = dogFrame(P_IDLE0); const DogParts& dp = DOG_PARTS[dogSize()][P_IDLE0];
  int DX = 80 - s.w, top = 120 - s.h * scale;
  spotsLeft_ = 6;
  static const int8_t SPREAD[6][2] = {{-6, -3}, {0, -5}, {6, -2}, {-3, 4}, {4, 5}, {0, 0}};  // tenths of the body radii
  for (int i = 0; i < 6; i++) {
    int ox = dp.bodyX + dp.bodyRX * SPREAD[i][0] / 10, oy = dp.bodyY + dp.bodyRY * SPREAD[i][1] / 10;
    if (i == 5) { ox = dp.headX; oy = dp.headY + 2; }
    spots_[i].x = (int8_t)(DX + ox * scale - 3); spots_[i].y = (int8_t)(top + oy * scale - 2); spots_[i].on = true;
  }
  bathDoneMs_ = 0; go(SC_BATH);
}
void Game::updateBath() {
  if (bathDoneMs_) { if (ms_ >= bathDoneMs_) { pet::bathe(save_, now_); markDirty(); go(SC_HOME); toast("Squeaky clean!"); dogAct_ = 3; dogActUntil_ = ms_ + 1200; } return; }
  if (backButton()) { go(SC_HOME); return; }
  if (in_.down) {
    if ((ms_ / 33) % 3 == 0) spawn(in_.x, in_.y, 3, 1);
    for (auto& sp : spots_) if (sp.on) {
      int dx = in_.x - sp.x - 3, dy = in_.y - sp.y - 2;
      if (dx * dx + dy * dy <= 15 * 15 && (in_.x != in_.px || in_.y != in_.py)) { sp.on = false; spotsLeft_--; spawn(sp.x + 3, sp.y + 2, 3, 4); }
    }
    if (spotsLeft_ <= 0) { bathDoneMs_ = ms_ + 1800; spawn(80, 80, 2, 14); }
  }
}
void Game::drawBath() {
  const int scale = 2; const Sprite& s = dogFrame(P_IDLE0);
  int DX = 80 - s.w;
  clear(C_SKY);
  rect(0, 120, 160, 40, C_WATER); for (int x = 0; x < 160; x += 10) hline(x + (ms_ / 150) % 10, 122, 5, C_WHITE);
  // tub rim
  rect(20, 112, 120, 10, C_WHITE); rect(24, 122, 112, 30, C_LTGRAY); frame(20, 112, 120, 10, C_DKGRAY);
  if (bathDoneMs_) { blitScaled(s, DX, 120 - s.h * scale, scale, (ms_ / 120) % 2); }
  else {
    bool saved = save_.dirty; save_.dirty = 0; drawDog(DX, 120, spotsLeft_ ? P_IDLE0 : P_JUMP, false, scale); save_.dirty = (uint8_t)saved;
    for (auto& sp : spots_) if (sp.on) blit(SPR_MUD, sp.x, sp.y);
  }
  textCentered(80, 34, bathDoneMs_ ? "Shake shake shake!" : "Rub the mud away!", C_NAVY);
  if (!bathDoneMs_) { char b[24]; snprintf(b, sizeof b, "%d spots left", spotsLeft_); textCentered(80, 46, b, C_DKGRAY); }
  drawBackButton();
}

// ---------------------------------------------------------------- stats / stickers / hats
void Game::updateStats() {
  if (backButton()) { go(SC_HOME); return; }
  if (in_.tapIn(20, 112, 50, 22)) { go(SC_STICKERS); }
  if (in_.tapIn(74, 112, 34, 22)) { hatSel_ = save_.hat; go(SC_HATS); }
  if (in_.tapInCircle(126, 123, 14)) { save_.muted = !save_.muted; markDirty(); }
  if (in_.longPress && in_.hit(60, 140, 40, 16)) go(SC_CONFIRM_RESET);
}
void Game::drawStats() {
  clear(C_WALL);
  drawPanel(16, 22, 128, 88, C_CREAM, C_DKBROWN);
  textCentered(80, 26, save_.petName, C_PLUM, 2);
  char b[48];
  static const char* STAGE_NAMES[3] = {"Puppy", "Dog", "Grown dog"};
  snprintf(b, sizeof b, "%s - day %d", STAGE_NAMES[save_.stage], pet::ageDays(save_, now_) + 1); textCentered(80, 46, b, C_DKBROWN);
  drawHearts(80, 56, pet::hearts(save_));
  drawStatPips(22, 68, save_.food, SPR_BONE, C_ORANGE); text(54, 67, "food", C_DKGRAY);
  drawStatPips(86, 68, save_.fun, SPR_BALL, C_RED); text(116, 67, "fun", C_DKGRAY);
  drawStatPips(22, 80, save_.energy, SPR_MOON, C_BLUE); text(52, 79, "sleep", C_DKGRAY);
  drawStatPips(86, 80, save_.clean, SPR_BUBBLE, C_WATER); text(114, 79, "clean", C_DKGRAY);
  int nt = pet::learnedTrickCount(save_);
  snprintf(b, sizeof b, "%d book%s, %d trick%s", save_.booksRead, save_.booksRead == 1 ? "" : "s", nt, nt == 1 ? "" : "s"); textCentered(80, 91, b, C_NAVY);
  snprintf(b, sizeof b, "%d day streak", save_.streak); textCentered(80, 100, b, C_NAVY);
  if (save_.streak >= 3) blit(SPR_STAR, 80 + textWidth(b) / 2 + 4, 99);
  button(20, 112, 50, 22, "Stickers", nullptr, C_PINK);
  button(74, 112, 34, 22, "Hats", nullptr, C_PLUM);
  iconButton(126, 123, 11, save_.muted ? SPR_SPEAKER_OFF : SPR_SPEAKER, save_.muted ? C_DKGRAY : C_GREEN, C_WHITE);
  snprintf(b, sizeof b, "%s, age %d", save_.kidName, save_.kidAge); textCentered(80, 136, b, C_DKGRAY);
  drawBackButton();
}
void Game::updateStickers() {
  if (backButton()) { go(SC_STATS); return; }
  for (int i = 0; i < 16; i++) { int x = 32 + (i % 4) * 26, y = 30 + (i / 4) * 26; if (in_.tapIn(x, y, 24, 24)) { toast((save_.stickersMask & (1u << i)) ? STICKER_NAMES[i] : "???"); } }
}
void Game::drawStickers() {
  clear(C_WALL);
  static const Sprite* ICONS[16] = {&SPR_BOWL, &SPR_BOOK, &SPR_BOOK, &SPR_BOOK, &SPR_BONE, &SPR_STAR, &SPR_SUN, &SPR_SUN,
                                    &SPR_PAW, &SPR_TROPHY, &SPR_BUBBLE, &SPR_HEART, &SPR_SUN, &SPR_CAKE, &SPR_TROPHY, &SPR_HAT_PARTY};
  int n = 0;
  for (int i = 0; i < 16; i++) {
    int x = 32 + (i % 4) * 26, y = 30 + (i / 4) * 26; bool got = save_.stickersMask & (1u << i); if (got) n++;
    roundRect(x, y, 24, 24, got ? C_WHITE : C_LTGRAY); frame(x, y, 24, 24, got ? C_GOLD : C_DKGRAY);
    if (got) { if (ICONS[i] == &SPR_PAW) blitTint(*ICONS[i], x + 8, y + 8, C_PLUM); else blit(*ICONS[i], x + 12 - ICONS[i]->w / 2, y + 12 - ICONS[i]->h / 2); }
    else textCentered(x + 12, y + 8, "?", C_DKGRAY);
  }
  char b[24]; snprintf(b, sizeof b, "%d / 16", n); textCentered(80, 138, b, C_DKBROWN);
  drawBackButton();
}
void Game::updateHats() {
  if (backButton()) { go(SC_STATS); return; }
  if (in_.tapInCircle(26, 80, 16)) { do { hatSel_ = (hatSel_ + 7) % 8; } while (hatSel_ && !(save_.hatsMask & (1u << hatSel_))); }
  if (in_.tapInCircle(134, 80, 16)) { do { hatSel_ = (hatSel_ + 1) % 8; } while (hatSel_ && !(save_.hatsMask & (1u << hatSel_))); }
  if (in_.tapIn(50, 122, 60, 24)) { save_.hat = (uint8_t)hatSel_; markDirty(); toast("Looking good!"); }
}
void Game::drawHats() {
  clear(C_WALL);
  textCentered(80, 28, HAT_NAMES[hatSel_], C_PLUM);
  uint8_t keep = save_.hat; save_.hat = (uint8_t)hatSel_;
  drawDog(80 - dogFrame(P_SIT).w, 112, P_SIT, false, 2);
  save_.hat = keep;
  circle(26, 81, 12, C_DKBROWN); circle(26, 80, 11, C_ORANGE); blitTint(SPR_ARROW, 22, 76, C_WHITE, true);
  circle(134, 81, 12, C_DKBROWN); circle(134, 80, 11, C_ORANGE); blitTint(SPR_ARROW, 132, 76, C_WHITE);
  button(50, 122, 60, 22, save_.hat == hatSel_ ? "Wearing" : "Wear it", nullptr, save_.hat == hatSel_ ? C_DKGRAY : C_GREEN);
  int owned = 0; for (int i = 1; i < 8; i++) if (save_.hatsMask & (1u << i)) owned++;
  char b[24]; snprintf(b, sizeof b, "%d hat%s found", owned, owned == 1 ? "" : "s"); textCentered(80, 40, b, C_DKGRAY);
  drawBackButton();
}
void Game::updateConfirmReset() {
  if (in_.tapIn(20, 90, 50, 24)) go(SC_STATS);
  if (in_.tapIn(90, 90, 50, 24)) {   // delete this house, keep the others
    int idx = active_; leaveHouse(); dirtyHouse_[idx] = false;
    for (int i = idx; i < nHouses_ - 1; i++) { houses_[i] = houses_[i + 1]; dirtyHouse_[i] = true; }
    nHouses_--; eraseSlot_ = nHouses_; dogAct_ = 0;
    memset(&save_, 0, sizeof save_);
    go(nHouses_ ? SC_STREET : SC_INTRO);
  }
}
void Game::drawConfirmReset() {
  clear(C_PLUM);
  textCentered(80, 44, "Start over?", C_WHITE);
  char b[48]; snprintf(b, sizeof b, "%s will forget you!", save_.petName); textCentered(80, 60, b, C_PINK);
  button(20, 90, 50, 24, "No!", nullptr, C_GREEN);
  button(90, 90, 50, 24, "Yes", nullptr, C_RED);
}

// ---------------------------------------------------------------- gift & celebrate
void Game::updateGift() {
  if (!giftOpened_) {
    if (in_.tap && ms_ - screenMs_ > 300) { gift_ = pet::openGift(save_, now_); giftOpened_ = true; screenMs_ = ms_; markDirty(); sound(SND_FANFARE); spawn(80, 70, 4, 18); spawn(80, 70, 1, 8); }
  } else if (in_.tap && ms_ - screenMs_ > 700) {
    if (gift_.kind == pet::GIFT_PARTY) { save_.hat = 2; celebrate("Party time!", "One whole week together!", 4, SC_HOME); }
    else go(SC_HOME);
  }
}
void Game::drawGift() {
  drawRoom();
  drawPanel(18, 30, 124, 98, C_CREAM, C_DKBROWN);
  char b[48];
  if (!giftOpened_) {
    snprintf(b, sizeof b, "A gift for %s!", save_.petName); textCentered(80, 36, b, C_PLUM);
    int wob = (ms_ / 200) % 2 ? 1 : 0; blitScaled(SPR_PARCEL, 66, 56 + wob, 2);
    snprintf(b, sizeof b, "Day %d together", pet::ageDays(save_, now_) + 1); textCentered(80, 88, b, C_DKBROWN);
    if ((ms_ / 500) % 2) textCentered(80, 108, "tap to open", C_DKGRAY);
  } else {
    switch (gift_.kind) {
      case pet::GIFT_BOOK: textCentered(80, 36, "A new book!", C_PLUM); drawCover(64, 48, 32, 40, BOOKS[gift_.index], ms_);
        { char lines[2][40]; int nl = wrap(BOOKS[gift_.index].title, 110, lines, 2); for (int i = 0; i < nl; i++) textCentered(80, 94 + i * 9, lines[i], C_DKBROWN); } break;
      case pet::GIFT_HAT: textCentered(80, 36, "A new hat!", C_PLUM); blitScaled(SPR_HATS[gift_.index], 80 - SPR_HATS[gift_.index].w, 52, 2);
        textCentered(80, 94, HAT_NAMES[gift_.index], C_DKBROWN); textCentered(80, 104, "Find it under Hats", C_DKGRAY); break;
      case pet::GIFT_TREATS: textCentered(80, 36, "Treats!", C_PLUM); blitScaled(SPR_COOKIE, 56, 56, 2); blitScaled(SPR_BONE, 84, 60, 2);
        textCentered(80, 94, "Yum, a snack basket.", C_DKBROWN); break;
      case pet::GIFT_PARTY: textCentered(80, 36, "One week party!", C_PLUM); blitScaled(SPR_CAKE, 54, 50, 2); blitScaled(SPR_HAT_PARTY, 96, 54, 2);
        textCentered(80, 96, "Cake and a hat!", C_DKBROWN); break;
    }
    if ((ms_ / 500) % 2) textCentered(80, 116, "tap", C_DKGRAY);
  }
}
void Game::updateCelebrate() {
  if ((ms_ / 200) % 3 == 0 && (ms_ / 33) % 4 == 0) spawn(20 + (int)(pet::rnd(save_) % 120), 10, 4, 1);
  if (in_.tap && ms_ - screenMs_ > 700) { go(nextAfterCelebrate_); }
}
void Game::drawCelebrate() {
  clear(C_NAVY);
  for (int i = 0; i < 20; i++) { int x = (i * 41) % 160, y = (i * 23) % 160; if (((ms_ / 250) + i) % 3) pixel(x, y, C_YELLOW); }
  drawPanel(14, 28, 132, 70, C_CREAM, C_GOLD);
  char lines[2][40]; int nl = wrap(celebTitle_, 120, lines, 2);
  for (int i = 0; i < nl; i++) textCentered(80, 34 + i * 10, lines[i], C_PLUM);
  char body[4][40]; int nb = wrap(celebText_, 122, body, 4);
  for (int i = 0; i < nb; i++) textCentered(80, 56 + i * 9, body[i], C_DKBROWN);
  int pose = celebIcon_ == 3 ? P_SIT : (ms_ / 250) % 2 ? P_JUMP : P_IDLE1;
  drawDog(80 - dogFrame(pose).w / 2, 134, pose, false);
  if (celebIcon_ == 2) blit(SPR_STAR, 112, 104);
  if (celebIcon_ == 4) blitScaled(SPR_CAKE, 108, 100, 1);
  if ((ms_ / 500) % 2) textCentered(80, 146, "tap", C_LTGRAY);
}

// ---------------------------------------------------------------- street (choose a house)
static const int HOUSE_CX[MAX_HOUSES] = {40, 80, 120};
void Game::updateStreet() {
  for (int i = 0; i < nHouses_; i++) if (in_.tapIn(HOUSE_CX[i] - 19, 56, 38, 70)) {
    if (houses_[i].pin) { pinSlot_ = i; pinLen_ = 0; pinBuf_[0] = 0; go(SC_PIN_ENTER); } else enterHouse(i);
    return;
  }
  if (nHouses_ < MAX_HOUSES && in_.tapIn(HOUSE_CX[nHouses_] - 19, 56, 38, 70)) { memset(&save_, 0, sizeof save_); dogAct_ = 0; go(SC_INTRO); }
}
void Game::drawStreet() {
  int hr = pet::hourOf(now_); bool night = hr >= 20 || hr < 6;
  clear(night ? C_NIGHT : C_SKY);
  if (night) { for (int i = 0; i < 16; i++) { int x = (i * 37) % 160, y = (i * 17) % 60; if (((ms_ / 500) + i) % 3) pixel(x, y, C_WHITE); } blit(SPR_MOON, 118, 16); }
  else { blit(SPR_SUN, 112, 14); blit(SPR_CLOUD, 14 + (ms_ / 300) % 70, 22); }
  rect(0, 112, 160, 48, night ? C_DKGREEN : C_LEAF); rect(0, 134, 160, 26, C_DKGRAY);
  for (int x = -16; x < 160; x += 16) rect(x + (int)((ms_ / 200) % 16), 146, 8, 2, C_YELLOW);
  textCenteredShadow(80, 34, "Paw Street", C_NAVY, C_WHITE);
  for (int i = 0; i < nHouses_; i++) {
    const Save& h = houses_[i]; int cx = HOUSE_CX[i];
    bool resting = pet::resting(h, now_);
    bool pr = in_.down && in_.hit(cx - 19, 56, 38, 70);
    drawHouseIcon(cx, 112 + (pr ? 1 : 0), 32, themeOf(h), night && !h.asleep);
    if (h.asleep || resting) textCentered(cx + 12, 78 - (int)((ms_ / 400) % 3), "z", C_NAVY);
    else blit(DOG_FRAMES[SZ_PUP][(ms_ / 500 + i) % 2 ? P_IDLE1 : P_IDLE0], cx - 22, 94, false);
    if (resting) { char b[16]; snprintf(b, sizeof b, "%lu min", (unsigned long)((h.restUntil - now_ + 59) / 60)); int px = cx < 60 ? 22 : cx > 100 ? 98 : cx - 20; drawPanel(px, 52, 40, 11, C_WHITE, C_DKBROWN); textCentered(px + 20, 54, b, C_DKBROWN); }
    else if (pet::giftReady(h, now_)) textCentered(cx, 52 - (int)((ms_ / 300) % 2), "!", C_RED);
    if (h.pin) blitTint(SPR_GEAR, cx + 9, 100, C_GOLD);
    int w = textWidth(h.kidName) + 6; drawPanel(cx - w / 2, 115, w, 11, C_CREAM, C_DKBROWN); textCentered(cx, 117, h.kidName, C_DKBROWN);
  }
  if (nHouses_ < MAX_HOUSES) {
    int cx = HOUSE_CX[nHouses_];
    for (int y = 84; y < 112; y += 3) { pixel(cx - 16, y, C_WHITE); pixel(cx + 15, y, C_WHITE); }
    for (int x = cx - 16; x < cx + 16; x += 3) { pixel(x, 84, C_WHITE); pixel(x, 111, C_WHITE); }
    textCenteredShadow(cx, 90, "+", C_NAVY, C_WHITE, 2);
    textCentered(cx, 117, "new", C_NAVY);
  }
  textCentered(80, 140, nHouses_ == 0 ? "tap + to start" : "tap a house", C_WHITE);
}

// ---------------------------------------------------------------- age (picks the story level)
static const int AGES[6] = {5, 6, 7, 8, 9, 10};
void Game::updateAge() {
  for (int i = 0; i < 6; i++) {
    int x = 17 + (i % 3) * 42, y = 56 + (i / 3) * 32;
    if (in_.tapIn(x, y, 40, 26)) {
      save_.kidAge = (uint8_t)AGES[i];
      if (haveSave_) { markDirty(); refreshBookList(); }
      go(ageReturn_);
    }
  }
}
void Game::drawAge() {
  clear(C_WALL);
  char b[40]; snprintf(b, sizeof b, "%s, how old", save_.kidName[0] ? save_.kidName : "Hey"); textCentered(80, 26, b, C_DKBROWN);
  textCentered(80, 36, "are you?", C_DKBROWN);
  for (int i = 0; i < 6; i++) {
    int x = 17 + (i % 3) * 42, y = 56 + (i / 3) * 32; char l[4]; snprintf(l, sizeof l, i == 5 ? "10+" : "%d", AGES[i]);
    bool pr = in_.down && in_.hit(x, y, 40, 26);
    uint8_t col = i < 2 ? C_GREEN : i < 4 ? C_BLUE : C_PLUM;
    drawButton(x, y, 40, 26, nullptr, nullptr, col, pr);
    textCentered(x + 20, y + 5 + (pr ? 1 : 0), l, inkOn(col), 2);
  }
}

// ---------------------------------------------------------------- house colors
void Game::updateTheme() {
  for (int i = 0; i < 4; i++) if (in_.tapIn(16 + i * 32, 56, 32, 44)) { save_.theme = (uint8_t)i; pinLen_ = 0; pinBuf_[0] = 0; go(SC_PIN_SET); }
}
void Game::drawTheme() {
  clear(C_SKY);
  rect(0, 100, 160, 60, C_LEAF);
  textCentered(80, 30, "Pick your house!", C_NAVY);
  for (int i = 0; i < 4; i++) {
    bool pr = in_.down && in_.hit(16 + i * 32, 56, 32, 44);
    drawHouseIcon(32 + i * 32, 100 + (pr ? 1 : 0), 24, THEMES[i], false);
  }
  char b[40]; snprintf(b, sizeof b, "for %s and %s", save_.kidName, save_.petName); textCentered(80, 118, b, C_NAVY);
}

// ---------------------------------------------------------------- secret code keypad (set at adoption, asked on the street)
static const int PIN_KX[3] = {44, 68, 92}, PIN_KY[4] = {52, 72, 92, 112};
void Game::updatePin() {
  bool setting = screen_ == SC_PIN_SET;
  if (!setting && backButton()) { go(SC_STREET); return; }
  for (int k = 0; k < 12; k++) {
    int x = PIN_KX[k % 3], y = PIN_KY[k / 3];
    if (!in_.tapIn(x - 2, y - 2, 24, 22)) continue;
    if (k < 9) { if (pinLen_ < 4) { pinBuf_[pinLen_++] = (char)('1' + k); pinBuf_[pinLen_] = 0; } }
    else if (k == 10) { if (pinLen_ < 4) { pinBuf_[pinLen_++] = '0'; pinBuf_[pinLen_] = 0; } }
    else if (k == 9) { if (setting) { save_.pin = 0; finishAdoption(); } else if (pinLen_ > 0) pinBuf_[--pinLen_] = 0; }
    else if (k == 11 && pinLen_ == 4) {
      int code = atoi(pinBuf_); if (code == 0) code = 1;
      if (setting) { save_.pin = (uint16_t)code; finishAdoption(); }
      else if (code == houses_[pinSlot_].pin) enterHouse(pinSlot_);
      else { pinWrongUntil_ = ms_ + 900; pinLen_ = 0; pinBuf_[0] = 0; toast("Hmm, that's not it", 1500); }
    }
    return;
  }
}
void Game::drawPin(const char* prompt) {
  clear(C_WALL);
  bool setting = screen_ == SC_PIN_SET;
  textCentered(80, 18, prompt, C_DKBROWN);
  if (setting) textCentered(80, 27, "4 numbers", C_DKGRAY);
  int shake = ms_ < pinWrongUntil_ ? ((ms_ / 50) % 2 ? 1 : -1) : 0;
  for (int i = 0; i < 4; i++) {
    int x = 58 + i * 12 + shake; rect(x, 38, 10, 12, C_WHITE); frame(x, 38, 10, 12, ms_ < pinWrongUntil_ ? C_RED : C_DKBROWN);
    if (i < pinLen_) { if (setting) { char l[2] = {pinBuf_[i], 0}; textCentered(x + 5, 40, l, C_NAVY); } else circle(x + 5, 44, 2, C_NAVY); }
  }
  for (int k = 0; k < 12; k++) {
    int x = PIN_KX[k % 3], y = PIN_KY[k / 3];
    char l[12]; uint8_t col = C_WHITE;
    if (k < 9) snprintf(l, sizeof l, "%d", k + 1); else if (k == 10) snprintf(l, sizeof l, "0");
    else if (k == 9) { snprintf(l, sizeof l, setting ? "skip" : "<"); col = C_LTGRAY; }
    else { snprintf(l, sizeof l, "OK"); col = pinLen_ == 4 ? C_GREEN : C_LTGRAY; }
    bool pr = in_.down && in_.hit(x - 2, y - 2, 24, 22);
    roundRect(x, y + 1, 20, 18, C_DKBROWN); roundRect(x, y + (pr ? 1 : 0), 20, 18, C_DKBROWN); roundRect(x + 1, y + 1 + (pr ? 1 : 0), 18, 16, pr ? (uint8_t)C_YELLOW : col);
    textCentered(x + 10, y + 5 + (pr ? 1 : 0), l, C_DKBROWN);
  }
  if (!setting) drawBackButton();
}

// ---------------------------------------------------------------- resting (turn taking)
void Game::updateRest() {
  if (!pet::resting(save_, now_)) { save_.playSec = 0; markDirty(); go(SC_HOME); char b[40]; snprintf(b, sizeof b, "%s is ready to play!", save_.petName); toast(b); return; }
  if (in_.tapIn(40, 118, 80, 26)) { leaveHouse(); go(SC_STREET); }
}
void Game::drawRest() {
  drawRoom();
  drawDog(52, DOG_BASE_Y, (ms_ / 1000) % 2 ? P_SLEEP1 : P_SLEEP0, false);
  textCentered(dogX_ + 20, DOG_BASE_Y - 30 - (int)((ms_ / 400) % 4), "z", C_NAVY);
  drawPanel(22, 30, 116, 46, C_CREAM, C_DKBROWN);
  char b[40]; snprintf(b, sizeof b, "%s is tired.", save_.petName); textCentered(80, 36, b, C_PLUM);
  uint32_t left = save_.restUntil > now_ ? save_.restUntil - now_ : 0;
  snprintf(b, sizeof b, "Nap time: %lu min", (unsigned long)((left + 59) / 60)); textCentered(80, 48, b, C_DKBROWN);
  textCentered(80, 60, "Let a friend play!", C_DKBROWN);
  button(40, 118, 80, 24, "To the street", nullptr, C_GREEN);
}
