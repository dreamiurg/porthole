# Marble Kick: game brief

A wooden labyrinth toy seen from above, with a felt pitch inside: tilt the device and the ball rolls like a marble,
past red pegs into the goal cut into the rim. Built to answer whether tilt feels good for a 6-year-old holding the
device in its case; Tilt FC reuses the shared tilt input, not this game's code or look. Design and slices:
`docs/superpowers/specs/2026-09-27-marble-kick-design.md` (slice 1, "Roll", is what exists).

The shared app brief (`apps/porthole/CLAUDE.md`) covers what every game shares. This file has Marble Kick's product
contract, module map and Save layout.

## Product contract

- **Tilt is the only control.** During play every tap is ignored except the back knob: a hand holding the case may
  brush the glass. Taps only move on from the Calibrate, Goal and Done pages.
- **Neutral is how the kid holds it.** Every visit starts on the Calibrate page; the gravity held when the kid taps
  the red button is level from then on (`Input.gx/gy` minus it, then the dead zone and the clamp in `tune.h`).
- **Nothing punishes.** No timer, no holes, no lives, no score. A peg only nudges the ball; a level is only not
  finished yet. Every level ships with a recorded solution that `host/test_marble.cpp` replays to a goal, so no level
  can trap the ball.
- **Its own look** (root rule 7): walnut rim, maple tray with turned grain, striped felt with chalk lines, red lacquer
  pegs, brass posts, knob and coins, a cream ball, wooden letter blocks. Colors live in `render.cpp` only. No font:
  the few glyphs (digits, `GOAL!`) are 5x7 cells drawn in code, each logged to `gfx::textLog` for the UI audit.
- **No sound** in slice 1 (`soundOn()` is always false).

Pages (screen names are prefixed `mk_`; the back knob at the bottom of the tray leaves from every one):

| Page | Shows | Goes to |
| --- | --- | --- |
| Calibrate (`mk_calibrate`) | the tray, a bubble level showing the tilt now, the red button | Play (button: neutral stored) |
| Play (`mk_play`) | the level's pegs, the ball, the level on a brass coin | Goal (the ball's center crosses the rim in the gap) |
| Goal (`mk_goal`) | pennants on the posts, `GOAL!` blocks | the next level's Play (a tap or 2 s), or Done after the last |
| Done (`mk_done`) | every level's coin, the ball, the red button | Play on the last level (button) |

## Module map

Everything is in `namespace marble` (Pets Club owns the global `Game`).

- `tune.h`: the feel knobs: dead zone (87 mg), full tilt (423 mg), acceleration, felt drag, rolling friction, top
  speed, rim and peg restitution, the 5 ms substep, the Goal page's 2 s. Retune here after playing on the device.
- `physics.h` / `physics.cpp`: tilt to acceleration, one substep (roll, move, bump off pegs, goalposts and the back
  knob, the rim open in the goal gap), goal detection. Floats in panel px from the tray's center, +y down. Geometry:
  `PITCH_R` 188 (the felt's edge), `BALL_R` 20, `POST_R` 7, the knob at (0, 195) r 30. At `V_MAX` the ball moves 3 px
  a substep against a 27 px contact distance at the thinnest post: no tunneling. No drawing.
- `levels.h`: `LEVELS[]` and each level's solution (`Step{tx, ty, ms}`: milli-g from neutral, whole 40 ms frames).
  Array order is persisted (the save holds an index): append levels, never reorder or remove. Change a level and its
  solution must still reach the goal in `host/test_marble.cpp`, and in the playtests, which replay the same steps.
- `save.h`: the Save (below), `seal`, `loadBlob`. Header-only.
- `render.h` / `render.cpp`: the look. One clipped span writer under everything; the tray is painted row by row from
  the distance to the center (rings of wood, grain, felt and chalk; the net cut into the rim above the goal), so any
  rectangle can be repainted without a cached image. Shadows are the tray's own colors at 70%, baked, never blended.
- `game.h` / `game.cpp`: the App: the four pages, the launcher icon (a 32x32 sprite in the shell's indexed palette,
  as ASCII art converted at compile time), and `render()`'s buffer bookkeeping: each of the panel's two buffers
  (`os/app.h`) gets the whole page after a page change, then only the moving part's old and new boxes (the ball, the
  bubble), and nothing when those are unchanged. `debugCmd("level<N>")` calibrates as held now and plays level N.

## Save layout

`Save` (`save.h`), 16 bytes, NVS namespace `"marblekick"` (`STORE`, never renamed once shipped), key
`s<profile id>`, written by the shell. Magic `0x4B524D4D` ("MMRK"), `version` 1 (`SAVE_VERSION`), `size`, then
`level` (uint8: levels finished in a row from the first, the index of the level to play next; `NUM_LEVELS` means all
done, and a value past this build's levels, from a newer firmware, is kept as is and played as the last level),
`reserved[3]` (zero), `crc` last. Append-only like every other game's: a new field goes right before `crc` with a
version bump, and `loadBlob` already zero-fills the tail of an older, shorter blob. A blob with a wrong magic, size,
version or crc is ignored (the kid starts from level 1); it is never half-loaded.

## Known gaps (slice 1)

- The QMI8658's x/y mapping to the panel is unverified on the board, and the knobs in `tune.h` were set in the sim.
  Slice 2 settles both with the kid playing on the device.
- A tilt game is played without touching the glass: the firmware dims the backlight after a minute without a touch,
  and the shell counts touch-idle time as neither play nor rest. Both need a platform change (not in this game).
