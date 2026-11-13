# Pumpkin Hide & Seek, hold-to-hide retest 6: child-perspective black-box playtest (Leo, 6)

Emulator roleplay by an independent agent. This is not a real-child study and not a physical-device test.

## Build and setup

- Revision at start: `3a824f8` (clean tree). During the session HEAD moved to `f40be3d`, set by someone else. The
  only difference is 9 added lines in `docs/superpowers/specs/2026-10-17-pumpkin-hide-and-seek-design.md`, found with
  `git diff --stat` only; I did not read the content. No game source changed.
- Executable: `make -C apps/porthole snap` reported "Nothing to be done". `build/host/snap` sha256
  `138970c3b71bc21527a036f0b3c2155c4242df230586fb37b2003cbe9201d680`. The hash was the same before and after
  `make playtest`, and on my private copy.
- The private copy ran in a scratch subfolder (`leo6-10059/run/snap`) as `snap --serve --fresh` with the default
  clock (Tuesday 16:00). The driver sent ordinary serve commands only: `down`/`move`/`up` touches, `tilt X Y Z`
  gravity, `tick MS` and `frame`. No `dbg`, no `skip`, no `app`, no `debug`/`screen`/`ui` queries.
- Profile created through the screens: name "Leo" on the letter keyboard, frog face, age 6, code skipped. Then the
  launcher's pumpkin tile ("Spooky" caption), START, and READY on the calibrate page.
- Leo holds the board in both hands, tipped toward himself: gravity (0, 0.5, -0.87) g. His steering tilts are
  ±0.25-0.4 g, and ±0.6 g when stuck.
- Session 2: after the suite run I started a fresh process without `--fresh`, so Leo's profile was kept and the game
  opened from the picker. One more partial round was played there. The trace marks the restart.

### Disclosure

- Loaded automatically before play: the repo `CLAUDE.md` and `apps/porthole/CLAUDE.md`. The latter describes Pumpkin
  in a few lines: three hiding spots, three candies and a door, sweeps / turn-backs / spot checks, a frosting-to-pie
  retry, "the ghost acts on the lift of any press", "the action sign hides the pumpkin exactly while a finger that
  touched it ... stays down, wherever it drifts", and "both ignore a press that began before the round did". I knew
  these before playing.
- Read for the protocol only: `apps/porthole/tools/webemu.py` (serve commands, frame format), `docs/playtesting.md`,
  and the `playtest` and `tour` skills.
- Listing the shared scratchpad showed file names from other sessions (for example `hold-fix5-report.md`). I read
  none of them. While moving my binary into a private folder I ran `rm -rf scratchpad/run/build`. That folder may
  have existed before me (another session's scratch build output).
- After play: the suite's output contact sheet `build/playtest/79_pumpkin_own_spot-own-spot.png`, then
  `pumpkin-hold-playtest-5.md` for the comparison. No source, test scripts, AGENTS.md or git logs were read.

## Persona and brief (verbatim dispatch)

> You are a fresh, independent child-perspective playtester for Pumpkin Hide & Seek on Porthole. Porthole is a round
> 480x480 touchscreen with no buttons, emulated on the host. Follow apps/porthole/docs/playtesting.md and the
> `playtest` skill.
>
> This is the last check of the final build after a cleanup refactor and a small timing change to the "no" cue. Play
> SOURCE-FREE. Until you finish, don't read any of these:
> - source code (apps/porthole/games/, os/, shell/, host/)
> - tests/playtests scripts
> - docs/superpowers/
> - AGENTS.md files
> - earlier playtest reports
> - git logs
>
> Disclose anything that loaded automatically. Decide everything from the screen. Build once at the start, then run
> the sim from your own copy of the snap binary in a private scratch subfolder.
>
> Persona: Leo, 6. He can't read and tilts with both hands. He presses firmly for 1-3 s, re-presses when worried, and
> sometimes rests or slides a thumb. This is a simulated perspective only.
>
> Setup:
> - Run `make -C apps/porthole snap`. Record `git rev-parse --short HEAD` and the snap's `sha256sum`.
> - Run `snap --serve --fresh` with the default clock.
> - Create the profile (age 6, no code) through the screens, then open "Spooky" from the launcher.
> - Use ordinary touches and tilt inputs only: no dbg, shortcuts or clock skips.
>
> Scope: play at least six rounds. Keep playing until Grandma has named his hiding spot at least twice. Find out:
> - Does hiding by holding the sign work?
> - Does the jump-out with the scared face tell him to move when she names his spot?
> - Do the "<<<" arrows on the sign say "not here, go that way" while he presses away from cover, and do they linger
>   sensibly after he lifts?
> - Does anything feel broken, sticky or confusing?
> Record win/loss.
>
> Output: write apps/porthole/docs/pumpkin-hold-playtest-6.md, with screenshots and the trace in
> apps/porthole/docs/pumpkin-hold-playtest-6/. Don't save .py files; save scripts as .txt. Include:
> - revision and hash
> - the persona and this brief
> - win/loss
> - ranked findings with repro steps
> - your assessment
> - gaps
>
> Afterwards you may compare with apps/porthole/docs/pumpkin-hold-playtest-5.md. Don't fix code or commit. Reply ONLY
> with the verdict (acceptable / needs fixes), win/loss, the top findings and the report path, in under 6 lines.

No brief revisions during this pass.

## Outcomes

12 complete rounds in session 1: **5 wins, 6 losses** and one round left through the exit picture. One more partial
round in session 2 (not counted). Times are game ms in `trace.txt`.

| Round | Result | What happened |
| --- | --- | --- |
| R1 | Win | Tilted right for a candy; Grandma came. Held the sign at the plant: "HIDING", the beam swept, "PHEW!". She came back about 0.5 s after he lifted; hid again. Rolled left for the last candy. The pumpkin **parked at the armchair** with a 0.4 g tilt for about 1.5 s before a harder 0.65 g tilt reached the door. |
| R2 | Win | Hid at the plant twice. As he rolled away, Grandma said "THE PLANT?" (a spot he had left). Escaped. |
| R3 | Win | Hid five times (plant x3, table), every visit a 6-7 s sweep. A long, samey round. |
| R4 | Loss (deliberate probe) | Pressed the sign in the open and kept the board still. ">>>" pointed at the plant, "QUICK! HIDE!", "THERE YOU ARE!", pie. |
| R5 | Win | Pressed in the open while still tilting toward the plant: ">>>", then hid automatically on arrival. "THE TABLE?" came while he sat unhidden at the plant (not his spot), and he walked out. |
| R6 | **Loss** | Rolling left past the table for a candy, Grandma appeared without the "GRANDMA!" warning: "THE TABLE?". He stopped under the table without pressing (a frozen kid): "RUN AWAY!", pumpkin smiling, "← FIND COVER" dim, "FOUND YOU!". |
| R7 | Win | Short worried presses (2.4 s down, 0.4 s up) through a sweep. The pumpkin popped out ("QUICK! HIDE!") between presses and re-hid each time. Never caught. |
| R8 | Loss | Idle at the table without pressing when she came (a distracted kid). |
| R9 | Loss (harness lag) | Grandma returned within about 1 s of "HIDDEN!" while my batched input was still rolling him left. No press in time. A watching Leo would likely have pressed. |
| R10 | **Loss** | All three candies, the door glowing. "THE CHAIR?" (no warning) while he rolled left toward the door. The pumpkin stopped at the armchair with a 0.4 g tilt, beside the open door. He pressed the sign: ">>>" pointing **away from the door**, toward Grandma's side. "GOTCHA DEAR!". |
| R11 | **Loss** | Leaving the table for the last candy, "THE PLANT?" named the spot ahead of him. He pressed within about 0.3 s and kept rolling into the plant for 2.4 s (scripted observation delay): "<<<", "RUN AWAY!". Then he followed the arrow left, but momentum held him at the plant: "PEEKABOO!". |
| R12 | Left | Hid at the table and the chair, all sweeps. Pressed the ghost ("WHAT WAS THAT?" decoy), then held the arrow picture at the top: back to the launcher. |
| S2 | Partial | Re-entered from the picker after a process restart, title and calibrate again. One sweep while hidden. |

**Named-spot count.** Grandma named the spot Leo was at or rolling into 3 times (R6 table, R10 chair, R11 plant). He
was unhidden every time and was caught every time. She never named a spot while he was already hidden in it: 0 of
about 32 visits that began with him hidden were spot checks, all sweeps (`11_hidden_visits_always_sweeps_R6.png`).
The brief's stop condition, his hiding spot named twice, was **not met** in about 9.5 minutes of play. That is a gap.

## Answers to the brief's questions

- **Does hiding by holding the sign work?** Yes, reliably.
  - At cover, a press turns the sign into a yellow "HIDING" and the pumpkin tucks behind the pot, chair or table
    (`02_R1_first_hide_plant_win.png`).
  - Off-centre presses at (70,133), (75,128) and (85,128) worked.
  - With the finger down while rolling, the pumpkin hides the moment it reaches cover (R5 241-245, R11 681).
  - Short lifts mid-sweep showed "QUICK! HIDE!" and a re-press re-hid at once. He was never caught for it
    (`07_R7_short_presses_quick_hide_recovered.png`). Lifting mid-sweep at R2 092 was also forgiven.
- **Does the jump-out with the scared face tell him to move?** Not observed in natural play, so I cannot answer it from
  play.
  - After play, the suite's `79_pumpkin_own_spot` contact sheet shows the startle: a "!" and an O mouth.
  - In my play, every own-spot naming happened while the pumpkin was unhidden. Then there was no startle: the pumpkin
    kept its smile (`06_*`, `08_*`) and only text ("RUN AWAY!") said to move.
  - Pictorial help was Grandma's pointing arm, a "?" bubble over the spot, and (only while pressing) chevrons.
- **Do the chevrons say "not here, go that way"?** Yes.
  - Pressing in the open shows a dark sign with big ">>>" or "<<<" toward the nearest cover that is not named
    (R4 toward the plant, R5 into the plant, R11 toward the table).
  - Rolling the way they point while still pressing re-hides on arrival. That is a good, learnable loop.
  - **Linger:** after lift the chevrons stayed 0.7 s (7 frames at 100 ms), then the sign went back to the dim
    "FIND COVER →" pointing the same way (`10_chevrons_linger_after_lift_100ms.png`). Sensible: long enough to see,
    short enough not to lie.
  - One problem: the chevrons only know cover, not the exit. In R10, with every candy collected and the door glowing
    beside him, ">>>" sent him back toward Grandma.
- **Broken, sticky or confusing?** No crash, freeze or stuck screen. Sticky and confusing parts:
  - The armchair parks the pumpkin short of the door at moderate tilts (R1, R5, R10).
  - Grandma comes back 0.4-1.5 s after "HIDDEN!" (R1 035, R10 587, R11 w11b).
  - Own-spot checks arrive with no "GRANDMA!" warning and name the spot he is rolling into.

## Qualitative assessment

- **Goal and actions from the screen.** Good for a non-reader:
  - The candy shelf fills (three slots), the door lights up with a big "←" when the shelf is full, and Grandma plus a
    sweeping beam is an obvious threat.
  - "HOLD TO HIDE" is unreadable to Leo, but it is the only big button, and pressing it visibly tucks the pumpkin
    away. He learned it on the first visit (R1).
  - The calibrate page shows hands holding the board.
  - The ghost is tempting. Tapping it summons Grandma to a sparkle elsewhere ("WHAT WAS THAT?"). Leo may read that as
    "the ghost calls Grandma" rather than "the ghost distracts her". It was harmless here.
- **Decisions and feedback.**
  - Real choices: which candy next, when to dash, where to hide, when to spend the ghost.
  - Wins come fast (R1 about 25 s, R2 about 25 s) and the win card is cheerful (pumpkin among candies).
  - Losses are clear (orange frosting wipe, pie). The pie lines ("YUM YUM!", "PIE TIME!", "SWEET PUMPKIN!") mean
    Grandma ate him, which may bother a sensitive 6-year-old (n2).
- **Fairness.**
  - Hidden play is completely safe: every hidden visit was a sweep.
  - Every own-spot loss happened while moving into the named spot. Nothing visual on the pumpkin says why, so from
    Leo's view these losses come from nowhere: he did what he always does and lost.
- **Repetition, boredom and stopping.**
  - Sweeps last 6-7 s and come back within a second or two. R3 had five of them, about 35 s of holding still.
  - Leo presses 1-3 s, so he must re-press several times per sweep. That works (R7) but is tiresome.
  - Leo's first voluntary stop: after R3 or at the R4 pie, about 2-3 minutes in. Three easy wins, then "just holding"
    gets dull.
  - The win card's PLAY AGAIN is a natural stopping point. Everything after R4 was coverage-driven, not his
    attention span.
- **Recovery and interruptions.**
  - Wrong presses in the open are recoverable through the chevrons.
  - TRY AGAIN restarts at once.
  - The exit picture (top, "↩") needs about 0.6-0.9 s of press in play. A firm curious press (Leo's style) ends the
    round without asking, but nothing persistent is lost.
  - Leave and re-enter goes back to the title and calibrate. A process restart goes back to the picker, profile kept.

## Findings (ranked)

### Major

**M1. Own-spot check on an unhidden pumpkin: no picture cue on the pumpkin, the chevrons ignore the open door, and he
loses every time he rolls into the named spot.**
- **Screen:** play (spot check: "THE TABLE?" / "THE CHAIR?" / "THE PLANT?", then "RUN AWAY!").
- **Issue:** Grandma names the spot the pumpkin is in or rolling into, with no "GRANDMA!" warning.
  - The pumpkin keeps its happy face (the "!" / O-mouth startle shows only for a pumpkin that was already hidden).
  - The sign stays a dim "FIND COVER" until pressed. When pressed, the chevrons point to other cover, even when the
    lit exit door is right beside him (R10).
  - He was caught 3 of 3 times (R6, R10, R11): half his losses. The startle the brief asks about never appeared in
    natural play.
- **Concern:** comprehension, recovery, feedback.
- **Caveats:** R6 he never pressed (I played him frozen). R11 I held the wrong tilt about 2.4 s after the arrows
  appeared. R10 is clean: he pressed on arrival and the arrows sent him away from the exit.
- **Suggestion (owner's call):**
  - Show the startle face and "!" on any pumpkin inside the named spot, hidden or not.
  - Let the chevrons (or the door's glow) point to the door once all three candies are on the shelf.
  - Consider the "GRANDMA!" warning before spot checks too.
- **Repro:** `trace.txt` lines 1482-1515 (R10: three candies, rolling left at -0.4 g. "THE CHAIR?" at 383160. Parked at
  the chair. `down 80 130` at 385910 shows ">>>". "GOTCHA DEAR!" by 386910.) R6: lines 825-842. R11: lines 1870-1899.
- **Screenshot:** `pumpkin-hold-playtest-6/08_R10_chair_named_next_to_open_door_loss.png`,
  `06_R6_table_named_unhidden_smiling_loss.png`, `09_R11_plant_named_ahead_chevrons_loss.png`,
  `frames/631_r10_named.png`, `frames/344_r6_l.png`.

### Minor

**m1. The armchair parks the pumpkin short of the door; moderate tilts stall.**
- **Screen:** play, left side.
- **Issue:** with a 0.35-0.4 g left tilt the pumpkin stops in front of the armchair, about 70 physical px from the
  glowing door. About 0.6 g is needed to finish (R1 parked about 1.5 s, R5 about 1.2 s, R10 parked and caught there).
  The start from rest is also sluggish: 0.9 s at 0.35 g did not leave the table (S2 803).
- **Concern:** challenge, controls.
- **Repro:** R1, `trace.txt` lines 150-174 (tilt -400 then -650 at 44660). R5 lines around 270-276 shots.
- **Screenshot:** `13_pumpkin_parks_at_chair_before_door.png`, `frames/057_r1_door_d.png`.

**m2. Pacing: Grandma returns almost at once, and hidden visits are risk-free waiting.**
- **Screen:** play.
- **Issue:**
  - "HIDDEN!" is followed by "GRANDMA!" 0.4-1.5 s after he lifts (R1 035, R10 586-591, R11 w11b).
  - Sweeps last 6-7 s.
  - About 32 hidden visits were all sweeps: once he hides on sight there is no decision left, just holding.
  - R3 was five sweeps in a row.
- **Concern:** boredom, pacing.
- **Repro:** R3 (`trace.txt` shots 116-168). R6 keep-hidden run (shots 293-316).
- **Screenshot:** `11_hidden_visits_always_sweeps_R6.png`.

**m3. No pressed state on START / READY / PLAY AGAIN / TRY AGAIN.**
- **Screen:** title, calibrate, win and loss cards.
- **Issue:** 500 ms into a firm press the button looks the same (frame 011 matches the idle title). It acts on lift.
  A kid who holds 1-3 s gets no sign that the press registered. Same as report 5's m3.
- **Concern:** feedback.
- **Repro:** `trace.txt` around 15560 (`down 82 131`, shot `011_start_pressed`).
- **Screenshot:** `pumpkin-hold-playtest-6/01_entry_title_calibrate.png`.

### Nit

- **n1. The sign keeps flipping when Grandma is not there.**
  - **Issue:** while rolling with no threat, the sign cycles "HOLD TO HIDE" → "FIND COVER →" → "← FIND COVER" (R2
    096-099). For a non-reader the arrows change direction for no visible reason.
  - **Concern:** comprehension.
  - **Repro:** R2 shots 096-099.
  - **Screenshot:** `frames/102_r2_l_g.png` area / `03_*`.
- **n2. Loss card tone.**
  - **Issue:** "YOU LOST" over the pumpkin baked into a pie, with "YUM YUM!" / "PIE TIME!" / "SWEET PUMPKIN!" / "OH
    MY! PIE!". Funny for most kids, but the hero gets eaten. Worth a designer's look against "failure is gentle".
  - **Concern:** tone.
  - **Screenshot:** `frames/196_r4_caught_d.png`, `frames/639_r10_lost.png`.
- **n3. Exit picture behaves differently on the title and in play.**
  - **Issue:** a 0.28 s brush exits from the title, while in play it takes a hold of about 0.6-0.9 s.
  - **Concern:** consistency. Harmless (no progress on the title).
  - **Repro:** `trace.txt` around 530240 (`701_brush`).
  - **Screenshot:** `12_ghost_decoy_and_exit.png`.

### Positives worth keeping

- Holding the sign hides the pumpkin, and sliding in with the thumb down hides it on arrival.
- Short lifts are forgiven ("QUICK! HIDE!" with no catch).
- The chevrons and their 0.7 s linger work as described above.
- The ghost decoy is a real, once-per-round choice.
- The shelf counts candies, and the lit door with its arrow shows the next goal without any reading.

## Gesture and interruption matrix

| Gesture / interruption | Result | Evidence |
| --- | --- | --- |
| Firm 1-1.8 s press on START / READY / PLAY AGAIN / TRY AGAIN | Acts on lift. No carried touch into the round. | trace 15560-20860, R2-R11 starts |
| Firm press on the sign at cover, held 2-15 s | Hides for the whole press | `02_*`, `11_*` |
| Short presses (2.4 s down, 0.4 s up) during a sweep | Pops out ("QUICK! HIDE!"), re-hides on re-press, never caught | `07_*` |
| Off-centre sign presses (70,133), (75,128), (85,128) | Work | R1, R5, R12 |
| Press in the open, board still | Chevrons toward cover, then caught (R4) | `04_*` |
| Press in the open while rolling (thumb slid down mid-roll) | Chevrons, then auto-hide on arrival | `05_*`, R11 681 |
| Lift after pressing in the open | Chevrons linger 0.7 s, then dim "FIND COVER" the same way | `10_*` |
| Press on the named spot (unhidden) | Not hidden, chevrons away from the named spot | `08_*`, `09_*` |
| Held finger when the hidden spot is named | **Not reached in play** | gap |
| Ghost press 1.2 s | Acts on lift: decoy, ghost dims | `12_*` |
| Exit picture, firm press in play | Launcher within about 0.9 s. The held lift does nothing there. | `12_*` |
| Exit picture, 0.28 s brush on the title | Leaves at once (n3) | `12_*` |
| Leave and re-enter (ordinary resume) | Title and calibrate again. Round not kept (no save by design). | `700_reenter` |
| Process restart (new `snap --serve`, saves kept) | Picker, profile kept, title again | S2, `803_*` |
| Untested | Dragging off the sign onto the room or ghost; pressing the pumpkin, candies or Grandma directly; tilts above 0.7 g or fast flips; a board lying flat; Shake; input during the frosting wipe; double tap on PLAY AGAIN; the 60 s idle dim; the shell's rest or daily-cap screen mid-round; two fingers; the browser emulator; a physical board | gaps |

## Supplementary QA (separate from the black-box pass)

- `make -C apps/porthole playtest` (20.9 s): **95 of 95 scenarios pass, 0 UI FAIL, 0 UI WARN**.
  - The Pumpkin scenarios 70-79 all pass.
  - `build/playtest/report.md` was read in full.
  - Its header names `f40be3d` (HEAD at run time, see Build).
- The `79_pumpkin_own_spot` sheet was viewed after play. It shows "pumpkin_startled" (the "!" and O mouth),
  "pumpkin_named_no" and "pumpkin_named_drifted_no" (smiling, "<<<"). So the startle exists and is covered by the
  suite. It just never came up in this natural session.

## Comparison with report 5

- Report 5 (Leo) saw his spot named while hidden 3 times (E1 caught, E2 and E3 escaped) and called the build
  acceptable.
- In this build and session it happened 0 times in about 32 hidden visits. Every spot check came while he was moving,
  and he lost all 3 checks that named the spot he was rolling into.
- I cannot tell from black-box play whether that is chance (seeded moves) or the timing change. The asymmetry, 0 of
  about 32 hidden versus about 7 of about 15 unhidden arrivals, is large enough that the owner should check that a
  hidden pumpkin can still be spot-checked.
- Report 5's m1 (Grandma returns quickly) and m3 (no pressed state) are still present (my m2, m3).

## Suggested brief changes

- Say whether "named his hiding spot" counts an unhidden pumpkin standing in the named spot. Here the two cases behave
  differently and only one has the startle.
- Allow a disclosed fixture, or a cap such as "play up to 12 rounds; if the event never occurs, record a gap". The
  stop condition depends on seeded luck, and this session could not meet it.
- Ask for reaction latency. Batch input lags a real child's reaction, and one loss here (R9) is harness lag. The
  frame-watching helper `watch.txt` (press about 300 ms after Grandma's face appears) fixed that from R11 on.

## Gaps

- The startle/jump-out on a hidden, named spot was never reached in natural play (0 of 2 required).
- No real child, no physical board, no browser emulator.
- The untested gestures listed in the matrix.
- One persona only.

## Verdict

**Needs fixes.**
- Hiding, the chevrons and their linger work, and nothing broke.
- But the own-spot checks a non-reader actually meets (while moving, unhidden) give no picture cue on the pumpkin and
  caused half his losses. In R10 the arrows sent him away from the open door.
- The startle this build was meant to deliver never appeared in 12 rounds.
- Recommend M1, and a check of spot-check targeting for hidden pumpkins. Then replay by a fresh agent.

## Evidence index (`pumpkin-hold-playtest-6/`)

- `01`-`13_*.png`: labelled sequences (240 px thumbnails, left to right, top to bottom) for each moment cited above.
- `frames/`: 32 native 480x480 frames cited above.
- `trace.txt`: every input with its game time (ms), and every frame name. Session 2 starts at the marker line (its
  clock restarts at 0).
- `driver.txt` (Python, saved as .txt): the serve driver. Session 1 ran it with `--fresh` hard-coded; the saved copy
  passes extra arguments instead.
- `go.txt` sends a batch. `watch.txt` is Leo's look-then-press reaction. `strip.txt` builds the sequences.
