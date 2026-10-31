#include "engine.h"
#include <limits.h>
#include <string.h>

namespace kingdom {
namespace {
uint32_t mix(uint32_t value) {
  value ^= value >> 16; value *= 0x7feb352dU;
  value ^= value >> 15; value *= 0x846ca68bU;
  return value ^ (value >> 16);
}
int clamp(int value, int minimum, int maximum) {
  return value < minimum ? minimum : (value > maximum ? maximum : value);
}
bool bit(const uint8_t* bytes, uint16_t id) { return (bytes[id / 8] & (1U << (id % 8))) != 0; }
void setBit(uint8_t* bytes, uint16_t id) { bytes[id / 8] |= uint8_t(1U << (id % 8)); }
void record(Save& save, uint16_t id) {
  for (int i = 0; i < save.journalCount; ++i) if (save.journal[i].id == id) return;
  if (save.journalCount < MAX_JOURNAL) save.journal[save.journalCount++] = {id, save.day};
}
int baseValue(const Save& save, const Condition& rule) {
  switch (rule.kind) {
    case Kind::Fact: return rule.id < MAX_FACTS && hasFact(save, rule.id);
    case Kind::Item: return itemCount(save, rule.id);
    case Kind::Quest: return rule.id < QUEST_COUNT ? save.quests[rule.id] : -1;
    case Kind::Skill: return rule.id < SKILL_COUNT ? save.skills[rule.id] : -1;
    case Kind::Relation: return rule.id < MAX_NPCS ? save.relations[rule.id] : -1;
    case Kind::Promise: return rule.id < MAX_PROMISES ? save.promises[rule.id] : -1;
    case Kind::Decision: return hasDecision(save, rule.id);
    default: return -1;
  }
}
int worldValue(const Save& save, const Condition& rule) {
  switch (rule.kind) {
    case Kind::Day: return save.day;
    case Kind::Variant: return rule.id < 4 ? save.variants[rule.id] : -1;
    case Kind::Outcome: return save.outcome;
    case Kind::Seen: return hasSeen(save, rule.id);
    case Kind::Event: return rule.id < MAX_EVENTS && (save.eventDone & (1U << rule.id)) != 0;
    default: return baseValue(save, rule);
  }
}
bool compare(int actual, const Condition& rule) {
  switch (rule.compare) {
    case Compare::Eq: return actual == rule.value;
    case Compare::Ne: return actual != rule.value;
    case Compare::Ge: return actual >= rule.value;
    case Compare::Le: return actual <= rule.value;
  }
  return false;
}
bool condition(const Save& save, const Condition& rule) {
  if (rule.kind != Kind::All && rule.kind != Kind::Any) return compare(worldValue(save, rule), rule);
  bool any = rule.kind == Kind::Any;
  for (int i = 0; i < rule.count; ++i) if (condition(save, rule.children[i]) == any) return any;
  return !any;
}
bool addItem(Save& save, uint16_t id, int delta) {
  if (!id || id > content().itemCount) return false;
  ItemSlot* empty = nullptr;
  for (ItemSlot& slot : save.inventory) {
    if (!slot.id) empty = &slot;
    if (slot.id != id) continue;
    int quantity = slot.quantity + delta;
    if (quantity < 0 || quantity > 99) return false;
    slot = quantity ? ItemSlot{uint8_t(id), uint8_t(quantity)} : ItemSlot{};
    return true;
  }
  if (delta == 0) return true;
  if (delta < 0 || delta > 99 || !empty) return false;
  *empty = {uint8_t(id), uint8_t(delta)};
  return true;
}
bool setPromise(Save& save, const Effect& effect) {
  if (effect.id >= MAX_PROMISES || effect.value < 0 || effect.value > BROKEN) return false;
  if (effect.value >= KEPT && !save.promises[effect.id]) return true;
  if (effect.value == OPEN && !save.promises[effect.id]) save.promiseDays[effect.id] = save.day;
  save.promises[effect.id] = effect.value;
  return true;
}
bool setStatus(Save& save, const Effect& effect) {
  switch (effect.kind) {
    case Kind::Quest:
      if (effect.id >= QUEST_COUNT || effect.value < 0 || effect.value > RESOLVED) return false;
      if (save.quests[effect.id] == RESOLVED) return true;
      save.quests[effect.id] = effect.value; return true;
    case Kind::Promise: return setPromise(save, effect);
    case Kind::Skill:
      if (effect.id >= SKILL_COUNT) return false;
      save.skills[effect.id] = clamp(save.skills[effect.id] + effect.value, 0, 6); return true;
    case Kind::Relation:
      if (effect.id >= MAX_NPCS) return false;
      save.relations[effect.id] = clamp(save.relations[effect.id] + effect.value, -5, 5); return true;
    default: return false;
  }
}
bool setWorld(Save& save, const Effect& effect) {
  switch (effect.kind) {
    case Kind::Outcome:
      if (effect.value < 0 || effect.value > 4) return false;
      save.outcome = effect.value; return true;
    case Kind::Location:
      if (effect.id >= LOCATION_COUNT) return false;
      save.location = effect.id; return true;
    case Kind::Deed: save.deeds = clamp(save.deeds + effect.value, 0, 8); return true;
    case Kind::Schedule:
      if (effect.id >= content().eventCount || effect.value < 1) return false;
      if (!save.eventDue[effect.id] && !(save.eventDone & (1U << effect.id)))
        save.eventDue[effect.id] = clamp(int(save.day) + effect.value, 1, UINT16_MAX);
      return true;
    default: return setStatus(save, effect);
  }
}
bool effect(Save& save, Effect rule) {
  if (rule.variant >= 4 || rule.variant < -1) return false;
  if (rule.variant >= 0) rule.value += save.variants[rule.variant];
  if (rule.kind == Kind::Fact) {
    if (rule.id >= MAX_FACTS) return false;
    uint64_t flag = uint64_t(1) << rule.id;
    save.facts = rule.value ? save.facts | flag : save.facts & ~flag;
    return true;
  }
  if (rule.kind == Kind::Item) return addItem(save, rule.id, rule.value);
  if (rule.kind != Kind::Day) return setWorld(save, rule);
  if (rule.value < 0) return false;
  advanceDays(save, rule.value);
  return true;
}
uint32_t crc32(const void* data, size_t bytes) {
  const uint8_t* p = static_cast<const uint8_t*>(data);
  uint32_t crc = 0xffffffffU;
  for (size_t i = 0; i < bytes; ++i) {
    crc ^= p[i];
    for (int bit = 0; bit < 8; ++bit) crc = (crc >> 1) ^ (0xedb88320U & (0U - (crc & 1U)));
  }
  return ~crc;
}
bool validCursor(const UiCursor& cursor) {
  if (cursor.mode > INVENTORY_OVERVIEW) return false;
  const Scene* scene = sceneById(cursor.scene);
  if (cursor.scene && !scene) return false;
  if (cursor.mode >= READ && cursor.mode <= RESULT && !scene) return false;
  if ((cursor.mode == CONFIRM || cursor.mode == RESULT) && cursor.selectedChoice >= scene->choiceCount) return false;
  if (cursor.mode >= READ && cursor.mode <= RESULT && cursor.choicePage >= (scene->choiceCount + 2) / 3) return false;
  return true;
}
bool validInfo(const UiCursor& cursor) {
  if (cursor.mode == PARTY || cursor.mode == PARTY_OVERVIEW) return cursor.infoPage < 3;
  if (cursor.mode == INVENTORY || cursor.mode == INVENTORY_OVERVIEW) return cursor.infoPage < 6;
  if (cursor.mode == TRAVEL) return cursor.infoPage < LOCATION_COUNT;
  return true;
}
bool validInventory(const Save& save) {
  uint32_t occupied = 0;
  for (const ItemSlot& slot : save.inventory) {
    if (slot.id > content().itemCount || slot.quantity > 99 || bool(slot.id) != bool(slot.quantity)) return false;
    if (slot.id && (occupied & (1U << slot.id))) return false;
    occupied |= 1U << slot.id;
  }
  return true;
}
bool validStatuses(const Save& save) {
  for (int i = 0; i < QUEST_COUNT; ++i) if (save.quests[i] > RESOLVED) return false;
  for (int i = 0; i < MAX_PROMISES; ++i) if (save.promises[i] > BROKEN) return false;
  return true;
}
bool validProgress(const Save& save) {
  if (save.deeds > 8) return false;
  for (uint8_t rank : save.skills) if (rank > 6) return false;
  for (int8_t relation : save.relations) if (relation < -5 || relation > 5) return false;
  for (int i = 0; i < 4; ++i) if (save.variants[i] > (i == 3 ? 2 : 1)) return false;
  for (uint8_t value : save.reserved) if (value) return false;
  return true;
}
bool validCursors(const Save& save) {
  return validCursor(save.cursor) && validCursor(save.returnCursor) && validInfo(save.cursor) && validInfo(save.returnCursor);
}
bool validState(const Save& save, uint8_t profile) {
  if (save.profile != profile || profile > 3 || save.location >= LOCATION_COUNT || !save.day || save.outcome > 4) return false;
  if (save.journalCount > MAX_JOURNAL) return false;
  return validInventory(save) && validCursors(save) && validStatuses(save) && validProgress(save);
}
void openScene(Save& save, const Scene* scene) {
  save.cursor = {};
  save.cursor.mode = scene ? READ : MAP;
  if (!scene) return;
  save.cursor.scene = scene->id;
  save.location = scene->location;
}
}  // namespace

bool hasFact(const Save& save, uint8_t id) { return id < MAX_FACTS && (save.facts & (uint64_t(1) << id)); }
bool hasSeen(const Save& save, uint16_t id) { return id < MAX_SCENES && bit(save.seen, id); }
bool hasDecision(const Save& save, uint16_t id) { return id < MAX_DECISIONS && bit(save.decisions, id); }
int itemCount(const Save& save, uint8_t id) {
  for (const ItemSlot& slot : save.inventory) if (slot.id == id && id) return slot.quantity;
  return 0;
}
bool meets(const Save& save, Requirements requirements) {
  for (int i = 0; i < requirements.count; ++i) if (!condition(save, requirements.rules[i])) return false;
  return true;
}
bool applyEffects(Save& save, Effects effects) {
  Save changed = save;
  for (int i = 0; i < effects.count; ++i) if (!effect(changed, effects.rules[i])) return false;
  save = changed;
  return true;
}
bool allowed(const Save& save, const Choice& choice) {
  if (!meets(save, choice.requirements)) return false;
  Save changed = save;
  return applyEffects(changed, choice.effects);
}
const char* requirementReason(const Save& save, const Choice& choice) {
  for (int i = 0; i < choice.requirements.count; ++i) {
    const Condition& rule = choice.requirements.rules[i];
    if (!condition(save, rule)) return rule.reason;
  }
  return allowed(save, choice) ? "Ready." : "Needs supplies or an empty inventory slot.";
}
const Scene* currentScene(const Save& save) { return sceneById(save.cursor.scene); }
bool sceneEligible(const Save& save, const Scene& scene) {
  if (!meets(save, scene.requirements)) return false;
  if (scene.once) return !hasSeen(save, scene.id);
  return !hasSeen(save, scene.id) || uint32_t(save.lastVisit[scene.id]) + scene.cooldown <= save.day;
}
const Scene* selectScene(const Save& save, Location location) {
  const Scene* selected = nullptr;
  uint32_t best = 0;
  for (int i = 0; i < content().sceneCount; ++i) {
    const Scene& scene = content().scenes[i];
    if (scene.location != location || !sceneEligible(save, scene)) continue;
    uint32_t rank = mix(save.seed ^ save.day ^ scene.id);
    if (selected && scene.priority < selected->priority) continue;
    if (selected && scene.priority == selected->priority && rank <= best) continue;
    selected = &scene; best = rank;
  }
  return selected;
}
void advanceDays(Save& save, uint16_t days) {
  save.day = uint16_t(clamp(int(save.day) + days, 1, UINT16_MAX));
  for (int i = 0; i < content().eventCount; ++i) {
    if (!save.eventDue[i] || save.eventDue[i] > save.day || (save.eventDone & (1U << i))) continue;
    Save changed = save;
    changed.eventDone |= uint8_t(1U << i);
    changed.eventDue[i] = 0;
    if (!applyEffects(changed, content().events[i].effects)) continue;
    record(changed, uint16_t(0x8000U | i));
    save = changed;
  }
}
bool visit(Save& save, Location location) {
  if (location >= LOCATION_COUNT || save.cursor.mode != MAP) return false;
  if (save.location != location) advanceDays(save, 1);
  save.location = location;
  const Scene* scene = selectScene(save, location);
  openScene(save, scene);
  return scene != nullptr;
}
void rest(Save& save) {
  if (save.cursor.mode != MAP) return;
  advanceDays(save, 1);
}
bool commit(Save& save, uint8_t choiceIndex) {
  const Scene* scene = currentScene(save);
  if (!scene || (save.cursor.mode != CHOICE && save.cursor.mode != CONFIRM) || choiceIndex >= scene->choiceCount) return false;
  const Choice& choice = scene->choices[choiceIndex];
  if (scene->once && hasSeen(save, scene->id)) return false;
  if (!allowed(save, choice)) return false;
  Save changed = save;
  if (!applyEffects(changed, choice.effects)) return false;
  setBit(changed.seen, scene->id);
  setBit(changed.decisions, choice.id);
  changed.lastVisit[scene->id] = changed.day;
  record(changed, choice.id);
  changed.cursor.mode = RESULT;
  changed.cursor.page = 0;
  changed.cursor.choicePage = 0;
  changed.cursor.selectedChoice = choiceIndex;
  save = changed;
  return true;
}
void finishResult(Save& save) {
  const Scene* scene = currentScene(save);
  if (!scene || save.cursor.mode != RESULT || save.cursor.selectedChoice >= scene->choiceCount) return;
  const Choice& choice = scene->choices[save.cursor.selectedChoice];
  const Scene* next = sceneById(choice.next);
  openScene(save, next && sceneEligible(save, *next) ? next : nullptr);
}
void newCampaign(Save& save, uint32_t seed, uint8_t profile) {
  save = {};
  save.magic = SAVE_MAGIC; save.version = SAVE_VERSION; save.bytes = sizeof(Save);
  save.seed = seed; save.rng = mix(seed); save.profile = profile; save.day = 1;
  for (int i = 0; i < 4; ++i) { save.rng = mix(save.rng + uint32_t(i) + 1); save.variants[i] = save.rng % (i == 3 ? 3 : 2); }
  save.skills[0] = 3; save.skills[1] = 2; save.skills[2] = 2;
  save.quests[0] = ACTIVE; save.quests[1] = AVAILABLE; save.quests[2] = AVAILABLE;
  openScene(save, sceneById(1));
  sealSave(save);
}
void sealSave(Save& save) {
  save.magic = SAVE_MAGIC; save.version = SAVE_VERSION; save.bytes = sizeof(Save);
  save.crc = crc32(&save, offsetof(Save, crc));
}
bool loadSave(Save& out, const void* data, size_t bytes, uint8_t profile) {
  if (!data || bytes != sizeof(Save)) return false;
  Save loaded;
  memcpy(&loaded, data, sizeof(loaded));
  if (loaded.magic != SAVE_MAGIC || loaded.version != SAVE_VERSION || loaded.bytes != sizeof(Save)) return false;
  if (loaded.crc != crc32(&loaded, offsetof(Save, crc)) || !validState(loaded, profile)) return false;
  out = loaded;
  return true;
}
}  // namespace kingdom
