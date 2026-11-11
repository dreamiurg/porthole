// games/enabled.h drives APPS: each game is in it exactly when its switch is on, in launcher order, and test_apps_all
// (the same checks built with -DPORTHOLE_ALL_GAMES, as make playtest's sims are) has every game. ALL_STORES names
// every game's save namespace either way, so a profile delete reaches a switched-off game's saves too. Everything
// expected comes from the switches: flipping one needs no edit here.
// Run: make test
#include "host/check.h"
#include <stdio.h>
#include <string.h>
#include "games/apps.h"

static int checks = 0;
static bool shipped(const char* store) {
  for (App* a : APPS) if (!strcmp(a->store(), store)) return true;
  return false;
}
// Every game's switch beside its namespace, in ALL_STORES' order.
static const struct { const char* store; bool on; } SWITCHES[] = {
    {"crago", PORTHOLE_GAME_PETS_CLUB}, {biscuit::STORE, PORTHOLE_GAME_BISCUIT}, {marble::STORE, PORTHOLE_GAME_MARBLE_KICK},
    {fc::STORE, PORTHOLE_GAME_TILT_FC}, {"pumpkin", PORTHOLE_GAME_PUMPKIN}, {"radar", PORTHOLE_GAME_RADAR},
    {"starfighter", PORTHOLE_GAME_STARFIGHTER}, {"kingdom", PORTHOLE_GAME_KINGDOM}, {"toytown", PORTHOLE_GAME_TOY_TOWN},
    {"racing", PORTHOLE_GAME_RACING}};
static constexpr int GAMES = sizeof SWITCHES / sizeof SWITCHES[0];

int main() {
  int stores = 0;
  while (ALL_STORES[stores]) stores++;
  CHECK(stores == GAMES);   // a new game gets its namespace in ALL_STORES and its switch here
  int on = 0;
  for (int i = 0; i < GAMES; i++) {
    CHECK(!strcmp(SWITCHES[i].store, ALL_STORES[i]));
    CHECK(shipped(SWITCHES[i].store) == SWITCHES[i].on);
    on += SWITCHES[i].on;
    for (int j = 0; j < i; j++) CHECK(strcmp(ALL_STORES[i], ALL_STORES[j]));
  }
  CHECK(N_APPS == on);
  int k = 0;   // APPS is ALL_STORES' order with the switched-off games left out
  for (int i = 0; i < stores && k < N_APPS; i++) if (!strcmp(APPS[k]->store(), ALL_STORES[i])) k++;
  CHECK(k == N_APPS);
  if (PORTHOLE_ALL_GAMES) CHECK(on == GAMES);
  printf("%s: %d checks passed (%d of %d games on)\n", PORTHOLE_ALL_GAMES ? "test_apps_all" : "test_apps", checks, on, GAMES);
  return 0;
}
