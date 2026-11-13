# Screen-change tap gate, round 3: black-box kid playtest

Emulator roleplay by an independent agent. Not a real-child study, and not run on a physical board.

## Build

- Revision: `96c3fbc` (clean tree).
- Executable: `apps/porthole/build/host/snap` built by `make -C apps/porthole snap`, copied into a private scratch
  folder and run from there. sha256 `2eb5bc67d39c9eba47d8d8faf7c913ab597bcb6de931c39ed8933361845084b7`. The same hash
  was still in place after `make playtest`.
- Run: `snap --serve --fresh`, default clock. A small socket broker (session scratch only, not saved here because it is
  Python) sent `down/move/up/tick/tilt/frame` lines and saved each `frame` as a PNG. Touch was in logical px (0-159),
  tilt as `tilt X Y Z` in milli-g, the same as the browser emulator. No `dbg`, no `app`, no `profile`, no clock skips
  during play.

## Personas

- **Leo, 6.** Can't read. Presses firmly (200-400 ms), mashes 3-6 times with a finger that drifts a few px, taps
  "again!" about once a second, rests a thumb on the glass while tilting.
- **Zoe, 8.** Reads confidently. Taps quickly (100 ms presses, 150-250 ms apart) from button to button and types
  numbers fast.

Both are simulated perspectives.

## Dispatch brief (verbatim)

> You are a fresh, independent child-perspective playtester for Porthole, a round 480x480 touchscreen with no buttons,
> emulated on the host. Follow apps/porthole/docs/playtesting.md and the `playtest` skill. Play source-free: don't read
> source (apps/porthole/games/, os/, shell/, host/), tests/playtests scripts, docs/superpowers/ or earlier playtest
> reports until you finish. Disclose anything that loaded automatically. Decide from the screen. Run the sim from your
> own copy of the snap binary in a private scratch subfolder.
>
> ## Personas (two passes)
> A. Leo, 6, can't read, taps firmly (200-400 ms presses), mashes 3-6 times with fingers that scatter a few
> millimetres, taps "again!" about once a second, and rests a thumb on the screen while tilting.
> B. Zoe, 8, a confident reader who taps quickly and deliberately from button to button and types numbers fast.
> Simulated perspectives only.
>
> ## Setup
> - `make -C apps/porthole snap`. Record `git rev-parse --short HEAD` and `sha256sum` of the snap you run.
> - `snap --serve --fresh`, default clock. Ordinary touches only; no dbg, shortcuts or clock skips. Tilt inputs through
> the serve protocol are allowed for tilt games. Vary tap positions by a few pixels like a real finger.
>
> ## Scope
> - New profile setup: Leo mashes the age choice and the code screens. Does a code or digits get entered that he didn't
> mean? Zoe types her age and a code quickly. Does every deliberate key count?
> - Sign in on a profile with a code (make one as Zoe), then mash the name row as Leo.
> - Picker → Home → All games → a game and back, mashing as Leo and tapping fast as Zoe. Does any tap act on the next
> screen without being aimed? Does any deliberate tap get ignored?
> - Pumpkin: two rounds. Double-tap READY and TRY AGAIN; HIDE short, long, mashed (also while Grandma is coming); a
> thumb resting on HIDE from the round start; the ghost.
> - Star Fighter: play to a win or a loss while tapping to fire, and watch the result screen.
>
> ## Output
> Write apps/porthole/docs/screen-gate-playtest-3.md, with screenshots and the input trace in
> apps/porthole/docs/screen-gate-playtest-3/ (no .py files; save scripts as .txt). Include the revision and hash, the
> personas, this brief, findings ranked with repro steps, the assessment and gaps. Afterwards you may read
> apps/porthole/docs/screen-gate-playtest-2.md and say which findings recurred. Don't fix code or commit. Reply with
> ONLY (under 8 lines): a verdict (acceptable / needs fixes), the top findings, and the report path.

No brief revisions were made.

## Disclosure

- Read before play, as instructed: `apps/porthole/docs/playtesting.md` and the `playtest` and `tour` skills. The
  playtesting doc's "Timing guards worth knowing" section describes the screen-change gate (800 ms same-spot hold,
  450 ms after a timer change, a 2.5 s cap). That told me what the mechanism was meant to do, but not where any control
  sits.
- To learn the serve protocol, I read `tools/webemu.py` (only its touch and tilt mapping). A grep also showed the
  one-line command list in the header comment of `host/sim.cpp`. I read no other source.
- After play ended, writing this report into `apps/porthole/docs/` auto-loaded `apps/porthole/AGENTS.md`. It describes
  the gate's design: the pad ignores a press near the button that opened it, and `waited()` exists "for Pumpkin's
  resting thumb". No play decision used it. It does inform the severity notes on M1 and m1 below.
- I read `screen-gate-playtest-2.md` only after this report's findings were written (see "Recurrence").

## Method and evidence

All files are in `apps/porthole/docs/screen-gate-playtest-3/`:

- `input-trace.txt`: every command sent, with game time and shot names. `# tap X Y hold= gap=` lines expand to
  `down / tick hold / up / tick gap`.
- `01_...png` to `13_...png`: labeled strips of the native 480x480 frames for each segment. The shot names match the
  trace.
- `native_*.png`: full-size frames for the key findings.
- `repro_*.txt`: raw `snap --script` repros (supplementary, made after the black-box pass) and their frames
  (`zoe_code_repro_frames.png`, `zoe_confirm_mismatch_frames.png`, `pumpkin_thumb_rest_grid.png`). These scripts use
  `profile` as a fixture where noted.

Three of my own taps missed their target because I misread a coordinate. Each is labeled in the trace: the code-pad
gap at (99,74), the Library arrow at x=80, and the gap above a tile at (79,88). They are recorded as attempts, not
findings. The Library arrow region is 24x22, at the floor.

## What happened

### Setup and sign-in

- **Leo, name.** He typed L, E, O. Then he mashed OK 4 times: the face screen opened and nothing else happened. He
  mashed the fox 3 times: Age opened, and the drifting second press (inside the "7" button's top edge) did nothing.
  He mashed "6" 5 times: the code pad opened with no digit typed (`01_`, `02_`). The gate held every carried press.
- **Leo, code.** He mashed the pad's "6" key 4 times at about 450 ms spacing. All 4 counted: 6666, then "Once more!".
  He can't read that, so he mashed "6" again (5 taps at about 1/s). The first was held, the next 4 confirmed 6666, and
  Home opened. The 5th tap landed on Home's Marble tile and was correctly held (`02_`). So a mash of one key gave him a
  code he never chose to make. It happens to be easy to repeat (see m3).
- **Zoe, new profile.** New, N-Z, Z, O, A-M, E, OK, owl, 8: all at 250-280 ms spacing, all counted (`03_`).
- **Zoe, code.** She typed 7 7 3 1 about 300 ms after tapping "8". The two 7s were silently dropped and the boxes
  showed "3 1" (`04_`, `native_013_7731.png`). The pad has no delete key, so she finished with 7 7: code 3177. On
  "Once more!" she typed her real 7 7 3 1 quickly. Both 7s were dropped again (the last key she had pressed was 7), and
  the boxes showed "3 1" (`native_015_confirm_a.png`). She added 7 7 again, so her saved code is 3177, not the 7731
  she meant. That is finding M1.
- **Zoe, sign-in.** At sign-in, 7731 got "Hmm, that's not it" (gentle, the boxes flash red). 3177 opened her Home.
  All 4 fast keys counted both times (`05_`).
- **Leo, name row.** He mashed his row 6 times (450 ms spacing). The pad opened and the mash typed nothing (`06_`).
  He tapped the row again about once a second, 6 times. Taps 2-3 were held, and taps 4-6 (after about 2.5 s) typed
  three hidden digits. A 7th made a wrong code ("Hmm"), then his 6666 worked. This is m4.

### Picker, Home, All games, a game and back

- Leo's mashes on Home's top button, the All games tile (the Radar tile sits under it: not opened), and the Pumpkin
  tile (the game's mid-screen did nothing) never acted on the next screen (`07_`).
- **Zoe, fast.**
  - Star Fighter EXIT goes straight to the Library. Her tap on the ◀ arrow 250 ms later was dropped. Her next tap on
    the house went to Home, and the house again 200 ms later was dropped (`13_`).
  - After sign-in, Home ignored her first tap at about 250 ms, and once at about 600 ms after the code confirm. A
    probe (`repro_home_entry_guard.txt`) shows Home takes a tap only about 0.8-0.9 s after the last code key: the
    pad's pause, then the 450 ms gate.
  - Pumpkin's top sign exits with a plain tap on the title screen. In play it needs a hold of about 0.6 s (m2).
- **Drag tests** (supplementary): a picker-row press slid off before lift cancels. A slide of 6 px or less still
  opens the row; 10 px cancels.

### Pumpkin (3 rounds, Leo)

- **Round start.** He double-tapped START: calibration ("HOLD IT COMFY AND STILL") opened once. He double-tapped
  READY: the round started, and the second press did not hide (`07_`). A supplementary probe with a 400 ms second
  press also did not hide.
- **Round 1 (lost).** He tilted right and got candy 1. Then Grandma came and the sign said FIND COVER. Leo pressed it
  6 times. The sign looks pressed for an instant, but nothing else happens, and there is no picture of where cover is.
  The beam found him: "YOU LOST, OH MY! PIE!" (`08_`, `native_031_m6.png`). TRY AGAIN double-tapped: one new round,
  and HIDE was not hit.
- **Round 2 (won, about 53 s).**
  - A short tap under the table hid him on press: "HIDDEN!", the sign became COME OUT.
  - He mashed the sign 5 more times at about 450 ms spacing. Every press toggled (out, in, out, in, out), so he ended
    in the open with the beam sweeping in (`09_`, `native_035_m6.png`). One more tap hid him and he survived. This is
    M2.
  - A 1.2 s long press hid once and did not toggle back (`10_`).
  - Pictures carried the rest: Grandma's "?" bubble over the spot she checks, and "TO THE DOOR!" with a lit door and
    an arrow. He won: "YOU ESCAPED! WITH THE CANDY".
- **Round 3.**
  - He pressed PLAY AGAIN and held his thumb there for 3 s while tilting. The new round began on lift and nothing
    carried.
  - His thumb then landed on HIDE about 200 ms into the round and rested there. The pumpkin hid by itself about 0.5 s
    into the round, with no new tap (`11_`, `native_056_b.png`). The grid probe shows the same for landings at 100,
    300 and 440 ms (`pumpkin_thumb_rest_grid.png`). This is m1.
  - He tapped the ghost: it dims, knocks the mug off the shelf, and Grandma says "WHAT WAS THAT?" and leaves. A
    second tap on the dim ghost does nothing. The cause and effect is clear, and Leo would love it. On the title
    screen the ghost does nothing.
  - He tapped the top sign 4 times (300 ms) to leave: nothing (`native_058_b4.png`). Holding it fills a thin orange
    bar under the arrow and leaves at about 0.6 s.

### Star Fighter (Leo firing, Zoe leaving)

- Tile, then title (START / EXIT), then "BANK TO STEER / LIFT UPRIGHT" (WAIT until upright, then CONTINUE), then the
  mission card (text only), then FLY. Each step's button sits on the same spot, and Leo's double taps never skipped a
  step.
- About 2.5 minutes of identical fighter waves while tapping 2-3 times a second. Fire responds to every quick tap, and
  the explosions are satisfying. Then the trench, where turret fire took his ships within about 10 s: GAME OVER, score
  11800 (`12_`, `native_071_30.png`).
- His firing taps kept coming as the results appeared. None restarted the game. TRY AGAIN lights up about 1.2 s
  later, and a double tap on it went to calibration once.
- Natural result: a **loss**. No win was reached (gap).

## Findings (prioritized)

### M1 (Major): a reader's fast code digits are silently dropped next to the button that opened the pad, or the key just pressed

- **Screen**: new profile, "Pick 4 numbers" and "Once more!".
- **Issue**: a quick key within about 24 px of the age button that opened the pad is dropped. So is the first confirm
  digit when it repeats the last digit entered. The boxes then show a shorter code, there is no delete key, and the
  saved code differs from the intended one (or "Not the same!" restarts the setup).
- **Concern**: feedback / recovery (deliberate input lost on a secret-code screen; the scope question "does every
  deliberate key count?" is answered no). Every age button sits on keys: 5 over 1, 6 over 2, 7 over 3, 8 over 4 and
  7, 9 over 5 and 8, 10+ over 6 and 9. A child who types her age's neighbour first loses it. The design brief
  (auto-loaded after play) chose this to stop age/row mashes typing digits, a real gain (Leo's mash typed nothing).
  But a reader moving on to a key is a deliberate target too. A visible cue, or keeping the hold only on presses that
  begin before the previous lift, might separate the two.
- **Repro**: `screen-gate-playtest-3/repro_zoe_age_then_code.txt` (age 8, then 7 7 3 1 at 250 ms spacing gives
  "31__"). `repro_zoe_confirm_first_digit.txt` (code 7317, confirm 7317 begun 600 ms later gives "317_", then
  "Not the same!"). Live: `input-trace.txt` from `shot 012_code` to `shot 016_done`.
- **Screenshot**: `apps/porthole/docs/screen-gate-playtest-3/native_013_7731.png`, `native_015_confirm_a.png`,
  `zoe_code_repro_frames.png`, `zoe_confirm_mismatch_frames.png`.

### M2 (Major): a mash on HIDE toggles at every press, so a panicked 6-year-old can uncover himself as Grandma arrives

- **Screen**: Pumpkin, play, Grandma entering.
- **Issue**: at Leo's cadence (300 ms presses, about 450 ms apart) each press flips HIDE / COME OUT, so whether a mash
  ends hidden depends on whether the number of presses is odd or even. His 6 presses ended in the open as the beam
  started.
- **Concern**: challenge / fairness / comprehension (he pressed the hide sign the whole time; to a non-reader both
  labels are just "the button"). A hide-only press while Grandma is in the room, or a separate COME OUT action, would
  match intent.
- **Repro**: `input-trace.txt` `shot 034_up` to `shot 035_m6`. Hidden under the table: `tap 79 130 300 150`,
  `tap 82 132 300 150`, `tap 78 129 250 150`, `tap 81 131 300 150`, `tap 80 130 300 150`.
- **Screenshot**: `apps/porthole/docs/screen-gate-playtest-3/09_pumpkin_r2_hide_toggle_mash.png`,
  `native_035_m6.png`.

### m1 (Minor): a thumb resting on HIDE from the round start hides the pumpkin about 0.5 s in, with no new tap

- **Screen**: Pumpkin, round start.
- **Issue**: a press that begins inside the round-start gate and stays down acts when the gate ends. Leo's resting
  thumb hides him, and he then tilts at a pumpkin that will not roll.
- **Concern**: feedback / comprehension. The pumpkin's flattened pose and "HIDDEN!" do show it. The design brief says
  this is intended (`waited()`), so this may be accepted. The tradeoff is that a resting hand still acts without aim.
- **Repro**: `screen-gate-playtest-3/repro_pumpkin_thumb_rest.txt` (READY, then `down 79 131` 300 ms later, held).
  Live: `shot 056_a` to `shot 056_e`.
- **Screenshot**: `apps/porthole/docs/screen-gate-playtest-3/pumpkin_thumb_rest_grid.png`, `native_056_b.png`.

### m2 (Minor): Pumpkin's in-play exit needs a hold the screen does not explain; the same sign is a tap on the title

- **Screen**: Pumpkin, play.
- **Issue**: four firm 300 ms taps on the top sign did nothing. Leaving takes about 0.6 s of holding, shown only by a
  thin bar that appears under the arrow while held. Star Fighter labels its hold sign "HOLD"; Pumpkin does not.
- **Concern**: consistency / feedback (Leo can't read the label anyway; he can't find the way out).
- **Repro**: `input-trace.txt` `shot 058_b1` to `shot 059_b`.
- **Screenshot**: `apps/porthole/docs/screen-gate-playtest-3/native_058_b4.png`,
  `11_pumpkin_r3_thumb_rest_ghost_exit.png`.

### m3 (Minor): FIND COVER looks like the HIDE button but refuses taps, and only words say why

- **Screen**: Pumpkin, play, away from furniture.
- **Issue**: the sign keeps the same box and position. It depresses on touch and nothing happens; "FIND COVER" is the
  only cue. Leo lost round 1 pressing it 6 times.
- **Concern**: comprehension (non-reader). Out of this gate's scope, but it decides whether Leo learns to hide. A
  picture cue (glowing furniture, a dimmed sign) would carry it.
- **Repro**: `input-trace.txt` `shot 031_m1` to `shot 032_c`, `shot 039_d1`.
- **Screenshot**: `apps/porthole/docs/screen-gate-playtest-3/08_pumpkin_r1_find_cover_loss.png`, `native_031_m6.png`.

### m4 (Minor): mashing a code key sets a code, and "again!" row taps type digits after 2.5 s

- **Screen**: "Pick 4 numbers" / "Once more!"; picker row to "Leo's code?".
- **Issue**: 4 presses on one key make a code and "Once more!". A non-reader repeating the same mash confirms it, and
  "skip" is text only. Row taps about once a second reach the pad after the 2.5 s cap and type hidden digits. The cost
  is a gentle "Hmm, that's not it".
- **Concern**: meaningful choice / recovery (recoverable; a code Leo can repeat by habit).
- **Repro**: `input-trace.txt` `shot 008_c1` to `shot 009_c6`; `shot 023_picker` to `shot 024_b`.
- **Screenshot**: `apps/porthole/docs/screen-gate-playtest-3/02_leo_code_mash_6666.png`,
  `06_leo_name_row_mash.png`.

### m5 (Minor): Zoe's quick deliberate taps are dropped after a timer or game-exit change, and on a same-spot "back, back"

- **Screen**: Library after a game's EXIT (◀ dropped at 250 ms); Home after sign-in (first tap dropped at about 250
  and 600 ms); Library house to Home house (second tap at 200 ms dropped).
- **Issue**: a quick reader's taps are ignored for 450-900 ms with nothing on screen showing why. Each recovers with
  one more tap.
- **Concern**: pacing / feedback (documented tradeoff).
- **Repro**: `input-trace.txt` `shot 074_a` to `shot 075_b`, `shot 077_a`; `repro_home_entry_guard.txt` (vary the
  `wait`).
- **Screenshot**: `apps/porthole/docs/screen-gate-playtest-3/13_zoe_fast_nav.png`, `native_076_c.png`.

### Nits

- Star Fighter's look closely copies a famous film franchise (a moon-sized battle station, its fighters, the trench
  run). Flagged for the owner's originality and IP judgment, not a gate issue.
- Star Fighter's retry goes through calibration, the mission card and FLY again: three steps before play.
- Star Fighter's long first stage (about 2.5 min of one enemy type) is followed by a trench that ended his game in
  about 10 s.

## Qualitative assessment

- **Can the persona see the goal and act?** Mostly, with pictures doing the work. Leo's best moments came from
  pictures: Grandma's "?" bubble, the lit door with its arrow, the ghost knocking the mug down, Star Fighter's
  explosions. Text carries the code screens ("Pick 4 numbers", "Once more!", "skip") and Pumpkin's FIND COVER, so
  Leo makes a code and loses round 1 without knowing why. Zoe reads everything. Her confusion is caused by input
  ("I pressed 7!"), not comprehension.
- **Decisions, skill, feedback.** Pumpkin has real decisions: when to come out, which cover to use, when to spend the
  ghost. Hiding has clear feedback: a flattened pumpkin, "HIDDEN!", "PHEW!". Star Fighter is pure reflex fun, with
  responsive fire and a stable result screen. Feedback fails in four places: a refused FIND COVER, the in-play exit
  hold, a silently dropped key on the pad, and a dropped tap after a screen change.
- **Repetition, frustration, stopping points.**
  - Leo would likely stop Pumpkin after round 1's unexplained loss, unless someone showed him the table. Round 2
    (53 s, a win) is a natural, satisfying stop.
  - In Star Fighter, Leo would drift after 1.5-2 min of identical waves; the explosions keep him longer than the
    mechanics do. The quick trench loss and three-step retry make "again!" slow.
  - Zoe would be annoyed at the code screen (two attempts showing digits she didn't type) but would push through.
  - Beyond these points I continued only for coverage.
- **Recovery without an adult.** Mostly yes. Every dropped tap recovers with one more tap, and wrong codes are gentle.
  But Zoe's saved code is not the one she meant, and only her reading of the boxes saved her. Leo can't get out of
  Pumpkin mid-round without discovering the hold.

## Gesture and interruption matrix

| Gesture | Setup screens | Picker / pad | Home / Library | Pumpkin | Star Fighter |
| --- | --- | --- | --- | --- | --- |
| Single firm tap | ok | ok | ok | ok (exit sign: no, hold needed) | ok |
| Fast deliberate tap chain | ok (name, face, age) | **drops digits near the age button / repeated digit** | dropped 250-600 ms after timer/game exit | ok | ok |
| Double tap | ok, acts once | ok | ok | START, READY, TRY AGAIN: once | START, TRY AGAIN: once |
| Mash 3-6 (about 450 ms) | ok, no carry | row mash typed nothing; key mash sets a code | ok, no carry | HIDE toggles each press (M2) | fires, fine |
| "Again!" at about 1 Hz | not tested | types after 2.5 s (m4) | not tested | not tested | n/a |
| Long press 1-1.2 s | not tested | not tested | not tested | HIDE once; exit works | not tested |
| Thumb resting across a change | n/a | n/a | held exit finger did not act on the Library | PLAY AGAIN: no carry; on HIDE from start: hides (m1) | not tested |
| Drag off / small slide | not tested | off cancels; under 6 px ok, 10 px cancels | not tested | not tested | not tested |
| Off-target by a few px | gap taps do nothing | gap taps do nothing | arrow missed by 4 px (24x22 region) | n/a | n/a |
| Leave / resume mid-game | gap | gap | gap | exit only, no resume | exit only |
| Process restart | gap | gap | gap | gap | gap |

## Recurrence vs. screen-gate-playtest-2

Read after writing the findings above.

- **Round 2's M1** (READY/TRY AGAIN double tap hides): did **not** recur; both double taps acted once, with a 400 ms
  second press too. A narrower relative remains: a resting thumb from the round start hides (m1).
- **Round 2's M2** (mash while hidden comes out, then ignores HIDE): **changed**. Presses are no longer ignored, but
  every press toggles, so a mash can still end uncovered (M2 here).
- **Round 2's M3** (mash on age/row types digits or sets a code): **fixed** for the carried mash; age, face and row
  mashes typed nothing. Two side effects are new or remain: the gate now drops a reader's deliberate digits (M1 here),
  and mashing the keys themselves still sets a code (m4).
- **Round 2's m2** (reader's back-back on a same-spot house): **recurred** (m5).
- **Round 2's m4** (6-press mash reaches the next screen after 2.5 s): **recurred** as Leo's 1 Hz row taps typing
  digits (m4).
- **Round 2's m1, m3 and m5** (Toy Town, Pets Club package, trick lesson): not in this scope, not tested.

## Automated suite (supplementary)

`make -C apps/porthole playtest` at `96c3fbc`: **93 of 93 scenarios pass, 0 UI FAIL, 0 UI WARN**
(`apps/porthole/build/playtest/report.md`, read in full: summary table, empty findings table, contact-sheet list).
The suite does not catch M1 or M2; scenarios `a8_code_pad_input` and `73_pumpkin_kid_taps` are where they would
belong.

## Gaps

- No Star Fighter win; one loss only.
- Pumpkin played by Leo only: 3 rounds, 1 loss, 1 win, 1 abandoned. No Zoe Pumpkin round.
- Not tested: leave and resume mid-game, process restart, the rest and daily caps, any game other than Pumpkin and
  Star Fighter, adult Admin flows, and long presses on setup and Library tiles.
- Tilt came through the serve protocol, not a real accelerometer.
- No physical device and no real child.

## Verdict

**Needs fixes**: M1 (a reader's deliberate code digits are silently dropped) and M2 (a HIDE mash can uncover the
pumpkin) are Major for the stated personas. The carried-tap protection itself works well: no mash, double tap or held
finger acted on a new screen except where noted.
