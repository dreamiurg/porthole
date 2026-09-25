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

const Sprite& Game::icon() const { return SPR_APP_ICON; }
void Game::enter(const AppEnter& e) {
  now_ = e.nowSec; ms_ = e.ms;
  nKids_ = e.n > MAX_PROFILES ? MAX_PROFILES : e.n; self_ = 0;
  for (int i = 0; i < nKids_; i++) {
    kids_[i] = e.all[i];
    kidHas_[i] = e.saves[i].len && pet::loadBlob(e.saves[i].data, e.saves[i].len, kidSaves_[i]);
    if (e.all[i].id == e.who->id) self_ = i;
  }
  memset(&save_, 0, sizeof save_);
  haveSave_ = dirty_ = wantsHome_ = false; sound_ = SND_NONE;
  for (auto& p : parts_) p.life = 0;
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

void Game::enterHouse() {
  save_ = kidSaves_[self_]; haveSave_ = true;
  // the kid's name and age live in the profile; the copies in Save pick the reading level and show on the stats page
  memcpy(save_.kidName, kids_[self_].name, sizeof save_.kidName); save_.kidAge = kids_[self_].age;
  uint32_t ev = pet::simulate(save_, now_, false);
  lastTickSec_ = now_; stickersSeen_ = save_.stickersMask; refreshBookList();
  dogX_ = dogTargetX_ = 60; dogAct_ = 0; event_ = 0; trickShow_ = -1;
  markDirty();
  go(SC_HOME);
  if (ev & pet::EV_LONG_AWAY) {
    char buf[40];
    if (save_.asleep) snprintf(buf, sizeof buf, "%s is napping", save_.petName); else snprintf(buf, sizeof buf, "%s missed you!", save_.petName);
    toast(buf, 3000);
  }
}
void Game::finishAdoption() {
  char name[12]; memcpy(name, save_.petName, sizeof name); uint8_t theme = save_.theme;
  Save& s = kidSaves_[self_];
  pet::adopt(s, now_, kids_[self_].name, name); s.theme = theme; pet::seal(s);
  kidHas_[self_] = true;
  enterHouse();
  char b[48]; snprintf(b, sizeof b, "%s and %s, best friends.", s.kidName, s.petName);
  celebrate("Welcome home!", b, 0, SC_HOME);
}
const Book& Game::curBook() const { return BOOKS[bookList_[bookSel_ < nBooks_ ? bookSel_ : 0]]; }

void Game::go(Screen s) { screen_ = s; screenMs_ = ms_; toastUntil_ = 0; in_.tap = in_.pressed = in_.longPress = false; }  // a tap acts on one screen only
void Game::toast(const char* s, uint32_t ms) { strncpy(toast_, s, sizeof toast_ - 1); toast_[sizeof toast_ - 1] = 0; toastUntil_ = ms_ + ms; }

Tint Game::tint() const {
  if (screen_ <= SC_NAME_PET || screen_ == SC_THEME) return TINT_DAY;
  if (save_.asleep && screen_ == SC_HOME) return TINT_NIGHT;
  int h = pet::hourOf(now_);
  if (h >= 21 || h < 6) return TINT_NIGHT;
  if (h >= 18 || h < 7) return TINT_EVENING;
  return TINT_DAY;
}

bool Game::takeSave(const void** data, size_t* len, bool allowed) {
  if (!allowed || !dirty_) return false;
  if (haveSave_) { pet::seal(save_); out_ = save_; kidSaves_[self_] = save_; *data = &out_; *len = sizeof out_; }
  else { *data = nullptr; *len = 0; }   // no pup (after "start over"): the shell erases this kid's save
  dirty_ = false;
  return true;
}

bool Game::soundOn(uint32_t ms) {
  if (sound_ == SND_NONE) return false;
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
  else if (!strcmp(cmd, "younger")) { save_.kidAge = 6; refreshBookList(); }
  else if (!strcmp(cmd, "older")) { save_.kidAge = 9; refreshBookList(); }
  markDirty();
}
const char* Game::screenName() const {
  static const char* N[] = {"splash", "intro", "name_pet", "home", "feed", "playmenu", "fetch", "words", "library", "read",
                            "tricks", "train", "bath", "stats", "stickers", "gift", "celebrate", "confirm_reset", "hats", "street", "theme"};
  return N[screen_];
}
void Game::debugPrint() {
  if (screen_ == SC_TRAIN) printf("train: trick=%d phase=%d round=%d len=%d seq=%d%d%d%d%d%d input=%d\n", trickSel_, trainPhase_, trainRound_, seqLen_, seq_[0], seq_[1], seq_[2], seq_[3], seq_[4], seq_[5], seqInput_);
  printf("[%s/%s age=%d] day=%d stage=%d food=%d fun=%d energy=%d clean=%d bond=%d hearts=%d streak=%d books=%d/%d asleep=%d poop=%d dirty=%d gift=%d screen=%d\n",
         save_.kidName, save_.petName, save_.kidAge, pet::ageDays(save_, now_), save_.stage, save_.food, save_.fun, save_.energy, save_.clean,
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
// The shared widgets live in os/ui; these bind them to this frame's input.
static uint8_t inkOn(uint8_t fill) { return ui::inkOn(fill); }
void Game::drawBackButton() { ui::drawBack(); }
bool Game::backButton() { return ui::back(in_); }
void Game::drawToast() {
  if (ms_ >= toastUntil_ || !toast_[0]) return;
  ui::toast(toast_);
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
using ScreenFn = void (Game::*)();
void Game::update(uint32_t nowSec, uint32_t ms, const Input& in) {
  static const ScreenFn UPDATE[] = {&Game::updateSplash, &Game::updateIntro, &Game::updateNamePet, &Game::updateHome, &Game::updateFeed,
    &Game::updatePlayMenu, &Game::updateFetch, &Game::updateWords, &Game::updateLibrary, &Game::updateRead, &Game::updateTricks,
    &Game::updateTrain, &Game::updateBath, &Game::updateStats, &Game::updateStickers, &Game::updateGift, &Game::updateCelebrate,
    &Game::updateConfirmReset, &Game::updateHats, &Game::updateStreet, &Game::updateTheme};
  static_assert(sizeof UPDATE / sizeof UPDATE[0] == SC_THEME + 1, "one update per screen, in Screen order");
  now_ = nowSec; ms_ = ms; in_ = in;
  tapGuard_.filter(in_, ms_, screen_);
  if (haveSave_ && now_ != lastTickSec_) tick();
  if (in_.pressed && haveSave_ && screen_ != SC_SPLASH) pet::touchDay(save_, now_);
  updateParticles();
  (this->*UPDATE[screen_])();
  if (screen_ == SC_HOME) checkStickers();
}
void Game::tick() {
  uint32_t ev = pet::simulate(save_, now_, true);
  if (ev & pet::EV_STAGE_UP) {
    char t[24], b[48];
    snprintf(t, sizeof t, "%s grew up!", save_.petName);
    snprintf(b, sizeof b, save_.stage == STAGE_GROWN ? "All grown up. What a good dog!" : "Not a puppy anymore!");
    celebrate(t, b, 1, SC_HOME);
  }
  if ((now_ % 300) == 0) markDirty();  // periodic checkpoint; actions save on their own
  lastTickSec_ = now_;
}

void Game::render() {
  static const ScreenFn DRAW[] = {&Game::drawSplash, &Game::drawIntro, &Game::drawNamePet, &Game::drawHome, &Game::drawFeed,
    &Game::drawPlayMenu, &Game::drawFetch, &Game::drawWords, &Game::drawLibrary, &Game::drawRead, &Game::drawTricks,
    &Game::drawTrain, &Game::drawBath, &Game::drawStats, &Game::drawStickers, &Game::drawGift, &Game::drawCelebrate,
    &Game::drawConfirmReset, &Game::drawHats, &Game::drawStreet, &Game::drawTheme};
  static_assert(sizeof DRAW / sizeof DRAW[0] == SC_THEME + 1, "one draw per screen, in Screen order");
  (this->*DRAW[screen_])();
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
  if (ms_ - screenMs_ <= 1800 && !in_.tap) return;
  if (kidHas_[self_]) enterHouse();
  else { memset(&save_, 0, sizeof save_); dogAct_ = 0; go(SC_INTRO); }
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
  if (backButton()) { wantsHome_ = true; return; }
  if (dogAct_ == 0 && in_.tap) { dogAct_ = 1; screenMs_ = ms_; spawn(80, 96, 1, 12); }
  else if (dogAct_ == 1 && ms_ - screenMs_ > 900 && in_.tap) { dogAct_ = 0; nameLen_ = 0; nameBuf_[0] = 0; memset(&save_, 0, sizeof save_); go(SC_NAME_PET); }
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
  drawBackButton();
}

// ---------------------------------------------------------------- pup name (the kid's name comes from the profile)
void Game::updateNamePet() {
  if (backButton()) { wantsHome_ = true; return; }   // not adopted yet: back out to the launcher
  if (in_.tapInCircle(123, 28, 12)) {                  // random name (hit circle ends 0.5 px inside the bezel)
    const char* n = PET_NAME_IDEAS[pet::rnd(save_) % 9]; strncpy(nameBuf_, n, ui::NAME_LEN); nameBuf_[ui::NAME_LEN] = 0; nameLen_ = (int)strlen(nameBuf_);
    return;                                            // its hit circle overlaps key row 0: one tap, one action
  }
  if (ui::keyboard(in_, nameBuf_, nameLen_)) { strncpy(save_.petName, nameBuf_, sizeof save_.petName - 1); go(SC_THEME); }
}
void Game::drawNamePet() {
  clear(C_WALL);
  ui::drawKeyboard(in_, nameBuf_, "Pup's name", ms_);
  circle(123, 29, 9, C_DKBROWN); circle(123, 28, 8, C_PINK); textCentered(123, 25, "?", C_DKBROWN);
  drawBackButton();
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

// Home layout: the back button sits top center; the stat pips (they open the stats page) stand in two pairs either side.
static const int PIPS_L = 30, PIPS_R = 106, PIPS_Y = 19;
void Game::updateHome() {
  if (backButton()) { wantsHome_ = true; return; }
  if (in_.tapIn(16, 36, 22, 32)) { go(SC_STREET); return; }   // the front door: the neighbours on Paw Street
  homeBrain();
  homeEvents();
  homeTouches();
  if (in_.longPress && !save_.asleep) {
    int dogW = dogFrame(P_IDLE0).w, dogTop = DOG_BASE_Y - dogFrame(P_IDLE0).h;
    if (in_.hit(dogX_ - 4, dogTop - 6, dogW + 8, dogFrame(P_IDLE0).h + 8) && !save_.dirty) { pet::petDog(save_, now_); spawn(in_.x, in_.y - 6, 0, 6); dogAct_ = 2; markDirty(); }
  }
}
void Game::homeBrain() {   // idle wandering, sitting and showing off tricks
  if (!save_.asleep && dogAct_ != 10 && ms_ >= nextIdleMs_) pickIdle();
  if (dogAct_ == 1) homeWalk();
  if (dogAct_ == 10 && ms_ >= dogActUntil_) { dogAct_ = 0; trickShow_ = -1; if (!save_.asleep) { save_.tricksShown++; pet::addBond(save_, now_, 1); markDirty(); } }
  if ((dogAct_ == 3 || dogAct_ == 4 || dogAct_ == 5) && ms_ >= dogActUntil_) dogAct_ = 0;
}
void Game::pickIdle() {
  int r = (int)(pet::rnd(save_) % 100);
  if (r < 35) { dogAct_ = 1; dogTargetX_ = 16 + (int)(pet::rnd(save_) % (uint32_t)(112 - dogFrame(P_IDLE0).w)); dogFlip_ = dogTargetX_ < dogX_; }
  else if (r < 50) dogAct_ = 2;
  else if (r < 58 && pet::learnedTrickCount(save_) > 0) { int t; do { t = (int)(pet::rnd(save_) % NUM_TRICKS); } while (!pet::trickLearned(save_, t)); startTrickShow(t); }
  else dogAct_ = 0;
  nextIdleMs_ = ms_ + 2500 + pet::rnd(save_) % 4000;
  if (dogAct_ == 1) nextIdleMs_ = ms_ + 6000;
}
void Game::homeWalk() {
  if (dogX_ < dogTargetX_) dogX_++; else if (dogX_ > dogTargetX_) dogX_--; else { dogAct_ = 0; }
  if ((ms_ / 33) % 2) { if (dogX_ < dogTargetX_) dogX_++; else if (dogX_ > dogTargetX_) dogX_--; }
}
void Game::homeEvents() {   // butterflies, the squirrel at the window, rain
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
}
// Every hotspot is tested each frame (the UI audit relies on it); only a tap acts, and only the first one that matches.
void Game::homeTouches() {
  int dogW = dogFrame(P_IDLE0).w, dogTop = DOG_BASE_Y - dogFrame(P_IDLE0).h;
  bool onDog = in_.tapIn(dogX_ - 4, dogTop - 6, dogW + 8, dogFrame(P_IDLE0).h + 8);
  if (event_ == 1 && inCircle(eventX_ + 4, eventY_ + 3, 14) && in_.tapIn(eventX_ - 8, eventY_ - 8, 24, 22)) {  // caught the butterfly's attention (once it is on the glass)
    event_ = 0; dogAct_ = 3; dogActUntil_ = ms_ + 1500; spawn(in_.x, in_.y, 1, 6);
    save_.fun = (uint8_t)clampi(save_.fun + 5, 0, 100); pet::addBond(save_, now_, 2); markDirty();
  } else if (save_.poop && in_.tapIn(dogX_ > 70 ? 30 : 100, 98, 22, 20)) {
    pet::cleanPoop(save_, now_); spawn(in_.x, in_.y, 1, 8); toast("All clean!"); markDirty();
  } else if (pet::giftReady(save_, now_) && in_.tapIn(76, 46, 28, 22)) {
    giftOpened_ = false; go(SC_GIFT);
  } else if (in_.tapIn(10, 64, 26, 30)) toggleLamp();
  else if (save_.asleep && in_.tap) toast("Shh... sleeping");
  else if (onDog) tapDog();
  else homeFurniture();
}
void Game::homeFurniture() {   // the floor bowl, the toy ball, the bookshelf and the stat pips open their pages
  if (in_.tapIn(16, 96, 32, 20)) { go(SC_FEED); feedAnimFood_ = -1; }
  else if (in_.tapIn(118, 92, 28, 22)) { go(SC_PLAYMENU); }
  else if (in_.tapIn(110, 38, 40, 56)) { go(SC_LIBRARY); }
  else if (in_.tapIn(PIPS_L - 4, PIPS_Y - 3, 36, 18) || in_.tapIn(PIPS_R - 6, PIPS_Y - 3, 36, 18)) { statsPage_ = 0; go(SC_STATS); }
}
void Game::toggleLamp() {
  if (save_.asleep) {
    if (pet::canWake(save_)) { pet::setAsleep(save_, now_, false); toast("Good morning!"); }
    else { char b[40]; snprintf(b, sizeof b, "%s is too sleepy", save_.petName); toast(b); }
  } else { pet::setAsleep(save_, now_, true); toast("Sweet dreams..."); pet::addBond(save_, now_, pet::isNightHour(pet::hourOf(now_)) ? 5 : 1); }
  markDirty();
}
void Game::tapDog() {
  if (save_.dirty) { startBath(); return; }
  pet::petDog(save_, now_); spawn(in_.x, in_.y - 6, 0, 3); dogAct_ = 3; dogActUntil_ = ms_ + 900; markDirty();
}
void Game::drawHome() {
  drawRoom();
  drawStatPips(PIPS_L, PIPS_Y, save_.food, SPR_BONE, C_ORANGE);
  drawStatPips(PIPS_L, PIPS_Y + 10, save_.fun, SPR_BALL, C_RED);
  drawStatPips(PIPS_R, PIPS_Y, save_.energy, SPR_MOON, C_BLUE);
  drawStatPips(PIPS_R, PIPS_Y + 10, save_.clean, SPR_BUBBLE, C_WATER);
  if (pet::giftReady(save_, now_)) {  // the mail arrives on the window sill
    int b = (ms_ / 300) % 2; blit(SPR_PARCEL, 83, 52 - b);
    textCentered(90, 38 - b * 2, "!", C_RED);
  }
  if (save_.poop) blit(SPR_POOP, dogX_ > 70 ? 36 : 106, 104);
  bool flip = dogFlip_;
  int pose = dogAct_ == 10 && trickShow_ >= 0 ? trickPose(flip) : idlePose();
  drawDog(dogX_, DOG_BASE_Y, pose, flip);
  drawThought();
  if (save_.asleep) { textCentered(dogX_ + 20, DOG_BASE_Y - 40 - (int)((ms_ / 400) % 4), "z", C_NAVY); textCentered(dogX_ + 26, DOG_BASE_Y - 46 - (int)((ms_ / 400) % 4), "Z", C_NAVY); }
  if (event_ == 1) blit((ms_ / 150) % 2 ? SPR_BUTTERFLY0 : SPR_BUTTERFLY1, eventX_, eventY_ + (int)((ms_ / 200) % 3));
  drawHomeButtons();
  drawBackButton();
}
int Game::trickPose(bool& flip) {
  uint32_t t = ms_ % 600;
  switch (trickShow_) {
    case 0: return P_SIT;
    case 1: return t < 300 ? P_SIT : P_SIT_PAW;
    case 2: if (t < 300) { textCentered(dogX_ + 16, DOG_BASE_Y - 34, "Woof!", C_DKBROWN); return P_BARK; } return P_SIT;
    case 3: flip = (ms_ / 240) % 2; blit(SPR_DUST, dogX_ + 12, DOG_BASE_Y - 3); return (ms_ / 120) % 2 ? P_WALK0 : P_WALK1;
    case 4: flip = (ms_ / 300) % 4 >= 2; return (ms_ / 300) % 2 ? P_BACK : P_IDLE0;
    case 5: return P_DEAD;
    case 6: return P_BEG;
    default: flip = (ms_ / 400) % 2; if ((ms_ / 200) % 3 == 0) blit(SPR_NOTE, dogX_ + 30, DOG_BASE_Y - 34); return (ms_ / 200) % 2 ? P_JUMP : P_BEG;
  }
}
void Game::drawThought() {
  Want w = pet::computeWant(save_, now_);
  if (w == WANT_NONE || save_.asleep || dogAct_ == 10 || (ms_ / 3000) % 2) return;
  int bx = dogX_ + (dogFlip_ ? -14 : dogFrame(P_IDLE0).w - 6), by = DOG_BASE_Y - dogFrame(P_IDLE0).h - 18;
  bx = clampi(bx, 8, 134);
  roundRect(bx, by, 18, 14, C_WHITE); frame(bx, by, 18, 14, C_DKGRAY); pixel(bx + (dogFlip_ ? 15 : 2), by + 15, C_WHITE); pixel(bx + (dogFlip_ ? 17 : 0), by + 17, C_WHITE);
  const Sprite* ic = &SPR_BONE;
  switch (w) { case WANT_PLAY: ic = &SPR_BALL; break; case WANT_SLEEP: ic = &SPR_MOON; break; case WANT_BATH: ic = &SPR_TUB; break; case WANT_POOP: ic = &SPR_POOP; break; case WANT_STORY: ic = &SPR_BOOK; break; default: break; }
  blit(*ic, bx + (18 - ic->w) / 2, by + (14 - ic->h) / 2);
}
void Game::drawHomeButtons() {
  static const Sprite* BTN_ICONS[4] = {&SPR_BOWL, &SPR_BALL, &SPR_BOOK, &SPR_PAW};
  static const uint8_t BTN_COLS[4] = {C_ORANGE, C_GREEN, C_BLUE, C_PLUM};
  for (int i = 0; i < 4; i++) {
    bool pr = in_.down && in_.hit(BTN_X[i] - 2, BTN_Y - 2, BTN_W + 4, BTN_H + 4);
    ui::drawButton({{BTN_X[i], BTN_Y, BTN_W, BTN_H}, nullptr, nullptr, BTN_COLS[i]}, pr);
    const Sprite& ic = *BTN_ICONS[i]; int ix = BTN_X[i] + (BTN_W - ic.w * 2) / 2, iy = BTN_Y + (BTN_H - ic.h * 2) / 2 + (pr ? 1 : 0);
    if (i == 3) { for (int sy = 0; sy < ic.h; sy++) for (int sx = 0; sx < ic.w; sx++) if (ic.px[sy * ic.w + sx] != C_T) rect(ix + sx * 2, iy + sy * 2, 2, 2, C_WHITE); }
    else blitScaled(ic, ix, iy, 2);
    if (!in_.tapIn(BTN_X[i] - 3, BTN_Y - 3, BTN_W + 6, BTN_H + 6)) continue;
    if (i == 0) { go(SC_FEED); feedAnimFood_ = -1; } else if (i == 1) go(SC_PLAYMENU); else if (i == 2) go(SC_LIBRARY); else go(SC_TRICKS);
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
  ui::panel({14, 40, 132, 62}, C_CREAM, C_DKBROWN);
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
  ui::panel({24, 34, 112, 100}, C_CREAM, C_DKBROWN);
  bool p1 = in_.down && in_.hit(30, 40, 100, 40), p2 = in_.down && in_.hit(30, 88, 100, 40);
  ui::drawButton({{34, 42, 92, 36}, "Fetch!", nullptr, C_GREEN}, p1); blitScaled(SPR_BALL, 40, 52, 2); blitScaled(SPR_BONE, 96, 55, 2);
  ui::drawButton({{34, 90, 92, 36}, "Word Fetch", nullptr, C_BLUE}, p2); blitScaled(SPR_BOOK, 40, 96, 1); text(106, 96, "A", C_DKBROWN, 2);
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
  drawFetchHud();
  if (!fetchDone_) {
    if (ms_ - screenMs_ < 2000) { textCenteredShadow(80, 60, "Catch the bones!", C_NAVY, C_WHITE); textCentered(80, 72, "(slide your finger)", C_NAVY); }
    drawBackButton();
    return;
  }
  ui::panel({30, 46, 100, 50}, C_CREAM, C_DKBROWN);
  textCentered(80, 52, "Time's up!", C_DKBROWN);
  char b[16]; snprintf(b, sizeof b, "%d bones!", fetchScore_); textCentered(80, 66, b, C_PLUM, 1);
  textCentered(80, 80, fetchScore_ >= 12 ? "Amazing!" : fetchScore_ >= 6 ? "Great job!" : "Good try!", C_DKBROWN);
}
void Game::drawFetchHud() {   // score and seconds left, either side of the home button
  int left = fetchDone_ ? 0 : (int)((fetchEndMs_ - ms_) / 1000);
  ui::panel({30, 14, 36, 13}, C_WHITE, C_DKBROWN);
  blit(SPR_BONE, 33, 17); char b[16]; snprintf(b, sizeof b, "%d", fetchScore_); text(46, 17, b, C_DKBROWN);
  ui::panel({94, 14, 36, 13}, C_WHITE, C_DKBROWN);
  snprintf(b, sizeof b, "%ds", left); textCentered(112, 17, b, left <= 5 ? C_RED : C_DKBROWN);
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
  if (wordsDone_) { drawWordsDone(); return; }
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
  drawWordTiles();
  if (wordSolvedUntil_) textCenteredShadow(80, 96, "WOOF! Got it!", C_PLUM, C_WHITE);
  snprintf(b, sizeof b, "word %d/5", wordRound_ + 1); textCentered(80, wordCols_ == 4 ? 140 : 145, b, C_DKGRAY);   // 146 cuts the corners at the rim
  drawBackButton();
}
void Game::drawWordsDone() {
  ui::panel({26, 40, 108, 60}, C_CREAM, C_DKBROWN);
  textCentered(80, 46, "Word Fetch done!", C_DKBROWN);
  char b[32]; snprintf(b, sizeof b, "%d of 5 words", wordCorrect_); textCentered(80, 62, b, C_NAVY);
  textCentered(80, 78, "Super speller!", C_DKBROWN);
  drawDog(64, 140, P_JUMP, false);
}
void Game::drawWordTiles() {
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
  ui::button(in_, {{44, 118, 72, 24}, done ? "Read again" : "Read to me!", nullptr, C_GREEN});
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
  if (page_ < npages) drawReadPage(top, npages); else drawReadQuestion(tn == 2 ? 44 : 36);
  drawBackButton();
}
void Game::drawReadPage(int top, int npages) {
  char lines[8][40]; int nl = wrap(curBook().pages[page_], 122, lines, top == 46 ? 7 : 8);
  for (int i = 0; i < nl; i++) text(19, top + i * 10, lines[i], C_DKBROWN);
  for (int i = 0; i < npages; i++) circle(80 - npages * 3 + i * 6, 130, i == page_ ? 2 : 1, i == page_ ? C_PLUM : C_LTGRAY);
  blitTint(SPR_ARROW, 118, 124, C_ORANGE);
  if (page_ > 0) blitTint(SPR_ARROW, 36, 124, C_ORANGE, true);
  // listening dog
  int pose = (ms_ / 700) % 5 == 0 ? P_LISTEN : P_SIT;
  drawDog(58, 152, pose, false);
  if ((ms_ / 5000) % 3 == 1) blit(SPR_HEART, 84, 128);
}
void Game::drawReadQuestion(int qTop) {
  const Book& bk = curBook();
  char lines[3][40]; int nl = wrap(bk.question, 116, lines, 3);
  for (int i = 0; i < nl; i++) textCentered(80, qTop + i * 9, lines[i], C_DKBROWN);
  int ay = qTop + nl * 9 + 4;
  int na = 0; while (na < 3 && bk.answers[na]) na++;
  for (int i = 0; i < na; i++) {
    uint8_t col = answerPick_ < 0 ? C_BLUE : i == bk.correct ? C_GREEN : i == answerPick_ ? C_RED : C_LTGRAY;
    bool pr = in_.down && in_.hit(26, ay + i * 22, 108, 21);
    int by = ay + i * 22 + (pr ? 1 : 0);
    ui::drawButton({{26, ay + i * 22, 108, 20}, nullptr, nullptr, col}, pr);
    char al[2][40]; int an = wrap(bk.answers[i], 102, al, 2);
    for (int k = 0; k < an; k++) textCentered(80, by + (an == 2 ? 2 + k * 9 : 6), al[k], inkOn(col));
  }
  if (answerPick_ >= 0) drawDog(64, 158, answerPick_ == bk.correct ? P_JUMP : P_SAD, false);
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
  ui::panel({18, 28, 124, 110}, C_CREAM, C_DKBROWN);
  for (int i = 0; i < NUM_TRICKS; i++) {
    int x = i % 2 ? 82 : 20, y = 34 + (i / 2) * 24;
    bool learned = pet::trickLearned(save_, i), avail = pet::trickAvailable(save_, i);
    uint8_t col = learned ? C_GREEN : avail ? C_BLUE : C_DKGRAY, ink = inkOn(col);
    bool pr = in_.down && in_.hit(x - 2, y - 2, 64, 24);
    int dy = pr ? 1 : 0;
    ui::drawButton({{x, y, 60, 21}, nullptr, nullptr, col}, pr);
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
  if (in_.tapIn(26, 112, 54, 22)) { go(SC_STICKERS); }
  if (in_.tapIn(84, 112, 50, 22)) { hatSel_ = save_.hat; go(SC_HATS); }
  if (in_.longPress && in_.hit(60, 140, 40, 16)) go(SC_CONFIRM_RESET);
}
void Game::drawStats() {
  clear(C_WALL);
  ui::panel({16, 22, 128, 88}, C_CREAM, C_DKBROWN);
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
  ui::button(in_, {{26, 112, 54, 22}, "Stickers", nullptr, C_PINK});
  ui::button(in_, {{84, 112, 50, 22}, "Hats", nullptr, C_PLUM});
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
  ui::button(in_, {{50, 122, 60, 22}, save_.hat == hatSel_ ? "Wearing" : "Wear it", nullptr, save_.hat == hatSel_ ? C_DKGRAY : C_GREEN});
  int owned = 0; for (int i = 1; i < 8; i++) if (save_.hatsMask & (1u << i)) owned++;
  char b[24]; snprintf(b, sizeof b, "%d hat%s found", owned, owned == 1 ? "" : "s"); textCentered(80, 40, b, C_DKGRAY);
  drawBackButton();
}
void Game::updateConfirmReset() {
  if (in_.tapIn(20, 90, 50, 24)) go(SC_STATS);
  if (in_.tapIn(90, 90, 50, 24)) {   // this kid's pup only: the shell erases the save, the profile stays
    haveSave_ = kidHas_[self_] = false; dirty_ = true; dogAct_ = 0;
    memset(&save_, 0, sizeof save_);
    go(SC_INTRO);
  }
}
void Game::drawConfirmReset() {
  clear(C_PLUM);
  textCentered(80, 44, "Start over?", C_WHITE);
  char b[48]; snprintf(b, sizeof b, "%s will forget you!", save_.petName); textCentered(80, 60, b, C_PINK);
  ui::button(in_, {{20, 90, 50, 24}, "No!", nullptr, C_GREEN});
  ui::button(in_, {{90, 90, 50, 24}, "Yes", nullptr, C_RED});
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
  ui::panel({18, 30, 124, 98}, C_CREAM, C_DKBROWN);
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
  ui::panel({14, 28, 132, 70}, C_CREAM, C_GOLD);
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

// ---------------------------------------------------------------- Paw Street: every profile's house, only your own opens
// Slots: one row of up to two houses, or two rows once there are three or four.
static void streetSlot(int i, int n, int& cx, int& gy) {
  int row = n > 2 ? i / 2 : 0, inRow = row ? n - 2 : (n < 2 ? n : 2);
  cx = inRow == 1 ? 80 : (i % 2 ? 110 : 50);
  gy = n > 2 ? 74 + row * 48 : 104;
}
void Game::updateStreet() {
  if (backButton()) { go(SC_HOME); return; }
  int cx, gy; streetSlot(self_, nKids_, cx, gy);
  if (in_.tapIn(cx - 18, gy - 36, 36, 50)) go(SC_HOME);
}
static void fitName(const char* name, char* out, int maxW) {   // cut to fit a name plate; a house is known by its pup too
  int n = 0; out[0] = 0;
  while (name[n] && n < 11) { out[n] = name[n]; out[n + 1] = 0; if (textWidth(out) > maxW) { out[n] = 0; break; } n++; }
}
void Game::drawStreet() {
  int hr = pet::hourOf(now_); bool night = hr >= 20 || hr < 6;
  clear(night ? C_NIGHT : C_SKY);
  if (night) { for (int i = 0; i < 16; i++) { int x = (i * 37) % 160, y = (i * 17) % 50; if (((ms_ / 500) + i) % 3) pixel(x, y, C_WHITE); } blit(SPR_MOON, 118, 20); }
  else { blit(SPR_SUN, 112, 18); blit(SPR_CLOUD, 14 + (ms_ / 300) % 70, 40); }
  rect(0, 56, 160, 104, night ? C_DKGREEN : C_LEAF);
  textCenteredShadow(80, 26, "Paw Street", C_NAVY, C_WHITE);
  for (int i = 0; i < nKids_; i++) drawStreetHouse(i, night);
  drawBackButton();
}
void Game::drawStreetHouse(int i, bool night) {   // the house, its pup by the door (if adopted), the kid's name plate
  int cx, gy; streetSlot(i, nKids_, cx, gy);
  bool mine = i == self_, has = mine ? haveSave_ : kidHas_[i];
  const Save& h = mine ? save_ : kidSaves_[i];
  bool pr = mine && in_.down && in_.hit(cx - 18, gy - 36, 36, 50);
  drawHouseIcon(cx, gy + (pr ? 1 : 0), 24, has ? themeOf(h) : THEMES[0], night && has && !h.asleep);
  if (has && h.asleep) textCentered(cx - 12, gy - 22 - (int)((ms_ / 400) % 3), "z", night ? C_WHITE : C_NAVY);
  else if (has) blit(DOG_FRAMES[SZ_PUP][(ms_ / 500 + i) % 2 ? P_IDLE1 : P_IDLE0], cx - 24, gy - 18, false);
  char nm[12]; fitName(kids_[i].name, nm, 42);
  int w = textWidth(nm) + 6; ui::panel({cx - w / 2, gy + 1, w, 11}, mine ? C_YELLOW : C_CREAM, C_DKBROWN); textCentered(cx, gy + 3, nm, C_DKBROWN);
}

// ---------------------------------------------------------------- house colors
void Game::updateTheme() {
  if (backButton()) { go(SC_NAME_PET); return; }
  for (int i = 0; i < 4; i++) if (in_.tapIn(16 + i * 32, 56, 32, 44)) { save_.theme = (uint8_t)i; finishAdoption(); return; }
}
void Game::drawTheme() {
  clear(C_SKY);
  rect(0, 100, 160, 60, C_LEAF);
  textCentered(80, 30, "Pick your house!", C_NAVY);
  for (int i = 0; i < 4; i++) {
    bool pr = in_.down && in_.hit(16 + i * 32, 56, 32, 44);
    drawHouseIcon(32 + i * 32, 100 + (pr ? 1 : 0), 24, THEMES[i], false);
  }
  char b[40]; snprintf(b, sizeof b, "for %s and %s", kids_[self_].name, save_.petName); textCentered(80, 118, b, C_NAVY);
  drawBackButton();
}
