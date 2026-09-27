# Marble Kick: game brief

A wooden labyrinth toy seen from above, with a felt pitch inside: tilt the device and the ball rolls like a marble,
round wooden walls, past pegs and holes, into the goal cut into the rim. Twelve levels ramp up: 1-2 teach tilting,
3-4 walls and detours (and the first hole), 5-6 moving defenders, 7-8 the keeper and a goal that swings, 9-12 all of
it together. Built to answer whether tilt feels good for a 6-year-old holding the device in its case; Tilt FC reuses
the shared tilt input, not this game's code or look. Design and slices:
`docs/superpowers/specs/2026-09-27-marble-kick-design.md` (slice 1, "Roll", is what exists).

The shared app brief (`apps/porthole/CLAUDE.md`) covers what every game shares. This file has Marble Kick's product
contract, module map and Save layout.

## Product contract

- **Tilt is the only control.** During play every tap is ignored: a hand holding the case may brush the glass.
  Leaving the Play page takes holding the home knob for 0.6 s (`ui::Hold`, os/ui.h, shared with Tilt FC: a ring
  fills around it; letting go early does nothing, a finger that drifts on the knob still counts, and one that drifts
  off and back starts over). Taps move on from the Calibrate, Goal and
  Done pages, and the knob there is a plain tap. A press on the red button that waits for a steady grip stays shown
  pressed until play starts.
- **Neutral is how the kid holds it, any way.** Every visit starts on the Calibrate page. Pressing the red button
  stores the average gravity of the last 300 ms as level, and waits while any reading in it is not a plausible 1 g
  (800-1200 mg), so a jolt never becomes level. Tilt is gravity turned by the rotation that lays that neutral flat:
  from upright, leaning back rolls the ball up and leaning forward rolls it down, as far as from lying flat.
- **A puzzle, not a funnel.** Tilting straight at the goal wins levels 1-2 only: from level 3 on, a greedy player
  (full tilt at the goal's mouth wherever it is, from each of three grips) must not score in 20 s (`greedyFails` in
  the test). Walls and cups catch it, holes swallow it, the keeper and a moving goal want timing. Three optional stars
  per level reward the long way round; the best count per level is kept.
- **Nothing punishes.** No timer, no lives, no score. A peg only nudges the ball; a hole only sends it back to the
  level's start after a short sink (its stars stay picked up, the level's clock runs on); a level is only not
  finished yet. The ball is never stuck for good: wherever it comes to rest under a tilt, some other tilt frees it
  (`neverStuck` samples resting spots on every level), no gap is ball-sized (`noBallSizedGaps`: narrower, or a ball's
  width plus 8 px), the goal's mouth has no corner to park in, and every level ships with a recorded solution that
  `host/test_marble.cpp` replays to a goal. The replay runs from three grips, each tilt turned into a real 1 g
  reading by the test's own inverse of the tilt mapping, so it shows the solutions survive whole-milli-g rounding
  at each grip; that the mapping itself tilts the right way is checked separately, against plain rotations.
- **Its own look** (root rule 7): walnut rim, maple tray with turned grain, striped felt with chalk lines, maple rails,
  red lacquer pegs and defenders (with a groove pressed into the felt along a defender's track), a brass keeper in a
  red band, dark holes with a maple lip, brass stars, brass posts, home knob (a carved house) and coins, a cream
  ball, wooden letter blocks. Colors live in `render.cpp` only. No font: the few glyphs (digits, `GOAL!`) are 5x7
  cells drawn in code, each logged to `gfx::textLog` for the UI audit.
- **No sound** in slice 1 (`soundOn()` is always false).

Pages (screen names are prefixed `mk_`; the home knob at the bottom of the tray leaves from every one):

| Page | Shows | Goes to |
| --- | --- | --- |
| Calibrate (`mk_calibrate`) | the tray, a dish whose small ball rolls the way the game's will and settles when held still (its ring turns brass), the red button | Play (button, once the grip is steady: neutral stored) |
| Play (`mk_play`) | the level (rails, pegs, holes, stars), the ball, the level on a brass coin | Goal (the ball's center crosses the rim in the mouth) |
| Goal (`mk_goal`) | pennants on the posts, `GOAL!` blocks, this run's three stars | the next level's Play (a tap or 2 s), or Done after the last |
| Done (`mk_done`) | pennants, 3 s of confetti, twelve level coins with their best stars, the red button | Play on a coin's level, or level 1 (button) |

After all twelve, the next visit queues level 1.

## Module map

Everything is in `namespace marble` (Pets Club owns the global `Game`).

- `tune.h`: the feel knobs: dead zone (87 mg), full tilt (423 mg), acceleration, felt drag, rolling friction, top
  speed, rim and peg restitution, the 5 ms substep, the Goal page's 2 s. Retune here after playing on the device.
- `physics.h` / `physics.cpp`: tilt from the 3D neutral (`tilt::from` in `os/tilt.h`, Rodrigues onto face-up; the
  Calibrate page's steady neutral is `tilt::Steady`, shared with Tilt FC) to acceleration, one
  substep (roll, move, bump off pegs (a moving one hits back with its own velocity: a nudge), rails (capsules
  `RAIL_R` thick) and the home knob, then the rim; drop into a slow-enough hole, pick up stars), goal detection. The
  ball's center may be on the felt or in the goal's mouth, a channel `goalHalf - POST_R - BALL_R` either side of the
  goal's line; outside, it goes back to the nearer of the two edges, so a ball rolling along the rim flows into the
  mouth. The mouth is worked out in the goal's own frame at `goalAngle(t)`, so a moving goal is the same channel,
  turned; the keeper and the posts ride with it. Everything that moves follows the level's clock (`Ball::ms`), which
  starts at the first real tilt and runs on through holes, so solutions replay exactly. Floats in panel px from the
  tray's center, +y down. Geometry: `PITCH_R` 188 (the felt's edge), `BALL_R` 20, the knob at (0, 195) r 30. At
  `V_MAX` the ball moves 3 px a substep against a 27 px contact distance at a rail or a 7 px peg: no tunneling. No
  drawing. Holes: `HOLE_SKIM`, `SINK_MS`, `RESPAWN_MS` in `tune.h`.
- `levels.h`: `LEVELS[]` (`inline constexpr`: one copy in the image): per level a start, the goal's width and how it
  moves (`GoalMove`: `FIXED`, `SPIN` degrees a second turning back every period, or `SWING` between two angles),
  pegs (`peg`, `defender` on a line, `keeper` across the mouth), rails, holes, three stars, and its solution
  (`Step{tx, ty, ms}`: milli-g from neutral, whole 40 ms frames). The index is persisted (the save holds an index
  and per-index stars): append levels, never reorder or remove; a level's content may change. A changed level must
  keep the rules above (`build/host/test_marble --levels` prints greedy, stuck, gap and solution results for every
  level without stopping at the first) and its solution must still score when replayed from lying flat, 45 degrees
  and upright (see the Nothing punishes point above for what that replay proves). The playtests replay the same
  solutions: each `# solution N from X Y Z` block in `tests/playtests/` must match what `levels.h` gives for that
  grip, and `build/host/test_marble --write-playtests` rewrites them. The recorder that made the solutions was a
  throwaway waypoint-following controller, not kept: it steered toward hand-placed waypoints round the walls and
  holes, the last one the mouth wherever it was, and kept a run that scored from all three grips.
- `save.h`: the Save (below), `seal`, `loadBlob`. Header-only.
- `render.h` / `render.cpp`: the look, on the runtime's clipped span writer (`os/canvas.h`, shared with Tilt FC); the
  tray is painted row by row from the distance to the center (rings of wood, grain, felt and chalk; the net cut into the
  rim above the goal), so any rectangle can be repainted without a cached image. Shadows are the tray's own colors at
  70%, baked, never blended. Also rails, grooves, holes, stars, the ball (sinking: smaller and darker in baked steps),
  pegs, the keeper, knob, coins, the red button, pennants, the Calibrate dish and the Done page's confetti. The net is
  cut into the rim at the goal's angle, drawn at the nearest quarter degree (`Mouth::key`), so a buffer's net is named
  by that key.
- `game.h` / `game.cpp`: the App: the four pages, the launcher icon (a 32x32 sprite in the shell's indexed palette, as
  ASCII art converted at compile time), and `render()`'s buffer bookkeeping (`canvas::Frames`, os/canvas.h): each of the
  panel's two buffers (`os/app.h`) gets the whole page after a page change or when the page's `look` changed (a pressed
  button, the hold ring, the dish's glow, a star picked up, a confetti frame); otherwise each mover (the ball, keyed by
  its sink time; every moving peg; a moving goal, keyed by its drawn angle; the dish's ball) that changed gets its old
  and new boxes repainted, and nothing when none did. `host/test_marble.cpp` checks every such frame against a full
  repaint, over solution replays, greedy runs into holes, and moving goals. `debugCmd("level<N>")` calibrates as held
  now and plays level N; `debugPrint` also reports `ready=` and `starting=`.

## Save layout

`Save` (`save.h`), 48 bytes, NVS namespace `"marblekick"` (`STORE`, never renamed once shipped), key
`s<profile id>`, written by the shell. Magic `0x4B524D4D` ("MMRK"), `version` 2 (`SAVE_VERSION`), `size`, then
`level` (uint8: levels finished in a row from the first, the index of the level to play next; `NUM_LEVELS` means all
done, and a value past this build's levels, from a newer firmware, is kept as is; either way the next visit plays
level 1), `reserved[3]` (zero), `stars[32]` (v2: the most stars picked up in one run of each level, 0-3, by level
index), `crc` last. Version 1 (16 bytes, before `stars`) still loads: its level kept, no stars (`saveMigration` in the
test). Append-only like every other game's: a new field goes right before `crc` with a version bump, and `loadBlob`
zero-fills the tail of an older, shorter blob; a blob must be exactly its version's size (`SAVE_SIZES`). A blob with a
wrong magic, size, version or crc is ignored (the kid starts from level 1); it is never half-loaded. So a firmware
downgrade after a Save change resets progress (see `save.h`): a v1 firmware ignores a v2 save.

## Known gaps (slice 1)

- Measured on the board (2026-09-27, levels driven through the serial gravity override `G`): 55 fps while the ball
  rolls, render ~2 ms, present ~16 ms (vsync); levels 1 and 2 scored. So `V_MAX` (600 px/s) moves the ball ~11 px a
  frame and stays as it is.
- How hard the levels are was judged in the sim: the recorded solutions take 3-10 s, a few coarse hand tilts solve
  levels 3, 7 and 8 in 3-7 s, and a greedy tilt never solves 3-12. Whether a 6-year-old finds them fun rather than
  frustrating is for slice 2, on the device.
- The QMI8658's axes were measured by hand on the board (2026-09-27, `SCREEN_FROM_CHIP` in `firmware/board.cpp`), so
  a tilt rolls the ball the way the device leans. The knobs in `tune.h` and the calibration dish were set in the sim;
  slice 2 tunes them with the kid playing on the device.
- Left as they are on purpose after the first review, to revisit: the launcher and shell text spacing, and the night
  tint (`tint()` follows the clock, but the RGB565 page draws its own daylight colors).
- Held within about 10-20 degrees of face down (lying on your back, the device overhead), which way a tilt rolls the
  ball depends on the exact grip it was calibrated in: the rotation that lays that grip flat turns sharply there (it
  is undefined at exactly face down, where `tilt::from` falls back to half a turn about x). No fix for now.
