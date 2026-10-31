#include "host/check.h"
#include <stdio.h>
#include <string.h>
#include "games/living-kingdom/engine.h"

using namespace kingdom;
static int checks = 0;

static void openingIsAtomic() {
  Save s;
  newCampaign(s, 42, 2);
  CHECK(s.cursor.mode == READ && s.cursor.scene == 1);
  const Scene* scene = currentScene(s);
  CHECK(scene && scene->paragraphCount > 1 && scene->choiceCount > 0);
  Save before = s;
  for (int page = 0; page < 12; ++page) s.cursor.page = page;
  CHECK(s.facts == before.facts && s.day == before.day);
  CHECK(!commit(s, 0));  // Paging cannot accidentally spend a decision.
  s.cursor.mode = CHOICE;
  CHECK(commit(s, 0));
  CHECK(s.cursor.mode == RESULT && s.journalCount == 1);
  Save committed = s;
  CHECK(!commit(s, 0));
  CHECK(memcmp(&s, &committed, sizeof(s)) == 0);
  finishResult(s);
  CHECK(s.cursor.mode == READ || s.cursor.mode == MAP);
}

static void exactSnapshot() {
  Save s, loaded;
  newCampaign(s, 77, 1);
  s.cursor.page = 3;
  s.returnCursor = s.cursor;
  s.cursor.mode = JOURNAL;
  s.cursor.infoPage = 2;
  sealSave(s);
  CHECK(loadSave(loaded, &s, sizeof(s), 1));
  CHECK(memcmp(&s, &loaded, sizeof(s)) == 0);
  CHECK(!loadSave(loaded, &s, sizeof(s), 2));
  s.seed ^= 1;
  CHECK(!loadSave(loaded, &s, sizeof(s), 1));
}

static Condition rule(Kind kind, uint16_t id, int16_t value, Compare op = Compare::Eq) {
  return {kind, id, value, op, nullptr, 0, "A known requirement."};
}
static bool checkRule(const Save& s, Condition condition) { return meets(s, {&condition, 1}); }
static bool change(Save& s, Kind kind, uint16_t id, int16_t value) {
  const Effect effect = {kind, id, value};
  return applyEffects(s, {&effect, 1});
}
static void declarativeRules() {
  Save s;
  newCampaign(s, 19, 0);
  CHECK(checkRule(s, rule(Kind::Skill, 0, 3)));
  CHECK(checkRule(s, rule(Kind::Skill, 1, 1, Compare::Ge)));
  CHECK(checkRule(s, rule(Kind::Skill, 2, 3, Compare::Le)));
  CHECK(checkRule(s, rule(Kind::Outcome, 0, 1, Compare::Ne)));
  CHECK(!checkRule(s, rule(Kind::Fact, 0, 1)));
  CHECK(change(s, Kind::Fact, 0, 1));
  CHECK(hasFact(s, 0) && checkRule(s, rule(Kind::Fact, 0, 1)));
  CHECK(change(s, Kind::Fact, 0, 0) && !hasFact(s, 0));
  CHECK(change(s, Kind::Quest, 2, DEFERRED));
  CHECK(checkRule(s, rule(Kind::Quest, 2, DEFERRED)));
  CHECK(change(s, Kind::Relation, 0, 3));
  CHECK(checkRule(s, rule(Kind::Relation, 0, 2, Compare::Ge)));
  CHECK(change(s, Kind::Promise, 0, OPEN) && s.promiseDays[0] == 1);
  CHECK(checkRule(s, rule(Kind::Promise, 0, OPEN)));
  CHECK(change(s, Kind::Day, 0, 3) && s.day == 4);
  CHECK(checkRule(s, rule(Kind::Day, 0, 4)));
  CHECK(checkRule(s, rule(Kind::Variant, 0, s.variants[0])));
  CHECK(change(s, Kind::Promise, 0, KEPT) && s.promiseDays[0] == 1);
  CHECK(change(s, Kind::Promise, 1, KEPT) && s.promises[1] == NO_PROMISE);
  CHECK(change(s, Kind::Relation, 0, 20) && s.relations[0] == 5);
  CHECK(change(s, Kind::Relation, 0, -30) && s.relations[0] == -5);
  CHECK(change(s, Kind::Skill, 0, 20) && s.skills[0] == 6);
  CHECK(change(s, Kind::Skill, 0, -30) && s.skills[0] == 0);
  CHECK(change(s, Kind::Outcome, 0, 3) && s.outcome == 3);
  CHECK(change(s, Kind::Location, TOWER, 0) && s.location == TOWER);
  CHECK(change(s, Kind::Deed, 0, 1) && s.deeds == 1);
}

static void groupedRules() {
  Save s;
  newCampaign(s, 9, 0);
  Condition children[] = {rule(Kind::Day, 0, 1), rule(Kind::Outcome, 0, 2)};
  Condition group = {Kind::Any, 0, 0, Compare::Eq, children, 2, "One clue needed."};
  CHECK(checkRule(s, group));
  group.kind = Kind::All;
  CHECK(!checkRule(s, group));
  children[1].value = 0;
  CHECK(checkRule(s, group));
  group.count = 0;
  CHECK(checkRule(s, group));
  group.kind = Kind::Any;
  CHECK(!checkRule(s, group));
  Choice choice = {99, "test", "Test", "", "", "", {&children[0], 1}, {nullptr, 0}, 0, false};
  CHECK(allowed(s, choice) && !strcmp(requirementReason(s, choice), "Ready."));
  children[0].value = 2;
  CHECK(!allowed(s, choice) && !strcmp(requirementReason(s, choice), "A known requirement."));
}

static void atomicInventory() {
  Save s;
  newCampaign(s, 2, 0);
  CHECK(change(s, Kind::Item, 1, 2) && itemCount(s, 1) == 2);
  CHECK(checkRule(s, rule(Kind::Item, 1, 2, Compare::Ge)));
  const Effect costs[] = {{Kind::Fact, 0, 1}, {Kind::Item, 1, -3}};
  Save before = s;
  CHECK(!applyEffects(s, {costs, 2}) && !memcmp(&s, &before, sizeof(s)));
  CHECK(change(s, Kind::Item, 1, -2) && itemCount(s, 1) == 0);
  for (const ItemSlot& slot : s.inventory) CHECK(!slot.id && !slot.quantity);
  CHECK(change(s, Kind::Item, 1, 99));
  CHECK(!change(s, Kind::Item, 1, 1) && itemCount(s, 1) == 99);
  CHECK(change(s, Kind::Item, 1, 0));
  CHECK(!change(s, Kind::Item, 0, 1));
  CHECK(!change(s, Kind::Item, 255, 1));
  CHECK(!change(s, Kind::Item, 2, -1));
  CHECK(!change(s, Kind::Item, 2, 100));
  for (ItemSlot& slot : s.inventory) slot = {1, 1};  // capacity probe for future seventh item type
  CHECK(!change(s, Kind::Item, 2, 1));
  const Effect noRoom = {Kind::Item, 2, 1};
  Choice choice = {99, "test", "Test", "", "", "", {nullptr, 0}, {&noRoom, 1}, 0, false};
  CHECK(!allowed(s, choice));
  CHECK(strstr(requirementReason(s, choice), "inventory"));
}

static void rejectedEffects() {
  Save s;
  newCampaign(s, 1, 0);
  CHECK(!change(s, Kind::Quest, QUEST_COUNT, ACTIVE));
  CHECK(!change(s, Kind::Quest, 0, 6));
  CHECK(!change(s, Kind::Promise, MAX_PROMISES, OPEN));
  CHECK(!change(s, Kind::Promise, 0, -1));
  CHECK(!change(s, Kind::Skill, SKILL_COUNT, 1));
  CHECK(!change(s, Kind::Relation, MAX_NPCS, 1));
  CHECK(!change(s, Kind::Fact, MAX_FACTS, 1));
  CHECK(!change(s, Kind::Outcome, 0, 5));
  CHECK(!change(s, Kind::Location, LOCATION_COUNT, 0));
  CHECK(!change(s, Kind::Schedule, MAX_EVENTS, 2));
  CHECK(!change(s, Kind::Schedule, 0, 0));
  CHECK(!change(s, Kind::Day, 0, -1));
  CHECK(!change(s, Kind::Decision, 0, 1));
  CHECK(!checkRule(s, rule(Kind::Skill, SKILL_COUNT, 1)));
  CHECK(!checkRule(s, rule(Kind::Quest, QUEST_COUNT, 1)));
  CHECK(!checkRule(s, rule(Kind::Relation, MAX_NPCS, 1)));
  CHECK(!checkRule(s, rule(Kind::Promise, MAX_PROMISES, 1)));
  CHECK(!checkRule(s, rule(Kind::Variant, 4, 1)));
  CHECK(!checkRule(s, rule(Kind::Event, MAX_EVENTS, 1)));
  CHECK(!checkRule(s, rule(Kind::Schedule, 0, 1)));
}

static void scheduledOnce() {
  Save s;
  newCampaign(s, 51, 0);
  for (int i = 0; i < content().eventCount; ++i) CHECK(change(s, Kind::Schedule, i, 2));
  CHECK(change(s, Kind::Schedule, 0, 7) && s.eventDue[0] == 3);
  advanceDays(s, 1);
  CHECK(!s.eventDone && !s.journalCount);
  advanceDays(s, 1);
  CHECK(s.eventDone == (1U << content().eventCount) - 1);
  CHECK(s.journalCount == content().eventCount);
  uint64_t facts = s.facts;
  advanceDays(s, 100);
  CHECK(s.facts == facts && s.journalCount == content().eventCount);
  CHECK(change(s, Kind::Schedule, 0, 2) && !s.eventDue[0]);
  CHECK(checkRule(s, rule(Kind::Event, 0, 1)));
  CHECK(strstr(journalText(s.journal[0]), " "));
}

static void corruptSnapshots() {
  Save s, loaded;
  newCampaign(s, 98, 1);
  CHECK(!loadSave(loaded, nullptr, sizeof(s), 1));
  CHECK(!loadSave(loaded, &s, sizeof(s) - 1, 1));
  for (size_t byte = 0; byte < sizeof(s); ++byte) {
    Save corrupt = s;
    reinterpret_cast<uint8_t*>(&corrupt)[byte] ^= 1;
    CHECK(!loadSave(loaded, &corrupt, sizeof(corrupt), 1));
  }
  s.version = 99;
  CHECK(!loadSave(loaded, &s, sizeof(s), 1));
  newCampaign(s, 98, 1);
  s.cursor.mode = RESULT;
  s.cursor.selectedChoice = 255;
  sealSave(s);
  CHECK(!loadSave(loaded, &s, sizeof(s), 1));
  s.cursor.scene = 127;
  sealSave(s);
  CHECK(!loadSave(loaded, &s, sizeof(s), 1));
}

static void invalidSnapshot(Save save) {
  Save loaded;
  sealSave(save);
  CHECK(!loadSave(loaded, &save, sizeof(save), save.profile));
}
static void stateBounds() {
  Save s;
  newCampaign(s, 6, 0);
  s.cursor.mode = 255; invalidSnapshot(s);
  s.cursor.mode = READ; s.cursor.scene = 0; invalidSnapshot(s);
  newCampaign(s, 6, 0);
  s.location = LOCATION_COUNT; invalidSnapshot(s);
  s.location = 0; s.day = 0; invalidSnapshot(s);
  s.day = 1; s.outcome = 5; invalidSnapshot(s);
  s.outcome = 0; s.journalCount = 97; invalidSnapshot(s);
  s.journalCount = 0; s.quests[0] = 6; invalidSnapshot(s);
  s.quests[0] = ACTIVE; s.promises[0] = 4; invalidSnapshot(s);
  s.promises[0] = 0; s.inventory[0] = {255, 1}; invalidSnapshot(s);
  s.inventory[0] = {1, 100}; invalidSnapshot(s);
  s.inventory[0] = {1, 0}; invalidSnapshot(s);
  s.inventory[0] = {0, 1}; invalidSnapshot(s);
  s.inventory[0] = {1, 1}; s.inventory[1] = {1, 1}; invalidSnapshot(s);
  JournalEntry entry = {0xffff, 1};
  CHECK(strstr(journalText(entry), "earlier"));
  entry.id = 511;
  CHECK(strstr(journalText(entry), "earlier"));
}

static void invalidInfoCursors() {
  Save s;
  newCampaign(s, 4, 0);
  s.cursor.mode = PARTY; s.cursor.infoPage = 3; invalidSnapshot(s);
  s.cursor.mode = INVENTORY; s.cursor.infoPage = 6; invalidSnapshot(s);
  s.cursor.mode = TRAVEL; s.cursor.infoPage = LOCATION_COUNT; invalidSnapshot(s);
  s.cursor.mode = CHOICE; s.cursor.choicePage = 255; invalidSnapshot(s);
  s.cursor.mode = READ; invalidSnapshot(s);
  s.cursor.mode = CONFIRM; invalidSnapshot(s);
  s.cursor.mode = RESULT; invalidSnapshot(s);
}
static void invalidProgress() {
  Save s;
  newCampaign(s, 4, 0);
  s.skills[0] = 7; invalidSnapshot(s);
  s.skills[0] = 3; s.relations[0] = 6; invalidSnapshot(s);
  s.relations[0] = -6; invalidSnapshot(s);
  s.relations[0] = 0; s.variants[0] = 2; invalidSnapshot(s);
  s.variants[0] = 0; s.variants[3] = 3; invalidSnapshot(s);
  s.variants[3] = 0; s.deeds = 9; invalidSnapshot(s);
  s.deeds = 1; s.reserved[31] = 1; invalidSnapshot(s);
}

static void eligibilityAndNavigation() {
  Save s;
  newCampaign(s, 16, 0);
  Save before = s;
  CHECK(!visit(s, TOWER));  // must finish the pending scene first
  rest(s);
  finishResult(s);
  CHECK(!memcmp(&s, &before, sizeof(s)));
  CHECK(!commit(s, 255));
  s.cursor.mode = CHOICE;
  CHECK(!commit(s, 255));
  CHECK(commit(s, 0));
  CHECK(s.cursor.page == 0 && hasSeen(s, 1));
  CHECK(checkRule(s, rule(Kind::Seen, 1, 1)));
  CHECK(checkRule(s, rule(Kind::Decision, 0, 0)));
  CHECK(!hasSeen(s, 128) && !hasDecision(s, 512) && !hasFact(s, 64));
  s.cursor.mode = CHOICE;
  CHECK(!commit(s, 0));
  Scene repeated = *sceneById(1);
  repeated.once = false; repeated.cooldown = 2;
  CHECK(!sceneEligible(s, repeated));
  advanceDays(s, 2);
  CHECK(sceneEligible(s, repeated));
  Condition missing = rule(Kind::Fact, 63, 1);
  repeated.requirements = {&missing, 1};
  CHECK(!sceneEligible(s, repeated));
  s.cursor = {}; s.cursor.mode = MAP;
  CHECK(!visit(s, LOCATION_COUNT));
  rest(s);
  CHECK(s.day == 4);
  advanceDays(s, 65535);
  CHECK(s.day == 65535);
  CHECK(change(s, Kind::Item, 2, 0));
  CHECK(!checkRule(s, rule(Kind::Day, 0, 1, static_cast<Compare>(99))));
}

static void metadata() {
  CHECK(!strcmp(locationName(VILLAGE), "Willowbrook"));
  CHECK(!strcmp(locationName(LOCATION_COUNT), "Unknown"));
  CHECK(strstr(itemDescription(0), "empty"));
  CHECK(!strcmp(itemName(0), "Unknown"));
  CHECK(!strcmp(skillName(255), "Unknown"));
  CHECK(!strcmp(npcName(255), "Unknown"));
  CHECK(!strcmp(promiseName(255), "Unknown"));
  CHECK(!strcmp(questName(255), "Unknown"));
  CHECK(factId("missing") == -1 && itemId("missing") == -1 && eventId("missing") == -1);
  for (int i = 0; i < content().factCount; ++i) CHECK(factId(content().facts[i].key) == i);
  for (int i = 0; i < content().itemCount; ++i) CHECK(itemId(content().items[i].key) == i + 1);
  for (int i = 0; i < content().eventCount; ++i) CHECK(eventId(content().events[i].key) == i);
  CHECK(!sceneById(127) && !choiceById(511));
  Save s;
  newCampaign(s, 1, 0);
  CHECK(strstr(questObjective(s, 255), "Explore"));
  s.quests[0] = 255;
  CHECK(strstr(questObjective(s, 0), "Explore"));
  s.quests[0] = ACTIVE;
  CHECK(strlen(questObjective(s, 0)) > 0);
  CHECK(strlen(itemName(1)) && strlen(itemDescription(1)));
  CHECK(strlen(skillName(0)) && strlen(npcName(0)) && strlen(promiseName(0)));
}

static int selectChoice(const Save& save, const Scene& scene, int route, int style) {
  if (scene.id == 22) return allowed(save, scene.choices[route - 1]) ? route - 1 : 4;
  for (int n = 0; n < scene.choiceCount; ++n) {
    int index = style ? scene.choiceCount - n - 1 : n;
    if (allowed(save, scene.choices[index])) return index;
  }
  fprintf(stderr, "No available choice: seed=%u scene=%s day=%u\n", save.seed, scene.key, save.day);
  CHECK(false);
  return 0;
}

static uint8_t reached[128] = {};
static void playScene(Save& save, int route, int style) {
  const Scene* scene = currentScene(save);
  CHECK(scene && scene->paragraphCount && scene->choiceCount);
  reached[scene->id] = 1;
  Save before = save;
  selectScene(save, VILLAGE);
  CHECK(!memcmp(&before, &save, sizeof(save)));
  save.cursor.page = 2;  // a mid-reading interruption and full reread remain pure
  save.returnCursor = save.cursor;
  save.cursor.mode = JOURNAL;
  sealSave(save);
  Save loaded;
  CHECK(loadSave(loaded, &save, sizeof(save), save.profile));
  CHECK(loaded.returnCursor.page == 2 && loaded.returnCursor.scene == scene->id);
  save.cursor = save.returnCursor;
  save.cursor.page = 0; save.cursor.mode = CHOICE;
  int choice = selectChoice(save, *scene, route, style);
  CHECK(commit(save, choice));
  CHECK(save.cursor.mode == RESULT && save.cursor.page == 0);
  CHECK(hasDecision(save, scene->choices[choice].id));
  before = save;
  CHECK(!commit(save, choice) && !memcmp(&before, &save, sizeof(save)));
  sealSave(save);
  CHECK(loadSave(loaded, &save, sizeof(save), save.profile));
  CHECK(!commit(loaded, choice));
  finishResult(save);
  finishResult(loaded);
  CHECK(!memcmp(&save, &loaded, sizeof(save)));
}

static Save completeRoute(uint32_t seed, int route, int style) {
  Save save;
  newCampaign(save, seed, 0);
  int destination = 0;
  for (int action = 0; action < 240; ++action) {
    if (save.quests[0] == RESOLVED && save.cursor.mode == MAP) break;
    if (save.cursor.mode == READ) { playScene(save, route, style); continue; }
    CHECK(save.cursor.mode == MAP);
    Location place = static_cast<Location>(save.location);
    if (!selectScene(save, place)) place = static_cast<Location>(destination++ % LOCATION_COUNT);
    visit(save, place);
  }
  if (save.quests[0] != RESOLVED) fprintf(stderr, "Stuck seed=%u route=%d scene=%u day=%u\n", seed, route, save.cursor.scene, save.day);
  CHECK(save.quests[0] == RESOLVED && save.deeds == 1 && save.outcome == route);
  CHECK(save.eventDone != 0 && hasSeen(save, 34));
  return save;
}

static void allWorldsAndRoutes() {
  uint32_t variants = 0;
  for (uint32_t seed = 0; seed < 128; ++seed) {
    Save a, b;
    newCampaign(a, seed, 0); newCampaign(b, seed, 0);
    CHECK(!memcmp(&a, &b, sizeof(a)));
    int variant = ((a.variants[0] * 2 + a.variants[1]) * 2 + a.variants[2]) * 3 + a.variants[3];
    variants |= 1U << variant;
    for (int route = 1; route <= 4; ++route) {
      Save done = completeRoute(seed, route, seed % 2);
      CHECK(hasFact(done, factId("flag.chapter_done")));
      if (route == 4) CHECK(done.quests[1] != RESOLVED);  // paid channel work does not repair the bridge
      for (int action = 0; action < 18; ++action) {
        if (done.cursor.mode == READ) playScene(done, route, seed % 2);
        else visit(done, static_cast<Location>(action % LOCATION_COUNT));
      }
      CHECK(done.deeds == 1 && done.quests[0] == RESOLVED);
    }
  }
  CHECK(variants == 0xffffffU);  // all 24 authored initial combinations exercised
  for (int id = 1; id <= content().sceneCount; ++id) {
    if (!reached[id]) fprintf(stderr, "Route coverage missed scene %d\n", id);
    CHECK(reached[id]);
  }
}

static void completedQuestsStayComplete() {
  Save save;
  newCampaign(save, 9, 0);
  CHECK(change(save, Kind::Quest, 3, RESOLVED));
  CHECK(applyEffects(save, sceneById(4)->choices[0].effects));
  CHECK(save.quests[3] == RESOLVED);  // first mill visit after river follow-through
  CHECK(applyEffects(save, sceneById(18)->choices[1].effects));
  CHECK(save.quests[2] == RESOLVED);  // the slow recipe actually supplies families
  CHECK(applyEffects(save, sceneById(16)->choices[0].effects));
  CHECK(save.quests[2] == RESOLVED);  // later introductory ledger cannot undo delivery
  CHECK(change(save, Kind::Quest, 3, BLOCKED) && save.quests[3] == RESOLVED);
}

static void seededSchedule() {
  for (int variant = 0; variant < 2; ++variant) {
    Save save;
    newCampaign(save, 42, 0);
    save.variants[2] = variant;
    const Effect schedule = {Kind::Schedule, 3, 2, 2};
    CHECK(applyEffects(save, {&schedule, 1}));
    CHECK(save.eventDue[3] == uint32_t(3 + variant));
    advanceDays(save, 1 + variant);
    CHECK(!(save.eventDone & 8));
    advanceDays(save, 1);
    CHECK(save.eventDone & 8);
    const Effect invalid = {Kind::Schedule, 0, 2, 4};
    CHECK(!applyEffects(save, {&invalid, 1}));
    const Effect negative = {Kind::Schedule, 0, 2, -2};
    CHECK(!applyEffects(save, {&negative, 1}));
  }
}

int main() {
  openingIsAtomic();
  exactSnapshot();
  declarativeRules();
  groupedRules();
  atomicInventory();
  rejectedEffects();
  scheduledOnce();
  corruptSnapshots();
  stateBounds();
  invalidInfoCursors();
  invalidProgress();
  eligibilityAndNavigation();
  metadata();
  completedQuestsStayComplete();
  seededSchedule();
  allWorldsAndRoutes();
  printf("kingdom: %d checks passed\n", checks);
}
