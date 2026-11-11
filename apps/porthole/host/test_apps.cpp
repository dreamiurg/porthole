// games/enabled.h drives APPS: the default build ships every game but Biscuit, and test_apps_all (the same checks
// built with -DPORTHOLE_ALL_GAMES, as make playtest's sims are) has every game in launcher order. ALL_STORES names every
// game's save namespace either way, so a profile delete reaches a switched-off game's saves too.
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
int main() {
  int stores = 0;
  while (ALL_STORES[stores]) stores++;
  for (int i = 0; i < stores; i++) for (int j = 0; j < i; j++) CHECK(strcmp(ALL_STORES[i], ALL_STORES[j]));
  int k = 0;   // APPS is ALL_STORES' order with the switched-off games left out
  for (int i = 0; i < stores && k < N_APPS; i++) if (!strcmp(APPS[k]->store(), ALL_STORES[i])) k++;
  CHECK(k == N_APPS);
  CHECK(shipped(biscuit::STORE) == (bool)PORTHOLE_GAME_BISCUIT);
  CHECK(shipped("crago") && shipped("racing"));
#if PORTHOLE_ALL_GAMES
  CHECK(N_APPS == stores);   // so ALL_STORES misses no game
#else
  CHECK(N_APPS == stores - 1 && !shipped(biscuit::STORE));
#endif
  printf("%s: %d checks passed (%d games)\n", PORTHOLE_ALL_GAMES ? "test_apps_all" : "test_apps", checks, N_APPS);
  return 0;
}
