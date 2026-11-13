# Pumpkin Hide & Seek, hold-to-hide retest: child-perspective black-box playtest 2 (Leo, 6, and Zoe, 8)

This is independent emulator roleplay. **It is not a study with real children, and it was not run on a physical
device.** Every observation comes from the headless host simulator (`snap --serve`) and its rendered 480x480
frames.

## Build and setup

- Revision: `dd7edef` (clean tree). Executable: a private copy of `apps/porthole/build/host/snap` in a session
  scratch folder, sha256 `07f2cc9878edc03f48e86d99973688c447508e307a9bebcc2a19fc567c920bd1`.
  `make -C apps/porthole snap` was already up to date. The hash was the same after `make playtest`.
- Run: one `snap --serve --fresh` process with the default clock (Tuesday 16:00), started in a private
  subfolder. A small socket broker kept that process alive and sent only `down`/`move`/`up`/`tick MS`/
  `tilt X Y Z`/`frame`. The driver is saved as `pumpkin-hold-playtest-2/traces/driver-*.txt`. The full input
  trace, with cumulative simulated time, is `traces/serve-trace.txt` (`[t=ms]`, comment lines give the
  persona's intent). Both personas played in the same process, Leo first, then Zoe as a second profile.
- Tilt: gravity in milli-g in the glass plane. Leo calibrated holding the board at (0, 600). He steered with
  x at +-300-500 and once laid the board flat. Zoe calibrated at (50, 800) and steered with x at +-500-600.
  TRY AGAIN and PLAY AGAIN skip calibration, so each calibrated once.
- Profiles were created through the screens: Leo (frog, age 6, no code) and Zoe (fox, age 8, no code). The
  Spooky tile was opened from the launcher. In the black-box pass I used no `dbg`, `app`, clock skip or
  hidden-state query. The supplementary checks below are labeled.
- The trace gives reproducible inputs. I did not replay it in a second fresh process to compare pixels, so I
  make no frame-exact replay claim.

### Disclosure

Before play I read `apps/porthole/docs/playtesting.md`, the `playtest` and `tour` skills, and the top of
`apps/porthole/tools/webemu.py` (only for the `--serve` protocol). Several things reached me automatically or
by accident:

- The root `CLAUDE.md` says the goal: grab three candies and escape while Grandma searches with a flashlight.
- `apps/porthole/CLAUDE.md` loaded when I touched that folder. It says "three hiding spots", "sweeps,
  turn-backs, spot checks", "a frosting-to-pie retry sequence" and "Pumpkin's ghost instead acts on the lift
  of any press, and its action sign hides the pumpkin exactly while a finger that touched it, pressed there or
  slid on, stays down, wherever it drifts".
- A `grep` for `--serve` printed two lines of that same file, about `test_pumpkin.cpp` (a winnability bot and
  input gates), and the protocol comment line of `host/sim.cpp`.

So as the tester I knew in advance that hiding is hold-based, that slide-on and drift are meant to work, and
that the ghost does something. The personas still decided everything from the screen. Leo's choices were
limited to what a non-reader could see. I did not read game, os, shell or host source, `tests/playtests/`,
`docs/superpowers/`, AGENTS.md files on purpose, or earlier reports. After all play and the automated suite, I
read `docs/pumpkin-hold-playtest.md` for the comparison at the end.

## Personas and brief

- **A. Leo, 6:** can't read and tilts with both hands. He presses firmly for 1-3 s and double-taps when
  worried. Sometimes he rests a thumb on the glass and slides it.
- **B. Zoe, 8:** a confident reader who taps quickly and likes to work out the rules, including trying to cheat
  by mashing.

Both are simulated perspectives only.

Dispatch brief, verbatim:

> You are a fresh, independent child-perspective playtester for Pumpkin Hide & Seek on Porthole. Porthole is a
> round 480x480 touchscreen with no buttons, emulated on the host. Follow apps/porthole/docs/playtesting.md and
> the `playtest` skill.
>
> This retest follows fixes to a changed core mechanic, so play SOURCE-FREE and first-play. Until you finish,
> don't read:
> - source (apps/porthole/games/, os/, shell/, host/)
> - the tests/playtests scripts
> - docs/superpowers/
> - AGENTS.md files
> - earlier playtest reports
>
> Disclose anything that loaded automatically. Decide everything from the screen. Run the sim from your own
> copy of the snap binary, in a private scratch subfolder.
>
> ## Personas (two passes)
> A. Leo, 6. He can't read and tilts with both hands. He presses firmly for 1-3 s and double-taps when worried.
> He sometimes rests a thumb on the glass and slides it.
> B. Zoe, 8. A confident reader who taps quickly and likes to work out the rules, including trying to cheat by
> mashing.
> Both are simulated perspectives only.
>
> ## Setup
> - Run `make -C apps/porthole snap`. Record `git rev-parse --short HEAD` and the `sha256sum` of the snap you
>   run.
> - Run `snap --serve --fresh` with the default clock. Create a profile (age 6 or 8, no code) through the
>   screens, then open the Pumpkin game ("Spooky") from the launcher.
> - Use ordinary touches and tilt inputs only. No dbg, shortcuts or clock skips.
>
> ## Scope
> Play at least five rounds per persona, as each kid would.
> - Does Leo now work out how to hide in time, from pictures alone? Does he tell "can hide here" from "no
>   cover here"? Does he notice the arrow, and does it help him find cover?
> - Does he know when it's safe to let go?
> - Are holds, slides onto the sign, double taps and a resting thumb all handled the way a kid expects?
> - Can Zoe beat Grandma by mashing? Is a check of her own spot fair and readable?
> - Do they collect all the candy and escape? Where do they get frustrated or bored? Would they play again?
> Record win/loss per persona.
>
> ## Output
> Write apps/porthole/docs/pumpkin-hold-playtest-2.md. Put screenshots and the input trace in
> apps/porthole/docs/pumpkin-hold-playtest-2/. Don't save .py files; save scripts as .txt.
>
> The report includes:
> - the revision and hash
> - the personas and this brief
> - win/loss per persona
> - findings ranked, with repro steps
> - the qualitative assessment
> - gaps
>
> Afterwards you may read apps/porthole/docs/pumpkin-hold-playtest.md and compare. Don't fix code and don't
> commit. Reply with ONLY these, in under 8 lines: a verdict (acceptable / needs fixes), win/loss per persona,
> the top findings, and the report path.

The brief was not revised during this run.

## Outcomes

| Persona | Rounds | Wins | Losses | Notes |
| --- | --- | --- | --- | --- |
| Leo, 6 | 6 | 4 (R1, R2, R5, R6) | 2 (R3, R4) | R3 was a deliberate probe (he sat at cover without pressing). R4 was a natural loss: a worried press on the dark sign in the beam. |
| Zoe, 8 | 6 | 3 (R1, R2, R6) | 3 (R3, R4, R5) | R3 (hid in the spot Grandma named) and R4 (mashed there) were her rule experiments. R5 was greedy: candy over cover in a sweep. |

Leo collected all three candies and escaped in R1, R2, R5 and R6. Zoe did in R1, R2 and R6. Rounds took about
13-50 s. Contact sheets: `pumpkin-hold-playtest-2/sheets/` (every captured frame, labeled). Key native frames:
`pumpkin-hold-playtest-2/shots/`. The sheets and shots are all 480x480 frames from the emulator.

## What Leo did (first play)

1. **Launcher to title (trace lines 42-63).** He held the jack-o'-lantern tile for 1.5 s. The caption read
   "Spooky", and the game opened on lift (`s006_holding_tile`, `s007`). The title says "TILT TO ROLL / START /
   GET CANDY". He can read none of it, but the big cream box is the obvious button. A 1.2 s press on it
   opened calibrate: a picture of hands holding the board, "HOLD IT COMFY AND STILL", "READY!" and a READY
   button (`s010`). One more firm press, and he was in play (`s012`). The pumpkin sat under the table, three
   candy corns lay on the rug, an empty three-slot shelf hung on the wall, and the sign read HOLD TO HIDE.
2. **Round 1 (lines 74-171): won, never really hid.** He tipped right toward the candy. It sparkled into the
   shelf (`s016`), and Grandma appeared at the right ("GRANDMA!"). As the pumpkin left the table the sign went
   dark slate with an arrow, "<- FIND COVER", and flipped to "FIND COVER ->" as he neared the plant (`s017`,
   `s019`, `s020`). At the plant it went back to cream HOLD TO HIDE. The beam came ("QUICK! HIDE!", `s022`).
   Worried, he double-tapped the sign, which did nothing visible and did no harm (`s023`, `s024`). The beam
   swung away and Grandma left ("PHEW!"). He rolled left for the last candy. The door lit up with a big
   arrow, Grandma came back, and he rolled out of the door: "YOU DID IT! GRANDMA IS PUZZLED" (`s056`). He won
   without learning anything about hiding.
3. **Round 2 (lines 172-288): won, and discovered hiding.** At the plant with the beam coming, he pressed the
   cream sign firmly (his worried firm press) for 2.4 s. The pumpkin tucked into the plant pot, and the sign
   turned orange: HIDING (`s075`). This is clear picture feedback that pressing makes the pumpkin hide. He
   let go when the beam had swung away. Later Grandma pointed at the table, with a "?" bubble over it ("THE
   TABLE?"), while he rolled past the chair, and he escaped: "YOU ESCAPED! WITH THE CANDY" (`s103`).
4. **Round 3 (lines 289-367): lost (probe).** He sat under the table, where the sign read HOLD TO HIDE, and did
   not press. The beam's bright pool reached the table. "QUICK! HIDE!" showed for about 0.9 s (`s127`), then
   "THERE YOU ARE!" (`s128`), an orange frosting wipe, and "YOU LOST" with a jack-o'-lantern pie and "OH MY!
   PIE!" (`s144`). Being near cover is not enough; you have to press.
5. **Round 4 (lines 368-444): lost.** A double tap on TRY AGAIN started one round, not two, and the second tap
   did not hide the pumpkin. He rolled for candy, and Grandma came while he was in the open between the table
   and the plant: "<- FIND COVER" on the dark sign, with the beam on him (`s163`, `s165`). He pressed the dark
   sign firmly. Nothing happened, and "FOUND YOU!" came within 0.25 s (`s166`). The pie followed. The sign
   gave no false "hiding" look, which fixes the old M1. He just learned that the dark box does not work.
6. **Round 5 (lines 445-717): won.** He went for candy, saw Grandma, and tilted toward the plant because the
   arrow pointed there. When the sign turned cream he held 3 s, the beam passed over the hidden pumpkin, and
   he let go once it had gone (`s190`-`s205`). At the chair he held 2 s while Grandma pointed at the table.
   He let go with the beam still sweeping toward him, and it stopped short. He was lucky (`s226`-`s237`).
   Then came the slide test: his thumb landed on the rug above the sign and slid down onto it. The pumpkin
   hid at once (`s253`, `s254`). His thumb drifted up off the sign into the room, still down, and the pumpkin
   stayed hidden until he lifted after "PHEW!" (`s263`, `s265`). He collected the last candy and escaped.
7. **Round 6 (lines 718-940): won.** He held PLAY AGAIN for 3 s. Nothing visible happened until he lifted (no
   pressed look, `s296`), then the round began. The carried press did not hide the pumpkin (`s301`). He pressed
   the dark sign at a quiet moment: no change at all, no pressed look, no wiggle (`s302`-`s305`). Then he kept
   holding it while tilting toward the plant. **On arrival the pumpkin hid by itself, and the sign switched
   to HIDING** (`s307`-`s311`). He held through a whole visit, about 9 s. Grandma left, and the bubble read
   "HIDDEN!" (`s352`). He escaped past a "THE CHAIR?" check (`s391`).
8. **Curiosity (lines 941-1030).** He pressed the ghost in the window firmly. On lift it went dark, a crash
   sparkle appeared by the plant, and Grandma came in: "WHAT WAS THAT?" (`s395`, `s396`). She looked and left.
   To Leo that was a funny surprise, but it summoned Grandma, and he had no idea what it was for. Then he
   pressed the picture frame at the top (an arrow in a frame on the wall). **After about 0.45 s of holding it
   left the game for the launcher, mid-round** (`s406`). A quick 100 ms tap on it did nothing (`s412`). While
   he held it, only a tiny yellow bar grew under the frame (`s416`).
9. **First voluntary stopping point: after round 6** (about 4 minutes of play). He was winning more than
   losing, the pie is funny enough that a loss is not upsetting, and the hiding feedback is satisfying. By R6
   he has worked out "find the cream box, hold until Grandma goes". There is no new room or new thing to see,
   so I expect him to wander off after a win at about this point, or to leave by accident through the frame.

## What Zoe did (first play)

1. **Title (lines 1082-1098).** She read everything: TILT TO ROLL, GET CANDY, HOLD IT COMFY AND STILL, and
   HOLD TO HIDE in play. Her first rule was "roll to candy, hold the sign at a cover when she comes".
2. **Round 1 (lines 1099-1495): won.** She grabbed two candies in about 1.5 s. Under the table she mashed the
   sign (80 ms down, 80 ms up, about 6 a second) through a sweep. The pool crossed the table and the pumpkin
   ("QUICK! HIDE!"), and she was not caught (`zm08`, `zm11`). She mashed again at the plant. Each press hid
   the pumpkin and each lift showed it (`zn03u`). Grandma left, and the bubble said "HIDDEN!". After the mash
   she left the pumpkin exposed at the plant while the beam's cone passed over it, and it was still safe
   (`zo3`). Only the bright pool at the end of the beam seems to find you. She escaped (`z069`).
3. **Round 2 (lines 1496-1653): won.** She camped hidden at the plant for about 45 s across three visits. Every
   sweep passed harmlessly ("PHEW!" / "HIDDEN!"), with no spot check on her spot. That was boring, but it
   taught her that a held hide beats any sweep. She left, Grandma pointed at "THE CHAIR?" as she approached
   it, and she rolled straight past to the door.
4. **Round 3 (lines 1654-1800): lost, experiment.** With all three candies and the door open, she hid at the
   chair to see what a check would do. Grandma came back and said "THE CHAIR?", with a "?" bubble over the
   chair, for about 3 s. **The sign kept showing the orange, safe-looking HIDING the whole time** (`z191`,
   `z196`). Then "RUN AWAY!", with the sign saying RUN!, and about 0.6 s later "GOTCHA DEAR!" while she was
   still holding (`z197`, `z198`). The pie said "YUM YUM!" (`z202`). Zoe read "THE CHAIR?" and found the rule
   fair: "she said the chair, and I stayed".
5. **Round 4 (lines 1801-2233): lost, cheat test.** She mashed at the chair through two whole sweeps and
   survived both, including the pool landing on her (`zp19`, then "PHEW!" in `zp23`). Then came "THE CHAIR?".
   She kept mashing, and this time the header said **"QUICK! HIDE!"** rather than "RUN AWAY!" (`zq87`), then
   "GOTCHA DEAR!" (`zq91`). Mashing does not beat a spot check. A control in R5, sitting unhidden at the far
   left of the chair through a sweep, also survived (`z228`-`z235`), so mashing gave no clear edge over doing
   nothing in that sweep either.
6. **Round 5 (lines 2234-2308): lost, greedy.** She kept tilting toward the plant candies during a sweep and
   reached the third candy in the beam ("QUICK! HIDE!", `z245`). "FOUND YOU!" came about 0.8 s later
   (`z247`). It was fair, and she knew why.
7. **Round 6 (lines 2309-2394): won in about 13 s.** She tried the ghost with a quick tap while Grandma was
   in the room. Grandma said "WHAT WAS THAT?", looked at the crash beside the plant and left (`z257`, `z259`).
   Zoe worked out that the ghost sends Grandma away once per round. She grabbed the last candy and escaped
   (`z283`).
8. **Zoe's stopping point: after round 6** (about 3 minutes of play). She had worked out every rule: hold at
   cover, only the light pool counts, move when Grandma names your spot, the ghost sends her away, and mashing
   doesn't help. The candies were in the same three places every round, the room never changes, and the
   pumpkin is slow at full tilt (about 3.5 s across half the rug). She would play again another day but would
   want "a harder level" now.

## Qualitative assessment

**Can this persona recognize the goal and actions from the screen and tell what changed?** Zoe, yes, within
round 1 and entirely from the words. Leo, mostly from pictures, and faster than before. Pictures carry
tilt-to-roll, candy (sparkle plus shelf), the threat (Grandma and the beam), the exit (the lit door with an
arrow) and, most importantly, the hide itself: pressing the cream box tucks the pumpkin into the furniture and
turns the box orange. He found the press by accident in R2 (a worried press) and used it on purpose from R5.
He can tell "can hide here" from "no cover here" by color: the box is cream at cover and dark slate with an
arrow in the open. The dark box now gives no false success. Pressing it does nothing, so he learns "dark box
= no". The arrow is noticeable, and in R5 it sent him the right way, to the plant. It flips direction between
spots (`s148` -> vs `s150` <-), which may read as twitchy, but it always points at a real hiding place. The
most kid-friendly fix is that holding the dark box while rolling hides him the moment he reaches cover. Leo's
natural "press and keep pressing" just works.

**Does play offer decisions, understandable consequences and useful feedback? Is challenge fair?** For Zoe,
yes: which spot, when to spend the ghost, when to cross during a check, and candy greed versus safety. Every
loss had a reason she could state. Sweeps are lenient, since only the bright pool catches, so most risk comes
from checks and greed. For Leo, sweeps are forgiving: he won R1 without ever hiding, and a 2-3 s hold was
usually enough because the beam had moved on by then. Grandma visible means keep holding, and Grandma gone
plus "PHEW!" or "HIDDEN!" means safe. In this build "PHEW!" appeared only once Grandma had left, so he got no
false all-clear. The unfair case for a non-reader is a spot check of his own spot (Major M1 below). The box
keeps saying HIDING for 3 s while the game has decided he will be caught, and it changes to RUN! only about
0.6-0.8 s before the catch. The pumpkin cannot get out of the pool in that time.

**Where does play become repetitive, passive, confusing or frustrating? When would each stop?** Leo: the
confusing moments were the dark box doing nothing in a panic (R4) and the ghost summoning Grandma for no
visible reason. Neither was upsetting, because the pie is funny and rounds are short. He would stop after
about six rounds, around a win, or leave by accident through the wall picture. Zoe: camping hidden is safe
but dull (R2, 45 s). The same three candy spots every round, one room and a slow pumpkin make it thin after
about six rounds. Holding through a whole visit (5-9 s) is tense rather than boring for Leo.

**Can imprecise input, a wrong guess or an interruption be recovered from?** Mostly yes. A resting thumb on
the bezel does nothing. A thumb that slides onto the box hides. Drift off the box keeps the hide. Double taps
and mashing are harmless. A press carried from PLAY AGAIN into the round does not hide. START, READY, TRY
AGAIN and PLAY AGAIN all accept 1-3 s presses (on lift). A loss costs only a short round, and the next one
starts at once. The exceptions: a firm curious press on the wall-picture exit leaves mid-round after about
0.45 s and drops that round's candies (m1), and a press about 1 mm below the box misses it (n5).

## Findings (ranked)

### Major

**M1. When Grandma checks the spot where the kid is hiding, the sign keeps saying HIDING (the safe state) for
about 3 s, then shows RUN! only about 0.7 s before the catch, too late to escape.**
- **Screen**: `pumpkin_play`.
- **Issue**: During "THE CHAIR?" (a "?" bubble over the chair), the orange HIDING sign stays exactly as it
  looks when hiding is working. Then "RUN AWAY!"/RUN! shows for 4 frames at 200 ms (about 0.6-0.8 s), then
  "GOTCHA DEAR!" with the finger still down. At the pumpkin's speed (about 20 logical px/s at a strong tilt),
  RUN! leaves no real way out. If the kid is mashing instead of holding, the header even says "QUICK! HIDE!"
  during the check of their own spot.
- **Concern**: comprehension / feedback / fairness (non-reader).
- **Evidence**: Zoe read the rule and called it fair, but she reads. For Leo the only cue is Grandma's
  pointing arm and the small "?" bubble, and the box he has learned to trust says "safe". I did not see a
  check of Leo's own spot in his six natural rounds (two checks named a spot he was passing). His loss there
  is an inference from the same frames, not an observation.
- **Repro**: `traces/serve-trace.txt` line 2471 (R7): hold `down 80 131` at the chair from about t=341.2 s.
  "THE CHAIR?" at line 2567 (`tp145`, t=350.45 s) through `tp159`, RUN! at `tp160` (line 2597), caught at
  `tp164` (line 2605). The same thing happens in Zoe R3, lines 1727-1783 (`z191`, `z196`, `z197`, `z198`), and
  while mashing in R4, lines 2199-2216 (`zq87`, `zq91`).
- **Screenshot**: `pumpkin-hold-playtest-2/shots/tp145.png`, `tp159.png`, `tp160.png`, `tp163.png`, `tp164.png`,
  `z196.png`, `z197.png`, `zq87.png`; sequence `sheets/probes_r7.png`.
- **Suggested direction** (owner decides): from the first frame of a check of the kid's current spot, show it
  in pictures on the sign itself, for example the dark "no cover" style with an arrow away, or RUN! with
  Grandma's pointing icon. Never show "QUICK! HIDE!" during a check of the kid's own spot.

### Minor

**m1. The wall-picture exit leaves mid-round after about 0.45 s of pressing, which is shorter than a
6-year-old's normal press.**
- Leo pressed it out of curiosity (it looks like a picture on the wall) and was on the launcher before he
  lifted, with that round's two candies gone. A quick tap correctly does nothing. The only hold feedback is a
  tiny yellow bar under the frame.
- **Concern**: recovery / consistency. The exit is also not the orange home button the shell uses
  (previous n7).
- **Repro**: lines 974-982 (`down 80 22`, 1.5 s hold, during play); lines 1010-1019 for the frames.
- **Screenshot**: `shots/s406.png`, `shots/s416.png`, `shots/s412.png` (quick tap ignored), sheet
  `sheets/leo_ghost_exit.png`.

**m2. Pressing the dark FIND COVER box gives no response at all.**
- This fixes the old M1's false success: there is no pressed look and nothing hides. But to a non-reader the
  silence looks like "button broken". In R4, Leo's worried press got nothing back before "FOUND YOU!". A
  wiggle of the arrow or a "no" shake of the pumpkin would teach "not here, go that way".
- **Concern**: feedback.
- **Repro**: lines 737-745 (`down 80 131` in the open, 1.5 s). Lines 418-421 (in the beam).
- **Screenshot**: `shots/s302.png`-`s305.png`, `shots/s165.png`, `shots/s166.png`.

**m3. Repetition: the same three candy spots every round, and one room.**
- All twelve natural rounds had candy just left of the table/chair, right of the table, and right of the plant
  pot. With the ghost, Zoe's R6 took 13 s.
- **Concern**: boredom / meaningful choice (Zoe).
- **Repro**: the first frames of each round: `sheets/leo_r2.png` (s057), `leo_r3_r4.png` (s109, s145),
  `zoe_r2.png` (z070), `zoe_r5_r6.png`.
- **Screenshot**: `shots/s012.png`, `shots/z007.png`.

**m4. The ghost's purpose is invisible to a non-reader, and using it summons Grandma.**
- For Leo the result is "I touched the ghost and Grandma came" (`s395`, `s396`). Zoe decoded it from "WHAT WAS
  THAT?" and found it the best tool in the game.
- **Concern**: comprehension (Leo).
- **Repro**: line 949 (`down 33 40`, 1.2 s, during play). Line 2335 (Zoe's quick tap).
- **Screenshot**: `shots/s395.png`, `shots/s396.png`, `shots/z257.png`, `shots/z259.png`.

**m5. "YOU LOST" over a baked-pumpkin pie (unchanged from the previous report's m6).**
- The captions are playful ("OH MY! PIE!", "PIE TIME!", "YUM YUM!", "SWEET PUMPKIN!"), and neither persona was
  upset, but "YOU LOST" is the one blunt phrase.
- **Concern**: tone.
- **Screenshot**: `shots/s144.png`, `shots/z202.png`.

### Nit

- **n1.** The pumpkin is slow for Zoe at full tilt (about 3.5 s from the chair to the plant at 0.6 g, sheet
  `zoe_setup_r1.png` z032-z043). It suits Leo. Concern: pacing.
- **n2.** The win and lose banners are drawn over the ghost ("YOU DID IT!", "SNEAKY PUMPKIN!", "THERE YOU
  ARE!"; `s056`, `s391`, `s128`). On the win card the pumpkin is pictured back at the table rather than out
  the door. Concern: overlap.
- **n3.** "HIDDEN!" or "PHEW!" stays in the bubble for several seconds after Grandma has gone, and changes
  straight to "GRANDMA!" when she returns (`z186`-`z190`, `s352`). It is harmless, but there is no clear "all
  clear, go" beat. Concern: feedback.
- **n4.** START, READY, PLAY AGAIN and TRY AGAIN show no pressed look during a long press. They act only on
  lift, so a 3 s press looks dead until he lets go (`s296`). Concern: feedback.
- **n5.** The sign's hit area matches its drawn box exactly. A press 1 mm below it (y=145 logical) or 4 px
  to the side misses (`off_80_145`, `off_38_131`; `off_42_131` and `off_80_119` hit). It is fine at 80x24,
  but a little padding at the bottom would catch a thumb resting near the bezel. Concern: target size
  (precision, not an audit violation).
- **n6.** The FIND COVER arrow can flip between <- and -> on adjacent frames when the pumpkin is between two
  spots (`s148`, `s150`). Concern: feedback.

### Positives worth keeping

- The hide happens on press, survives drift off the sign and starts from a slide-on. Holding the dark box
  while rolling hides the pumpkin the moment it reaches cover. These are exactly what a firm-pressing,
  thumb-resting 6-year-old does.
- The dark versus cream sign and the arrow are a real picture cue. The dark box no longer pretends to hide.
- "PHEW!" now waits until Grandma has left, and "HIDDEN!" rewards a clean hide.
- Double taps, mashing, carried presses and a resting thumb on the bezel do nothing harmful. START, READY,
  TRY AGAIN and PLAY AGAIN take 1-3 s presses.
- Losses are short, funny and recover instantly. The door-with-arrow exit cue is clear.

## Answers to the scoped questions

- **Does Leo now work out how to hide in time from pictures alone?** Yes, by round 2 (found by a worried
  press) and deliberately by round 5. He never needed to read.
- **"Can hide here" versus "no cover here"?** Yes, by the cream/dark color and the pumpkin actually tucking
  in. The silent dark box is the weak point (m2).
- **Does he notice the arrow, and does it help?** He follows it (R5, toward the plant). It sometimes flips
  between spots (n6).
- **Does he know when it's safe to let go?** Mostly. Grandma on screen means hold, and Grandma gone means safe.
  He released early twice and got away with it because the pool had moved on. The exception is a check of his
  own spot, where the sign says safe when it isn't (M1).
- **Holds, slide-on, double taps, resting thumb?** All handled as a kid expects (see the matrix).
- **Can Zoe beat Grandma by mashing?** No, not reliably. Mashing at cover survived sweeps (three of three),
  but standing still sometimes survives a sweep too, and mashing was caught by the spot check.
- **Is a check of her own spot fair and readable?** Readable and fair for a reader, with a 3 s "THE CHAIR?"
  warning. It is not pictorially readable for a non-reader (M1).
- **Collect all the candy and escape? Frustration, boredom, play again?** Both did: Leo in 4 rounds, Zoe in 3.
  Leo would play again. Zoe would play again another day but wants variety (m3).

## Gesture and interruption matrix

| Gesture / case | Persona (round) | Result |
| --- | --- | --- |
| Firm hold on HIDE at cover, 2-9 s | Leo R2, R5, R6; Zoe R2, R3 | hides on press, stays hidden while held |
| Hold released mid-sweep | Leo R5 (2 s, 3 s) | safe both times (the pool had moved on) |
| Hold at a spot Grandma names | Zoe R3, R7 probe | HIDING for about 3 s, RUN! about 0.7 s, caught: M1 |
| Mash at cover (6/s) through sweeps | Zoe R1, R4 | survived 3 of 3 |
| Mash at cover through a check of that spot | Zoe R4 | "QUICK! HIDE!", caught: M1 |
| Double tap on HIDE | Leo R1 | harmless |
| Double tap on TRY AGAIN | Leo R5 | one round, no hide |
| Press the dark box in the open | Leo R4 (in beam), R6 (quiet) | nothing, no feedback: m2 |
| Hold the dark box while rolling into cover | Leo R6 | hides on arrival |
| Thumb lands on the rug, slides onto the box | Leo R5 | hides |
| Thumb drifts off the box while hiding | Leo R5 | stays hidden |
| Thumb resting on the left bezel for 3 s, then lift | probe R7 | ignored |
| Offset presses around the box | probe R7 | x 42-118 and y 119 hit; x 38/122 and y 117/145/148 miss: n5 |
| PLAY AGAIN held 3 s, carried into the round | Leo R6 | starts on lift, no hide, no pressed look: n4 |
| START / READY held 1-1.5 s | Leo | act on lift |
| Ghost: firm 1.2 s press / quick tap | Leo R7 / Zoe R6 | acts, Grandma investigates and leaves: m4 |
| Wall-picture exit: quick tap / press of 0.45 s or more, in play | Leo | ignored / leaves mid-round: m1 |
| Leave mid-round and reopen | Leo | title again, fresh round (no saved state) |
| Tilt over-correction (0.5-0.6 g) | Leo R1, Zoe | the pumpkin speed is capped, no overshoot |
| Board laid flat | probe R7 | pumpkin stays put |
| Tilt while hidden (rustle) | Leo R6 (0.35 g while held) | no "SHH!" seen: **gap**, threshold not reached |
| Hold HIDE and tap the ghost at once | - | **gap**: single-touch sim |
| Changing grip between rounds (TRY AGAIN skips calibrate) | - | **gap** |
| Process restart mid-round | - | **gap** (no persisted state, low risk) |
| Session rest / daily cap mid-round, night tint, shake | - | **gap** |
| A check of Leo's own spot during his natural rounds | - | **gap**: did not occur in six rounds; M1 is inferred for him |
| Physical board, real multi-touch, real children | - | **gap**: emulator roleplay only |

## Supplementary QA (not part of the black-box pass)

- `make -C apps/porthole playtest` at `dd7edef`: **94 of 94 scenarios pass, 0 UI FAIL, 0 UI WARN.** That
  includes `70_pumpkin_win` to `78_pumpkin_hold` (`74_pumpkin_monkey` under ASan/UBSan). I read
  `build/playtest/report.md` in full; its UI findings table is empty. I did not open the scenario files.
- An ad hoc `ui` audit (`traces/supp-ui-audit-script.txt`, script mode with `profile`; output in
  `traces/supp-ui-audit-output.txt`) covered `pumpkin_intro`, `pumpkin_calibrate`, `pumpkin_play` and
  `pumpkin_lost`. The sign is 80x24, the exit 24x22 and the ghost 24x24, all at or above the 24x22 floor.
  The lowest text, "GET CANDY", is a 53x7 box at y 146, worst corner about 77.5 px from the center, inside
  the 79 px limit. Text is dark on cream or cream on dark navy. No finding.
- No new scenario was added. M1 needs the owner's chosen behavior first. A regression could then hold at a
  spot until a check of that spot and expect the sign's state to change when the check begins.

## Comparison with the previous report (`pumpkin-hold-playtest.md`, `088bf6c`)

- **Fixed:** old M1, where FIND COVER looked pressed and like a hide. Now the dark box has an arrow and never
  shows a pressed state, and holding it into cover hides. Old m2, a slide-on thumb not hiding, now hides.
  Old m1, "PHEW!" mid-search, now waits for Grandma to leave. The arrow toward cover, suggested last time,
  was added and helps.
- **Still open:** old m3, hiding in the named spot as a reading-only rule, is now M1. I measured it and found
  the HIDING sign contradicting the check for about 3 s, and "QUICK! HIDE!" while mashing, so I rate it Major
  for the non-reading persona. Also open: old m5 (same candy spots; here m3), old m6 ("YOU LOST"; here m5),
  old n1 (stale bubble; here n3), old n5 (banner overlap and the win picture; here n2), old n6 (slow pumpkin;
  here n1) and old n7 (picture-frame exit; here m1, which I raised because a 0.45 s press leaves mid-round).
- **Not re-observed:** the rustle ("SHH!"), old m4, did not appear at Leo's 0.35 g tilts while hidden. Grandma
  returning quickly (old m7) still happens (`z268`), but it did not cost either persona a round.
- **Outcomes:** the previous Leo went 1 win, 4 losses and would stop after round 3. This Leo went 4 wins, 2
  losses, one of them a probe, and would stop after about six rounds around a win. The previous Zoe went 4-1.
  This Zoe went 3-3, but two losses were her own rule experiments and one was greed.

## Iteration history and verdict

- Iteration 1 (`088bf6c`, snap `e28928e3...`): `docs/pumpkin-hold-playtest.md`, verdict needs fixes (M1:
  false hide feedback on FIND COVER).
- Iteration 2 (this run, `dd7edef`, snap `07f2cc98...`): that M1 and m2 are fixed and verified. A new Major
  (M1 above) was promoted from old m3 with measured timing.
- **Verdict: needs fixes.** One Major remains for the non-reading 6-year-old. During a check of his own spot,
  the sign says he is safe until it is too late to move. Everything else that blocked Leo last time is fixed,
  and both personas can understand, control and enjoy the core loop. m1 (fast mid-round exit) and m2 (silent
  dark box) are the next most valuable fixes. m3 (sameness) is the main limit on Zoe's interest.
- **Suggested brief improvements:**
  - Ask the tester to keep playing the non-reader until a check of his own spot happens while he hides, or to
    report it as a gap. In six rounds it never happened to Leo naturally.
  - Name the rustle probe explicitly, with a range of tilt sizes while hidden, so it is either seen or marked
    as a gap.
  - Ask for a deliberate "touch everything on the wall" curiosity pass. It found the 0.45 s exit.
  - Note that the sim is single-touch, so "hold hide and use the ghost" can only be judged on hardware.

The black-box pass and its findings are emulator roleplay. They do not stand in for real children or the
physical board.
