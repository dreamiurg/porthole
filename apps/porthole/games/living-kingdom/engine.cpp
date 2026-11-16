#include "engine.h"
#include <limits.h>
#include <string.h>
#include "bits.h"
#include "crc32.h"

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
// In place: on false the save is left half-changed, so every caller hands in a scratch copy it then drops.
// (One 935-byte Save per transaction on the loop task's stack, not two.)
bool applyAll(Save& save, Effects effects) {
  for (int i = 0; i < effects.count; ++i) if (!effect(save, effects.rules[i])) return false;
  return true;
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
  if (!applyAll(changed, effects)) return false;
  save = changed;
  return true;
}
bool allowed(const Save& save, const Choice& choice) {
  if (!meets(save, choice.requirements)) return false;
  Save changed = save;
  return applyAll(changed, choice.effects);
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
    if (!applyAll(changed, content().events[i].effects)) continue;
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
  if (!meets(save, choice.requirements)) return false;
  Save changed = save;
  if (!applyAll(changed, choice.effects)) return false;   // with the line above, exactly allowed(), without its second copy
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
  save.seed = seed; save.rng = mix(seed); save.profile = profile; save.day = 1;
  for (int i = 0; i < 4; ++i) { save.rng = mix(save.rng + uint32_t(i) + 1); save.variants[i] = save.rng % (i == 3 ? 3 : 2); }
  save.skills[0] = 3; save.skills[1] = 2; save.skills[2] = 2;
  save.quests[0] = ACTIVE; save.quests[1] = AVAILABLE; save.quests[2] = AVAILABLE;
  openScene(save, sceneById(1));
  sealSave(save);
}
void sealSave(Save& save) {
  save.magic = SAVE_MAGIC; save.version = SAVE_V1; save.bytes = sizeof(Save);
  save.crc = os::crc32(&save, offsetof(Save, crc));
}

// ---- version 2: Save's fields, bit-packed (os/bits.h: one description both writes and reads)
namespace {
// seen and decisions repeat what lastVisit and the journal say (commit() sets them together): XORed with that they
// are all zeros in a real campaign, which pack to a bit a byte, and still exact in any other. Its own inverse.
void fold(Save& s) {
  for (int i = 0; i < MAX_SCENES; ++i) if (s.lastVisit[i]) s.seen[i / 8] ^= uint8_t(1U << (i % 8));
  for (int i = 0; i < s.journalCount && i < MAX_JOURNAL; ++i) {
    const uint16_t id = s.journal[i].id;
    if (id < MAX_DECISIONS) s.decisions[id / 8] ^= uint8_t(1U << (id % 8));
  }
}
void packCursor(os::Bits& b, UiCursor& c) {
  c.scene = b.io(c.scene, 16); c.page = b.io(c.page, 16); c.mode = b.io(c.mode, 4);
  c.choicePage = b.io(c.choicePage, 8); c.selectedChoice = b.io(c.selectedChoice, 8); c.infoPage = b.io(c.infoPage, 8);
}
// A field validState bounds gets the bits its range needs (a wider value fails the pack, and reads back as invalid);
// one it does not bound is kept whole.
void packWorld(os::Bits& b, Save& s) {
  s.seed = b.io(s.seed, 32); s.rng = b.io(s.rng, 32); s.day = b.io(s.day, 16);
  s.profile = b.io(s.profile, 2); s.location = b.io(s.location, 3); s.outcome = b.io(s.outcome, 3); s.deeds = b.io(s.deeds, 4);
  packCursor(b, s.cursor); packCursor(b, s.returnCursor);
  const uint32_t low = b.io(uint32_t(s.facts), 32), high = b.io(uint32_t(s.facts >> 32), 32);
  s.facts = uint64_t(high) << 32 | low;
  for (int i = 0; i < 4; ++i) s.variants[i] = b.io(s.variants[i], 2);
  for (int i = 0; i < SKILL_COUNT; ++i) s.skills[i] = b.io(s.skills[i], 3);
  for (int i = 0; i < QUEST_COUNT; ++i) s.quests[i] = b.io(s.quests[i], 3);
  for (int i = 0; i < MAX_NPCS; ++i) s.relations[i] = int8_t(int(b.io(uint32_t(s.relations[i] + 5), 4)) - 5);
  for (int i = 0; i < MAX_PROMISES; ++i) s.promises[i] = b.io(s.promises[i], 2);
  for (ItemSlot& slot : s.inventory) { slot.id = b.io(slot.id, 5); slot.quantity = b.io(slot.quantity, 7); }
  for (int i = 0; i < MAX_EVENTS; ++i) s.eventDue[i] = b.sparse(s.eventDue[i], 32);
  s.eventDone = b.io(s.eventDone, 8);
}
// What grows with the story. Days and journal ids take the bits the widest one needs (a chapter's fit in half of
// their 16); only the journal's used entries are kept (the rest carry nothing: record() writes each before it counts).
void packStory(os::Bits& b, Save& s) {
  uint16_t days = 0, ids = 0;
  for (int i = 0; i < MAX_SCENES; ++i) days |= s.lastVisit[i];
  for (int i = 0; i < MAX_PROMISES; ++i) days |= s.promiseDays[i];
  for (int i = 0; i < s.journalCount && i < MAX_JOURNAL; ++i) { days |= s.journal[i].day; ids |= s.journal[i].id & 0x7fff; }
  const int dayBits = int(b.io(os::bitsFor(days), 5)), idBits = int(b.io(os::bitsFor(ids), 4));
  s.journalCount = b.io(s.journalCount, 7);
  if (dayBits > 16 || s.journalCount > MAX_JOURNAL) { b.bad = true; return; }
  for (int i = 0; i < MAX_PROMISES; ++i) s.promiseDays[i] = b.sparse(s.promiseDays[i], dayBits);
  for (int i = 0; i < MAX_SCENES / 8; ++i) s.seen[i] = b.sparse(s.seen[i], 8);
  for (int i = 0; i < MAX_DECISIONS / 8; ++i) s.decisions[i] = b.sparse(s.decisions[i], 8);
  for (int i = 0; i < MAX_SCENES; ++i) s.lastVisit[i] = b.sparse(s.lastVisit[i], dayBits);
  for (int i = 0; i < s.journalCount; ++i) {
    const uint32_t event = b.io(s.journal[i].id >> 15, 1);
    s.journal[i].id = uint16_t(event << 15 | b.io(s.journal[i].id & 0x7fff, idBits));
    s.journal[i].day = b.io(s.journal[i].day, dayBits);
  }
}
bool unpackSave(Save& out, const uint8_t* data, size_t bytes) {
  uint32_t crc;
  memcpy(&crc, data + bytes - sizeof crc, sizeof crc);
  os::Bits b = os::Bits::reader(data, bytes - sizeof crc);
  if (b.io(0, 32) != SAVE_MAGIC || b.io(0, 16) != SAVE_VERSION || b.io(0, 16) != bytes) return false;
  if (crc != os::crc32(data, bytes - sizeof crc)) return false;
  out = {};
  packWorld(b, out); packStory(b, out);
  if (!b.ok() || b.bytes() != bytes - sizeof crc) return false;
  fold(out); sealSave(out);
  return true;
}
}  // namespace

size_t packSave(const Save& save, uint8_t* out, size_t cap) {
  if (cap < 12) return 0;
  memset(out, 0, cap);
  Save s = save;
  fold(s);
  os::Bits b = os::Bits::writer(out, cap - 4);
  b.io(SAVE_MAGIC, 32); b.io(SAVE_VERSION, 16); b.io(0, 16);
  packWorld(b, s); packStory(b, s);
  if (!b.ok()) return 0;
  const uint16_t bytes = uint16_t(b.bytes() + 4);
  memcpy(out + 6, &bytes, sizeof bytes);
  const uint32_t crc = os::crc32(out, bytes - sizeof crc);
  memcpy(out + bytes - sizeof crc, &crc, sizeof crc);
  return bytes;
}
bool loadSave(Save& out, const void* data, size_t bytes, uint8_t profile) {
  if (!data || bytes < 12) return false;
  Save loaded;
  uint16_t version;
  memcpy(&version, static_cast<const uint8_t*>(data) + offsetof(Save, version), sizeof version);
  if (version == SAVE_V1) {   // the struct itself
    if (bytes != sizeof(Save)) return false;
    memcpy(&loaded, data, sizeof(loaded));
    if (loaded.magic != SAVE_MAGIC || loaded.bytes != sizeof(Save)) return false;
    if (loaded.crc != os::crc32(&loaded, offsetof(Save, crc))) return false;
  } else if (!unpackSave(loaded, static_cast<const uint8_t*>(data), bytes)) return false;
  if (!validState(loaded, profile)) return false;
  out = loaded;
  return true;
}
}  // namespace kingdom
