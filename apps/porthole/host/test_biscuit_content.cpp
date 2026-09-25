// Self-check for Biscuit's content plumbing: personalize() and the generated art tables. Run: make test
// (Wording, glyphs and id order are checked by games/biscuit/tools/check_content.py; pixels by the art tool's --check.)
#include <assert.h>
#include <stdio.h>
#include <string.h>
#include "games/biscuit/content_daily.h"
#include "games/biscuit/content_discoveries.h"
#include "games/biscuit/content_stories.h"
#include "games/biscuit/generated/discovery_art.h"
#include "games/biscuit/generated/scenes.h"
#include "games/biscuit/personalize.h"
#include "games/biscuit/pet.h"

using namespace biscuit;

static void personalizing() {
  char out[64];
  assert(personalize("{pet} nudged {name}'s hand.", "Sam", "Pip", out, sizeof out) == 22);
  assert(strcmp(out, "Pip nudged Sam's hand.") == 0);
  personalize("Night, {name}. Love, {pet}", "", nullptr, out, sizeof out);   // no names yet
  assert(strcmp(out, "Night, friend. Love, Biscuit") == 0);
  personalize("{named} {pe} {}", "Sam", "Pip", out, sizeof out);             // only whole tokens are filled
  assert(strcmp(out, "{named} {pe} {}") == 0);
  assert(personalize("{name}{name}{name}", "Alexandra", "Pip", out, 10) == 9);  // cut, never overrun
  assert(strcmp(out, "Alexandra") == 0);
  assert(personalize("x", "Sam", "Pip", out, 0) == 0);
}

// Every scene is 160x160 and its runs cover exactly that; every picture exists. Catches a truncated art_data.inc.
static void art() {
  static_assert(sizeof DISCOVERIES / sizeof DISCOVERIES[0] == DISCOVERY_COUNT, "one picture per discovery");
  for (int i = 0; i < DISCOVERY_COUNT; i++)
    assert(DISCOVERY_ART[i].width == DISCOVERY_W && DISCOVERY_ART[i].height == DISCOVERY_H && DISCOVERY_ART[i].pixels);
  const gfx565::RleImage* scene = &SCENES[0][0][0][0];
  for (int i = 0; i < SCENE_STAGES * SCENE_TIMES * SCENE_ACTIVITIES * SCENE_FRAMES; i++, scene++) {
    uint32_t pixels = 0;
    for (uint32_t r = 0; r < scene->runCount; r++) pixels += scene->runs[r * 2];
    assert(scene->width == SCENE_W && scene->height == SCENE_H && pixels == SCENE_W * SCENE_H);
  }
}

int main() {
  personalizing();
  art();
  // The rules and the content count the same things (and share one namespace without clashing).
  static_assert(NUM_STORIES == STORY_COUNT && NUM_DISCOVERIES == DISCOVERY_COUNT && NUM_TRICKS == TRICK_COUNT &&
                NUM_STICKERS == STICKER_COUNT, "pet.h and the content agree");
  assert(STORY_COUNT == 7 && TOPIC_COUNT == 12 && ADVENTURE_COUNT == 7 && TRICK_COUNT == 6 && STICKER_COUNT == 12);
  printf("biscuit content: all checks passed\n");
  return 0;
}
