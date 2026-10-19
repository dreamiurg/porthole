#include "content.h"
#include <string.h>

namespace kingdom {
namespace {
#include "generated/content.inc"
const Named* named(const Named* table, int count, int id) {
  return id >= 0 && id < count ? &table[id] : nullptr;
}
const char* nameOf(const Named* table, int count, int id) {
  const Named* entry = named(table, count, id);
  return entry ? entry->name : "Unknown";
}
int find(const Named* table, int count, const char* key) {
  for (int i = 0; i < count; ++i) if (!strcmp(table[i].key, key)) return i;
  return -1;
}
}  // namespace

const Content& content() { return CHAPTER; }
const Scene* sceneById(uint16_t id) {
  for (int i = 0; i < CHAPTER.sceneCount; ++i) if (CHAPTER.scenes[i].id == id) return &CHAPTER.scenes[i];
  return nullptr;
}
const Choice* choiceById(uint16_t id) {
  for (int i = 0; i < CHAPTER.sceneCount; ++i) {
    const Scene& scene = CHAPTER.scenes[i];
    for (int j = 0; j < scene.choiceCount; ++j) if (scene.choices[j].id == id) return &scene.choices[j];
  }
  return nullptr;
}
const char* locationName(Location location) {
  static const char* const NAMES[] = {"Willowbrook", "Old Bridge", "Miller's Mill", "North Woods", "Riverside Marsh", "Old Watchtower"};
  return location < LOCATION_COUNT ? NAMES[location] : "Unknown";
}
const char* itemName(uint8_t id) { return nameOf(CHAPTER.items, CHAPTER.itemCount, int(id) - 1); }
const char* itemDescription(uint8_t id) {
  const Named* entry = named(CHAPTER.items, CHAPTER.itemCount, int(id) - 1);
  return entry ? entry->description : "An empty pocket.";
}
const char* skillName(uint8_t id) { return nameOf(CHAPTER.skills, CHAPTER.skillCount, id); }
const char* npcName(uint8_t id) { return nameOf(CHAPTER.npcs, CHAPTER.npcCount, id); }
const char* promiseName(uint8_t id) { return nameOf(CHAPTER.promises, CHAPTER.promiseCount, id); }
const char* questName(uint8_t id) { return id < CHAPTER.questCount ? CHAPTER.quests[id].name : "Unknown"; }
const char* questObjective(const Save& save, uint8_t id) {
  if (id >= CHAPTER.questCount || save.quests[id] > RESOLVED) return "Explore the kingdom.";
  return CHAPTER.quests[id].objectives[save.quests[id]];
}
const char* journalText(const JournalEntry& entry) {
  if (entry.id & 0x8000) {
    uint16_t event = entry.id & 0x7fff;
    return event < CHAPTER.eventCount ? CHAPTER.events[event].description : "An earlier world event.";
  }
  const Choice* choice = choiceById(entry.id);
  return choice ? choice->journal : "An earlier decision remains part of our story.";
}
int factId(const char* key) { return find(CHAPTER.facts, CHAPTER.factCount, key); }
int itemId(const char* key) {
  int id = find(CHAPTER.items, CHAPTER.itemCount, key);
  return id < 0 ? -1 : id + 1;
}
int eventId(const char* key) {
  for (int i = 0; i < CHAPTER.eventCount; ++i) if (!strcmp(CHAPTER.events[i].key, key)) return i;
  return -1;
}
}  // namespace kingdom
