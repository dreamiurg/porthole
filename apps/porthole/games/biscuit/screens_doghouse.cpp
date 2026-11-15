// The doghouse: the pup's own room, and a shop of things for it bought with the kid's stars (os/stars.h, earned in
// stories, puzzles and other games). The room is the stage's picture with each thing bought drawn over it
// (generated/doghouse_art.h); a thing's page shows it, its price, and Buy once there are enough stars.
#include <stdio.h>
#include "content_doghouse.h"
#include "game.h"
#include "generated/doghouse_art.h"
#include "gfx565.h"
#include "ui565.h"

namespace biscuit {
using namespace ui565;
static_assert(sizeof DECOR / sizeof *DECOR == NUM_DECOR && DECOR_COUNT == NUM_DECOR, "one name and price per thing");
namespace {
constexpr int SHOP_PAGES = (NUM_DECOR + ROWS - 1) / ROWS;
constexpr Button SHOP = {WIDE, "Shop", &FONT20, PEACH, true};
Button shopRow(const Save& p, int row, int page, char* label, size_t cap) {
  const int id = page * ROWS + row;
  if (owns(p, id)) snprintf(label, cap, "%s - at home", DECOR[id].name);
  else snprintf(label, cap, "%s - %u stars", DECOR[id].name, (unsigned)DECOR[id].price);
  return {ROW[row], label, &FONT20, owns(p, id) ? SAGE : PEACH, true};
}
}  // namespace

void Game::updateDoghouse() {
  const Box& back = WORLD_BACK;
  if (in_.tapIn(back.x, back.y, back.w, back.h)) { go(SC_WORLD); return; }
  if (tapped(in_, SHOP)) { page_ = 0; go(SC_SHOP); }
}
void Game::drawDoghouse() {
  packed565::draw(DOGHOUSE_ROOMS[(int)stage(save_)], 0, 0, 3);   // the whole glass
  for (int i = 0; i < NUM_DECOR; i++)
    if (owns(save_, i)) packed565::draw(DECOR_ART[i].image, DECOR_ART[i].x * 3, DECOR_ART[i].y * 3, 3);
  roundButton(in_, WORLD_BACK, "<");
  char s[24];
  snprintf(s, sizeof s, "%d star%s", starCount(), starCount() == 1 ? "" : "s");
  text(HOUSE_STARS, s, INK);
  button(in_, SHOP);
}

void Game::updateShop() {
  if (tapped(in_, BACK_BUTTON)) { go(SC_DOGHOUSE); return; }
  const int turn = navTapped(in_, page_, SHOP_PAGES);
  if (turn) { page_ += turn; fresh(); return; }
  for (int row = 0; row < ROWS && page_ * ROWS + row < NUM_DECOR; row++) {
    char label[48];
    if (tapped(in_, shopRow(save_, row, page_, label, sizeof label))) { item_ = page_ * ROWS + row; go(SC_ITEM); return; }
  }
}
void Game::drawShop() {
  gfx565::clear(PAPER);
  top(in_, "Shop", starCount());
  for (int row = 0; row < ROWS && page_ * ROWS + row < NUM_DECOR; row++) {
    char label[48];
    button(in_, shopRow(save_, row, page_, label, sizeof label));
  }
  nav(in_, page_, SHOP_PAGES);
}

// One thing: Buy while it isn't home yet (on only with enough stars), See it once it is.
static Button itemButton(const Save& p, int id, int stars) {
  if (owns(p, id)) return {WIDE, "See it", &FONT20, SAGE, true};
  return {WIDE, "Buy", &FONT20, PEACH, stars >= DECOR[id].price};
}
void Game::updateItem() {
  if (tapped(in_, BACK_BUTTON)) { go(SC_SHOP); return; }
  if (!tapped(in_, itemButton(save_, item_, starCount()))) return;
  if (!owns(save_, item_) && buyDecor(save_, wallet_, item_, DECOR[item_].price)) markDirty();
  go(SC_DOGHOUSE);
}
void Game::drawItem() {
  gfx565::clear(PAPER);
  const DecorItem& d = DECOR[item_];
  top(in_, d.name, starCount());
  const packed565::Image& img = DECOR_ART[item_].image;
  int scale = ITEM_PICTURE_W / img.width, tall = ITEM_PICTURE_H / img.height;   // as big as its box allows
  if (tall < scale) scale = tall;
  packed565::draw(img, 240 - img.width * scale / 2, ITEM_PICTURE_CY - img.height * scale / 2, scale);
  char s[48];
  const int short_ = (int)d.price - starCount();
  if (owns(save_, item_)) snprintf(s, sizeof s, "It's in the doghouse!");
  else if (short_ > 0) snprintf(s, sizeof s, "%u stars\n%d more to go!", (unsigned)d.price, short_);
  else snprintf(s, sizeof s, "%u stars", (unsigned)d.price);
  text(ITEM_PRICE, s, INK);
  button(in_, itemButton(save_, item_, starCount()));
}
}  // namespace biscuit
