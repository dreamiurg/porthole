# Pumpkin Hide & Seek, hold-to-hide retest 4: child-perspective black-box playtest (Leo, 6)

This is independent emulator roleplay. **It is not a study with real children, and it was not run on a physical
device.** Every observation comes from the headless host simulator (`snap --serve`) and its native 480x480
frames.

## Builds and setup

The repository moved while I was playing, so there are two passes on two builds.

| Pass | Revision | `snap` sha256 | Rounds |
| --- | --- | --- | --- |
| A, source-free first play | `830752d` (clean tree) | `3b257f30a55f39a14c6826d3735d249e28ca93f285d096d1988c02600f3f7aa4` | R1-R8 |
| B, final build (retest) | `747deaf` (clean tree, HEAD at handoff) | `b0eb264777d86a79cdeea78e5948af72ff558d07790a97989c768f9d1853a191` | F1-F4 |

`747deaf` landed during pass A ("fix(porthole): Pumpkin's slide-on in the open always says no while coasting"). I
found out from `make playtest`, whose report named the new HEAD. Pass B replays the affected flow on the final
build: slide-ons and presses in the open while coasting, checks of Leo's own spot, worried re-presses, and a
natural win. `snap-asan` (automated suite only): `c47ba95d86d2cb61b52a300c900a986a600a6a30291c31e936f155dfde32dbfa`.

Setup, the same for both passes:

- `make -C apps/porthole snap`, then a private copy of the binary in its own scratch subfolder.
- `snap --serve --fresh` with the default clock (Tuesday 16:00).
- The profile was made through the screens: name "Leo", the frog face, age 6, "skip" for the code.
- "Spooky" was opened from the launcher's jack-o'-lantern tile, with a 1.5 s press.

Input was only the serve protocol's `down` / `move` / `up` / `tick` / `tilt` / `frame`. No `dbg`, `app`, `screen`,
`debug`, clock skip or other hidden-state query was used. Leo held the board at gravity (20, 350) mg and steered
with x from about -700 to +700. The traces log one command per line with the game time:

- `pumpkin-hold-playtest-4/trace_830752d.txt` (pass A)
- `pumpkin-hold-playtest-4/trace_final_747deaf.txt` (pass B)

In the traces:

- `press X Y MS` is `down`, `tick MS`, `up`.
- `slide` is `down`, the moves, then `up`.
- `shot NAME` grabs a frame.

The driver and helpers are saved as text:

- `driver.txt`, run with `python3 -I driver.txt serve|send ...`.
- `sheet.txt` builds the contact strips.
- `waitred.txt` steps 250 ms at a time and stops when the sign turns red. This stands in for Leo noticing the
  red sign; it reads only the rendered frame.
- `slidedark.txt` steps 40 ms at a time until the sign turns dark, then slides a resting thumb onto it.

The traces were not replayed in a second fresh process, so I make no byte-identical replay claim.

### Disclosure

Before play I read `apps/porthole/docs/playtesting.md`, the `playtest` and `tour` skills, and lines 60-100 of
`apps/porthole/tools/webemu.py` (the serve protocol only). Several things reached me without being chosen:

- **Auto-loaded briefs.** The root `CLAUDE.md` loaded ("grab three candies and escape while Grandma searches
  with a flashlight"). So did `apps/porthole/CLAUDE.md`. Its runtime section says Pumpkin's ghost "acts on the
  lift of any press". It also says the action sign "hides the pumpkin exactly while a finger that touched it
  (pressed there, or slid on where it can hide at once) stays down, wherever it drifts". So I knew in advance
  that hiding is hold-based.
- **Another agent's file names.** The shared session scratch folder already held another agent's files. I saw
  their names in a directory listing (for example `hold-find-cover.png` and `rt_77_pumpkin_rustle.log`), then
  moved to a fresh subfolder. I opened none of them.
- **Pass B was not first play.** Before it I had seen three things:
  - the titles of the two newest commits (`858071d` "Pumpkin's named spot is not cover, and the sign says no
    while coasting", and `747deaf`);
  - the automated suite's scenario names (`78_pumpkin_hold`, `79_pumpkin_own_spot`);
  - `pumpkin-hold-playtest-3.md`.

  Pass B is therefore a retest by an informed tester. Pass A is the only first-play evidence.

I never read game, os, shell, host or tools source, `tests/playtests/` or `docs/superpowers/`.

## Persona and brief

**Leo, 6.** He can't read and tilts with both hands. He presses firmly for 1-3 s, double-taps and re-presses
when worried, and sometimes rests or slides a thumb. This is a simulated perspective only.

Dispatch brief, verbatim:

> You are a fresh, independent child-perspective playtester for Pumpkin Hide & Seek on Porthole: a round
> 480x480 touchscreen with no buttons, emulated on the host. Follow apps/porthole/docs/playtesting.md and the
> `playtest` skill. This is a focused retest of the final build after a small rule change, played SOURCE-FREE.
> Until you finish, don't read source (apps/porthole/games/, os/, shell/, host/), tests/playtests scripts,
> docs/superpowers/, AGENTS.md files or earlier playtest reports. Disclose anything that loaded automatically.
> Decide from the screen. Run the sim from your own copy of the snap binary in a private scratch subfolder.
>
> Persona: Leo, 6. He can't read and tilts with both hands. He presses firmly for 1-3 s, double-taps and
> re-presses when worried, and sometimes rests or slides a thumb. Simulated perspective only.
>
> Setup:
> - Run `make -C apps/porthole snap`. Record `git rev-parse --short HEAD` and the `sha256sum` of the snap.
> - Run `snap --serve --fresh` with the default clock. Create a profile (age 6, no code) through the screens,
>   then open "Spooky" from the launcher.
> - Use ordinary touches and tilt inputs only: no dbg, shortcuts or clock skips.
>
> Scope: play at least six rounds. Keep playing until Grandma names or checks a spot Leo is hiding in or near
> at least twice. Find out:
> - When she names his spot, does he understand without reading that he must leave? Does a worried re-press
>   near that spot get in his way or confuse him?
> - Does pressing the dark sign, both while rolling and while stopped, clearly say "not here, go that way"?
> - Do holds and slides still hide where they should?
> - How many rounds does he win and lose, where does he get frustrated, and would he play again?
>
> Output: write apps/porthole/docs/pumpkin-hold-playtest-4.md, with screenshots and the trace in
> apps/porthole/docs/pumpkin-hold-playtest-4/. Don't save .py files; save scripts as .txt. Include:
> - the revision and hash
> - the persona
> - this brief
> - win/loss
> - ranked findings with repro steps
> - your assessment
> - gaps
>
> Afterwards, you may compare with apps/porthole/docs/pumpkin-hold-playtest-3.md. Don't fix code or commit.
> Reply ONLY with: verdict (acceptable / needs fixes), win/loss, top findings and the report path, in under 8
> lines.

The brief was not revised. One suggestion is under "Suggested brief changes" below.

## Outcomes

| Round | Build | Result | Time | What decided it |
| --- | --- | --- | --- | --- |
| R1 | A | **Win** | 15 s | Pressed the dark sign while rolling, hid at the plant, escaped. |
| R2 | A | **Win** | 52 s | "THE TABLE?" was named while he rolled away from it; the press showed "<<<" and he hid at the chair. |
| R3 | A | **Loss** | 17 s | Hid at the table; "THE TABLE?" was named; he kept holding and was caught after about 5 s. |
| R4 | A | **Loss** | 37 s | Hid at the plant; "THE PLANT?" was named; held and caught. Tester-paced: one 6 s step without looking, the same as freezing. |
| R5 | A | **Win** | 31 s | Long hides (about 25 s on one hold), then a dash. |
| R6 | A | **Win** | 74 s | Two checks of his own spot (table, plant). Each time he lifted, re-pressed ("<<<") and fled. |
| R7 | A | **Win** | 70 s | Two own-spot checks. Tilting while holding gave "SHH!" / "SHE HEARD YOU!"; he lifted and fled. |
| R8 | A | **Loss** | 6 s | After resuming, he rushed right without hiding and was caught at Grandma's feet. |
| F1 | B | **Loss** | about 40 s | "THE TABLE?" was named while he hid there; two worried re-presses (about 2.5 s), then he fled and was caught in the beam ("PEEKABOO!"). |
| F2 | B | **Loss** | about 45 s | Probe: slid onto the sign beside Grandma as "QUICK! HIDE!" appeared; "FOUND YOU!" on the next frame. |
| F3 | B | **Loss** | about 35 s | Held press in the open, under-tilted, stopped just short of the chair. Then "THE CHAIR?" was named and he was caught. |
| F4 | B | **Win** | about 30 s | Natural: candy, hid at the plant, raced past the named chair and out of the door. |

**Totals: 6 wins and 6 losses in 12 rounds.** Pass A was 5 wins and 3 losses; pass B (final build) was 1 win
and 3 losses. One more round (R9) was left by the exit sign for coverage and is not counted. Each win collected
all three candies.

**Grandma named Leo's spot or a spot beside him 12 times. The requirement was two.**

Named while he was hiding there (8):

| Round | Spot | Leo's reaction | Result |
| --- | --- | --- | --- |
| R3 | table | kept holding | caught |
| R4 | plant | kept holding | caught |
| R6 | table | lift, then two re-presses | escaped |
| R6 | plant | thumb slide, lift, re-press | escaped |
| R7 | table | tilted while holding, then lift | escaped |
| R7 | plant | lift, then pressed the pumpkin and the plant in the room | escaped |
| F1 | table | lift, then two re-presses | caught |
| F4 (B) | plant | quick lift, one re-press | escaped |

The F4 check came after a tester hold that ran into F4's natural part. It is shown in strip F5.

Named while he was next to it (4): R2 table, R7 plant, F3 chair, F4 chair. He escaped three times and was caught
once (F3).

## What Leo saw and did

1. **Entry (`01_entry.png`, `F1_final_entry_and_slide_on_at_cover.png`).**
   - His 1.5 s press on the jack-o'-lantern tile showed a yellow "Spooky" caption and opened the game on lift.
   - The title has one big cream box, which he pressed. The calibrate screen shows hands holding a board, and he
     pressed READY.
   - In the room the pumpkin sits under the table. Three candy corns lie on the rug, and an empty three-slot
     shelf hangs on the wall. The cream sign says "HOLD TO HIDE", which he can't read.
2. **The sign is his whole vocabulary.**
   - Cream means he can hide here.
   - Dark slate with a small arrow means no cover here.
   - Orange "HIDING" means hidden; the pumpkin tucks behind the chair, table or plant pot.
   - Red "RUN!" means danger.

   He learned "press and hold the box when Grandma comes" within round 1. The sign changes colour on every
   state change, so it reads well without words.
3. **Pressing the dark sign.**
   - **While rolling:** big "<<<" chevrons pointing to the nearest cover (`03_*`, `20_a`; final build
     `F4_*`). This fixes report 3's m1.
   - **While stopped:** "<<<" from about 0.1 s to 0.8 s, then back to the small "<- FIND COVER" while his finger
     is still down (`09_press_dark_sign_when_stopped_timing.png`).

   The chevrons clearly say "that way". They do not stay up for his normal 1-3 s press (m1).
4. **Grandma names his spot (`04_*`, `05_*`, `06_*`, `07_*`, `08_*`, `F2_*`, `F5_*`).**
   - She stands at the right, points her arm and gets a "?" bubble over the spot.
   - On that frame his orange HIDING sign turns red ("RUN!"), and blue sweat drops appear beside the hidden
     pumpkin.
   - About 3.5 s later the bubble says "RUN AWAY!" and the beam comes. About 1 s after that he is caught ("FOUND
     YOU!", "THERE YOU ARE!", "PEEKABOO!"), followed by the orange frosting wipe and the pie card.

   **While his finger stays down nothing points him anywhere: no arrow on the red sign, and the pumpkin stays
   tucked in.** The "go that way" arrow appears only after he lifts. In R3, the first time, he did what he had
   learned: hold harder. He lost without knowing why (M1).
5. **Worried re-press (the rule change).**
   - After a lift at a named spot, the pumpkin pops out and the sign goes dark with an arrow away from the named
     spot.
   - A re-press there **does not hide him again**. It shows "<<<" toward the next spot (`56_c_repress`,
     `f09_c_repress`, `f28_b_repress`), every time and on both builds.
   - A held re-press, or one made while rolling, hides him at the next cover when he arrives (`58_a`, R6).

   So the re-press never traps him in the named spot and never confuses the direction. **But each worried
   re-press costs time.** On the final build two re-presses (about 2.5 s) left about 1 s to roll. The pumpkin
   starts slowly, and he was caught in the beam just outside the table (F1, `F2_*.png`).
6. **Holds and slides.** Every case behaved as the auto-loaded description says:
   - A hold at cream hides.
   - A thumb slid from the rug onto the sign at a spot hides (`87_c`).
   - A thumb drifting off the sign keeps the hide (`64_b`).
   - A thumb slid on in the open while coasting shows chevrons and hides on arrival at the next spot (final
     build `F4_*`: ">>>" for 2 frames, then "<<<", then HIDING at the chair).
   - A finger held in the open never hides there.
   - Tilting while hidden keeps him hidden but rustles ("SHH!", "SHE HEARD YOU!", a "!" by Grandma; `08_*`). That
     is report 3's m2, unchanged.
7. **Other probes.**
   - The ghost on the wall acts on lift: "WHAT WAS THAT?" and Grandma goes to check a noise by the plant. It
     greys out after one use. Leo likes it.
   - Tapping the pumpkin, the plant or the chair in the room does nothing (n2).
   - Holding the picture frame at the top fills a small yellow bar and leaves for the launcher at about 1 s
     (`12b_exit_hold_bar_crop.png`). The held finger opened nothing on the launcher.
   - Reopening Spooky gives the title, calibrate and a fresh round. No round state is kept.
   - A worried double tap on PLAY AGAIN / TRY AGAIN started one round and did not hide the pumpkin.
8. **Where Leo would stop.**
   - **Pass A:** after R6 (about 4.5 min in the game). It was a big "FREEDOM! AND ALL THE CANDY" win after two
     scary checks. R7 then ran 70 s, with Grandma returning every few seconds and a 20 s hold at the chair. That
     is where he would get bored (`14_r7_grind_repeat_visits.png`).
   - **Pass B:** after F3 (three losses in a row), out of frustration. F4 was played for coverage.

## Qualitative assessment

- **Can he recognise the goal and actions and tell what changed?** Mostly yes.
  - Candy corns on the rug fly up to the shelf slots with a sparkle.
  - The door glows with an arrow once the shelf is full.
  - Grandma's arrival, her beam, the pointing "?" bubble and the four sign colours are all pictures.
  - The win card shows the pumpkin with its candy; the loss card shows a pie.

  Reading is not needed to play the basic loop. It is needed to know why he lost and what "RUN!" asks him to do
  (M1).
- **Interesting decisions and fair challenge?** The real choice is when to dash and when to hide, plus using the
  ghost. Holding beats sweeps reliably, and the named-spot check is the one real threat. It is fair once he
  knows "let go and roll away". Learning it took him two losses (R3, R4) and an accidental lift during a
  worried re-press.
- **Where it gets repetitive or frustrating:**
  - Long holds of 15-25 s while Grandma sweeps.
  - Grandma coming back 2-3 s after "PHEW!" (`F8_final_grandma_returns_quickly.png`: "PHEW!" at 113.3 s,
    "GRANDMA!" at 115.8 s).
  - The third candy sitting right of the plant pot, where Grandma stands.
  - On the final build, three losses in a row. Two of them came from the persona's own habits (worried
    re-presses; a finger held while under-tilting), not from careless play.
- **Recovery and interruption.** Losses are gentle and one press away from a retry. Exit is a deliberate hold
  with a visible bar. A finger carried into a new screen never activated anything. Nothing is lost but the
  current round.
- **Would he play again?** Yes, on a good day: the hiding, the ghost and the pie are fun. After two or three
  unexplained losses he would ask a grown-up what "RUN" means.

## Findings (ranked)

### Major

**M1. When Grandma names the spot Leo is hiding in, nothing that a non-reader can see says "let go" or "which
way" while his finger is down. Worried re-presses and tilting also eat the roughly 4 s window.**
- **Screen**: pumpkin play.
- **Issue**:
  - The orange sign turns red "RUN!", and that is text only. The pumpkin stays tucked in.
  - The arrow (and "<<<" on a re-press) appears only after he lifts.
  - Leo's learned rule is "hold = safe", so his first reaction is to hold harder. If he tilts while holding, he
    rustles and Grandma hears him.
  - From the red sign to the catch is about 4.3-5 s. The pumpkin needs about 1 s to get rolling.

  Of 8 checks of his own spot, 3 were losses: R3 and R4 kept holding, and F1 used two worried re-presses. The
  loss card does not show why.
- **Concern**: comprehension / recovery / feedback.
- **Suggestion**: put the direction on the red sign (for example "<<<" beside or instead of "RUN!", pointing to
  the next cover). Also consider one of these:
  - pop the pumpkin out (or show a "lift finger" picture) shortly after the spot is named, since the spot is no
    longer cover;
  - let a re-press on a named spot start a nudge in the arrow's direction.
- **Repro**:
  - R3: `trace_830752d.txt` lines 330-370 (hide at the table with `down 80 131`, keep holding; red at `34_a`,
    line 362).
  - Final build F1: `trace_final_747deaf.txt` lines 270-307 (red at `f08r_21`; `up`, `down 81 132` for 0.75 s,
    `up`, `down 80 130` for 1.2 s, then `tilt -550 350`; caught at `f10_d`, line 307).
- **Screenshot**: `pumpkin-hold-playtest-4/04_r3_named_while_holding_caught.png`,
  `F2_final_named_table_two_worried_represses_caught.png`, `05_r4_named_while_holding_caught.png`,
  `frames/34_a.png`.

### Minor

**m1. The "<<<" cue lasts about 0.8 s, shorter than Leo's normal press. If he stops short of cover with his
finger down, the pumpkin sits in the open while he believes he is hiding.**
- **Screen**: pumpkin play.
- **Issue**:
  - A press on the dark sign shows chevrons from about 0.1 to 0.8 s, then the small "<- FIND COVER" text while
    the finger stays down.
  - In F3 he pressed while rolling, under-tilted and stopped just short of the chair. He kept holding, thinking
    he was hidden.
  - Grandma then named the chair. The arrow flipped to point right, toward the table and Grandma, and he was
    caught (`F6_*`). The same stall without a catch: `57_c`/`57_d`.
  - Keeping the chevrons (or a pulsing arrow) for as long as a finger is down and not hidden would make the
    state obvious.
- **Concern**: feedback / comprehension.
- **Repro**:
  - `trace_830752d.txt` lines 1218-1236 (`down 80 131` on the dark sign with the pumpkin still, frames at
    100-1200 ms).
  - `trace_final_747deaf.txt` lines 1199-1276 (`tilt -600 350`, `down 80 131`, `tilt 20 350` after 0.3 s; the
    pumpkin stops short of the chair; "THE CHAIR?" at `f29r_14`; caught at `f30_b`).
- **Screenshot**: `09_press_dark_sign_when_stopped_timing.png`,
  `F6_final_held_finger_stopped_short_of_chair_caught.png`, `13_held_finger_off_cover_and_slide_on.png`.

**m2. Pacing: Grandma returns 2-3 s after leaving, so later rounds become 60-100 s of holding.**
- **Screen**: pumpkin play.
- **Issue**:
  - Crossing the room takes about 3 s at a strong tilt. "PHEW!" / "HIDDEN!" is often followed by "GRANDMA!" in
    2.5-4 s.
  - R6 and R7 took 70-74 s, with single holds of 15-25 s.
  - The third candy often sits right of the plant pot, at Grandma's standing spot. Leo under-shot it three
    times; hiding at the plant does not collect it.

  This is where he would get bored, and it pushes him into risky dashes.
- **Concern**: pacing / boredom.
- **Repro**:
  - `trace_final_747deaf.txt` lines 955-985 (hold at the chair; "PHEW!" at `f23r_30`, "GRANDMA!" at `f23r_40`).
  - `trace_830752d.txt` lines 1216-1653 (R7).
- **Screenshot**: `F8_final_grandma_returns_quickly.png`, `14_r7_grind_repeat_visits.png`.

**m3. A forgotten or worried hold plus tilt still rustles, and only words explain why (report 3's m2,
unchanged).**
- **Screen**: pumpkin play.
- **Issue**: after "THE TABLE?" Leo tilted to flee with his finger still down. The pumpkin stayed hidden and the
  bubble said "SHH!" and then "SHE HEARD YOU!". He escaped only because he lifted next. This is part of M1's
  window problem.
- **Concern**: comprehension / recovery.
- **Repro**: `trace_830752d.txt` lines 1285-1300 (`tilt -500 350` while held after `68_16`).
- **Screenshot**: `08_r7_named_table_tilt_while_holding_rustle.png`.

### Nit

- **n1.** The arrow points to the nearest unnamed cover. It can point back the way the pumpkin is coasting, then
  flip:
  - ">>>" for 2 frames (about 80 ms), then "<<<" (`F4_*`, lines 841-845);
  - "<- FIND COVER" / "FIND COVER ->" swaps (`22_c`/`22_d`).

  After the chair was named, the arrow pointed toward Grandma's side (F3). Concern: consistency. Screenshot:
  `F4_final_slide_on_while_coasting_in_open.png`.
- **n2.** Tapping the pumpkin, the plant or the chair in the room gives no response. A kid's first idea, "tap
  where to hide", gets nothing. Concern: feedback. Repro: `trace_830752d.txt` lines 1586-1596. Screenshot:
  `10_r7_named_plant_scene_taps_then_escape.png`.
- **n3.** Caught at Grandma's feet while her beam points the other way (R8, `86_g`). In R1 (`12_signpress_d`) the
  same spot was safe. To Leo, "she wasn't looking at me". Concern: consistency. Screenshot:
  `11_r8_caught_at_her_feet_beam_away.png`.
- **n4.** The loss card ("YOU LOST", a pie, a funny line) never shows what caught him, so a non-reader can't
  learn from it. Concern: comprehension. Screenshot: `frames/35_b.png`.

### Positives worth keeping

- The re-press at a named spot never hides him there again, and it shows the chevrons. The named spot also
  stops counting as cover while he rolls through it (`75_c`, `f34_e`).
- "<<<" now appears while rolling (report 3's m1 is fixed).
- The sign's four colours make a vocabulary a non-reader can use.
- The sweat drops and the "?" bubble make the check feel exciting.
- The ghost decoy.
- "HIDDEN!" and "PHEW!" feedback.
- The gentle pie loss.
- The exit hold bar.
- Carried touches never act on the next screen.

## Gesture and interruption matrix

| Gesture or interruption | Where | Result |
| --- | --- | --- |
| Firm press of 1.2-2.5 s on START / READY / TRY AGAIN / PLAY AGAIN | everywhere | acts on lift |
| Double tap on PLAY AGAIN / TRY AGAIN | R2, R5 | one round; no hide |
| Hold at a cream sign | all rounds | hides |
| Press the dark sign while rolling | R1, R2, F4 | "<<<" toward cover; hides on arrival if held |
| Press the dark sign while stopped | R3, F (pass B) | "<<<" for about 0.8 s, then the text arrow; no hide (m1) |
| Thumb slid from the rug onto the sign at cover | R9, F1 | hides |
| Thumb slid onto the sign in the open while coasting (final build) | F4 | chevrons, then hides at the chair |
| Thumb drifting off the sign while hidden | R6 | stays hidden |
| Lift at a named spot | R6, R7, F1, F4 | pops out; dark sign points away |
| Re-press at a named spot (1-2 times) | R6, F1, F4 | "<<<", no hide; costs time (M1) |
| Keep holding at a named spot | R3, R4 | caught after about 4-5 s (M1) |
| Tilt while held at a named spot | R7 | rustle, "SHE HEARD YOU!" (m3) |
| Under-tilt with the finger held in the open | R6 (`57_c`), F3 | sits in the open; caught in F3 (m1) |
| Taps on the pumpkin, the plant or the room | R7 | nothing (n2) |
| Ghost on the wall | R9 | lure on lift ("WHAT WAS THAT?"); greys out |
| Quick tap on the exit picture | R9 | nothing |
| Hold on the exit picture | R9, F end | bar, launcher at about 1 s; the lift opened nothing |
| Leave mid-round and reopen | R8/R9 | title, calibrate, fresh round (no state) |
| A finger held from play into the frosting and loss card | R3, F2 | ignored; TRY AGAIN needs a new press |
| Process restart | not tested | gap (the game keeps no state) |
| Rest or daily cap screen during play | not tested | gap |
| Big tilt overshoot, or the board flat/upside down | not tested | gap |

## Supplementary QA (separate from the black-box pass)

`make -C apps/porthole playtest` ran at `747deaf`: 95 of 95 scenarios pass with 0 UI FAIL and 0 UI WARN. That
includes `70_pumpkin_win`, `71_pumpkin_loss`, `72_pumpkin_tilt`, `73_pumpkin_kid_taps`, `74_pumpkin_monkey`,
`75_pumpkin_navigation`, `76_pumpkin_ghost`, `77_pumpkin_rustle`, `78_pumpkin_hold` and `79_pumpkin_own_spot`. I
read `build/playtest/report.md` in full; its UI findings table is empty. I added no scenario. M1 and m1 need the
owner's chosen behavior first, and the traces above are the repros.

## Comparison with report 3 (`pumpkin-hold-playtest-3.md`, `3338eec`)

- **Fixed:** report 3's m1. Pressing the dark sign while rolling now shows "<<<" at once.
- **New and working:** a named spot is no longer cover, so a re-press there shows chevrons instead of hiding
  again.
- **Unchanged:**
  - The text-only red "RUN!" while held. Report 3 accepted it because Leo escaped by lifting quickly. This run,
    with a worried re-presser, lost three of eight own-spot checks, so I rate it Major here (M1).
  - The rustle trap (m3, report 3's m2).
  - The sameness and long holds (m2, report 3's m5).
  - Scene taps doing nothing (n2).
  - Odd catches at Grandma's feet (n3; report 3's n5 saw the opposite case).

## Suggested brief changes

- Ask the tester to poll frames at 250 ms or less whenever Grandma is on screen. My R4 loss came from a 6 s step
  without looking, which simulates a freeze rather than a choice.
- Name the "under-tilt with a held finger" case explicitly. It is a natural 6-year-old pattern and it found m1.

## Gaps

- Emulator roleplay only: no real child and no physical device, tilt sensor or touch panel.
- Pass B is an informed retest (see the disclosure); first-time comprehension was judged on `830752d` only. The
  `747deaf` change (slide-on while coasting) was checked in pass B (`F4_*`).
- One persona only. There was no fluent reader and no masher this time.
- Not tested: process restart, the rest and daily-cap screens interrupting play, extreme tilt (flat, upside
  down) and recalibration.
- Traces were not replayed in a fresh process.
- Win/loss counts include tester-chosen probes (F2, and R4's long step); they are labelled above.

## Verdict

**Needs fixes** (M1). The rule change itself works:

- A worried re-press at a named spot never traps Leo again and points him the right way.
- The dark sign now answers presses while rolling.
- Holds and slides hide where they should.

But for a 6-year-old non-reader the moment that matters most, "she found my spot", still asks him to read "RUN!"
and to know he must lift first. His natural reactions (hold harder, re-press, tilt while holding) each cost the
escape. Three of eight own-spot checks were losses, and the final build gave him three losses in a row. The m1
and m2 Minors should be fixed or explicitly deferred.

## Evidence index (`pumpkin-hold-playtest-4/`)

- Pass A strips: `01_entry.png` to `14_r7_grind_repeat_visits.png`, plus `12b_exit_hold_bar_crop.png`. Full
  frames are in `frames/`.
- Pass B strips: `F1_*.png` to `F8_*.png`. Full frames are in `frames_final/`.
- Traces: `trace_830752d.txt`, `trace_final_747deaf.txt`.
- Tools (text): `driver.txt`, `sheet.txt`, `waitred.txt`, `slidedark.txt`.
