# Rally adaptive rivals: black-box playthrough (emulator roleplay)

This is emulator roleplay by an independent playtester agent. No real child played and no physical board was used.
The playtester did not read `games/racing/` or the racing host tests. It read the shared sim harness header
(`host/sim.cpp`: the `--serve` protocol and where saves go) and the existing racing playtest scenarios' `dbg`
names (only after the natural races, for the supplementary pass and the new regression scenario).

## Build and setup

- Branch `claude/rally-adaptive`, commit `53e2f41` (feat(porthole): Rally rivals use boosts when chasing).
  `build/host/snap` sha256 `4036f82ee341c1e757c90424c998bf23b3c82b58779278b8ece43c4612efd1b0`, copied and frozen
  for the whole session.
- Fixture (`fixture.txt`, `--fresh --script`): `reset`, `tilt 0 1000 0`, `profile Zoe 9`, `profile Sam 6`. After that,
  `snap --serve` (no `--fresh`) in its own working directory, driven only through `down/move/up/tick/tilt/frame`.
- Inputs: every menu tap was chosen from a rendered 480x480 frame. Steering used `drv.py`, a pixel-only helper. It
  reads the road's extent (colour learned from the first race frame), the red car's x and, from the second pass on,
  rival-coloured pixels just ahead of the car. It turns that into a wheel angle `tilt 1000·sinθ 1000·cosθ 0`, one
  step per 99 ms. "Kid" mode taps Boost (3 o'clock) about every 8 s and Rocket (9 o'clock) when a rival is close
  ahead. Sam's runs add Gaussian wheel noise (14-40°) and a low gain, so he understeers and wobbles. Frames were
  saved every 1-2 s of play and inspected as contact sheets, with native frames at key moments.
- Input traces: `zoe-session.json.gz` (Zoe's whole session: four Desert Canyon races) and `sam-session.json.gz`
  (Sam's two Snow Track races). Each is the full command list after the fixture. In a second process, one replayed
  frame (level 12, lap 3, the same timestamp) matched byte for byte. Other runs were not re-verified. **The session
  traces are not committed** (about 700 KB together), so the `Repro:` lines below that start from them cannot be
  replayed from the repository; a rerun starts a fresh session with the committed driver.
- To rerun: `make -C apps/porthole snap`, then `python3 apps/porthole/docs/racing/adaptive-playtest/drv.py S.json
  ACTION...` (a missing `S.json` starts fresh; commands already in it are replayed first). The driver writes its sim
  directory and frames under `apps/porthole/build/adaptive-playtest/` (or `$RALLY_DRV_DIR`) and runs a frozen
  `snapf` there when present, else `build/host/snap`. Since pass 2 it has been tidied (lint, no writes next to
  itself); its steering is unchanged.

## Persona and dispatch brief (as received, unchanged)

> Independent child-perspective black-box playtest of Rally's new adaptive rivals. What changed: rivals now adapt
> per kid per track. Win a race and the next one on that track is a bit harder; lose and it gets easier (twice as
> fast). At higher levels rivals carry Boost charges and fire them (with visible flames) when they are chasing you.
> Goal from the owner: once a kid has mastered the controls, races should still feel challenging (rivals visible
> ahead, overtakes, lead changes), never a walk in the park, and never unfair or punishing.
> Personas: Zoe, 9, confident reader, plays games a lot, will try to win every race; Sam, 6, early reader,
> imprecise tilts and taps. Natural races first on a fresh profile (at least 3 on one track as Zoe, 2 as Sam).
> Then, as a supplementary fixture only, `dbg level 6`, `dbg level 12` (and `dbg level -10` for Sam). Use `debug`
> output (`level=`, place) to confirm.

## Natural play (first-play evidence, no `dbg`)

| # | Kid | Track | Level going in* | Place | Time | What the race looked like |
| --- | --- | --- | --- | --- | --- | --- |
| 1 | Zoe | Desert Canyon | 0 | 1st | 1:30.3 | Boost at the start; 1st by about 19 s of race; no rival seen after that. |
| 2 | Zoe | Desert (Replay) | 1 | 1st | 1:31.2 | Green rival led, visible close ahead for all of lap 1 and half of lap 2 (`01`); Zoe passed it mid lap 2; alone in lap 3. |
| 3 | Zoe | Desert (Replay) | 2 | 2nd | 1:32.5 | Yellow led from lap 1. Zoe bumped rivals and mud in lap 1. She took 2nd with a boost in lap 2, but never saw the leader again. |
| 4 | Zoe | Desert (Replay) | 0 | 2nd | 1:32.4 | Hits in lap 1 traffic. Green led all race; it was **visible a few car lengths ahead for all of lap 3** (`02`), and Zoe never closed the gap. |
| 5 | Sam | Snow Track | 0 | 1st | 1:55.4 | 1st about 6 s in; alone for nearly two minutes. |
| 6 | Sam | Snow (Replay) | 1 | 1st | 1:55.5 | Sloppier steering (gain 0.5, 25° noise). 1st about 7 s in; alone for the rest of the race (`03`). |

*Levels were read with `debug` only after the session (Zoe's saved Desert level `-2` = 0 +1 +1 −2 −2; Sam's Snow
level `2`).

**Did difficulty visibly change?** On Desert, yes. Race 1 was a solo drive; race 2 had a real duel for about 60 s,
with the pass that wins it; races 3-4 had a leader Zoe could see but not catch. On Snow, no. Sam was alone from the
first 10 s at levels 0 and 1, the same as in the earlier leaderboard playtest.

## Supplementary fixture pass (hidden state, not natural play)

`dbg level N` was set on the results page before Replay (Zoe: Desert; Sam: Snow), and `debug` was read in a
separate `--script` copy of the save. The fixture clamps to −16..15.

| Driver | Level | Place / time | Observation |
| --- | --- | --- | --- |
| Zoe, no dodging | 6 | 3rd, 1:34.3 | Pack of three visible all race. A rival's boost flames show as two orange streaks under the car (`05`). Many rear-end bumps (helper limit). |
| Zoe, no dodging | 12 | 3rd, 1:33.1 | Lap 3: Zoe boosts to 1st, then two rivals boost past within about 2 s (yellow with flames) (`06`). Her rocket hits the yellow car, and she rear-ends it at once: 90 → 60 mph (`07`). |
| Zoe, dodging | 12 | 2nd, 1:32.3 | Lead changes in lap 2 and lap 3 (briefly 1st twice); the winner was visible ahead to the line. |
| Zoe, dodging | −6, −2, 0 | 1st, 1st, 1st | 1:33.1, 1:31.5, 1:31.4 |
| Zoe, dodging | 1 | 2nd, 1:33.6 | |
| Zoe, dodging | **2, 3, 4, 5, 6, 9** | **3rd, 1:34.5 every time** | **Pixel-identical races** (all 50 frames of 3 vs 6 compared, and the result screens of 2-9 hash-identical). The yellow leader stays visible just ahead all race. |
| Zoe, dodging | 10, 11, 12, 15 | 2nd-3rd, 1:32.2-1:32.3 | 10 and 11 differ from each other and from 9. |
| Zoe-like but sloppy | 3 vs 7, 10, 11 | all differ | So the identical band depends on how the kid drives. |
| Sam, very sloppy, no boosts | −10 | 3rd, 2:05.4 | Leaders were visible in lap 1 and gone by lap 2. |
| same, same inputs | 0 | 3rd, 2:04.8 | The same place as at −10. |
| Sam, very sloppy, taps boosts | −12 | 2nd, 2:02.1 | Led from about 6 s to lap 3. Green caught up and passed in the final lap (`08`). Close and exciting, but a loss. |
| Sam, very sloppy, no boosts | −16 (floor) | 2nd, 2:04.1 | |
| Sam, very sloppy, taps boosts | −16 (floor) | **1st**, 2:01.3 | The first win for this driver. |
| — | quit mid race (pause → Home → Yes) | — | Sam's Snow level stayed 2. Quitting is neither a win nor a loss. |

## Qualitative assessment

**Can the persona recognise the goal and what changed?** Zoe sees place (`4th` → `1st`), lap and rivals clearly, and
the results page says `1st/2nd` plus her time. She cannot see that the rivals adapt, and she doesn't need to. What
she can see is that "the green one is faster now". On the results page, `1:32.5 (+2.2)` sits right under `2nd`.
Zoe would read `+2.2` as how far behind the winner she was; it is actually the gap to her own best (`04`). Nothing
says how close the winner was. Sam reads `1st` and the big colours; his runs never showed him a change.

**Decisions, skill, fairness.** From level 1 up on Desert, Boost is a real decision: race 2 was won by a boost
pass. Rival boosts read as "they used a boost too": the flames are the same idiom as the player's, and the rival
surges past at the same time. Nothing looked like a rival pulling away at the kid's top speed without flames. At
level 12 the late-lap pattern is harsh: Zoe's boost puts her 1st, and two rivals boost past within about 2 s. That
is exciting once and could feel rigged on repeat. Bumping into a car ahead costs the player a lot of speed (90 →
57-61 mph), and dense packs at high levels make that frequent. My helper does not dodge as well as a 9-year-old,
so the losses at 6/12 overstate the difficulty. With dodging, level 12 was a close 2nd with lead changes, which is
exactly the owner's target.

**Does a rival ever feel like cheating?** There was no flameless rival pulling away at the kid's top speed. The
closest case is natural race 4 at level 0. After losing time in lap 1 traffic, Zoe sat at 90 mph for two laps and
boosted to 113 mph in lap 3. The green leader held a steady gap of a few car lengths and she never closed it
(`02`). That is "I'm at full speed and can't catch them". It is fair (the gap came from her own bumps), but the
owner should confirm that at level 0 a rival ahead is meant to hold the player's top speed.

**Does the low level let a struggling kid pass and win?** Only at the very bottom, and only if he uses Boost. The
very sloppy Sam (about 8-10 s slower than his clean pace) finished 3rd at −10 and at 0 (the same place) and 2nd at
−12 (passed on the last lap). At the −16 floor he won only when tapping Boost (2:01.3); without boosts he was 2nd.
Starting from 0 at −2 per loss, the floor is 8 losses away. A real 6-year-old who steers this badly would lose six
or more races in a row before the first win. He would mostly place 2nd-3rd, never last. The helper's sloppiness
is not a model of a real child, so treat this as a direction, not a count (finding 3).

**Repetition, boredom, stopping points.** Zoe: after race 2 (a duel she won) she is hooked. After two losses
(races 3-4) a 9-year-old would likely try once more, then switch track. This is her first voluntary stop, at
about 7 minutes of play. Sam: his first voluntary stop is the results page of race 2. Both races were 1:55 of
driving alone after the first 10 s, and the adaptive change was invisible to him on Snow.

**Recovery and imprecise input.** Offset taps on the picker, launcher arrow, Rally tile, track card and Next all
landed. A double tap on Go started one race. Lying flat paused; Home asks `Go home? This race ends.`, and Yes
quits without changing the level. A loss lowers the next race on that track, so nothing is lost by losing.

## Findings

### Major (owner to confirm: found with the hidden-state fixture, not natural play)

1. **Levels 2-9 played an identical race for one driving style, so "lose → easier" and "win → harder" did not
   show up across 8 steps.**
   - Screen: race (Desert Canyon).
   - Concern: challenge / fairness / pacing of adaptation.
   - Repro: restore `zoe-session.json.gz` (results after race 4) and run
     `drv.py S "dbg level L" "tap 52 111" "tick 3000" "drive 100 X 0 smooth 1 0 kiddodge" "tick 15000" "shot R"`
     for L = 2..9. Every run gives 3rd, 1:34.5 and identical frames. L = 1 and L = 10 differ.
   - Impact if real: a kid who is behind the pack and losing at level 9 drops 9 → 7 → 5 → 3 → 1. That is four
     identical losses before the race gets easier, which contradicts "lose gets easier, twice as fast" and risks
     the "punishing" outcome the owner wants to avoid. A sloppier driver did see differences between 3 and 7, so
     the dead band may depend on the kid's position in the pack (rival level acting only when rivals are
     behind?).
   - Screenshot: `09-level-sweep-results.png` (top: L0 1st, L15 3rd; below: L−6, −2, 1, 2, 4, 9, 12, 15).

### Minor

2. **Snow Track at levels 0-1 is a solo drive for a 6-year-old who steers only roughly.**
   - Screen: race (Snow Track).
   - Concern: boredom / challenge.
   - Repro: `sam-session.json.gz`. Sam was 1st by 6-7 s and alone for about 1:50, in both races.
   - Detail: at +1 per win it takes many wins before Snow is contested, while a very sloppy Sam at −12 still got
     a close race. The step per win (or the starting level per track) looks too small for Snow.
   - Screenshot: `03-sam-snow-race2-alone.png`.
3. **A struggling kid needs many losses before the race gets easy enough to win.**
   - Screen: race (Snow Track).
   - Concern: challenge / recovery.
   - Repro: `sam-session.json.gz`, then `dbg level L` and Replay with `drive 150 X 3000 jerky 0.35 40 kid` for
     L = −10, −12, −16. Results: 3rd, 2nd (passed on the last lap), 1st.
   - Detail: at −2 per loss from 0, the win comes about eight losses in. Consider a bigger first drop or a lower
     starting level after consecutive losses.
   - Screenshot: `08-sam-level-minus12-late-pass.png`.

4. **The `(+N)` beside the place reads as the gap to the winner, but it is the gap to the kid's own best.**
   - Screen: results.
   - Concern: comprehension / feedback.
   - Repro: Zoe race 3. Results show `2nd`, then `1:32.5 (+2.2)`, and the board lists only Zoe's 1:30.3.
   - Suggestion: show the winner's gap ("Yellow won by 1.4 s") or label it ("2.2 slower than your best").
   - Screenshot: `04-results-2nd-plus-vs-own-best.png`.
5. **A successful rocket hit on the car just ahead makes the shooter rear-end it.**
   - Screen: race.
   - Concern: feedback / fairness.
   - Repro: `zoe-session.json.gz`, then `dbg level 12`, Replay and `drive 100 X 1000 smooth 1 0 kid` (no dodging).
     In lap 3: `Hit!`, then debris on Zoe's car and 90 → 60 mph within 0.3 s.
   - Detail: this pre-dates the change, but adaptive packs make it common. A kid sees "I hit them" and still gets
     slowed down.
   - Screenshot: `07-rocket-hit-then-rear-end.png` (frames 3-6).
6. **Rival boost flames are small and identical in colour to the player's.**
   - Screen: race.
   - Concern: feedback.
   - Detail: two orange streaks about 6x20 px under the car. They read up close (`05`) but are invisible on a
     distant rival, and there is no cue when a rival boosts from behind until it is alongside. This is acceptable
     for Zoe; Sam would not notice.
   - Screenshot: `05-level6-rival-boost-flames.png` (right: 3x crop).

### Nit

7. **At high levels, rivals answer the kid's boost-to-1st with two immediate boost passes.**
   - Screen: race. Concern: fairness / pacing.
   - Repro: the level-12 run from finding 5.
   - Detail: seen in both level-12 runs. It is fine as drama, but if it happens every race it will read as
     rubber-banding. Worth a cooldown or a random chance.
   - Screenshot: `06-level12-lap3-lead-changes.png` (frames 2-5).

## Gesture / interruption matrix

| Case | Result |
| --- | --- |
| Offset tap on picker row, launcher arrow, Rally tile, track card, Next | Each acted once |
| Double tap on Go | One race |
| Boost/Rocket mashing with 0 charges | Nothing happens; no error |
| Lie flat mid-race → Paused; Home → `This race ends` → Yes | Launcher; level unchanged |
| Replay ×5 | Starts the countdown each time (skips the grip page) |
| Process restart mid-race | **gap**: not tested |
| Restart from pause, Resume after a flat lay | **gap** this pass (covered by the earlier leaderboard playtest) |
| Long-press / drag off a button | **gap** |
| Night Run, Industrial Ruins with adaptive rivals | **gap** |

## Automated checks

- `make -C apps/porthole playtest`: 81 of 81 scenarios pass, 0 UI FAIL / 0 WARN. Racing: `a0_racing_flow`,
  `a1_racing_monkey`, `a4_racing_board`, and the new `a5_racing_level`. (At the time: 19 checks of +1 per win and
  −2 per loss. It now checks the current rules: +2 per runaway win with `dbg lastlap`, a loss from 1..9 to −2 with
  `dbg lastplace`, persistence across a reboot, per kid and per track, a never-raced track starting inside 0..3, and
  the clamps at 18 and −16.)

## Owner's goal and verdict

- **Partly met.** On Desert, from level 1 up, races have rivals visible ahead, overtakes and lead changes; level 12
  with dodging was a close 2nd with the lead changing twice. Rival boosts read as fair: flames, and a surge at the
  same time. That is the intended experience.
- **Not met yet:**
  - Snow stays a solo drive at levels 0-1 (finding 2).
  - A struggling kid needs about eight losses to reach a winnable race (finding 3).
  - For one driving style, levels 2-9 played an identical race (finding 1), so for that kid the adaptation is
    invisible across eight steps.
- **Verdict: needs another pass.** The owner should confirm or fix finding 1 and decide on findings 2-3. After any
  change, a fresh replay should cover Desert (natural, three or more races) and Snow.

## Iteration history

- Pass 1 (this report), build `53e2f41` / `4036f82e…`. No game fixes have been requested or made yet. The brief was
  used unchanged.
- Suggested brief changes for the next pass:
  - Require a steering helper that dodges from the start.
  - Ask for at least one natural streak of losses (four or more) on one track, so "lose → easier" is judged in
    play rather than only through the fixture.
  - Include one race each on Industrial Ruins and Night Run.

## Limitations

- Pixel steering stands in for hands. It neither dodges cars (first pass) nor dodges them as well as a kid does
  (second pass), and its "sloppy" noise is not a model of a real 6-year-old's tilting.
- Natural play covered two tracks and six races. Level ±10 behaviour beyond what is listed is untested.
- One fixture oddity: after `dbg level -30` (clamped to −16) and a lost race, Sam's saved Snow level read `2`, his
  level before the fixture. The fixture value did not persist, probably because a loss at the floor changes
  nothing and so writes nothing. This is fixture-only: `a5_racing_level` shows natural losses saving and clamping
  correctly.
- This is emulator roleplay, not a real child and not the physical board.

## Resolutions after pass 1 (superseded: see "Resolutions after pass 2" at the end)

Fixed on `claude/rally-adaptive` after this report and the code review. The coordinator also asked for
`dbg level N` to save, which explains the "Limitations" fixture oddity. Numbers come from the sweep in
`host/test_racing_rules.cpp` (`competent`, `staircase`). A fresh black-box pass on the new build has not been run yet.

| # | Finding | Resolution |
| --- | --- | --- |
| 1 | Levels 2-9 played identical races for a driver who stays behind | Fixed on the easing side. Rival boosts only fire when the kid is ahead of that rival, so for a kid who stays behind those levels are the same race by design. A loss above 0 now halves the level instead of taking two (`nextLevel`): 9 → 4 → 2 → 0, so a losing streak from the top reaches 0 in about three races. Below 0 a loss still takes two. |
| 2 | Snow Track at levels 0-1 is a solo drive | Fixed. A win with no rival within 100 m at the line now adds two levels (`CLEAR_LEAD`), so a runaway kid reaches a contested level about twice as fast. A never-raced track starts one below the kid's best level elsewhere, capped at 3 (review). |
| 3 | A struggling kid needs about eight losses to reach a winnable race | Improved. The slow-down now grows deeper down (`slowdown`: 0.35% a level plus a quadratic term, about 0.87 at -16, was 0.94). A steady driver who never dodges or presses anything now first wins in race 5-6 from level 0, at -8 to -10; before, that took -12. A driver who never steers still finishes 4th at -16 (a test). |
| 4 | `(+2.2)` reads as the gap to the winner, but it is the gap to the kid's own best | Deferred. It predates this change and is results-page wording; it needs its own design pass (winner's gap vs own best). |
| 5 | A successful rocket hit makes the shooter rear-end the slowed car | Deferred. It predates adaptive rivals; changing contact or rocket rules is a separate change. |
| 6 | Rival boost flames are small and the same colour as the player's | Fixed. Rival flames are now half as big again and drawn in the boost's cyan and cream; the kid's stay orange and gold. |
| 7 | Two rivals boost past within about 2 s of the kid's boost to 1st | Fixed. No rival fires within 2.5 s of another rival's boost (`RIVAL_BOOST_GAP_MS`). |
| — | Limitation: the `dbg level -30` value did not persist | Fixed. `dbg level N` now marks the save dirty. |
| — | Snow and Industrial Ruins tops (review) | Industrial Ruins still won about 52% at the old top (15). Widening the chase window did nothing: rivals spend their charges long before the kid is 40 m clear. Levels 16-18 now let a rival take its three charges back at each lap line, as the kid does from pickups; Industrial's top falls to about 40%. |

## Pass 2 (retest of the resolutions, same playtester, emulator roleplay)

The same independent playtester ran this pass. It did not read `games/racing/` source. It knew the engineer's resolutions
table above (from the brief), so it is not a fresh first-time-comprehension check. No real child played and no
board was used.

### Build, setup, brief

- Commit `0128565` on `claude/rally-adaptive`. `build/host/snap` sha256 `f82737a675bf33e024d4a5f1731f4d7c0770540be11bc4d4e33ad06dc8ce8280`,
  frozen as `snapf` for the session.
- Same fixture and `drv.py`. The only change to `drv.py` is a `skip` passthrough, so the session can wait out the
  rest screen. Touch coordinates are logical (160x160). Every menu tap was chosen from a rendered frame.
  - Zoe: `smooth 1 0 kiddodge` (dodges, taps Boost about every 8 s, Rocket when a rival is close ahead).
  - Zoe "bad day": `jerky 0.6 25 kid`.
  - Sam: `jerky 0.5 25 kid`.
  - Struggling Sam: `jerky 0.5 32 kid`.
- Levels were read afterwards from copies of the save (`debug` after tapping each track card in a `--script` run).
  They were not consulted during play.
- Brief (as received): Zoe plays at least 4 Desert races trying to win, then at least 4 losses in a row on one track
  if she can produce them, then one race each on Industrial Ruins and Night Run. Sam plays at least 3 races on Snow.
  Then supplementary fixtures only: `dbg level 6/12/18` (are rival flames readable? any gang-up or rubber-banding?
  any flameless pull-away?) and `dbg level -10` for Sam (can a struggling kid pass and win?).

### Natural play (no `dbg`)

| # | Kid | Track | Level in | Place | Time | What happened |
| --- | --- | --- | --- | --- | --- | --- |
| 1 | Zoe | Desert | 0 | 2nd | 1:32.5 | Green led. It was visible as a dot ahead all of lap 3, and Zoe never closed at 90 mph. |
| 2 | Zoe | Desert | −2 | **1st** | 1:32.5 | A duel with green. Zoe boosted past it in the final lap (`10`). |
| 3 | Zoe | Desert | −1 | 2nd | 1:32.7 | Green far ahead in lap 3. |
| 4 | Zoe | Desert | −3 | 1st | 1:30.9 | 1st from about 12 s, then nobody on screen. **Counted +1, not the runaway +2.** |
| 5 | Zoe | Desert | −2 | 1st | 1:31.6 | Passed green mid lap 2, then alone. +1. |
| 6 | Zoe | Desert | −1 | 2nd | 1:32.9 | Green far ahead in lap 3. |
| 7 | Zoe (bad day) | Desert | −3 | 2nd | 1:34.2 | Wobbly. Swapped places with yellow and green. |
| 8 | Zoe (bad day) | Desert | −5 | — | — | Led from 10 s. **The 15-minute session rest cut the race in lap 3 while she was 1st** (`11`). No result; level unchanged. |
| 9 | Zoe | Industrial (new) | −6 | 3rd | 1:57.7 | Blue visible just ahead nearly all race; passed at the end. The helper hit walls often here. |
| 10 | Zoe | Night Run (new) | −6 | 1st | 1:46.1 | 1st by 12 s and alone. This time the runaway +2 fired (−6 → −4). |
| 11 | Sam | Snow | 0 | 2nd | 1:58.4 | Yellow close for two laps; Sam passed it in lap 3. Contested. |
| 12 | Sam | Snow | −2 | 1st | 1:55.7 | 1st from about 12 s; **alone for about 1:45**. +1. |
| 13 | Sam | Snow | −1 | 1st | 1:55.8 | Same: alone. +1. |
| 14 | Sam | Snow | 0 | 1st | 1:55.3 | Alone. +1. |
| 15 | Sam | Snow | 1 | 1st | 1:55.8 | 1st by 6 s, alone (`12`). +1 (→ 2). |

New-track starts behaved as described ("one below the best elsewhere"): Industrial and Night started at −6 after
Zoe's bad-day dip on Desert (−5).

A natural streak of four losses was not produced. Zoe never got above 0 on Desert, and below 0 a single loss (−2)
already made the next race easy. The halving was exercised only from the fixture (below).

### Supplementary fixtures (hidden state)

| Driver | Level | Place / time | Observation |
| --- | --- | --- | --- |
| Zoe, dodging | 6 | 3rd, 1:34.2 | Yellow just ahead all race; Zoe 2nd twice briefly. No rival flames in the 1 s samples: rivals boost only to pass the kid, and she was behind. |
| Zoe, dodging | 12 | **1st**, 1:31.3 | Green boosts past from behind with **cyan** flames, clearly unlike Zoe's orange (`13`). Zoe boosts back to 1st in lap 3. Lead changes, no gang-up. |
| Zoe, dodging | 18 | 3rd, 1:32.6 | Lap 1: green, yellow and blue each boost past, 5.2 s and 4.5 s apart (`14`). The 2.5 s gap holds, but Zoe drops 1st → 4th in about 10 s. A pack all race. |
| same, natural losses after 18 | 9, 4, 2, 0 | 3rd 1:34.2, 3rd 1:34.5, 2nd 1:33.5, **1st 1:31.3** | Halving confirmed: 18 → 9 → 4 → 2 → 0, then the win at 0 (→ 1). Four losses, then a win. The day's 30-minute cap fired at the GO of one race (no result, level unchanged). |
| Steady driver, no presses (Snow) | 0 / −10 | 3rd 1:59.4 / 1st 1:56.3 | Consistent with the engineer's claim. At −10 the win is a solo drive from 6 s. |
| Struggling Sam (Snow) | −10, −12, −14, −16 | 3rd 2:11.4, 3rd 2:14.3, 2nd 2:11.0, 2nd 2:16.1 | 15-20 s off his clean pace. He passes a car in every race and is close behind the leader at −14 (`15`), but **never wins, even at the floor**. |
| Very sloppy (`jerky 0.35 40`) | −10 | invalid | The helper drove on the snow at 50 mph from the start, so this run is not evidence. In pass 1 the same parameters gave 2:02-2:05, so struggling-Sam numbers are not comparable across passes. |

Rival flames were cyan and readable at 12 and 18, including on a rival several car lengths ahead. No rival pulled
away at Zoe's top speed without flames. That is a qualitative read of the contact sheets, not a frame-by-frame gap
measurement. At levels 0-2 a leader ahead held a steady gap at Zoe's 90 mph, as in pass 1.

### Pass-1 findings: status

| # | Status |
| --- | --- |
| 1 Levels 2-9 identical | **Mitigated.** A trailing driver still gets the same race at 6 and 9 (3rd, 1:34.2) and at 4 (1:34.5). The halving gets out of that band in two races, and 2 and 0 differ (2nd, then 1st). |
| 2 Snow solo at 0-1 | **Still present.** See Major 1. |
| 3 Many losses before a win | **Improved for a steady driver** (wins by −10). A driver 15-20 s off pace places 2nd-3rd and never wins, even at −16. See Minor 2. |
| 4 `(+N)` reads as the gap to the winner | Deferred; still present (`+3.3` under `3rd`). |
| 5 Rocket hit then rear-end | Deferred; not re-examined. |
| 6 Rival flames small, same colour | **Fixed.** Cyan and cream, clearly the rival's (`13`, `14`). |
| 7 Two boosts right after the kid's | **Fixed as specified** (no two rival boosts within 2.5 s). At 18 the serial version remains. See Nit. |

### Findings (pass 2)

**Major**

1. **The runaway "+2" rarely fires for wins that look solo to the kid, so Snow stays a walk in the park for Sam.**
   - Screen: race (Snow Track; also Desert race 4).
   - Concern: boredom / challenge.
   - Evidence: five natural wins with no rival on screen after the first 6-15 s each added +1 (Zoe race 4;
     Sam races 12-15). Only Night Run at −6 got +2.
   - Likely cause: rivals behind the kid are never drawn, so a rival 2 s back (under 100 m at 90 mph) is invisible.
     The kid drives alone for 1:45, but the rule sees a close finish.
   - Result: Sam needs about five more solo races before Snow is contested. A 6-year-old stops after two.
   - Suggestion: base the bonus on what the kid sees (time led with no rival visible ahead, or the lead at the line
     in seconds), or show rivals behind (a mirror or a "2nd 1.8 s" tag).
   - Repro: fresh Sam, Snow, `drv.py S "drive 140 X 3000 jerky 0.5 25 kid"` × 5 with Replay (`tap 50 113`). Levels
     read: −2, −1, 0, 1, 2.
   - Screenshot: `12-sam-snow-level1-alone.png`.

**Minor**

1. **The play-time rest cuts a race mid-lap, even in the final lap while leading, with no warning.**
   - Screen: shell rest page during a race.
   - Concern: recovery / feedback (owned by the shell, not Rally).
   - Repro: Zoe's natural session, race 8: the 15-minute session ends in lap 3 while she is 1st (`11`). Nothing
     appeared in the 75 s of 0.5 s frames before it. The daily 30-minute cap also fired at a race's GO.
   - Impact: the level is unchanged, so the cut is not punishing in the rules. From the kid's seat, though, it is
     "I was winning and it vanished". This is by design (`AGENTS.md`: running games enforce the schedule).
   - Suggestion for the owner: a grace period to the race's end, or a "last race" notice at the start.
2. **A kid 15-20 s off his clean pace never wins on Snow, even at the floor (−16).** He places 2nd-3rd and passes
   cars, so it is not punishing, but there is no win.
   - Concern: challenge.
   - Repro: the fixture table above.
   - Screenshot: `15-sam-minus14-boost-pass-green-ahead.png`.
3. **A bad-day dip carries into new tracks.** Zoe's one sloppy loss put Desert at −5, so Night Run started at −6 and
   was a solo win on her first visit.
   - Concern: challenge.
   - Repro: natural races 7, 9 and 10.

**Nit**

1. **At level 18 three rivals boost past one after another in lap 1** (1st → 4th in about 10 s, `14`). The gap rule
   holds and 18 is earned, so this is drama rather than unfairness. Watch it if it repeats every lap.

### Gesture / interruption matrix (pass 2)

| Case | Result |
| --- | --- |
| Offset taps on picker, launcher arrow, Rally tile, track cards, Next, Replay, Tracks | Each acted once |
| Re-entry after Home or the rest page | The How to race page shows again first (expected; caught a scripted tap meant for Go) |
| Session rest mid-race; daily cap at GO | Race ends with no result; level unchanged (Minor 1) |
| Process restart mid-race (Sam at −16, Zoe at 0) | Boots to the picker; level unchanged |
| Long-press, drag off a button, Resume after pause | **gap** this pass |

### Automated checks

`make -C apps/porthole playtest` at `0128565`: 81 of 81 scenarios pass, 0 UI FAIL / 0 WARN (racing: `a0_racing_flow`
41 checks, `a1_racing_monkey`, `a4_racing_board`, `a5_racing_level` 19 checks).

### Verdict

**Needs another pass.** Verified: the halving (18 → 0 in four losses, then a win), cyan rival flames, the 2.5 s boost
gap, the new-track start, and the steady-driver easing. Desert with Zoe alternates wins and losses with rivals
visible ahead and overtakes, which is the owner's target. Not met: Snow is still a solo drive for Sam at −2..1,
because the runaway rule's trigger does not match what the kid sees (Major 1).

### Suggested brief changes

- Plan around the 15-minute session cap: `skip 600` between race blocks, or a grown-up Allow 30 min.
- Ask the engineer to expose (or the brief to allow reading) the finish gap after a race, so "solo" can be checked
  against the rule.
- Pin the struggling-Sam helper parameters and validate them before use (one invalid run this pass).

## Resolutions after pass 2 (game-engineer, third pass)

Current state of every finding from both passes, after the review of `f8fc072` and pass 2. Numbers come from
`host/test_racing_rules.cpp`: the `competent` driver and `staircase` (80 races per track from level 0, asserted on the
last 40). A third black-box pass on this build has not been run.

The level rule now, in full: a win adds one, two after a runaway (the kid led for 60% of the race). A loss at 0 or
below takes two; from 1..9 it drops straight to -2; above 9 it takes three. So from the top (18) a losing streak is
below 0 after four losses (15, 12, 9, -2). Industrial Ruins and Night Run top out at 6 and 10 (their levels sit 12
and 8 rungs up the ladder), so on Industrial Ruins any loss above 0 lands at -2, and on Night Run two losses do. A never-raced track starts one below
the kid's best level elsewhere, inside 0..3.

| Pass / # | Finding | Status |
| --- | --- | --- |
| 1 / 1 | Levels 2-9 played identical races for a driver who stays behind | **Fixed.** A loss from 1..9 now drops straight to -2. Below 0 the rivals are slower for everyone, so the next race differs even for a kid stuck behind. |
| 1 / 2, 2 / M1 | Snow Track is a solo drive; the runaway +2 rarely fired | **Fixed.** The +2 now follows what the kid sees: leading for 60% of the race, not a 100 m gap at the line (a rival 2 s back was invisible but inside 100 m). Snow check: a competent driver from a fresh 0, six series of eight races: never two runaway (+2) wins in a row; each was followed by a race with a rival close enough to lose to or to win by less. |
| 1 / 3, 2 / m2 | A struggling kid needs many losses; one 15-20 s off pace never won on Snow | **Fixed except Desert Canyon.** The floor is now set per track: rivals at 0.80 of their pace at -16 (0.87 on Desert, whose straights let a kid who never steers finish within 18%). A simulated wobbly driver about 16-20 s off a clean drive wins 17-19 of 20 at -16 on Industrial Ruins, Snow Track and Night Run. On Desert it does not: that driver is barely faster than one who never steers, and the idle driver must still finish 4th (a test). |
| 2 / m3 | A bad-day dip carries into new tracks | **Fixed.** A never-raced track starts inside 0..3 (one below the best elsewhere, never below 0). |
| — (review) | Industrial Ruins settled near 85% wins; the staircase band had been loosened | **Fixed.** Industrial Ruins' levels sit 12 rungs up and Night Run's 8 (`LEVEL_SHIFT`, fading out by -8 so a struggling kid needs the same number of losses on every track). Last 40 of 80 races, seed series of the test: Desert 26, Industrial 30, Snow 28, Night 26 (65-75%); five other seed series 25-31. The band is back to 45-80%. |
| 1 / 6 | Rival flames small, same colour as the kid's | **Fixed** (pass 2 confirmed: cyan, half as big again). |
| 1 / 6 (part) | No cue when a rival boosts from behind until it is alongside | **Partly.** Rivals behind the kid are not drawn, so the flames show only once the rival is level or ahead. A mirror or "behind" cue is a separate design. |
| 1 / 7, 2 / Nit | Rivals boosting one after another | **Fixed as specified** (2.5 s apart). At 18 three passes in a lap remain, as intended drama. |
| 2 / m1 | The play-time rest cuts a race mid-lap | **Deferred: shell-owned.** Running games enforce the schedule (`apps/porthole/AGENTS.md`). The level is unchanged, so it costs nothing in the rules. A grace period to the line is a shell decision. |
| 1 / 4 | `(+N)` reads as the gap to the winner | **Deferred.** It predates this change; results wording needs its own design pass. |
| 1 / 5 | A rocket hit then rear-ending the slowed car | **Deferred.** It predates adaptive rivals. |
| 1 / Lim. | The `dbg level` value did not persist | **Fixed.** It saves now. On a track the kid has never finished, the next entry or finish derives it again. |
