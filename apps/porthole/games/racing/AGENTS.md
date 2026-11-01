# Rally: agent brief

A short, silent arcade race for the round 480x480 Porthole: hold the puck like a steering wheel, collect Boost and
Rocket charges, jump ramps and race three rivals for three laps. Launcher name **Rally**, namespace `racing`; the save
holds each kid's best times for the leaderboard, the rivals' level for that kid on each track and the kid's difficulty
setting. Design package: `docs/superpowers/specs/porthole-racing/` at the repository root (gameplay brief, the four
tracks, platform contract, and the two approved reference boards the look follows).

## Player contract

- Every launcher entry starts at the four track cards (Desert Canyon, Industrial Ruins, Snow Track, Night Run), all
  open, with Home, Times and the difficulty chip under them ("Rivals" over Auto / Easy / Normal / Hard, a user request of
  2026-10-31; a tap cycles it and checkpoints it at once (`saveUrgent`), per kid, Auto by default; a quick double tap
  skips a setting, by design: it cycles). Easy races at level -8, Normal at level 0 on every track (each track's
  `LEVEL_SHIFT` sets how strong that is: a competent driver wins 45-73%), Hard at `HARD_LEVEL` (by track: 3 on Desert
  Canyon, 2 on Snow Track, 1 on Industrial Ruins, 4 on Night Run; a competent driver wins 33-43% there, and both the
  competent and the kid driver win Hard less often than Normal, `modeBands`); a fixed setting never moves the
  adaptive levels. Times count the same in every setting. A finish in a fixed setting on a never-raced track still
  makes it raced, so its seeded level stays as it was and is no longer derived from the other tracks. The first card of a visit shows "How to race" (steer, pickups, three laps, hold the lap chip to pause); later
  cards go straight to the grip page.
- Grip: hold the puck upright like a wheel, steady for 350 ms, then Go. Flat or implausible gravity never passes.
  Clockwise steers right; 3-degree dead zone, full lock at 20 degrees, about 100 ms smoothing (Starfighter's
  validated math, kept local in `controls.h`). During a race the puck may lean back up to about 75 degrees from upright
  or turn hard and keep racing (`controls::held`: at least 250 mg of gravity in the screen plane, a user decision of
  2026-10-31: pausing at 49 degrees caught kids mid-race). Only a nearly flat or implausible pose for 300 ms stops it
  on the Pause page ("Hold like a wheel"); Resume works once it is held again.
- Countdown 3-2-1-GO: nothing moves and no press spends a charge before GO. The car accelerates by itself.
- The speed: an arc of 24 slots along the top rim lights left to right (green to gold to orange up to the dry-road top
  speed, cyan in the boost range past it), with mph digits under it (top speed reads 90, a boost about 121). Held back
  by a slow patch, off-road or a hit, the lit slots and digits turn red-orange, so a kid sees what a trap costs (a user
  decision of 2026-10-31, after round-watch arc gauges and arcade tachometers; the design brief had no speed readout).
  The slot of every arc pixel is computed once into the heap scratch; a frame only looks it up.
- Two drawn gameplay buttons, Rocket at 9 o'clock and Boost at 3 o'clock on the rim (moved off the road view at the
  user's request, 2026-10-31). Each shows its icon and a 0-3 count; spent buttons stay visible, dotted. One fresh
  press spends one charge; holding, dragging in from the road or a carried touch never fires. Boost: 1.35 speed cap
  for 1.5 s; a press during a boost is ignored. Rocket: 0.45 s cooldown, flies straight down the road, can miss.
- Road pickups (cyan bolt = Boost, orange rocket = Rocket) add one, capped at three; at three the gate stays for a
  later lap and "Full" shows by the button. Raised pickups over a ramp are only reachable in the air.
- Rocks, crates, barriers, rivals and rockets cost speed once per contact (floor rule:
  `min(before, max(0.25, before * factor))`) with 0.8 s protection, never damage, lives or removal. A rock, crate,
  snow block or barrier hits only where it is drawn: its picture is exactly as wide as its hit box (Industrial's
  narrow crate stack drawn twice side by side), and it hits only while drawn level with the car (`CAR_REACH`;
  `obstaclesHitWhereDrawn` in `host/test_racing.cpp`), a fix of 2026-10-31 after a dodged hazard still hit. Two cars
  touch the same way, only when their pictures overlap (dists under `CAR_REACH` apart and closer than both
  half-widths; `carsTouchWhereDrawn`): a car following a rival with road showing between them is never bumped. Mud, oil, slush,
  puddles and off-road cap speed; ice softens steering. Ramps give a fixed 0.7 s hop over continuous road.
- Hold the "1/3 Hold" lap chip 600 ms to pause; sliding off cancels, and the opening touch never presses a menu
  button. Pause: Resume (1 s "Ready" cue, then the exact same race), Recenter (grip page, race frozen), Restart,
  Tracks, Home; the last three ask "This race ends." Yes/No.
- Finishing three laps locks place and time once, after a 1.5 s "Finish!" on the line. Results show the place, the
  kid's time ("New best!" when it beat their best race or best lap on this track) and the track's **leaderboard
  across every profile on the device**: each kid's best race time and best lap, fastest race first, this kid's row in
  gold (a user decision of 2026-10-31: no medals, no ghost mode). The same board opens without a race from Times on
  the track cards (a user request of 2026-11-01): "Best times", the track between two arrows that go round all four
  (starting on the last one picked), the fastest race and the fastest lap across every kid in green, then the rows;
  "No times yet" on a track nobody has finished; Back to the cards. Looking saves nothing. Laps run line to line, the first from the flying
  crossing just after GO. The grip page shows the time to beat ("To beat: Zoe 1:31.2", or "Your record ..."). Fourth
  place is a finish like any other. Replay starts a fresh race on the same track and grip.
- In Auto, rivals adapt to each kid on each track, never shown (a user decision of 2026-10-31): a hidden `level`
  from -16 up to the track's top (14 on Desert Canyon, 15 on Industrial Ruins, 19 on Snow Track, 16 on Night Run:
  `LEVEL_MAX` 21 less the track's `LEVEL_SHIFT`). A win adds one, two after a runaway
  (the kid led for 3/5 of the race). A loss at 0 or below takes two; from 1..3 it drops to 0; above that, while the
  level's rung (below) is 9 or less, straight to -2 (there rivals only boost to pass a kid who leads, so a kid stuck
  behind would get the same race again; at -2 they are slower); from a higher rung it takes three. So a losing streak
  from the top goes 14, 11, 8, 5, 2, 0 on Desert Canyon; 15, 12, 9, 6, 3, 0 on Industrial Ruins; 19, 16, 13, 10, 7,
  -2 on Snow Track; 16, 13, 10, 7, 4, -2 on Night Run. A competent driver settles at 55-70% wins (the staircase's last
  40 races: Desert Canyon 55%, Industrial Ruins 60%, Night Run 65%, Snow Track 70%). Only a finish counts;
  Restart, Tracks, Home or leaving never move it. A track never finished starts one level below the kid's highest
  level elsewhere, inside 0..3 (a level, so a higher rung by the track's shift).
- What a level does. Below 0, on every track, the rivals' top speed drops, gently first and faster deeper down (to
  0.87 of their pace on Desert Canyon at -16, 0.80 elsewhere: Desert's straights let a kid who never steers finish
  within 18%), and from -7 a rival with the kid close behind it (2 m at -7, up to 16 m) moves out of the kid's lane.
  Separately each level is a rung of a charge-and-pace ladder: rungs 1-9 each give one rival one Boost charge (the
  slower two first, three at most each, like the kid's); rungs 10-15 raise the slower two by 0.01 (rival 2 stops at
  0.99: level with the pace-setter they form a wall); rungs 16-18 let a rival take its three charges back at each lap
  line, as the kid does from pickups. A track's rung is its level plus its `LEVEL_SHIFT` (7 on Desert Canyon, 6 on
  Industrial Ruins, 2 on Snow Track, 5 on Night Run), one rung a level all the way down (a rung at or below 0 gives
  nothing): the track's base strength, so level 0 (Normal) is a race a competent driver wins 45-73% of everywhere. A
  level just below 0 has both slower rivals and boosts. Neighbouring levels differ by at most 25 points of a competent
  driver's wins except at two known cliffs of 42-45 (`neighbourLevels`): -1 to 0, where even 0.35% off the
  pace-setter's top hands it the race, and Industrial Ruins' level 1 (rung 7, the pace-setter's first charge). A rival
  avoids an obstacle until level with it and a patch until past its end. A rival fires a charge only while the kid is 3-40 m ahead of it, never
  within 2.5 s of another rival's boost, with the kid's 1.35 cap and 1.5 s, and shows cyan exhaust flames half as big
  again as the kid's orange ones. No rival's dry-road top exceeds the kid's; the slower two stay below 1.0. The
  ladder was ordered by a simulated sweep (`staircase` in `host/test_racing_rules.cpp`). The race stays a pure
  function of track, seed, level and inputs.
- Pro: rungs 19-21, the top three levels of each track (12-14 on Desert Canyon, 13-15 on Industrial Ruins, 17-19 on
  Snow Track, 14-16 on Night Run; kids asked that a mastered track get harder, 2026-10-31), leave the rivals as at 18
  and can each switch on one more Pro hazard (`Feature::rung`): two rocks on Desert Canyon (both from rung 19: the
  first alone made races easier for the competent driver once rivals varied) and a barrier on Night Run (rung 19; 20 and 21 repeat it); none on Industrial Ruins or Snow Track. Each is
  drawn in the track's own art, sits on the road 1% of a lap clear of every gate and ramp, out of their lane within 3%,
  and 1% clear of any hazard in an overlapping lane (a `static_assert` in `track.cpp`). One is kept only while the
  competent driver and a late, wobbly kid driver each place no better on its rung than on the one below
  (`proNoEasier`: 30 races each, within 2 places; the sweeps are in `docs/racing/pro-playtest/README.md`). A crate and
  an oil patch on Industrial Ruins, a second barrier on Night Run, a third rock on Desert Canyon and a snow block on
  Snow Track made a race easier for one of the drivers and were dropped. The bots rarely win at rung 18 on Desert Canyon
  and Night Run (a few percent in the sweeps), so Pro is for kids beyond them (accepted). A loss at the top drops below the Pro rungs,
  so the hazards stay off until the kid climbs back (by design). In simulation a narrower road made races easier
  (rivals jam) and sharper bends changed nothing, so neither is used.
- Every race is a new field (a user request of 2026-11-01: each color used to finish in the same place every time).
  The three colors are dealt to the three parts (slowest, middle, pace-setter: pace, grid slot, lane, ladder rungs)
  in a seeded shuffle drawn from its own mixed stream (consecutive runs' seeds would otherwise rotate the colors in a
  fixed cycle; the entry seed mixes in the clock, so a power cycle does not repeat the first race). The parts keep
  their car slots, so the shuffle never touches the balance. Each rival also gets its day: the slower two's pace
  within +-0.035 of their part's (capped at 0.99; below level 0 never under the track's floor, so a kid who never
  steers still finishes fourth), how much it cuts a bend (0.2-0.5 of the road), how often it drifts off line (one new
  line in 2-6), and whether it steers round slow patches (three in four; the slowest part never does). The
  pace-setter keeps the kid's pace: its form could only ever slow it, which made Normal easier everywhere. So the
  slower two often swap, and the middle rival beats the pace-setter in about 5-15% of races on Desert Canyon and Snow
  Track, 35-60% on Industrial Ruins and Night Run (`tune.h` `RIVAL_*`, `variety` in `host/test_racing_rules.cpp`).
- No sounds, no network. Leaving and coming back starts at the cards.

## Module map

- `tune.h`: every tuning number from the brief (normalized speed, meters, road half-widths), the 20 ms step.
- `track.h` / `track.cpp`: the four circuits as tables (curvature sections, pickup gates, ramps, slow/ice patches,
  obstacles) in percent of a lap; `curve()` feeds both physics and drawing. A Pro hazard carries the rung that
  switches it on; race and drawing skip it below that rung (`active`), never copying the table.
- `race.h` / `race.cpp`: the pure race: cars, rockets, pickups, swept gate/ramp/finish crossing, contacts, rival
  lines, ranking. No input or drawing dependencies.
- `controls.h`: the wheel and its calibration.
- `game.h` / `game.cpp`: the App and its pages, fresh-touch rules, pause/resume/recenter, grip loss.
- `render.h` / `render.cpp`: the pseudo-3D road, things on it, the HUD and menu pages; hit boxes in logical px.
- `pictures.h`, `skyline0-3.h`: generated by `tools/pictures.py` from `art/*.png` (Codex images made with both
  reference boards attached; prompts and provenance in `apps/porthole/docs/racing/art.md`). `pic.h`: their shared type.
- `fonts.h`: generated by `tools/font.py` from `art/fonts/RussoOne-Regular.ttf` (SIL OFL 1.1, license beside it): four
  sizes (22, 28, 44, 64 px) in the runtime's anti-aliased format (`os/font.h`). Rendering pins Pillow 12.3.0;
  `--check` (in `make lint`) catches a changed font file with any Pillow. It replaced a pixel face that read poorly
  (a user decision of 2026-10-31).
- `assets.h`: generated by `tools/art.py`: each track's road colors.
- `save.h`: the Save (below), `seal`, `loadBlob`, `record`. Header-only.

## Save layout

`Save` (`save.h`), 52 bytes, NVS namespace `"racing"` (never renamed once shipped), key `s<profile id>`, written by
the shell. Magic `0x594C4C52` ("RLLY"), `version` 2, `size`, then `best[4]` by track index (`TRACK[]` order, never
reordered): `{raceMs, lapMs}`, 0 = not driven, each kept only when a finish beats it; v2 adds `int8_t level[4]`, the
adaptive level by track, then `uint8_t mode` (`Mode`: 0 Auto, 1 Easy, 2 Normal, 3 Hard; never renumbered; a value past Hard loads as Auto) and 3
pad bytes; a v1 44-byte blob loads with every level 0 and Auto. A 48-byte blob from the unshipped first v2 is rejected. `crc` last. A finish that moves the level, or a
tap on the chip, is saved even without a new best. Append-only: a new
field goes right before `crc` with a version bump, and `loadBlob` must keep loading every older size. A blob with a
wrong magic, size, version or crc is ignored (no times shown for that kid), never half-loaded. On entry the game reads
every profile's save for the leaderboard and writes only the current kid's.

Host tests: `host/test_racing_rules.cpp` (race rules, every track finishes for idle, steady and busy drivers, the
idle one last at every level, the level rules, rival boosts, the `competent` driver and the `kid` driver (300 ms late,
bends at 70%, a wobbly line), the competent-driver `staircase`, `modeBands`, `proHazards` (on from their rung only)
and `proNoEasier` (each Pro rung no easier than the one below for each driver), `neighbourLevels`, `variety`) and `host/test_racing.cpp` (wheel,
pages, touch rules, pause/resume, grip loss, frame-rate invariance, replay, text and targets on the glass, save and
levels, `obstaclesHitWhereDrawn`, `carsTouchWhereDrawn`, `proHazardsDrawn` and `firstTapAfterEntry`). Playtests:
`tests/playtests/a0_racing_flow.txt`, `a1_racing_monkey.txt`, `a4_racing_board.txt`, `a5_racing_level.txt`,
`a6_racing_mode_kids.txt`, `a7_racing_times.txt`, `a8_racing_field_variety.txt`, `a9_racing_times_fingers.txt`,
`a10_racing_top_losses.txt`, `a11_racing_short_top_losses.txt`; the black-box playthroughs
are `docs/racing/adaptive-playtest/` and `docs/racing/pro-playtest/`.

## Screens and test hooks

Screens: `race_tracks`, `race_howto`, `race_calibrate`, `race_countdown`, `race_play`, `race_pause`, `race_confirm`,
`race_results`. `debug` prints `screen track phase lap place boost rocket speed x dist`, then `race_ms finished
finish_ms cars_done jumps pickups fired hits impacts`, then `best_race best_lap new_best kids level mode race_level` (this track; `level` the stored adaptive
level, `race_level` the one the next race runs at), then `steer grip_lost resume ready calibration neutral`.
`dbg level N` sets this kid's adaptive level on the current track (clamped to the track's range) and saves it; on a track the
kid has never finished, the next entry or finish derives it again (one below the best elsewhere).
`dbg track N|tracks|calibrate|go|lastlap|lastplace`: `go` starts a race past the countdown from the current grip,
`lastlap` puts the player 60 m from the finish, `lastplace` also brings every rival home first. Fixtures only, not
evidence of natural play.

## Hardware validation still required

Frame time and memory on the board (`M` over serial), wheel comfort in small hands and the 20-degree full lock have
not been measured on a device. The emulator proves layout, rules and flow only.
