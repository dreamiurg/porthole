# Pumpkin Hide & Seek, hold-to-hide retest 3: child-perspective black-box playtest (Leo, 6, and Zoe, 8)

This is independent emulator roleplay. **It is not a study with real children, and it was not run on a physical
device.** Every observation comes from the headless host simulator (`snap --serve`) and its 480x480 frames.

## Build and setup

- Revision: `3338eec` (clean tree). `make -C apps/porthole snap` was already up to date.
- Executable: a private copy of `apps/porthole/build/host/snap` in a session scratch folder, sha256
  `00369100576031c5f0e8190e33d0dde31a7a42e1165e3af4f3d307446eb45762`. The hash was the same after
  `make playtest`. `snap-asan` (suite only): `ebcba86ca30470060311fab7617dc74d50930c6ff4bad9981f8b444471087720`.
- Run: `snap --serve --fresh` with the default clock (Tuesday 16:00), started in a private subfolder. Each persona
  had their own process: Leo (frog, age 6, no code) and Zoe (fox, age 8, no code). Both profiles were created
  through the name, face, age and code screens, and the game was opened from the launcher's jack-o'-lantern tile
  ("Spooky").
- Input: only the serve protocol's `down X Y` / `move X Y` / `up` / `tick MS` / `tilt X Y Z` / `frame`. Each
  trace in `pumpkin-hold-playtest-3/traces/` is one line per action:
  - `tap X Y MS` is `down`, `tick MS`, `up`, `tick 40`.
  - `slide` / `move` lines are raw moves.
  - `film NAME MS STEP` ticks STEP ms at a time and grabs a frame after each step.
  - `shot NAME` grabs one frame.
  - Comment lines give the persona's intent.

  Coordinates are logical (0-160). Tilt is gravity in milli-g in the glass plane: Leo held the board at (0, 500)
  and steered at x +-300 to 650; Zoe held it at (0, 400) and steered at +-450 to 800.
- Replay: a trace is replayed from scratch in a fresh `--serve --fresh` process every time it is extended. Four
  frames from Leo's full trace (`l021_caught_005`, `l050_r5hide_014`, `l062_r6door_002`, `l070_r7door_008`) were
  re-rendered in a separate fresh process and are byte-identical.
- No `dbg`, `app`, `screen`/`debug` query, clock skip or other hidden-state probe was used anywhere in the
  black-box pass.

### Disclosure

Before play I read `apps/porthole/docs/playtesting.md`, the `playtest` and `tour` skills, and lines 40-100 of
`apps/porthole/tools/webemu.py` (the `--serve` protocol only). Two things loaded automatically:

- The root `CLAUDE.md`: "grab three candies and escape while Grandma searches with a flashlight".
- `apps/porthole/CLAUDE.md`, which loaded when I touched that folder. It says the game has three hiding spots,
  "a seeded mix of Grandma moves (sweeps, turn-backs, spot checks)" and "a frosting-to-pie retry sequence". It
  also says the action sign "hides the pumpkin exactly while a finger that touched it (pressed there, or slid on
  where it can hide at once) stays down, wherever it drifts", that the ghost acts on lift, and that both ignore
  a press that began before the round.

So the tester knew in advance that hiding is hold-based and that slides and drift are meant to work. The
personas still decided everything from the screen, and Leo's choices were limited to what a non-reader could
see. I did not read game, os, shell or host source, `tests/playtests/`, `docs/superpowers/`, AGENTS.md files or
earlier reports during play. After all play and the automated suite, I read `docs/pumpkin-hold-playtest-2.md`
for the comparison at the end. After that reading I ran one supplementary ghost probe, labeled as such.

## Personas and brief

- **A. Leo, 6:** can't read and tilts with both hands. He presses firmly for 1-3 s and double-taps when worried.
  Sometimes he rests a thumb on the glass and slides it.
- **B. Zoe, 8:** a confident reader who taps quickly, likes to work out the rules, and tries to cheat by
  mashing.

Both are simulated perspectives only.

Dispatch brief, verbatim:

> You are a fresh, independent child-perspective playtester for Pumpkin Hide & Seek on Porthole, a round 480x480
> touchscreen with no buttons, emulated on the host. Follow apps/porthole/docs/playtesting.md and the `playtest`
> skill.
>
> This is a retest after fixes to a changed core mechanic, so play SOURCE-FREE and first-play. Until you finish,
> don't read source (apps/porthole/games/, os/, shell/, host/), tests/playtests scripts, docs/superpowers/,
> AGENTS.md files or earlier playtest reports. Disclose anything that loaded automatically. Decide everything
> from the screen. Run the sim from your own copy of the snap binary, in a private scratch subfolder.
>
> ## Personas (two passes)
> A. Leo, 6: can't read, tilts with both hands, presses firmly for 1-3 s, double-taps when worried, sometimes
> rests a thumb on the glass and slides it.
> B. Zoe, 8: a confident reader who taps quickly, likes to work out the rules, and tries cheating by mashing.
> These are simulated perspectives only.
>
> ## Setup
> - Run `make -C apps/porthole snap`. Record `git rev-parse --short HEAD` and the `sha256sum` of the snap you run.
> - Start `snap --serve --fresh` with the default clock. Create the profile (age 6 or 8, no code) through the
>   screens, then open the Pumpkin game ("Spooky") from the launcher.
> - Use ordinary touches and tilt inputs only: no dbg, shortcuts or clock skips.
>
> ## Scope
> Play at least six rounds per persona, as each would.
> - Make a point of meeting Grandma checking the spot where the kid is hiding: keep playing until it happens at
>   least twice for Leo.
> - Does Leo understand, without reading, when his hiding spot is unsafe and he must leave? Can he get away in
>   time?
> - Does pressing the sign with no cover nearby clearly say "not here, go that way"?
> - Are holds, slides, double taps and a resting thumb all handled the way the kid expects?
> - Can Zoe beat Grandma by mashing?
> - Do they collect all the candy and escape? Where do they get frustrated or bored? Would they play again?
> Record win/loss per persona.
>
> ## Output
> Write apps/porthole/docs/pumpkin-hold-playtest-3.md, with screenshots and the input trace in
> apps/porthole/docs/pumpkin-hold-playtest-3/. Don't save .py files; save scripts as .txt. Include:
> - the revision and hash
> - the personas and this brief
> - win/loss per persona
> - findings ranked, with repro steps
> - your qualitative assessment
> - gaps
>
> Afterwards you may read apps/porthole/docs/pumpkin-hold-playtest-2.md and compare. Don't fix code or commit.
> Reply with ONLY: a verdict (acceptable / needs fixes), win/loss per persona, the top findings and the report
> path, in under 8 lines.

The brief was not revised during this run.

## Outcomes

| Persona | Natural rounds | Wins | Losses | Branch probes (same trace prefix, not counted) |
| --- | --- | --- | --- | --- |
| Leo, 6 | 8 | 7 (R2-R8) | 1 (R1) | A: kept holding during a check of his spot, caught. C: forgot his thumb on the glass, rustled, caught. |
| Zoe, 8 | 6 | 4 (R1, R2, R5, R6) | 2 (R3, R4) | B: stood still at the table in a sweep, caught. |

Leo collected all three candies and escaped in every round from R2 on. Zoe did in R1, R2, R5 and R6.

- Leo's R1 loss was natural: he kept tilting toward the last candy into Grandma's light.
- Zoe's R3 loss was a rush that brushed past Grandma at the plant. Her R4 loss was a deliberate cheat test:
  she mashed during a check of her own spot.

Rounds took about 15-60 s. Leo spent about 6.5 minutes in the game and Zoe about 2 minutes.

**Checks of the kid's own hiding spot:** two, as the brief required.

1. Leo R5 at the table, natural (`06_leo_r5_spotcheck_escape.png`). Leo escaped. In branch A he kept holding
   instead and was caught (`07_*.png`).
2. Leo R6 branch C at the plant, triggered by his own rustle (`09_*.png`): caught.

Zoe met a third (R4, the table, triggered by her deliberate wiggle).

## What Leo did (first play)

1. **Entry (`01_leo_entry.png`; `leo.txt` lines 1-53).** He pressed the jack-o'-lantern tile for 1.5 s. The
   caption said "Spooky", and the game opened on lift. The title has a pumpkin, candy corns and one big cream
   box, so he pressed that. Pressing the pumpkin and the ghost on the title did nothing. The calibrate screen
   shows hands holding a board. Its dark "READY!" label sits just above the cream READY button, and **his first
   press went to the label**, which did nothing (line 50). His second went to the button (line 52), and play
   began. The pumpkin sat under the table, three candy corns lay on the rug and an empty three-slot shelf hung
   on the wall.
2. **R1: lost, natural (`02_leo_r1_loss.png`, lines 54-73).** He tilted left, and a candy sparkled up onto
   the shelf. The sign went dark slate with an arrow ("FIND COVER ->" / "<- FIND COVER") in the open and cream
   at the chair. Grandma came with her flashlight. He pressed the pumpkin itself (nothing), then the big box,
   and **the pumpkin tucked under the chair and the box turned orange** ("HIDING", `frames/l017_hold_011.png`).
   He held about 3 s, past "PHEW!" and "HIDDEN!". Then he rolled right for more candy. Grandma came back
   pointing at "THE PLANT?". He got the second candy, kept tilting right into her light ("RUN AWAY!" is
   unreadable to him) and got "GOTCHA DEAR!", the orange frosting wipe and "YOU LOST" with a pie
   (`frames/l021_caught_005.png`). The pie is funny, not upsetting.
3. **R2: won (lines 74-99).** He grabbed a candy, saw the beam start and pressed the dark box at once (line 79).
   **Nothing visibly happened for the whole 2 s press** (`03_leo_nocover_no_feedback.png`, finding m1). He kept
   the finger down and tilted toward the arrow. The moment the pumpkin reached the chair it hid by itself
   (`05_leo_hold_autohide.png`). He then held through three sweeps (about 18 s), let go after one, crossed the
   room for two candies while Grandma pointed at "THE CHAIR?" (which he had just left), and rolled out of the
   lit door: "YOU DID IT! GRANDMA IS PUZZLED" (`frames/l033_door_004.png`).
4. **R3 (lines 100-126) and R4 (lines 127-145): won.** A worried double tap on PLAY AGAIN started one round and
   did not hide the pumpkin. His routine became "hold the box when Grandma comes, let go when she leaves, roll
   for candy". Sweeps never caught a held hide. In R3 he hid about 30 s in total (table, then chair).
5. **R5: own-spot check, escaped (`06_leo_r5_spotcheck_escape.png`, lines 146-161).** He hid at the table from
   the start. After one sweep, Grandma came back and pointed at the table, which got a "?" bubble. **On that
   same frame his orange HIDING box turned red ("RUN!")**, and little sweat drops appeared beside the pumpkin
   under the table (`frames/l050_r5hide_014.png`). To a non-reader this reads as: she is pointing at my table,
   the box went red. He let go about 0.4 s later and tilted away from her. The pumpkin rolled to the chair side,
   picking up a candy on the way, and the light swept the table and then the floor and missed him. He then got
   the other two candies and escaped.
   - Branch A (`leoA.txt` line 152, `07_*.png`): if he keeps holding, "THE TABLE?" lasts about 3.2 s, then
     "RUN AWAY!" with the light on the table about 1 s, then "FOUND YOU!".
   - That is about 4.3 s from the first warning frame to the catch. In the main branch he was out of reach
     within about 1.5 s.
6. **R6: thumb probes, won (`08_leo_r6_thumb_slide_drift.png`, lines 162-193).**
   - He held PLAY AGAIN for 3 s. Nothing changed until he lifted (no pressed look), and the carried press did
     not hide the pumpkin.
   - His thumb rested on the rug at the left edge for 3 s while he tilted: no effect.
   - When Grandma came, he slid that thumb down onto the box and the pumpkin hid at once (at the plant). The
     thumb then drifted off the box into the room and the pumpkin stayed hidden.
   - **Then he forgot the thumb and tilted left to leave.** The pumpkin would not move. The bubble said "SHH!",
     then "SHE HEARD YOU!", and a "!" appeared at the right edge where Grandma comes in
     (`frames/l060_r6stuck_003.png`).
   - Main branch: he noticed the pumpkin was stuck, lifted his thumb, rolled out and won.
   - Branch C (`leoC.txt`, `09_*.png`): if the thumb stays down, Grandma comes back pointing at "THE PLANT?",
     the box turns red and he is caught about 8 s after the rustle began. Meanwhile the bubble keeps switching
     between "SHH!", "SHE HEARD YOU!", "HMM?", "PHEW!", "THE PLANT?" and "RUN AWAY!", every 0.3-0.5 s
     (finding n1).
7. **R7 and R8: won (lines 194-232).** He hid at the table and the chair for 16-20 s at a stretch. Every visit
   was a sweep, with no check of his spot. On the R7 win card the "?" bubble over the chair and Grandma's
   pointing arm stayed on screen (`frames/l070_r7door_008.png`, n3).
8. **Exit and resume (`11_leo_exit_and_resume.png`, lines 233-245).** A quick tap on the arrow picture at the
   top did nothing. A firm press left for the launcher before he lifted (about 0.45 s, measured in Zoe's
   branch X, `16_zoe_exit_hold_progress.png`: a small orange bar fills under the frame). The lift on the
   launcher opened nothing. Reopening Spooky gave the title, calibrate and a fresh round; the game keeps no
   state between visits.
9. **First voluntary stopping point: around round 4-5** (about 3-4 minutes). By then he had won three in a row,
   and every round starts the same way: pumpkin under the table, Grandma's first visit always a sweep, about
   4 s in. Holding through sweeps is safe and mostly waiting (8-20 s per hide). The R5 check, with a red box
   and Grandma pointing at his table, is the one new thrill, and it might carry him one or two rounds more.
   R6-R8 were played for coverage.

## What Zoe did (first play)

1. **Entry.** She read "TILT TO ROLL / START / GET CANDY", "HOLD IT COMFY AND STILL" and, in play, "HOLD TO
   HIDE". She mashed READY four times. That started one round, and the extra presses did not hide the pumpkin.
2. **R1: won (`12_zoe_mash_sweep_vs_idle.png`, `zoe.txt` lines 41-244).** Her cheat idea: mash the sign
   (80 ms down, 120 ms up, about 5 a second) at the table through a sweep. She survived ("PHEW!").
   - Control branch B (`zoeB.txt`): standing at the table without touching gets "QUICK! HIDE!", then
     "PEEKABOO!", then the pie. So a mash counts as hiding.
   - Mashing while tilting toward a candy (line 127) did not move the pumpkin at all. Each press freezes and
     hides it, so a mash is just a choppy hold, with no gain.
   - She then played properly and escaped: "YOU ESCAPED! WITH THE CANDY".
3. **R2: won by rushing, never hiding (`14_*.png`, lines 245-267).** A mashed PLAY AGAIN started one round. She
   rolled hard right and left through two sweeps without touching the sign: "FREEDOM! AND ALL THE CANDY" in
   about 15 s (`frames/z020_rushdoor_001.png`).
4. **R3: lost by rushing (lines 268-275).** The same plan. She brushed past Grandma at the plant during
   "QUICK! HIDE!": "PEEKABOO!", then the pie. So rushing is a gamble rather than a sure thing.
5. **R4: lost, cheat test on a spot check (`13_zoe_mash_spotcheck_and_carry.png`, lines 276-367).** She hid at
   the table, then wiggled the board while hiding to see what would happen: "SHE HEARD YOU!" for about 2.5 s,
   then "THE TABLE?" with the red RUN! box. She switched to mashing. **Released, the box now read "<- FIND
   COVER"**: the table no longer counts as cover, so the mash did not hide her. "RUN AWAY!", "FOUND YOU!", pie.
   Mashing does not beat a check. Her mash continued through the frosting wipe, and **one of the presses hit TRY
   AGAIN**: the "YOU LOST" card was on screen under a second before a new round began (finding m3).
6. **R5: won (lines 368-376).** She grabbed candy between visits, held at the plant during a sweep and rolled
   out: "SNEAKY PUMPKIN! CANDY IS YOURS".
7. **R6: no-cover probe, then won (`15_zoe_nocover_chevrons.png`, lines 377-398).** She stopped in the open
   right of the table, waited until the pumpkin was still, and pressed the dark "<- FIND COVER" box. **For about
   0.9 s the box showed big "<<<" chevrons pointing to the cover**, then went back to "<- FIND COVER" while she
   held. Nothing hid. She went to the plant, hid through a sweep and escaped.
   - Dragging off PLAY AGAIN before lifting cancelled it (`17_*.png`).
   - A tap 2 px above the button still counted.
8. **Zoe's stopping point: after round 5-6** (about 2 minutes). By then she had worked out every rule she met:
   - hold at cover;
   - a mash is a choppy hold;
   - moving while hidden makes noise;
   - when Grandma names your spot it stops being cover;
   - rushing sometimes works.

   One room, three spots, the same start every round and a slow pumpkin give her little left to discover. She
   would play again another day, but would want "harder" or "a new room".

## Qualitative assessment

**Can each persona recognize the goal and actions from the screen and tell what changed?**
- **Zoe:** yes, from the words, in R1.
- **Leo:** yes, from pictures, by the end of R1:
  - The tilt moves the pumpkin.
  - Candy sparkles up onto the shelf, and the shelf fills.
  - Grandma and her light are the danger.
  - The cream box tucks the pumpkin into furniture and turns orange.
  - A lit door with a big arrow is the way out.

  Two first-time stumbles cost him nothing: pressing the "READY!" label before the button, and pressing the
  pumpkin itself. The new own-spot warning is pictorial: in the same frame, the box goes from orange to red,
  Grandma points at his spot with a "?" bubble over it, and sweat drops appear. He understood "she means me" and
  left in time with more than a second to spare.

**Decisions, consequences, feedback, fairness?**
- Zoe has real choices: which spot, when to cross, candy greed versus safety, and rush versus hide. Every loss
  had a reason she could state.
- For Leo the checks are now fair: about 4.3 s from the first warning frame to the catch, and the red box
  appears at the start.
- Sweeps are lenient. Zoe won R2 without hiding once, and a held hide always beats a sweep, so hiding through
  sweeps is safe but passive.
- The rustle rule is a good idea ("don't wiggle while hiding") but is taught only in words, and its bubble
  flickers between messages.
- Pictures carry the core loop. Reading is needed only for the rustle and for "THE TABLE?" (which the pointing
  arm and "?" bubble also show).

**Where does play become repetitive, passive, confusing or frustrating? When would each stop?**
- Repetitive: every round starts with the pumpkin under the table and a first-visit sweep about 4 s in, and the
  candy spots are about the same. Leo would stop around R4-5, Zoe around R5-6.
- Passive: holds through sweeps of 8-20 s, with Grandma returning about 2-3 s after "HIDDEN!".
- Confusing:
  - Leo's silent press on the dark box while still rolling (m1).
  - The stuck pumpkin when his forgotten thumb keeps it hidden (m2). The red box eventually signals danger, but
    nothing pictorial explains that the thumb is the cause.
- Frustrating: nothing seriously. Losses are short and funny, and the next round starts at once.

**Can imprecise input, a wrong guess or an interruption be recovered from?** Mostly yes, without coaching:

- Firm 1-3 s presses on START, READY, TRY AGAIN and PLAY AGAIN act on lift.
- Double taps and mashes start one round, and presses carried from a button into play never hide.
- Slide-on hides, drift keeps the hide, and dragging off a button cancels it.
- A loss costs one short round.

The weak points:

- The forgotten resting thumb (m2).
- A mash carried from play into the loss card restarts the round almost immediately (m3).
- The wall-picture exit leaves mid-round after about 0.45 s of pressing (m4, unchanged).

## Findings (ranked)

No Blocker or Major was found.

### Minor

**m1. Pressing the dark FIND COVER box while the pumpkin is still rolling gives no feedback at all. The "<<<" cue
appears only once it has stopped.**
- **Screen**: pumpkin play.
- **Issue**: Leo's natural worried press, made just after tilting, showed "<- FIND COVER" unchanged for the whole
  2 s. The same press 1 s later, with the pumpkin still, shows the big "<<<" chevrons for about 0.9 s. To a
  non-reader the first case looks like a broken button. He was saved only because he kept holding while
  rolling into the chair.
- **Concern**: feedback / comprehension.
- **Repro**:
  - No cue: `traces/leoN.txt` (`leo.txt` up to line 79): `tilt 0 500 -866`, then at once `down 80 136`. Frames
    every 60 ms show no change.
  - Cue: `traces/leoN2.txt`: the same, with `tick 1000` before the press.
  - Zoe's still case: `zoe.txt` lines 377-389.
- **Screenshot**: `pumpkin-hold-playtest-3/03_leo_nocover_no_feedback.png` (no cue),
  `04_leo_nocover_after_stop_chevrons.png`, `frames/lN2_press_002.png`, `15_zoe_nocover_chevrons.png` (cue).

**m2. A forgotten thumb (or any finger that touched the sign) keeps the pumpkin hidden and frozen. Tilting then
"rustles" and brings a check of that spot, and only words explain why.**
- **Screen**: pumpkin play.
- **Issue**: by design the hide lasts while the finger stays down, wherever it drifts. Leo's resting thumb drifted
  off the sign and stayed on the glass. When he tilted to go, the pumpkin did not move and the bubble said
  "SHH!" and "SHE HEARD YOU!". If he doesn't lift, the red RUN! box comes and he is caught. The red box tells him
  he is in danger, but nothing in pictures says "let go of the glass". A cue on the sign (orange HIDING turning
  to a "lift your finger" picture once he tilts) or a visible shake of the hiding spot could teach it.
- **Concern**: comprehension / recovery.
- **Repro**: `traces/leoC.txt` lines 168-191: thumb `down 22 112`, slide onto the sign at 60 134, drift to 40
  108, `tilt -600 500 -624` with the thumb still down, film 11 s.
- **Screenshot**: `08_leo_r6_thumb_slide_drift.png`, `09_leo_r6_branchC_forgotten_thumb_caught.png`,
  `frames/l060_r6stuck_003.png`, `frames/l062C_keep_002.png`.

**m3. A mash carried from play through the frosting wipe lands on TRY AGAIN, so "YOU LOST" shows for under a
second before a new round starts.**
- **Screen**: pumpkin lost card.
- **Issue**: Zoe's mash (about 5 presses a second) continued through the catch. The loss card appeared and one of
  the next presses restarted the game. That is harmless, but the kid barely sees the outcome and a round starts
  that they did not choose. Ignoring presses that began before the card appeared, or a short guard, would fix it.
- **Concern**: feedback / recovery.
- **Repro**: `traces/zoe.txt` lines 291-367: a mash of 30 taps at (78-82, 132-134) starting at "THE TABLE?".
  The card is visible at `z026_mashcheck_21`, and a new round is under way at `_23`.
- **Screenshot**: `13_zoe_mash_spotcheck_and_carry.png`, `frames/z026_mashcheck_21.png`,
  `frames/z026_mashcheck_23.png`.

**m4. The wall-picture exit leaves mid-round after about 0.45 s of pressing (unchanged).**
- **Screen**: pumpkin play.
- **Issue**: a quick tap is correctly ignored. A firm press, which is Leo's normal press, leaves within about
  0.45 s and drops the round's candies. The hold cue is a small orange bar under the frame. Leo did not press it
  by accident during play in this run.
- **Concern**: recovery / consistency.
- **Repro**: `traces/zoeX.txt` lines 391-396 (`down 80 22`, frames every 100 ms); `leo.txt` lines 233-240.
- **Screenshot**: `16_zoe_exit_hold_progress.png`, `11_leo_exit_and_resume.png`.

**m5. Repetition and passive sweeps limit replay.**
- **Screen**: pumpkin play.
- **Issue**: every round starts with the pumpkin under the table, and Grandma's first visit is a sweep about 4 s
  in. The candy spots are about the same each round. Held hides always beat sweeps, so hiding is 8-20 s of
  waiting. Rushing without hiding won Zoe's R2 in 15 s.
- **Concern**: boredom / challenge / meaningful choice.
- **Repro**: the first frames of each round (`leo.txt` lines 74, 100, 127, 146, 194, 217); Zoe's rush at
  `zoe.txt` lines 259-267.
- **Screenshot**: `14_zoe_rush_win_then_rush_loss.png`, `02_leo_r1_loss.png`.

### Nit

- **n1.** During a rustle the bubble switches every 0.3-0.5 s between "SHH!", "SHE HEARD YOU!", "HMM?",
  "PHEW!", "THE PLANT?" and "RUN AWAY!". "SHE HEARD YOU!" also stays up while Grandma is off screen, and "PHEW!"
  flashes in the middle. That is hard to read even for Zoe. Concern: feedback. Repro: `leoC.txt` lines 187-191.
  Screenshot: `09_*.png`.
- **n2.** No pressed look on START, READY, TRY AGAIN or PLAY AGAIN during a 1-3 s press. They act only on lift
  (carried over from report 2, n4). Concern: feedback. Repro: `leo.txt` lines 162-166. Screenshot:
  `08_*.png` (first frame).
- **n3.** Stale state on the win card: the "?" bubble over the chair and Grandma's pointing arm stay; on another
  card the beam's pool stays. Concern: consistency. Repro: `leo.txt` lines 220-232. Screenshot:
  `frames/l070_r7door_008.png`, `frames/z020_rushdoor_001.png`.
- **n4.** On calibrate, the dark "READY!" label just above the READY button drew Leo's first press, which did
  nothing. Concern: comprehension. Repro: `leo.txt` line 50. Screenshot: `01_leo_entry.png` (third frame).
- **n5.** The pumpkin can sit right at Grandma's feet behind the light without being noticed. That is fine as a
  rule, but it looks odd. Concern: consistency. Repro: `leo.txt` line 158. Screenshot: `frames/l053_r5right_005.png`.
- **n6.** The pumpkin is slow even at a hard tilt: about 3 s from the plant to the chair at 0.8 g. That suits
  Leo, and Zoe finds it slow. Concern: pacing.

### Positives worth keeping

- **The own-spot check is now readable and escapable for a non-reader:**
  - the box turns red at the first warning frame;
  - Grandma points at the spot with a "?" bubble;
  - sweat drops appear;
  - about 4.3 s to escape, and the hiding spot stops counting as cover.
- **Hold handling:** hold to hide, slide-on hides, drift keeps the hide, and holding the dark box while rolling
  hides on arrival.
- **Robust input:** mashing and double taps never start two rounds or hide with a carried press. Drag-off
  cancels.
- **Cheating doesn't pay:** mashing beats neither a check nor movement.
- **Gentle failure:** short, funny losses with an instant retry.

## Answers to the scoped questions

- **Does Leo understand, without reading, when his spot is unsafe, and can he get away?** Yes. The red box, the
  pointing arm and the "?" bubble came on the first frame of the check, and he left in about 0.4 s and escaped
  (R5). If he keeps holding (branch A), he is caught about 4.3 s after the first warning.
- **Does pressing the sign with no cover clearly say "not here, go that way"?** Yes when the pumpkin is still
  ("<<<" chevrons for about 0.9 s). No when it is still rolling: nothing changes at all (m1).
- **Holds, slides, double taps, resting thumb?** Holds, slides, drift and double taps work as a kid expects. A
  thumb resting on the rug before touching the sign is ignored. A thumb left down after hiding traps the
  pumpkin (m2).
- **Can Zoe beat Grandma by mashing?** No. A mash at cover counts as a choppy hold: it survives sweeps, but the
  pumpkin can't move while mashing, and it loses to a check of that spot.
- **Do they collect all the candy and escape? Frustration, boredom, play again?**
  - Leo: 7 of 8 rounds. He would play again; boredom comes from sameness and passive holds.
  - Zoe: 4 of 6. She would play again another day but wants variety.

## Gesture and interruption matrix

| Gesture / case | Persona (round) | Result |
| --- | --- | --- |
| Firm press on START / READY / TRY AGAIN / PLAY AGAIN, 0.9-3 s | Leo all | acts on lift; no pressed look (n2) |
| Press on the "READY!" label above the button | Leo | ignored (n4) |
| Mash READY / double-tap or mash PLAY AGAIN | Zoe, Leo R3 | one round; no hide carried into play |
| Drag off PLAY AGAIN before lifting | Zoe | cancelled |
| Tap 2 px above PLAY AGAIN | Zoe | counts (padding) |
| Hold the box at cover through sweeps, 3-20 s | Leo R1-R8, Zoe | hidden, never caught by a sweep |
| Own-spot check, let go and roll away | Leo R5 | escaped with about 1.5 s to spare |
| Own-spot check, keep holding | Leo branch A | caught about 4.3 s after the first warning |
| Own-spot check, mash | Zoe R4 | box says FIND COVER, no hide, caught |
| Mash at cover in a sweep | Zoe R1 | survived (control branch B, untouched: caught) |
| Mash while tilting | Zoe R1 | pumpkin frozen, no movement |
| Press the dark box while still rolling | Leo R2 | no feedback (m1) |
| Press the dark box when still | Zoe R6, Leo N2 | "<<<" for about 0.9 s, no hide |
| Hold the dark box and roll into cover | Leo R2 | hides on arrival |
| Thumb resting on the rug, then tilting | Leo R6 | ignored |
| Thumb slides from the rug onto the box | Leo R6 | hides at once |
| Thumb drifts off the box while hiding | Leo R6 | stays hidden |
| Forgotten thumb + tilt | Leo R6 main / C | pumpkin frozen, "SHE HEARD YOU!"; lift = recovered; keep = caught (m2) |
| Wiggle while hidden, 0.6 g | Zoe R4 | rustle, check of that spot |
| Mash carried into the loss card | Zoe R4 | card skipped in under 1 s (m3) |
| Press the pumpkin sprite / title ghost | Leo | ignored |
| Exit picture: quick tap / firm press | Leo, Zoe | ignored / leaves after about 0.45 s (m4) |
| Leave mid-round and reopen | Leo | title, calibrate, fresh round |
| Tilt over-correction to 0.8 g | Zoe | speed capped, no overshoot |
| Ghost in the window during play | supplementary, after reading report 2 | "WHAT WAS THAT?", Grandma investigates a crash by the plant and leaves (as in report 2) |
| Board laid flat; changing grip between rounds | - | **gap** |
| Hold the box and touch the ghost at once (multi-touch) | - | **gap**: single-touch sim |
| Process restart mid-round; session rest / daily cap mid-round; night tint | - | **gap** |
| Physical board, real children | - | **gap**: emulator roleplay only |

## Supplementary QA (separate from the black-box pass)

- `make -C apps/porthole playtest` at `3338eec`: **95 of 95 scenarios pass, 0 UI FAIL, 0 UI WARN.** That includes
  `70_pumpkin_win` through `79_pumpkin_own_spot` (`74_pumpkin_monkey` under ASan/UBSan). I read
  `build/playtest/report.md` in full; its UI findings table is empty. I did not open the scenario files.
- I added no new scenario. m1-m3 need the owner's chosen behavior first. Each trace above is a ready repro.
- After reading report 2, I ran one ghost probe (`traces/supp_ghost.txt`), with the same result as that report.

## Comparison with report 2 (`pumpkin-hold-playtest-2.md`, `dd7edef`)

- **Fixed:**
  - Old M1, the own-spot check. The box now turns red at the first warning frame, not about 0.7 s before the
    catch. Grandma points with a "?", sweat drops appear, the spot stops counting as cover, and a mash shows
    FIND COVER, not "QUICK! HIDE!". There are about 4.3 s to escape. A non-reader escaped naturally.
  - Old m2, the silent dark box, is fixed when the pumpkin is still (the "<<<" chevrons). It is still silent
    while rolling (m1 here).
- **Newly observed:**
  - The rustle (not reached in report 2) shows the forgotten-thumb trap (m2) and the flickering bubble (n1).
  - A mash carried into the loss card (m3).
  - Rushing without hiding can win.
- **Unchanged:** the 0.45 s mid-round exit (m4), the sameness (m5), no pressed look on buttons (n2), the stale
  bubble or win-card leftovers (n3), and the slow pumpkin (n6).
- **Outcomes:**
  - Leo went 4-2 in report 2 and 7-1 here. He met an own-spot check naturally and escaped it.
  - Zoe went 3-3 in report 2 and 4-2 here; both losses were deliberate experiments or rushes.

## Iteration history and verdict

- Iteration 1 (`088bf6c`): `pumpkin-hold-playtest.md`, needs fixes.
- Iteration 2 (`dd7edef`, snap `07f2cc98...`): `pumpkin-hold-playtest-2.md`, needs fixes. Its Major was the
  own-spot check.
- Iteration 3 (this run, `3338eec`, snap `00369100...`): that Major is fixed and verified. Leo met it naturally,
  understood it from pictures and escaped. No Blocker or Major remains.
- **Verdict: acceptable**, with the Minor findings below for the coordinator to fix or explicitly defer.
  - m1 and m2 are the most valuable for the 6-year-old.
  - m3 is a small input guard.
  - m4 and m5 carry over and need an explicit decision. m5 is the main limit on how long either kid keeps
    playing.

  Both personas can understand, control and enjoy the core loop: roll for candy, hold to hide, flee when she
  names your spot, roll out of the lit door.

**Suggested brief improvements:**
- Ask the tester to press the dark box both while rolling and when still, because the cue differs.
- Ask the tester to probe a forgotten finger after hiding, which is how the rustle shows up for a 6-year-old.
- Ask the tester to probe mashing through a loss into the result card.
- Ask for one "rush, never hide" round, to measure how much hiding matters.

The black-box pass and its findings are emulator roleplay. They do not stand in for real children or the
physical board.
