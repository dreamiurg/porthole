// The save partition is small and the host cannot feel it: the sim's FileStore takes any number of bytes, the board's
// NVS (partitions.csv: nvs, 0x5000) does not. This adds up what the firmware keeps there at its fullest and fails
// when a save grows past what fits, which on a board is a write that quietly does not land (Shell::flushApp retries
// it, nobody is told, and the progress is gone at power-off).
//
// NVS arithmetic (ESP-IDF, blob format 2): a page is 4096 bytes = 126 entries of 32 bytes; one page is always kept
// free for garbage collection; a blob of n bytes takes ceil(n / 32) data entries, one chunk header and one index
// entry; a namespace takes one entry; rewriting a blob needs room for the new copy before the old one is freed.
//
// A game that keeps a save gets a line in GAMES below (there is no way to ask an App for its largest save).
#include "host/check.h"
#include <stdio.h>
#include "games/biscuit/pet.h"
#include "games/living-kingdom/save.h"
#include "games/marble-kick/save.h"
#include "games/pets-club/pet.h"
#include "games/plane-radar/save.h"
#include "games/racing/save.h"
#include "games/tilt-fc/save.h"
#include "games/times-cards/deck.h"
#include "games/toy-town-life/state.h"
#include "shell/pinlock.h"
#include "shell/profiles.h"
#include "shell/system.h"
#include "os/timezone.h"

constexpr int blob(size_t bytes) { return (int)((bytes + 31) / 32) + 2; }
constexpr int NVS_PAGES = 0x5000 / 4096, USABLE = (NVS_PAGES - 1) * 126;   // 504 entries

struct Kept { const char* what; size_t bytes; };
// One per profile, in the game's own namespace under s<id>.
constexpr Kept GAMES[] = {
  {"crago", sizeof(::Save)}, {"biscuit", sizeof(biscuit::Save)}, {"marblekick", sizeof(marble::Save)},
  {"tiltfc", sizeof(fc::Save)}, {"radar", sizeof(radar::Save)}, {"kingdom", sizeof(kingdom::Save)},
  {"toytown", sizeof(toytown::State)}, {"racing", sizeof(racing::Save)}, {"timescards", sizeof(cards::Save)},
};
// One per profile, kept by the shell: porthole/p<id> and porthole/f<id>.
constexpr Kept SHELL[] = {{"p", sizeof(shell::Record)}, {"f", sizeof(shell::Favorites)}};
// One per device. "sys": the shell's and firmware/net.cpp's keys at their largest; "porthole": the migration marks.
constexpr Kept DEVICE[] = {
  {"admin", 2}, {"games", sizeof(shell::GamesOff)}, {"limits", sizeof(shell::Limits)}, {"lite", 1},
  {"pinlock", sizeof(shell::PinLocks)}, {"ran", 15}, {"rtc_clock", sizeof(timezones::RtcJournal)}, {"sound", 1},
  {"utc_off", 2}, {"window", sizeof(shell::PlayWindow)}, {"chan", 1}, {"home", sizeof(shell::Home)}, {"last", 4},
  {"pass", 64}, {"ssid", 32}, {"tried", 15}, {"tz", sizeof(timezones::Settings)}, {"m", 1}, {"mb", 1},
};
constexpr int NAMESPACES = 14 + 1;   // "sys", "porthole", one per game in the image (a load creates it), the radio's "phy"
constexpr int RADIO = blob(1904) + blob(6) + 1;   // ESP-IDF's wifi calibration, written to "phy" on the first join

template <size_t N> constexpr int entries(const Kept (&list)[N]) { int n = 0; for (const Kept& k : list) n += blob(k.bytes); return n; }
template <size_t N> constexpr size_t largest(const Kept (&list)[N]) { size_t n = 0; for (const Kept& k : list) n = k.bytes > n ? k.bytes : n; return n; }

int main() {
  const int device = entries(DEVICE) + NAMESPACES + RADIO, profile = entries(GAMES) + entries(SHELL);
  const int rewrite = blob(largest(GAMES));
  const int fit = (USABLE - device - rewrite) / profile;
  for (const Kept& g : GAMES) {
    assert(g.bytes <= shell::BLOB_MAX);
    printf("  %-11s %4zu bytes, %2d entries\n", g.what, g.bytes, blob(g.bytes));
  }
  printf("nvs budget: %d entries; the device keeps %d, a profile that has played every game %d, a rewrite needs %d free\n",
         USABLE, device, profile, rewrite);
  printf("nvs budget: %d of %d such profiles fit (%d would need %d entries)\n", fit, MAX_PROFILES,
         MAX_PROFILES, device + MAX_PROFILES * profile + rewrite);
  // Two kids who have played everything must always fit. Four already do not (616 of 504 entries on 2026-11-15: the audit's
  // BUILD finding on NVS capacity); until the saves shrink or move, at least stop a profile's cost from growing.
  assert(fit >= 2);
  assert(profile <= 110);   // a ratchet: lower it when a save shrinks, never raise it
  puts("nvs budget: ok");
}
