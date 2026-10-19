#pragma once
#include "save.h"

namespace kingdom {
enum class Kind : uint8_t {
  Fact, Item, Quest, Skill, Relation, Promise, Decision, Day, Variant, Outcome,
  Seen, Event, All, Any, Schedule, Location, Deed
};
enum class Compare : uint8_t { Eq, Ne, Ge, Le };
struct Condition {
  Kind kind;
  uint16_t id;
  int16_t value;
  Compare compare;
  const Condition* children;
  uint8_t count;
  const char* reason;
};
struct Requirements { const Condition* rules; uint8_t count; };
struct Effect {
  Kind kind;
  uint16_t id;
  int16_t value;
  int8_t variant = -1;  // optional seed variant added to value; never rerolled
};
struct Effects { const Effect* rules; uint8_t count; };
struct Choice {
  uint16_t id;
  const char *key, *caption, *detail, *journal, *result;
  Requirements requirements;
  Effects effects;
  uint16_t next;
  bool confirm;
};
struct Scene {
  uint16_t id;
  const char *key, *title;
  Location location;
  int16_t priority;
  bool once;
  uint8_t cooldown;
  Requirements requirements;
  const char* const* paragraphs;
  uint16_t paragraphCount;
  const Choice* choices;
  uint8_t choiceCount;
};
struct Named { const char *key, *name, *description; };
struct Quest { const char *key, *name; const char* objectives[6]; };
struct Event { const char *key, *description; Effects effects; };
struct Content {
  const Scene* scenes; uint16_t sceneCount;
  const Named* facts; uint8_t factCount;
  const Named* items; uint8_t itemCount;
  const Named* skills; uint8_t skillCount;
  const Named* npcs; uint8_t npcCount;
  const Named* promises; uint8_t promiseCount;
  const Quest* quests; uint8_t questCount;
  const Event* events; uint8_t eventCount;
};
const Content& content();
const Scene* sceneById(uint16_t id);
const Choice* choiceById(uint16_t id);
const char* locationName(Location location);
const char* itemName(uint8_t id);
const char* itemDescription(uint8_t id);
const char* skillName(uint8_t id);
const char* npcName(uint8_t id);
const char* promiseName(uint8_t id);
const char* questName(uint8_t id);
const char* questObjective(const Save& save, uint8_t id);
const char* journalText(const JournalEntry& entry);
int factId(const char* key);
int itemId(const char* key);
int eventId(const char* key);
}  // namespace kingdom
