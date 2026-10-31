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
  (Sam's two Snow Track races). They are not committed (about 700 KB together); the commands below need them. Each is the full command list after the fixture. In a second process, one replayed
  frame (level 12, lap 3, the same timestamp) matched byte for byte. Other runs were not re-verified.
- To rerun: put `drv.py`, `fixture.txt` and a copy of `build/host/snap` named `snapf` in one directory, gunzip a
  session to `S.json`, then `python3 drv.py S.json ACTION...`. Commands already in `S.json` are replayed first.

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
  `a1_racing_monkey`, `a4_racing_board`, and the new `a5_racing_level` (19 checks: +1 per win and −2 per loss with
  `dbg lastlap/lastplace`, persistence across a reboot, per kid and per track, clamped at 15 and −16).

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

## Resolutions (second pass, game-engineer)

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
