// Every game in the image, in launcher order: the one table the firmware (firmware/main.cpp) and the host sim
// (host/sim.cpp) both run. Include it from exactly one file per build. Append a new game here, and its switch to
// games/enabled.h. A switched-off game has no instance, so the linker drops it from the image.
#pragma once
#include "games/enabled.h"
#include "games/biscuit/game.h"
#include "games/living-kingdom/game.h"
#include "games/marble-kick/game.h"
#include "games/pets-club/game.h"
#include "games/plane-radar/game.h"
#include "games/pumpkin-hide-seek/game.h"
#include "games/starfighter/game.h"
#include "games/tilt-fc/game.h"
#include "games/toy-town-life/game.h"
#include "games/racing/game.h"
#include "shell.h"

#if PORTHOLE_GAME_PETS_CLUB
static Game g_pets;
#endif
#if PORTHOLE_GAME_BISCUIT
static biscuit::Game g_biscuit;
#endif
#if PORTHOLE_GAME_MARBLE_KICK
static marble::Game g_marble;
#endif
#if PORTHOLE_GAME_TILT_FC
static fc::Game g_tiltfc;
#endif
#if PORTHOLE_GAME_PUMPKIN
static pumpkin::Game g_pumpkin;
#endif
#if PORTHOLE_GAME_RADAR
static radar::Game g_radar;
#endif
#if PORTHOLE_GAME_STARFIGHTER
static starfighter::Game g_starfighter;
#endif
#if PORTHOLE_GAME_KINGDOM
static kingdom::Game g_kingdom;
#endif
#if PORTHOLE_GAME_TOY_TOWN
static toytown::Game g_toytown;
#endif
#if PORTHOLE_GAME_RACING
static racing::Game g_racing;
#endif
static App* const APPS[] = {
#if PORTHOLE_GAME_PETS_CLUB
    &g_pets,
#endif
#if PORTHOLE_GAME_BISCUIT
    &g_biscuit,
#endif
#if PORTHOLE_GAME_MARBLE_KICK
    &g_marble,
#endif
#if PORTHOLE_GAME_TILT_FC
    &g_tiltfc,
#endif
#if PORTHOLE_GAME_PUMPKIN
    &g_pumpkin,
#endif
#if PORTHOLE_GAME_RADAR
    &g_radar,
#endif
#if PORTHOLE_GAME_STARFIGHTER
    &g_starfighter,
#endif
#if PORTHOLE_GAME_KINGDOM
    &g_kingdom,
#endif
#if PORTHOLE_GAME_TOY_TOWN
    &g_toytown,
#endif
#if PORTHOLE_GAME_RACING
    &g_racing,
#endif
};
static constexpr int N_APPS = sizeof APPS / sizeof APPS[0];
static_assert(N_APPS <= MAX_APPS, "the launcher and Shell hold at most MAX_APPS games: raise it and its layouts first");
// Every game's save namespace, switched on or off, then null: a profile delete (and a new profile) erases s<id> in
// each, so a switched-off game's old save never passes to the next kid in that slot. host/test_apps.cpp holds it to
// APPS' store() names.
static const char* const ALL_STORES[] = {Game::STORE, biscuit::STORE, marble::STORE, fc::STORE, pumpkin::STORE,
                                         radar::STORE, starfighter::STORE, kingdom::STORE, toytown::STORE,
                                         racing::STORE, nullptr};
static_assert(sizeof ALL_STORES / sizeof ALL_STORES[0] - 1 <= MAX_APPS, "Shell erases at most MAX_APPS stores");
