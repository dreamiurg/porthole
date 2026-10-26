// Self-check for Biscuit's content: personalize(), the generated art tables, and the pixel gate that every string
// fits the box its screen draws it in (games/biscuit/layout.h) and stays on the round glass. Run: make test
// (Glyphs, tokens, ids and order are checked by games/biscuit/tools/check_content.py; art pixels by the art tool's --check.)
#include <assert.h>
#include <ctype.h>
#include <stdio.h>
#include <string.h>
#include "crc32.h"
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

// Every scene and picture's runs cover exactly its dimensions. Catches a truncated art_data.inc.
static void art() {
  static_assert(sizeof DISCOVERIES / sizeof DISCOVERIES[0] == DISCOVERY_COUNT, "one picture per discovery");
  for (const gfx565::RleImage& picture : DISCOVERY_ART) {
    uint32_t pixels = 0;
    for (uint32_t r = 0; r < picture.runCount; r++) {
      assert(picture.runs[r * 2] > 0);
      pixels += picture.runs[r * 2];
    }
    assert(picture.width == DISCOVERY_W && picture.height == DISCOVERY_H && pixels == DISCOVERY_W * DISCOVERY_H);
  }
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

// Frozen from the original uncompressed RGB565 arrays, independently checked against all 96 legacy SHA-256
// manifest entries. Exercise the actual renderer at list (1x) and cover (3x) scales, including every expanded pixel.
static void discoveryPixels() {
  static const uint32_t originalCrc[DISCOVERY_COUNT] = {
    0x55E833B9u, 0xB70B05BEu, 0xABA73DFCu, 0x9C38F67Du, 0xF8AAA38Au, 0x2E452652u, 0x43B42E32u, 0x409B2EC5u,
    0xC9C452DCu, 0xAABD9A75u, 0xA7B178E0u, 0xBAE496B5u, 0x7B410885u, 0xB8278F7Au, 0x0FE7961Cu, 0x0847180Fu,
    0x3AFC42DCu, 0xC9AF002Du, 0x2111C491u, 0x85A5828Bu, 0x2A8E4D3Fu, 0xA56A0D31u, 0xF1AB34FBu, 0xB676D381u,
    0x532D971Eu, 0x9D9AD7CFu, 0x3CFBF904u, 0xD0CC101Fu, 0x20E892E1u, 0xFA0203A4u, 0x68FAB573u, 0x465520E8u,
    0x82940995u, 0xCEA1B340u, 0x15D1950Eu, 0xC6B741B7u, 0x4A9FE9C2u, 0x4ECCAC1Du, 0x3C758927u, 0x3FAC9274u,
    0xDB2ED57Bu, 0x13F10A1Au, 0x275FD886u, 0x376E6451u, 0x2D2B329Au, 0xFF96F5D1u, 0xC82BDC45u, 0x5AC09FD6u,
    0x6089CD06u, 0xB8188308u, 0x81F56363u, 0x8DB7693Au, 0x2F9819F1u, 0xF769A39Eu, 0x43137780u, 0xC7AE16C4u,
    0x217E62F3u, 0xE0D4E423u, 0x7AE4DD86u, 0x6637918Au, 0x5446039Du, 0xF239B091u, 0xE41DBBD7u, 0x7D4BCDFFu,
    0x31E65B75u, 0xE3700259u, 0x938288CEu, 0x001E035Au, 0xB31CA9EAu, 0x5A4352B1u, 0x88C8C98Fu, 0x295C6371u,
    0xBA22DF99u, 0x10C019EEu, 0xD1EC3810u, 0x73F3B85Du, 0xB02C5FEDu, 0x7BB6B037u, 0x7E0695D2u, 0xE79E0BB6u,
    0x47831148u, 0xF1E9ACE6u, 0x8D5AB928u, 0xC8B73808u, 0xED474844u, 0x9B665676u, 0x7810B52Fu, 0xE572F0B2u,
    0x9F041C4Du, 0x5FA728B5u, 0x77FB5257u, 0x11756D67u, 0xCE8547E0u, 0x912C0F07u, 0xA906F4F8u, 0x88AF18E8u,
  };
  uint16_t original[DISCOVERY_W * DISCOVERY_H];
  gfx565::target(g_fb);
  for (int i = 0; i < DISCOVERY_COUNT; i++) {
    gfx565::clear(0);
    gfx565::blitRle(DISCOVERY_ART[i], 0, 0, 1);
    for (int y = 0; y < DISCOVERY_H; y++)
      memcpy(original + y * DISCOVERY_W, g_fb + y * gfx565::W, DISCOVERY_W * sizeof(uint16_t));
    assert(os::crc32(original, sizeof original) == originalCrc[i]);
    gfx565::clear(0);
    gfx565::blitRle(DISCOVERY_ART[i], COVER_X, COVER_Y, 3);
    for (int y = 0; y < DISCOVERY_H * 3; y++)
      for (int x = 0; x < DISCOVERY_W * 3; x++)
        assert(g_fb[(COVER_Y + y) * gfx565::W + COVER_X + x] == original[(y / 3) * DISCOVERY_W + x / 3]);
  }
}

static const char* filled(const char* s) {   // the screens fill into 256 bytes: never cut there
  static char out[1024];
  const size_t n = personalize(s, NAME, PET, out, sizeof out);
  if (n >= 256) { printf("content gate: %zu bytes filled, the screens hold 255: \"%s\"\n", n, out); failures++; }
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
// A story up to its choice, an ending or a discovery, filled and joined with a space as Game::openPages does, in at
// most READ_BYTES and READ_SCREENS screens of PAGE, at the widest names and at short ones. The flow fills every screen
// but the last until the next word no longer fits, and pageBreaks gives the last one MIN_LAST_WORDS words when the
// screen before can spare them, so only the last screen can be short: it may not be.
static int words(const char* s) {   // pageBreaks leaves single spaces between words
  int n = *s != 0;
  for (; *s; s++) n += *s == ' ';
  return n;
}
static void stream(const char* where, const char* const* pages, int count, const char* name, const char* pet) {
  static char buf[4 * READ_BYTES];
  size_t n = 0;
  for (int i = 0; i < count && n + 1 < sizeof buf; i++) {
    if (i) buf[n++] = ' ';
    n += personalize(pages[i], name, pet, buf + n, sizeof buf - n);
  }
  if (n >= READ_BYTES) { printf("content gate: %s fills %zu bytes, the reader holds %d\n", where, n, READ_BYTES - 1); failures++; return; }
  const char* screens[READ_SCREENS];
  const int k = font::pageBreaks(PAGE.box, buf, screens, READ_SCREENS, MIN_LAST_WORDS);
  if (k > READ_SCREENS) { printf("content gate: %s takes %d screens, the reader holds %d\n", where, k, READ_SCREENS); failures++; return; }
  for (int i = 0; i < k; i++) fits(PAGE, where, screens[i]);
  if (words(screens[k - 1]) < MIN_LAST_WORDS) {
    printf("content gate: %s ends on a %d-word screen: \"%s\"\n", where, words(screens[k - 1]), screens[k - 1]);
    failures++;
  }
}
static void stream(const char* where, const char* const* pages, int count) {
  char shortNames[80];
  snprintf(shortNames, sizeof shortNames, "%s (Sam / Pip)", where);
  stream(where, pages, count, NAME, PET);
  stream(shortNames, pages, count, "Sam", "Pip");
}
static void storyGate() {
  char where[64], text[128];
  for (const Story& st : STORIES) {
    snprintf(text, sizeof text, "Day %d: %s", st.unlockDay, st.title);
    fits(STORY_BUTTON, st.id, text);
    snprintf(text, sizeof text, "%s *", st.title);
    fits(STORY_BUTTON, st.id, text);
    fits(SUBTITLE, st.id, st.subtitle);
    snprintf(where, sizeof where, "%s story", st.id);
    stream(where, st.pages, STORY_PAGES);
    snprintf(where, sizeof where, "%s prompt", st.id);
    fits(PROMPT, where, filled(st.prompt));
    for (const StoryChoice& c : st.choices) {
      fits(CHOICE_BUTTON, st.id, c.label);
      snprintf(where, sizeof where, "%s '%s'", st.id, c.label);
      stream(where, c.ending, ENDING_PAGES);
    }
  }
}
static void discoveryGate() {
  char where[64];
  for (const Topic& t : TOPICS) { fits(HEADER, t.id, t.name); fits(rowText(t.name), t.id, t.name); }
  for (const Discovery& d : DISCOVERIES) {
    fits(coverTitle(d.title), d.id, d.title);
    fits(rowText(d.title), d.id, d.title);
    stream(d.id, d.pages, 2);
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
    fits(ROW_LABEL, t.id, text);
    snprintf(text, sizeof text, "%s   3/3", t.name);
    fits(ROW_LABEL, t.id, text);
  }
  Label album = STICKER;   // the lowest of the album's rows
  album.y = (int16_t)(album.y + (STICKER_ROWS - 1) * STICKER_STEP);
  for (const char* s : STICKERS) fits(album, s, s);
  fits(album, "album", "A little surprise awaits");
  for (int bit = 0; bit < 6; bit++) {   // Today's activities, once done too
    snprintf(text, sizeof text, "* %s", ACTIVITIES[bit]);
    fits(TODAY_BUTTON, ACTIVITIES[bit], text);
  }
}
// The pup's own lines and the screens' status text, at the widest names and the biggest counts a save allows.
static void homeGate() {
  char text[160];
  const unsigned big = MAX_COUNT, stars = NUM_STORIES * 3;
  for (int i = 0; i < SAY_COUNT; i++) { snprintf(text, sizeof text, "SAY[%d]", i); fits(BUBBLE, text, filled(SAY[i])); }
  for (const char* stage : STAGES) {
    snprintf(text, sizeof text, "Day %u \xC2\xB7 %s", big, stage);             // Home's mood (a lowercase first letter)
    fits(HOME_MOOD, stage, text);
    snprintf(text, sizeof text, "Day %u \xC2\xB7 %s \xC2\xB7 %u stars", big, stage, stars);   // World
    fits(WORLD_LINE, stage, text);
    snprintf(text, sizeof text, "%s - Cuddlebug", stage);                        // the scrapbook's badge
    fits(BOOK_BADGE, stage, text);
  }
  fits(HOME_MOOD, "asleep", "dreaming of stories");
  static const char* const ACTION_LABELS[4][2] = {{"Feed", nullptr}, {"Play", nullptr}, {"Read", nullptr}, {"More", "Wake"}};
  for (int i = 0; i < 4; i++)
    for (const char* s : ACTION_LABELS[i]) if (s) fits(actionLabel(ACTIONS[i], &FONT20), "action", s);
  fits(WORLD_NAMES, "pet alone", PET);   // World shows "{pet} & {name}" only when it fits, else the pup's name
  snprintf(text, sizeof text, "{name} & {pet}\nDay %u together\n%u friendship\n%u story stars", big, big, stars);
  fits(BOOK_LINES, "scrapbook", filled(text));
}
// The words the new screens compose around the content: titles, button labels, counters, World's tiles.
static Label inTile(const Label& l, const Box& tile) {
  return {l.box, (int16_t)(tile.x * 3 + l.x), (int16_t)(tile.y * 3 + l.y), l.middle};
}
static void screensGate() {
  static const char* const TITLES[] = {"Our bookshelf", "Story time", "The story continues", "What shall we do?",
                                       "Little discoveries", "Our little notebook", "So much to explore",
                                       "Where we found it", "Our sticker album", "Our scrapbook",
                                       "Little paws, big ideas"};
  for (const char* t : TITLES) fits(HEADER, "title", t);
  static const char* const PAIRS[] = {"Discoveries", "Notebook", "Topics", "Today's three"};
  for (const char* t : PAIRS) fits(PAIR_LABEL, "pair", t);
  static const char* const NAVS[] = {"Previous", "Next", "Choose", "The end", "Source", "Keep", "Our stickers", "Rename pup"};
  for (const char* t : NAVS) { fits(buttonLabel(PREV, &FONT20), "prev", t); fits(buttonLabel(NEXT, &FONT20), "next", t); }
  static const char* const WIDES[] = {"Let's find out", "Back to our book", "Back to today", "My turn", "Peek again"};
  for (const char* t : WIDES) fits(buttonLabel(WIDE, &FONT20), "wide", t);
  fits(TODAY_BUTTON, "word", "A lovely word");
  fits(WONDER_TITLE, "wonder", "I wonder...");
  fits(NOTEBOOK_EMPTY, "notebook", "A place for all the things we find together.");
  static_assert(READ_SCREENS <= MAX_PAGES && NUM_DISCOVERIES / 2 <= MAX_PAGES, "the counters' widest");
  char s[16];
  for (int count = 1; count <= MAX_PAGES; count++)
    for (int page = 0; page < count; page++) {
      pageNumber(s, page, count);
      if (font::textWidth(*PAGE_NUMBER.box.font, s) > PAGE_NUMBER.box.w) { printf("content gate: counter \"%s\" is too wide\n", s); failures++; }
    }
  pageNumber(s, MAX_PAGES - 1, MAX_PAGES);
  fits(PAGE_NUMBER, "counter", s);
  const Box tiles[] = {WORLD_TRICKS, WORLD_NAP, WORLD_BOOK, WORLD_TODAY};
  const char* const tileTitles[4][2] = {{"Learn tricks", nullptr}, {"Cozy nap", "Wake up"}, {"Our scrapbook", nullptr},
                                        {"Today's adventure", nullptr}};
  const char* const tileDetails[4][2] = {{"6 of 6 mastered", nullptr}, {"A lovely place to pause", nullptr},
                                         {"Growing up, page by page", nullptr}, {"Something to discover", "Sticker earned!"}};
  for (int i = 0; i < 4; i++)
    for (int j = 0; j < 2; j++) {
      if (tileTitles[i][j]) fits(inTile(TILE_TITLE, tiles[i]), "tile", tileTitles[i][j]);
      if (tileDetails[i][j]) fits(inTile(TILE_DETAIL, tiles[i]), "tile", tileDetails[i][j]);
    }
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
  homeGate();
  screensGate();
  if (failures) printf("content gate: %d strings do not fit (widest names: {name} %s, {pet} %s)\n", failures, NAME, PET);
  assert(failures == 0);
}

int main() {
  personalizing();
  art();
  discoveryPixels();
  pixelGate();
  // The rules and the content count the same things (and share one namespace without clashing).
  static_assert(NUM_STORIES == STORY_COUNT && NUM_DISCOVERIES == DISCOVERY_COUNT && NUM_TRICKS == TRICK_COUNT &&
                NUM_STICKERS == STICKER_COUNT, "pet.h and the content agree");
  assert(STORY_COUNT == 7 && TOPIC_COUNT == 12 && ADVENTURE_COUNT == 7 && TRICK_COUNT == 6 && STICKER_COUNT == 12);
  printf("biscuit content: all checks passed\n");
  return 0;
}
