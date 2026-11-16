# Pumpkin Hide & Seek, hold-to-hide retest 5: child-perspective black-box playtest (Leo, 6 and Zoe, 8)

This was emulator roleplay by an independent agent, not a study with real children or a test on a physical
device. The personas are simulated perspectives.

## Build and setup

- Revision: `15ea19f` (`git rev-parse --short HEAD`, clean tree). `make -C apps/porthole snap` was "Nothing to be
  done".
- `build/host/snap` sha256: `9147ca0c55d77211694ed202b7a0328f1c2ffeb85cc7ca0725c42c89fe7e23fa`. I played from a
  private copy with the same hash, made once at the start, in a scratch folder. `make playtest` later left the
  hash unchanged.
- Process: `snap --serve --fresh` with the default clock (Tuesday 16:00). One process ran the whole session, so
  there were no clock skips, no `dbg` and no `app` shortcuts.
- Driver: `pumpkin-hold-playtest-5/driver.txt`. It is a small socket relay that forwards
  `down`/`move`/`up`/`tick`/`tilt` and saves frames. I ran it with `python3 -I driver.txt server|client`.
  Contact sheets came from `sheet.txt`.
- Profiles were made through the screens:
  - "Leo": fox, age 6, code skipped.
  - "Zoe": owl, age 8, code skipped. She was added from the picker's "+ New" later in the same process.

  Both kids opened Pumpkin from the launcher tile, whose caption reads "Spooky".
- Tilt model:
  - Leo holds the board at about 30 degrees toward himself (`tilt 0 500 -866` at calibration) and steers with
    ±350 to ±600 mg.
  - Zoe holds it flatter (`0 300 -954`) and steers with ±500 to ±800 mg.
- Input trace: `pumpkin-hold-playtest-5/trace_15ea19f.txt`. Every input is stamped with sim time in ms, and `#`
  lines carry my intent. The shot names in this report match the `shot` lines in the trace.

### Disclosure

- Loaded automatically: the root `CLAUDE.md` and `apps/porthole/CLAUDE.md`, which is the app `AGENTS.md`. The app
  brief has a one-paragraph summary of the game ("three hiding spots, three candies ... escaping through the door,
  ... sweeps, turn-backs, spot checks and a frosting-to-pie retry sequence"). It also has one sentence on input:
  "Pumpkin's ghost ... acts on the lift of any press, and its action sign hides the pumpkin exactly while a
  finger that touched it (pressed there, or slid on where it can hide at once) stays down ... ignore a press that
  began before the round did". So I knew before play that the sign hides and that the ghost does something. I
  tried to play Leo as though I did not.
- Read before play: `apps/porthole/docs/playtesting.md`, the `playtest` and `tour` skills, and the serve-protocol
  lines of `apps/porthole/tools/webemu.py` (lines 56-100: `tick`/`frame`/`down`/`move`/`up`/`tilt`).
- Not read: game, OS, shell or host source, `tests/playtests`, `docs/superpowers`, game `AGENTS.md` files, earlier
  reports and git logs.
- After play, for the comparison only: the findings and verdict of `pumpkin-hold-playtest-4.md`.

## Persona and brief (verbatim dispatch)

> You are a fresh, independent child-perspective playtester for Pumpkin Hide & Seek on Porthole, a round 480x480
> touchscreen with no buttons, emulated on the host. Follow apps/porthole/docs/playtesting.md and the `playtest`
> skill.
>
> This is a retest of the final build after a rule change, so play SOURCE-FREE. Until you finish, don't read
> source (apps/porthole/games/, os/, shell/, host/), tests/playtests scripts, docs/superpowers/, AGENTS.md files,
> earlier playtest reports or git logs. Disclose anything that loaded automatically. Decide from the screen. Run
> the sim from your own copy of the snap binary, in a private scratch subfolder, built once at the start; the build
> won't change while you play.
>
> ## Personas (two passes)
> A. Leo, 6. Can't read, tilts with both hands, presses firmly for 1-3 s, double-taps and re-presses when worried,
> sometimes rests or slides a thumb.
> B. Zoe, 8. A confident reader, taps quickly and works out the rules.
> Both are simulated perspectives only.
>
> ## Setup
> - Run `make -C apps/porthole snap`. Record `git rev-parse --short HEAD` and the `sha256sum` of the snap.
> - Run `snap --serve --fresh` with the default clock. Create the profile (age 6 or 8, no code) through the screens
> and open "Spooky" from the launcher.
> - Use ordinary touches and tilt inputs only: no dbg, shortcuts or clock skips.
>
> ## Scope
> Play at least eight rounds as Leo and five as Zoe. Keep playing until Grandma names a spot Leo is hiding in at
> least three times.
> - When she names his spot, does he notice, without reading, that he has to move? Does he get away?
> - Does a held finger behave the way he expects?
> - Does pressing the sign with no cover say "not here, go that way" for as long as he presses?
> - Is the game fair and fun for each kid? How long are the rounds? Where do they get bored or frustrated? Would
> they play again?
> - Record win/loss per persona.
>
> ## Output
> Write apps/porthole/docs/pumpkin-hold-playtest-5.md, with screenshots and the trace in
> apps/porthole/docs/pumpkin-hold-playtest-5/. Don't save .py files; save scripts as .txt. Include the revision and
> hash, the personas, this brief, win/loss, ranked findings with repro steps, your assessment and the gaps.
>
> Afterwards you may compare with apps/porthole/docs/pumpkin-hold-playtest-4.md. Don't fix code or commit. Reply
> ONLY with: a verdict (acceptable / needs fixes), win/loss per persona, the top findings and the report path, in
> under 8 lines.

The brief was not revised during this pass.

## Outcomes

The sim starts when the round's READY or PLAY AGAIN is lifted and stops on the end card. Round lengths are
approximate, at about 0.4 s resolution.

| Round | Result | Length | Notes |
| --- | --- | --- | --- |
| L1 | Win | ~28 s | First play. Pressed the only button-like thing, the sign, and saw the pumpkin tuck under the chair. Got all three candies, then the door. |
| L2 | Win | ~48 s | Probe: camped under the table for ~33 s (more than 1-3 s). Grandma named the chair, not the table. Later named the table while he was rolling past it. |
| L3 | Win | ~33 s | Released mid-sweep after 2.8 s, then a worried re-press on "QUICK! HIDE!". Pressed while rolling and auto-hid at the plant. |
| L4 | Win | ~17 s | |
| L5 | **Loss** | ~17 s | **E1**: the chair was named while he held (thumb resting). He reacted about 2.6 s later, tilting away from the wall he had been pushing into. Caught about 0.3 s short of the table. |
| L6 | Win | ~51 s | Pressed the sign in the open and saw "<<<"; auto-hid on reaching the chair. Long holds through repeat visits. |
| L7 | Win | ~21 s | **E2**: the plant was named while he held. He tilted left with his thumb still down, rolled out and re-hid under the table automatically. Escaped. |
| L8 | Win | ~35 s | Tapped the ghost out of curiosity ("WHAT WAS THAT?": Grandma investigated and left). **E3**: the chair was named while he held, he tilted right about 3 s later and just out-rolled the beam. |
| L9 | Quit | - | Mistake probes (pressing the pumpkin and the room, slide-off, exit brush and exit hold). Left through the frame-shaped exit mid-round. |
| Z1 | Win | ~21 s | Used the ghost to clear Grandma, then ran through a sweep on the way out and was not caught. |
| Z2 | Win | ~11 s | Experiment: never hid once. Rolled straight through two beams. |
| Z3 | **Loss** | ~5 s | Deliberate experiment: stood still in the beam. "QUICK! HIDE!" then "GOTCHA DEAR!" after about 1 s. |
| Z4 | Win | ~15 s | |
| Z5 | Win | ~48 s | Waited at the chair, then the plant, for her own spot to be named. **E4**: the plant was named and she escaped with the ghost. |
| Z6 | Win | ~32 s | Extra round, bored of waiting. Grandma came back immediately after "HIDDEN!". |

- **Leo**: 7 wins, 1 loss over 8 completed rounds, plus round 9 quit for the exit probe.
- **Zoe**: 5 wins, 1 deliberate loss over 6 rounds.
- **Grandma named the spot Leo was hiding in 3 times**: E1 caught, E2 escaped, E3 escaped. She named Zoe's spot
  once (E4, escaped with the ghost).
- She also named a spot while Leo was only rolling past it, three times (L1 plant, L2 table, L3 table). Each
  time he was already leaving, and the bubble turned to "RUN AWAY!".

## What happened when Grandma named Leo's spot

| Event | Spot named → his reaction | Name → catch window | Outcome | Evidence |
| --- | --- | --- | --- | --- |
| E1 (L5) | Chair. The pumpkin popped out despite his thumb and the sign showed ">>>". His tilt was still left (into the wall by the door). He tilted right about 2.6 s later. | Named at ~166.8 s. Beam on the chair at ~170.3 s. "FOUND YOU!" at ~171 s, about 4.2 s in all. | Caught | `03_L5_E1_chair_named_while_holding_caught.png`, `frames/E1_named_chair_while_hiding.png` |
| E2 (L7) | Plant. The pumpkin popped out and the sign showed "<<<". He tilted left about 2.5 s later, thumb still down. | Beam reached the plant about 1 s after he left it. | Escaped. Re-hid under the table on arrival because his thumb was down. | `04_L7_E2_plant_named_thumb_down_tilt_escape.png`, `frames/E2_named_plant_while_hiding.png` |
| E3 (L8) | Chair. The pumpkin popped out, ">>>". He tilted right about 3 s later, thumb down. | Beam was on the chair as he started rolling, and he out-rolled it. | Escaped and re-hid under the table | `05_L8_E3_chair_named_escape.png`, `frames/E3_named_chair_while_hiding.png` |

My reaction times are 2.5-3 s because I watched in batches of 300-400 ms frames and then decided. That is a
plausible lag for a 6-year-old, but it is not measured. A quicker kid (about 1 s) would escape all three by a
wide margin.

## Answers to the brief's questions

- **Does Leo notice, without reading, that he must move?** Mostly yes. Unlike build 4, the pumpkin now **pops out
  of hiding even with his thumb down**, so it is visibly exposed. At the same moment:
  - Grandma turns and points her arm at that spot;
  - a "?" bubble sits on the furniture;
  - the sign becomes big chevrons (">>>" / "<<<") pointing to the next unnamed cover.

  These are picture cues, and together they read as "she found me, go that way". Two things are weak. The small
  "!" over the pumpkin shows for less than 0.4 s, and the pumpkin keeps its happy smile. When the chair is named,
  the only arrow points across the room toward Grandma, which a 6-year-old may not want to follow (E1).
- **Does he get away?** In 2 of 3 cases, with a 2.5-3 s reaction. The loss (E1) combined that reaction with a
  tilt jammed into the left wall, and he fell about 0.3 s short. Fair, but tight.
- **Does a held finger behave as he expects?** Yes, in almost every case:
  - Hold = hidden.
  - Pressing while rolling hides him the moment he reaches cover (`07_*`).
  - Sliding a resting thumb from the room onto the sign hides him (`08_*`).
  - A 120 ms lift and re-press keeps him hidden with no flicker (`08_*`, `069_L7*`).
  - When his spot is named, holding no longer keeps him in, and tilting with his thumb still down carries him
    to the next cover and tucks him in there (E2, E3). That is the behavior he needs.
  - A held PLAY AGAIN does not leak into the round: the round starts on the lift (`12_*`).

  The one surprise: pressing the pumpkin itself or the furniture does nothing (`09_*`), and a non-reader's
  first guess may be to press the pumpkin.
- **Does the sign say "not here, go that way" for as long as he presses?** Yes. With the pumpkin stopped in the
  open, a 3.2 s press kept "<<<" for the whole press (`06_sign_chevrons_held_3s_no_cover.png`). That is build 4's
  m1, now fixed. The direction skipped the nearest spot because Grandma had just named it (plant), which was
  sensible. While rolling with the finger down, the chevrons hand off to "HIDING" on arrival (`07_*`).
- **Fair and fun? Round length? Boredom? Play again?** See the assessment below.

## Qualitative assessment

**Can each kid recognize the goal and actions, and tell what changed?**
- Leo:
  - The title shows a pumpkin, three candies and a big cream button, and that is enough to start.
  - The candy shelf at the top fills one candy at a time with a sparkle, so he can see progress without words.
  - The door lights up with a big arrow once all three are collected, so he knows where to go without reading.
  - The sign turns bright yellow and the pumpkin tucks under the furniture when he hides, which is a clear
    change.
  - "HOLD IT COMFY AND STILL" on the calibrate screen is text only, but a non-reader just presses READY and it
    works.
- Zoe reads every bubble ("GRANDMA!", "HMM?", "QUICK! HIDE!", "THE TABLE?", "RUN AWAY!", "TO THE DOOR!") and had
  the rules in one round.

**Decisions, skill, consequences, feedback.**
- Real choices: which candy first, when to hide versus dash, and when to spend the one ghost per round.
- The beam gives about 1 s of grace ("QUICK! HIDE!" then "GOTCHA DEAR!", `14_*`). That forgives Leo, but it also
  lets Zoe sprint through sweeps without hiding (Z2 was won without one hide, `13_*`).
- The ghost removes Grandma's whole visit, even a check of Zoe's own spot (`10_*`, `15_*`).
- Losses are gentle and funny: an orange frosting wipe, then a pumpkin pie and "YOU LOST / PIE TIME!".
  "YOU LOST" is blunt, but the pie softens it and TRY AGAIN starts a new round with one tap. The loss card does
  not show what went wrong.

**Repetition, waiting, reading burden, stopping points.**
- It is the same room, the same three spots and nearly the same three candy places every round. The variety
  comes from Grandma's moves and the ghost.
- Grandma often returns 0-3 s after "PHEW!"/"HIDDEN!" (`16_*`: L3 about 1.5 s, Z6 immediately). A kid who
  waits for safety ends up holding through visit after visit, and those rounds reach 50 s. A kid who moves
  between visits finishes in 15-35 s.
- **Leo** would likely stop voluntarily after about 5-6 rounds (4-5 minutes). The early wins feel great, the pie
  loss gives him one "again!", and then the room offers nothing new. He would come back another day.
- **Zoe** worked it out by round 2: the ghost clears a visit, a moving pumpkin survives the beam, and
  standing still is fatal. She would stop at about round 4 (3 minutes), saying it is easy. She might return
  briefly but wants something harder. There is no escalation, score, best time or more candy.
- Natural stopping point: every win card is a clean stop. The frame-shaped exit at the top leaves straight to
  the launcher.

**Recovery from imprecise input, wrong guesses and interruptions.** Good. Details are in the matrix below.
Nothing was lost by leaving, but the game keeps no state, so re-entry starts from the title and asks for
calibration again.

## Findings (ranked)

No Blocker or Major.

### Minor

**m1. Grandma comes back almost at once after leaving, so waiting kids hold through visit after visit (build 4's
m2, unchanged).**
- **Screen**: pumpkin play.
- **Issue**:
  - "PHEW!"/"HIDDEN!" is followed by "GRANDMA!" after 0-3 s.
  - In Z6 she was pointing at the table on the very frame Zoe let go.
  - Rounds spent waiting for a safe gap reach about 50 s (L2, L6, Z5), with 10-30 s of holding.
  - Kids learn that waiting does not help and start dashing.
- **Concern**: pacing / boredom.
- **Repro**: `trace_15ea19f.txt` around shots `040_L3_h_2`..`040_L3_r_6` (PHEW at about 109.5 s, GRANDMA at
  about 111.2 s) and `137_Z6a_2`..`137_Z6b_04` (HIDDEN!, `up`, "THE TABLE?" within 0.4 s).
- **Screenshot**: `pumpkin-hold-playtest-5/16_grandma_returns_quickly.png`.

**m2. Too easy for the 8-year-old after two rounds.**
- **Screen**: pumpkin play.
- **Issue**:
  - The beam's grace of about 1 s lets a moving pumpkin cross a sweep unhurt.
  - The ghost cancels a whole visit, even an own-spot check.
  - There is no progression.

  Zoe won Z2 in about 11 s without hiding and lost only when she chose to stand still. Fair and very kind for
  Leo, thin for Zoe: meaningful choices collapse to "keep moving, save the ghost".
- **Concern**: challenge / meaningful choice / boredom.
- **Repro**: Z2 in the trace (shots `112_Z2*`..`115_Z2_2`: `tilt 700 300 -648` through "QUICK! HIDE!", then
  `tilt -700 300 -648` to the door).
- **Screenshot**: `13_Z2_run_through_light_no_hiding.png`, `15_Z5_named_plant_ghost.png`,
  `14_Z3_stand_still_in_light_caught.png`.
- **Suggestion**: hand to game-designer. A gentle ramp after a win streak would help (a shorter grace, Grandma a
  little quicker, a fourth candy), leaving round 1 as it is.

**m3. START, READY, PLAY AGAIN and TRY AGAIN show no pressed state during a firm press.**
- **Screen**: pumpkin intro, calibrate, end cards.
- **Issue**:
  - Leo presses for 1-3 s. Nothing changes until he lifts.
  - Frames during a 600 ms press are pixel-identical to the frame before (`008` vs `009`, `012` vs `013`).
  - It still works on the lift and double taps act once, but a worried 6-year-old re-presses.
- **Concern**: feedback.
- **Repro**: `tilt 0 500 -866`, `down 80 130`, `tick 600`, `shot`. Compare with the frame before.
- **Screenshot**: `frames/008_open_2_1s.png` vs `frames/009_start_pressed.png`.

**m4. The "she found you" moment is shown mostly with Grandma's pointing arm and a dark chevron panel. The
pumpkin's own alarm is brief.**
- **Screen**: pumpkin play.
- **Issue**:
  - The pop-out is clear, but the "!" lasts under 0.4 s and the pumpkin keeps smiling.
  - The ">>>" panel is the dimmest version of the sign.
  - When the chair is named, the arrow points toward Grandma's side of the room.
  - In E1, Leo's tilt was still into the wall, and he was caught about 0.3 s short of the table.
  - 2 of 3 own-spot checks were escapes, so the window is fair. A scared face or a longer "!" and a brighter
    arrow would buy a 6-year-old the missing half second.
- **Concern**: comprehension / feedback.
- **Repro**: L5 in the trace (`down 79 130` at the chair; held; chair named between shots `054_L5_05` and
  `054_L5_06`; `tilt 400 500 -768` at about 169.4 s; "FOUND YOU!" at `055_L5_06`).
- **Screenshot**: `03_L5_E1_chair_named_while_holding_caught.png`, `frames/E1_named_chair_while_hiding.png`.

### Nit

- **n1.** Pressing the pumpkin, the chair or the rest of the room does nothing and gives no feedback (build 4's
  n2). Repro: `down 30 98` for 2 s at the chair, then `down 125 80`. Screenshot: `09_press_pumpkin_or_room_no_effect.png`.
  Concern: feedback.
- **n2.** The win card keeps the frozen flashlight beam across the celebrating pumpkin and Grandma still
  pointing. The "AND ALL THE CANDY" banner sits over her face. A non-reader may half-wonder whether he was caught.
  Screenshot: `frames/045_L3_12.png`, `17_win_and_loss_cards.png`. Concern: consistency.
- **n3.** The loss card never shows what caught him (build 4's n4). Screenshot: `frames/055_L5_12.png`. Concern:
  comprehension.
- **n4.** The calibrate screen shows a "READY!" label just above the "READY" button, which is a redundant
  look-alike. Screenshot: `01_entry_title_calibrate.png`. Concern: consistency.

### Positives worth keeping

- **The pop-out on a named spot, plus re-hiding at the next cover while the thumb stays down, fixes build 4's
  M1.** Leo's natural "keep pressing and tilt" now works.
- Chevrons last as long as the press (build 4's m1 fixed) and skip a just-named spot.
- The ghost is a delightful discovery for a curious tap: the cup crashes and she says "WHAT WAS THAT?".
- Exit protection: a 120 ms brush on the frame-shaped exit during play is ignored, a deliberate press of about
  0.6 s leaves, and the lift on the launcher does not hit the shell's home button.
- I saw no rustle or "SHE HEARD YOU!" when tilting while holding (build 4's m3). Tilting with a held finger at a
  named spot carried him to new cover instead.

## Gesture and interruption matrix

| Gesture / interruption | Result | Evidence |
| --- | --- | --- |
| Quick tap on the sign (Zoe) | Hides only while the finger is down, out on lift. Consistent with "HOLD". | `107_Z1_tap*` (trace) |
| Firm 1-3 s press on the sign | Hides for the whole press | `02_*`, `019_L1_*` |
| Lift then re-press within 120 ms | Stays hidden, no flicker | `08_*` |
| Thumb resting on the room, slid onto the sign | Hides | `08_*` (`052_L5_*`) |
| Press on the sign in the open, still | "<<<" for the full 3.2 s press | `06_*` |
| Press on the sign while rolling to cover | Chevrons, then auto-hide on arrival | `07_*` |
| Held finger when Grandma names the spot | Pops out. Tilting with the finger down re-hides at the next cover. | `03_*`, `04_*`, `05_*` |
| Press on the pumpkin, furniture or room | Nothing | `09_*` (n1) |
| Ghost tap | One distraction per round, ghost dims | `10_*` |
| Double tap PLAY AGAIN | Acts once. The second press does not hide in the new round. | `12_*` (`026_L2_*`) |
| Hold PLAY AGAIN 2.5 s | Starts on lift. No carried touch into the round. | `12_*` (`037_L3_*`) |
| Press PLAY AGAIN and drag off | Cancels | `12_*` (`082_*`) |
| Exit brush (120 ms) mid-round | Ignored | `11_*` (`091_brush_*`) |
| Exit firm press mid-round | Leaves to the launcher within about 0.6 s. The held lift on the launcher does nothing. | `11_*` (`092_exithold_*`) |
| Exit tap on the win card | Leaves at once | `11_*` (`083_exit_1`) |
| Leave and reopen (ordinary resume) | Back to the title, calibrate again. No state kept (by design). | `11_*` (`084_reenter*`) |
| Process restart mid-round (script `reboot`, supplementary) | Profile picker, round discarded (no save by design) | `restart.txt` |
| Tilt into a wall while hidden | Stays hidden | L2, L7 |
| Untested | Strong tilts above about 0.8 g or tilts that flip direction fast; calibrating while the board moves; the shell's rest or daily-cap screen mid-round; two-finger touches; the browser emulator; a physical board | Gaps |

## Supplementary QA (separate from the black-box pass)

- `make -C apps/porthole playtest` at `15ea19f`: **95 of 95 scenarios pass, 0 UI FAIL, 0 UI WARN.** This includes
  `70_pumpkin_win` through `79_pumpkin_own_spot` and the `74_pumpkin_monkey` sanitizer run. I read
  `build/playtest/report.md` in full; its UI findings table is empty.
- `ui` dump (`ui_tour.txt`, output `ui_out.txt`):
  - Exit 68,8 24x22; sign 40,119 80x24; ghost 22,30 24x24. All are at least 24x22 and inside the glass.
  - Text is 7 px tall, cream on dark, 211C31/FFEFC6.
  - The lowest title text ("GET CANDY", 53,146 53x7) has its far corner at 77.8 px from the center. It is inside
    the 79 px limit but visibly close to the rim.

## Comparison with report 4 (`pumpkin-hold-playtest-4.md`)

- **M1 (named spot, non-reader, held finger): fixed.** Report 4 had 3 losses in 8 own-spot checks, with Leo
  stuck holding. Here there was 1 loss in 3 checks, and the loss came from slow reaction and tilt direction, not
  from the finger.
- **m1 (short chevrons): fixed.**
- **m2 (Grandma returns quickly): unchanged.** It is now m1 here.
- **m3 (rustle): not seen.**
- **n2 and n4: unchanged.** They are now n1 and n3.

## Suggested brief changes

- For reaction-window checks, ask the playtester to poll at 1 frame (or 200 ms) and to probe a 1 s and a 3 s
  reaction separately. My batched frames fixed Leo's reaction at about 2.5-3 s.
- Make "rests a thumb" (long holds) and "presses 1-3 s" separate probes. They give different pacing results.
- Ask the playtester to test the ghost as a non-reader would find it, and to report how much it trivializes a
  visit for an older kid.

## Gaps

- Emulator roleplay only: no real child and no physical board, so there is no evidence about accelerometer noise
  or how the hand covers the glass.
- Three own-spot events for Leo is a small sample. The 4.2 s window and the 1 s beam grace are timings I
  observed, not measured parameters.
- Reaction times were limited by my 300-400 ms frame batches.
- The untested cells in the matrix above.
- Process-restart behavior was checked only in script mode.

## Verdict

**Acceptable.** No Blocker or Major remains. The rule change does what build 4 needed: a non-reader whose spot is
named sees the pumpkin pop out with Grandma pointing and an arrow. His instinct to keep pressing and tilt now
carries him to the next cover. A held finger, slides, re-presses and long presses on a coverless sign all behave
as a 6-year-old expects.

Minors m1 (Grandma returns quickly), m2 (thin challenge for an 8-year-old), m3 (no pressed state on the big
buttons) and m4 (the found moment needs half a second more alarm) should be fixed or explicitly deferred, with
their player impact, by the coordinator.

## Evidence index (`pumpkin-hold-playtest-5/`)

- **Sheets**:
  - `01_entry_title_calibrate.png`, `02_L1_first_round_hide_and_win.png`
  - `03`-`05`: the three own-spot events
  - `06`-`09`: sign and finger probes
  - `10_ghost_distraction.png`, `11_exit_brush_hold_tap_reenter.png`, `12_playagain_doubletap_slideoff_carry.png`
  - `13`-`15`: Zoe's runs
  - `16_grandma_returns_quickly.png`, `17_win_and_loss_cards.png`
- **Native frames** (`frames/`): E1-E4, chevrons held, the pumpkin press, the win and loss cards, START before and
  during a press, and the picker.
- **Inputs and scripts**:
  - `trace_15ea19f.txt`, the full input trace with sim ms
  - `driver.txt`, `client.sh.txt`, `film.sh.txt`, `sheet.txt`
  - `ui_tour.txt`, `ui_out.txt`, `restart.txt` (supplementary)
