// Which games ship: one switch per game, 1 = in APPS (the launcher, the firmware image, the default sim and the web
// emulator), 0 = switched off. A switched-off game stays in the repo and keeps being built and tested: its host tests,
// art and content checks, and its playtest scenarios (make playtest runs them on sims built with every game on).
// A profile delete still erases its saves; otherwise they stay on the device, so switching it back on brings them back.
// A build can override without editing this file: -DPORTHOLE_ALL_GAMES=1 turns every game on, and a per-game flag
// (-DPORTHOLE_GAME_BISCUIT=1) sets that one game and wins over both.
#pragma once
#ifndef PORTHOLE_ALL_GAMES
#define PORTHOLE_ALL_GAMES 0
#endif
// The trailing digit is the switch.
#ifndef PORTHOLE_GAME_PETS_CLUB
#define PORTHOLE_GAME_PETS_CLUB (PORTHOLE_ALL_GAMES || 1)
#endif
#ifndef PORTHOLE_GAME_BISCUIT
#define PORTHOLE_GAME_BISCUIT (PORTHOLE_ALL_GAMES || 0)
#endif
#ifndef PORTHOLE_GAME_MARBLE_KICK
#define PORTHOLE_GAME_MARBLE_KICK (PORTHOLE_ALL_GAMES || 1)
#endif
#ifndef PORTHOLE_GAME_TILT_FC
#define PORTHOLE_GAME_TILT_FC (PORTHOLE_ALL_GAMES || 1)
#endif
#ifndef PORTHOLE_GAME_PUMPKIN
#define PORTHOLE_GAME_PUMPKIN (PORTHOLE_ALL_GAMES || 1)
#endif
#ifndef PORTHOLE_GAME_RADAR
#define PORTHOLE_GAME_RADAR (PORTHOLE_ALL_GAMES || 1)
#endif
#ifndef PORTHOLE_GAME_STARFIGHTER
#define PORTHOLE_GAME_STARFIGHTER (PORTHOLE_ALL_GAMES || 1)
#endif
#ifndef PORTHOLE_GAME_KINGDOM
#define PORTHOLE_GAME_KINGDOM (PORTHOLE_ALL_GAMES || 1)
#endif
#ifndef PORTHOLE_GAME_TOY_TOWN
#define PORTHOLE_GAME_TOY_TOWN (PORTHOLE_ALL_GAMES || 1)
#endif
#ifndef PORTHOLE_GAME_RACING
#define PORTHOLE_GAME_RACING (PORTHOLE_ALL_GAMES || 1)
#endif
