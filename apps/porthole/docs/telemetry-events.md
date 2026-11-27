# Telemetry events

What a board sends when "Usage stats" is on (`os/telemetry.h`, root `AGENTS.md` rule 4), and how a game gets it.
The weekly review reads this page: it queries the Porthole PostHog project (events where `$lib = 'porthole'`).

Every event carries `app_version`, `device` (the board's random id), `player` (the kid's random id, once a kid
is picked; never a name) and a `uuid` made from the board, the boot and a count, so a batch sent twice counts once.
An event holds at most 10 properties (`telemetry::MAX_PROPS`); a text is cut at 23 characters.

## Free for every game (the shell)

`shell/game_telemetry.h` runs around the open game. A new game gets all of this without a line of its own, as long
as its `screenName()` names its screens (it must anyway: the sim and the playtests read it).

- `game_opened` {game}: a kid opened it from the launcher.
- `screen` {game, screen, secs}: a screen the game showed for a second or more, sent when it leaves it. `screen` is
  `App::screenName()`. Time on each screen, the path through a game, and its win and lose pages.
- `game_heartbeat` {game, active_s, where}: every minute in the game. A board unplugged mid-game still leaves its
  minutes; the last heartbeat tells where it stopped.
- `game_closed` {game, seconds, stars, exit, active_s, where}: `seconds` from open to close, `active_s` without idle
  stretches (a minute without a touch or a move), `stars` earned minus spent, `exit` is `home`, `rest` (the kid
  rested) or `time_up` (the day's play time ran out), `where` the screen it was on: the quit point.

The shell also sends `player_selected` {avatar, age}; the firmware `device_started` {reset, channel} and `crashed`.

## One line per game: rounds

A game with rounds, races, matches or levels calls `telemetry::roundFinished()` when one ends:

- `round_finished` {game, level, result, secs, score, + one of the game's own}: `result` is `won`, `lost` or `done`
  (no winner, or no losing); `level` how hard it was; `score` the game's own measure.

| Game | level | score | extra |
|---|---|---|---|
| Rally | the rivals' level on the track | place, 1 to 5 | `track` |
| Star Fighter | 0 | points | `stage` reached |
| Tilt FC | difficulty | the kid's goals | `against` |
| Spooky | 0 | candies | |
| Marble Kick | level number | the level's stars | |

Times Cards and Pets Club's Ukrainian send the older `level_finished` {game, what, ...} instead; count both.

A shop sends `bought` {game, item, cost} (Pets Club's hats and collars, Rally's garage).

## Learning games

- `times_cards_answer` {fact, grade, correct, ms, box, retry}
- `uk_answer` {kind, word, step, rating, correct, ms}, `uk_sentence` {sentence, step, dir, misses, ms, rating,
  correct}, `uk_calibrated` {rating, answers}
- `book_finished` {lang, level, paws, pages, secs, right, first, counted, rating}

Before this page (when an event held 6 properties) `uk_sentence` lost `correct` and `book_finished` lost `first`,
`counted` and `rating`.

## Adding an event

- Use `telemetry::emit(name, {telemetry::P(key, value)...})`: a literal name, literal keys, a short text or a number.
  It is a no-op when telemetry is off or in the sim and tests.
- Never a name, a typed word that could be one, or anything a kid typed.
- Prefer what the shell already sends; a new event answers a question the ones here cannot.
- Add it to this page in the same change. The anonymous level keeps only `device_started`, `game_opened`,
  `game_closed` and `crashed` with `game`, `seconds`, `reset`, `channel` (`os/telemetry.cpp`).
