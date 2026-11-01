# Rally rival variety: black-box playtest (emulator roleplay)

This was emulator roleplay by an independent playtester agent. No real child played, and no physical board was
used. The playtester did not read `games/racing/` or the racing host tests. It read `host/sim.cpp`'s header (the
`--serve` protocol), the previous Rally playtest reports and their pixel-steering helper, and the existing racing
playtest scenarios' screen names. Rally's `debug` line was read once, after all the races, to look for a rival
field. It has none.

## Build

- Branch `claude/brave-einstein-ipuhqp`. The races were played at `3a4e8d5` (feat(porthole): Rally rivals get a new
  field every race), `build/host/snap` sha256 `bcbc786af349df1e9913a58e2e433ef2679835979fa4c20e4160362a1f1637d0`.
  That binary was copied and frozen before play.
- During the session the branch moved to `797513f` (refactor: rival parts as `tune.h` tables) and `6634127` (test
  only). The rebuilt `snap` (sha256 `12156042fccd0334a35d95ded7193ff8036185aa94c787aa63fa500f9214b99b`, built
  19:17 UTC) got a short confirmation session: 2 finished races plus the start-grid probes. The working tree also
  had uncommitted Rally edits from another agent (game.cpp, render.cpp: a Times view, according to the updated
  brief). Those edits are **not** covered here.

## Persona and brief (as received, unchanged)

> Persona: Sam, 8, a confident reader who has raced Rally a dozen times. Sam's complaint (relayed by a parent): "the
> cars always finish in the same place by color, so every race is the same". Sam steers imprecisely (overcorrects,
> sometimes drifts into the rails), presses Boost randomly, fires rockets at whatever is ahead, and loses interest
> fast when a race feels like a replay.
> What changed (player-visible claim to test): each race the three rival cars should come out differently. Colors
> shouldn't hold fixed finishing places, and which rival leads the pack should change from race to race. Races
> should still feel fair: Easy winnable for a sloppy driver, Normal a real contest, no rival unbeatably fast.
> Race at least 6 times across at least 2 tracks, using Replay and the track cards [...] Try Easy once or twice with
> sloppy steering, and Normal a few times [...] Avoid the `dbg` fixtures for judging outcomes.

## Setup and inputs

- Fixture `fixture.txt` (`--fresh --script`): `reset`, `tilt 0 1000 0`, `profile Sam 8`. After that, `snap --serve`
  ran in its own directory and was driven only by `down/up/tick/tilt/skip/frame`. No `dbg` was used anywhere.
- Every menu tap was chosen from a rendered 480x480 frame. The route was the picker, the launcher's page 2, Rally,
  the Rivals chip (Auto to Easy to Normal), a track card, How to race / Next, the wheel test / Go, and then Replay
  or Tracks on the results page.
- Steering used `drv.py`, a pixel-only helper adapted from the adaptive-playtest driver. It reads the road colour,
  the red car's x position and road curvature ahead, and outputs a wheel angle every 99 ms with fast (0.85)
  overcorrecting smoothing. Sloppiness comes from a low gain (0.45-0.6), Gaussian noise of 20-30 degrees and, at
  noise 25 or more, a slow 6.5 s weave. Boost was tapped at random intervals (`BOOST_S` 6-8 s). Rocket was tapped
  whenever a rival-coloured car showed in the lane ahead. R11 used a steadier Sam (gain 0.9, noise 8).
- To tell which colour was ahead, the helper also logged green, blue and yellow car pixels in the road band. The
  log is `cars-ahead.log.gz`; `summ.py` turns it into a far-to-near order every 3 s. Every claim below was checked
  against the frames (1 s apart for the first 25 s, then every 2.5-3 s).
- Traces: `session-sam-main.json.gz` (R1-R11 plus the guard-rail probe) and `session-sam-confirm.json.gz` (the
  refactor build). Each is the raw command list after the fixture. To replay one, copy a `snap` next to `drv.py`
  as `snapf` (or set `SNAP=`), gunzip the trace and run `python3 drv.py session.json` with no actions.
- One wall-clock jump was used: `skip 620` after the shell's "Sam is resting. Back in 10 min" screen, in place of
  a real 10-minute wait.

## Per-race table (first-play evidence, build `3a4e8d5`)

Grid is listed front to back, so the first car is furthest ahead and the last starts just in front of Sam. "Early"
covers about the first 25 s. Rival places a kid could not see are marked unseen.

| # | Track | Rivals | Sam | Time | Grid | Early (who led) | Rivals' finish, as far as visible |
| --- | --- | --- | --- | --- | --- | --- | --- |
| R1 | Desert Canyon | Easy | 1st | 1:31.5 | B Y G | Blue. Sam 1st by about 12 s | unseen: alone for 80 s |
| R2 | Desert (Replay) | Easy, sloppier | 1st | 1:36.2 | G Y B | Green, which passed Sam back at about 15 s | unseen: alone after 20 s |
| R3 | Desert (Tracks) | Normal | 2nd | 1:36.0 | G B Y | Green and blue ahead, with yellow climbing from the back to lead by the end of lap 1 | **Yellow 1st** (out of sight); Sam passed blue in lap 2 and green earlier, so G/B were 3rd/4th |
| R4 | Desert (Replay) | Normal | 1st | 1:32.0 | B Y G | Green pulled out front with Sam | **Green 2nd** after a duel: it passed Sam in lap 1 and was beside him in lap 3 to the line. B/Y unseen |
| R5 | Snow Track | Normal | 1st | 1:55.4 | G Y B | Pack of three. Sam boosted through it at about 15 s | unseen: alone for 1:40 |
| R6 | Snow (Replay) | Normal, sloppiest | 4th | 2:09.0 | G B Y | Yellow at 10 s, then a blue+yellow pair | B/Y pair 1st/2nd (unseen order); **green 3rd** (swapped places with Sam in lap 1-2) |
| R7 | Industrial Ruins | Normal | none | none | B Y G | Out of sight | Did not finish. The helper misread the road and left Sam outside the right rail at 50 mph, and the shell's rest screen ended the race in lap 2 |
| R8 | Industrial (Tracks) | Normal | 3rd | 1:57.4 | Y B G | Green+blue pulled away | G/B 1st/2nd (unseen order); **yellow 4th** (ahead of Sam for two laps, passed in lap 3) |
| R9 | Industrial (Replay) | Normal | 4th | 1:55.3 | G Y B | Blue+yellow pulled away | B/Y 1st/2nd; **green 3rd** (visible just ahead all race) |
| R10 | Night Run | Normal | 3rd | 1:53.0 | G B Y | Blue and yellow ahead. Green and Sam traded places in lap 1 | B/Y 1st/2nd; **green 4th** |
| R11 | Night (Replay) | Normal, steadier Sam | 1st | 1:46.4 | Y B G | Blue led lap 1. Lead swapped with Sam several times into lap 2 | Sam passed blue for good in lap 2; blue probably 2nd. Y/G unseen |

Confirmation on the refactor build (`12156042...`, fresh profile, Normal): Desert 2nd, 1:34.0 (grid B Y G, yellow
ahead for about 70 s); Desert Replay 1st, 1:31.0 (grid G Y B). A Snow race was cut short by the helper's time box
and is not counted.

Score: Easy 2/2 wins. Normal finished races: 1st x3, 2nd x1, 3rd x2, 4th x2.

## Does the variety claim hold from Sam's seat?

**Partly yes, where Sam can see it.** In Normal races where Sam is in the pack, the rival that matters changes:
- R3: yellow wins from the back of the grid.
- R4: green duels Sam to the line.
- R8: yellow is the straggler Sam beats.
- R9: green is the car Sam chases and never catches.
- R10: green is the car Sam beats.
- R11: blue is the leader Sam duels and passes.

The same colour finished 1st, 2nd, 3rd and 4th across races (yellow 1st in R3 and 4th in R8; green 2nd, 3rd and 4th
in R4, R9 and R10). The colour leading early changed too (blue, green, yellow and the blue+yellow pair), and nothing looked like a fixed
colour ranking. One pattern does show: the starting grid rotates on a fixed cycle (finding 5). The car on the back
row, just ahead of Sam, went green, blue, yellow, green, blue, yellow... on every start in an app session.

**But most of the claim cannot be seen.** The results page shows only Sam's place, time and the kids' best times.
It never shows how the rivals finished. In 5 of 10 finished races (R1, R2, R5, and the leaders in R6/R9/R10), the
cars that decided the order were out of sight for over a minute. Sam's complaint is about where the colours
*finish*. The game gives him no way to check, so he can only judge "who was in front of me" and "who did I race".

## Qualitative assessment (as Sam)

- **Goal and actions.** Clear to an experienced 8-year-old. The Rivals chip reads Auto / Easy / Normal on the
  track cards (Sam tapped it without help). The HUD place number (4th to 1st) responds at once to passes, and
  "Hit!" confirms a rocket. Replay is one tap. The how-to page comes back the first time Rally opens in each
  session, and Sam taps through it. In one run, Go landed on Next and Sam sat on the wheel test for 2 minutes
  (the helper's fault, but it shows the page can catch an impatient tap).
- **Decisions and feedback.** The best moments came from the change: R4's green duel to the line, R11's lead swaps
  with blue, and R9's green staying just out of reach for three laps. Rockets at the start pack ("Hit!" on every
  start) are fun. Boost is the main overtaking tool: in R5, one boost took Sam from 4th to 1st. Colours are easy to
  tell apart on all four tracks (`02-cars-close-quarters.jpg`).
- **Where it gets repetitive.** Whenever Sam leads by 10-15 s (R1, R2, R5), the next 80-100 s look the same every
  race, an empty road, no matter how the rivals were shuffled. Sam's "every race is the same" feeling would
  persist in those races. This was already reported in the adaptive-playtest pass and is not caused by this
  change. On Normal with sloppy steering (R6, R9, R10), the two leaders vanish and Sam fights only the third car.
  That is still a race, but who won stays a mystery.
- **Where Sam would stop.** After R5 (third race in a row alone in front, with Easy and Snow Normal both
  procession wins), Sam would switch tracks or quit. The Industrial and Night races (R8-R11) re-engaged him,
  because there was always a car close by. He would stop for good at the rest screen (R7). A natural stopping
  point is any results page.
- **Fairness.** Easy was won 2/2 by a very sloppy driver. Normal was a contest: 3 wins, 1 second, 2 thirds and 2
  fourths, and the place tracked how cleanly Sam drove. No rival looked unbeatable: the leader was passed in R4 and
  R11, and in R3/R6/R9/R10 the leader got away only after Sam's own crashes and slowdowns (50-60 mph readings).
  No car was seen pulling away from a clean driver. Hard was not tested.
- **Recovery.** Off-road and rail hits cost speed only. Steering back rejoined within 1 s (guard-rail probe,
  session trace). The rest screen loses the race in progress (R7). Leaving with pause, Home and reopen does not
  repeat the field.

## Findings

1. **Minor: Results page (`race_results`). Rivals' finishing order is never shown, so the change is mostly
   invisible.** Concern: feedback/comprehension. Repro: any race where Sam leads early or falls well behind, e.g. R1
   or R9 in `session-sam-main.json.gz`; the results show only "1st/4th, time, Best times". Screenshot:
   `01-races-grid-moment-result.jpg` (result column). Suggestion: list the field on the results page, e.g. four
   small car chips in finishing order with Sam's highlighted. That would let Sam see "Blue won this time".
2. **Minor (already reported, not caused by this change): Race. On Easy and on Snow Normal, Sam leads from about
   15 s and races alone for the remaining 80-100 s, so those races look the same whatever the shuffle.** Concern:
   boredom/pacing. Repro: R1, R2, R5. Screenshot: `01-races-grid-moment-result.jpg` (R5 moment).
3. **Minor (shell; already reported in adaptive-playtest item 11): Race to the rest screen. The session rest cuts a
   race mid-lap with no result.** Concern: recovery. Repro: R7, about 15 min into the session.
   Screenshot: `04-r7-off-rail-then-rest-screen.jpg`.
4. **Nit: Countdown. After a power cycle, the first race's grid matched the first race before it (`reboot-probe.txt`,
   `05-reboot-same-first-grid.jpg`). A fresh profile's first Desert grid was also identical on three different
   simulated days.** In `tests/playtests/a8_racing_field_variety.txt` the post-reboot grid did differ (after four
   starts). There are only 6 possible grid orders, and the race 10 s in had already diverged, so the impact is low:
   a kid who plays one race per power-on might see the same opening order. Concern: consistency.

5. **Minor: Countdown / grid. The starting grid follows a fixed rotation instead of a fresh shuffle.** Concern:
   meaningful variety/consistency. Grids for R1-R11, front to back: BYG, GYB, GBY, BYG, GYB, GBY, BYG | (rest
   screen, relaunch) YBG, GYB, GBY, YBG. The back-row car cycles G, B, Y every time, and green started at the front
   in 6 of 11 races. The confirmation build (BYG then GYB) and a separate fresh profile (GBY then BYG then GYB, by
   Restart) followed the same rotation. Race outcomes did vary *within* the same grid (R1 and R4 both BYG: blue
   led R1, green duelled in R4; R3 and R6 both GBY: yellow won R3, a blue+yellow pair led R6). So this is about how
   the start looks, not who wins. An experienced kid who looks at the grid would still spot the "green, blue,
   yellow" back row. Repro: `session-sam-main.json.gz`, or `tests/playtests/a8_racing_field_variety.txt` (its
   sheet). Screenshot: `01-races-grid-moment-result.jpg` (grid column), `03-start-grid-restart-reopen-reboot.jpg`.

No visual glitches were found with the cars, across about 1,000 frames sampled at 1-3 s intervals. Colours were
right, sprite depth order was right when cars were side by side, there was no flicker, and the HUD place matched
the cars in view. Frame-by-frame flicker was not checked (a gap).

Non-finding: in R7 and one confirmation race, the helper left Sam outside the guard rail at 50 mph. A manual probe
(tilt 30 degrees back) rejoined the road within 1 s, so the cause was the helper's misread, not a trap. The car
crossing through rails is pre-existing (blackbox `05-offroad-outside-guardrail.png`).

## Automated suite

`make -C apps/porthole playtest` (tree `6634127-dirty`, snap `12156042...`): **82/82 scenarios pass, 0 UI FAIL,
0 UI WARN**, every racing scenario included (a0, a1 monkey, a4, a5, a6). Added
`tests/playtests/a8_racing_field_variety.txt`: start grid across Restart x2, Home + reopen, and a reboot, as one
contact sheet (`03-start-grid-restart-reopen-reboot.jpg`). It passes with 0 FAIL / 0 WARN. The sim exposes no
rival state, so it asserts screens only. A reviewer looks at the sheet.

## Gesture / interruption matrix

| Case | Tested | Result |
| --- | --- | --- |
| Replay x6, Tracks card switch x4 | yes | grid differs from the previous race but follows a 3-step rotation (Minor 5) |
| Pause, Restart, Yes (x2) | yes | grid follows the same rotation |
| Pause, Home, Yes, relaunch | yes | new grid |
| Process restart (`reboot`) | yes | first grid repeated once, differed once (Nit 4) |
| Rest screen mid-race | yes (natural) | race lost (Minor 3) |
| Off-road / rail drift and recovery | yes | speed loss only |
| Hard setting, Auto setting over many races | no | gap |
| Two kids alternating | no | gap |
| Frame-by-frame flicker | no | gap |

## Suggested brief changes for a retest

- Say up front that the results page does not list rivals, so "rivals' finishing order" can only be inferred. Ask
  the playtester to also judge whether a kid *could* tell that the variety exists.
- Ask for a few races where Sam stays in the pack (or a steadier "good day" Sam). Leading by 15 s or trailing far
  hides the rivals and makes the comparison uninformative.
- Ask for the start grid of every race in order, so a rotation shows up without extra work.
- Add a power-cycle check (first race after `reboot`) and Hard, which this pass did not cover.
- Warn about the 15-minute session rest: plan about 5 races per session, or note the `skip` used to wait it out.

## Resolutions (implementer, 2026-11-01)

- Grid rotating on a fixed cycle (finding 2) and the first race repeating after a power cycle (finding 5): fixed. The
  colors are dealt from their own stream with the seed's bits mixed first, and the entry seed mixes in the clock.
  `tests/playtests/a8_racing_field_variety.txt` (added by this pass) now shows five different-looking grids.
- Results page doesn't list the rivals' order (finding 1): open, raised with the user.
- Races alone in front on Easy / Snow Normal (finding 3) and the rest screen mid-race (finding 4): pre-existing,
  out of scope.
- The driver helpers (`drv.py`, `summ.py`, `sheet.py`, `crop.py`) are not committed: they fail the repository's mypy
  (a duplicate `drv` module) and complexity gates. The traces and screenshots are kept.
