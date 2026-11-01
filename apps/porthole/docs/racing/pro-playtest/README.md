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
| Losses from Desert 21 (`dbg lastplace`) | 21 → 18 → 15 → 12 → 9 → -2 → -4 → -6 → -8. A win at 21 stays 21. Now `tests/playtests/a10_racing_top_losses.txt`. |

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
The racing scenarios a0-a7 pass. New: `tests/playtests/a10_racing_top_losses.txt` (a win at 21 stays 21, then five
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
| Losses from the top (script, `dbg lastplace`) | Industrial 9 -> 6 -> 3 -> 0 -> -2; Night 13 -> 10 -> 7 -> 4 -> 1. A hands-off loss (car scraping the kerb at 55 mph, 2:56.9) from Industrial 9 lands on 6 and the results page says "Finished! 4th ... Your best yet!" New scenario `tests/playtests/a11_racing_short_top_losses.txt`. |

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
| P2-1 Major: a rival bumps with road visible between the cars | **Fixed** for every pair of cars (kid and rival, rival and rival): two cars touch only while their pictures overlap, their dists under `CAR_REACH` (2.85 m) apart (was `CAR_LEN`, 4.5 m) and closer than both drawn half-widths. `carsTouchWhereDrawn` (`host/test_racing.cpp`) holds a car behind a rival with one clear row of road (no bump) and with a little overlap (one bump), and alongside with 4 px between them and 4 px of overlap. Before/after frame of the first bump of a hands-off Desert race (the old build bumped with the blue rival about 4 m ahead, drawn clear of the car) is in the session scratch. Balance: rivals as traffic cost the kid much less, so on Desert Canyon and Snow Track the brief's level-0 rivals lost 95% and 83% of races to the competent driver and Normal failed its band; Designer's call: Desert and Snow get a `LEVEL_SHIFT` (9 and 2) like Industrial and Night, so Normal stays level 0 everywhere (competent wins 50-65%) and Auto settles at 57-65%; Hard is 2, 7, 2, 4 by track (35-40%). Tops are now Desert 12, Industrial 9, Snow 19, Night 13, with Pro on each track's top three levels. |
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

## Pass 3 (after the car-contact fix and the per-track retune)

Same playtester agent as passes 1-2, so again not a fresh first-time-comprehension check. Emulator roleplay only; no
real child and no board. `games/racing/` source was not read.

- Build: `claude/rally-pro-obstacles` at `94a58e5`, `build/host/snap` sha256
  `34a6f3804645ef3f62792b8172c84d4a33f5a39f4f02a38faf17ad0308ec154a`, frozen as `build/pro3/snapf` for every run.
- Brief (as received): (1) the pass-2 repro (fresh fixture, Desert level 0, Go, `drive 60 ZX 0 smooth 1 0`: any bump
  with visible road between cars? the grid side-bump at t~11009); (2) natural races on Auto from fresh profiles: Sam 6
  (imprecise) Desert and Snow, Zoe 10 Industrial and Night, two each: fair and not crushing for Sam now that rivals
  start stronger on Desert, challenging for Zoe; (3) the Rivals chip: Sam on Easy on Desert once (still winnable?),
  Zoe on Hard on Desert once; (4) top levels via fixture: Desert 12, Night 13.
- Driver: `drv2.py` unchanged; Sam = see-anything dodge reacting late (`SEEN=2 SEEY=290 SEEW=60`, gain 0.8, 6 degrees
  of noise), Zoe = `SEEN=2 SEEY=260 SEEW=60`, gain 1, no noise; both tap Boost/Rocket like a kid. Menu taps chosen from
  rendered frames, as in pass 2, plus the chip (101,130) and Tracks (108,111) on the results page. To inspect a bump
  frame by frame, the session was replayed to just before it and the recorded steering re-sent in 33 ms steps with a
  frame after each (exact trajectory). Traces (JSON) under `build/pro3/`, uncommitted; frames deleted for disk.
- Unsuccessful attempt, kept for the record: the first Snow (Sam) and Night (Zoe) races ran in the same driver process
  as the previous track, so the helper steered with the previous track's road colour, drove off-road at 50 mph and
  finished 4th in 2:38.8 / 3:06.4. A driver artifact (`drv2.py` learns the road colour once per process), not the game;
  both were rerun from the grip page in a fresh process (SS1/ZN1 below).
- Levels: read with `debug` from a raw-script replay only where no race had run yet (script-mode replay of a race
  diverges from `--serve`, so the post-race Auto levels are **not verified**). `dbg level 12/13` was confirmed
  (`race_level=12`, `13`).

| # | Who | Track | Rivals | Place | Time | Speed-loss events (HUD red) |
| --- | --- | --- | --- | --- | --- | --- |
| R | hands centred | Desert | Auto, level 0 | - | 60 s | 9: three side/corner bumps with blue in lap 1, then six rear-ends on yellow every ~5 s. All at contact (`08`) |
| SD1 | Sam | Desert | Auto, level 0 | 2nd | 1:33.4 | 10: rivals 7 (corner touches), mud 3 |
| SD2 | Sam | Desert | Auto | 2nd | 1:34.8 | 22: about 19 corner bumps on yellow every ~4 s in laps 2-3, mud, one after his own rocket slowed green |
| SS1 | Sam | Snow | Auto, level 0 | **1st** | 1:55.0 | 5 |
| SS2 | Sam | Snow | Auto | 3rd | 1:59.1 | 21: yellow corner bumps, ice |
| ZI1 | Zoe | Industrial | Auto, level 0 | 3rd | 1:50.4 | 11: oil 4, rivals, kerb |
| ZI2 | Zoe | Industrial | Auto | 3rd | 1:50.2 | 13 |
| ZN1 | Zoe | Night | Auto, level 0 | 3rd | 1:49.5 | 21: puddles, left-kerb scrapes, rivals |
| ZN2 | Zoe | Night | Auto | 3rd | 1:51.1 | 15 |
| SE | Sam | Desert | **Easy** | **1st** | 1:30.7 | 0; alone in front from lap 1 |
| ZH | Zoe | Desert | **Hard** (level 2) | 3rd | 1:33.3 | 4 |
| TD | Zoe | Desert | `dbg level 12` | 4th | 1:36.8 | 16: one Pro rock (overlapping), rivals, mud, kerb. Next level 9 |
| TN | Zoe | Night | `dbg level 13` | 4th | 1:51.1 | 18: one Pro barrier (overlapping), puddles, rivals, kerb |

### Pass-3 findings

1. **Minor. The first tap on Rally's track page, right after opening Rally from the launcher, is often ignored.**
   (Screen: `race_tracks`. Concern: feedback, input.) From Sam's session (picker, launcher arrow, Rally tile, 1.5 s),
   one tap on Industrial (106,55), Snow (52,104), Night (106,104), Home (40,130) or the Rivals chip (101,130) changes
   nothing, not even a pixel; the second tap works. A tap on Desert (52,55/52,60/70,55) works first time. In Zoe's
   session Industrial worked first time but the chip did not. After Back from a card, or Tracks from the results page,
   the first tap works. The chip is where Sam would notice: he taps "Rivals Auto", nothing happens, and he may decide
   it is not a button. Repro: `--serve`, fixture, `down 80 81`/`tick 66`/`up`, `tick 600`, the same at (96,135) and
   (80,104), `tick 1500`, then `down 106 104`, `tick 66`, `up`, `tick 700`: still `race_tracks`, frame identical
   (`drv2.py` session `build/pro3/samD.json` cut at t=2898). A raw `--script` run with `tap` does **not** reproduce
   it. **Not caused by this branch:** the same replay on `origin/main` `7dc6812` (this branch's base, built from
   `git archive` in the session scratch, run once, deleted) ignores the first tap on Night, Industrial, Snow, Home and
   the chip too; on the older `7c23bbf` (before the Rivals chip) all four cards work first time. So it arrived on main
   between those two commits and needs its own owner. Not checked on the board. Screenshot: `09-pass3-track-page-first-tap-ignored.png` (Night tapped once, chip tapped once).
2. **Nit (observation). A kid who drives on a rival's rear corner is bumped every 4-5 s.** (Concern: pacing.) SD2 and
   SS2 each had about 15-19 bumps on yellow in laps 2-3. Each one is a real touch now: at 33 ms the corners meet or sit
   within about 8 physical px (under 3 logical) as the bump fires, the pass-2 graze tolerance (Nit P2-3). Much of the
   repetition is the helper (its dodge parks the car just far enough over to clip yellow's corner), and Sam still
   finished 2nd. Because the bump now looks like a bump, he can learn to go round. No change asked. Screenshot:
   `08-pass3-rival-bump-at-contact.png` (lower row: yellow on the roof line, two 33 ms frames before and the frame of the
   bump).

Not findings (checked): no bump with clear road between the cars in any race or probe (about 70 rival bumps
from seven races looked at as before/after pairs, four at 33 ms). The start-grid side bump is gone: in the repro the first bump is
13 s after Go (about 10 s after the start lights), and none fired at the grid in any race. Pro rock (Desert 12) and Pro barrier (Night 13) hits fire
only with the hazard overlapping the car (`10`). Chip cycle Auto, Easy, Normal, Hard, Auto, and the pick shows on the
card page.

### Pass-2 findings, status

| Pass-2 finding | Status in pass 3 |
| --- | --- |
| 1 Major: a rival bumps with road visible between the cars | **Fixed.** The repro's nine bumps, and every rival bump inspected (SD1, SD2, SS2, ZI1, ZN1, TD, TN), fire at contact (`08`: blue at the kid's front corner, yellow on the kid's roof line, two 33 ms frames before the bump and the bump frame). The grid side-bump at t~11009 no longer happens. |
| 2 Minor: Pro on Industrial adds little | Not retested (Industrial's top was not in this brief). |
| 3 Nit: grazes within about 4 logical px | Unchanged and the same for car corners (finding 2 above). |

### Qualitative (pass 3)

- **Sam, 6, Auto.** Desert from a fresh profile: 2nd and 2nd with "Your best yet!" on the first; Snow: 1st, then
  3rd. Stronger Desert rivals are not crushing: with the gap-bump gone he is close to the front. The slowdowns he gets
  are ones he can see (a car he touched, mud, ice). The point he would stop is SD2/SS2's long spell stuck behind
  yellow; he would say "the yellow car keeps blocking me", but it now visibly is his car hitting it.
- **Sam on Easy.** Winnable, very: 1st by 1:30.7, alone in front from the first lap with no bumps. For a 6-year-old
  who wants to win that is the point of Easy; it is a lonely race, which he may find less exciting after a couple.
- **Zoe, 10, Auto.** Industrial 3rd, 3rd; Night 3rd, 3rd. Not a walk in the park: she has to pass three cars and
  dodge oil, puddles and kerbs. The helper is worse than a good 10-year-old (it eats every oil patch and scrapes kerbs
  on long bends), so a real Zoe would place higher, which matches the bot sweep's ~60% Auto wins. On Hard (Desert, 3rd
  1:33.3) she has a real race.
- **Top levels (supplementary).** Desert 12 and Night 13: 4th both, with a fair hazard each (visible, hits only on
  overlap). A loss drops Desert 12 to 9.
- **Goal, actions, feedback.** Unchanged screens: pick a track picture, How to race (pictures plus four short
  lines), the wheel test, race, results with place and "Your best yet!". A 6-year-old can act from the pictures; the
  Rivals chip is the one control that needs reading ("Rivals Auto"), and its ignored first tap (finding 1) makes it
  look dead. Every speed loss inspected has a visible cause at the moment it fires (a car touched, mud, ice, oil,
  puddle, kerb, a Pro rock or barrier), and the HUD speed turns red, so both kids can tell what happened.
- **Where they would stop.** Sam: during SD2/SS2's spell behind yellow, or after SS2's 3rd following SS1's win
  ("I won and now I'm 3rd"); the results page with Replay/Tracks/Home is a natural stopping point. Zoe: four 3rds in a
  row at Industrial and Night (within 1.6 s of each other) would make her switch track or try Hard; she would not quit
  for unfairness, since every slowdown has a cause.
- **Recovery.** A bump or a hazard costs about 30 mph and two seconds; nothing is lost; a missed menu tap needs a
  second tap (finding 1); Back exists on every pre-race page.

### Gesture and interruption matrix (pass 3)

| Case | This pass |
| --- | --- |
| Picker, launcher arrow, Rally tile, cards, Next/Go, Replay, Tracks, Back from a card | Acted; first tap on the track page often ignored (finding 1) |
| Rivals chip | Cycles on each tap after the first; the choice holds into the race |
| Pause hold, Home mid-race, rest screen, reboot mid-race, drag off a button | **Gap** (not in this brief; passes 1-4 of `../adaptive-playtest/` cover them on older builds) |

### Automated (pass 3)

`make -C apps/porthole playtest` at `94a58e5`: 84 of 84 scenarios pass, 0 UI FAIL, 0 UI WARN (monkeys under the
sanitizers included). No scenario added: finding 1 does not reproduce in the raw script language the scenarios use.

### Limitations (pass 3)

- The helper is not a child: it parks on a rival's corner, eats oil and scrapes kerbs. Places are direction only.
- Post-race Auto levels were not read (only starting levels and the `dbg` ones), so whether Auto rose after SS1's win
  is not shown.
- TN (Night 13, 4th) and ZN2 (Night Auto, 3rd) both finished in 1:51.1 (+1.6) with different rivals and hit counts.
  Both results pages were read from separate runs; the coincidence was not chased.
- Only one race each on Easy and Hard; no board; every top-level race is `dbg level` (supplementary).

### Verdict (pass 3)

**Ready.** Pass-2's Major (rival bumps with road between the cars) is fixed: every bump seen is a touch, the grid
side-bump is gone, and Pro hazards still hit only where drawn. The retune reads fair: Sam is near the front on Desert
and Snow, Easy is a sure win, Zoe has to work for every place on Auto and Hard. Finding 1 (an ignored first tap on the
track page) is Minor, already on `origin/main` before this branch, and needs its own fix; it does not block this
change.

## Resolutions after pass 3 (game-engineer)

| Finding / review item | Resolution |
| --- | --- |
| P3-1 Minor: first tap on the track page ignored | **Not reproduced** on this tree (`fa6e50a`, the same code as `94a58e5` plus docs). The stated repro (fixture, Sam, arrow, Rally, `tick 1500`, a 66 ms touch) run in `--serve` mode, by hand and through `drv2.py` itself (t=2898 at the tap, as in the report), takes Night, Industrial, Snow, Desert, Home and the chip on the first tap, across 0-1500 ms waits and 0-99 ms touches. `samD.json` was under `build/` and lost to a `make clean` before it was replayed. `firstTapAfterEntry` (`host/test_racing.cpp`) now holds every target to acting on the first 66 ms touch at 33 ms frames. Owner: a fresh repro with its full session trace, kept outside `build/`. |
| P3-2 Nit: rear-corner bumps every 4-5 s | No change (a real touch now). |
| Review: Industrial's Pro oil made the competent driver better | Dropped; `proNoEasier` checks each driver on its own again (within 2 places of 30 races). With rivals keeping clear of a whole patch (below) Desert's third rock made the kid driver's races easier (93 places against 99 below) and was dropped too. Industrial Ruins and Snow Track have no Pro hazard. |
| Review: the shift fade skipped rungs near 0 | One rung per level all the way down (`rungOf`). `neighbourLevels` checks levels -4..4 on every track on their own seeds: most steps are under 25 points; two known cliffs remain for the designer (-1 to 0, where the slow-down starts, and Industrial's level 1, the pace-setter's first charge), 42-45 points, held so they grow no worse. |
| Review: rivals stopped avoiding a patch at its middle | A rival now avoids a patch until past its end (an obstacle until level with it). Rivals got much better on the oil and puddle tracks, so the base strengths moved: `LEVEL_SHIFT` 7, 6, 2, 5 (was 9, 12, 2, 8). |
| Review: Hard vs Normal for both drivers; Night Hard | `modeBands` checks Hard below Normal for both drivers. `HARD_LEVEL` 2, 1, 2, 4. |
| Review: a released v2 save above today's top | Clamped on load (`loadKids`), so `debug` and the save read today's top; tested with a level-18 Desert save. |

Final balance (competent / kid driver, wins out of 40, the modeBands seeds; staircase: last 40 of 80):

| Track | Shift | Top | Pro levels (hazards) | Staircase | Normal | Hard (level) |
| --- | --- | --- | --- | --- | --- | --- |
| Desert Canyon | 7 | 14 | 12-14 (2 rocks) | 23 (57%) | 21 / 16 | 17 / 13 (2) |
| Industrial Ruins | 6 | 15 | 13-15 (none) | 24 (60%) | 30 / 30 | 14 / 9 (1) |
| Snow Track | 2 | 19 | 17-19 (none) | 27 (68%) | 20 / 20 | 12 / 14 (2) |
| Night Run | 5 | 16 | 14-16 (barrier) | 24 (60%) | 27 / 20 | 14 / 13 (4) |

Neighbour levels -4..4 (competent wins out of 40, their own seeds): Desert 40 40 39 36 18 17 15 15 13; Industrial 40 40
35 29 26 9 9 9 10; Snow 40 40 40 38 20 20 15 15 9; Night 40 40 40 33 26 24 18 17 15.

## Pass 4 (final gate: rivals clear whole patches, per-track base strength, Pro trimmed)

Same playtester agent as passes 1-3, so again not a fresh first-time-comprehension check. Emulator roleplay only; no
real child and no board. `games/racing/` source was not read. The app brief (`apps/porthole/AGENTS.md`) was read and
says `FreshGate` makes every screen change ignore touches for 450 ms.

- Build: `claude/rally-pro-obstacles` at `392d4f0`, `build/host/snap` sha256
  `7fe6d9a934923ff9e9dd4186194bed50e2030e1bf04f1164a2b1ccde63f95442`, frozen as a scratch copy for every run and deleted after.
- Brief (as received): Sam 6 on Auto, Desert and Industrial from a fresh profile (2 each): fair, fun, not crushing?
  Zoe 10 on Auto, Snow and Night (1-2 each): not a walk in the park? Zoe on Hard on Industrial once. Top via fixture:
  Desert 14, Night 16 once each. Try hard to reproduce pass-3 finding 1 (first tap on the track page ignored after
  opening Rally from the launcher).
- Driver: `drv2.py` unchanged, settings as pass 3 (Sam `SEEN=2 SEEY=290 SEEW=60`, gain 0.8, 6 degrees of noise;
  Zoe `SEEY=260`, gain 1, no noise; both tap Boost/Rocket like a kid). One driver process per track. The fixture's
  Zoe is 9, as in every pass. Session traces (JSON) are in the session scratch `pro4/` (uncommitted); frames deleted.
- Unsuccessful runs, kept for the record: Zoe's first `dbg level 14` attempt never raced: the shell's rest screen
  ("Zoe is resting. Back in 10 min") cut her 9th race of the session mid-lap 2 (shell-owned, known since
  `../adaptive-playtest/` pass 2). The top runs were redone in a fresh session.

| # | Who | Track | Rivals | Place | Time | Speed-loss events (HUD red) |
| --- | --- | --- | --- | --- | --- | --- |
| SD1 | Sam | Desert | Auto, level 0 | 3rd | 1:34.4 | 11: rivals, mud, kerb; stuck behind yellow in laps 2-3 |
| SD2 | Sam | Desert | Auto | **1st** | 1:30.8 | 0; alone in front from about 8 s |
| SI1 | Sam | Industrial | Auto | **1st** | 1:47.7 | 13; 4th to 1st, the lead taken in the final lap |
| SI2 | Sam | Industrial | Auto | **1st** | 1:48.6 | 14; 2nd most of the race, the lead taken at the end |
| ZS1 | Zoe | Snow | Auto, level 0 | 3rd | 2:00.6 | 31: about 20 rear bumps on blue (the helper parks behind it), ice |
| ZS2 | Zoe | Snow | Auto | **1st** | 1:56.5 | 8; alone in front from lap 1 |
| ZN1 | Zoe | Night | Auto, level 0 | 3rd | 1:51.0 | 17: puddles, kerbs, rivals |
| ZN2 | Zoe | Night | Auto | **1st** | 1:47.3 | 3; alone from about 12 s |
| ZN3 | Zoe | Night | Auto | 3rd | 1:50.3 | 12 |
| ZH | Zoe | Industrial | **Hard** | 2nd | 1:48.3 | 11; in a group all race |
| TD | Zoe | Desert | `dbg level 14` | 4th | 1:34.9 | 8: one Pro rock (at contact, `12` top row), mud, kerb, cones, a rival |
| TN | Zoe | Night | `dbg level 16` | 4th | 1:51.3 | 15: Pro barriers (at contact, `12` bottom row), puddles, rival bumps |

(Each top run followed one Auto race on the same track in that session to reach a results page: Desert 4th,
Night 4th. Not counted above.) The levels were not read back in `--serve` (`debug` answers nothing there); the
suite's `a10_racing_top_losses.txt` and `a11_racing_short_top_losses.txt`, passing on this build, take the same path
(results page, `dbg level 14` / `16`, Replay) and read `race_level=14` / `16` back. Night's barriers are drawn only at
Pro levels, so TN shows one; Desert also has an ordinary rock, so the rock in `12` alone does not prove 14.

Method notes: a second track in a session skips How to race, so on Zoe's later tracks the "Next" tap landed on Go
about 0.9 s before the driver's `tilt 0 1000 0` (harmless: the start lights were still on). ZH's Hard was read on
the track-page chip only (`Rivals Hard`); nothing later shows the mode.

### Pass-3 finding 1: not reproduced

0 of 31 attempts reproduced it: Sam (16) and Zoe (15) from the picker, launcher arrow, Rally tile, then a 66 ms touch on
Desert, Industrial, Snow, Night, Home or the Rivals chip 900, 1000, 1500, 2500 or 3000 ms after the tile; every first tap
acted. What does happen: a tap 0-300 ms after the tile's release changes nothing (frame identical, `11`), from 400 ms
on it acts. The launcher arrow behaves the same after the picker (ignored at 100 and 300 ms, acts at 400 ms). This is
the shell's documented 450 ms `FreshGate`, not a Rally defect. Trace: `pro4/early.json` (fixture, then
`down 80 81, tick 66, up, tick 600, down 96 135, tick 66, up, tick 600, down 80 104, tick 66, up, tick 300,
down 106 104, tick 66, up, tick 700` = still the track page; the same touch again opens How to race).

### Findings (pass 4)

No Blocker or Major.

- **Minor (observation, pacing). After a loss, Auto's next race can be a lonely win.** SD2, ZS2 and ZN2 each followed a
  3rd with a race led alone from the first 10-15 s; ZN3 went back to 3rd. Each kid alternates a hard race and an easy
  one. For Sam this is welcome. For Zoe two of the five Auto races were a walk; the other three (and Hard) were not.
  The helper is worse than a good 10-year-old in the losses (it parks on a rival's bumper), so a real Zoe would see
  fewer losses and fewer drops. No change asked; the designer may want to know.
- **Minor (known, shell-owned). The rest screen ends a race mid-lap** (see above). Unchanged since its deferral.
- Nit, carried: a kid who drives on a rival's rear corner is bumped every few seconds (ZS1). Each is a visible touch.

### Qualitative (pass 4)

- **Sam, 6.** Desert 3rd then 1st; Industrial 1st, 1st, both decided in the last lap. Fair, fun, not crushing: the
  Industrial wins were close races, which is the best outcome for him. The point he would stop: the Desert loss spent
  behind yellow, or happily after two wins.
- **Zoe, 10.** Snow 3rd, 1st; Night 3rd, 1st, 3rd; Hard Industrial 2nd in a pack. Not a walk in the park overall; the
  wins after a loss are (Minor above). She would try Hard after an easy win; Hard gave her a real race.
- **Top levels (supplementary).** Desert 14 and Night 16: 4th both, close to 3rd. Rocks and barriers are visible about
  a second ahead and hit only on overlap. Industrial and Snow have no Pro hazard now.
- **Goal, actions, feedback, recovery.** Unchanged screens; every speed loss inspected has a visible cause.

### Gesture and interruption matrix (pass 4)

| Case | This pass |
| --- | --- |
| Picker, launcher, Rally tile, cards, Go, Replay, Tracks, chip to Hard | Acted on the first tap after the 450 ms gate |
| Rest screen mid-race | Hit once (ZD): race ends, rest page |
| Pause hold, Home mid-race, reboot mid-race, drag off a button | **Gap** (not in this brief) |

### Automated (pass 4)

`make -C apps/porthole playtest` at `392d4f0`: 84 of 84 scenarios pass, 0 UI FAIL, 0 UI WARN.

### Verdict (pass 4)

**Ready.** No Blocker or Major. Sam is near or at the front on Auto and enjoys close wins; Zoe has real races on Auto
and Hard, with an easy race after each loss; Pro hazards at the top are visible and hit only where drawn. Pass-3
finding 1 does not reproduce on this build; only the shell's intended 450 ms gate is seen.

## Pass 5 (after merging main, b3d33d5)

A new playtester agent, but not a fresh first-time-comprehension check: it read the earlier passes in this file and
`drv2.py` before playing (the brief asked it to follow their method) and knew the change from the brief. It did not
read `games/racing/` source or tests. Emulator roleplay only; no real child and no board.

- Build: `claude/rally-pro-obstacles` at `b3d33d5` (merge of main, incl. #128 rivals vary every race and the Times
  page). `build/host/snap` sha256 `ced5e81abf550fc6c731697f16b83c76e17a24528caf017eec3e67d86266f046`, frozen as a
  scratch copy for every run and deleted afterwards (disk is tight; traces and frames deleted too).
- Persona (as received): an 8-year-old who has raced Rally a lot and is good at it, reads short words, holds the puck
  like a wheel, sometimes over-steers and taps a little off-target. Played on the fixture's Zoe profile (age 9 in
  `../adaptive-playtest/fixture.txt`; nothing in Rally depends on the age).
- Brief (as received, summarised): Desert Canyon Pro 12-14 now shows two rocks from level 12 (mid-road before the
  ramp, left shoulder) and rivals vary every race: are both rocks visible early enough, do they hit only where
  drawn, is the race fair and close? Desert on Hard now races at level 3 (was 2): harder than Normal but winnable,
  is Normal vs Hard noticeable? Night Run Pro (14+) barrier as a control. At least 3 Desert Pro, 2 Desert Hard,
  1 Desert Normal, 1 Night Pro, a pause/resume, a finish and an exit to the cards. `dbg level N` only to reach Pro.
- Input: everything through taps and tilt in `snap --serve` via `drv2.py` (unchanged), from the picker: Zoe, the
  launcher arrow, the Rally tile, the Rivals chip, the track cards, Next/Go, Replay/Tracks. Taps are in the 160x160
  logical space (the first two taps at native 480 px coordinates did nothing; that was my mistake, not the game).
  Driver `SEEN=2 SEEY=260 SEEW=60`, gain 1, 2 degrees of steering noise (the over-steer), kid taps Boost/Rocket.
  N1 used gain 1.2 and 4 degrees of noise (a sloppier kid) and was not used for comparisons.
- Fixture (supplementary, hidden state): `dbg level 12`, `13`, `14` on the Desert cards/results page before the
  race, and `dbg level 14` on the cards before Night. Night's barriers appear only at Pro, and they were on the road,
  which confirms the level took. The Desert levels were not read back (`debug` answers nothing in `--serve`).

| # | Track | Rivals / level | Place | Time | Speed-loss events (HUD red) | Notes |
| --- | --- | --- | --- | --- | --- | --- |
| N1 | Desert | Normal | 3rd | 1:37.5 | 34 | First race, How to race read; sloppier driver |
| N2 | Desert | Normal | 3rd | 1:35.7 | 21 | Replay |
| N3 | Desert | Normal | 3rd | 1:34.9 | 17 | Second session; yellow 2nd a few car lengths ahead all last lap |
| H1 | Desert | **Hard** | 3rd | 1:33.8 | 6 | Pause at 40 s (lap 2, in 2nd), Resume, "Ready" restart; then Results, Tracks |
| H2 | Desert | Hard | 3rd | 1:35.8 | 17 | Replay |
| H3 | Desert | Hard | 3rd | 1:33.3 | 8 | Second session; 2nd and 3rd swapped in the last lap |
| P1 | Desert | `dbg level 12` | 4th | 1:33.8 | 8 | Both rocks seen together in lap 2 (`13` top row) |
| P2 | Desert | `dbg level 13` | 4th | 1:36.3 | 27 | Helper parked behind yellow most of the race; 3rd until the last 10 s |
| P3 | Desert | `dbg level 14` | 4th | 1:34.1 | 9 | One mid-road rock clip, rest rivals/kerb/mud |
| NP | Night | `dbg level 14` | — | — | 15 | **Cut mid-lap 2 in 3rd** by the shell rest page ("Zoe is resting", `15`) about 14.7 min into the session |
| NQ | Night | `dbg level 14` | 4th | 1:52.2 | 27 | Fresh session; barriers seen, none hit; losses were kerbs, puddles, rivals |
| N4 | Desert | Normal | 3rd | 1:36.8 | 27 | Probe: same driver with **no** steering noise; most losses are bumps into the pack |
| H4 | Desert | Hard | 3rd | 1:33.2 | 6 | Probe: no steering noise; clear road behind the leaders |

Exit to the cards: Tracks from Results (four times) and Back on How to race. Every race ended on Results.

### What I checked about the rocks (Desert Pro)

- **Both rocks visible together at 12.** In P1 lap 2, after the mud on the left, the left-shoulder rock and the
  mid-road rock come into view about a second apart (`13` top row). The mid-road rock is visible as a distinct orange
  rock about 1.3 s before it reaches the car at 90 mph (P3, 33 ms frames), longer at the left one.
- **Hit only where drawn.** Every rock hit inspected frame by frame (P1 x2, P3 x1) was a corner clip with 10-15 px
  of sprite overlap, the puff between the rock and the car (`13` bottom row). No rock hit with a clear gap and no hit
  on an empty road. Three P1/P3 hits that first looked unexplained were, at 33 ms steps, rear-corner touches on the
  rival just ahead (yellow or blue), already a carried Nit. One P1 puff on blue was blue in the mud.
- **Dodging works.** The driver (reacts about 0.5 s ahead) took 1 rock hit in P3 and 2 in P1, out of 6 rock passes
  each (3 laps x 2 rocks, inferred from the layout, not counted pass by pass). A kid
  who looks ahead can steer round both; the gap between them is wide enough to pass.
- **Rivals vary.** Grid colour order differed race to race (e.g. blue/yellow/green, then green/blue/yellow), and
  who finished 2nd changed.

### Findings (pass 5)

No Blocker. One Major (finding 1).

1. **Major (challenge, meaningful choice): Hard on Desert did not feel harder than Normal.** Screen: track
   cards / Results. The brief's goal for this change ("harder than Normal but winnable, Normal vs Hard noticeable")
   was not shown. Four Normal and four Hard races, all 3rd, no win in either mode. Hard was if anything easier:
   fewer slowdowns (6, 17, 8, 6 against 21, 17, 27; N1 excluded) and faster times (1:33.2-1:35.8 against
   1:34.9-1:36.8). In the frames, on Normal the kid catches the pack and loses speed bumping into it; on Hard the
   leaders pull away and she drives clean road to a lonely 3rd. To an 8-year-old who flips the chip to Hard, the
   result does not change, and on Desert she never wins either mode. The noise-free probe (N4, H4) gave the same
   picture, so the 2-degree over-steer is not the cause. Caveats: the helper is weaker than this persona in traffic
   (it parks on a rival's bumper), so the modes may separate at the front of the field, and I cannot tell whether
   #128's rival variety made the field stronger. The pass-3 neighbour row above (Desert levels 0, 2, 3: 18, 15, 15
   competent wins of 40, before pass 4's retune) suggests Hard 2 to 3 is a small step. Repro: Rally cards, chip to
   Normal, Desert, Go, race; Tracks, chip to Hard, Desert, race (driver `smooth 1 0 kidsee`). Screenshot:
   `14-pass5-desert-normal-vs-hard-both-3rd.png`. Owner: game-designer, to check Hard vs Normal on Desert by place
   and gap, not only win counts, and with a driver that races in traffic.
2. **Minor (known, shell-owned). The rest page ends a Pro race mid-lap.** NP was in 3rd in lap 2 of Night Pro when
   "Zoe is resting. Back in 10 min" replaced it; the race is lost. Same as pass 4; unchanged. Repro: one session,
   eight races back to back (about 14.7 min of play). Screenshot: `15-pass5-night-pro-race-cut-by-rest-page.png`.
3. **Observation (pacing at the top).** 4th in all four finished Pro races (three Desert, one Night), close to 3rd
   (P2 was 3rd until the last 10 s). For a kid who climbed to Pro by winning, four 4ths in a row would be where she
   stops ("too hard now"); but the ladder drops a level after a loss, so a real kid would not stay at 14. Helper
   caveat as in 1. No change asked.
4. Nit, carried: rear-corner touches on the rival just ahead slow the car with a small puff (P1, P3); each is a
   visible touch at 33 ms steps.

### Qualitative (pass 5)

- **Goal, actions, feedback.** Unchanged screens; the chip reads Easy/Normal/Hard/Auto in yellow and the kid can tell
  which is on. Every speed loss inspected has a visible cause (rock, mud, kerb, puddle, rival). The rocks look like
  the desert's own rocks and sit where a kid would expect them.
- **Fair, close, not cheap.** Rocks are seen in time and only hit on overlap, so a kid blames her own steering, not
  the game. Two rocks a lap at Pro make the race busier, not unfair. Close: rivals in sight in the last lap in most
  races. Not cheap.
- **Where she would stop.** After a few 3rds on Normal and Hard that feel the same (finding 1), or after the
  rest page cut a Pro race she was doing well in (finding 2). Satisfying stop: the Results page with "Your best yet!".
- **Recovery.** Pause by holding the lap text, Resume gives a "Ready" count before racing; Tracks and Back return to
  the cards. A hold just above the lap text (on the speed arc, logical 22,20) did nothing; the hold region is
  26-58 x 28-52 logical, covering the "1/3 II Hold" text, so this was a genuine miss, not a defect.

### Gesture and interruption matrix (pass 5)

| Case | This pass |
| --- | --- |
| Picker, launcher arrow, Rally tile, cards, chip to Normal/Hard/Auto, Next, Go, Replay, Tracks, Back | Acted |
| Pause hold on the lap text, Resume | Acted (H1); a hold above the text did nothing |
| Rest page mid-race | Hit once (NP): race ends |
| Home mid-race, Restart, New grip, reboot mid-race, drag off a button | **Gap** (not in this brief) |

### Automated (pass 5)

`make -C apps/porthole playtest` at `b3d33d5`: 87 of 87 scenarios pass, 0 UI FAIL, 0 UI WARN (report read in full).

### Verdict (pass 5)

**Needs another pass for change 2 (Desert Hard); ready for changes 1 and 3.** Desert Pro's two rocks are visible
together from level 12 about a second ahead and hit only on overlap, and the races stay close (4th, near 3rd); Night
Pro's barrier is unchanged. Finding 1 (Major): Hard at level 3 was not shown to be harder than Normal or winnable for
this driver; the designer should check it, then replay Normal vs Hard on Desert. Finding 2 is the known shell rest
page.

### Pass 5 resolutions (game designer, 2026-11-01)

- Major 1 (Desert Hard not harder than Normal): not changed. Same place (3rd) in both modes, but the driver's own
  times were faster on Hard (1:33.2-1:35.8 against 1:34.9-1:36.8 on Normal) because it drove clear road while the
  leaders pulled away: the rivals were faster, so Hard was harder, only lonelier. On Normal the car caught the pack and
  lost speed in it. Over 40 races (`modeBands`), Desert Hard (level 3) is won less often than Normal by both drivers:
  competent 14 against 18, kid 8 against 11. Eight races cannot overturn that. Winnable on Hard rests on those sweeps;
  this pass got no win.
- Minor 2 (rest page cuts a race): the shell's, known; no change here.
- Rocks (change 1) and the Night Run barrier: accepted as played.
