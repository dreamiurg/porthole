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

> Superseded: the level rules and Normal/Hard levels here were changed later (rung-keyed losses, a
> `LEVEL_SHIFT` on every track, cars touching only where drawn). See `games/racing/AGENTS.md` and `docs/racing/pro-playtest/`.

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

## Pass 3 (final retest, same playtester, emulator roleplay)

The same playtester ran this pass. It did not read `games/racing/` source, but it knew the rule changes from the brief
and both earlier passes, so this is not a fresh first-time-comprehension check. No real child played and no board was
used.

### Build, setup, brief

- Commit `88fcff3` on `claude/rally-adaptive`. `build/host/snap` sha256 `967874117742fec22c2a6d87f28d798f6042aad831b5eacc2a107fa42f8f1122`,
  frozen as `snapf` for the session.
- Same fixture and `drv.py` (unchanged). Every menu tap was chosen from a rendered frame. Zoe: `smooth 1 0 kiddodge`.
  Sam: `jerky 0.5 25 kid`. Struggling Sam: `jerky 0.35 45 kid`, calibrated this pass (Desert 1:36-1:38, about 5-6 s
  off Zoe's 1:31.6; Snow 2:03-2:07, about 8-11 s off Sam's 1:55.8). It is not comparable with the pass 1-2 struggling
  numbers.
- Levels were read after each race from copies of the save (`debug` after tapping each track card in a `--script`
  run). They were never fed back into the driving.
- Screen time: one grown-up Reset usage (Admin, Screen time) between Zoe's Industrial and Night blocks. Struggling Sam's
  Desert and Snow blocks ran in separate fresh sessions.
- Brief (as received): Sam at least 4 races on Snow (contested within 2-3 races?). Zoe at least 4 on Desert, then 2
  each on Industrial Ruins and Night Run (first visits: fair or crushing?). A struggling Sam on Desert and Snow until
  he wins or about 5 races. `dbg level -16` on Desert only if needed. Check each open finding from pass 2.

### Natural play (no `dbg`)

Every level change below matches the stated rules (+1 per win, +2 when the kid led most of the race; a loss at 0 or
below takes two; a loss from 1..9 drops to -2).

| # | Kid | Track | Level in | Place | Time | What happened |
| --- | --- | --- | --- | --- | --- | --- |
| 1 | Zoe | Desert | 0 | 2nd | 1:32.5 | Green led and was visible ahead all of lap 3. Zoe stayed at 90 mph and never closed. |
| 2 | Zoe | Desert | -2 | 2nd | 1:32.9 | Same shape; green just ahead in lap 3. |
| 3 | Zoe | Desert | -4 | 1st | 1:31.6 | Passed green in lap 1, then alone. Runaway +2. |
| 4 | Zoe | Desert | -2 | 2nd | 1:33.0 | Rocket `Hit!` in lap 1; green ahead in lap 3. |
| 5 | Zoe | Desert | -4 | 1st | 1:33.1 | Passed green in lap 3 (+1). |
| 6 | Zoe | Desert | -3 | 2nd | 1:33.2 | Green ahead in lap 3. Ended at -5. |
| 7 | Zoe | Industrial (new) | 0 | 1st | 1:46.8 | Duel; led from mid lap 2. The helper hit a wall (59 mph). |
| 8 | Zoe | Industrial | 1 | 2nd | 1:47.5 | Lead changed four times with yellow; yellow won, visible ahead (`17`). |
| 9 | Zoe | Night Run (new) | 0 | 1st | 1:48.3 | Yellow ahead for two laps; Zoe boosted past in the final lap (`17`). |
| 10 | Zoe | Night Run | 1 | 3rd | 1:49.6 | Pack all race; yellow just ahead at the line. |
| 11 | Sam | Snow | 0 | 1st | 1:55.8 | Alone from about 5 s. Runaway +2. |
| 12 | Sam | Snow | 2 | 1st | 1:56.1 | Contested. Sam fell to 4th, then caught green and boosted past it at 113 mph in lap 3 (`16`). |
| 13 | Sam | Snow | 3 | 2nd | 1:57.9 | Rivals ahead all race. → -2. |
| 14 | Sam | Snow | -2 | 1st | 1:55.8 | Alone from about 10 s. Runaway +2. |
| 15 | Sam | Snow | 0 | 1st | 1:55.8 | Alone; the same race as #11. → 2. |
| 16-21 | Struggling Sam | Desert | 0, -2, -4, -6, -8, -10 | 3rd ×4, then 2nd ×2 | 1:36.4-1:38.6 | No win. At -10 he led lap 1, then green and yellow passed; green visible ahead to the line (`18`). Ended at -12. |
| 22-27 | Struggling Sam | Snow | 0, -2, -4, -6, -8, -10 | 3rd ×4, 2nd, **1st** | 2:03.6-2:07.2 | First win in race 6, at -10: alone from about 10 s. The rest screen replaced its results page within 3 s (`19`). |

Supplementary (hidden state): with `dbg level -16` on Desert after race 21, struggling Sam won 1st (1:36.6), alone from
lap 1. Natural play reaches -16 in two more losses, so his first Desert win would come in race 7-9.

### Qualitative assessment

- **Goal and feedback.** Place, lap and the rivals ahead read clearly on every track. Results keep `(+N)`, the gap to
  the kid's own best, and now show it under a win too: `1st 2:04.0 (+0.4)` was struggling Sam's first win after five
  losses (`19`).
- **Decisions and fairness.** The races were won by a boost pass (#9, #12) or a late pass (#5), and lost with the
  leader in view (#1, #2, #6, #8, #10). No rival pulled away at the kid's top speed without flames. On Desert at -2..-4,
  green still holds a steady gap at Zoe's 90 mph after she loses time in lap 1 traffic (as in passes 1-2). It is fair,
  but this helper wins only 2 of 6 there.
- **First visits.** Fair, not crushing. Both new tracks started at 0 even though Desert sat at -4 (the pass-2 dip no
  longer carries over). Each first visit was a win by a pass, and the second visit was a close loss.
- **Stopping points.** Zoe: after two Desert losses in a row (#1-2), she switches track, and Industrial/Night then
  hold her. Sam: hooked by #12. After the loss in #13, #14-15 are solo again; he likely stops there, bored, at about
  10 minutes. Struggling Sam: a 6-year-old stops around the third straight 3rd place on either track, well before the
  win.
- **Recovery and imprecise input.** Losing never costs anything visible. A long-press on Replay started one race. A
  tap on the `1/3 Hold` sign does nothing; a 1.5 s hold opens Paused. Resume continues. Restart asks `Start over? This
  race ends.`, and Yes keeps the level (-5).

### Findings (pass 3)

No Blocker or Major.

**Minor**

1. **Snow swings from contested back to two solo races after one loss.**
   - Screen: race (Snow Track).
   - Concern: boredom / challenge.
   - Repro: fresh Sam, Snow, `drive 125 X 10000 jerky 0.5 25 kid` × 5 with Replay (`tap 52 111`). Levels 0, 2, 3, -2,
     0; races #11, #14 and #15 were solo.
   - Detail: Snow was contested by race 2 (the brief's question: yes). The loss at 3 drops to -2, though, and two
     runaway wins are needed before the rivals are close again. That makes 3 of 5 races a walk in the park.
     Owner option: drop a loss from 1..3 to 0 instead of -2.
   - Screenshot: `16-sam-snow-race2-boost-pass.png` (the contested race); the solo races look like `12`.
2. **A struggling kid on Desert loses at least six in a row.**
   - Screen: race (Desert Canyon).
   - Concern: challenge / recovery.
   - Repro: fresh Sam, Desert, `jerky 0.35 45 kid` × 6.
   - Detail: this confirms the engineer's "fixed except Desert". He places 2nd-3rd and is never last; at -10 he led
     lap 1. The -16 floor does produce a win, but it is 7-9 losses away. On Snow the same driver won in race 6.
   - Screenshot: `18-sam-desert-struggle-six-losses.png`.
3. **`(+0.4)` sits under struggling Sam's first `1st`.**
   - Screen: results.
   - Concern: comprehension / feedback.
   - Detail: this is pass-1 #4 (owner-deferred), with new evidence. On a first win, a plus number reads like a
     shortfall. Recommend Minor.
   - Screenshot: `19-sam-snow-struggle-first-win-race6.png` (7th tile).
4. **Shell-owned, still present: the rest screen interrupts Rally.** It replaced the results page of struggling Sam's
   first-ever Snow win within 3 s (`19`, last tile). It also cut a supplementary -16 Desert race in lap 2 while he was
   1st. The level is unchanged. This is the pass-2 m1 deferral.

**Nit**

1. Struggling Sam's Snow went from a close 2nd at -8 to a solo win at -10. The bottom step is a cliff, but a first win
   that comes easily is acceptable.

### Earlier findings: status

| Finding | Status |
| --- | --- |
| p2 M1 runaway +2 / Snow solo | **Fixed.** +2 fired on every solo win (#3, #11, #14). Snow was contested at race 2. The swing after a loss remains (Minor 1). |
| p2 m1 rest cuts a race | Still present; shell-owned (Minor 4). |
| p2 m2 struggling kid never wins | **Fixed on Snow** (win in race 6). **Not on Desert** (Minor 2). |
| p2 m3 bad-day dip carries into new tracks | **Fixed.** Both new tracks started at 0 while Desert sat at -4. |
| p2 Nit / p1 #7 boost chains at 18 | Not retested (no high-level fixture this pass). |
| p1 #1 identical races at 2-9 | Moot: no natural level stayed in 1..9 (a loss there drops to -2). |
| p1 #4 `(+N)` | Deferred; new evidence (Minor 3). |
| p1 #5 rocket hit then rear-end | Not re-examined. |
| p1 #6 rival flames | Not retested: Zoe was mostly behind, and rivals boost only when the kid is ahead. |

### Gesture / interruption matrix (pass 3)

| Case | Result |
| --- | --- |
| Long-press on Replay | One race |
| Tap on the `1/3 Hold` sign; hold 1.5 s on it; hold outside its box | Nothing; Paused; nothing |
| Resume; Restart → Yes | Continues; countdown, level unchanged |
| Rest screen at a race finish / mid race | Results replaced; race ends, no result, level unchanged |
| Reset usage (grown-up) between blocks | Levels and bests kept |
| Process restart mid-race, drag off a button, lie flat | **gap** this pass (passes 1-2) |

### Limitations

- In one supplementary run at -16, the car crawled on the shoulder at 50 mph all race. That was a harness artifact:
  the helper's last steering tilt (about 33°) was still held when the grip page set the neutral. The run is not
  evidence; the rerun held the board level.
- A `dbg level -16` set on the grip page of a never-finished track did not take, as the engineer's note says.
- Frames for Sam's Snow races 2-4 were lost on the first run and rebuilt by deterministic replay. A 1 ms `tick` was
  dropped from the struggling-Desert session log after race 1; the replays stayed consistent (six losses).
- This Zoe helper won 2 of 6 on Desert, so this pass cannot confirm "settles near 2/3". That reflects the helper's
  lap-1 bumps, not a measured game rate.
- Emulator roleplay only: no real child, no physical board.

### Automated checks

`make -C apps/porthole playtest` at `88fcff3`: 81 of 81 scenarios pass, 0 UI FAIL / 0 WARN, no sanitizer report (racing:
`a0_racing_flow` 41 checks, `a1_racing_monkey` 4, `a4_racing_board` 9, `a5_racing_level` 19).

### Verdict

**Ready, with Minor deferrals for the owner to accept or fix.** On every track, the competent kid gets races with
rivals visible ahead, passes and lead changes. First visits are fair. Snow is contested by race 2. A struggling kid
wins on Snow by race 6. What remains:
- Snow's two solo races after a loss (Minor 1). This is the one owner call: if "never a walk in the park" is strict,
  it is the next change.
- Desert's long losing run for a struggling kid (Minor 2, known).
- Results wording (Minor 3).
- The shell's rest timing (Minor 4).

## Resolutions after pass 3 (game-engineer, fourth pass)

The final pass after pass 3 and the review of `88fcff3`. It also adds the difficulty setting the owner asked for.
No black-box pass has been run on this build.

The level rule now: a win adds one, two after a runaway (the kid led for 3/5 of the race). A loss at 0 or below
takes two; from 1..3 it drops to 0; from 4..9 straight to -2; above 9 it takes three. From 18 a losing streak is
below 0 after four losses (15, 12, 9, -2). Below 0 the rivals are slower on **every** track. Before this pass, on
Industrial Ruins and Night Run the shift could turn a level such as -2 back into charge rungs only, which a trailing
kid never sees. Now the shift moves only the charge-and-pace ladder, so the slow-down follows the kid's own level.

| Pass / # | Finding | Status |
| --- | --- | --- |
| 3 / m1 | Snow swings from contested back to two solo races after one loss at 3 | **Fixed.** A loss from 1..3 now lands at 0. At 0 a kid stuck behind meets the same race as at 1..3; a further loss goes below 0, where the rivals are slower. |
| 3 / m2 | A struggling kid on Desert loses six or more in a row | **Deferred.** On Desert a driver that wobbly is barely faster than one who never steers, and the never-steering driver must still finish 4th at the floor (a test). |
| 3 / m3 | `(+0.4)` under a first-ever 1st reads as a shortfall | **Fixed.** The gap to the kid's own best is not shown after a win; "Your best yet!" and "Track record!" are unchanged. |
| 3 / m4 | The rest screen interrupts Rally | **Deferred: shell-owned** (running games enforce the schedule). The level is unchanged. |
| 3 / Nit, 2 / Nit | Boost chains at high levels; the bottom step on Snow is a cliff | **Deferred.** The chains are spaced 2.5 s apart and are earned drama at 16-18. The -1 to 0 step (30-36 points on Snow, Industrial and Night in the sweep) is where the pace-setter reaches the kid's own top speed. |
| — (review) | On a shifted track a loss to -2 or -4 could still give a trailing kid the same race | **Fixed** (above). |
| — (owner) | A difficulty setting per kid | **Added.** The chip "Rivals" under the track cards reads Adaptive (default), Easy, Normal or Hard; a tap cycles it and saves it. Easy races at level -8, Normal at 0, Hard at a per-track level a competent driver wins 33-53% of. A fixed setting never moves the adaptive levels. `a5_racing_level` taps the chip and checks `mode=`. |

## Pass 4 (focused retest of the Rivals chip and the new level rules, emulator roleplay)

The same playtester as passes 1-3 ran this pass. It did not read `games/racing/` source. It read `host/sim.cpp` (harness
only, as before) and, after the natural races, the `mode=` lines of `a5_racing_level`. It knew the rule changes from the
brief, so this is not a fresh first-time-comprehension check. No real child played and no board was used.

### Build, setup, brief

- Commit `abfa1e6` on `claude/rally-adaptive`. `build/host/snap` sha256 `e5f231ea5ab34d756f2f22275583dff0b81ab61833c8714afe2ef51fc7b5b44a`,
  frozen as `snapf` for the session. Same fixture and `drv.py` (unchanged). Zoe: `smooth 1 0 kiddodge`. Sam:
  `jerky 0.5 25 kid`. Struggling Sam (supplementary): `jerky 0.35 45 kid`. Levels were read after each race from a copy
  of the save (`debug` on the track card); they never fed back into the driving.
- Brief (as received): (1) Sam (6) first sees the track picker: does he notice and understand the chip, what does he
  think "Adaptive" means, does an accidental tap confuse him, is Home easy to find? (2) Sam sets Easy and races Snow
  twice: fun, winnable, rivals around? (3) Zoe sets Hard and races Desert and Night once each: challenging but fair?
  (4) Zoe back on Adaptive on Snow, 3 races: the swing after a loss (no two solo races in a row). (5) The mode persists
  after Home and back and after a reboot, per kid. (6) No `(+N)` under a 1st. Verdict on the chip word.

### Races

| # | Kid | Track | Mode / level in → out | Place | Time | What happened |
| --- | --- | --- | --- | --- | --- | --- |
| 1 | Sam | Snow | Easy (-8) | 1st | 1:55.4 | 1st about 10 s in, alone to the line. |
| 2 | Sam | Snow | Easy (-8) | 1st | 1:55.9 | The same race: 1st about 10 s in, alone. No `(+0.5)` under it. |
| 3 | Zoe | Desert | Hard (4) | 2nd | 1:33.3 | Pack in lap 1; yellow ahead and in view through lap 2, one rival boosting with flames; leader gone in lap 3. Adaptive Desert level stayed 0. |
| 4 | Zoe | Night | Hard | 1st | 1:47.2 | Duel with green through laps 2-3; Zoe passed it with a boost (122 mph) in the final lap (`21`). |
| 5 | Zoe | Snow | Adaptive 0 → 2 | 1st | 1:54.8 | Alone from about 20 s. |
| 6 | Zoe | Snow | 2 → 3 | 1st | 1:56.1 | Yellow ahead all of lap 1 (Zoe hit ice and mud); passed in lap 2. No `(+1.3)` under the 1st. |
| 7 | Zoe | Snow | 3 → 5 | 1st | 1:54.6 | Alone from about 10 s. |
| 8 | Zoe | Snow | 5 → 7 | 1st | 1:54.4 | Alone from about 10 s. |
| 9 | Zoe | Snow | 7 → 7 | — | — | Contested; Zoe 1st in lap 3. The rest screen ended the race (shell, known); no result, level unchanged. |
| 10 | Zoe | Snow | 7 → 9 | 1st | 1:55.5 | Yellow ahead in laps 1-2 after a rocket `Hit!`; passed. |
| 11 | Zoe | Snow | 9 → 11 | 1st | 1:54.6 | Green ahead in lap 1; passed in lap 2. |
| 12 | Zoe | Snow | 11 → 8 | 3rd | 1:57.4 | Pack all race; yellow just ahead to the line. `(+2.9)`. |
| 13 | Zoe | Snow | 8 → -2 | 2nd | 1:55.6 | Green ahead and in view for laps 2-3. |
| 14 | Zoe | Snow | -2 → -1 | 1st | 1:55.4 | Green ahead all of lap 1; passed in lap 2. |
| 15 | Zoe | Snow | -1 → 1 | 1st | 1:54.8 | Alone from about 10 s. |
| 16 | Zoe (probe: a sloppy "bad day", Sam's driver) | Snow | 1 → **0** | 2nd | 1:56.6 | Next local day. Pack in lap 1; green ahead to the line. |
| 17 | Zoe | Snow | 0 → 2 | 1st | 1:54.3 | **Alone** from about 10 s. |
| 18 | Zoe | Snow | 2 → 4 | 1st | 1:54.7 | **Alone** from about 10 s (`22`: #17 and #18 at the same point). |
| S1 | Struggling Sam (supplementary) | Desert | Easy | 2nd | 1:39.0 | Led lap 1; mud dropped him to 3rd; a pass in lap 3. |
| S2 | Struggling Sam | Snow | Easy | 2nd | 2:05.9 | Three lead changes; green ahead to the line. |
| S3 | Struggling Sam | Snow | Easy | 2nd | 2:05.4 | Green ahead and in view most of the race. |

Excluded as harness artifacts: a first try of #3 at 50 mph all race (the last steering tilt was held when the grip page
set the neutral; truncated and rerun level), and a struggling-Sam Snow race (4th, 2:25.5) run in the same driver process
as S1, where the helper had learned the Desert road colour; S2 reran it in a fresh process.

### Qualitative assessment

- **The chip, through Sam's eyes (6, early reader).** He sees a box with a grey `Rivals` and a yellow `Adaptive` beside
  the orange Home box (`20`). He cannot decode either word; "Adaptive" is three syllables and abstract, and "Rivals" is
  not a 6-year-old's word either. The yellow word and the box shape say "button", so he taps it out of curiosity. One tap
  shows `Easy`, a word he can read, and nothing else changes. An accidental tap is harmless and even helps him. The only
  confusion is that the word changes with no other cue, and three more taps bring back the word he could not read. Home
  stayed easy to find: the same orange box at bottom left, and a tap at x=66 always went Home.
- **Sam on Easy (standard driver): winnable, but no rivals.** Both races were solo after about 10 s, the same as
  Adaptive at 0 in passes 1-3. He would stop after race 2, bored. Easy works for the struggling driver (S1-S3: close 2nds
  with a rival in view), so it is tuned for a struggler, not for the pass-3 Sam.
- **Zoe on Hard: challenging and fair.** One loss with the leader in view, one win by a final-lap boost pass. She would
  keep it on Hard.
- **Zoe back on Adaptive on Snow.** With a clean drive, levels 0 to 5 were solo (#5, #7, #8, #15, #17, #18). Races were
  contested only when she lost time in lap 1 (#6, #14) or at 7 and above (#9-#13). Zoe would stop after #8, the
  third solo win in four races, or switch to Hard. After the loss at 1 (#16), the new rule landed at 0 as stated, but
  the next two races were both solo. After the loss at 8 (#13: to -2) the next race was contested and the one after was
  solo.
- **Feedback.** No `(+N)` under any 1st (#2, #6, #10, #11, #14); it is still there under a 2nd or 3rd (#12, #13, #16).

### Findings (pass 4)

No Blocker or Major.

**Minor**

1. **Snow is still two solo races in a row after a loss, for a clean driver.**
   - Screen: race (Snow Track, Adaptive). Concern: boredom / challenge.
   - Repro: Zoe's session, #16-#18: after a loss at 1 (to 0), `drive 128 X 5000 smooth 1 0 kiddodge` with Replay (`tap 52 111`) twice. Levels 0 → 2 → 4, both 1st and alone from about 10 s.
   - Detail: the "1..3 lands at 0" fix works as a rule, but for this driver 0 to 5 is all solo on Snow, so landing at 0
     instead of -2 changes nothing. Also #7-#8 (3 and 5) were back to back solo. Pass-3 m1 recurs for Zoe; Sam was not
     retested on Adaptive this pass. Owner options: a smaller step when a loss is narrow, or Snow's 0..5 band nearer the
     kid's top speed.
   - Screenshot: `22-p4-zoe-snow-solo-after-loss.png`.
2. **Easy on Snow is solo for the ordinary 6-year-old driver.**
   - Screen: race (Snow Track, Easy). Concern: boredom.
   - Repro: fresh Sam, tap the chip once, Snow, `drive 130 X 10000 jerky 0.5 25 kid` twice with Replay.
   - Detail: both wins were alone after about 10 s. For the struggling driver Easy was close (S1-S3). Easy is a fixed
     -8, so it also never gets easier than that for a kid who keeps losing, where Adaptive goes down to -16. An
     owner call, not a defect; it is consistent with the stated design.
   - Screenshot: the solo races look like `12-sam-snow-level1-alone.png`.
3. **The chip word.** `Adaptive` fails a 6-year-old early reader (concern: comprehension). Use **`Auto`**: one syllable,
   a word kids meet on toys and cars, and it reads as "the game picks". `Easy`, `Normal` and `Hard` are fine. The
   grey `Rivals` heading is also beyond Sam, but he does not need it: the yellow word is what he taps.
   - Screenshot: `20-p4-track-picker-rivals-chip.png`.
4. **The rest screen still ends a race** (shell-owned, the pass-2 m1 deferral): #9 was cut in lap 3 while Zoe led.

**Nit**

1. **A chip change is lost if the power goes within about 5 s.** The shell writes a game's save at most every 5 s, and
   Home flushes it. Repro: `tests/playtests/a6_racing_mode_kids.txt` without its `wait 6000` before `reboot` fails at
   `expect mode=Hard` (got `Adaptive`); Sam's Easy, set before a Home, survives. Rare on a board; the
   `App::saveUrgent()` checkpoint would close it.
2. **A 1.5 s hold on the chip does nothing**, while a long-press on Replay starts a race (pass 3). A kid who holds
   instead of tapping sees no change. Low impact.
3. **Home and the chip are 4 logical px apart** (Home 34..74, chip 78..126, both 24 high at y 118). The boxes do not
   overlap and each meets the 24x22 floor; a finger between them hits nothing.
4. `a5_racing_level.txt`'s header still says "a loss from 1..9 drops it to -2"; the rule is now 1..3 → 0, 4..9 → -2.

### Earlier findings: status

| Finding | Status |
| --- | --- |
| p3 m1 Snow solo swing after a loss | **Recurs for a clean Zoe** (Minor 1); the rule itself works (1 → 0). Not retested for Sam on Adaptive. |
| p3 m3 `(+0.4)` under a first 1st | **Fixed.** No gap under any 1st. |
| p3 m4 rest cuts a race | Still present; shell-owned (Minor 4). |
| p3 m2 struggling kid on Desert | Not retested on Adaptive. On Easy he was a close 2nd (S1). |

### Gesture / interruption matrix (pass 4)

| Case | Result |
| --- | --- |
| Chip: one tap; two quick taps; tap at its left edge (x 82) and right edge (x 124) | Next mode; two modes on; next; next |
| Chip: 1.5 s hold; tap just below it (y 142) | Nothing; nothing |
| Tap at x 66 (Home) next to the chip | Home, mode unchanged |
| Home and back (Sam), Tracks from results (Sam) | Easy kept |
| Other kid (Zoe after Sam set Easy) | Adaptive: per kid |
| Process restart from the saves on disk (`--script`, both kids) | Sam Easy, Zoe Adaptive; `a6` adds Zoe Hard |
| Power cut within 5 s of a chip tap | Lost (Nit 1) |
| Rest screen mid-race | Race ends, level unchanged |
| A fixed mode race (Hard) | Adaptive Desert level unchanged (0) |
| Next local day (`skip 86400`) | Levels and mode kept |

### Automated checks

`make -C apps/porthole playtest` at `abfa1e6` plus the new `a6_racing_mode_kids` (per-kid mode, restart): 82 of 82
scenarios pass, 0 UI FAIL / 0 WARN, no sanitizer report (racing: `a0_racing_flow` 41 checks, `a1_racing_monkey` 4,
`a4_racing_board` 9, `a5_racing_level` 31, `a6_racing_mode_kids` 10). `a6` is the only file this pass added under
`tests/playtests/`.

### Verdict

**Ready for the chip and the Hard setting; Adaptive on Snow needs an owner call.** The chip works, is per kid, survives
Home and a restart, and an accidental tap only makes races easier. Hard is challenging and fair. `(+N)` under a 1st is
gone. Rename `Adaptive` to `Auto`. If "never a walk in the park" is strict, Snow's low band (Minor 1) and Easy for an
ordinary driver (Minor 2) are the next changes.

## Resolutions after pass 4 (game-engineer, fifth pass)

| Pass / # | Finding | Status |
| --- | --- | --- |
| 4 / m1 | Snow can give a very clean driver two solo races after a loss | **Accepted.** After a loss the runaway +2 climbs back fast, so the race is contested again within a race or two. |
| 4 / m2 | Easy on Snow is solo for an ordinary 6-year-old | **Accepted.** Easy is meant to be easy. Auto is the default. |
| 4 / m3 | `Adaptive` fails an early reader | **Fixed.** The chip says **Auto**. The setting's value (0) and its behaviour are unchanged. |
| 4 / m4 | The rest screen still ends a race | **Deferred: shell-owned**, as before. The level is unchanged. |
| 4 / Nit 1 | A chip change is lost if the power goes within about 5 s | **Fixed.** A chip tap asks the shell for an urgent checkpoint (`saveUrgent`), written after a second, not five. |
| 4 / Nit 2 | Holding the chip does nothing | **By design.** It is a tap target. A quick double tap skips a setting, also by design: it cycles. |
| 4 / Nit 3 | Home and the chip are 4 px apart | **Accepted.** That is inside the layout rule (2 px apart, 24x22 each); the UI audit passes. |
| 4 / Nit 4 | The `a5_racing_level` header was stale | **Fixed.** |
| — (review) | A fixed-setting finish on a never-raced track | **Documented.** It makes the track raced, so its seeded level stays and is no longer derived from the other tracks. |
| — (review) | A setting value from a later firmware | **Fixed.** It loads as Auto. A 48-byte blob from the unshipped first v2 is rejected (v2 never shipped). |
| — (review) | Normal and Hard bands | **Tested.** `modeBands` in `host/test_racing_rules.cpp`, 40 races per track: Normal 16-25 wins, Hard 14-22, and Hard never easier than Normal. |
