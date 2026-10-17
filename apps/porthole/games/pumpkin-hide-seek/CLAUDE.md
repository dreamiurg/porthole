# Pumpkin Hide & Seek

A silent, forgiving hide-and-seek round in one cozy Halloween room: grab three candies, escape out the door.
The launcher label is `Pumpkin`; namespace `pumpkin` is reserved, but this slice
writes no saves. No lives, score, offline simulation or progression to migrate.

- `rules.*`: allocation-free 10 ms simulation, horizontal roll between x=24..136
  on y=99, three hiding spots (36,80,124), 60 mg dead zone, friction and bounded
  acceleration. Tap the action sign within 13 px of a spot to hide; tap again
  to emerge. Tilt never ejects a hidden pumpkin.
- Goal: roll over three candy corns (within 7 px, never while hidden), then reach the
  door at x=24 out in the open. No timer: staying hidden stalls, it never wins. Candy
  sits at 52..66, 94..108 and 130..136, drawn from the round's seed.
- Grandma loops Rest -> Warn -> Search. The first rest is 3 s and the first move is
  always a plain 5 s sweep from her side (right). Later rests are 2.5..4.5 s and each Warn
  picks a move: Sweep (30%, 5 s), Fast sweep (20%, 3.2 s), TurnBack (20%, 4 s, covers 60%
  of the lane and returns) or Check (30%, 2 s on one spot, about 73% the pumpkin's own
  spot when it's hidden). Sweeps start from her side two times in three. Warnings last 2 s,
  3.5 s for a Check. Detection shares `beamX` with the drawn cone, 20 px half-width; a
  Check also sees into the spot it names. Continuous exposure for 900 ms catches;
  hiding or leaving the beam resets exposure. Getting out of the beam, or having it
  pass over the hidden pumpkin, shows PHEW! for 1.2 s.
- `newRound(seed)` seeds candy and moves (xorshift32). The game seeds from the
  millisecond clock and a play counter; tests and tours are deterministic because
  simulated time is. `host/test_pumpkin.cpp` plays 100 seeds with a bot, including one
  reacting 0.7 s late, to keep every round winnable.
- `game.*`: Intro -> Calibrate -> Play -> Catch (600 ms) -> Icing (1600 ms) -> Lost,
  or Play -> Won (escaped). Retry starts Play with the current grip. Each launcher visit
  starts Intro and requires calibration again. A jolt delays a pending start
  until `tilt::Steady` reports plausible gravity.
- All pages use `ui::FreshGate`. Home is always visible at {68,8,24,22}; during
  Play it requires a 600 ms hold, elsewhere a tap. Action is {40,119,80,24}.
  Home never hides or emerges. Only the action sign changes hiding state.
- Time comes from the monotonic millisecond input, with frame gaps capped at
  100 ms; the wall clock is unused and absence never advances a round.
- `render.*`: original crisp pixel art on a 160px logical grid expanded 3x onto
  the RGB565 surface with its own palette and font. No borrowed game assets.
  Frosting starts at the upper circular edge. Art and text stay within the glass.
- `host/test_pumpkin.cpp`: rules, grip/reentry, input gates, clock behavior and
  round-screen rendering. Scripted emulator tours live in `tests/playtests/`.

Screen names are prefixed `pumpkin_`. Debug reports position, hiding, elapsed
round time, exposure, calibration readiness, candy, Grandma's phase/move and PHEW; it provides no gameplay cheats.
See `docs/superpowers/specs/2026-10-17-pumpkin-hide-and-seek-design.md` at repo root.

## Visual regression checks

Install Pillow in the Python environment used by make (`python3 -m pip install pillow`)
before running the screenshot checks; CI already includes it.

Eight full-size golden PNGs in `docs/` capture intro, calibration, hiding, victory,
catch, two frosting stages and the pie. `make check` compares their pixels with the
win/loss tours and checks crisp 3x scaling inside the glass. After an intentional
art change, run `make snap && python3 tools/pumpkin_goldens.py --update` from the
app directory, review all eight images, then commit them with the source change.
The emulator applies its physical circular mask at the outermost edge; images are
stored at 480x480 without resampling.
