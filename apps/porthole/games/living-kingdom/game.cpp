#include "game.h"
#include <stdio.h>
#include <string.h>
#include "layout.h"

namespace kingdom {
namespace {
constexpr font::Box BODY_BOX = {&art::BODY, layout::TEXT_W, layout::TEXT_H, 2, font::Align::LEFT};
constexpr font::Box FIRST_BOX = {&art::BODY, layout::TEXT_W, layout::FIRST_TEXT_H, 2, font::Align::LEFT};
font::Box readerBox(uint8_t mode) {
  font::Box box = BODY_BOX;
  if (mode == JOURNAL) box.h = layout::JOURNAL_TEXT_H;
  return box;
}
const ui::Box CARDS[] = {layout::CARD0, layout::CARD1, layout::CARD2};
const ui::Box PARTY_TILES[] = {layout::PARTY0, layout::PARTY1, layout::PARTY2};
const ui::Box ITEM_TILES[] = {layout::SLOT0, layout::SLOT1, layout::SLOT2, layout::SLOT3, layout::SLOT4, layout::SLOT5};
int overviewTap(const Input& input, uint8_t mode) {
  const bool party = mode == PARTY_OVERVIEW;
  const ui::Box* tiles = party ? PARTY_TILES : ITEM_TILES;
  for (int i = 0; i < (party ? 3 : 6); i++) if (ui::clicked(input, tiles[i])) return i;
  return -1;
}
const ui::Box TOOLS[] = {layout::TOOL0, layout::TOOL1, layout::TOOL2, layout::TOOL3, layout::TOOL4, layout::TOOL5};
} // namespace

void Game::enter(const AppEnter& e) {
  const uint8_t profile = e.who ? e.who->id : 0;
  newCampaign(save_, e.nowSec ^ ((uint32_t)profile + 1) * 0x9e3779b9u, profile);
  fresh_ = true;
  for (int i = 0; i < e.n; i++)
    if (e.all[i].id == profile && e.saves[i].len && loadSave(save_, e.saves[i].data, e.saves[i].len, profile)) fresh_ = false;
  ms_ = e.ms; in_ = {}; title_ = true; home_ = false;
  changed();
}

void Game::changed() {
  dirty_ = true; gate_.shown(ms_); frames_.reset(); prepare();
}
void Game::mode(uint8_t value) {
  save_.cursor.mode = value; save_.cursor.page = 0;
  changed();
}
bool Game::clicked(const ui::Box& box) const { return ui::clicked(in_, box); }

void Game::prepareStory(const Scene& scene) {
  size_t used = 0;
  for (int i = 0; i < scene.paragraphCount; i++) {
    const int n = snprintf(prose_ + used, sizeof prose_ - used, "%s%s", i ? "\n\n" : "", scene.paragraphs[i]);
    if (n < 0 || (size_t)n >= sizeof prose_ - used) break;
    used += (size_t)n;
  }
  snprintf(heading_, sizeof heading_, "%s", scene.title);
}

void Game::prepare() {
  prose_[0] = heading_[0] = 0;
  const Scene* scene = currentScene(save_);
  if (save_.cursor.mode == READ && scene) prepareStory(*scene);
  if (save_.cursor.mode == RESULT && scene && save_.cursor.selectedChoice < scene->choiceCount) {
    const Choice& c = scene->choices[save_.cursor.selectedChoice];
    snprintf(prose_, sizeof prose_, "%s", c.result);
    snprintf(heading_, sizeof heading_, "The kingdom remembers");
  }
  if (save_.cursor.mode == CONFIRM && scene && save_.cursor.selectedChoice < scene->choiceCount) {
    const Choice& c = scene->choices[save_.cursor.selectedChoice];
    snprintf(heading_, sizeof heading_, "%s", c.caption);
    snprintf(prose_, sizeof prose_, "%s\n\n%s", c.detail,
      allowed(save_, c) ? "Read again or choose another option before you decide." : requirementReason(save_, c));
  }
  if (save_.cursor.mode >= PARTY && save_.cursor.mode <= JOURNAL) prepareInfo();
  const bool illustrated = save_.cursor.mode == READ || save_.cursor.mode == RESULT;
  reader_.open(prose_, readerBox(save_.cursor.mode), illustrated ? &FIRST_BOX : nullptr);
  if (save_.cursor.page >= reader_.count()) save_.cursor.page = (uint16_t)(reader_.count() - 1);
}

void Game::openMenu() { save_.returnCursor = save_.cursor; mode(MENU); }
void Game::closeInfo() { save_.cursor = save_.returnCursor; changed(); }

void Game::updateTitle() {
  if (clicked(layout::HOME)) { home_ = true; return; }
  if (clicked(layout::START)) { title_ = fresh_ = false; changed(); }
}
void Game::updateMap() {
  for (int i = 0; i < LOCATION_COUNT; i++) {
    if (!clicked(layout::PLACES[i])) continue;
    save_.returnCursor = save_.cursor; mode(TRAVEL); save_.cursor.infoPage = (uint8_t)i; changed(); return;
  }
  if (clicked(layout::MENU)) openMenu();
}
void Game::updateRead() {
  if (clicked(layout::PREV)) {
    if (!save_.cursor.page) { home_ = true; return; }
    save_.cursor.page--; changed(); return;
  }
  if (clicked(layout::NEXT)) {
    if (save_.cursor.page + 1 >= reader_.count()) mode(CHOICE);
    else { save_.cursor.page++; changed(); }
    return;
  }
  if (clicked(layout::MENU)) openMenu();
}
void Game::updateChoice() {
  const Scene* scene = currentScene(save_); if (!scene) { mode(MAP); return; }
  if (clicked(layout::MENU)) { mode(READ); return; }
  if (clicked(layout::PREV)) { openMenu(); return; }
  if (clicked(layout::NEXT) && scene->choiceCount > 3) {
    save_.cursor.choicePage = (uint8_t)((save_.cursor.choicePage + 1) % ((scene->choiceCount + 2) / 3));
    changed(); return;
  }
  for (int i = 0; i < 3; i++) {
    const int index = save_.cursor.choicePage * 3 + i;
    if (index >= scene->choiceCount || !clicked(CARDS[i])) continue;
    save_.cursor.selectedChoice = (uint8_t)index;
    if (scene->choices[index].confirm || !allowed(save_, scene->choices[index])) mode(CONFIRM);
    else if (commit(save_, (uint8_t)index)) changed();
    return;
  }
}
void Game::updateConfirm() {
  const Scene* scene = currentScene(save_); if (!scene) { mode(MAP); return; }
  if (clicked(layout::PREV)) {
    if (save_.cursor.page) { save_.cursor.page--; changed(); }
    else mode(CHOICE);
    return;
  }
  if (clicked(layout::MENU)) { mode(READ); return; }
  if (clicked(layout::NEXT) && save_.cursor.selectedChoice < scene->choiceCount) {
    if (save_.cursor.page + 1 < reader_.count()) { save_.cursor.page++; changed(); }
    else if (commit(save_, save_.cursor.selectedChoice)) changed();
  }
}
void Game::updateResult() {
  if (clicked(layout::PREV)) {
    if (save_.cursor.page) { save_.cursor.page--; changed(); }
    else openMenu();
    return;
  }
  if (clicked(layout::MENU)) { openMenu(); return; }
  if (!clicked(layout::NEXT)) return;
  if (save_.cursor.page + 1 < reader_.count()) save_.cursor.page++;
  else finishResult(save_);
  changed();
}
void Game::updateMenu() {
  if (clicked(layout::MENU)) { closeInfo(); return; }
  for (int i = 0; i < 6; i++) {
    if (!clicked(TOOLS[i])) continue;
    if (i < 3) {
      const uint8_t modes[] = {PARTY_OVERVIEW, INVENTORY_OVERVIEW, JOURNAL};
      save_.cursor.infoPage = 0; mode(modes[i]);
    }
    else if (i == 3) { save_.cursor = save_.returnCursor; home_ = true; changed(); }
    else if (i == 4) mode(REST);
    else if (i == 5) closeInfo();
    return;
  }
}
void Game::updateInfo() {
  if (clicked(layout::MENU)) { closeInfo(); openMenu(); return; }
  const uint8_t current = save_.cursor.mode;
  if (current == PARTY_OVERVIEW || current == INVENTORY_OVERVIEW) {
    if (clicked(layout::PREV)) { closeInfo(); return; }
    const int selected = overviewTap(in_, current);
    if (selected >= 0) {
      save_.cursor.infoPage = (uint8_t)selected;
      mode(current == PARTY_OVERVIEW ? PARTY : INVENTORY);
    }
    return;
  }
  if (clicked(layout::PREV)) {
    if (current != JOURNAL && !save_.cursor.page) mode(current == PARTY ? PARTY_OVERVIEW : INVENTORY_OVERVIEW);
    else infoStep(-1);
  } else if (clicked(layout::NEXT)) infoStep(1);
}
void Game::updateTravel() {
  if (clicked(layout::PREV)) { closeInfo(); return; }
  if (clicked(layout::MENU)) { closeInfo(); openMenu(); return; }
  if (clicked(layout::NEXT)) {
    const Location destination = (Location)save_.cursor.infoPage;
    save_.cursor.mode = MAP;
    visit(save_, destination); changed();
  }
}
void Game::updateRest() {
  if (clicked(layout::PREV)) { closeInfo(); return; }
  if (clicked(layout::MENU)) { closeInfo(); openMenu(); return; }
  if (save_.returnCursor.mode == MAP && clicked(layout::NEXT)) { save_.cursor.mode = MAP; rest(save_); mode(MAP); }
}
void Game::update(uint32_t, uint32_t ms, const Input& in) {
  ms_ = ms; in_ = in; gate_.filter(in_, ms);
  if (title_) { updateTitle(); return; }
  switch (save_.cursor.mode) {
    case MAP: updateMap(); break;
    case READ: updateRead(); break;
    case CHOICE: updateChoice(); break;
    case CONFIRM: updateConfirm(); break;
    case RESULT: updateResult(); break;
    case MENU: updateMenu(); break;
    case PARTY: case INVENTORY: case JOURNAL: case PARTY_OVERVIEW: case INVENTORY_OVERVIEW: updateInfo(); break;
    case TRAVEL: updateTravel(); break;
    case REST: updateRest(); break;
    default: mode(MAP); break;
  }
}
bool Game::takeSave(const void** data, size_t* len, bool allowed) {
  if (!allowed || !dirty_) return false;
  sealSave(save_); *data = &save_; *len = sizeof save_; dirty_ = false; return true;
}
const char* Game::screenName() const {
  static const char* N[] = {"lk_title", "lk_map", "lk_read", "lk_choice", "lk_confirm", "lk_result",
    "lk_party", "lk_inventory", "lk_journal", "lk_menu", "lk_travel", "lk_rest",
    "lk_party_overview", "lk_inventory_overview"};
  return title_ ? N[0] : save_.cursor.mode < sizeof N / sizeof *N ? N[save_.cursor.mode] : "lk_map";
}
void Game::debugPrint() {
  printf("[kingdom seed=%lu day=%u location=%u scene=%u page=%u pages=%d outcome=%u deeds=%u journal=%u]\n",
    (unsigned long)save_.seed, save_.day, save_.location, save_.cursor.scene, save_.cursor.page,
    reader_.count(), save_.outcome, save_.deeds, save_.journalCount);
  printf("screen=%s\n", screenName());
}
} // namespace kingdom
