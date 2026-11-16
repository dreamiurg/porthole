# Pumpkin Hide & Seek: black-box kid playtest 7 (Leo, 6)

Emulator roleplay by an independent, source-free agent. This is not a study with a real child, and it is not a
test on the physical board.

## Build identity

| Session | Revision | `snap` sha256 | What was played |
| --- | --- | --- | --- |
| A (main, first play) | `8dcb4bd` | `d7329c4f94a8a7d133dd82d9593084c1f110ea5531832f1f34b4fc7b44de81a4` | Rounds A1-A10 plus supplementary QA |
| B (replay) | `a29a817` | `cb394939358ae9fa6d44e779c732fe2102cc46b79c617bea6ca6fb101e987112` | Rounds B1-B4 and a ghost probe |

HEAD moved from `8dcb4bd` to `a29a817` while session A was running. `make playtest` rebuilt `snap` at the new
revision. Without reading the diff I ran `git diff --stat 8dcb4bd HEAD`. It showed changes to Pumpkin's
`rules.cpp`, its `AGENTS.md`, `host/test_pumpkin.cpp` and the design doc. Because rules changed, I replayed four
rounds on `a29a817` (session B). The first-play comprehension evidence comes from session A on `8dcb4bd`. Session
B is a replay by an agent that had already played the game, so it cannot support claims about first-time
understanding.

## Persona and brief (verbatim from the coordinator)

> You are a fresh, independent child-perspective playtester for Pumpkin Hide & Seek on Porthole, a round 480x480
> touchscreen with no buttons, emulated on the host. Follow apps/porthole/docs/playtesting.md and the `playtest`
> skill.
>
> This is the final check of the build, and you play SOURCE-FREE. Until you finish, don't read: source
> (apps/porthole/games/, os/, shell/, host/); tests/playtests scripts; docs/superpowers/; AGENTS.md files; earlier
> playtest reports; git logs. Disclose anything that loaded automatically. Decide from the screen. Build once at the
> start; run the sim from your own copy of the snap binary in a private scratch subfolder.
>
> Persona: Leo, 6 (simulated). He can't read and tilts with both hands. He presses firmly for 1-3 s, re-presses
> when worried, and sometimes rests or slides a thumb. He reacts to what he sees within about 1-2 s. Don't play him
> frozen, and don't hold a wrong tilt longer than a 6-year-old would.
>
> Setup: Run `make -C apps/porthole snap`. Record `git rev-parse --short HEAD` and the snap's `sha256sum`. Run
> `snap --serve --fresh` with the default clock. Create the profile (age 6, no code) through the screens and open
> "Spooky" from the launcher. Use ordinary touches and tilt inputs only: no dbg, shortcuts or clock skips.
>
> Play at least eight rounds. Answer these: When Grandma names a spot he is in, next to, or rolling toward, does
> he see without reading that he's in danger, and get away? With all candy collected, does the sign's arrow lead
> him to the exit? Does hiding by holding the sign still work, and do the arrows say "not here, go that way"? Does
> anything feel broken, sticky or confusing? Would he play again?
>
> Record win/loss. List how many times Grandma named a spot he was in, next to, or rolling toward, and what
> happened each time. If it doesn't happen naturally at least twice, say so as a gap.
>
> Write apps/porthole/docs/pumpkin-hold-playtest-7.md, with screenshots and the trace in
> apps/porthole/docs/pumpkin-hold-playtest-7/. Save scripts as .txt; no .py files. [...] Afterwards you may compare
> it with apps/porthole/docs/pumpkin-hold-playtest-6.md. Don't fix code or commit.

The coordinator did not revise the brief.

### Disclosure

- Two files loaded automatically: the repo-root `CLAUDE.md` and `apps/porthole/CLAUDE.md`. The second is the
  app's `AGENTS.md`, through a symlink. Its Pumpkin lines describe the game in outline: three hiding spots, three
  candies, a door, sweeps, turn-backs and spot checks, and a frosting-to-pie retry. They also say that the action
  sign hides the pumpkin while a finger that touched it stays down, and that the ghost acts on lift. I knew all of
  this before playing.
- To learn the `--serve` protocol (`tick`, `frame`, `down/move/up`, `tilt x y z`) I read
  `apps/porthole/docs/playtesting.md`, the `playtest` and `tour` skills, and `apps/porthole/tools/webemu.py`.
- I did not read game source, playtest scripts, specs, earlier reports or git logs. The exceptions were the
  file-name-only `git diff --stat` above and the scenario names that `make playtest` printed. I read
  `pumpkin-hold-playtest-6.md` only after finishing this report (see the comparison at the end).

## Setup and input method

- Session A: `snap --serve --fresh`, from a private copy of the binary in the session scratch directory. Default
  clock. I drove the binary with a small stdin harness, saved here as
  [`pumpkin-hold-playtest-7/driver.txt`](pumpkin-hold-playtest-7/driver.txt) (plus `send.txt`). The harness sends
  only `tick`, `down`, `move`, `up`, `tilt` and `frame`.
- The profile was created through the screens: name keyboard L-E-(N-Z)-O, OK, fox face, age 6, "skip" on the code
  pad. Spooky (the pumpkin tile) was opened from Home.
- Grip: Leo holds the board leaning back, gravity (0, 0.6, -0.8) g, and calibrated there. From A8 on he held it
  more upright, (0, 0.85, -0.53) g, and calibrated again. Tilts were moderate (0.25-0.45 g sideways) with a few
  over-corrections (0.55-0.8 g).
- Pacing: I looked at frames every 400 ms around Grandma, and acted on the next step or the one after it. That
  gives a reaction time of about 0.4-1.2 s. The A1 and A2 reactions were slower because I batched frames, which
  stood in for a kid who isn't looking yet.
- Presses were 0.7-1.5 s on buttons and 2-6 s on the sign.
- Traces: [`trace.txt`](pumpkin-hold-playtest-7/trace.txt) for A plus QA, and
  [`b/trace.txt`](pumpkin-hold-playtest-7/b/trace.txt) for B and the ghost probe. Each input is stamped with game
  time.

## Results: win and loss

| Round | Build | Result | What happened |
| --- | --- | --- | --- |
| A1 | 8dcb4bd | **Loss** | Rolled right after a candy, toward Grandma's "HMM?" light by the plant pot. Caught about 1.5 s after the light appeared ([big_r1def](pumpkin-hold-playtest-7/big_r1def.png), [st05](pumpkin-hold-playtest-7/st05.png), [st06](pumpkin-hold-playtest-7/st06.png)). |
| A2 | 8dcb4bd | **Loss** | Found "HOLD TO HIDE" by pressing the bright sign at the chair ([r2o](pumpkin-hold-playtest-7/r2o.png)). Lifted after 2.5 s, his normal press length, just as the beam reached the chair. "QUICK! HIDE!", then "PEEKABOO!" about 1.2 s later ([st11](pumpkin-hold-playtest-7/st11.png)). |
| A3 | 8dcb4bd | **Loss** | Hid at the table twice and survived both sweeps ([st15](pumpkin-hold-playtest-7/st15.png), [st17](pumpkin-hold-playtest-7/st17.png)), with a thumb slide while hidden ([st23](pumpkin-hold-playtest-7/st23.png)). Got all three candies. "THE TABLE?" came while he was at the pot next to it; he rolled toward the glowing door, through the table, and was caught ([r5b](pumpkin-hold-playtest-7/r5b.png), [big_r5ghi](pumpkin-hold-playtest-7/big_r5ghi.png), [st27](pumpkin-hold-playtest-7/st27.png)). |
| A4 | 8dcb4bd | **Win** | Chair hide, lift, worried re-press. "THE CHAIR?" just after he left the chair ([r6y](pumpkin-hold-playtest-7/r6y.png)); he hid at the table ([st37](pumpkin-hold-playtest-7/st37.png), [st38](pumpkin-hold-playtest-7/st38.png)). Pot hide, then a dash to the door as Grandma came in ([r8w](pumpkin-hold-playtest-7/r8w.png)). |
| A5 | 8dcb4bd | **Win** | Off-centre PLAY AGAIN worked. Quick lift and re-press while hidden. "THE CHAIR?" while he was rolling toward the chair for the last candy; scared face, kept rolling, out the door ([s10h](pumpkin-hold-playtest-7/s10h.png), [st58](pumpkin-hold-playtest-7/st58.png), [st59](pumpkin-hold-playtest-7/st59.png)). |
| A6 | 8dcb4bd | **Win** | Pressed the dark sign off a spot and got ">>>" ([st61](pumpkin-hold-playtest-7/st61.png)); followed it to the pot. "RUN AWAY!" / "THE TABLE?" while he was at the table ([s11w](pumpkin-hold-playtest-7/s11w.png)); rolled to the chair and hid ([st66](pumpkin-hold-playtest-7/st66.png), [st67](pumpkin-hold-playtest-7/st67.png)). Hid with a press on the sign's corner. Door ([st77](pumpkin-hold-playtest-7/st77.png)); sign pointed right with all candy ([st74](pumpkin-hold-playtest-7/st74.png)). |
| A7 | 8dcb4bd | Left mid-round | Big over-tilt into the door with 1 candy: the pumpkin just stops. A press above the sign did not hide ([st80](pumpkin-hold-playtest-7/st80.png)); a re-press on the sign did ([st81](pumpkin-hold-playtest-7/st81.png)). He pressed the "picture" at the top: a quick tap did nothing, a firm press exited to Home within 0.8 s ([st84](pumpkin-hold-playtest-7/st84.png)). The finger still held there did not press Home's house ([s15g](pumpkin-hold-playtest-7/s15g.png)). Reopened: title screen. |
| A8 | 8dcb4bd | **Win** | Upright grip and recalibration worked. Rolled off boldly while Grandma was in the room, outran the beam to the table and hid, then reached the door ("YOU DID IT! GRANDMA IS PUZZLED"). |
| A9 | 8dcb4bd | **Win** | 6.4 s resting hold at the pot. Grandma came back about 1.2 s after "HIDDEN!"; he hid at the table, then dashed to the door ([st100](pumpkin-hold-playtest-7/st100.png)). |
| A10 | 8dcb4bd | **Win** | Paused in the middle of the room and hid late, about 0.7 s after "QUICK! HIDE!", and survived ([st105](pumpkin-hold-playtest-7/st105.png)). Door dash ([st108](pumpkin-hold-playtest-7/st108.png)). |
| B1 | a29a817 | **Win** | Table hide, pot hide (a turn-back sweep hit the pot while he was hidden), door ([b/sb7](pumpkin-hold-playtest-7/b/sb7.png)). |
| B2 | a29a817 | **Loss** | Forgot to hide at the pot after "GRANDMA!". Pressed 0.7-1.1 s after "QUICK! HIDE!" and was caught ([b/sb12](pumpkin-hold-playtest-7/b/sb12.png), [b/sb13](pumpkin-hold-playtest-7/b/sb13.png), [b/sb14](pumpkin-hold-playtest-7/b/sb14.png)). The hide press, still held through to the pie screen and lifted over TRY AGAIN, did not retry ([b/b3q](pumpkin-hold-playtest-7/b/b3q.png)). |
| B3 | a29a817 | **Win** | With all the candy, the beam started at the door, between him and the exit. He hid at the table, then went out ([b/sb23](pumpkin-hold-playtest-7/b/sb23.png), [b/sb24](pumpkin-hold-playtest-7/b/sb24.png), [b/sb25](pumpkin-hold-playtest-7/b/sb25.png)). |
| B4 | a29a817 | **Win** | "RUN AWAY!" / "THE TABLE?" while he was in the table zone with all the candy. Kept rolling left and went out the door ([b/b7o](pumpkin-hold-playtest-7/b/b7o.png), [b/sb33](pumpkin-hold-playtest-7/b/sb33.png), [b/sb34](pumpkin-hold-playtest-7/b/sb34.png), [b/sb35](pumpkin-hold-playtest-7/b/sb35.png)). |

Fourteen rounds were started and 13 finished: **9 wins, 4 losses**, plus one left on purpose. On `8dcb4bd` it
was 6 wins and 3 losses, the three losses coming first. On `a29a817` it was 3 wins and 1 loss. Rounds took 16-40 s.

## Grandma naming a spot he was in, next to, or rolling toward

| # | Round | Spot named | Where Leo was | What he saw | Outcome |
| --- | --- | --- | --- | --- | --- |
| 1 | A3 | THE TABLE? | At the pot next to it, with all candy. Then rolling toward and through it to reach the glowing door. | A "?" bubble on the table and Grandma pointing, for about 2.7 s. The pumpkin looked normal until it entered the table zone; then the beam, a scared face and "RUN AWAY!". | **Caught**, about 0.8 s after the scared face appeared. He was holding a left tilt toward the door and could not reverse in time. |
| 2 | A4 | THE CHAIR? | Had just rolled out of the chair, next to it. | "?" on the chair. The sign read "FIND COVER ->", pointing to the table ([r6y](pumpkin-hold-playtest-7/r6y.png)). | **Got away.** Kept rolling right, hid at the table, and the beam checked the chair ([st37](pumpkin-hold-playtest-7/st37.png)). |
| 3 | A5 | THE CHAIR? | Rolling toward the chair for the last candy. | No warning before he arrived. A scared face once he was in the chair zone, and the door glowing next to it ([s10h](pumpkin-hold-playtest-7/s10h.png)). | **Got away**, but by going on through to the door, not by seeing the danger beforehand. |
| 4 | A6 | THE TABLE? (bubble read "RUN AWAY!" first) | In the table zone. | Scared face, "?" on the table, pointing, sign "<- FIND COVER" ([s11w](pumpkin-hold-playtest-7/s11w.png)). | **Got away.** Tilted harder left, reached the chair and hid ([st66](pumpkin-hold-playtest-7/st66.png)). |
| 5 | B4 | THE TABLE? (bubble read "RUN AWAY!" first) | In the table zone with all candy. | Scared face. The sign first read "FIND COVER ->", pointing back toward Grandma, then "<- FIND COVER" 0.4 s later ([b/b7o](pumpkin-hold-playtest-7/b/b7o.png), [b/sb33](pumpkin-hold-playtest-7/b/sb33.png)). | **Got away**, out the door. |

In total there were 5 named-spot events, 4 on `8dcb4bd` and 1 on `a29a817`. He got away 4 times and was caught
once. On `a29a817` the event happened only once in four rounds; that is a gap.

## Answers to the brief's questions

**1. Does he see the danger without reading, and get away?**

- **Spot he is in:** yes. The pumpkin's scared face (big round eyes, "O" mouth), the "?" bubble on the furniture
  and Grandma's pointing arm say "she's looking here" without words. Events 4 and 5 were escapes in under 1 s.
- **Spot he is next to:** the "?" bubble and the pointing arm are the only cue. In event 2 he got away, helped by
  already rolling the right way and by the sign's arrow.
- **Spot he is rolling toward:** no, not before he arrives. Nothing on the pumpkin or the floor changes until he
  enters the named zone. In events 1 and 5 he was heading for the door, which is the stronger attraction. Event 1
  cost a whole three-candy run: the beam was already lit when he entered, and he was caught within about 0.8 s.
  Event 3 ended well only because he kept going.
- See finding M1.

**2. With all the candy, does the sign's arrow lead him to the exit?**

- Not reliably. The door lights up with its own left arrow and the speech bubble says "TO THE DOOR!", and the
  glowing door is what led Leo out in all 9 wins.
- The sign's arrow points to the nearest hiding spot and flips when he passes the middle between two spots. With
  all the candy collected it pointed right, away from the door, several times: [r5f in st26](pumpkin-hold-playtest-7/st26.png)
  in A3, r8p ([st48](pumpkin-hold-playtest-7/st48.png)), s13a ([st74](pumpkin-hold-playtest-7/st74.png)), and
  [b/b7o](pumpkin-hold-playtest-7/b/b7o.png) on the new build.
- In B3 the right-pointing arrow made sense, because the beam was sitting on the door ([b/sb23](pumpkin-hold-playtest-7/b/sb23.png)).
- Leo ignored the sign whenever the door was glowing. See finding m2.

**3. Does hiding by holding the sign still work, and do the arrows say "not here, go that way"?**

- Holding the sign works on both builds. The sign turns orange and reads "HIDING", the pumpkin squashes under the
  chair, table or pot, and the sweep passes over it.
- It held up under thumb drift off the sign ([st23](pumpkin-hold-playtest-7/st23.png)), a press on the sign's
  corner, a quick lift and re-press, and a 6.4 s resting hold. It also worked after Grandma had already started,
  up to about 1.5 s after "QUICK! HIDE!" (A10), but not when he pressed only once the beam was on him (B2).
- A press on the rug just above the sign correctly did nothing ([st80](pumpkin-hold-playtest-7/st80.png)).
- Lifting at a natural 2.5 s while the beam was on his spot got him caught after about 1.2 s (A2). In A4 a
  worried re-press within about 0.8 s saved him. The rule is fair and he learned it after one loss.
- Off a spot, the dark "FIND COVER" sign carries an arrow, and pressing it shows a big ">>>"
  ([st61](pumpkin-hold-playtest-7/st61.png)). That reads as "not here, go that way". Leo followed it in A6.

**4. Does anything feel broken, sticky or confusing? Would he play again?**

- Nothing broke. There were no stuck screens, no stray activations after carried touches, and recalibration
  worked.
- Sticky or confusing points are findings m1-m3 and n1-n4 below.
- **Would he play again: yes, probably.** Rounds are short, the pie is funny ("OH MY! PIE!", "YUM YUM!"), TRY
  AGAIN is one big button, and the first win came in round 4, about 1.5 minutes in.

## Qualitative assessment

**Goal and actions from the screen.**
- The title screen gives a non-reader a pumpkin, candies on the floor and one big light button.
- On the calibration page Leo pressed the only button without reading "HOLD IT COMFY AND STILL". It still worked,
  because he was already holding the board steady.
- Tilting to roll was learned straight away from the pumpkin's movement. The candy shelf at the top fills, with
  sparkles, as he collects.
- Hiding was not obvious at first: he only found "HOLD TO HIDE" in A2, after one loss, because the sign turned
  bright at the chair. After two losses he understood that light means danger and that holding the sign keeps
  him safe.

**Decisions, skill and feedback.**
- There are real choices: grab the candy now or hide, which spot to use, and whether to dash for the door while
  Grandma comes in.
- Pictures carry most of it: the light pool, the beam, Grandma, the scared face, the glowing door, the orange
  "HIDING" sign and ">>>". Reading is never required, though the speech bubbles add flavour he can't use.
- The losses felt fair after A1. Recovery is instant: a pie, then TRY AGAIN.

**Repetition and stopping point.**
- Grandma's visit is nearly always the same: "GRANDMA!", then "HMM?", then a sweep from right to left over about
  5 s, then "PHEW!" or "HIDDEN!".
- He held the sign 4-6 s per visit, two or three visits a round, with the same three candy spots each time.
- Grandma often came back 1.2-1.6 s after "HIDDEN!" (A4, A9, B3). That keeps tension up but adds more holding.
- Once Leo learned to dash for the door the moment Grandma entered, he won every time (A4, A5, A8-A10, B1, B3, B4).
- First natural stopping point: the first win (A4) is satisfying. Leo would most likely stop on his own after
  about A6, the third win in a row and roughly 2.5-3 minutes of play, because rounds start to feel the same. I
  kept playing A7-A10 and B1-B4 for coverage; that is not evidence he would want to.
- He would come back another day for the pie and the dash.

**Recovery from imprecise input, wrong guesses and interruptions.**
- Everything recovered without help from an adult. Off-centre taps on PLAY AGAIN worked, dragging off START
  cancelled cleanly, and carried fingers across screen changes did nothing.
- A curious press on the top "picture" leaves the game and loses the round (m3). The game keeps no save, so
  reopening starts again from the title screen.

## Findings (ranked)

No Blocker or Major findings.

### M1 (Minor, comprehension): rolling toward a named spot gives no warning until he is inside it

- **Screen:** play (`8dcb4bd`; seen once more in B4 on `a29a817`).
- **Issue:** after "THE TABLE?" or "THE CHAIR?", the only cue is a small "?" bubble on the furniture and Grandma's
  pointing arm. The pumpkin's scared face and the beam appear only once he enters the zone, and if the check is
  already lit he is caught within about 0.8 s.
- **Concern:** comprehension, fairness.
- **Repro:** A3 in [`trace.txt`](pumpkin-hold-playtest-7/trace.txt), game time 72.4-76.3 s. Collect all candy at
  the pot. When "THE TABLE?" appears, tilt about 0.35 g left and hold it toward the door.
- **Screenshot:** [`r5b.png`](pumpkin-hold-playtest-7/r5b.png), [`big_r5ghi.png`](pumpkin-hold-playtest-7/big_r5ghi.png), [`st27.png`](pumpkin-hold-playtest-7/st27.png).
- **Suggestion:** from the moment she names a spot, light or tint that spot's floor zone, using the "light =
  danger" language the sweeps already taught. Or show the scared face when the pumpkin is heading into the named
  zone.
- **Why only Minor:** 4 of 5 events ended in escape, and a loss is only a pie and a one-tap retry.

### m2 (Minor, consistency): with all candy, the sign's arrow can point away from the exit

- **Screen:** play, after the third candy.
- **Issue:** the arrow points to the nearest cover and flips at the midpoints between spots, so it sometimes
  points away from the glowing door, and toward Grandma in B4.
- **Concern:** consistency, comprehension.
- **Repro:** A6 in `trace.txt`, about 170.8-171.6 s: all candy at the pot, tilt left, and the sign reads "FIND
  COVER ->". On `a29a817`, B4 in `b/trace.txt` at about 115.3 s.
- **Screenshot:** [`st74.png`](pumpkin-hold-playtest-7/st74.png), [`st48.png`](pumpkin-hold-playtest-7/st48.png), [`b/b7o.png`](pumpkin-hold-playtest-7/b/b7o.png).
- **Suggestion:** with all candy collected and no beam between him and the door, point the arrow at the door.
- **Player impact:** small, because Leo follows the glowing door.

### m3 (Minor, recovery and feedback): presses that do nothing look the same as presses that work

- **Screen:** pie (loss), title and win screens.
- **Issue:** the buttons have no pressed look while held (frame diff of START and TRY AGAIN with a finger down
  against the idle frame: identical). A press that begins as the pie appears, in the first ~0.4 s, is silently
  ignored. Leo, who presses for 1-3 s, gets no "I'm being pressed" signal; in A3 he had to press TRY AGAIN a
  second time.
- **Concern:** feedback.
- **Repro:** `trace.txt` about 78.8 s: press TRY AGAIN as the pie appears; nothing happens. A second press at 80.3 s
  works. QA timing: a press 0.8 s after the pie appears works ([st111](pumpkin-hold-playtest-7/st111.png)).
- **Screenshot:** [`st28.png`](pumpkin-hold-playtest-7/st28.png), [`st29.png`](pumpkin-hold-playtest-7/st29.png), [`st113.png`](pumpkin-hold-playtest-7/st113.png).
- **Note:** the pie screen is a big orange picture, so pressing early is likely. The guard itself is right, since
  it protected carried presses in B2.

### m4 (Minor, recovery): the exit looks like a picture on the wall, and a firm press on it ends the round

- **Screen:** play.
- **Issue:** the exit is a framed back-arrow at the top of the wall. A quick tap does nothing (good), but Leo's
  ordinary firm press exits to Home within about 0.8 s, and the round's candy is lost.
- **Concern:** recovery.
- **Repro:** A7 in `trace.txt`, about 195.0-197.2 s.
- **Screenshot:** [`st84.png`](pumpkin-hold-playtest-7/st84.png).
- **Player impact:** low. Rounds are short and reopening takes two taps. It is still a way for a curious
  6-year-old to quit by accident; consider a visible fill as the hold starts, if one isn't already shown (a pale
  fill appeared at 0.4 s).

### n1 (Nit, consistency): win screen keeps stale action

- **Screen:** win.
- **Issue:** the flashlight beam ([`s21n.png`](pumpkin-hold-playtest-7/s21n.png)), and the "?" bubble with
  Grandma still pointing ([`st59.png`](pumpkin-hold-playtest-7/st59.png), [`b/sb35.png`](pumpkin-hold-playtest-7/b/sb35.png)),
  stay on the victory scene. The "?" bubble sits over the pumpkin.

### n2 (Nit, overlap): banner covers Grandma's face

- **Screen:** win.
- **Issue:** the "AND ALL THE CANDY" banner covers Grandma's glasses and face ([`r8w.png`](pumpkin-hold-playtest-7/r8w.png)).

### n3 (Nit, feedback): the win is quieter than the loss

- **Screen:** win and loss.
- **Issue:** a loss is a full-screen orange pie; a win is the room with a text banner, a candy pile and a little
  confetti. A non-reader may find losing more spectacular than winning.

### n4 (Nit, feedback): the closed door gives no sign it is closed

- **Screen:** play.
- **Issue:** with fewer than 3 candies, rolling into the door just stops the pumpkin against it
  ([`st79.png`](pumpkin-hold-playtest-7/st79.png)). No "need candy" picture or shelf flash.

### Observation (not a defect): the ghost

- On `a29a817`, a press on the ghost during play makes it go dark, and Grandma comes to check a sparkle by the pot
  ("WHAT WAS THAT?") ([`b/sc1.png`](pumpkin-hold-playtest-7/b/sc1.png)).
- To a 6-year-old this probably reads as "the ghost calls Grandma", not as a decoy he can use. I did not explore
  it further. On the title screen the ghost does nothing.

## Gesture and interruption matrix

| Case | Result |
| --- | --- |
| Firm 1-1.5 s press on START / READY / TRY AGAIN / PLAY AGAIN | Works on lift |
| Off-centre press near a button edge (PLAY AGAIN at x=50) | Works |
| Drag off START before lifting | Cancels |
| Press as the pie appears (fresh-screen guard) | Ignored, with no feedback (m3) |
| Hold the sign 2-6.4 s, thumb sliding off the sign, corner press, quick lift and re-press | Hiding holds |
| Press on the rug above the sign | No hide (correct) |
| Press the dark sign off a spot | ">>>" toward cover |
| Hide press carried into the pie screen and lifted over TRY AGAIN | No retry (correct) |
| Quick tap on the exit picture | Nothing |
| Firm press on the exit picture | Home within about 0.8 s; finger carried onto Home's house does nothing |
| Leave mid-round and reopen | Title screen, round lost (the game keeps no save) |
| New grip, then recalibrate | Works |
| Over-tilt at 0.8 g | Rolls at a bounded speed and stops at the walls or door |
| Process restart / reboot | **Gap:** not tested (the game keeps no save) |
| Device lying flat or dropped | **Gap:** not tested |
| Daily cap or rest timer during play | **Gap:** not tested |

## Supplementary automated suite

`make -C apps/porthole playtest` ran at `a29a817`, after the rebuild.

- **95 of 95 scenarios pass**, with 0 UI FAIL and 0 UI WARN in every scenario. That includes the Pumpkin
  scenarios 70-79 (win, loss, tilt, monkey, navigation, ghost, rustle, hold, own spot).
- I read the report (`apps/porthole/build/playtest/report.md`, 198 lines) in full. It has no failures or findings
  section entries.
- The suite does not cover the comprehension issues above.

## Remaining limitations and gaps

- Emulator roleplay only, with a deterministic default clock: no real child and no physical board. The tilt is
  synthetic, without hand tremor.
- Session B on `a29a817` saw only **one** named-spot event, in four rounds. That falls short of the brief's "at
  least twice" for the final build. Four events were seen on `8dcb4bd`.
- First-play comprehension evidence is from `8dcb4bd`. Session B was a replay by an agent that already knew the
  game.
- Not tested: process restart, flat or dropped device, shell rest and daily cap during a round, evening tints,
  and the ghost's decoy strategy.
- My reaction model was 0.4-1.2 s at 400 ms resolution, slightly faster than the persona's 1-2 s, except in A1
  and A2.

## Verdict

**Acceptable**, with minor fixes recommended (M1 first, then m2 and m3).

- A non-reading 6-year-old can understand, control and enjoy this: he finds hiding after one loss, follows the
  glowing door out, and loses gently.
- The main weakness is reading danger when he is rolling toward a spot Grandma has named.
- Pacing becomes repetitive after about 6 rounds.
- Because the rules changed during the session, a fresh check of named-spot events on the final build is
  advisable.

## Comparison with playtest 6

Read after the report above was written.

See [`pumpkin-hold-playtest-6.md`](pumpkin-hold-playtest-6.md).

- **Report 6 M1 (Major): no cue on an unhidden pumpkin in a named spot, caught 3 of 3.** Largely fixed on both
  builds I played.
  - The unhidden pumpkin now shows the scared face inside the named spot (A3, A5, A6, B4), and "RUN AWAY!" often
    comes first.
  - Leo got away 4 of 5 times.
  - Still missing: a cue *before* he enters a spot he is rolling toward (my M1).
- **Report 6's other M1 parts.**
  - The arrows still ignore the open door once all three candies are in (my m2).
  - Spot checks still sometimes come without the "GRANDMA!" warning: in A6 and B4 her first bubble was "RUN AWAY!".
- **Report 6's lesser findings.**
  - "Grandma returns quickly" and "no pressed state" are still present (my pacing notes and m3).
  - Report 6's m1, the pumpkin stalling short of the door, was not reproduced: every door dash got out within about
    1.2 s of leaving the chair.
- **Spots named.** In my rounds the plant pot was never named; only the table and the chair were.
- **Verdict change:** from report 6's "needs fixes" to "acceptable with minor fixes". The caveat is that only one
  named-spot event was observed on the final `a29a817`.
