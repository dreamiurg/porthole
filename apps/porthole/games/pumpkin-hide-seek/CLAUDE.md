# Pumpkin Hide & Seek

A silent, forgiving 40-second hide-and-seek round in one cozy Halloween room.
The launcher label is `Pumpkin`; namespace `pumpkin` is reserved, but this slice
writes no saves. No lives, score, offline simulation or progression to migrate.

- `rules.*`: allocation-free 10 ms simulation, horizontal roll between x=24..136
  on y=99, three hiding spots (36,80,124), 60 mg dead zone, friction and bounded
  acceleration. Tap the action sign within 13 px of a spot to hide; tap again
  to emerge. Tilt never ejects a hidden pumpkin.
- Grandma warns at 3..5 seconds and searches at 5..10 seconds of each 12-second
  cycle. The beam's endpoint and detection share `beamX`, with 20 px half-width.
  Continuous exposure for 900 ms catches; concealment or leaving the beam resets
  exposure. Surviving 40 seconds wins. Staying hidden is deliberately sufficient
  in this easy first slice.
- `game.*`: Intro -> Calibrate -> Play -> Catch (600 ms) -> Icing (1600 ms) -> Lost,
  or Play -> Won. Retry starts Play with the current grip. Each launcher visit
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
round time, exposure and calibration readiness; it provides no gameplay cheats.
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
