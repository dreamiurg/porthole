#include "game.h"
#include <math.h>
#include <stdlib.h>
#include <stdio.h>
#include "crc32.h"
#include "gfx565.h"
#include "layout.h"

namespace kingdom {
namespace {
using namespace art;
constexpr uint16_t SECONDARY = rgb888_to_565(0xB89DBE);
constexpr uint16_t MUTED = rgb888_to_565(0x51425B);
void label(const char* text, int y, int height = 52, uint16_t color = INK) {
  font::textBox({&SMALL, 348, (int16_t)height, 0, font::Align::CENTER}, text, 66, y, color);
}
void paper(int x, int y, int w, int h, uint16_t fill = CREAM) {
  const PanelStyle style = fill == GOLD ? PanelStyle::GOLD : fill == SECONDARY ? PanelStyle::VIOLET
    : fill == NAVY ? PanelStyle::DARK : PanelStyle::PAPER;
  art::panel(x, y, w, h, style);
}
void button(const ui::Box& b, const char* text, uint16_t color = GOLD) {
  paper(b.x * 3, b.y * 3, b.w * 3, b.h * 3, color);
  const font::Font& face = font::textWidth(SMALL, text) <= b.w * 3 - 8 ? SMALL : DETAIL;
  const int width = font::textWidth(face, text);
  font::text(face, text, b.x * 3 + (b.w * 3 - width) / 2,
    b.y * 3 + (b.h * 3 - face.lineHeight) / 2, INK);
}
void header(const char* text) {
  const font::Font& face = font::textWidth(HEADING, text) <= 296 ? HEADING : SMALL;
  const int height = font::textHeight(face, text, 296, 0);
  paper(84, height > 28 ? 48 : 42, 312, height > 28 ? 66 : 54, PARCHMENT);
  font::textBox({&face, 296, 52, 0, font::Align::CENTER}, text, 92, (height > 28 ? 84 : 72) - height / 2, INK);
}
void body(const char* text, bool first = false) {
  font::textBox({&BODY, layout::TEXT_W, (int16_t)(first ? layout::FIRST_TEXT_H : layout::TEXT_H), 2, font::Align::LEFT}, text,
    layout::TEXT_X, first ? layout::FIRST_TEXT_Y : layout::TEXT_Y, INK);
}
void footer(const char* left, const char* middle, const char* right, const char* note = nullptr) {
  if (left) button(layout::PREV, left, SECONDARY);
  if (middle && !note) button(layout::MENU, middle, CREAM);
  if (middle && note) {
    const ui::Box& b = layout::MENU;
    paper(b.x * 3, b.y * 3, b.w * 3, b.h * 3);
    font::text(DETAIL, middle, 240 - font::textWidth(DETAIL, middle) / 2, b.y * 3 + 5, INK);
    font::text(DETAIL, note, 240 - font::textWidth(DETAIL, note) / 2, b.y * 3 + 33, MUTED);
  }
  if (right) button(layout::NEXT, right);
}
void readerFooter(const char* left, const char* middle, const char* right, int page, int count) {
  char line[24]; snprintf(line, sizeof line, "%d / %d", page + 1, count);
  footer(left, middle, right, line);
}
uint8_t changes(const Save& save) {
  return (save.eventDone ? save.outcome : 0) | (hasFact(save, (uint8_t)factId("flag.bridge_open")) ? 128 : 0);
}
void setting(uint8_t location, uint8_t state, bool preview = false) {
  const int bottom = preview ? 240 : 198;
  art::scene(location, 0, preview ? 48 : 27, 1, state);
  gfx565::rect(0, bottom, 480, 480 - bottom, NAVY);
}
void edge() {
  constexpr int OUTER = 238, INNER = 232;
  const uint16_t outside = rgb888_to_565(0x191827);
  const uint16_t rim = rgb888_to_565(0x30263F);
  for (int y = 0; y < 480; y++) {
    const int d = y - 240, a = OUTER * OUTER - d * d, b = INNER * INNER - d * d;
    if (a < 0) { gfx565::hline(0, y, 480, outside); continue; }
    const int outer = (int)sqrtf((float)a), inner = b < 0 ? 0 : (int)sqrtf((float)b);
    gfx565::hline(0, y, 240 - outer, outside);
    gfx565::hline(241 + outer, y, 239 - outer, outside);
    gfx565::hline(240 - outer, y, outer - inner + 1, rim);
    gfx565::hline(240 + inner, y, outer - inner + 1, rim);
  }
}
void diamond(int x, int y, uint16_t color) {
  for (int row = -5; row <= 5; row++) {
    const int half = 5 - abs(row);
    gfx565::hline(x - half, y + row, half * 2 + 1, color);
  }
}
} // namespace

void Game::render() {
  const uint32_t look = os::crc32(&save_, sizeof save_) ^ (title_ ? 0x77777777u : 0);
  frames_.render(nullptr, 0, look, [&] { draw(); });
}
void Game::draw() {
  gfx565::clear(art::NAVY);
  if (title_) { drawTitle(); edge(); return; }
  switch (save_.cursor.mode) {
    case MAP: drawMap(); break;
    case READ: drawRead(); break;
    case CHOICE: drawChoice(); break;
    case CONFIRM: drawConfirm(); break;
    case RESULT: drawResult(); break;
    case MENU: drawMenu(); break;
    case PARTY: case INVENTORY: case JOURNAL: drawInfo(); break;
    case PARTY_OVERVIEW: drawPartyOverview(); break;
    case INVENTORY_OVERVIEW: drawInventoryOverview(); break;
    case TRAVEL: drawTravel(); break;
    case REST: drawRest(); break;
    default: drawMap(); break;
  }
  edge();
}
void Game::drawTitle() {
  art::landscape(0, 0, 1);
  button(layout::HOME, "Exit", SECONDARY);
  button(layout::START, fresh_ ? "Begin" : "Continue");
}
void Game::drawMap() {
  art::map(0, 0, 1, changes(save_));
  char day[64]; snprintf(day, sizeof day, "DAY %u / WILLOWBROOK", save_.day);
  header(day);
  const char* names[] = {"Village", "Bridge", "Mill", "Woods", "Marsh", "Tower"};
  for (int i = 0; i < LOCATION_COUNT; i++)
    button(layout::PLACES[i], names[i], save_.location == i ? art::GOLD : art::CREAM);
  char deeds[24]; snprintf(deeds, sizeof deeds, "%u deed%s", save_.deeds, save_.deeds == 1 ? "" : "s");
  footer(nullptr, "Menu", nullptr, deeds);
}
void Game::drawRead() {
  const bool first = save_.cursor.page == 0;
  const Scene* scene = currentScene(save_);
  if (first) setting(scene ? (uint8_t)scene->location : save_.location, changes(save_));
  header(heading_);
  paper(48, first ? 198 : 114, 384, first ? 156 : 240);
  body(reader_.page(save_.cursor.page), first);
  readerFooter(first ? "Exit" : "Back", "Menu", save_.cursor.page + 1 == reader_.count() ? "Choose" : "Next", save_.cursor.page, reader_.count());
}
void Game::drawChoice() {
  header("WHAT DO YOU DO?");
  const Scene* scene = currentScene(save_); if (!scene) return;
  const ui::Box boxes[] = {layout::CARD0, layout::CARD1, layout::CARD2};
  for (int i = 0; i < 3; i++) {
    const int index = save_.cursor.choicePage * 3 + i;
    if (index >= scene->choiceCount) break;
    const Choice& choice = scene->choices[index]; const ui::Box& b = boxes[i];
    const bool ready = allowed(save_, choice);
    paper(b.x * 3, b.y * 3, b.w * 3, b.h * 3, ready && i == 0 ? art::GOLD : SECONDARY);
    font::textBox({&art::LABEL, (int16_t)(b.w * 3 - 2 * layout::CARD_PAD), 26, 0, font::Align::LEFT}, choice.caption,
      b.x * 3 + layout::CARD_PAD, b.y * 3 + layout::CAPTION_Y, art::INK);
    const char* detail = ready ? choice.detail : "Needs help - tap to learn why";
    font::textBox({&art::DETAIL, (int16_t)(b.w * 3 - 2 * layout::CARD_PAD), 48, 0, font::Align::LEFT}, detail,
      b.x * 3 + layout::CARD_PAD, b.y * 3 + layout::DETAIL_Y, art::INK);
  }
  footer("Menu", "Reread", scene->choiceCount > 3 ? "More" : nullptr);
}
void Game::drawConfirm() {
  header(heading_); paper(48, 114, 384, 240); body(reader_.page(save_.cursor.page));
  const Scene* scene = currentScene(save_);
  const bool ready = scene && save_.cursor.selectedChoice < scene->choiceCount && allowed(save_, scene->choices[save_.cursor.selectedChoice]);
  readerFooter("Back", "Reread", ready ? (save_.cursor.page + 1 < reader_.count() ? "Next" : "Decide") : nullptr, save_.cursor.page, reader_.count());
}
void Game::drawResult() {
  const bool first = save_.cursor.page == 0;
  if (first) setting(save_.location, changes(save_));
  header(heading_); paper(48, first ? 198 : 114, 384, first ? 156 : 240);
  body(reader_.page(save_.cursor.page), first);
  readerFooter("Back", "Menu", save_.cursor.page + 1 < reader_.count() ? "Next" : "Onward", save_.cursor.page, reader_.count());
}
void Game::drawMenu() {
  header("ADVENTURE PACK");
  const ui::Box boxes[] = {layout::TOOL0, layout::TOOL1, layout::TOOL2, layout::TOOL3, layout::TOOL4, layout::TOOL5};
  const char* names[] = {"Party", "Items", "Journal", "Save & quit", "Rest", "Return"};
  for (int i = 0; i < 6; i++) button(boxes[i], names[i], i == 3 ? GOLD : art::CREAM);
  button(layout::MENU, "Back", SECONDARY);
}
void Game::drawInfo() {
  header(save_.cursor.mode == JOURNAL ? "YOUR JOURNAL" : heading_);
  paper(48, 114, 384, 240);
  if (save_.cursor.mode == JOURNAL) {
    font::textBox({&HEADING, 348, 56, 0, font::Align::CENTER}, heading_, 66, 126, INK);
    gfx565::hline(78, 186, 324, BORDER);
    font::textBox({&BODY, 348, layout::JOURNAL_TEXT_H, 2, font::Align::LEFT}, reader_.page(save_.cursor.page),
      66, layout::JOURNAL_TEXT_Y, INK);
  } else body(reader_.page(save_.cursor.page));
  readerFooter("Back", "Menu", "Next", save_.cursor.page, reader_.count());
}
void Game::drawPartyOverview() {
  header("YOUR PARTY");
  const ui::Box boxes[] = {layout::PARTY0, layout::PARTY1, layout::PARTY2};
  const char* names[] = {"Princess", "Lucas", "Dribble"};
  for (int i = 0; i < 3; i++) {
    const int x = boxes[i].x * 3 + boxes[i].w * 3 / 2;
    art::portrait((uint8_t)i, x - 40, 114, 1);
    font::text(DETAIL, names[i], x - font::textWidth(DETAIL, names[i]) / 2, 232, PARCHMENT);
  }
  const char* namesSkill[] = {"Courage", "Ingenuity", "Insight"};
  const uint16_t colors[] = {GOLD, rgb888_to_565(0x71BADB), rgb888_to_565(0xCC6EA1)};
  for (int i = 0; i < 3; i++) {
    font::text(DETAIL, namesSkill[i], 96, 271 + i * 27, PARCHMENT);
    for (int pip = 0; pip < 6; pip++) diamond(260 + pip * 24, 284 + i * 27, pip < save_.skills[i] ? colors[i] : MUTED);
  }
  footer("Back", "Menu", nullptr);
}
void Game::drawInventoryOverview() {
  header("INVENTORY");
  const ui::Box boxes[] = {layout::SLOT0, layout::SLOT1, layout::SLOT2, layout::SLOT3, layout::SLOT4, layout::SLOT5};
  const char* shortNames[] = {"Empty", "Bread", "Rope", "Timber", "Thyme", "Flour", "Map"};
  for (int i = 0; i < 6; i++) {
    const ui::Box& b = boxes[i]; const ItemSlot& slot = save_.inventory[i];
    paper(b.x * 3, b.y * 3, b.w * 3, b.h * 3, NAVY);
    if (save_.cursor.infoPage == i) gfx565::frame(b.x * 3 + 2, b.y * 3 + 2, b.w * 3 - 4, b.h * 3 - 4, GOLD);
    if (slot.id) art::item(slot.id, b.x * 3 + 27, b.y * 3 + 14, 1);
    else {
      gfx565::rect(b.x * 3 + 49, b.y * 3 + 23, 3, 27, BORDER);
      gfx565::rect(b.x * 3 + 37, b.y * 3 + 35, 27, 3, BORDER);
    }
    const char* name = slot.id < sizeof shortNames / sizeof *shortNames ? shortNames[slot.id] : itemName(slot.id);
    font::textBox({&DETAIL, 94, 24, 0, font::Align::CENTER}, name, b.x * 3 + 4, b.y * 3 + 71, PARCHMENT);
    if (slot.quantity > 1) {
      char number[8]; snprintf(number, sizeof number, "%u", slot.quantity);
      font::text(DETAIL, number, b.x * 3 + 76, b.y * 3 + 6, PARCHMENT);
    }
  }
  footer("Back", "Menu", nullptr);
}
void Game::drawTravel() {
  const Location location = (Location)save_.cursor.infoPage;
  setting((uint8_t)location, changes(save_), true);
  header(locationName(location));
  paper(48, 240, 384, 114);
  const Scene* next = selectScene(save_, location);
  label(next ? next->title : "A quiet visit", 250, 52);
  font::textBox({&DETAIL, 348, 48, 0, font::Align::CENTER}, save_.location == location ? "Look around here. No day passes." : "Travel takes one world day.\nYour story waits when you stop.", 66, 303, INK);
  footer("Back", "Menu", "Go");
}
void Game::drawRest() {
  const bool ready = save_.returnCursor.mode == MAP;
  art::camp(0, 48, 1);
  gfx565::rect(0, 240, 480, 240, NAVY);
  header(ready ? "MAKE CAMP" : "YOUR STORY WAITS");
  paper(48, 240, 384, 114);
  font::textBox({&DETAIL, 348, 96, 0, font::Align::CENTER}, ready ? "Rest advances one world day.\nYour actions move the world.\nYou can stop at any time."
    : "Rest is available on the map.\nFinish your scene first.\nSave and stop on any page.", 66, 251, INK);
  footer("Back", "Menu", ready ? "Rest" : nullptr);
}
} // namespace kingdom
