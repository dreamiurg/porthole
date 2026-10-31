# Rally: agent brief

A short, silent arcade race for the round 480x480 Porthole: hold the puck like a steering wheel, collect Boost and
Rocket charges, jump ramps and race three rivals for three laps. Launcher name **Rally**, namespace `racing`; the save
holds each kid's best times for the leaderboard and the rivals' level for that kid on each track. Design package: `docs/superpowers/specs/porthole-racing/` at the repository root (gameplay brief, the four
tracks, platform contract, and the two approved reference boards the look follows).

## Player contract

- Every launcher entry starts at the four track cards (Desert Canyon, Industrial Ruins, Snow Track, Night Run), all
  open. The first card of a visit shows "How to race" (steer, pickups, three laps, hold the lap chip to pause); later
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
  `min(before, max(0.25, before * factor))`) with 0.8 s protection, never damage, lives or removal. Mud, oil, slush,
  puddles and off-road cap speed; ice softens steering. Ramps give a fixed 0.7 s hop over continuous road.
- Hold the "1/3 Hold" lap chip 600 ms to pause; sliding off cancels, and the opening touch never presses a menu
  button. Pause: Resume (1 s "Ready" cue, then the exact same race), Recenter (grip page, race frozen), Restart,
  Tracks, Home; the last three ask "This race ends." Yes/No.
- Finishing three laps locks place and time once, after a 1.5 s "Finish!" on the line. Results show the place, the
  kid's time ("New best!" when it beat their best race or best lap on this track) and the track's **leaderboard
  across every profile on the device**: each kid's best race time and best lap, fastest race first, this kid's row in
  gold (a user decision of 2026-10-31: no medals, no ghost mode). Laps run line to line, the first from the flying
  crossing just after GO. The grip page shows the time to beat ("To beat: Zoe 1:31.2", or "Your record ..."). Fourth
  place is a finish like any other. Replay starts a fresh race on the same track and grip.
- Rivals adapt to each kid on each track, never shown (a user decision of 2026-10-31): a hidden `level` in
  `LEVEL_MIN..LEVEL_MAX` (-16..18, `tune.h`). A win adds one, two after a runaway (the kid led for 60% of the race).
  A loss at 0 or below takes two; from 1..9 it drops straight to -2 (there rivals only boost to pass a kid who leads,
  so a kid stuck behind would get the same race again); above 9 it takes three, so from 18 a losing streak is below 0
  after four losses (15, 12, 9, -2). A competent driver settles near two wins in three on every track. Only a finish
  counts; Restart, Tracks, Home or leaving never move it. A track never finished starts one below the kid's highest
  level elsewhere, inside 0..3. Below 0 the rivals' top speed drops, gently first and faster deeper down (to 0.87 of
  their pace on Desert Canyon at -16, 0.80 elsewhere: Desert's straights let a kid who never steers finish within
  18%), and from -7 a rival with the kid close behind it (2 m at -7, up to 16 m) moves out of the kid's lane. Level 0
  is the brief's pace. Above 0 a level is a rung of a ladder: rungs 1-9 each give one rival one Boost charge (the
  slower two first, three at most each, like the kid's); rungs 10-15 raise the slower two by 0.01 (rival 2 stops at
  0.99: level with the pace-setter they form a wall); rungs 16-18 let a rival take its three charges back at each lap
  line, as the kid does from pickups. Industrial Ruins and Night Run hand a capable kid a long lead, so their levels
  sit 12 and 8 rungs up (`LEVEL_SHIFT`, fading out by -8 so a struggling kid meets the same easy rivals on every
  track); their top levels are 6 and 10. A rival fires a charge only while the kid is 3-40 m ahead of it, never within
  2.5 s of another rival's boost, with the kid's 1.35 cap and 1.5 s, and shows cyan exhaust flames half as big again
  as the kid's orange ones. No rival's dry-road top exceeds the kid's; the slower two stay below 1.0. The ladder was
  ordered by a simulated sweep (`staircase` in `host/test_racing_rules.cpp`). The race stays a pure function of
  track, seed, level and inputs.
- No sounds, no network. Leaving and coming back starts at the cards.

## Module map

- `tune.h`: every tuning number from the brief (normalized speed, meters, road half-widths), the 20 ms step.
- `track.h` / `track.cpp`: the four circuits as tables (curvature sections, pickup gates, ramps, slow/ice patches,
  obstacles) in percent of a lap; `curve()` feeds both physics and drawing.
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

`Save` (`save.h`), 48 bytes, NVS namespace `"racing"` (never renamed once shipped), key `s<profile id>`, written by
the shell. Magic `0x594C4C52` ("RLLY"), `version` 2, `size`, then `best[4]` by track index (`TRACK[]` order, never
reordered): `{raceMs, lapMs}`, 0 = not driven, each kept only when a finish beats it; v2 adds `int8_t level[4]`, the
rivals' level by track (a v1 44-byte blob loads with every level 0); `crc` last. A finish that moves the level is
saved even without a new best. Append-only: a new
field goes right before `crc` with a version bump, and `loadBlob` must keep loading every older size. A blob with a
wrong magic, size, version or crc is ignored (no times shown for that kid), never half-loaded. On entry the game reads
every profile's save for the leaderboard and writes only the current kid's.

Host tests: `host/test_racing_rules.cpp` (race rules, every track finishes for idle, steady and busy drivers, the
level rules, rival boosts and the competent-driver staircase) and `host/test_racing.cpp` (wheel, pages, touch rules,
pause/resume, grip loss, frame-rate invariance, replay, text and targets on the glass, save and levels). Playtests:
`tests/playtests/a0_racing_flow.txt`, `a1_racing_monkey.txt`, `a4_racing_board.txt`, `a5_racing_level.txt`; the
adaptive rivals' black-box playthrough is `docs/racing/adaptive-playtest/`.

## Screens and test hooks

Screens: `race_tracks`, `race_howto`, `race_calibrate`, `race_countdown`, `race_play`, `race_pause`, `race_confirm`,
`race_results`. `debug` prints `screen track phase lap place boost rocket speed x dist`, then `race_ms finished
finish_ms cars_done jumps pickups fired hits impacts`, then `best_race best_lap new_best kids level` (this track), then `steer grip_lost resume ready calibration neutral`.
`dbg level N` sets this kid's level on the current track for the next race (clamped) and saves it; on a track the
kid has never finished, the next entry or finish derives it again (one below the best elsewhere).
`dbg track N|tracks|calibrate|go|lastlap|lastplace`: `go` starts a race past the countdown from the current grip,
`lastlap` puts the player 60 m from the finish, `lastplace` also brings every rival home first. Fixtures only, not
evidence of natural play.

## Hardware validation still required

Frame time and memory on the board (`M` over serial), wheel comfort in small hands and the 20-degree full lock have
not been measured on a device. The emulator proves layout, rules and flow only.
