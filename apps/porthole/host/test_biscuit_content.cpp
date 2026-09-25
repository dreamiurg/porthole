// Self-check for Biscuit's content: personalize(), the generated art tables, and the pixel gate that every string
// fits the box its screen draws it in (games/biscuit/layout.h) and stays on the round glass. Run: make test
// (Glyphs, tokens, ids and order are checked by games/biscuit/tools/check_content.py; art pixels by the art tool's --check.)
#include <assert.h>
#include <ctype.h>
#include <stdio.h>
#include <string.h>
#include "games/biscuit/content_daily.h"
#include "games/biscuit/content_discoveries.h"
#include "games/biscuit/content_stories.h"
#include "games/biscuit/generated/discovery_art.h"
#include "games/biscuit/generated/scenes.h"
#include "games/biscuit/layout.h"
#include "games/biscuit/personalize.h"
#include "games/biscuit/pet.h"
#include "gfx565.h"

using namespace biscuit;

static void personalizing() {
  char out[64];
  assert(personalize("{pet} nudged {name}'s hand.", "Sam", "Pip", out, sizeof out) == 22);
  assert(strcmp(out, "Pip nudged Sam's hand.") == 0);
  personalize("Night, {name}. Love, {pet}", "", nullptr, out, sizeof out);   // no names yet
  assert(strcmp(out, "Night, Friend. Love, Biscuit") == 0);
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

// ---- the pixel gate: os/font.cpp measures and draws each string exactly as the device will ----
// Tokens are filled with the widest names a kid can end up with: 11 letters for the profile name (its record holds
// 11), NAME_LIMIT for the dog's, every letter a W. Any case is possible (a migrated Biscuit save keeps its names as
// typed), W is the widest letter in every Biscuit font (checked below), and letters never kern in them, so no name
// of that length is wider. Greedy wrapping never gets shorter when a word gets wider, so this is the worst case.
static const char NAME[] = "WWWWWWWWWWW", PET[] = "WWWWWWWWWWWW";
static_assert(sizeof NAME == 12 && sizeof PET == NAME_LIMIT + 1, "the longest names");
static uint16_t g_fb[gfx565::W * gfx565::H];
static int failures = 0;

static const char* filled(const char* s) {
  static char out[1024];
  assert(personalize(s, NAME, PET, out, sizeof out) + 1 < sizeof out);   // never cut
  return out;
}
// No taller than l.box.h, and every pixel textBox paints (it clips like the device's label) is on the glass.
static void fits(const Label& l, const char* where, const char* s) {
  const font::Box& b = l.box;
  int h = font::textHeight(*b.font, s, b.w, b.spacing), y = l.middle ? l.y + (b.h - h) / 2 : l.y;
  if (h > b.h) { printf("content gate: %s is %d px tall, its box %d: \"%s\"\n", where, h, b.h, s); failures++; return; }
  gfx565::clear(0);
  font::textBox(b, s, l.x, y, 0xFFFF);
  for (int py = 0; py < gfx565::H; py++)
    for (int px = 0; px < gfx565::W; px++)
      if (gfx565::fb[py * gfx565::W + px] && !gfx565::inCircle(px, py)) {
        printf("content gate: %s is cut by the round edge at (%d, %d): \"%s\"\n", where, px, py, s);
        failures++;
        return;
      }
}
static void page(const char* where, const char* s) {   // filled, in at most PAGE_SCREENS screens
  char buf[1024];
  snprintf(buf, sizeof buf, "%s", filled(s));
  const char* screens[PAGE_SCREENS];
  int n = font::pageBreaks(PAGE.box, buf, screens, PAGE_SCREENS);
  if (n > PAGE_SCREENS) { printf("content gate: %s takes %d screens: \"%s\"\n", where, n, filled(s)); failures++; return; }
  for (int i = 0; i < n; i++) fits(PAGE, where, screens[i]);
}
static void storyGate() {
  char where[64], text[128];
  for (const Story& st : STORIES) {
    snprintf(text, sizeof text, "Day %d: %s", st.unlockDay, st.title);
    fits(STORY_BUTTON, st.id, text);
    snprintf(text, sizeof text, "%s *", st.title);
    fits(STORY_BUTTON, st.id, text);
    fits(SUBTITLE, st.id, st.subtitle);
    for (int i = 0; i < STORY_PAGES; i++) { snprintf(where, sizeof where, "%s page %d", st.id, i + 1); page(where, st.pages[i]); }
    snprintf(where, sizeof where, "%s prompt", st.id);
    fits(PROMPT, where, filled(st.prompt));
    for (const StoryChoice& c : st.choices) {
      fits(CHOICE_BUTTON, st.id, c.label);
      for (int i = 0; i < ENDING_PAGES; i++) {
        snprintf(where, sizeof where, "%s '%s' page %d", st.id, c.label, i + 1);
        page(where, c.ending[i]);
      }
    }
  }
}
static void discoveryGate() {
  char where[64];
  for (const Topic& t : TOPICS) { fits(HEADER, t.id, t.name); fits(TOPIC_BUTTON, t.id, t.name); }
  for (const Discovery& d : DISCOVERIES) {
    fits(FACT_TITLE, d.id, d.title);
    fits(FACT_BUTTON, d.id, d.title);
    for (int i = 0; i < 2; i++) { snprintf(where, sizeof where, "%s page %d", d.id, i + 1); page(where, d.pages[i]); }
    snprintf(where, sizeof where, "%s wonder", d.id);
    fits(WONDER, where, filled(d.wonder));
    fits(SOURCE_NAME, d.id, d.sourceName);
    fits(SOURCE_URL, d.id, d.sourceUrl);
  }
}
static void dailyGate() {
  char text[64];
  for (const Adventure& a : ADVENTURES) {
    fits(HEADER, a.title, a.title);
    fits(ADVENTURE, a.title, filled(a.description));
    fits(HEADER, a.title, a.word);
    fits(MEANING, a.word, filled(a.meaning));
  }
  for (uint8_t i = 0; i < TRICK_COUNT; i++) {
    const Trick& t = TRICKS[i];
    fits(HEADER, t.id, t.name);
    snprintf(text, sizeof text, "%s   Day %d", t.name, trickUnlockDay(i));
    fits(TRICK_BUTTON, t.id, text);
    snprintf(text, sizeof text, "%s   3/3", t.name);
    fits(TRICK_BUTTON, t.id, text);
  }
  for (const char* s : STICKERS) fits(STICKER, s, s);
}
static void pixelGate() {
  const font::Font* F[4] = {&FONT16, &FONT20, &FONT24, &FONT28};
  for (const font::Font* f : F)
    for (char c = 'A'; c <= 'z'; c++) {   // the names above are the widest: W is the widest letter, letters never kern
      const char s[2] = {c, 0};
      int g = font::glyphIndex(*f, (uint32_t)c);
      assert(!isalpha(c) || (font::textWidth(*f, s) <= font::textWidth(*f, "W") && !f->kernLeft[g] && !f->kernRight[g]));
    }
  gfx565::target(g_fb);
  storyGate();
  discoveryGate();
  dailyGate();
  if (failures) printf("content gate: %d strings do not fit (widest names: {name} %s, {pet} %s)\n", failures, NAME, PET);
  assert(failures == 0);
}

int main() {
  personalizing();
  art();
  pixelGate();
  // The rules and the content count the same things (and share one namespace without clashing).
  static_assert(NUM_STORIES == STORY_COUNT && NUM_DISCOVERIES == DISCOVERY_COUNT && NUM_TRICKS == TRICK_COUNT &&
                NUM_STICKERS == STICKER_COUNT, "pet.h and the content agree");
  assert(STORY_COUNT == 7 && TOPIC_COUNT == 12 && ADVENTURE_COUNT == 7 && TRICK_COUNT == 6 && STICKER_COUNT == 12);
  printf("biscuit content: all checks passed\n");
  return 0;
}
