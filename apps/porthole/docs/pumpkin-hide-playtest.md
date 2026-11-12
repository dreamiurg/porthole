# Pumpkin Hide & Seek: child-perspective black-box playtest (Leo, 6)

Independent emulator roleplay. **This is not a study with a real child and was not run on a physical device.**
Every observation comes from the headless host simulator (`snap --serve`) and its rendered 480x480 frames.

## Build and setup

- Revision: `d9a3f90` (clean tree). Executable: a private copy of `apps/porthole/build/host/snap`,
  sha256 `2d7ff974f30660535b74a083e4ba1d04835031624afc907cbb589b910494bafa` (unchanged after `make playtest`).
- Run: `snap --serve --fresh`, default clock (Tuesday 16:00), from a private scratch folder, driven through a
  small socket broker that sends only `down`/`move`/`up`/`tick MS`/`tilt X Y Z`/`frame`. Every input is logged
  with simulated milliseconds in `pumpkin-hide-playtest/traces/session1-serve-trace.txt` (driver sources saved
  as `driver-*.txt`). A tap is `down`, a hold of H ms (120 ms unless stated), `up`, then 40 ms.
- Tilt: "in the hands" = gravity (0, 0.5) g in the glass plane at calibration; Leo's tilts were +-0.25 to 0.4 g
  sideways, one over-tilt of 0.75 g and one wobbly upright (0, 0.9) calibration.
- Profile: Leo, age 6, frog face, no code, created through the screens. Pumpkin opened from the launcher tile.
  No `dbg`, `app`, clock skips or hidden-state queries during play.
- Determinism: replaying the trace in a fresh `--serve --fresh` process reproduces every compared frame
  pixel-for-pixel (`14_d`, `19_d`, `26_c`, `26_d`, `31_c`, `32_c`). The A-D variant traces change only the
  one gesture under test and were also run in fresh processes.

### Disclosure

Before and during play I read `apps/porthole/docs/playtesting.md`, the `playtest` and `tour` skills, and
lines 1-110 of `apps/porthole/tools/webemu.py` (only for the `--serve` protocol). A grep for `--serve` printed
one-line snippets of earlier launcher/admin playtest reports (protocol notes only, nothing about Pumpkin).
Loaded automatically: the root `CLAUDE.md` ("grab three candies and escape while Grandma searches with a
flashlight") and `apps/porthole/CLAUDE.md`, which describes Pumpkin as three hiding spots, three candies, then
the door, "sweeps, turn-backs, spot checks", a "frosting-to-pie retry sequence", and says the action sign and
ghost "drop only a press that began before the round did". So I knew the goal and that presses are gated
before reading any screen. I did not read game, os, shell or host source, `docs/superpowers/` or earlier
Pumpkin reports. After play ended I ran `make playtest` and read only the first comment line of
`tests/playtests/73_pumpkin_kid_taps.txt`, to check whether my findings were already covered.

## Persona and brief

Leo, 6. He can't read, holds the board in both hands to tilt it, taps firmly and often holds 1-3 s, sometimes
rests a thumb on the glass, and double-taps when he's worried. This is a simulated perspective only.

Dispatch brief, verbatim:

> You are an independent child-perspective playtester for Pumpkin Hide & Seek on Porthole, a round 480x480
> touchscreen with no buttons, emulated on the host. Follow apps/porthole/docs/playtesting.md and the `playtest`
> skill. Play source-free: don't read source (apps/porthole/games/, os/, shell/, host/), tests/playtests
> scripts, docs/superpowers/ or earlier playtest reports until you finish. Disclose anything that loaded
> automatically. Decide from the screen. Run the sim from your own copy of the snap binary in a private scratch
> subfolder.
>
> ## Persona
> Leo, 6, can't read, tilts the board with both hands, taps firmly and often holds 1-3 s, sometimes rests a
> thumb on the glass, and double-taps when worried. Simulated perspective only.
>
> ## Setup
> - `make -C apps/porthole snap`. Record `git rev-parse --short HEAD` and `sha256sum` of the snap you run.
> - `snap --serve --fresh`, default clock. Create Leo's profile (age 6, no code) through the screens, then open
>   Pumpkin from the launcher. Ordinary touches and tilt inputs only; no dbg, shortcuts or clock skips.
>
> ## Scope
> Play at least three rounds. Press HIDE short, long (2-3 s) and as a double tap, especially when Grandma's
> flashlight comes. Rest a thumb on the sign from the moment you press READY and lift it mid-round. Use the
> ghost with a long press. Is hiding reliable and fair? Does anything happen that Leo didn't mean? Can he
> always leave?
>
> ## Output
> Write apps/porthole/docs/pumpkin-hide-playtest.md, with screenshots and the input trace in
> apps/porthole/docs/pumpkin-hide-playtest/ (no .py files; save scripts as .txt). Include the revision and hash,
> the persona, this brief, findings ranked with repro steps, and gaps. Don't fix code or commit. Reply with ONLY
> (under 6 lines): a verdict (acceptable / needs fixes), the top findings, and the report path.

No revisions were made to the brief during this run.

## What Leo did (first play, in order)

Times are the trace's simulated ms. Screenshots are in `pumpkin-hide-playtest/shots/` and contact sheets in
`pumpkin-hide-playtest/sheets/` (`zs_NN.png` = the frames named `NN_*`).

1. **Launcher (8.3 s).** He picked the dark tile with the smiling jack-o'-lantern. While his finger was on it the
   caption read "Spooky" (`06_finger_on_tile.png`); the game's title says "PUMPKIN HIDE & SEEK". He can't read
   either, so it didn't matter to him.
2. **Title (`07_open_b.png`).** The pumpkin sits under the table among three candy corns. The words "TILT TO
   ROLL / START / GET CANDY" mean nothing to him; the one big cream sign looks pressable, and he held it for
   1.5 s. It acted when he lifted, with no pressed look while his finger was down (`08_start_pressed.png`).
3. **Calibrate (`09_after_start_a.png`).** "HOLD IT COMFY AND STILL", "READY!" and a READY sign. He pressed it
   and **left his thumb there 3 s** (the brief's resting thumb): nothing changed on screen (`10_ready_held3s.png`).
   The round started only when he lifted, so the thumb could not rest into play from READY.
4. **Round 1 (19.4-46 s): lost, 2 candies.** He tilted right and the pumpkin rolled (good: he learned tilting
   from the pumpkin moving, not from the words). A candy vanished with sparkles and a little candy filled the
   wall shelf. Grandma walked in ("GRANDMA!", `13_d.png`) while he was beside the plant. He tapped the only sign
   (it said HIDE) and the pumpkin popped into the plant pot. The flashlight swept, "PHEW!", then "HIDDEN!"
   (`14_d.png`, `15_e.png`). Tilting did not bring him out (`16_d.png`); he had to press the same sign again (now
   "COME OUT", indistinguishable to a non-reader). He held it 2 s; it worked on lift, and Grandma came straight
   back in ("THE CHAIR?") about a second later (`zs_18.png`). Rolling left for the next candy, the flashlight
   came toward him and he **double-tapped in a panic while the sign said FIND COVER**: nothing happened, with no
   feedback (`19_a.png`). He rolled under the chair, the sign flipped to HIDE for under 0.6 s
   (`19_c.png`), then "FOUND YOU!" (`19_d.png`), an orange frosting wipe and **"YOU LOST", "PIE TIME!"** with the
   jack-o'-lantern baked into a pie (`20_h.png`).
5. **Round 2 (53.6-69.7 s): lost, 2 candies.** TRY AGAIN, held 1 s, went straight into play with no calibrate.
   **Resting thumb:** he put a thumb on the sign's lower-left corner right away while tilting, kept it there 4 s,
   then lifted it with no intention to act: **the pumpkin hid** (`22_f.png` then `23_a.png`). It happened to be at
   the plant with Grandma arriving, so this time it helped him. He rested the thumb again while hidden
   (2.3 s) and lifted it: **the pumpkin came out** with Grandma still in the room (`24_d.png`); her beam was
   elsewhere, so he got away with it. Later, at the table with the flashlight sweeping toward him ("HMM?",
   `26_a.png`), he **pressed HIDE firmly and held it 2.5 s**. The pumpkin did not hide while his finger was down;
   "QUICK! HIDE!" appeared over his own pressed finger (`26_c.png`), and he was caught before he lifted
   (`26_d.png`).
6. **Round 3 (74.5-86 s): lost, 2 candies.** He double-tapped TRY AGAIN: one action, and the second tap did not
   leak into the new round as a HIDE (good, `zs_27.png`). He long-pressed the ghost (1.5 s): on lift it turned
   dark, a mug fell off the shelf, Grandma said "WHAT WAS THAT?" and turned away (`28_e.png`), which is a clear,
   delightful cause and effect. At the plant, Grandma walked in and he **double-tapped HIDE** (two 120 ms taps,
   100 ms apart). The pumpkin stayed out (`31_a.png`); "QUICK! HIDE!", "THERE YOU ARE!", pie
   (`31_c.png`, `31_d.png`, `32_c.png`).
7. **First voluntary stopping point: here, after round 3** (about 67 s of play, three losses, two of them while
   he was pressing the button he had learned hides him). Leo would most likely say "it's broken, I pressed hide!"
   and either quit or hand it to a grown-up. Everything below continued for coverage, not because he would.
8. **Leaving.** One short tap on the arrow picture above "YOU LOST" went straight to the launcher (`zs_33.png`).
   Reopening showed the title again; a lost round is not remembered (rounds are short, so nothing is lost).
9. **Round 4 (89.8-117 s): won.** Short, single taps only. Under the chair? No, the table: hidden, "PHEW!",
   "HIDDEN!" (`zs_38.png`); out, last candy by the plant, the door lit up with an arrow (`zs_40.png`), a near
   miss ("QUICK! HIDE!" then "PHEW!"), "TO THE DOOR!", then "YOU DID IT! GRANDMA IS PUZZLED" with the pumpkin in
   sunglasses and the three candies (`41_h.png`). The door arrow is a good picture cue. The win screen shows the
   pumpkin back at the table rather than outside the door, so "I got out!" is not visible.
10. **Rounds 5-6 (probes).** Holding PLAY AGAIN 3 s started on lift (`zs_42.png`). A short tap on the arrow
    picture during play did nothing, with no feedback (`43_a.png`); holding it left within 0.3-0.6 s, and his
    still-down finger did nothing on the launcher (`zs_44.png`). Tapping the chair, plant, table or the pumpkin
    itself never hides (`45_taptable.png`, `46_holdpumpkin.png`). Taps 3-4 px outside the sign are ignored; one
    just inside its corner works (`zs_47.png`). Dragging off the sign before lifting cancels, and sliding onto it
    does nothing (`zs_49.png`). The ghost held 2 s while hidden works on lift (`zs_48.png`). The arrow picture
    leaves from the title and the calibrate page with a short tap (`zs_50.png`).
11. **Round 7 (149-171 s): lost, 1 candy.** He calibrated holding the board nearly upright and wobbling: fine,
    and the pumpkin stayed put (`zs_51.png`). Over-tilting at 0.75 g kept the pumpkin controllable
    (`zs_52.png`). At the chair, with the beam approaching, his usual firm 1.2 s HIDE: "QUICK! HIDE!" appeared while
    he was pressing (`56_b.png`), and he hid on lift just in time (`56_c.png`). He came out, Grandma walked in
    again about a second later (`zs_57.png`), and at the table under "QUICK! HIDE!" he pressed HIDE for 1 s: caught
    ("PEEKABOO!") half a second into his press (`58_a.png`), pie ("SWEET PUMPKIN!").

Outcome tally: 7 rounds started: 1 win (round 4), 4 losses (rounds 1, 2, 3, 7), 2 left mid-round on purpose
(rounds 5 and 6). Rounds last about 20-30 s.

### Controlled repros (same trace, one gesture changed, fresh process)

| Moment | Leo's gesture | Result | Same moment, other gesture | Result |
| --- | --- | --- | --- | --- |
| Round 2, table, beam coming (trace t=67160) | HIDE held 2.5 s | caught at 1.7-2.5 s, finger still down (`26_c`, `26_d`) | varA: 120 ms tap | hidden, "PHEW!", "HIDDEN!" (`A_26_c`, `A_26_h`, `sheets/varA_short_tap.png`) |
| Round 3, plant, Grandma walks in (t=81420) | double tap | first tap hides, second comes out ~0.3 s later (`C_up1`, `C_up2`, `C_after`), then caught (`31_d`) | varB: one tap | hidden through the whole search (`sheets/varB_single_tap.png`) |
| Round 7, table, "QUICK! HIDE!" (t=168420, trace line 771) | HIDE held 1 s | caught at ~0.5 s, finger still down (`58_a`) | varD: 120 ms tap | hidden, "PHEW!", "HIDDEN!" (`D_b`, `D_d`) |

## Qualitative assessment

**Can Leo recognize the goal and actions and tell what changed?** Partly. Pictures carry most of it: the
pumpkin rolls when he tilts, candies vanish with sparkles into the shelf counter, Grandma and her yellow beam
are an obvious threat, the pot or chair visibly swallows the pumpkin, and the lit door with an arrow says "go
here". The words don't help him at all (every instruction, the title, the three sign labels, Grandma's lines,
"YOU LOST"/"YOU DID IT!"). The sign is the weak point. HIDE, FIND COVER and COME OUT look identical to him, so he
can't tell when pressing will do something. Pressing during FIND COVER does nothing with no feedback, and the
sign never looks pressed. He learned "press the sign to hide" by round 1, a positive case, and then the sign
failed him exactly when he pressed it his way.

**Interesting decisions, fair challenge, useful feedback?** The core loop is good for this age: dash for a
candy, watch Grandma, duck into the nearest spot, wait out the sweep, sneak to the door. The ghost is a lovely
"save me" card with visible cause and effect. Round 4 had real tension and a satisfying escape. But it is not
fair to Leo's hands. Hiding happens on lift, so his natural 1-3 s press keeps the pumpkin in the light for that
long ("QUICK! HIDE!" appears under his pressed finger), and a worried double tap hides and un-hides. In 3 of
his 4 losses he was pressing HIDE at a hiding spot at the time. Twice Grandma came back about a second after he
came out (rounds 1 and 7). Combined with the press latency that feels like a trap, though it may be the
designed "turn-back". Recovery after a loss is easy (TRY AGAIN, a fresh round, nothing lost).

**Where does it get repetitive, passive, confusing or frustrating? When would Leo stop?** The waits while
hidden (about 3-6 s of sweeping) are tense rather than boring. The frustration comes from being caught while
pressing HIDE (rounds 2, 3 and 7) and from silent FIND COVER presses (round 1). Leo would stop after round 3
(about 67 s of play, three straight losses). If he got lucky with quick taps, round 4's win is a satisfying
natural stopping point. "YOU LOST" plus the pumpkin baked into a pie is cartoonish, but it is the one harsh word
on the screen for a game whose audience rule is "failure is gentle".

**Can imprecise input, a wrong guess or an interruption be recovered from?** Mostly yes. Off-sign taps are
ignored, a drag off cancels, double taps never leak across a page change, scenery taps do nothing, he can
always leave (a hold on the arrow picture in play, a tap elsewhere) and a re-entry is a fresh round, as
designed. The exceptions are the in-round action sign's release timing and toggling, above. A press that
drifts about 5 mm over 2 s on READY is silently dropped (`35_after_drift.png`), so he has to press again
without knowing why.

## Findings (ranked)

### Major

**M1. HIDE takes effect only when the finger lifts, so Leo's normal 1-3 s press gets him caught.**
- **Screen**: `pumpkin_play`.
- **Issue**: While HIDE is held, the pumpkin stays in the open and can be found. "QUICK! HIDE!" even appears under
  the pressed finger. Two of the four losses (rounds 2 and 7), one near miss (round 7 at the chair).
- **Concern**: challenge / feedback / recovery (fairness).
- **Repro**: replay `pumpkin-hide-playtest/traces/session1-serve-trace.txt` into `snap --serve --fresh` to
  trace line 288 (`down 80 131` at t=67160, released at t=69660): caught at t≈68.9-69.7 s. `varA-serve-trace.txt`,
  the same moment with a 120 ms tap, survives. Second instance: trace line 771, `down 80 131` at t=168420 (held 1 s), caught.
  `varD-serve-trace.txt` (short tap) survives.
- **Screenshot**: `pumpkin-hide-playtest/shots/26_c.png`, `26_d.png`, `58_a.png`, `56_b.png`; contrast
  `A_26_h.png`, `D_d.png`.
- **Suggested direction** (owner decides): act on the press-down that began on the sign during the round (the
  press-before-round gate can stay), show the sign pressed while it's held, and make the lift of that same press
  do nothing.

**M2. A double tap on HIDE hides and immediately un-hides.**
- **Screen**: `pumpkin_play`.
- **Issue**: The second tap of a worried double tap (presses ~280 ms apart) is taken as COME OUT, so the pumpkin
  is back in the open as Grandma searches. This was round 3's loss.
- **Concern**: recovery / feedback (a shell-wide convention is that a double tap acts once).
- **Repro**: `varC-serve-trace.txt`: at t=81420, `down 80 131`, 120 ms, `up`, 100 ms, `down 80 131`, 120 ms,
  `up`. The frames show HIDE, then COME OUT, then HIDE (`C_down1`/`C_up1`/`C_up2`). `varB-serve-trace.txt` (one tap)
  stays hidden.
- **Screenshot**: `pumpkin-hide-playtest/shots/31_a.png`, `31_d.png`, `C_up1.png`, `C_up2.png`;
  `sheets/varC_double_tap.png`.
- **Suggested direction**: ignore a new press on the sign for about 600 ms after it toggles (or after its press
  began), as the shell does for double taps.

### Minor

**m1. Lifting a resting thumb acts.** A thumb laid on the sign during the round and lifted 2-4 s later toggles
hide (`22_f.png` then `23_a.png`: an unintended hide) or comes out with Grandma in the room (`24_d.png`). This has
the same root as M1 (any press is a tap on lift, however long), and the M1 fix covers it if the lift itself stops
acting. Repro: session trace line 227 (t=53600 `down 48 138`, lift at t=56600) and line 251 (t=58080 `down 112 139`, lift at t=60380).
Concern: recovery / feedback.

**m2. The sign gives no feedback for a press.** START, READY, HIDE/COME OUT and the ghost never look pressed
while a finger is on them (`08_start_pressed.png`, `10_ready_held3s.png`, `26_c.png`). Presses while the sign
says FIND COVER do nothing, silently (`19_a.png`). A non-reader can't tell FIND COVER from HIDE, and for a kid
who holds 1-3 s that is exactly what invites the double taps in M2. Concern: feedback / comprehension. A
pictorial state (e.g. an open pot or box icon for "hide here", dimmed when there is nowhere to hide) would help
more than words.

**m3. Tapping the hiding spot or the pumpkin does nothing.** A non-reader's first idea when Grandma comes is to
touch the chair, plant, table or pumpkin. No response, not even a wiggle (`45_taptable.png`,
`46_holdpumpkin.png`). Leo found the sign only because it's the only button. Repro: session trace t=129-133 s,
taps at (30,75), (125,70), (80,72), (80,98). Concern: comprehension.

**m4. Grandma returns about a second after he comes out.** This happened in rounds 1 and 7 (`zs_18.png`,
`zs_57.png`). On its own it may be the intended turn-back. With the M1 latency it feels like a trap, and Leo
gets no hint that waiting longer is safer. Retest after M1. Concern: challenge / pacing.

**m5. "YOU LOST" plus the pumpkin-pie ending.** This is the only harsh wording on screen. The pie art is funny,
but the hero being baked reads as an ending, and "YOU LOST" contradicts the gentle-failure rule (`20_h.png`).
Consider a picture-first, softer card (e.g. "Found you! Again?"). Concern: comprehension / tone.

**m6. A drifting READY press is dropped silently.** A press drifting 15 logical px (5 mm) over 2 s does nothing
and shows nothing (`35_after_drift.png`; session trace line 397, t=91340). Concern: feedback.

### Nit

- **n1.** The launcher caption says "Spooky" but the game's title is "PUMPKIN HIDE & SEEK" (`06_finger_on_tile.png`).
  Grown-ups only. Concern: consistency.
- **n2.** The exit is a wall picture with an arrow, not the orange home button used elsewhere. It works, a tap
  on non-play pages and a ~0.5 s hold in play, but a short tap in play shows nothing, so a quick tapper might
  think there is no way out (`43_a.png`). Fine for Leo, who holds firmly. Concern: consistency / feedback.
- **n3.** The win card shows the pumpkin back under the table, not out of the door, so the escape itself isn't
  pictured (`41_h.png`). Concern: feedback.

### Positives worth keeping

Tilt control is gentle and capped, even when over-tilted. The candy shelf counter and sparkles are clear. The
door arrow is a good cue. The ghost's cause and effect is great. Double taps never leak across a page change.
Off-target taps and drags off the sign are safe. There is always a way out.

## Gesture and interruption matrix

| Gesture / case | Tested | Result |
| --- | --- | --- |
| HIDE short tap at a spot | yes (R1, R4, varA/B/D) | hides, reliable |
| HIDE held 1 s / 1.2 s / 2.5 s under a sweep | yes (R7, R7, R2) | caught / near miss / caught: **M1** |
| HIDE double tap at a spot | yes (R3, varC) | hide then un-hide: **M2** |
| Double tap while FIND COVER | yes (R1) | nothing, no feedback: m2 |
| COME OUT held 2 s | yes (R1) | comes out on lift |
| Thumb resting from READY | yes (R1) | READY acts on lift; a thumb can't carry into play |
| Thumb laid on the sign in-round, lifted after 4 s / 2.3 s | yes (R2) | hides / comes out: m1 |
| Ghost held 1.5 s / 2 s | yes (R3, R6) | distracts Grandma on lift; used up for the round |
| Taps 3-4 px off the sign; corner just inside | yes (R6) | ignored; acts |
| Drag off the sign / slide onto it | yes (R6) | cancels / nothing |
| Taps on scenery and pumpkin | yes (R6) | nothing: m3 |
| TRY AGAIN double tap; PLAY AGAIN held 3 s | yes | one action, no leak into play; starts on lift |
| READY press drifting 5 mm | yes | dropped silently: m6 |
| Calibrate while wobbling upright; over-tilt 0.75 g | yes (R7) | fine |
| Leave: title, calibrate, lost (tap); play (hold) | yes | all reach the launcher; the carried finger does nothing |
| Leave mid-round and come back | yes (R5/R6) | title, new round (no persisted state by design) |
| Process restart mid-round | **gap** | not tested (no persisted state, so low risk) |
| Session rest / daily cap firing mid-round | **gap** | not tested |
| Shake, flat-on-table grip, evening/night tint | **gap** | not tested |
| Physical board, real touch noise, real children | **gap** | emulator roleplay only |

## Automated suite (supplementary)

`make -C apps/porthole playtest` at `d9a3f90`: **93 of 93 scenarios pass, 0 UI FAIL, 0 UI WARN**. That includes
`70_pumpkin_win`, `71_pumpkin_loss`, `72_pumpkin_tilt`, `73_pumpkin_kid_taps`, `74_pumpkin_monkey` (ASan/UBSan),
`75_pumpkin_navigation`, `76_pumpkin_ghost` and `77_pumpkin_rustle`. I read the report in full. An ad hoc `ui`
audit of every Pumpkin frame in a script-mode replay of my session (`pumpkin_intro`, `_calibrate`, `_play`,
`_catch`, `_icing`, `_lost`, `_won`) found no size, edge, clipping or contrast issue. The sign is 80x24, the
arrow picture 24x22 and the ghost 24x24. No existing scenario covers a long press or a double tap on the in-play
HIDE sign. After M1/M2 are fixed I recommend a scenario that holds HIDE 1-2 s and double-taps it under a sweep and
expects the pumpkin to stay hidden. I did not add one: it would fail CI before the fix, and the brief asked for
no commits.

## Iteration history and verdict

- Iteration 1 (this run, `d9a3f90`, snap `2d7ff974...`): findings above. No game changes have been made yet, so
  there is no retest.
- **Verdict: needs fixes.** M1 and M2 make hiding unreliable and unfair for this persona's normal press style.
  He loses while pressing the right button and would stop after about three rounds. After the fix, a fresh
  independent agent should replay rounds with 1-3 s holds and double taps under a sweep, plus the resting-thumb
  probe.
- Suggested brief improvement: add "press HIDE in the same moment two ways (short vs. long, single vs. double)
  and compare outcomes in fresh processes". Deterministic `--serve` replays made the fairness claim concrete.
