// White-box proof; the ordinary-input emulator playthrough is separate.
#include <algorithm>
#include "host/check.h"
#include <stdio.h>
#include <string.h>
#include <vector>
#include "games/living-kingdom/game.h"
#include "games/living-kingdom/layout.h"
#include "gfx565.h"

using namespace kingdom;
namespace {
constexpr uint32_t NOW = 1790000000;
int layoutErrors = 0;
const ui::Box CARDS[] = {layout::CARD0, layout::CARD1, layout::CARD2};
const ui::Box PARTY_TILES[] = {layout::PARTY0, layout::PARTY1, layout::PARTY2};
const ui::Box ITEM_TILES[] = {layout::SLOT0, layout::SLOT1, layout::SLOT2, layout::SLOT3, layout::SLOT4, layout::SLOT5};
const ui::Box TOOLS[] = {layout::TOOL0, layout::TOOL1, layout::TOOL2, layout::TOOL3, layout::TOOL4, layout::TOOL5};
struct Player {
  Game game;
  Profile profiles[2] = {{0, "Reader", 0, 9, 1}, {1, "Other", 1, 8, 1}};
  SaveSlot slots[2] = {};
  InputTracker input;
  uint32_t ms = 0;
  void step(bool down = false, int x = 0, int y = 0, uint32_t delta = 20) {
    ms += delta;
    game.update(NOW, ms, input.step(down, x, y, ms));
  }
  void tap(const ui::Box& box, uint32_t wait = 500) {
    const int x = box.x + box.w / 2, y = box.y + box.h / 2;
    step(false, 0, 0, wait); step(true, x, y); step(false, x, y, 40);
  }
  void enter(Save* saved = nullptr, int profile = 0) {
    if (saved) { sealSave(*saved); slots[profile] = {saved, sizeof *saved}; }
    game.enter({&profiles[profile], profiles, slots, 2, NOW, ms, true});
  }
  void start(Save* saved = nullptr) { enter(saved); tap(layout::START); }
  Save snapshot() {
    const void* data = nullptr; size_t length = 0;
    game.leave(); assert(game.takeSave(&data, &length, true));
    Save saved{}; assert(loadSave(saved, data, length, game.save().profile)); return saved;
  }
  void toChoice() {
    for (int n = 0; game.save().cursor.mode == READ && n < Reader::MAX_PAGES; n++) tap(layout::NEXT);
    assert(game.save().cursor.mode == CHOICE);
  }
};
Save fixture(uint8_t mode, uint16_t scene = 1) {
  Save saved; newCampaign(saved, 12345, 0);
  saved.cursor = {}; saved.cursor.mode = mode; saved.cursor.scene = scene;
  saved.returnCursor = saved.cursor; saved.returnCursor.mode = READ;
  return saved;
}
void cursorEqual(const UiCursor& a, const UiCursor& b) { assert(!memcmp(&a, &b, sizeof a)); }
void worldEqual(Save a, Save b) {
  a.cursor = b.cursor = {}; a.returnCursor = b.returnCursor = {};
  a.crc = b.crc = 0; assert(!memcmp(&a, &b, sizeof a));
}
void rendered(Game& game) {
  static std::vector<uint16_t> buffers[4] = {std::vector<uint16_t>(gfx565::W * gfx565::H),
    std::vector<uint16_t>(gfx565::W * gfx565::H), std::vector<uint16_t>(gfx565::W * gfx565::H),
    std::vector<uint16_t>(gfx565::W * gfx565::H)};
  static int pair = 0; pair ^= 2;
  auto& a = buffers[pair]; auto& b = buffers[pair + 1];
  std::fill(a.begin(), a.end(), 0xf81f); std::fill(b.begin(), b.end(), 0x07e0);
  const bool logging = gfx::textLogEnabled; gfx::textLogEnabled = false;
  gfx565::target(a.data()); game.render(); gfx565::target(b.data()); game.render(); assert(a == b);
  gfx565::target(a.data()); game.render(); assert(a == b);
  gfx565::target(b.data()); game.render(); assert(a == b);
  gfx::textLogEnabled = logging;
  if (logging) game.render();
}
void textAvoidsOtherControl(const gfx::TextBox& text, const UiAudit::Rect& target, const char* screen) {
  if (text.x >= target.x + target.w || text.x + text.w <= target.x
    || text.y >= target.y + target.h || text.y + text.h <= target.y) return;
  if (text.x >= target.x && text.x + text.w <= target.x + target.w
    && text.y >= target.y && text.y + text.h <= target.y + target.h) return;
  fprintf(stderr, "%s text=(%d,%d,%d,%d) intersects target=(%d,%d,%d,%d)\n", screen, text.x, text.y, text.w, text.h, target.x, target.y, target.w, target.h);
  layoutErrors++;
}
void textInsideGlass(const gfx::TextBox& box, const char* screen) {
  for (int x : {int(box.x), box.x + box.w}) for (int y : {int(box.y), box.y + box.h}) {
    if (!gfx::inCircle(x, y, 1)) {
      fprintf(stderr, "%s clipped text=(%d,%d,%d,%d) corner=(%d,%d)\n", screen, box.x, box.y, box.w, box.h, x, y);
      layoutErrors++;
    }
  }
}
void audit(Game& game) {
  UiAudit::enabled = true; UiAudit::reset(); gfx::textLogEnabled = true; gfx::textLogReset();
  game.update(NOW, 100000, {}); rendered(game);
  for (int n = 0; n < UiAudit::count; n++) {
    const auto& b = UiAudit::regions[n]; assert(b.w >= 24 && b.h >= 22);
    for (int x : {int(b.x), b.x + b.w - 1}) for (int y : {int(b.y), b.y + b.h - 1}) assert(gfx::inCircle(x, y));
  }
  assert(UiAudit::count > 0 && gfx::textLogCount > 0);
  for (int n = 0; n < gfx::textLogCount; n++) {
    const auto& b = gfx::textLog[n];
    textInsideGlass(b, game.screenName());
    for (int i = 0; i < UiAudit::count; i++) textAvoidsOtherControl(b, UiAudit::regions[i], game.screenName());
  }
  UiAudit::enabled = false; gfx::textLogEnabled = false;
}
void readerLoop() {
  Player p; p.start(); assert(p.game.pages() >= 2);
  const Save original = p.game.save();
  p.tap(layout::NEXT); assert(p.game.save().cursor.page == 1);
  p.tap(layout::PREV); assert(p.game.save().cursor.page == 0);
  p.toChoice(); const UiCursor choices = p.game.save().cursor;
  p.tap(layout::MENU); assert(p.game.save().cursor.mode == READ && p.game.save().cursor.page == 0);
  p.toChoice(); cursorEqual(p.game.save().cursor, choices); worldEqual(p.game.save(), original);
  const Scene* scene = currentScene(p.game.save()); assert(scene && allowed(p.game.save(), scene->choices[0]));
  const uint16_t decision = scene->choices[0].id;
  p.tap(CARDS[0]);
  for (int n = 0; p.game.save().cursor.mode == CONFIRM && n < Reader::MAX_PAGES; n++) p.tap(layout::NEXT);
  assert(p.game.save().cursor.mode == RESULT && hasDecision(p.game.save(), decision));
  assert(p.game.save().journalCount == original.journalCount + 1);
  const Save committed = p.game.save();
  p.tap(layout::NEXT, 100); worldEqual(p.game.save(), committed);
  Save saved = p.snapshot(); Player resumed; resumed.start(&saved);
  cursorEqual(resumed.game.save().cursor, committed.cursor); worldEqual(resumed.game.save(), committed);
  assert(!commit(saved, 0)); worldEqual(saved, committed);
}
void choiceRereadPages() {
  const Content& c = content(); const Scene* scene = nullptr;
  for (int i = 0; i < c.sceneCount; i++) if (c.scenes[i].choiceCount > 3) { scene = &c.scenes[i]; break; }
  assert(scene); Save saved = fixture(CHOICE, scene->id); Player p; p.start(&saved);
  p.tap(layout::NEXT); assert(p.game.save().cursor.choicePage == 1);
  p.tap(CARDS[0]); assert(p.game.save().cursor.selectedChoice == 3);
  if (p.game.save().cursor.mode == CONFIRM) {
    p.tap(layout::PREV); assert(p.game.save().cursor.mode == CHOICE && p.game.save().cursor.choicePage == 1);
  }
  saved = fixture(CHOICE, scene->id); saved.cursor.choicePage = 1; saved.cursor.selectedChoice = 3;
  p.start(&saved); const UiCursor before = p.game.save().cursor;
  for (int tool = 0; tool < 2; tool++) {
    p.tap(layout::PREV); p.tap(TOOLS[tool]); p.tap(tool ? ITEM_TILES[1] : PARTY_TILES[1]);
    p.tap(layout::MENU); p.tap(layout::MENU); cursorEqual(p.game.save().cursor, before);
  }
  p.tap(layout::MENU); p.toChoice(); cursorEqual(p.game.save().cursor, before);
}
void menuPreservesCursor() {
  Player p; p.start(); p.tap(layout::NEXT); const UiCursor original = p.game.save().cursor;
  const uint8_t expected[] = {PARTY_OVERVIEW, INVENTORY_OVERVIEW, JOURNAL};
  for (int tool = 0; tool < 3; tool++) {
    p.tap(layout::MENU); assert(p.game.save().cursor.mode == MENU);
    p.tap(TOOLS[tool]); assert(p.game.save().cursor.mode == expected[tool]);
    if (tool < 2) p.tap(tool ? ITEM_TILES[1] : PARTY_TILES[1]);
    p.tap(layout::NEXT); p.tap(layout::MENU); assert(p.game.save().cursor.mode == MENU);
    cursorEqual(p.game.save().returnCursor, original); p.tap(layout::MENU); cursorEqual(p.game.save().cursor, original);
  }
  p.tap(layout::MENU); p.tap(TOOLS[5]); cursorEqual(p.game.save().cursor, original);
  p.tap(layout::MENU); p.tap(layout::MENU); cursorEqual(p.game.save().cursor, original);
  p.tap(layout::MENU); p.tap(TOOLS[4]); assert(p.game.save().cursor.mode == REST);
  p.tap(layout::NEXT); assert(p.game.save().cursor.mode == REST);
  p.tap(layout::PREV); cursorEqual(p.game.save().cursor, original); p.tap(layout::MENU);
  p.tap(TOOLS[3]); assert(p.game.wantsHome() && !p.game.wantsHome()); cursorEqual(p.game.save().cursor, original);
}
void overviewNavigation() {
  for (int tool = 0; tool < 2; tool++) {
    Player p; p.start(); p.tap(layout::NEXT); const Save original = p.game.save();
    const uint8_t overview = tool ? INVENTORY_OVERVIEW : PARTY_OVERVIEW;
    const uint8_t detail = tool ? INVENTORY : PARTY;
    const ui::Box* tiles = tool ? ITEM_TILES : PARTY_TILES;
    p.tap(layout::MENU); p.tap(TOOLS[tool]);
    for (int i = 0; i < (tool ? 6 : 3); i++) {
      p.tap(tiles[i]); assert(p.game.save().cursor.mode == detail && p.game.save().cursor.infoPage == i);
      Save saved = p.snapshot(); p.start(&saved); cursorEqual(saved.cursor, p.game.save().cursor); worldEqual(original, p.game.save());
      if (p.game.pages() > 1) {
        p.tap(layout::NEXT); assert(p.game.save().cursor.page == 1);
        saved = p.snapshot(); p.start(&saved); cursorEqual(saved.cursor, p.game.save().cursor); worldEqual(original, p.game.save());
        p.tap(layout::PREV); assert(p.game.save().cursor.page == 0 && p.game.save().cursor.mode == detail);
      }
      p.tap(layout::PREV); assert(p.game.save().cursor.mode == overview && p.game.save().cursor.infoPage == i);
      saved = p.snapshot(); p.start(&saved); cursorEqual(saved.cursor, p.game.save().cursor); worldEqual(original, p.game.save());
    }
    p.tap(layout::PREV); cursorEqual(original.cursor, p.game.save().cursor); worldEqual(original, p.game.save());
    p.tap(layout::MENU); p.tap(TOOLS[tool]); p.tap(layout::MENU);
    assert(p.game.save().cursor.mode == MENU); cursorEqual(p.game.save().returnCursor, original.cursor);
    p.tap(TOOLS[3]); assert(p.game.wantsHome()); cursorEqual(original.cursor, p.game.save().cursor);
  }
}
void profilesAndCorruption() {
  Player p; p.start(); p.tap(layout::NEXT); Save saved = p.snapshot(); const UiCursor original = saved.cursor;
  p.enter(&saved); assert(!strcmp(p.game.screenName(), "lk_title")); p.tap(layout::START);
  cursorEqual(p.game.save().cursor, original);
  p.enter(nullptr, 1); assert(p.game.save().profile == 1 && p.game.save().cursor.page == 0);
  p.enter(nullptr, 0); p.tap(layout::START); cursorEqual(p.game.save().cursor, original);
  Save bad = saved; bad.seed ^= 1; p.slots[0] = {&bad, sizeof bad}; p.enter();
  assert(p.game.save().cursor.page == 0 && p.game.save().seed != bad.seed);
  p.slots[0] = {&saved, sizeof saved - 1}; p.enter(); assert(p.game.save().cursor.page == 0);
  p.slots[1] = {&saved, sizeof saved}; p.enter(nullptr, 1); assert(p.game.save().profile == 1);
}
void heldAndNoopInput() {
  Player p; p.start(); const Save before = p.game.save();
  p.tap({0, 0, 1, 1}); worldEqual(p.game.save(), before); cursorEqual(p.game.save().cursor, before.cursor);
  p.step(false, 0, 0, 500); p.step(true, layout::NEXT.x + 8, layout::NEXT.y + 8); p.step(true, layout::NEXT.x + 8, layout::NEXT.y + 8, 1000);
  cursorEqual(p.game.save().cursor, before.cursor); p.step(false, layout::NEXT.x + 8, layout::NEXT.y + 8);
  assert(p.game.save().cursor.page == 1);
  p.tap(layout::NEXT, 100); assert(p.game.save().cursor.page == 1);
  p.tap({68, 8, 24, 22}); assert(!p.game.wantsHome());
  p.tap(layout::MENU); p.tap(TOOLS[3]); assert(p.game.wantsHome() && !p.game.wantsHome());
  p.start(); p.tap(layout::PREV); assert(p.game.wantsHome());
  p.enter(); p.tap(layout::HOME); assert(p.game.wantsHome());
}
void travelAndRest() {
  Save saved = fixture(MAP); Player p; p.start(&saved); const uint16_t day = p.game.save().day;
  p.tap(layout::PLACES[VILLAGE]); assert(p.game.save().cursor.mode == TRAVEL);
  p.tap(layout::PREV); assert(p.game.save().cursor.mode == MAP);
  p.tap(layout::PLACES[BRIDGE]); p.tap(layout::MENU); assert(p.game.save().cursor.mode == MENU);
  p.tap(TOOLS[5]); p.tap(layout::MENU); p.tap(TOOLS[4]); assert(p.game.save().cursor.mode == REST);
  p.tap(layout::PREV); assert(p.game.save().cursor.mode == MAP && p.game.save().day == day);
  p.tap(layout::MENU); p.tap(TOOLS[4]); p.tap(layout::NEXT);
  assert(p.game.save().cursor.mode == MAP && p.game.save().day == day + 1);
  p.tap(layout::PLACES[BRIDGE]); p.tap(layout::NEXT);
  assert(p.game.save().location == BRIDGE && p.game.save().day == day + 2);
}
void informationEntries() {
  Save saved = fixture(JOURNAL); const Content& c = content();
  saved.facts = c.factCount == 64 ? ~uint64_t(0) : (uint64_t(1) << c.factCount) - 1;
  for (int i = 0; i < c.promiseCount; i++) { saved.promises[i] = uint8_t(OPEN + i % 3); saved.promiseDays[i] = 1; }
  saved.journal[0] = {c.scenes[0].choices[0].id, 1}; saved.journalCount = 1;
  if (c.eventCount) saved.journal[saved.journalCount++] = {0x8000, 2};
  for (int i = 0; i < c.questCount; i++) saved.quests[i] = uint8_t(i % 6);
  Player p; p.start(&saved); const UiCursor first = p.game.save().cursor; int inspected = 0;
  do {
    audit(p.game); p.tap(layout::NEXT); inspected++;
    assert(inspected < 512);
  } while (memcmp(&first, &p.game.save().cursor, sizeof first));
  int clues = 0;
  for (int i = 0; i < c.factCount; i++) clues += !strncmp(c.facts[i].key, "clue.", 5);
  assert(inspected >= c.questCount + clues + c.promiseCount + saved.journalCount);
  p.tap(layout::PREV); assert(p.game.save().cursor.infoPage > 0);
  p.tap(layout::NEXT); cursorEqual(first, p.game.save().cursor);
  for (int mode = PARTY; mode <= INVENTORY; mode++) {
    saved = fixture(uint8_t(mode));
    for (int i = 0; i < 6 && i < c.itemCount; i++) saved.inventory[i] = {uint8_t(i + 1), 1};
    p.start(&saved); const UiCursor begin = p.game.save().cursor; int pages = 0;
    do { audit(p.game); p.tap(layout::NEXT); assert(++pages < 128); }
    while (memcmp(&begin, &p.game.save().cursor, sizeof begin));
    p.tap(layout::PREV); assert(p.game.save().cursor.mode == (mode == PARTY ? PARTY_OVERVIEW : INVENTORY_OVERVIEW));
    p.tap(mode == PARTY ? PARTY_TILES[0] : ITEM_TILES[0]); cursorEqual(begin, p.game.save().cursor);
  }
}
void lockedChoice() {
  const Content& c = content(); bool tested = false;
  for (int n = 0; n < c.sceneCount && !tested; n++) {
    const Scene& scene = c.scenes[n]; Save saved = fixture(CHOICE, scene.id);
    for (int i = 0; i < scene.choiceCount && !tested; i++) {
      if (allowed(saved, scene.choices[i])) continue;
      saved.cursor.choicePage = uint8_t(i / 3); Player p; p.start(&saved); p.tap(CARDS[i % 3]);
      assert(p.game.save().cursor.mode == CONFIRM && p.game.save().cursor.selectedChoice == i); audit(p.game);
      const Save before = p.game.save(); p.tap(layout::NEXT); cursorEqual(before.cursor, p.game.save().cursor);
      worldEqual(before, p.game.save()); p.tap(layout::MENU); p.toChoice();
      assert(p.game.save().cursor.selectedChoice == i && p.game.save().cursor.choicePage == i / 3); tested = true;
    }
  }
  assert(tested);
}
void resultNavigation() {
  const Content& c = content(); bool multiPage = false;
  for (int n = 0; n < c.sceneCount && !multiPage; n++) {
    Save saved = fixture(RESULT, c.scenes[n].id); Player p; p.start(&saved);
    const UiCursor first = p.game.save().cursor; p.tap(layout::PREV); assert(p.game.save().cursor.mode == MENU);
    p.tap(layout::MENU); cursorEqual(first, p.game.save().cursor);
    p.tap(layout::MENU); p.tap(TOOLS[2]); p.tap(layout::MENU); p.tap(layout::MENU); cursorEqual(first, p.game.save().cursor);
    if (p.game.pages() > 1) {
      p.tap(layout::NEXT); assert(p.game.save().cursor.page == 1); p.tap(layout::PREV);
      cursorEqual(first, p.game.save().cursor); multiPage = true;
    }
    for (int i = 0; p.game.save().cursor.mode == RESULT && i < Reader::MAX_PAGES; i++) p.tap(layout::NEXT);
    assert(p.game.save().cursor.mode != RESULT);
  }
  assert(multiPage);
}
std::vector<uint16_t> preview(uint8_t location, uint8_t outcome, bool repaired = false) {
  Save saved = fixture(TRAVEL); saved.cursor.infoPage = location; saved.returnCursor.mode = MAP;
  saved.outcome = outcome; saved.eventDone = outcome ? 1 : 0;
  const int bridge = factId("flag.bridge_open"); assert(bridge >= 0);
  if (repaired) saved.facts |= uint64_t(1) << bridge;
  Player p; p.start(&saved); audit(p.game);
  return std::vector<uint16_t>(gfx565::fb, gfx565::fb + gfx565::W * gfx565::H);
}
void paintedRectangle(const std::vector<uint16_t>& a, const std::vector<uint16_t>& b, int width, int height) {
  assert(width > 0 && height > 0 && width <= gfx565::W && height <= gfx565::H);
  for (int y = 0; y < gfx565::H; y++) for (int x = 0; x < gfx565::W; x++) {
    const int i = y * gfx565::W + x;
    if (x < width && y < height) assert(a[i] == b[i]);
    else assert(a[i] == 0xf81f && b[i] == 0x07e0);
  }
}
std::vector<uint16_t> nativeBackdrop(uint8_t location, uint8_t changes = 0) {
  std::vector<uint16_t> a(gfx565::W * gfx565::H, 0xf81f), b(a.size(), 0x07e0);
  gfx565::target(a.data());
  if (location == LOCATION_COUNT) art::map(0, 0, 1, changes);
  else art::scene(location, 0, 0, 1, changes);
  gfx565::target(b.data());
  if (location == LOCATION_COUNT) art::map(0, 0, 1, changes);
  else art::scene(location, 0, 0, 1, changes);
  paintedRectangle(a, b, location == LOCATION_COUNT ? art::MAP_W : art::SCENE_W,
    location == LOCATION_COUNT ? art::MAP_H : art::SCENE_H);
  return a;
}
void worldArt() {
  for (uint8_t location : {uint8_t(MILL), uint8_t(MARSH)}) {
    const auto original = preview(location, 0), originalPanel = nativeBackdrop(location);
    std::vector<std::vector<uint16_t>> outcomes, panels;
    for (uint8_t outcome = 1; outcome <= 4; outcome++) {
      const auto changed = preview(location, outcome), panel = nativeBackdrop(location, outcome);
      assert(original != changed && originalPanel != panel);
      for (const auto& earlier : outcomes) assert(earlier != changed);
      for (const auto& earlier : panels) assert(earlier != panel);
      outcomes.push_back(changed); panels.push_back(panel);
    }
  }
  assert(preview(BRIDGE, 0) != preview(BRIDGE, 0, true));
  assert(preview(BRIDGE, 0) != preview(BRIDGE, 4));
}
void originalBackgrounds() {
  for (uint8_t location : {uint8_t(VILLAGE), uint8_t(WOODS), uint8_t(TOWER), uint8_t(LOCATION_COUNT)}) {
    const auto original = nativeBackdrop(location);
    for (uint8_t outcome = 0; outcome <= 4; outcome++) {
      assert(nativeBackdrop(location, outcome) == original);
      assert(nativeBackdrop(location, uint8_t(outcome | 128)) == original);
    }
  }
  const auto bridge = nativeBackdrop(BRIDGE), repaired = nativeBackdrop(BRIDGE, 128); assert(bridge != repaired);
  for (uint8_t outcome = 1; outcome <= 4; outcome++) {
    if (outcome == 4) assert(nativeBackdrop(BRIDGE, outcome) != bridge);
    else assert(nativeBackdrop(BRIDGE, outcome) == bridge);
    assert(nativeBackdrop(BRIDGE, uint8_t(outcome | 128)) == repaired);
  }
}
void artBoundaries() {
  std::vector<uint16_t> pixels(gfx565::W * gfx565::H, art::PARCHMENT); gfx565::target(pixels.data());
  art::scene(0, 0, 0, 1); const auto village = pixels;
  std::fill(pixels.begin(), pixels.end(), art::PARCHMENT); art::scene(255, 0, 0, 1); assert(pixels == village);
  art::scene(MARSH, 0, 0, 0); assert(pixels == village);
  std::fill(pixels.begin(), pixels.end(), art::PARCHMENT); art::portrait(0, 100, 100, 1); const auto princess = pixels;
  std::fill(pixels.begin(), pixels.end(), art::PARCHMENT); art::portrait(255, 100, 100, 1); assert(pixels == princess);
  art::portrait(1, 100, 100, 0); art::landscape(100, 100, 0); assert(pixels == princess);
  std::fill(pixels.begin(), pixels.end(), 0xf81f); art::landscape(0, 0, 1); const auto title = pixels;
  std::fill(pixels.begin(), pixels.end(), 0x07e0); art::landscape(0, 0, 1);
  paintedRectangle(title, pixels, art::LANDSCAPE_W, art::LANDSCAPE_H);
}
void itemArt() {
  std::vector<uint16_t> pixels(gfx565::W * gfx565::H, art::PARCHMENT); gfx565::target(pixels.data());
  const auto blank = pixels;
  for (uint8_t id = 1; id <= 6; id++) {
    std::fill(pixels.begin(), pixels.end(), art::PARCHMENT); art::item(id, 100, 100, 1);
    assert(pixels != blank); const auto item = pixels;
    art::item(0, 100, 100, 1); art::item(255, 100, 100, 1); art::item(id, 100, 100, 0); assert(pixels == item);
  }
  std::fill(pixels.begin(), pixels.end(), art::PARCHMENT); art::scene(VILLAGE, 0, 0, 1, 128); const auto repaired = pixels;
  art::scene(VILLAGE, 0, 0, 1, 255); assert(pixels == repaired);
  std::fill(pixels.begin(), pixels.end(), art::PARCHMENT); art::map(); assert(pixels != blank);
}
void exitThroughMenu(Player& p) {
  const uint8_t current = p.game.save().cursor.mode;
  if (current == CONFIRM) { p.tap(layout::PREV); p.tap(layout::PREV); }
  else if (current == CHOICE) p.tap(layout::PREV);
  else if (current != MENU) p.tap(layout::MENU);
  assert(p.game.save().cursor.mode == MENU); const UiCursor expected = p.game.save().returnCursor;
  p.tap(TOOLS[3]); assert(p.game.wantsHome() && !p.game.wantsHome()); cursorEqual(p.game.save().cursor, expected);
}
void modeRenderings() {
  for (uint8_t mode = MAP; mode <= INVENTORY_OVERVIEW; mode++) {
    Save saved = fixture(mode);
    if (mode == INVENTORY_OVERVIEW) for (int i = 0; i < 6; i++) saved.inventory[i] = {uint8_t(i + 1), uint8_t(i == 5 ? 99 : i + 1)};
    Player p; p.start(&saved); audit(p.game); exitThroughMenu(p);
  }
  Player title; title.enter(); audit(title.game); title.tap(layout::HOME); assert(title.game.wantsHome());
  assert(title.game.surface() == SURFACE_RGB565 && title.game.tint() == TINT_DAY);
  assert(!title.game.soundOn(0) && !title.game.asleep());
  assert(!strcmp(title.game.name(), "Kingdom") && !strcmp(title.game.store(), "kingdom"));
  assert(title.game.icon().px); title.game.debugCmd("ignored"); title.game.debugPrint();
}
void overviewSaveValidation() {
  const uint8_t modes[] = {PARTY, INVENTORY, PARTY_OVERVIEW, INVENTORY_OVERVIEW};
  for (uint8_t current : modes) {
    Save saved = fixture(current), restored{};
    const uint8_t count = current == PARTY || current == PARTY_OVERVIEW ? 3 : 6;
    saved.cursor.infoPage = count - 1; sealSave(saved);
    assert(loadSave(restored, &saved, sizeof saved, 0)); cursorEqual(restored.cursor, saved.cursor);
    saved.cursor.infoPage = count; sealSave(saved); assert(!loadSave(restored, &saved, sizeof saved, 0));
  }
  Save invalid = fixture(INVENTORY_OVERVIEW + 1), restored{}; sealSave(invalid);
  assert(!loadSave(restored, &invalid, sizeof invalid, 0));
}
void saveCadence() {
  Player p; p.enter(); assert(p.game.saveUrgent()); const void* data = nullptr; size_t len = 0;
  assert(!p.game.takeSave(&data, &len, false)); assert(p.game.saveUrgent());
  assert(p.game.takeSave(&data, &len, true)); assert(len == sizeof(Save) && !p.game.saveUrgent());
  assert(!p.game.takeSave(&data, &len, true)); p.game.leave(); assert(p.game.saveUrgent());
}
} // namespace
int main() {
  static_assert(art::SCENE_W == 480 && art::SCENE_H == 240, "scene viewport keeps the approved physical dimensions");
  static_assert(art::MAP_W == 480 && art::MAP_H == 480, "map keeps the complete physical viewport");
  static_assert(art::LANDSCAPE_W == 480 && art::LANDSCAPE_H == 350, "title keeps the approved physical viewport");
  static_assert(PARTY == 6 && INVENTORY == 7 && REST == 11, "existing saved mode meanings must remain stable");
  static_assert(PARTY_OVERVIEW == 12 && INVENTORY_OVERVIEW == 13, "new saved modes append after existing meanings");
  readerLoop(); menuPreservesCursor(); overviewNavigation(); profilesAndCorruption();
  heldAndNoopInput(); travelAndRest(); overviewSaveValidation(); modeRenderings(); saveCadence();
  informationEntries(); lockedChoice(); resultNavigation(); choiceRereadPages(); worldArt(); originalBackgrounds(); artBoundaries(); itemArt();
  assert(!layoutErrors);
  puts("kingdom UI: reader, reread, single commitment, exact resume, profiles, input and circular rendering passed");
}
