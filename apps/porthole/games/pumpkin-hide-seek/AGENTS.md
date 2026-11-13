# Pumpkin Hide & Seek

A silent, forgiving hide-and-seek round in one cozy Halloween room: grab three candies, escape out the door.
The launcher label is `Spooky` ("Pumpkin" is too wide for a slot on a page of three); namespace `pumpkin` is reserved, but this slice
writes no saves. No lives, score, offline simulation or progression to migrate.

- `rules.*`: allocation-free 10 ms simulation, horizontal roll between x=24..136 on y=99, three hiding spots
  (36,80,124), 60 mg dead zone, friction and bounded acceleration. Hold to hide: the pumpkin is hidden
  exactly while a finger that touched the action sign stays on the glass, wherever it drifts. A press that
  began on the sign arms anywhere; a finger slid onto it arms only where it can hide at once (within reach
  of cover), so a finger crossing the sign in the open never hides the pumpkin later. Cover is a spot a
  Check isn't naming: the named spot is decided, so a new hide there does nothing (`cover()`), and one hidden
  there startles out on the step her warning begins (`hidden = -1`, `startle` = `STARTLE_MS`, 1 s, ended early by
  a hide): it hops 4 px in the first `STARTLE_HOP_MS` (400 ms) and, for the whole second, wears a scared face (wide
  CREAM eyes, a round INK "o" for its smile) under a CREAM "!" up and away from her side, at its x. Kept hidden
  under a RUN! sign, a non-reader of 6 held on and was caught (kid playtest 4, M1); a smile and a 0.4 s "!" undersold
  the startle (kid playtest 5, m4). Out in the open within the beam's reach (20 px) of the spot a Check names, warned
  or searched, it wears that scared face too, with no hop or "!", for as long as that holds (`named()`): rolled into
  the named spot with a smile, a 6-year-old lost three of three (kid playtest 6, M1). A finger still held stays
  armed, so it hides at the next cover it rolls into; once a ghost hands the Check back, the spot is cover
  again, so a new press there hides it. It hides on the
  touch if it's within 13 px of cover, or the moment it rolls within 13 px of cover while still held, and comes
  out when that finger lifts. No toggle, so a double tap ends out and never pops
  it out mid-press. Tilt never ejects a hidden pumpkin. The sign says HOLD TO HIDE (cream) near a spot and
  HIDING while hidden; it draws pressed (gold, its word 2 px lower) only while hidden. Away from cover it is
  flat and dim (WALL face, GREY ink, no gold lip) and reads FIND COVER beside an arrow that bobs toward the
  nearest spot a Check isn't naming, or, within 6 px of the midpoint between two, the one it last rolled toward
  (`coverDir()`, from `Round::heading`), so coasting past turns it once and stopping there never flips it back (kid
  playtest 4, n1). Once all three candies are collected and a Check names the chair, the arrow points out the door where
  that run is no longer than leaving the chair's 20 px beam (x <= 40); from farther right it would cross the beam
  for longer than a catch takes, so it points at the table (kid playtest 6, M1, and its review); within reach of the named spot it shows FIND COVER
  too, never HOLD TO HIDE, and that furniture's gold in-reach strip stays dark. A finger on it away from
  cover, the named spot included (pressed or slid on), gets
  the non-word "no" from the frame it arrives and for as long as it stays there or an armed hold lasts (drifted
  off the sign or not) with no cover, and `Game::REFUSE_MS` (1 s) after, so a quick tap shows it: the sign shakes
  3 px side to side and three CREAM chevrons march toward cover in place of the words, plain in any single frame (a
  2 px shake alone went unseen, kid playtest 2, m2). A press that stopped just short of cover with the finger
  down showed a calm sign, which a 6-year-old took for hiding (kid playtest 4, m1). The "no" is skipped only
  while an armed hold really rolls it into cover within 0.5 s at the current tilt (`arriving()` rolls a copy,
  friction included): it just hides on arrival. Speed alone promised that for a pumpkin just levelled, which
  stops short (kid playtest 3, m1).
- Goal: roll over three candy corns (within 7 px, never while hidden), then reach the
  door at x=24 out in the open. No timer: staying hidden stalls, it never wins. Candy
  sits at 52..66, 94..108 and 130..136, drawn from the round's seed.
- Grandma loops Rest -> Warn -> Search. The first rest is 3 s and the first move is
  always a plain 5 s sweep from her side (right). Later rests are 2.5..4.5 s and each Warn
  picks a move: Sweep (30%, 5 s), Fast sweep (20%, 3.2 s), TurnBack (20%, 4 s, covers 60%
  of the lane and returns) or Check (30%, 2 s on one spot, about 73% the pumpkin's own
  spot when it's hidden). Sweeps start from her side two times in three. Warnings last 2 s,
  3.5 s for a Check. Detection shares `beamX` with the drawn cone, 20 px half-width; a
  hidden pumpkin is never seen (nothing hides in the spot a Check names). Continuous exposure
  for 900 ms catches, so mashing the sign during a Check of its own spot is refused and caught;
  hiding elsewhere or leaving the beam resets exposure. PHEW! shows for 1.2 s when a search
  ends after the beam was on the pumpkin, never while she is still looking: shown as the
  beam merely passed, it told a 6-year-old to let go mid-search.
- Rustle: hidden and tilted past 400 mg for 1 s straight (SHH!, shown from 300 ms so a brief bump
  doesn't flash it) means Grandma heard it (SHE HEARD YOU!, while hidden): her next move, after
  the teaching opener, is a Check of that spot, with the usual 3.5 s warning, which startles it
  out. The tilt counts only once it has dropped below 400 mg since that hide began (`settled`), so
  rolling into cover at full tilt while held is never heard. The rustle itself never ejects the
  pumpkin and never shortens a rest.
- Ghost: once a round, tapping the ghost at the window ({22,30,24,24}) knocks something
  over across the room (WHAT WAS THAT?, 1.5 s). A warning or search stops and becomes a
  5 s rest; a rest gets 4 s longer. A ghost during a warning hands the announced move back
  (`moves--`, and a heard Check restores `heard`), so the teaching opener and the heard check
  still come next. Grandma's move counter, not elapsed time, keeps the first move a plain
  sweep even after the ghost stretches the first rest. She stays drawn, looking at the crash
  (no beam), while WHAT WAS THAT? shows. The ghost glows, bobs and waves only in Play; on
  every other page it is drawn calm, since taps there do nothing.
- Play label, most urgent first: RUN AWAY! (seen during a Check, where hiding can't help: mashing
  included) / QUICK! HIDE!, SHE HEARD YOU! (heard, hidden and still rustling), SHH!, WHAT WAS THAT?,
  PHEW!, THE CHAIR?/TABLE?/PLANT? (RUN AWAY! in reach of the named spot), GRANDMA!, HMM?, SHE HEARD
  YOU! (heard and hidden), HIDDEN!, TO THE DOOR!. While the ghost
  can be tapped, the row at y 49 is centred at x 92 so a 14-letter plate (x 46..136) clears its
  tap box; once it's used the row centres at 80, off Grandma's glasses.
- Endings vary: four catch lines, four pie captions and four escape lines, picked by
  `seed % QUIPS` when the round ends.
- `newRound(seed)` seeds candy and moves (xorshift32). The game seeds from the
  millisecond clock and a play counter; tests and tours are deterministic because
  simulated time is. `host/test_pumpkin.cpp` plays 100 seeds with a bot, including one
  reacting 0.7 s late, to keep every round winnable. A clingy bot that never lets go while hidden is startled
  out in most rounds and never caught; it doesn't always escape within its 180 s, since staying hidden stalls.
- `game.*`: Intro -> Calibrate -> Play -> Catch (600 ms) -> Icing (1600 ms) -> Lost,
  or Play -> Won (escaped). Retry starts Play with the current grip. Each launcher visit
  starts Intro and requires calibration again. A jolt delays a pending start
  until `tilt::Steady` reports plausible gravity.
- All pages use `ui::FreshGate`. Home is always visible at {68,8,24,22}; during
  Play it requires a 600 ms hold, elsewhere a tap. Action is {40,119,80,24}. During Play
  the action sign hides while held (`holding_`: set on the first frame a visible finger is on it,
  whether it pressed there or slid on from the scenery, the ghost or Home within reach of cover (never the
  spot a Check names; still held when the Check ends, it hides there), and
  kept until that finger lifts; a 6-year-old presses 1-3 s and slides a resting thumb on, and a kid playtest on 2026-11-13
  was caught under his own finger when HIDE acted on lift). The ghost acts on the lift of a press of
  any length that began and ended on it, so a press slid from the ghost onto the sign only hides. A
  thumb already resting on the glass when the round began (from READY or TRY AGAIN) does neither:
  `FreshGate` hides it until it lifts (`restingThumb`). Home never hides or emerges. Only the action sign changes hiding state.
- Time comes from the monotonic millisecond input, with frame gaps capped at
  100 ms; the wall clock is unused and absence never advances a round.
- `render.*`: original crisp pixel art on a 160px logical grid expanded 3x onto
  the RGB565 surface with its own palette and font. It composes in the runtime's existing 160x160 indexed scratch buffer using its own 19-color table, then `gfx565::expandIndexed3()` writes the final 480x480 buffer once. This keeps artwork pixel exact while removing full-resolution overdraw and extra framebuffer allocation. No borrowed game assets.
  Frosting starts at the upper circular edge. Art and text stay within the glass.
- Two raster assets, both generated by Codex's image tool (prompts in `tools/gpt_art.py`):
  the YOU LOST pie (`art/pie.png` -> `pie.h`, 112x54 box) and the window ghost
  (`art/ghost.png` -> `ghost.h`, 20x20 box, palette plus sky). `tools/gpt_art.py` converts each
  onto the 160px grid, snapped to the game's palette with a 1px ink rim. `make art`
  regenerates them; `make check` fails if a header is stale. Never hand-edit `pie.h` or
  `ghost.h`. To change one, replace its PNG (pure #00FF00 background) and review it on the
  real screen before committing. The used ghost is the same sprite redrawn two-tone
  (BLUE, ink as NIGHT), not a separate asset.
- `host/test_pumpkin.cpp`: rules, grip/reentry, input gates, clock behavior and
  round-screen rendering. Scripted emulator tours live in `tests/playtests/`.

Screen names are prefixed `pumpkin_`. Debug reports position, hiding, elapsed
round time, exposure, calibration readiness, candy, Grandma's phase/move and PHEW, plus
`ghost` (used), `distract` (crash showing), `heard` (spot, or -1), `rustle` (building) and
`startle` (the scared second), `refused` (the sign's "no" showing) and `scared` (`named()`: in the open by the
named spot); it provides no gameplay cheats.
See `docs/superpowers/specs/2026-10-17-pumpkin-hide-and-seek-design.md` at repo root.

## Visual regression checks

Install Pillow in the Python environment used by make (`python3 -m pip install pillow`)
before running the screenshot checks; CI already includes it.

Eight full-size golden PNGs in `goldens/` capture intro, calibration, hiding, victory,
catch, two frosting stages and the pie. `make check` compares their pixels with the
win/loss tours and checks crisp 3x scaling inside the glass. After an intentional
art change, run `make snap && python3 tools/pumpkin_goldens.py --update` from the
app directory, review all eight images, then commit them with the source change.
The emulator applies its physical circular mask at the outermost edge; images are
stored at 480x480 without resampling.
