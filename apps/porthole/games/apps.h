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
#include "games/sand-jar/game.h"
#include "games/times-cards/game.h"
#include "psram.h"
#include "shell.h"

// Each game is built in PSRAM at start-up (os/psram.h), so none of them takes static internal RAM.
static Game& g_pets = psram::make<Game>();
static biscuit::Game& g_biscuit = psram::make<biscuit::Game>();
static marble::Game& g_marble = psram::make<marble::Game>();
static fc::Game& g_tiltfc = psram::make<fc::Game>();
static pumpkin::Game& g_pumpkin = psram::make<pumpkin::Game>();
static radar::Game& g_radar = psram::make<radar::Game>();
static starfighter::Game& g_starfighter = psram::make<starfighter::Game>();
static kingdom::Game& g_kingdom = psram::make<kingdom::Game>();
static toytown::Game& g_toytown = psram::make<toytown::Game>();
static racing::Game& g_racing = psram::make<racing::Game>();
static sandjar::Game& g_sandjar = psram::make<sandjar::Game>();
static cards::Game& g_cards = psram::make<cards::Game>();
static App* const APPS[] = {&g_pets, &g_biscuit, &g_marble, &g_tiltfc, &g_pumpkin, &g_radar, &g_starfighter, &g_kingdom, &g_toytown, &g_racing, &g_sandjar, &g_cards};
static constexpr int N_APPS = sizeof APPS / sizeof APPS[0];
static_assert(N_APPS <= MAX_APPS, "the launcher and Shell hold at most MAX_APPS games: raise it and its layouts first");
// Each game's stable id, in APPS order: kids' favorites (porthole/f<id>) and the games a grown-up switched off
// ("sys"/"games", a bit each) keep these, so an id never changes and is never reused, even once its game is gone.
// A new game takes the next unused id.
static constexpr uint8_t APP_IDS[] = {1, 2, 3, 4, 5, 6, 7, 8, 9, 10, 11, 12};
static_assert(sizeof APP_IDS == N_APPS, "one id per game, in APPS order");
static_assert(shell::validIds(APP_IDS, N_APPS), "ids are unique and 1..shell::MAX_GAME_ID");
