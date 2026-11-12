// Every game in the image, in launcher order: the one table the firmware (firmware/main.cpp) and the host sim
// (host/sim.cpp) both run. Include it from exactly one file per build. Append a new game here.
#pragma once
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

static Game g_pets;
static biscuit::Game g_biscuit;
static marble::Game g_marble;
static fc::Game g_tiltfc;
static pumpkin::Game g_pumpkin;
static radar::Game g_radar;
static starfighter::Game g_starfighter;
static kingdom::Game g_kingdom;
static toytown::Game g_toytown;
static racing::Game g_racing;
static App* const APPS[] = {&g_pets, &g_biscuit, &g_marble, &g_tiltfc, &g_pumpkin, &g_radar, &g_starfighter, &g_kingdom, &g_toytown, &g_racing};
static constexpr int N_APPS = sizeof APPS / sizeof APPS[0];
static_assert(N_APPS <= MAX_APPS, "the launcher and Shell hold at most MAX_APPS games: raise it and its layouts first");
// Each game's stable id, in APPS order: kids' favorites (porthole/f<id>) and the games a grown-up switched off
// ("sys"/"games", a bit each) keep these, so an id never changes and is never reused, even once its game is gone.
// A new game takes the next unused id.
static constexpr uint8_t APP_IDS[] = {1, 2, 3, 4, 5, 6, 7, 8, 9, 10};
static_assert(sizeof APP_IDS == N_APPS, "one id per game, in APPS order");
static_assert(shell::validIds(APP_IDS, N_APPS), "ids are unique and 1..shell::MAX_GAME_ID");
