#include "game.h"
#include <stdio.h>
#include <string.h>

namespace kingdom {
namespace {
bool evidence(const Save& save, int index) {
  return hasFact(save, (uint8_t)index) && !strncmp(content().facts[index].key, "clue.", 5);
}
} // namespace
int Game::infoCount() const {
  if (save_.cursor.mode == PARTY || save_.cursor.mode == PARTY_OVERVIEW) return 3;
  if (save_.cursor.mode == INVENTORY || save_.cursor.mode == INVENTORY_OVERVIEW) return 6;
  int total = content().questCount + save_.journalCount;
  for (int i = 0; i < content().factCount; i++) total += evidence(save_, i);
  for (int i = 0; i < content().promiseCount; i++) total += save_.promises[i] != NO_PROMISE;
  return total;
}
void Game::prepareJournal() {
  int index = save_.cursor.infoPage;
  if (index < content().questCount) {
    static const char* STATES[] = {"Not yet found", "Available", "Active", "Waiting for help", "Deferred", "Complete"};
    const int status = save_.quests[index] < 6 ? save_.quests[index] : 0;
    snprintf(heading_, sizeof heading_, "%s", questName((uint8_t)index));
    snprintf(prose_, sizeof prose_, "%s\n\n%s", STATES[status], questObjective(save_, (uint8_t)index)); return;
  }
  index -= content().questCount;
  for (int i = 0; i < content().factCount; i++) {
    if (!evidence(save_, i)) continue;
    if (index--) continue;
    snprintf(heading_, sizeof heading_, "Known: %s", content().facts[i].name);
    snprintf(prose_, sizeof prose_, "%s", content().facts[i].description); return;
  }
  for (int i = 0; i < content().promiseCount; i++) {
    if (save_.promises[i] == NO_PROMISE) continue;
    if (index--) continue;
    const char* state = save_.promises[i] == OPEN ? "Still open" : save_.promises[i] == KEPT ? "Kept" : "Needs repair";
    snprintf(heading_, sizeof heading_, "Promise: %s", promiseName((uint8_t)i));
    snprintf(prose_, sizeof prose_, "Day %u: %s.\n\n%s", save_.promiseDays[i], state, content().promises[i].description); return;
  }
  const int entry = save_.journalCount - index - 1;
  if (entry >= 0 && entry < save_.journalCount) {
    snprintf(heading_, sizeof heading_, "Our story, day %u", save_.journal[entry].day);
    snprintf(prose_, sizeof prose_, "%s", journalText(save_.journal[entry]));
  }
}
void Game::prepareInfo() {
  if (save_.cursor.infoPage >= infoCount()) save_.cursor.infoPage = 0;
  const int page = save_.cursor.infoPage;
  if (save_.cursor.mode == JOURNAL) { prepareJournal(); return; }
  if (save_.cursor.mode == INVENTORY) {
    const ItemSlot& slot = save_.inventory[page];
    if (slot.id) snprintf(heading_, sizeof heading_, "%s x%u", itemName(slot.id), slot.quantity);
    else snprintf(heading_, sizeof heading_, "An empty pocket");
    snprintf(prose_, sizeof prose_, "%s\n\n%s", slot.id ? itemDescription(slot.id) : "There is room for another useful thing.",
      slot.id ? "Items help through story choices. Reading never spends an item. Important clues stay in the journal." : "You have six pockets. Clues and promises do not need a pocket.");
    return;
  }
  static const char* NAMES[] = {"Princess Pulverizer", "Lucas", "Dribble"};
  snprintf(heading_, sizeof heading_, "%s", NAMES[page]);
  if (!page) {
    snprintf(prose_, sizeof prose_, "Courage %u / Ingenuity %u / Insight %u\n\nA future knight. Skills grow through accomplishments, never by repeating a choice. Good deeds: %u of 8.",
      save_.skills[0], save_.skills[1], save_.skills[2], save_.deeds); return;
  }
  const char* advice = page == 1 ? "Lucas notices repair marks, tracks and missing details. His insight can open careful ways through a problem."
                               : "Dribble knows cooking, warmth and living things. A dragon's unusual ideas can help where a sword cannot.";
  snprintf(prose_, sizeof prose_, "%s\n\n%s", advice, page == 1 ? "'A rumor is a question, not an answer. Let's hear both sides.'" : "'Bread needs flour. Eggs need water. Everybody needs supper.'");
}
void Game::infoStep(int delta) {
  const int count = infoCount(); if (!count) return;
  if (delta > 0 && save_.cursor.page + 1 < reader_.count()) save_.cursor.page++;
  else if (delta < 0 && save_.cursor.page) save_.cursor.page--;
  else {
    save_.cursor.infoPage = (uint8_t)((save_.cursor.infoPage + count + delta) % count);
    save_.cursor.page = 0; prepare();
    if (delta < 0) save_.cursor.page = (uint16_t)(reader_.count() - 1);
  }
  changed();
}
} // namespace kingdom
