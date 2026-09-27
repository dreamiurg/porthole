# Sand Jar: game brief

A tilt toy. Hold the round board and tilt it: sand, water or stars move inside like a jar. Nothing to win, nothing to
lose, nothing saved: a kid plays with it the way they would with a jar of sand. Ported from the approved browser
prototype (the user's `tilt-toy-lab.html`), which stays the reference for the physics, the colors and the look.

The shared app brief (`apps/porthole/CLAUDE.md`) covers what every game shares. This file has Sand Jar's modes, its
physics rules, its tuning knobs and its one exception to the shared constraints.

## The toy

One screen (`sandjar_jar`), full glass. At the top, every game's orange home button (back to the launcher). At the
bottom, the mode button: a dark pill with a dot per mode, the current one big and ringed; a tap pours the next mode.
Both are drawn over the jar. A touch anywhere else is a fingertip in the jar; a touch that *begins* on a button belongs
to that button until lifted, so tapping a button never also shoves sand.

- **Sand Jar**: warm sand in six tones, each grain its own brightness; lighter on the surface and while moving, with a
  glint now and then.
- **Rainbow Sand**: six colored strata, pink at the bottom; tilt, shake and a finger mix them into patterns.
- **Beach**: 45% sand under water. Sand sinks through water and slides under it, wet sand is darker, the water has
  foam on top, depth shading and moving caustic glints, and bubbles rise and pop at the surface.
- **Orbits**: ~580 glowing stars round a dim sun in the middle, most spinning the same way; tilt drifts every orbit
  like a wind; a touch is a heavy star everything swings round; each star leaves a fading trail.

The jar keeps its sand between visits (like a real one on a shelf) while the game object lives; a new mode is always
a fresh pour. No sounds (`soundOn` is always false), no save (`takeSave` is always false; `store()` is `"sandjar"`,
reserved all the same), no clock, no personal data.

## Module map

- `physics.h` / `physics.cpp` (`struct Jar`): no drawing, no allocation, deterministic (its own xorshift). Grains on a
  160x160 cell grid (a cell is 3 physical px, one logical px: a touch is a cell as is) inside the glass (radius 78.5
  cells); `occ[]` holds each cell's grain index + 1, or `EMPTY`, `WALL` (outside the glass), `FINGER`. Grain position
  and velocity are 8.8 fixed point (`uint16_t`/`int16_t`); a look byte packs kind (sand, water, bubble), shade and
  tone. Orbits' bodies are floats in panel px. The grains and (Orbits) the bodies plus the renderer's trail light share
  one union: the whole Jar is ~153 KB and must stay in the board's internal RAM (random access in PSRAM is ~14x
  slower), so it is a member of the static game object, never heap.
- `game.h` / `game.cpp` (`class Game`, `namespace sandjar`): App plumbing, the motion sensor (`Input::gx/gy/gz`), the
  touch, the physics clock, and the renderer.
- In `APPS[]` in both `firmware/main.cpp` and `host/sim.cpp`. On the board the game object (~166 KB, the Jar in it) is
  static in internal RAM, the two `seen_` histories are `new`ed once and land in PSRAM (the heap puts blocks over 4 KB
  there), and both files build at `-O2` (a `#pragma`, board only: the firmware is `-Os`).
- `tools/icon.py` -> `generated/icon.h`: the launcher icon (32x32 indexed, a jar of sand), `--check` in `make lint`.
- `host/test_sand_jar.cpp` (shared `host/`): the physics self-check. `tests/playtests/50_sand_jar.txt` and
  `24_monkey_sand_jar.txt`: the screen, every mode audited, and chaos under ASan/UBSan.

## Physics rules (the user validated these; keep them)

- **Gravity** comes from the sensor in any direction in the glass's plane, magnitude 0..1 g (the tilt). Lying flat is
  0 g (under `FLAT_G`, a dead zone against sensor noise).
- **Nothing moves uphill.** Every slide, roll and spill direction is ranked by its true angle to gravity (the dot
  product with the gravity direction), not by grid axes. After a hit a grain slides along an axis only if that axis
  goes downhill enough (`minDot`); the blocked part of its speed is lost: no bounce.
- **Water levels at any angle**: a stopped water grain walks along the true level line, alternating the two grid
  directions around it (`pe0`/`pe1`, taking `pe1` on a `pfr` share of steps, a Bresenham walk), and stops early where it
  can fall again.
- **Resting grains may roll or spill every step**, with a chance that scales with gravity: at 0 g nothing avalanches and
  grains only keep their momentum.
- **Sand loses speed on contact** (friction `SAND_FRIC`, no bounce); `SAND_SLIDE` is the chance to roll off a diagonal.
- **The finger** is a solid disc of radius `FINGER_R` (42 physical px, ~9 mm). Grains inside it are pushed out along
  the ray from its center, bent toward where it moves, to the first free cell outside; its moving rim drags what it
  touches along (water a lot, sand barely). In Orbits it is a point gravity well instead (`ORB_STAR`).
- **Shake**: a sensor reading more than `SHAKE_MG` off 1 g, or one that jumps more than `JOLT_MG` from the frame before,
  kicks every grain randomly (`SHAKE`, fading by `SHAKE_FADE` a step).
- A grain whose eight neighbors are all taken (for sand in Beach: taken by sand, since it may sink into water) cannot
  move this step: it stops, as a hit would stop it, and falls asleep. Most of a jar is buried like this. An asleep grain
  costs nothing until a write to `occ` next to it wakes it (`changed`): then it moves this step if its height is still
  to come, else the next. A settled jar only moves its surface.
- Grains are moved bottom first (awake grains in a list per height along gravity) so the ones above fall into the room
  the ones below leave.

Three additions to the prototype, each a fix for something it got wrong, with a test:

1. **Roll-over** (`rollOver`, `ROLL_DROP`): a stopped grain of sand with no free neighbor downhill may roll over the one
   beside it, when that ends it at least `ROLL_DROP` (0.5) cells lower along gravity. The prototype's level heap held
   any tilt under ~65 degrees (its surface grains only had each other to roll into), so tipping the jar 45 degrees did
   nothing; now a heap holds to ~38 degrees and flows from ~40, like damp sand.
2. **Sand slides under water**: a stopped grain of sand may roll into water as into an empty cell (with the same 30%
   chance as sinking), the water taking its old cell. The prototype's sand beside water stood as a wall.
3. **The finger squeezes grains out sideways** when the ray outward runs into the glass (and its rays are as long as the
   grid): the prototype left grains pressed against the rim under the finger.

## Tuning knobs

In `physics.h`: `FILL_PCT` (grains per 100 cells of glass per mode: 34/42/46; Orbits 3 -> 6568, 8114, 8887 grains and
579 bodies; `MAX_GRAINS`/`MAX_BODIES` bound them), `FINGER_R`, `WAKE`, `SAND_SLIDE`, `SAND_FRIC`, `ROLL_DROP`,
`SHAKE`/`SHAKE_FADE`, `BEACH_SAND`, `ORB_SUN`/`ORB_STAR`; the per-kind `PARAM` table in `physics.cpp` (the prototype's).
In `game.h`: `STEP_MS` (16: the prototype's 60 Hz) and `MAX_STEPS` (2 a frame; a slow frame slows the jar rather than
piling up steps), `LATE_MS`/`SLOW_MS` (a frame over 45 ms means one step a frame for 500 ms: a shake or a fresh pour
runs in brief half-speed slow motion rather than making every frame later), `TILT_EASE`, `FLAT_G`, `SHAKE_MG`/`JOLT_MG`, and Orbits' `TRAIL_MID`/`TRAIL_SIDE`/`CORE`.

Host cost (M3 Max, `-O2`, 34 ms frames of two steps, the fastest of repeated runs): update 0.09-0.36 ms and render
0.04-0.12 ms a frame in the grain modes, Orbits 0.03-0.04 and 0.10; the sim's `perf` (`-O1`) is within ~1.5x of that.
The host is 60-300x faster than the board, and not in proportion: it has a hardware divide and square root, and no
PSRAM. The board (M, `tools/devctl.py metrics`), last measured before the pass that took the library calls and the
small calls out of the per-grain, per-body and per-cell paths: Sand Jar and Rainbow Sand at 29 fps settled, flowing,
flat, shaken and under a finger (update 13-17 ms, render 11-14 ms: two vsyncs); Beach (update 19-22, render 16-20) and
Orbits (update 10-12, render 21-26) just over two vsyncs, at 19.5 fps (Orbits 27-29 at rest or held). Since then, with
`LATE_MS`/`SLOW_MS`: every mode 29 fps settled, flowing and right after a pour; shaken, Sand Jar and Rainbow Sand 21,
Beach 19.5, Orbits 29. Shaken, one step is 13-19 ms and the render 21-26 (every cell changes): the render is the limit
there now. The panel swaps
at vsync (58 Hz), so a frame is 17, 34 or 51 ms. The board has no hardware float divide or square root (each is a
library call, and so is `memcpy`): the per-grain, per-body and per-cell paths multiply by reciprocals and use
`invSqrt` (`physics.h`), and their small helpers are forced inline. With two panel buffers by turns, anything that
changes every frame or every other one (water's shimmer, a fading trail) is rewritten in both: animating it at half
rate writes no fewer cells. `FILL_PCT` and `STEP_MS`/`MAX_STEPS` are where to trade.

## Rendering, and the constraint-2 exception

Everything is drawn at cell resolution: a cell is a 3x3 block. Each frame `drawCells` computes every cell's color (the
mode's background from a per-ring table with a 16x16 dither tile, a grain's color, or Orbits' sky plus trail light;
then the glass's reflection) and writes the block only if it differs from what that panel buffer already holds. The
panel has two buffers used by turns, each holding the frame from two renders ago (`os/app.h`); `seen_` remembers one
cell history per buffer (two 51 KB arrays, allocated once, scanned in order, so the board may keep them in PSRAM), and
a buffer not seen before, the first frames after `enter()`, or a new pour, is drawn in full; otherwise only the cells
inside the glass are visited. The buttons are redrawn over their cells (put back first) only when a cell under them
changed or their own state did. A settled jar writes almost nothing.

**Exception to shared constraint 2.** RGB565 games keep to flat fills and pre-rendered art with no runtime blending.
Sand Jar blends at runtime, with the user's approval, because the toy is its lighting: each grain is its palette color
scaled by its tone, the surface and speed highlights, wet-sand darkening and a rim vignette; water mixes its depth
gradient, foam and caustic glints; Orbits adds decaying RGB trail light and star cores over the sky; the glass's
reflection is added over everything. The exception is narrow: it is only this game's cell colors (no text: the game
draws no strings at all), computed per cell, not per pixel.

## Testing

- `make test` runs `host/test_sand_jar.cpp`: counts conserved and one grain per cell, all inside the glass; a settled
  heap at 0 g stays put even when the last direction seen points elsewhere (the bug the user hit); water tipped 30, 45
  and -60 degrees levels within 8 cells (a stuck walk leaves ~40); a small tilt never lifts a grain; the finger's disc is
  cleared and nothing lost, and its cells free once lifted; sand ends under water; a shake loses nothing; Orbits' bodies
  stay on the glass; the same inputs replay the same jar. And the renderer: after frames of flowing sand and water, a
  sweeping finger, home held down, a shake and the jar upside down, in every mode, the buffer on the glass equals a full
  redraw of the same state, pixel for pixel.
- The sim: `tilt X Y Z` (milli-g), `shake`, `perf`, `dbg sand|rainbow|beach|orbits`; `debug` prints `mode`, `grains`,
  `sand`, `water`, `bubbles`, `tilt`, `finger`, `shaking`, `steps`. The SDL window's arrow keys turn gravity, space
  shakes; the web emulator has a tilt pad, Upright/Flat and Shake.
