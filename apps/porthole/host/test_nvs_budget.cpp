// The save partition is small and the host cannot feel it: the sim's FileStore takes any number of bytes, the board's
// NVS (partitions.csv: nvs, 0x5000) does not. This adds up what the firmware keeps there at its fullest and fails
// unless a whole household fits: MAX_PROFILES kids who have each played every game to its end, the device's own keys,
// and room to rewrite the largest save. On a board a save past that is a write that does not land (Shell::flushApp
// backs off and Info tells the grown-up, but the progress is gone at power-off).
//
// NVS arithmetic (ESP-IDF, blob format 2): a page is 4096 bytes = 126 entries of 32 bytes; one page is always kept
// free for garbage collection; a blob of n bytes takes ceil(n / 32) data entries, one chunk header and one index
// entry; a namespace takes one entry; rewriting a blob needs room for the new copy before the old one is freed.
//
// A game that keeps a save gets a line in main()'s `games` (there is no way to ask an App for its largest save). A
// fixed struct counts at its size. Kingdom and Times Cards store theirs bit-packed, so their size depends on what was
// played: each is measured here with the game's own packer on the fullest save play can make (kingdomFull, cardsFull).
#include "host/check.h"
#include <stdio.h>
#include "games/biscuit/pet.h"
#include "games/living-kingdom/content.h"
#include "games/living-kingdom/save.h"
#include "games/marble-kick/save.h"
#include "games/pets-club/pet.h"
#include "games/plane-radar/save.h"
#include "games/racing/save.h"
#include "games/tilt-fc/save.h"
#include "games/times-cards/deck.h"
#include "games/toy-town-life/state.h"
#include "host/fixtures/kingdom_save_v1.h"
#include "host/fixtures/times_cards_save_v3.h"
#include "shell/pinlock.h"
#include "shell/profiles.h"
#include "shell/system.h"
#include "os/telemetry.h"
#include "os/timezone.h"

constexpr int blob(size_t bytes) { return (int)((bytes + 31) / 32) + 2; }
constexpr int NVS_PAGES = 0x5000 / 4096, USABLE = (NVS_PAGES - 1) * 126;   // 504 entries

struct Kept { const char* what; size_t bytes; };

// Kingdom's fullest campaign on this content, however it was played: every scene visited, every promise dated, every
// event scheduled, and the longest journal there can be (an entry per event, one decision per once-only scene, every
// choice of a scene that comes back), all on the last day a campaign can count to. No campaign packs larger.
static size_t kingdomFull() {
  using namespace kingdom;
  const Content& c = content();
  kingdom::Save s{};
  s.day = 0xFFFF;
  auto note = [&s](uint16_t id) { assert(s.journalCount < MAX_JOURNAL); s.journal[s.journalCount++] = {id, s.day}; };
  for (int i = 0; i < c.eventCount; i++) { s.eventDue[i] = 0xFFFFFFFFu; note((uint16_t)(0x8000 | i)); }
  for (int i = 0; i < c.promiseCount; i++) s.promiseDays[i] = s.day;
  for (int i = 0; i < c.sceneCount; i++) {
    const Scene& scene = c.scenes[i];
    s.lastVisit[scene.id] = s.day; s.seen[scene.id / 8] |= (uint8_t)(1u << scene.id % 8);
    uint16_t widest = 0;   // a once-only scene is decided once: count the choice with the largest id
    for (int k = 0; k < scene.choiceCount; k++) widest = scene.choices[k].id > widest ? scene.choices[k].id : widest;
    for (int k = 0; k < scene.choiceCount; k++) {
      const uint16_t id = scene.choices[k].id;
      if (scene.once && id != widest) continue;
      note(id); s.decisions[id / 8] |= (uint8_t)(1u << id % 8);
    }
  }
  uint8_t blob[sizeof(kingdom::Save)], shipped[sizeof(kingdom::Save)];
  const size_t full = packSave(s, blob, sizeof blob);
  kingdom::Save real;   // a sanity check on the bound: the real finished campaign in the fixture is well inside it
  assert(loadSave(real, KINGDOM_SAVE_V1, sizeof KINGDOM_SAVE_V1, 2));
  const size_t finished = packSave(real, shipped, sizeof shipped);
  assert(full && finished && finished < full);
  printf("  kingdom: %zu bytes at its fullest (%d journal entries); the fixture's finished chapter %zu; whole struct %zu\n",
         full, s.journalCount, finished, sizeof(kingdom::Save));
  return full;
}
// Times Cards' fullest deck: every card met, none on its box's own wait (the short form), and the deck's first and last
// day eleven years apart (12 bits a day; a kid's deck spans the weeks between its oldest card and its newest). A deck
// made while the clock only ran forward never costs more a card (deck.h packCard: its due day is within nine weeks of
// the day it was seen). One whose clock was set back years can (up to about 1000 bytes): it still saves, as the whole
// household's budget allows.
static size_t cardsFull() {
  using namespace cards;
  cards::Save s{};
  s.level = TABLES; s.start = 4; s.answers = 100000;
  for (int c = 0; c < ALL; c++) {
    s.boxOf(c) = MAX_BOX; s.seenOf(c) = (uint16_t)(20000 + c * 4000 / (ALL - 1)); s.dueOf(c) = (uint16_t)(s.seenOf(c) + 2);
  }
  s.logMu = -1; s.logVar = 0xFFFF; s.step = STEP_MAX; s.streak = 0xFF; s.jumps = s.tries = s.rights = s.probeMiss = 0xFFFF;
  uint8_t blob[sizeof(cards::Save)], shipped[sizeof(cards::Save)];
  const size_t full = pack(s, blob, sizeof blob);
  cards::Save real;   // the same sanity check: the real four-month deck in the fixture is well inside the bound
  assert(loadBlob(TIMES_CARDS_SAVE_V3, sizeof TIMES_CARDS_SAVE_V3, real));
  const size_t played = pack(real, shipped, sizeof shipped);
  assert(full && played && played < full);
  printf("  timescards: %zu bytes at its fullest; the fixture's four-month deck %zu; whole struct %zu\n", full, played,
         sizeof(cards::Save));
  return full;
}

// One per profile, kept by the shell: porthole/p<id>, and porthole/f<id> (24 B), the favorites an older firmware
// wrote: nothing reads it now, but an updated device keeps it until the profile is deleted.
constexpr Kept SHELL[] = {{"p", sizeof(shell::Record)}, {"f", 24}};
// One per device. "sys": the shell's and firmware/net.cpp's keys at their largest; "porthole": the migration marks.
constexpr Kept DEVICE[] = {
  {"admin", 2}, {"games", sizeof(shell::GamesOff)}, {"limits", sizeof(shell::Limits)}, {"lite", 1},
  {"pinlock", sizeof(shell::PinLocks)}, {"ran", 15}, {"rtc_clock", sizeof(timezones::RtcJournal)}, {"sound", 1},
  {"utc_off", 2}, {"window", sizeof(shell::PlayWindow)}, {"chan", 1}, {"home", sizeof(shell::Home)}, {"last", 4},
  {"pass", 64}, {"ssid", 32}, {"tried", 15}, {"tz", sizeof(timezones::Settings)}, {"telem", sizeof(telemetry::Config)},
  {"m", 1}, {"mb", 1},
};
constexpr int NAMESPACES = 14 + 1;   // "sys", "porthole", one per game in the image (a load creates it), the radio's "phy"
constexpr int RADIO = blob(1904) + blob(6) + 1;   // ESP-IDF's wifi calibration, written to "phy" on the first join

template <size_t N> constexpr int entries(const Kept (&list)[N]) { int n = 0; for (const Kept& k : list) n += blob(k.bytes); return n; }
template <size_t N> constexpr size_t largest(const Kept (&list)[N]) { size_t n = 0; for (const Kept& k : list) n = k.bytes > n ? k.bytes : n; return n; }

int main() {
  // One per profile, in the game's own namespace under s<id>.
  const Kept games[] = {
    {"crago", sizeof(::Save)}, {"biscuit", sizeof(biscuit::Save)}, {"marblekick", sizeof(marble::Save)},
    {"tiltfc", sizeof(fc::Save)}, {"radar", sizeof(radar::Save)}, {"kingdom", kingdomFull()},
    {"toytown", sizeof(toytown::State)}, {"racing", sizeof(racing::Save)}, {"timescards", cardsFull()},
  };
  const int device = entries(DEVICE) + NAMESPACES + RADIO, profile = entries(games) + entries(SHELL);
  const int rewrite = blob(largest(games));
  const int need = device + MAX_PROFILES * profile + rewrite;
  for (const Kept& g : games) {
    assert(g.bytes <= shell::BLOB_MAX);
    printf("  %-11s %4zu bytes, %2d entries\n", g.what, g.bytes, blob(g.bytes));
  }
  printf("nvs budget: %d entries; the device keeps %d, a profile that has played every game %d, a rewrite needs %d free\n",
         USABLE, device, profile, rewrite);
  printf("nvs budget: %d such profiles need %d entries, %d to spare\n", MAX_PROFILES, need, USABLE - need);
  // The whole household must fit. Until 2026-11 it did not: Kingdom (935 bytes) and Times Cards (1020) stored their
  // structs whole, a profile cost 110 entries and four needed 616 of the 504, so two kids fit (the audit's PLAT-1,
  // SHELL-5, BUILD-1, LIFE-11). A new save, or one that grows, has to fit in what is left to spare.
  assert(need <= USABLE);
  assert(profile <= 80);   // a ratchet: lower it when a save shrinks, never raise it
  puts("nvs budget: ok");
}
