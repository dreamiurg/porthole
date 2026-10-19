#pragma once
#include <stddef.h>
#include <stdint.h>

namespace kingdom {
enum Location : uint8_t { VILLAGE, BRIDGE, MILL, WOODS, MARSH, TOWER, LOCATION_COUNT };
// Append modes: PARTY and INVENTORY retain their original saved detail-page meanings.
enum Mode : uint8_t {
  TITLE, MAP, READ, CHOICE, CONFIRM, RESULT, PARTY, INVENTORY, JOURNAL, MENU, TRAVEL, REST,
  PARTY_OVERVIEW, INVENTORY_OVERVIEW
};
enum QuestState : uint8_t { UNDISCOVERED, AVAILABLE, ACTIVE, BLOCKED, DEFERRED, RESOLVED };
enum PromiseState : uint8_t { NO_PROMISE, OPEN, KEPT, BROKEN };
constexpr uint32_t SAVE_MAGIC = 0x4c4b3031;
constexpr uint16_t SAVE_VERSION = 1;
constexpr int MAX_SCENES = 128, MAX_DECISIONS = 512, MAX_FACTS = 64;
constexpr int MAX_ITEMS = 16, MAX_EVENTS = 8, MAX_JOURNAL = 96;
constexpr int SKILL_COUNT = 3, QUEST_COUNT = 16, MAX_PROMISES = 16, MAX_NPCS = 16;

#pragma pack(push, 1)
struct UiCursor {
  uint16_t scene, page;
  uint8_t mode, choicePage, selectedChoice, infoPage;
};
struct ItemSlot { uint8_t id, quantity; };  // id zero means empty; metadata ids start at one.
struct JournalEntry { uint16_t id, day; };  // high bit identifies an event; others are decisions.
struct Save {
  // Version 1 is immutable. Future versions append fields and migrate this full prefix.
  uint32_t magic;
  uint16_t version, bytes;
  uint32_t seed, rng;
  uint16_t day;
  uint8_t profile, location, outcome, deeds;
  UiCursor cursor, returnCursor;
  uint64_t facts;
  uint8_t variants[4], skills[SKILL_COUNT], quests[QUEST_COUNT];
  int8_t relations[MAX_NPCS];
  uint8_t promises[MAX_PROMISES];
  uint16_t promiseDays[MAX_PROMISES];
  ItemSlot inventory[6];
  uint32_t eventDue[MAX_EVENTS];
  uint8_t eventDone, journalCount;
  uint8_t seen[MAX_SCENES / 8], decisions[MAX_DECISIONS / 8];
  uint16_t lastVisit[MAX_SCENES];
  JournalEntry journal[MAX_JOURNAL];
  uint8_t reserved[32];
  uint32_t crc;
};
#pragma pack(pop)
void sealSave(Save& save);
bool loadSave(Save& out, const void* data, size_t bytes, uint8_t profile);
constexpr size_t saveSize() { return sizeof(Save); }
}  // namespace kingdom
