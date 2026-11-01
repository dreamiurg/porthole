# Rally Pro road hazards: black-box playthrough (emulator roleplay)

This is emulator roleplay by an independent playtester agent. No real child played and no physical board was used.
The playtester did not read `games/racing/` source or the racing host tests. It read `docs/racing/art.md` (the art
prompts, to tell `rock.png` from the impact `puff.png`), the earlier passes in `../adaptive-playtest/` and the `dbg`
names in `tests/playtests/a5_racing_level.txt`. It knew the change from the brief, so this is not a fresh
first-time-comprehension check.

## Build and setup

- Branch `claude/rally-pro-obstacles`, commit `67918f2` (feat(porthole): Rally Pro obstacles at the top levels).
  `build/host/snap` sha256 `d3b2f0e554c2ab56a824d43ed1c67e02605b6d1e833456ac58f6e969a912457c`, frozen as `snapf`.
- Fixture `../adaptive-playtest/fixture.txt` (`reset`, `tilt 0 1000 0`, `profile Zoe 9`, `profile Sam 6`), then
  `snap --serve` driven through `down/move/up/tick/tilt/frame`. Menu taps were chosen from rendered frames. Every
  driver process re-sets `tilt 0 1000 0` before the grip page (the held-tilt artifact from earlier passes).
- Driver: `drv2.py` here, a copy of `../adaptive-playtest/drv.py` with five additions, all reading rendered pixels
  only: a hit log (the HUD speed turns red, plus the 1.5 s of frames before it), a fine-frame window (`FINE=a-b`),
  a lane offset (`OFF=px`), a "see anything" dodge (`kidsee`: any non-road, non-marking colour on the road ahead
  steers the car about 80 px to the side; `SEEY` is how far ahead it looks, `SEEW`/`SEEN` the window and threshold),
  and a car-x jump log (meant for oil; on Snow it only logs wobble, ignored). The race runs used
  `SEEY=260 SEEW=60 SEEN=2` (reacts about 0.5 s ahead at 90 mph). Replays are deterministic: one re-run frame
  matched the first run byte for byte.
- "Solo" runs: after `Go`, the car is steered onto the left shoulder for 14 s (50 mph) so the rivals drive away,
  then it drives clean laps with nothing in front. That isolates the hazards from rival occlusion.
- Session traces (JSON command lists, about 4 MB) are kept, uncommitted, under `build/pro-playtest/`. Frames and the
  frozen binary were deleted afterwards because disk was tight. Repro: start from a
  session that ends on a track's results page, then e.g.
  `RALLY_DRV_DIR=build/pro-playtest SEEN=2 SEEY=260 SEEW=60 python3 docs/racing/pro-playtest/drv2.py S.json
  "tilt 0 1000 0" "dbg level 21" "tap 52 111" "tick 3000" "drive 105 X 1000 smooth 1 0 kidsee"`.

## Persona and dispatch brief (as received, unchanged)

> Persona: Zoe, 10, very good, has climbed to the top. Natural play to the top isn't feasible in a session, so use
> the fixture `dbg level 19`, `20`, `21` (supplementary, say so) and race each track at 21 (and Industrial at 19 and
> 20). Judge: are the new hazards visible in time to react at speed (90 mph, boosting at 120)? Do they feel fair
> (never a blind hit behind a crest, a ramp landing, a gate, or a rival)? Do they look like they belong to the track?
> Does the race feel harder/more interesting, not just punishing? Any hazard you hit repeatedly no matter what?
> Also one natural race on Desert from level 0 to confirm nothing changed at ordinary levels.

What's new, per the brief: at the top adaptive levels a few extra hazards appear (rocks on Desert, a crate and an oil
patch on Industrial Ruins, a snow block on Snow, barriers on Night Run), using each track's existing art.

## Races

Natural play (no `dbg`; Zoe's profile, fresh):

| # | Track | Level | Driver | Place | Time | Notes |
| --- | --- | --- | --- | --- | --- | --- |
| N1 | Desert | 0 | rival dodge | 1st | 1:31.3 | Same as passes 1-4 (1:30.3-1:31.4). No new hazard on the road. Old rock, mud, cones as before. |
| N2 | Industrial (first visit) | 0..3 | rival dodge | 3rd | 1:48.7 | Existing dark oil patches, drums. No crate in the lanes. |
| N3 | Snow (first visit) | 0..3 | see-anything | 3rd | 1:59.6 | |
| N4 | Night (first visit) | 0..3 | see-anything | 4th | 1:56.9 | |

Supplementary (hidden state: `dbg level N` on the results page, then Replay):

| # | Track | Asked → raced at | Driver | Place | Time | Slowdowns* | Pro hazard hits |
| --- | --- | --- | --- | --- | --- | --- | --- |
| S1 | Desert | 21 | rival dodge (no hazard dodge) | 3rd | 1:34.3 | 15 | 2 rocks, one hidden behind the green rival until about 0.2 s before contact |
| S2 | Desert | 21 | see-anything dodge | 3rd | 1:34.3 | 6 | 2 rocks (one late swerve, one phantom: finding 1) |
| S3 | Industrial | 19, 20, 21 → **9** (all three) | see-anything | 3rd | 1:50.5 | 17 | crates 3+ (two with the crate still drawn ahead), oil 3 |
| S4 | Snow | 0 | see-anything | 2nd | 1:57.4 | 7 | — |
| S5 | Snow | 21 | see-anything | 4th | 1:58.4 | 6 | 0 (one hit with nothing visible: finding 4) |
| S6 | Night | 21 → **13** | see-anything | 3rd | 1:51.2 | 19 | barriers 3, one after it had visibly passed (finding 1) |

*HUD speed turning red: rival bumps, mud/oil/puddles, kerbs and hazards together. The helper bumps rivals far more
often than a good 10-year-old would; read places and times as direction, not a balance measurement.

Solo and probe runs (supplementary):

| Probe | Result |
| --- | --- |
| Where the new hazards start (solo laps, the same line at each level, diffed frame by frame) | Desert rocks at 19+ (none at 18); Snow block at 19+ (none at 18); Industrial crate at **7+** (none at 6); Night barrier at **11+** (none at 10). |
| `dbg level` ceiling per track (read with `debug`) | Desert 21, Snow 21, Industrial **9**, Night **13**. A 1st place at Industrial 9 stays 9. Asking for 19-21 on Industrial or Night races the same race as 9 / 13 (S3: three byte-identical runs). |
| Desert 21 solo, three fixed lanes (offset -70/-35/+35/+70 px) | Every fixed lane takes about two hazard hits per lap (rocks on all three lanes; the mud patch too on the far left). The rocks sit left, centre and right. |
| Desert 21 solo, see-anything dodge reacting about 0.5 s ahead | 0 hits in 2.5 laps. Reacting later (0.3 s ahead): 2-3 hits. |
| Desert 21 solo, boost (122 mph) into the centre rock | Visible about 0.8 s before contact (about 1.1 s at 90 mph). The 0.5 s-ahead dodge did not clear it; a no-dodge run hit it too. |
| Snow 21 solo, five lanes (-70..+70 px from the road centre) | No snow-block hits. The block sits at the right edge of the road, on the kerb line, just after a left bend, visible about 1 s. The kerb line itself was not driven (two tries at +100/+130 px went off the road earlier). |
| Industrial 9 vs 0 solo, left lane, diffed | The new oil patch is in the left lane; it looks exactly like the existing ones (black with a purple edge) and is visible about 1.2 s ahead. It is wide and easy to see. |
| Losses from Desert 21 (`dbg lastplace`) | 21 → 18 → 15 → 12 → 9 → -2 → -4 → -6 → -8. A win at 21 stays 21. Now `tests/playtests/a7_racing_top_losses.txt`. |

## Findings

1. **Major. Where a Pro hazard hits does not match where it is drawn, so a dodged hazard can still hit.** (Screen:
   race. Concern: fairness, feedback.) Two directions, both on curves:
   - Early and wide: the puff and slowdown fire while the rock or crate is still drawn about a car length ahead and
     15-20 px to the side of the car (no overlap). The slowed car then drives past it with clear road between them
     (`01` Desert rock, `02` Industrial crate).
   - Late and wide: on Night Run the car passed a barrier with about 55 px of road between them (nearly a car width),
     and the hit fired two frames (0.2 s) after the barrier had gone by, with nothing near the car (`04`).

   Zoe would say "I missed it and it still got me." That is the opposite of what the new hazards need: a reward for
   seeing and dodging. For comparison, the Desert rock that already exists at level 0 also registers while drawn a car
   length ahead, but with about 15 px of overlap, so it reads as a corner clip. Every clear-gap case I saw was a Pro
   hazard. Whether the new hazards' hit boxes or curve placement differ from the old rock's is for the owner to check.
   The Pro levels add three to five hazards per lap, so this happens several times a race.
   - Repro (Desert, `01`): Desert results page, `dbg level 21`, Replay, `drive 80 X 0 smooth 1 0 kidsee` with
     `SEEN=2 SEEY=260 SEEW=60 FINE=200600-202200`. The hit is at t=200959 (session start = the Desert results page
     after N1, t=120838).
   - Repro (Industrial, `02`): Industrial results page, `dbg level 9`, Replay, `tick 3000`, `tilt -342 940 0`,
     `tick 2000`, `tilt 0 1000 0`, `tick 12000`, `drive 20 X 0 smooth 1 0` with `FINE=394900-396300` (hit at
     395597).
   - Repro (Night, `04`): Night results page, `dbg level 13`, Replay, `tick 3000`, `drive 75 X 0 smooth 1 0
     kidsee` with `FINE=739600-740700` (hit at 740310).
   - Screenshots: `01-desert-rock-hit-before-contact.png`, `02-industrial-crate-hit-from-a-car-length-away.png`,
     `04-night-barrier-missed-then-hit.png`.

2. **Major (scope question for the owner). On Industrial Ruins and Night Run the top level is 9 and 13, not 21,
   and the Pro hazards start at 7 and 11.** (Concern: consistency with the brief, difficulty curve.) The brief says
   19-21 on every track. In the build, Industrial and Night cap lower, so "Industrial at 19 and 20" is not a different
   race (S3). The hazards still appear only at each track's top three levels. If that is the design, the brief and
   docs should say "the top three levels of each track". If not, Industrial's crate and Night's barriers appear much
   earlier on the adaptive ladder than Desert's rocks. Repro: on the Industrial card, `dbg level 21` then `debug` →
   `race_level=9`; a `dbg lastlap` win at 9 stays 9.

3. **Minor. A rock can sit hidden behind the rival directly ahead until about 0.2 s before contact.** (Concern:
   fairness.) S1, t=137632: the green rival drove over the spot (its own puff showed first), and the rock was under
   or behind it. The rock was not distinguishable before green's puff. With three rocks a lap on Desert and rivals
   bunched in lap 1, this happens. A kid who follows a rival into a puff and then gets slowed will not know why.
   Repro: Desert results page after N1, `dbg level 21`, Replay, `tick 3000`, `drive 105 X 500 smooth 1 0 kiddodge`
   (hit at 137632).

4. **Minor. One hit on Snow 21 with nothing visible near the car.** (Concern: feedback.) S5, t=530511: a puff and a
   slowdown at the car's nose with clear road ahead. The green rival was beside and behind the car, and something
   small and white showed above it a few frames before. The cause is unknown (a rival rocket?). It did not happen at
   level 0 on the same start. Repro: Snow results page, `dbg level 21`, Replay, `tick 3000`, `drive 10 X 0 smooth 1
   0 kidsee` with `SEEN=2 SEEY=260 SEEW=60 FINE=529800-530800`.

5. **Minor. The Industrial crate reads as scenery.** (Concern: comprehension.) At racing distance the crate is a
   narrow, dark-rust stack on a dark road at dusk. Its yellow-and-black stripes don't show until it is close, and it
   looks like the roadside oil drums. It is visible about 0.6 s ahead and is narrow, so it is easy to misjudge. Rock,
   snow block and barrier read clearly as "something on the road" (`03`). Screenshot: `03-pro-hazards-on-each-track.png`
   (second panel).

6. **Nit. After one loss at the top, the hazards vanish.** A loss from 21 lands on 18 (Desert/Snow), below the
   19+ threshold, so the next race has no Pro hazards. A win brings them back. A kid may notice "the rocks were there,
   now they're gone". Harmless, and arguably kind.

Not findings (checked): hazards look like they belong to their track (desert boulder, rusty crate stack, snow-capped
chevron block, red-and-white road-works plank with amber lamps; `03`). In the frames inspected (1 s overviews of every race, fine frames around each hit), no new
hazard sat right behind a crest or a ramp landing; this was not checked systematically. One Desert rock sits just past the end of a mud patch, but the car is already slowed and the rock is
visible ahead, so it reads fair. Gates (the start/finish checker) are clear. The Industrial oil patch is easy to see
(about 1.2 s) and reads like the existing oil. The Snow block was never met on lines up to 70 px right of centre;
if it sits outside every line a kid drives, it adds no challenge (not shown either way). Level-0 Desert is unchanged (N1).

## Qualitative assessment (Zoe, 10)

- **Goal and feedback.** Unchanged from earlier passes: place, lap and speed are clear. The new hazards need no
  words, and a 10-year-old who reached the top knows "don't hit things". A hit shows a puff and a red speed number,
  which is readable feedback. Finding 1 undermines it: when the puff fires beside a hazard she dodged, the feedback
  teaches nothing.
- **Decisions and skill.** At 90 mph the rocks, snow block and barriers are visible about 1 s ahead and dodgeable
  with one lane change made about 0.5 s ahead (0 hits in the solo dodge run). That is a real skill test for a good
  driver. Boosting at 120 mph cuts the warning to about 0.8 s: boosting through the rock section becomes a gamble,
  which is an interesting decision. With rivals bunched, a lane change into a rival is the other risk. Races at the
  top are harder: Desert went from 1st (1:31.3) at level 0 to 3rd (1:34.3) at 21.
- **Fairness.** Mostly fair to see, unfair to touch (finding 1). Rival occlusion (finding 3) and the unexplained
  Snow hit (finding 4) add a few "what hit me?" moments.
- **Repetition and stopping.** The hazards sit in fixed places each lap, so by lap 2 Zoe knows where they are. That
  is a satisfying mastery loop for her, and she would happily replay a track she can now dodge cleanly. The point she
  would stop is a lost race caused by phantom hits: two of those in a row at the top and she would switch track or
  quit. No forced waits were added.
- **Recovery.** A hit costs about 30 mph and recovers in about 2 s. Nothing is lost beyond place; a loss drops the
  level 3 at a time. Pause, Home and Replay behave as in earlier passes (not retested this pass).

## Gesture and interruption matrix

| Case | This pass |
| --- | --- |
| Menu taps (picker, launcher arrow, Rally tile, track card, Next/Go, Replay, Tracks) | Each acted once |
| Second track in the same session | Skips How to race and goes straight to the grip page; a tap aimed at Next landed on Go and started the race (expected, harmless) |
| Pause hold, Home mid-race, rest screen, reboot mid-race, drag off a button | **Gap** this pass (passes 1-4 cover them; nothing in this change touches them) |

## Automated checks

`make -C apps/porthole playtest` at `67918f2` plus the new scenario: 83 of 83 scenarios pass, 0 UI FAIL, 0 UI WARN.
The racing scenarios a0-a7 pass. New: `tests/playtests/a7_racing_top_losses.txt` (a win at 21 stays 21, then five
losses go 21 → 18 → 15 → 12 → 9 → -2).

## Limitations

- Emulator roleplay only. The pixel helper is not a 10-year-old: it bumps rivals often, reacts at a fixed distance,
  and does not learn hazard positions across laps. Zoe would learn them by lap 2, so the race places overstate the
  difficulty.
- Whether the Industrial oil patch makes the car slide was not measured (the car-x log was too noisy). Its look and
  warning time were checked.
- S1 and S2 both finished 3rd in 1:34.3 (+3.0) with different drivers (15 vs 6 slowdowns). Both results screens were
  read from separate runs; the coincidence was not chased.
- Every top-level race is supplementary (`dbg level`). No race reached the top naturally.
- `dbg lastlap` writes its 2-4 s finish to the leaderboard as a best time (fixture only, not a kid's path).

## Verdict

**Needs another pass.** The hazards are visible early enough, fit their tracks and make the top races harder in a
way a good 10-year-old would enjoy. Finding 1 is what blocks "ready": a dodged hazard can still cost the race, which is
the fairness the brief asks about. Finding 2 does not block by itself. If the per-track caps are the design, the
impact is a curve question: crates after about four wins on Industrial, rocks after about ten on Desert. It needs an
owner decision and a brief fix. Replay Desert, Industrial and Night at their top levels after a fix to finding 1.

## Resolutions (game-engineer, after this pass)

| Finding | Resolution |
| --- | --- |
| 1 Major: hits do not match the drawing | **Fixed in the shared obstacle code**, for every rock, crate, snow block and barrier. Root cause, two parts: (a) along the road an obstacle hit while the car was anywhere within half a car length of its whole authored span (about 5-7 m, 0.6% of a lap), so it fired with the obstacle still drawn about a car length ahead, and up to 0.2 s after it had been passed (Night); (b) sideways it hit at its authored half-width (0.16-0.18 of a road half-width) while its picture was narrower (the crate stack 0.06, the barrier 0.14). Now an obstacle hits only while its middle lies between the car's position and the top of the car's picture (`CAR_REACH`, 2.85 m), and it is drawn exactly as wide as it hits (the crate stack twice side by side). A car held off it eases out at 4 half-widths/s (was 2.5) so the shorter window still clears it. `obstaclesHitWhereDrawn` (`host/test_racing.cpp`) passes every obstacle on every track with 4 px of clear road between the pictures (no hit) and 4 px of overlap (one hit, while drawn level with the car). |
| 2 Major: top levels on Industrial and Night | **Design kept and documented**: Pro is each track's top three levels (Desert/Snow 19-21, Industrial 7-9, Night 11-13). A loss there now steps down three (the drop is keyed on the rung), so one loss no longer sends an Industrial kid from 7-9 to -2. |
| 3 Minor: rock hidden behind a rival | Accepted, no change: any obstacle can be behind a rival. |
| 4 Minor: unexplained Snow slowdown | Accepted, no change: likely a rival rocket. Snow Track has no Pro hazard after this fix (below). |
| 5 Minor: the crate reads as scenery | Partly addressed by fix 1: the crate is drawn at its hit width now, two stacks side by side, so it reads wider. Its art is unchanged. |
| 6 Nit: hazards vanish after a loss at the top | By design: a loss at the top drops below the Pro levels. |

Pro hazards after fix 1, re-swept (60 races each, seeds 9000 + 7919k; win share / mean place):

| Track | Driver | 18 | 19 | 20 | 21 |
| --- | --- | --- | --- | --- | --- |
| Desert Canyon | competent | 0% / 2.90 | 0% / 3.07 | 0% / 3.23 | 0% / 3.32 |
| Desert Canyon | kid | 0% / 2.98 | 0% / 3.08 | 0% / 3.23 | 0% / 3.23 |
| Industrial Ruins | competent | 41% / 1.73 | 26% / 2.20 | same | same |
| Industrial Ruins | kid | 25% / 2.08 | 13% / 2.43 | same | same |
| Snow Track | both | no Pro hazard | | | |
| Night Run | competent | 10% / 2.72 | 3% / 2.92 | same | same |
| Night Run | kid | 5% / 2.82 | 0% / 2.98 | same | same |

Dropped after fix 1 because a driver placed better on their rung than on the one below: the Industrial mid-road crate
(kid 2.08 → 2.03), the Snow Track snow block (competent 3.47 → 3.42) and the Night mid-road barrier (kid 2.82 → 2.70).
`proNoEasier` in `host/test_racing_rules.cpp` keeps the rule with both drivers (30 races a rung, ties allowed).

## Pass 2 (after the hit-where-drawn fix)

Same playtester agent as pass 1, so again not a fresh first-time-comprehension check. Emulator roleplay only; no real
child and no board. `games/racing/` source was not read. To tell a periodic slowdown's cause apart, the pre-fix build
(`e66d277`) was built from `git archive` in the session scratch directory, run once, and deleted.

- Build: `claude/rally-pro-obstacles` at `71c43b7`, `build/host/snap` sha256
  `32bfe63fce242518cf6baa699100093f0c4f93895faad671b40478443cda0a93`.
- Brief (as received): retest finding 1 with the pass-1 repros, deliberate near-misses and grazes at 90 mph and
  boosting; base game: Sam 6 (imprecise) one natural race on Desert and one on Snow at level 0, Zoe 10 one on
  Industrial at level 0; Zoe at the top (`dbg level`): Desert 21, Industrial 9, Night 13; a loss from Industrial 9.
- The pass-1 session traces were lost with `build/`, so the exact repros (Desert t=200959, Industrial 395597, Night
  740310) were **not replayed**. They were replaced by races on the same track and level plus lane, graze and boost
  probes (solo, after letting the rivals go), every hit inspected at the frame where the HUD speed turns red.
- Driver: `drv2.py` unchanged, plus a wrapper (session scratch, not committed) that also saves a frame whenever
  something that is not road is just beside or above the car, so near-misses can be inspected. Sam's runs use the same
  "see anything" dodge reacting later (`SEEY=290`), gain 0.8 and 6 degrees of steering noise. Menu taps were chosen
  from rendered frames: picker Zoe (80,55) / Sam (80,81), launcher arrow (96,135), Rally (80,104), cards Desert
  (52,55), Industrial (106,55), Snow (52,104), Night (106,104), Next/Go (106,123), Replay (52,111).
- Session traces (JSON, uncommitted) are under `build/pro2/`.

| # | Who | Track | Level | Driver | Place | Time | Hits (all causes) |
| --- | --- | --- | --- | --- | --- | --- | --- |
| P1 | Zoe | Industrial | 0 (natural) | see-anything | 3rd | 1:49.2 | 10: oil 4, rivals 3, kerbs 2, the base crate stack 1 (overlap at the car's corner) |
| P2 | Zoe | Industrial | 9 | see-anything | 3rd | 1:49.1 | 9: oil 3, rivals 4 (one, t=193988, with the green rival beside and about 25 px clear: finding 1), kerbs 2; no crate |
| P3 | Zoe | Desert | 21 | see-anything | 3rd | 1:34.3 | 9: one Pro rock (overlap), rivals 2, kerbs 3, mud 2, one unclear kerb/dirt |
| P4 | Zoe | Night | 13 | see-anything | 3rd | 1:50.8 | 12: barrier 1 (overlap), puddle 1, rivals 2, one probable rival rocket, kerbs 7 (the helper hugged the left kerb in lap 2). At t=34142 the speed went red with no puff as a barrier passed about 11 px clear on the left, with the right wheels on the kerb: a kerb scrape, but a kid could blame the barrier |
| P5 | Sam | Desert | 0 (natural) | late dodge, noisy | 3rd | 1:35.0 | 19, about 13 of them rival bumps every ~5 s while following yellow (finding P2-1) |
| P6 | Sam | Snow | 0 (natural) | late dodge, noisy | 2nd | 1:56.6 | 12 in lap 1, mostly rival bumps and ice |

Probes (solo, fixed lane offset from the road centre, no dodge):

| Probe | Result |
| --- | --- |
| Desert 21, lanes -55 / 0 / +55, 3 laps | Every rock hit fires with the rock drawn level with the car and touching or overlapping a corner (`05`). Rocks passed on other lanes go by with clear road and no hit. |
| Desert 21, rock A at +12/+24/+36/+46 | +12..+36 hit, +46 clear. At the hit frame the rock is beside the car's roof with at most about 13 physical px (about 4 logical) of road between them, mostly under the puff; +46 passes with about 14 px and no hit (`06`). Nit P2-3. |
| Desert 21 boosting (122 mph), lanes 0/30/46/60 | Same: hits touch or overlap (0, 30; rock B at 60), 46 passes about 15 px clear (`05`, lower row). |
| Night 13, lanes -50/0/+50, 3 laps | Barrier hits at +50 overlap the car's top corner by about 15 px (`05`, upper row). The pass-1 "hit 0.2 s after it passed" did not recur. |
| Industrial 9, lanes -50/0/+50 and +15..+45, 3 laps | The Pro oil patch (left lane) slows the car only when driven into; it is wide and visible. The crate stack sits by the right kerb and was not reached by lanes up to +45; P1 hit it with about 10 px overlap. |
| Sam Desert t=34142 (rock passed, then a slowdown) | 33 ms frames: the base rock passes on the right with clear road and no hit; the slowdown 0.2 s later comes with the yellow rival diagonally ahead, puff on that side. A rival bump, not the rock. |
| Losses from the top (script, `dbg lastplace`) | Industrial 9 -> 6 -> 3 -> 0 -> -2; Night 13 -> 10 -> 7 -> 4 -> 1. A hands-off loss (car scraping the kerb at 55 mph, 2:56.9) from Industrial 9 lands on 6 and the results page says "Finished! 4th ... Your best yet!" New scenario `tests/playtests/a8_racing_short_top_losses.txt`. |

### Pass-2 findings

1. **Major (pre-existing, not a regression; outside this change). A rival "bumps" the kid's car with visible road
   between them.** (Screen: race. Concern: fairness, feedback.) Following a rival, the slowdown and puff fire while the
   rival is still about 20-25 physical px (7-8 logical) ahead of the car's nose, and repeat every time the car catches
   up again: Sam's Desert race had about 13 of these at a steady ~5 s, Snow about 10 in lap 1. Steering does not
   matter (three different steering settings gave the same hit times), and the pre-fix build `e66d277` gives
   byte-identical hit times, so it predates the obstacle fix. It is the same "I didn't touch it" moment the fix removed
   for obstacles, and for a 6-year-old who follows a car it is far more frequent than any Pro hazard. Ask: rivals should
   follow the same hit-where-drawn rule. The start-grid side bump at t=11009 in every race looks like the same family
   (a rival beside, puff with little or no overlap). Repro: fresh fixture, Zoe or Sam, Desert, level 0, Go, `drive 60
   ZX 0 smooth 1 0` (hits at 42260, 47309, 52259, ...). Screenshot: `07-pass2-rival-tailgate-bump-with-gap.png`.
2. **Minor. Pro on Industrial adds almost nothing.** Zoe's same driver: 3rd 1:49.2 at level 0, 3rd 1:49.1 at
   level 9. One oil patch, easy to see and drive around, is not a felt difficulty step for a good 10-year-old. Desert
   21 (three rocks) and Night 13 (barrier) do change the line she has to drive. Owner's call; the sweep in the
   resolutions table shows a measurable effect for the bots.
3. **Nit. Grazes are judged within about 4 logical px.** A rock beside the car's roof with up to about 13 physical px
   of road showing (under the puff) can still register, while about 14 px passes. That is within one frame of motion
   at 90 mph and reads as a clip, not a phantom. No change needed unless the owner wants the test's 4 px tightened.
   Screenshot: `06-pass2-desert-graze-threshold.png` (left: +12 hit, middle: +36 hit, right: +46 clear).

### Pass-1 findings, status

| Pass-1 finding | Status in pass 2 |
| --- | --- |
| 1 Major: hits do not match the drawing | **Fixed** for every obstacle met: Desert rocks (base and Pro, 90 and 122 mph), Night barrier, Industrial crate stack and oil, Desert mud, Night puddles. No obstacle hit with a car-length gap, none after the obstacle had passed, no pass-through with overlap. Residual graze tolerance: Nit 3. |
| 2 Major: top level 9 / 13 | Design documented; losses now step down three (checked, `a8`). |
| 3 Minor: rock behind a rival | Accepted by owner; not re-seen. |
| 4 Minor: unexplained Snow hit | Snow has no Pro hazard now. The unexplained hits seen this pass were rival bumps (finding 1 above) or a rival rocket on Night. |
| 5 Minor: crate reads as scenery | Better: the base crate is two stacks side by side and reads as a road block (P1, t=40016-40214). |
| 6 Nit: hazards vanish after a loss | By design; unchanged. |

### Qualitative (pass 2)

- **Sam, 6, level 0 (Desert, Snow).** The rocks, mud and cones look the same kind of thing as before; the wider crate
  is the only obstacle that looks different. Obstacles he hits, he can see he hit. What he cannot explain is the
  repeated bump behind a rival car (finding 1): on Desert it is the main thing slowing him, about every five seconds
  in laps 2-3, and the HUD speed goes red with nothing touching. He still finished 3rd and 2nd with "Your best yet!", so
  nothing is lost, but this is where he would get cross ("it keeps crashing by itself"). Obstacles are not too easy
  to dodge for him: with a late, noisy reaction the base rock still clipped him once on Desert.
- **Zoe, 10, at the top.** Desert 21 is the real Pro challenge: three rocks in fixed places, a dodge made about 0.5 s
  ahead clears them, boosting through them is a gamble, and a clip now always looks like a clip. Night 13's barrier is
  fair and readable. Industrial 9 feels like Industrial 0 (finding 2). She would replay Desert to learn the rocks; the
  pass-1 stop point (phantom hits costing a race) is gone for obstacles.
- **Losing at the top** is gentle: one loss costs three levels, a results page with no blame, Replay one tap away.

### Gaps

Pause hold, Home mid-race, rest screen, reboot mid-race and drag-off were not retested (nothing in this change touches
them). No physical board. Every top-level race is `dbg level` (supplementary). The Snow kerb-line hazard and the
Industrial crate graze threshold were not probed lane by lane.

### Automated

`make -C apps/porthole playtest` at `71c43b7` with the new scenario: 84 of 84 scenarios pass, 0 UI FAIL, 0 UI WARN
(monkeys under the sanitizers included).

### Verdict (pass 2)

**Ready for the obstacle change.** Finding 1 of pass 1 is fixed: every obstacle hit seen this pass happens where the
obstacle is drawn, at 90 mph and boosting. The rival bump with a visible gap (pass-2 finding 1) is Major for the base
game, but it predates this branch (identical on `e66d277`) and needs its own fix and playthrough; it should not be
read as a regression of this change.

## Resolutions after pass 2 (game-engineer)

| Finding | Resolution |
| --- | --- |
| P2-1 Major: a rival bumps with road visible between the cars | **Fixed** for every pair of cars (kid and rival, rival and rival): two cars touch only while their pictures overlap, their dists under `CAR_REACH` (2.85 m) apart (was `CAR_LEN`, 4.5 m) and closer than both drawn half-widths. `carsTouchWhereDrawn` (`host/test_racing.cpp`) holds a car behind a rival with one clear row of road (no bump) and with a little overlap (one bump), and alongside with 4 px between them and 4 px of overlap. Before/after frame of the first bump of a hands-off Desert race (the old build bumped with the blue rival about 4 m ahead, drawn clear of the car) is in the session scratch. Balance: rivals as traffic cost the kid much less, so on Desert Canyon and Snow Track the brief's level-0 rivals lost 95% and 83% of races to the competent driver and Normal failed its band; Normal now races at a per-track level (`NORMAL_LEVEL`: 9 on Desert, 2 on Snow, 0 elsewhere; 50-65% wins) and Hard moved to 10 on Desert and 4 on Snow (35%). Auto is unchanged and settles at 58-80% wins. |
| P2-2 Minor: Industrial Pro adds little | Accepted: one oil patch is the only Industrial item that passed the gate. After the contact fix it still makes the two drivers together place worse (60 races: competent 2.02 to 1.90, kid 2.12 to 2.37 mean place; `proNoEasier` compares the sum within 2 places). |
| P2-3 Nit: grazes judged within about 4 logical px | Accepted as a clip: the car's drawn half-width (0.156) and its hit half-width (0.16) differ by under a pixel; the rest is the rock's transparent corners. |
| Night kerb scrape read as a barrier hit | Accepted, no change: off-road caps speed without a puff; the barrier passed clear. |

Pro after the contact fix (60 races, seeds 9000 + 7919k; win share / mean place):

| Track | Driver | 18 | 19 | 20 | 21 |
| --- | --- | --- | --- | --- | --- |
| Desert Canyon | competent | 10% / 2.70 | 0% / 2.92 | 3% / 3.02 | 0% / 3.12 |
| Desert Canyon | kid | 0% / 2.97 | 1% / 2.98 | 0% / 3.20 | 0% / 3.22 |
| Industrial Ruins | competent | 30% / 2.02 | 31% / 1.90 | same | same |
| Industrial Ruins | kid | 18% / 2.12 | 15% / 2.37 | same | same |
| Night Run | competent | 5% / 2.75 | 1% / 2.92 | same | same |
| Night Run | kid | 1% / 2.70 | 0% / 2.90 | same | same |
