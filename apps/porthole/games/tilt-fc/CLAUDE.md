# Tilt FC: game brief

Top-down street football on the round glass: tilt steers the kid's player, a tap anywhere passes or shoots. Two a side
plus a keeper each, first to three goals or three minutes. Built for a 6-year-old who loves football; the first build
of Marble Kick taught that tilting straight at the goal must not be enough, so the opponents defend from the first
match. Design, decisions and slices: `docs/superpowers/specs/2026-09-27-tilt-fc-design.md` (slice 1 is what exists).

The shared app brief (`apps/porthole/CLAUDE.md`) covers what every game shares. This file has Tilt FC's product
contract, module map and Save layout.

## Product contract

- **Tilt and tap, nothing else.** Tilt sets the controlled player's direction, never his speed (one run speed past a
  ~5 degree dead zone, `DEAD_MG`). A press anywhere on the Match page is the one button: with the ball it passes to the
  teammate (outfield or keeper) nearest the aim within +-30 degrees, shoots when the goal's middle is within
  `SHOOT_RANGE` and in that cone (at the spot where the aim crosses the goal line, shown as a sun-yellow dot in the
  net), or rolls the ball on ahead; without it, the player slides for the ball, and a slide that wins nothing costs
  half a second. The aim is the tilt, or where the player faces inside the dead zone. The kid always steers one teal
  outfield player: the one with the ball, the one a teal pass is heading to (he runs to meet it by himself), or else
  the one nearer the ball.
- **Neutral is how the kid holds it.** Every match starts on the Calibrate page: the go sign stores the steady grip
  (`tilt::Steady`, os/tilt.h) as level, and waits out a jolt.
- **A game, not a demo.** Coral press the carrier and stand in the passing lane; touching the ball someone carries
  knocks it loose (nobody falls, no fouls). Keepers cover the shot with a new random reaction every kick. Greedy play
  (straight at the goal, shoot in range) scores well under half what passing and aiming do and does not win reliably;
  `host/test_tiltfc.cpp` plays thirty seeded matches per way of playing per level and holds those bounds.
- **Nothing punishes.** Losing just shows the score; a loss by two or more makes the next match easier, a win harder
  (levels 0-4, `tune.h`). At a teal kickoff coral wait outside the centre circle until the ball moves (up to 3 s). At
  the whistle a teal attack plays on (up to 10 s). A keeper holding the ball gets room to throw it.
- **Leaving:** the round leave sign on the left. In a match (Kickoff, Match, Goal pages) it takes a 0.6 s hold (its rim
  fills with sun yellow; a brush of the glass does nothing); on the Calibrate and Full time pages a tap.
- **Its own look** (root rule 7): a sunny street court from above, blue-grey asphalt with chalk lines in a concrete
  curb, sandstone paving, white goals with a diamond net, teal (the kid's team, #10 and #9) against coral, a low sun
  throwing long shadows to the lower right, sun-yellow for everything the kid acts on. Rubik Mono One for the few words
  and digits. Colors live in `render.cpp` only.
- **No sound** in slice 1 (`soundOn()` is always false).

Pages (screen names are prefixed `fc_`):

| Page | Shows | Goes to |
| --- | --- | --- |
| Calibrate (`fc_calibrate`) | #10 in the centre circle, a chalk arrow the way a tilt would send him (a sun ring when held still), the go sign | Kickoff (go sign, once the grip is steady: neutral stored) |
| Kickoff (`fc_kickoff`) | everyone in place, a 3-2-1 | Match (1.2 s) |
| Match (`fc_match`) | the court, the scoreboard (coral above, teal below, the clock dial between) | Goal (a goal), Full time (the whistle) |
| Goal (`fc_goal`) | GOAL!, the ball in the net | Kickoff, the side that conceded on the ball (a tap or 2 s); Full time at three goals or after the whistle |
| Full time (`fc_fulltime`) | the score on two big tiles, a cup for a win | Calibrate (a tap anywhere) |

## Module map

Everything is in `namespace fc` (Pets Club owns the global `Game`).

- `tune.h`: every feel and difficulty knob: dead zone, speeds, shot and pass speeds, ranges, the cone, slides, the
  per-level coral speed and turning, keeper speed and reaction, the match length. Retune here after playing on the
  device, then `make test` (the challenge bounds).
- `match.h` / `match.cpp`: the rules, no drawing: a fixed `STEP_MS` step and a seeded random source, so the host test
  plays whole matches. The kid's player (`pickControl`, `steerKid`, `kidTap`), everyone else (`think`: keepers, a
  carrier, off-ball spots), the world (`move`, `room`, `roll`, `pickUp`, `tackle`, `whistle`). `aim()` is the one rule
  for what a tap does (the render shows it). Positions are panel px from the middle of the glass, +y down.
- `save.h`: the Save (below), `seal`, `loadBlob`. Header-only.
- `render.h` / `render.cpp`: the look, on the runtime's clipped span writer (`os/canvas.h`, which sets `font::clip` for
  text), so any rectangle can be repainted alone; the court is painted row by row from match.h's geometry, and shadows
  are the ground in shade (baked, 62%). Also the strings the game draws (`NUMBERS`, `GOAL_WORD`), checked against the
  font.
- `game.h` / `game.cpp`: the App: the five pages, the launcher icon (a 32x32 sprite in the shell's indexed palette), and
  `render()`'s buffer bookkeeping (`canvas::Frames`, os/canvas.h, shared with Marble Kick): each of the panel's two
  buffers gets the whole page after a page change or a change of `look` (score, countdown, a pressed sign, the hold),
  otherwise each mover (six players keyed by facing, control and pass target; the ball; the aim dot; the clock dial; the
  Calibrate arrow) that changed gets its old and new boxes repainted. `debugCmd`: `kickoff` (calibrate as held now and
  start a match), `score<T>-<C>`, `clock<S>` (seconds left); `debugPrint` reports `wins level teal coral clock control
  owner x y bx by neutralX/Y/Z ready starting`.
- `fonts/`: Rubik Mono One (`RubikMonoOne-Regular.ttf`, from github.com/google/fonts `ofl/rubikmonoone`) and its
  `OFL.txt`. `generated/fonts.h`: 18, 36 and 64 px, converted by `games/biscuit/tools/fontconv.py --game tiltfc`
  (commands in its docstring), bitmaps only for `GLYPHS` (" !0123456789AGLO").
- `host/test_tiltfc.cpp`: the rules (dead zone from three grips, the pass cone, control switching, goals, kickoff,
  tackles and slides, keeper room, the whistle, the kickoff wait), the challenge, the save, the full-time save and
  level, the strings, every incrementally drawn frame equal to a full repaint, and the playtests' `# match from X Y Z`
  blocks (the passing bot's recorded moves to a goal) in step with the rules: `build/host/test_tiltfc
  --write-playtests` rewrites them after a rules or tuning change.
- Playtests: `tests/playtests/60_tiltfc_calibrate.txt` (calibrate, jolt, leaving), `61_tiltfc_goal.txt` (a recorded
  goal, coral's kickoff), `62_tiltfc_fulltime.txt` (a win to Full time from a leaned grip, the whistle, the saved
  level), `63_tiltfc_monkey.txt` (chaos under ASan).

## Save layout

`Save` (`save.h`), 16 bytes, NVS namespace `"tiltfc"` (`STORE`, never renamed once shipped), key `s<profile id>`.
Magic `0x43464C54` ("TLFC"), `version` 1 (`SAVE_VERSION`), `size`, then `wins` (uint16: matches won), `level`
(uint8: the next match's difficulty, 0-4), `reserved` (zero), `crc` last. Append-only: a new field goes right before
`crc` with a version bump, and `loadBlob` zero-fills the tail of an older, shorter blob; a blob must be exactly its
version's size (`SAVE_SIZES`). A blob with a wrong magic, size, version or crc is ignored (level 0, no wins).

## Known gaps (slice 1)

- Not played on the board yet. The frame rate is estimated, not measured: on the host a full repaint costs about half
  of Marble Kick's and a playing frame about a quarter (Marble Kick runs at 55 fps on the board). Slice 3 measures it
  (`devctl.py metrics`) and tunes `DEAD_MG`, the speeds and the cone with the kid.
- The balance was set against bots in the host test, not a 6-year-old. Whether level 0 is gentle enough (a greedy kid
  scores now and then but mostly loses) is for the first real playtest.
- No keeper dive or save moment, no possession cap beyond coral's 6 s, no kits or real teams (slices 2 and 4).
