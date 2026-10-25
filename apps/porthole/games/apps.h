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
#include "shell.h"

static Game g_pets;
static biscuit::Game g_biscuit;
static marble::Game g_marble;
static fc::Game g_tiltfc;
static pumpkin::Game g_pumpkin;
static radar::Game g_radar;
static starfighter::Game g_starfighter;
static kingdom::Game g_kingdom;
static App* const APPS[] = {&g_pets, &g_biscuit, &g_marble, &g_tiltfc, &g_pumpkin, &g_radar, &g_starfighter, &g_kingdom};
static constexpr int N_APPS = sizeof APPS / sizeof APPS[0];
static_assert(N_APPS <= MAX_APPS, "the launcher and Shell hold at most MAX_APPS games: raise it and its layouts first");
