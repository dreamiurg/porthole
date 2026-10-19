// Validate the compiled manuscript with the same fonts and page dimensions used on the device.
#include <assert.h>
#include <ctype.h>
#include <stdio.h>
#include <string.h>
#include <string>
#include "games/living-kingdom/art.h"
#include "games/living-kingdom/content.h"
#include "games/living-kingdom/layout.h"
#include "games/living-kingdom/reader.h"

using namespace kingdom;
namespace {
const font::Box FIRST = {&art::BODY, layout::TEXT_W, layout::FIRST_TEXT_H, 2, font::Align::LEFT};
const font::Box JOURNAL_BOX = {&art::BODY, layout::TEXT_W, layout::JOURNAL_TEXT_H, 2, font::Align::LEFT};
const font::Box BODY = {&art::BODY, layout::TEXT_W, layout::TEXT_H, 2, font::Align::LEFT};
int words(const char* text) {
  int count = 0; bool inWord = false;
  for (; *text; text++) {
    const bool next = !isspace(static_cast<unsigned char>(*text));
    if (next && !inWord) count++;
    inWord = next;
  }
  return count;
}
void supported(const char* text, const font::Font& face = art::BODY) {
  assert(text);
  for (const unsigned char* p = reinterpret_cast<const unsigned char*>(text); *p; p++) {
    assert(*p < 128); // The authored font intentionally supports the complete ASCII manuscript.
    if (!isspace(*p)) assert(font::glyphIndex(face, *p));
  }
}
int paginate(const char* text, const font::Box& box = BODY, const font::Box* first = nullptr) {
  supported(text, *box.font); Reader reader; assert(reader.open(text, box, first)); size_t consumed = 0;
  for (int i = 0; i < reader.count(); i++) {
    const char* page = reader.page(i); const size_t length = strlen(page);
    assert(font::textHeight(*box.font, page, box.w, box.spacing) <= (!i && first ? first->h : box.h));
    while (isspace(static_cast<unsigned char>(text[consumed]))) consumed++;
    assert(!strncmp(text + consumed, page, length)); consumed += length;
  }
  while (isspace(static_cast<unsigned char>(text[consumed]))) consumed++;
  assert(consumed == strlen(text)); return reader.count();
}
bool cardFits(const Scene& scene, const Choice& choice, int row) {
  const ui::Box cards[] = {layout::CARD0, layout::CARD1, layout::CARD2}; const ui::Box& card = cards[row];
  const int width = card.w * 3 - layout::CARD_PAD * 2;
  const int caption = font::textHeight(art::LABEL, choice.caption, width, 0);
  const int detail = font::textHeight(art::DETAIL, choice.detail, width, 0);
  if (caption > art::LABEL.lineHeight || detail > art::DETAIL.lineHeight * 2 || layout::DETAIL_Y + detail > card.h * 3 - 3 || layout::CAPTION_Y + caption > layout::DETAIL_Y)
    fprintf(stderr, "%s / %s: card %d, caption=%d detail=%d height=%d\n", scene.key, choice.key, row, caption, detail, card.h * 3);
  const bool fits = caption <= art::LABEL.lineHeight && detail <= art::DETAIL.lineHeight * 2 && layout::DETAIL_Y + detail <= card.h * 3 - 3 && layout::CAPTION_Y + caption <= layout::DETAIL_Y;
  supported(choice.caption, art::LABEL); supported(choice.detail, art::DETAIL);
  assert(font::textHeight(art::DETAIL, "Needs help - tap to learn why", width, 0) <= art::DETAIL.lineHeight * 2);
  return fits;
}
std::string sceneProse(const Scene& scene) {
  std::string prose;
  for (int i = 0; i < scene.paragraphCount; i++) { if (i) prose += "\n\n"; prose += scene.paragraphs[i]; }
  return prose;
}
bool sceneChoices(const Scene& scene, bool* decisions) {
  bool fits = true;
  for (int i = 0; i < scene.choiceCount; i++) {
    const Choice& choice = scene.choices[i];
    assert(choice.id && choice.id < MAX_DECISIONS && !decisions[choice.id]); decisions[choice.id] = true;
    assert(choiceById(choice.id) == &choice && (!choice.next || sceneById(choice.next)));
    assert(*choice.journal && *choice.result && *choice.caption && *choice.detail && strlen(choice.result) < 8192);
    fits = cardFits(scene, choice, i % 3) && fits;
    paginate(choice.result, BODY, &FIRST); paginate(choice.detail); paginate(choice.journal, JOURNAL_BOX);
  }
  return fits;
}
void manuscript() {
  const Content& c = content(); assert(c.sceneCount >= 20); int totalWords = 0, totalPages = 0, choices = 0;
  bool scenes[MAX_SCENES] = {}, decisions[MAX_DECISIONS] = {}, fits = true;
  for (int n = 0; n < c.sceneCount; n++) {
    const Scene& scene = c.scenes[n]; assert(scene.id && scene.id < MAX_SCENES && !scenes[scene.id]);
    scenes[scene.id] = true; assert(sceneById(scene.id) == &scene);
    assert(scene.location < LOCATION_COUNT && scene.paragraphCount && scene.choiceCount);
    const font::Font& header = font::textWidth(art::HEADING, scene.title) <= 296 ? art::HEADING : art::SMALL;
    supported(scene.title, header); assert(font::textHeight(header, scene.title, 296, 0) <= 52);
    const std::string prose = sceneProse(scene);
    assert(prose.size() < 8192); totalWords += words(prose.c_str()); totalPages += paginate(prose.c_str(), BODY, &FIRST);
    fits = sceneChoices(scene, decisions) && fits; choices += scene.choiceCount;
  }
  fprintf(stderr, "compiled manuscript: %u scenes, %d words, %d pages, %d choices\n", c.sceneCount, totalWords, totalPages, choices);
  assert(totalWords >= 4500 && totalPages >= 60 && choices >= c.sceneCount);
  assert(fits);
  printf("kingdom manuscript: %u scenes, %d words, %d readable pages, %d choices fit the compiled fonts\n", c.sceneCount, totalWords, totalPages, choices);
}
void journalHeading(const char* prefix, const char* name) {
  const std::string heading = std::string(prefix) + name;
  supported(heading.c_str(), art::HEADING);
  const int height = font::textHeight(art::HEADING, heading.c_str(), layout::TEXT_W, 0);
  if (height > 56) fprintf(stderr, "journal heading exceeds56px: %s (%dpx)\n", heading.c_str(), height);
  assert(height <= 56);
}
void metadata() {
  const Content& c = content(); Save saved{};
  assert(c.itemCount <= MAX_ITEMS && c.factCount <= MAX_FACTS && c.eventCount <= MAX_EVENTS);
  assert(c.npcCount <= MAX_NPCS && c.promiseCount <= MAX_PROMISES && c.questCount <= QUEST_COUNT);
  for (int i = 0; i < c.itemCount; i++) {
    assert(itemId(c.items[i].key) == i + 1); assert(!strcmp(itemName(i + 1), c.items[i].name)); paginate(itemDescription(i + 1));
  }
  for (int i = 0; i < c.factCount; i++) {
    const Named& fact = c.facts[i]; assert(factId(fact.key) == i); paginate(fact.description, JOURNAL_BOX);
    if (!strncmp(fact.key, "clue.", 5)) {
      assert(strcmp(fact.name, fact.key) && strcmp(fact.description, fact.key)); journalHeading("Known: ", fact.name);
    }
  }
  for (int i = 0; i < c.eventCount; i++) {
    assert(eventId(c.events[i].key) == i); assert(!strcmp(journalText({uint16_t(0x8000 | i), 1}), c.events[i].description));
    paginate(c.events[i].description, JOURNAL_BOX);
  }
  for (int i = 0; i < c.questCount; i++) {
    assert(!strcmp(questName(i), c.quests[i].name)); journalHeading("", c.quests[i].name);
    for (int state = UNDISCOVERED; state <= RESOLVED; state++) { saved.quests[i] = state; paginate(questObjective(saved, i), JOURNAL_BOX); }
  }
  for (int i = 0; i < c.skillCount; i++) assert(!strcmp(skillName(i), c.skills[i].name));
  for (int i = 0; i < c.npcCount; i++) assert(!strcmp(npcName(i), c.npcs[i].name));
  for (int i = 0; i < c.promiseCount; i++) { assert(!strcmp(promiseName(i), c.promises[i].name)); paginate(c.promises[i].description, JOURNAL_BOX); journalHeading("Promise: ", c.promises[i].name); }
}
void datedHeadings() {
  for (uint16_t day : {uint16_t(1), uint16_t(65535)}) {
    char date[64]; snprintf(date, sizeof date, "Our story, day %u", day); journalHeading("", date);
  }
}
void missingEntries() {
  assert(!sceneById(0) && !sceneById(65535) && !choiceById(0) && !choiceById(65535));
  assert(factId("absent") == -1 && itemId("absent") == -1 && eventId("absent") == -1);
  assert(!strcmp(itemName(0), "Unknown") && !strcmp(itemName(255), "Unknown"));
  assert(!strcmp(itemDescription(0), "An empty pocket."));
  assert(!strcmp(skillName(255), "Unknown") && !strcmp(npcName(255), "Unknown"));
  assert(!strcmp(promiseName(255), "Unknown") && !strcmp(questName(255), "Unknown"));
  assert(!strcmp(locationName(LOCATION_COUNT), "Unknown"));
  for (int i = 0; i < LOCATION_COUNT; i++) assert(*locationName(static_cast<Location>(i)));
  Save saved{}; saved.quests[0] = 255;
  assert(!strcmp(questObjective(saved, 0), "Explore the kingdom."));
  assert(!strcmp(questObjective(saved, 255), "Explore the kingdom."));
  assert(!strcmp(journalText({65535, 1}), "An earlier world event."));
  assert(!strcmp(journalText({32767, 1}), "An earlier decision remains part of our story."));
}
} // namespace
int main() { manuscript(); metadata(); datedHeadings(); missingEntries(); }
