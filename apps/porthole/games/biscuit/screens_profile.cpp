// The pup's name (first naming and renaming, on the shell's indexed name keyboard), the scrapbook page and the
// sticker album.
#include <stdio.h>
#include <string.h>
#include "game.h"
#include "generated/scenes.h"
#include "gfx.h"
#include "gfx565.h"
#include "personalize.h"
#include "ui565.h"

namespace biscuit {
using namespace ui565;
namespace {
constexpr Button ALBUM = {PREV, "Our stickers", &FONT20, PEACH, true}, RENAME = {NEXT, "Rename pup", &FONT20, PURPLE, true};
constexpr int STICKER_PAGES = (NUM_STICKERS + STICKER_ROWS - 1) / STICKER_ROWS;
}  // namespace

// The keyboard types up to ui::NAME_LEN letters: a longer name (from before Porthole) starts the field empty.
void Game::startNaming(const char* current) {
  const size_t n = strlen(current);
  nameLen_ = n <= (size_t)ui::NAME_LEN ? (int)n : 0;
  memcpy(nameBuf_, current, (size_t)nameLen_); nameBuf_[nameLen_] = 0;
  namePage_ = 0;
}

// First visit: name the pup. Back leaves for the launcher; nothing is saved until it has a name.
void Game::updateSetupPet() {
  if (ui::back(in_)) { wantsHome_ = true; return; }
  if (!ui::keyboard(in_, nameBuf_, nameLen_, namePage_) || !setPetName(save_, nameBuf_, now_)) return;
  markDirty(); lastTickSec_ = lastCheckpointSec_ = now_;
  go(SC_HOME);
  say(SAY_HELLO, SCENE_IDLE);
}
void Game::drawSetupPet() {
  gfx::clear(C_WALL);
  ui::drawKeyboard(in_, nameBuf_, namePage_, "Pup's name", ms_);
  ui::drawBack();
}
void Game::updateRenamePet() {   // a rename keeps everything but the name
  if (ui::back(in_)) { go(SC_PROFILE); return; }
  if (!ui::keyboard(in_, nameBuf_, nameLen_, namePage_) || !setPetName(save_, nameBuf_, now_)) return;
  markDirty();
  go(SC_PROFILE);
}
void Game::drawRenamePet() {
  gfx::clear(C_WALL);
  ui::drawKeyboard(in_, nameBuf_, namePage_, "New name", ms_);
  ui::drawBack();
}

// ---------------------------------------------------------------- the scrapbook
void Game::updateProfile() {
  if (tapped(in_, BACK_BUTTON)) { go(SC_WORLD); return; }
  if (tapped(in_, ALBUM)) {   // open on today's sticker once it is earned
    page_ = save_.dailyClaimed ? dailySticker(save_.lastVisitDay) / STICKER_ROWS : 0;
    go(SC_STICKERS);
    return;
  }
  if (tapped(in_, RENAME)) { startNaming(save_.petName); go(SC_RENAME_PET); }
}
void Game::drawProfile() {
  gfx565::clear(PAPER);
  top(in_, "Our scrapbook", stars(save_));
  char s[96];
  const bool bookworm = save_.stories & (save_.stories - 1);   // two stories or more
  snprintf(s, sizeof s, "%s - %s", STAGES[(int)stage(save_)], bookworm ? "Bookworm" : "Cuddlebug");
  text(BOOK_BADGE, s, INK);
  char lines[128];   // the numbers first: personalize() then fills the names (letters only, never a %)
  snprintf(s, sizeof s, "{name} & {pet}\nDay %u together\n%u friendship\n%d story stars", (unsigned)save_.daysTogether,
           (unsigned)save_.friendship, stars(save_));
  personalize(s, who_.name, save_.petName, lines, sizeof lines);
  text(BOOK_LINES, lines, INK);
  button(in_, ALBUM);
  button(in_, RENAME);
}

// ---------------------------------------------------------------- the sticker album: one a completed day's adventure
void Game::updateStickers() {
  if (tapped(in_, BACK_BUTTON)) { go(SC_PROFILE); return; }
  const int turn = navTapped(in_, page_, STICKER_PAGES);
  if (turn) page_ += turn;
}
void Game::drawStickers() {
  gfx565::clear(PAPER);
  top(in_, "Our sticker album", stars(save_));
  for (int row = 0; row < STICKER_ROWS; row++) {
    const int i = page_ * STICKER_ROWS + row;
    Label l = STICKER;
    l.y = (int16_t)(l.y + row * STICKER_STEP);
    text(l, save_.stickers & (1u << i) ? STICKERS[i] : "A little surprise awaits", INK);
  }
  nav(in_, page_, STICKER_PAGES);
}
}  // namespace biscuit
