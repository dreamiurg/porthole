# Opt-in telemetry

A user decision of 2026-11-16 (Дмитро): telemetry for the family that owns the board, so Claude can see what the kids
play and how Times Cards works for each of them, without changing anything for other families.

## Rules

- Off on every board; a board sends nothing until a grown-up opts in.
- One switch on Admin 2/2, "Usage stats", off by default (Дмитро, 2026-11-20; it replaced the two switches of
  2026-11-17). On is the personal level: random board and player ids, never a name, the usual meaning of "anonymous"
  in products like Home Assistant, Firefox and VS Code. The board's id shows under the switch, so the owner finds the
  board in PostHog and labels the kids' player ids there. Off wipes the device id and salt. The anonymous level stays,
  for the SD file and serial only.
- From the menu, events go to the Porthole project in PostHog (us.i.posthog.com) with its public ingest key built in.
  The key only writes events, and the website ships the same key. A kid's events carry `$set: {device}`, so in PostHog
  each kid is a person tied to the board.
- The owner picks the backend: PostHog (`/batch/` events) or any OpenTelemetry collector (OTLP/HTTP JSON logs).
- Never a name. A kid is a player id: crc32 over a random per-device salt, the profile slot and the name, 16 hex
  digits. It is derived on the board, never stored, and changes if telemetry is turned off and on, or a new kid takes a
  deleted kid's slot.
- Two levels. Personal (the default for an owner's own boards): every event, with the player id. Anonymous (what other
  families would choose): only `device_started`, `crashed`, `game_opened` and `game_closed`, only the keys `game`,
  `seconds`, `reset` and `channel`, no player, and a device id that changes every month.
- The menu's Off holds until the SD file changes.

## Setup

For the Porthole project, nothing: hold "Who's playing?" 5 s, Admin, page 2, "Usage stats: on".

For another backend, `/porthole/telemetry.txt` on the SD card, read at boot:

```
posthog
https://us.i.posthog.com
phc_<project key>
```

or `otlp`, a collector URL and an optional bearer token. A fourth line `anonymous` picks that level; one line `off`
turns it off. Serial: `Y posthog https://us.i.posthog.com phc_<key>` (one line, same words). Only https URLs.

## How it runs

Games and the shell call `telemetry::emit()`; with no queue installed (the sim, the tests, a board left off) it does
nothing. The firmware keeps a 1024-event queue in PSRAM, and while the screen is dark and wifi is saved it sends the
oldest events as one request every 15 minutes (5 s between batches while more wait), as a radio job like the update
check. 2xx drops the sent events; 400, 413 and 422 drop them as bad; anything else keeps them for the next try.

## Events

| Event | Properties |
|-------|-----------|
| `device_started` | `reset`, `channel` |
| `crashed` | `reset` (panic, watchdog, brownout), on the next boot |
| `player_selected` | `avatar`, `age` |
| `game_opened` | `game` |
| `game_closed` | `game`, `seconds`, `stars`, `exit` (`home`, `rest`, `time_up`) |
| `times_cards_answer` | `fact`, `grade`, `correct`, `ms`, `box`, `retry` |
| `level_finished` | `game`, `what`, `easy`, `hard`, `wrong`, `level_up`; Pets Club's Ukrainian (`what` = `ukrainian`): `right`, `asked`, `rating`, `calibrating` |
| `uk_answer` | `kind` (`meaning`, `lookalike`, `syllables`), `word` (its English meaning), `step`, `rating` (the skills engine's, after the answer), `correct`, `ms` |
| `uk_calibrated` | `rating`, `answers`: the engine's calibration ended ([tuning](2026-11-21-ukrainian-telemetry.md)) |

## Known limits

- Events are lost on power-off (the queue is in RAM) and the oldest go when it is full.
- Timestamps before the first clock sync after boot can be wrong.
- A batch resent after a lost reply can arrive twice.
