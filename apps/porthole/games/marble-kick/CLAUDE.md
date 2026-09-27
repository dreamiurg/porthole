# Marble Kick: game brief

A wooden labyrinth toy seen from above, with a felt pitch inside: tilt the device and the ball rolls like a marble,
past red pegs into the goal cut into the rim. Built to answer whether tilt feels good for a 6-year-old holding the
device in its case; Tilt FC reuses the shared tilt input, not this game's code or look. Design and slices:
`docs/superpowers/specs/2026-09-27-marble-kick-design.md` (slice 1, "Roll", is what exists).

The shared app brief (`apps/porthole/CLAUDE.md`) covers what every game shares. This file has Marble Kick's product
contract, module map and Save layout.

## Product contract

- **Tilt is the only control.** During play every tap is ignored: a hand holding the case may brush the glass.
  Leaving the Play page takes holding the home knob for 0.6 s (a ring fills around it; letting go early does
  nothing). Taps move on from the Calibrate, Goal and Done pages, and the knob there is a plain tap.
- **Neutral is how the kid holds it, any way.** Every visit starts on the Calibrate page. Pressing the red button
  stores the average gravity of the last 300 ms as level, and waits while any reading in it is not a plausible 1 g
  (800-1200 mg), so a jolt never becomes level. Tilt is gravity turned by the rotation that lays that neutral flat:
  from upright, leaning back rolls the ball up and leaning forward rolls it down, as far as from lying flat.
- **Nothing punishes.** No timer, no holes, no lives, no score. A peg only nudges the ball; a level is only not
  finished yet. No level can trap the ball: every gap between pegs, goalposts, the knob and the rim is closed or a
  ball's width plus 8 px, the goal's mouth has no corner to park in, and every level ships with a recorded solution
  that `host/test_marble.cpp` replays to a goal from three grips.
- **Its own look** (root rule 7): walnut rim, maple tray with turned grain, striped felt with chalk lines, red lacquer
  pegs, brass posts, home knob (a carved house) and coins, a cream ball, wooden letter blocks. Colors live in
  `render.cpp` only. No font: the few glyphs (digits, `GOAL!`) are 5x7 cells drawn in code, each logged to
  `gfx::textLog` for the UI audit.
- **No sound** in slice 1 (`soundOn()` is always false).

Pages (screen names are prefixed `mk_`; the home knob at the bottom of the tray leaves from every one):

| Page | Shows | Goes to |
| --- | --- | --- |
| Calibrate (`mk_calibrate`) | the tray, a dish whose small ball rolls the way the game's will and settles when held still (its ring turns brass), the red button | Play (button, once the grip is steady: neutral stored) |
| Play (`mk_play`) | the level's pegs, the ball, the level on a brass coin | Goal (the ball's center crosses the rim in the mouth) |
| Goal (`mk_goal`) | pennants on the posts, `GOAL!` blocks | the next level's Play (a tap or 2 s), or Done after the last |
| Done (`mk_done`) | pennants, 3 s of confetti, the six level coins, the red button | Play on a coin's level, or level 1 (button) |

After all six, the next visit queues level 1.

## Module map

Everything is in `namespace marble` (Pets Club owns the global `Game`).

- `tune.h`: the feel knobs: dead zone (87 mg), full tilt (423 mg), acceleration, felt drag, rolling friction, top
  speed, rim and peg restitution, the 5 ms substep, the Goal page's 2 s. Retune here after playing on the device.
- `physics.h` / `physics.cpp`: tilt from the 3D neutral (`tiltFrom`, Rodrigues onto face-up) to acceleration, one
  substep (roll, move, bump off pegs and the home knob, then the rim), goal detection. The ball's center may be on
  the felt or in the goal's mouth, a channel `goalHalf - POST_R - BALL_R` either side of the middle above the center;
  outside, it goes back to the nearer of the two edges, so a ball rolling along the rim flows into the mouth. The
  posts sit in the wall at `POST_RING` and only mark the mouth. Floats in panel px from the tray's center, +y down.
  Geometry: `PITCH_R` 188 (the felt's edge), `BALL_R` 20, the knob at (0, 195) r 30. At `V_MAX` the ball moves 3 px a
  substep against a 27 px contact distance at a 7 px peg: no tunneling. No drawing.
- `levels.h`: `LEVELS[]` and each level's solution (`Step{tx, ty, ms}`: milli-g from neutral, whole 40 ms frames).
  Array order is persisted (the save holds an index): append levels, never reorder or remove. A changed level must
  keep every gap closed or at least 48 px (`noPockets` in the test) and its solution must still score from lying
  flat, 45 degrees and upright. The playtests replay the same solutions: each `# solution N from X Y Z` block in
  `tests/playtests/` must match what `levels.h` gives for that grip, and `build/host/test_marble --write-playtests`
  rewrites them (the recorder that made the solutions was a throwaway steering controller, not kept).
- `save.h`: the Save (below), `seal`, `loadBlob`. Header-only.
- `render.h` / `render.cpp`: the look. One clipped span writer under everything; the tray is painted row by row from
  the distance to the center (rings of wood, grain, felt and chalk; the net cut into the rim above the goal), so any
  rectangle can be repainted without a cached image. Shadows are the tray's own colors at 70%, baked, never blended.
  Also the ball, pegs, knob, coins, the red button, pennants, the Calibrate dish and the Done page's confetti.
- `game.h` / `game.cpp`: the App: the four pages, the launcher icon (a 32x32 sprite in the shell's indexed palette,
  as ASCII art converted at compile time), and `render()`'s buffer bookkeeping: each of the panel's two buffers
  (`os/app.h`) gets the whole page after a page change or when the page's `look` changed (a pressed button, the hold
  ring, the dish's glow, a confetti frame), then only the moving part's old and new boxes (the ball, the dish's
  ball), and nothing when those are unchanged; `host/test_marble.cpp` checks every such frame against a full repaint.
  `debugCmd("level<N>")` calibrates as held now and plays level N; `debugPrint` also reports `ready=` and `starting=`.

## Save layout

`Save` (`save.h`), 16 bytes, NVS namespace `"marblekick"` (`STORE`, never renamed once shipped), key
`s<profile id>`, written by the shell. Magic `0x4B524D4D` ("MMRK"), `version` 1 (`SAVE_VERSION`), `size`, then
`level` (uint8: levels finished in a row from the first, the index of the level to play next; `NUM_LEVELS` means all
done, and a value past this build's levels, from a newer firmware, is kept as is; either way the next visit plays
level 1), `reserved[3]` (zero), `crc` last. Append-only like every other game's: a new field goes right before `crc` with a
version bump, and `loadBlob` already zero-fills the tail of an older, shorter blob. A blob with a wrong magic, size,
version or crc is ignored (the kid starts from level 1); it is never half-loaded. So a firmware downgrade after a
future Save change resets progress (see `save.h`).

## Known gaps (slice 1)

- The QMI8658's x/y mapping to the panel is unverified on the board, and the knobs in `tune.h` were set in the sim.
  Slice 2 settles both with the kid playing on the device.
- Left as they are on purpose after the first review, to revisit: `V_MAX` (600 px/s, until the device's frame rate
  is measured), the launcher and shell text spacing, and the night tint (`tint()` follows the clock, but the RGB565
  page draws its own daylight colors).
