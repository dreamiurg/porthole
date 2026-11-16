# Pumpkin Hide & Seek, hold-to-hide: child-perspective black-box playtest (Leo, 6, and Zoe, 8)

Independent emulator roleplay. **This is not a study with real children, and it was not run on a physical
device.** Every observation comes from the headless host simulator (`snap --serve`) and its rendered 480x480
frames.

## Build and setup

- Revision: `088bf6c` (clean tree). Executable: a private copy of `apps/porthole/build/host/snap`,
  sha256 `e28928e34c525e462f858523c1ae0a4c47da68411050e8e382e638ed48d3ba03`. `make -C apps/porthole snap` was
  up to date, and the hash was unchanged after `make playtest`.
- Run: `snap --serve --fresh` with the default clock (Tuesday 16:00), from a private scratch folder. A small
  socket broker sent only `down`/`move`/`up`/`tick MS`/`tilt X Y Z`/`frame`. The broker and client are saved as
  `pumpkin-hold-playtest/traces/driver-*.txt`. In the traces, `tap X Y` is a macro: `down`, `tick 120`, `up`.
  `[t=...]` is simulated ms. One fresh process per persona: `traces/leo-serve-trace.txt` and
  `traces/zoe-serve-trace.txt`.
- Tilt: Leo's neutral grip was gravity (0, 0.70) g in the glass plane. His steering tilts were +-0.30-0.40 g.
  Zoe's neutral was (0, 0.57) g, steering +-0.25-0.70 g. Each calibrated once, on the first READY.
- Profiles were created through the screens (Leo: frog, age 6. Zoe: cat, age 8. No code), and the Pumpkin
  ("Spooky") tile was opened from the launcher. During play I used no `dbg`, `app`, clock skips or hidden-state
  queries. The supplementary checks below used script mode; they are labeled.
- The trace times are reproducible inputs. I did not replay a whole trace in a second fresh process to compare
  pixels, so I make no frame-exact replay claim.

### Disclosure

Before and during play I read `apps/porthole/docs/playtesting.md`, the `playtest` and `tour` skills, and lines
40-100 of `apps/porthole/tools/webemu.py` (only for the `--serve` protocol). Two files loaded automatically:
the root `CLAUDE.md`, which says to grab three candies and escape while Grandma searches with a flashlight,
and `apps/porthole/CLAUDE.md`. The second describes the mechanic under test: "Pumpkin's ghost instead acts on
the lift of any press, and its action sign hides the pumpkin exactly while the finger that pressed it stays
down, wherever it drifts; both ignore a press that began before the round did". It also mentions "three hiding
spots", "sweeps, turn-backs, spot checks" and a "frosting-to-pie retry sequence". So as the tester I knew in
advance that hiding is hold-based and survives drift. The personas' decisions were still taken from the screen.
Leo's choices in particular were limited to what a non-reader could see. I did not read game, os, shell or host
source, `tests/playtests/`, `docs/superpowers/` or any AGENTS.md. After all play and the automated suite, I
read `apps/porthole/docs/pumpkin-hide-playtest.md` for the comparison at the end.

## Personas and brief

- **A. Leo, 6:** can't read, tilts with both hands, presses firmly for 1-3 s, double-taps when worried,
  sometimes rests a thumb on the glass.
- **B. Zoe, 8:** a confident reader who taps quickly (about 120 ms) and likes to work out the rules.

Both are simulated perspectives only.

Dispatch brief, verbatim:

> You are a fresh, independent child-perspective playtester for Pumpkin Hide & Seek on Porthole, a round
> 480x480 touchscreen with no buttons, emulated on the host. Follow apps/porthole/docs/playtesting.md and the
> `playtest` skill. This is a changed core mechanic, so play SOURCE-FREE and first-play: don't read source
> (apps/porthole/games/, os/, shell/, host/), tests/playtests scripts, docs/superpowers/, AGENTS.md files or
> earlier playtest reports until you finish. Disclose anything that loaded automatically. Decide from the
> screen. Run the sim from your own copy of the snap binary in a private scratch subfolder.
>
> ## Personas (two passes)
> A. Leo, 6, can't read, tilts with both hands, presses firmly for 1-3 s, double-taps when worried, sometimes
> rests a thumb on the glass.
> B. Zoe, 8, a confident reader who taps quickly and likes to work out the rules.
> Simulated perspectives only.
>
> ## Setup
> - `make -C apps/porthole snap`. Record `git rev-parse --short HEAD` and `sha256sum` of the snap you run.
> - `snap --serve --fresh`, default clock. Create the profile (age 6 or 8, no code) through the screens and
>   open the Pumpkin game ("Spooky") from the launcher. Ordinary touches and tilt inputs only; no dbg,
>   shortcuts or clock skips.
>
> ## Scope
> Play at least four rounds per persona, as each would.
> - Does each figure out how to hide in time, from the screen alone? How long does it take?
> - Is hiding fair when the light comes? Do double taps, long presses and a resting thumb do what the kid
>   expects?
> - Can they collect all the candy and escape?
> - Is the ghost useful, and can they use it when they want to?
> - Where do they get frustrated or bored, and would they play again?
> Also record their win/loss counts.
>
> ## Output
> Write apps/porthole/docs/pumpkin-hold-playtest.md, with screenshots and the input trace in
> apps/porthole/docs/pumpkin-hold-playtest/ (no .py files; save scripts as .txt). Include the revision and
> hash, the personas, this brief, win/loss per persona, findings ranked with repro steps, the qualitative
> assessment and gaps. Afterwards you may read apps/porthole/docs/pumpkin-hide-playtest.md and compare. Don't
> fix code or commit. Reply with ONLY (under 8 lines): a verdict (acceptable / needs fixes), win/loss per
> persona, the top findings, and the report path.

The brief was not revised during this run.

## Outcomes

| Persona | Rounds | Wins | Losses | Round lengths |
| --- | --- | --- | --- | --- |
| Leo, 6 | 5 | 1 (R4) | 4 (R1, R2, R3, R5) | 6 s, 16 s, 6 s, 29 s (win), 8 s |
| Zoe, 8 | 5 | 4 (R1, R2, R4, R5) | 1 (R3) | 22 s, 20 s, 16 s, 9 s, 19 s |

Leo's R5 was a deliberate probe (holding the sign in the open), but it is something he would plausibly do.
Every other round was played as the persona would.

Shots are in `pumpkin-hold-playtest/shots/` (`leo_*`, `zoe_*`). Per-round contact sheets are in
`pumpkin-hold-playtest/sheets/` (`leo_r1.png` ... `zoe_r4_r5.png`, every frame I captured, labeled).

## What Leo did (first play)

1. **Launcher to title (t=8-11 s).** He pressed the jack-o'-lantern tile for 1.5 s, and the caption read
   "Spooky" (`leo_a06_holding_tile`). The title shows the pumpkin, three candy corns, "TILT TO ROLL / START /
   GET CANDY", and an arrow-in-a-picture-frame at the top (`leo_a07_opened`). He can read none of it. The big
   cream sign is the obvious thing to press. Calibrate ("HOLD IT COMFY AND STILL", READY) needed one more firm
   press (`leo_a12_2s`).
2. **Round 1 (t=16.7-22.3 s): lost, 1 candy, never hid.** The play sign said HOLD TO HIDE (`leo_a17_r4`). He
   poked the candy beside the pumpkin, which did nothing, then tipped the board toward it. The pumpkin rolled,
   the candy sparkled into the wall shelf, and Grandma walked in at once ("GRANDMA!", `leo_a18_tapcandy`). The
   sign changed between FIND COVER and HOLD TO HIDE as he rolled, but to him it was the same box. "HMM?",
   "QUICK! HIDE!", the beam, then "PEEKABOO!" about 2.7 s after Grandma appeared (`leo_a22`, `leo_a23`,
   `leo_a25`), an orange icing wipe and "YOU LOST" / "SWEET PUMPKIN!" over a jack-o'-lantern pie (`leo_a31`).
   He had no idea he was supposed to do anything.
3. **Round 2 (t=28.7-46.9 s): lost with all three candies.** TRY AGAIN went straight into play (no
   calibrate). Because the big sign had been the "go" button on every screen, he pressed it again and held it
   for about 2 s. It turned yellow (FIND COVER, pressed), his right tilt rolled the pumpkin into the plant pot,
   the sign read HIDING, and the pumpkin tucked into the pot: "HIDDEN!" (`leo_b02_press05`,
   `leo_b03_press15`). When Grandma came he held again, 2.4 s ("HMM?", "PHEW!", `leo_b10`, `leo_b11`), let go
   while the beam was still on and "PHEW!" still showed, and got away because the beam was swinging the other
   way (`leo_b12`). A worried double tap had no visible effect, which did no harm (`leo_b14`). At the table he
   held a full 3 s, the beam passed over him, and he let go during "HMM?". "QUICK! HIDE!" followed, then
   "THERE YOU ARE!" 0.9 s after release, with the lit door already open behind him (`leo_b33`-`leo_b35`), then
   pie (`leo_b39`). This loss hurts: he had every candy and had pressed the right thing, but not for long
   enough. At most visits Grandma searches for 3.5-6 s, longer than Leo's 1-3 s press.
4. **Round 3 (t=49.6-56.6 s): lost, 2 candies.** His left thumb rested on the floor area of the glass while he
   tilted. That did nothing, which is good (`leo_c05.png`, `leo_c10.png`; sheet `leo_r3`). When Grandma came, the
   resting thumb slid onto the sign without lifting. The sign did not light up and the pumpkin stayed out at
   the pot (`leo_c11_slid`, `leo_c12`): "THERE YOU ARE!" (`leo_c13`), pie.
5. **First voluntary stopping point: after round 3** (about 30 s of actual play, three quick losses). A
   non-reader with no pictorial hint of "hold until she leaves" would now ask a grown-up "how do I hide?" or
   drift off. The rounds are short and the pie is funny, so one more try is plausible, and I continued.
6. **Round 4 (t=59.7-89.6 s): won.** Curious about the ghost in the window, he pressed it for 1.5 s. On lift it
   went dark, a mug flew off the shelf and smashed, Grandma came in ("WHAT WAS THAT?"), looked at the mess and
   left (`leo_c18_ghostdown`, `leo_c19`, `leo_c22`). That is clear and funny, but he spent it with nobody
   chasing him. Then he held HIDE at the pot through the whole visit, about 5.5 s, letting go only when
   Grandma had walked out (`leo_c27`-`leo_c41`). She came back about 1 s later (`leo_c46`, `leo_c48`). He held
   again under the table (3.5 s), rolled left for the last candy, the door lit up with an arrow, and he
   escaped as Grandma arrived: "FREEDOM! AND ALL THE CANDY" (`leo_c64`, `leo_c66`, `leo_c68_win`). This was the
   positive case. By round 4 he had learned "hold until Grandma is gone", from the picture of Grandma standing
   there, not from any words. A natural stopping point.
7. **Round 5 (t=89.6-97.6 s): lost.** The pumpkin stopped in the open between the table and pot, and the sign
   read FIND COVER (`leo_d03`). When Grandma came he pressed and held it. **It turned yellow and pressed-looking
   exactly as it did when hiding** (`leo_d04`, `leo_d05`), but the pumpkin stayed in the open. He held through
   "HMM?" and "QUICK! HIDE!" and was found with his finger still down ("FOUND YOU!", `leo_d11`, `leo_d12`).
8. **Leaving.** One tap on the arrow picture above "YOU LOST" went straight to the launcher (`leo_d15_lost`,
   `leo_d16_after_back`). Reopening showed the title again (`leo_d18_reopen`). In play, a short tap on the arrow
   did nothing (`leo_d21_tapframe`), and a hold of under a second left (`leo_d22_hold04`, `leo_d23_hold10`).
   The still-down finger did nothing on the launcher.

## What Zoe did (first play)

1. **Title (t=9 s).** She read "TILT TO ROLL ... GET CANDY", START, then "HOLD IT COMFY AND STILL", READY, and
   in play "HOLD TO HIDE" (`zoe_z03_title`, `zoe_z04_play`). Before Grandma ever appeared her rule was "roll to
   candy, hold the sign to hide". A quick 120 ms tap on the sign was a blink of a hide with no consequence.
2. **Round 1 (t=10.8-32.9 s): won.** She found the pumpkin slow to steer (about 2-3 s to cross half the rug at
   a 0.45-0.7 g tilt). Grandma came about 4 s in. She reached the pot just in time and held:
   "HIDING"/"HIDDEN", "HMM?", "PHEW!" (`zoe_z14`, `zoe_z15`, `zoe_z19`). "PHEW!" appeared mid-search, then
   "HMM?" again as the beam swung back (`zoe_z22`). She read it as "she missed me, but she's still looking". To
   test the ghost she let go and tapped it during the sweep. The beam went off, the mug landed right beside the
   pumpkin, Grandma said "WHAT WAS THAT?" and left within about 1.2 s without spotting the exposed pumpkin
   (`zoe_z23`, `zoe_z25`, `zoe_z27`). Rule learned: the ghost makes Grandma leave, once (it stays dark). A
   candy hidden behind the pumpkin and Grandma's shoe confused her for a moment (the shelf said 1, but she
   could see no candy on the floor). It was there, under the sprites (`zoe_z14`, `zoe_z30`). At "THE CHAIR?"
   (a "?" bubble over the chair, `zoe_z36`) she hid under the table instead and was safe through the spot
   check and the sweep that followed (`zoe_z38`, `zoe_z44`). Grandma left. She rolled to the door as Grandma
   came straight back, with the beam on her ("QUICK! HIDE!"), and reached the door: "YOU DID IT! GRANDMA IS
   PUZZLED" (`zoe_z58`, `zoe_z59`).
3. **Round 2 (t=34.9-54.9 s): won.** She used the ghost the moment Grandma appeared, and Grandma left in about
   1.2 s (`zoe_y08`, `zoe_y10`, `zoe_y11`). "TO THE DOOR!" came with the third candy (`zoe_y15`). She tapped the
   dark ghost again: nothing, no response (`zoe_y20`). She hid at the pot through a full sweep, then made a
   long slow roll to the door (`zoe_y43`).
4. **Round 3 (t=56.3-72.7 s): lost.** She tested waiting: she pressed only when "QUICK! HIDE!" was already on
   the pumpkin (about 0.9 s into the beam) and was still safe (`zoe_x09`-`zoe_x12`), so there is a fair grace
   period. Her finger slid 15 px off the sign while hiding, and the pumpkin stayed hidden, which is good
   (`zoe_x17_drift`, `zoe_x20`). Then she made the plausible wrong choice: at "THE TABLE?" (Grandma pointing,
   "?" over the table) she was already at the table, and the sign said HOLD TO HIDE, so she hid there
   (`zoe_x27`, `zoe_x28`). After about 3 s, "RUN AWAY!" appeared while the sign still said HIDING (`zoe_x35`,
   `zoe_x36`), then "GOTCHA DEAR!" and pie ("YUM YUM!", `zoe_x37`, `zoe_x41_lost`). She understood why: "she
   said the table". Her first quick TRY AGAIN tap was ignored (`zoe_w00`). The second worked (`zoe_w02`).
5. **Round 4 (t=77.3-86.5 s): won in 9 s.** Ghost as soon as Grandma came (`zoe_w08`), all three candies,
   straight to the door (`zoe_w17`).
6. **Round 5 (t=87.9-106.5 s): won.** She tried holding the sign all the time while tilting. The pumpkin stays
   put when hidden, and tilting shakes the hiding spot: "SHH!", then "SHE HEARD YOU!" with a "!" over Grandma,
   who comes and sweeps (`zoe_v01`, `zoe_v02`, `zoe_v05`). The hidden pumpkin was still safe. After Grandma
   left, the bubble still said "SHE HEARD YOU!" (`zoe_v14`). She also learned she can roll past in the open
   during a spot check of a different place ("THE TABLE?" while she crossed the rug, `zoe_v23`, `zoe_v24`), and
   she escaped: "YOU ESCAPED! WITH THE CANDY" (`zoe_v26`).
7. **Zoe's stopping point: after round 5** (about 1.5 min of play). Every round had the same room and the same
   three candy spots (left of the table, right of the table, right of the pot), and the ghost made Grandma's
   first visit a non-event. Round 4 took 9 s. She worked out every rule by round 3 and would happily play again
   another day, but she would ask for "a harder one" now.

## Qualitative assessment

**Can each persona recognize the goal and actions and tell what changed?** Zoe, yes, entirely from the words
within round 1. "HOLD TO HIDE" before Grandma ever shows, "QUICK! HIDE!", "PHEW!", "THE TABLE?", "RUN AWAY!",
"TO THE DOOR!" and the ghost's "WHAT WAS THAT?" teach every rule, and she hid correctly at her first Grandma
visit, about 10 s into play. Leo, partly and slowly. Pictures give him tilt-to-roll (the pumpkin moves), candy
(the sparkle and shelf counter), the threat (Grandma and her yellow beam), success (the pumpkin tucked into
the pot or under the table, the yellow pressed sign), the ghost (the mug crash) and the exit (the door lit with
an arrow). Nothing pictorial says "press and keep holding while Grandma is here". He found the sign only
because it was the "go" button on every earlier screen (round 2, about 25 s of play). He learned "hold until
she's gone" by round 4, after about 1.5 min and three losses. The sign itself is the weak point for him:
FIND COVER and HOLD TO HIDE look identical, and both turn yellow when pressed, so he cannot tell "this will hide
me" from "this does nothing here" (Major M1).

**Interesting decisions, fair challenge, useful feedback?** The hold mechanic fixes the earlier report's
worst problems. The pumpkin hides the instant the press lands, stays hidden however long and wherever the
finger drifts, and a double tap is just two short hides, so it never toggles out. Pressing late under the beam
(about 0.9 s into "QUICK! HIDE!") is still forgiven. For Zoe there are real choices: which spot, whether to
spend the ghost now or save it, whether to cross the rug during a spot check, and when to dash for the door.
Her one loss (hiding where Grandma pointed) was understood and fair. For Leo the duration is the friction:
visits run 3.5-6 s, and up to about 8 s for spot check plus sweep. "PHEW!" appears mid-search when the beam
has merely passed, which invites a release, and Grandma sometimes comes back about a second after leaving. His
round-2 loss came after holding for 3 s at the right spot. The ghost is useful and easy to use when Zoe
wants it. Holding hide and pressing the ghost at once was impossible in this single-touch sim, so using it
means coming out first (a gap on hardware). Leo used it at a random moment, which summoned Grandma for a look
and wasted it.

**Where does play become repetitive, passive, confusing or frustrating? When would each stop?** Leo: the
frustration is in rounds 1-3 (about 30 s of play, three losses with no idea why). He would most likely stop
after round 3 or ask for help. If he pushes on, round 4's win is a satisfying stop. Holding still for 5 s while
Grandma searches is tense rather than boring. Zoe: no frustration. The boredom risk is sameness: identical
candy positions every round, one room, and a ghost that cancels the first visit, so rounds shrink to 9-20 s.
She would stop after about five rounds, satisfied, but would want new layouts or harder Grandmas.

**Can imprecise input, a wrong guess or an interruption be recovered from?** Mostly yes. A resting thumb on
the scenery does nothing, off-sign drift keeps the hide, quick taps and double taps are harmless, TRY AGAIN
and PLAY AGAIN take long presses, and you can always leave (a tap on the arrow outside play, a hold of under a
second in play). Losing costs only a short round. The exceptions: a resting thumb that slides onto the sign
does not hide and gives no sign why (m1), and a held FIND COVER looks like hiding but isn't (M1). "YOU LOST"
plus the pie is cartoonish, but it is the one harsh phrase on screen (m6).

## Findings (ranked)

### Major

**M1. FIND COVER and HOLD TO HIDE look the same, and both light up when pressed. A non-reader holding "hide"
in the open is caught while he thinks he is hiding.**
- **Screen**: `pumpkin_play`.
- **Issue**: Away from a hiding spot, pressing the sign turns it yellow exactly as a real hide does (FIND
  COVER, pressed), but nothing hides and nothing says why. Leo held it through "HMM?" and "QUICK! HIDE!" and
  was found with his finger down.
- **Concern**: comprehension / feedback (false feedback for the non-reading persona).
- **Repro**: `traces/leo-serve-trace.txt`. Round 5 starts at line 378 (`[t=89640] down 80 137`, TRY AGAIN).
  At line 393 (t=93240) the pumpkin is in the open and the sign says FIND COVER. Hold `down 80 137` from
  line 394 (1.8 s), then again from line 406 (2.5 s). Caught at about t=97.6 s.
- **Screenshot**: `pumpkin-hold-playtest/shots/leo_d03.png`, `leo_d04.png`, `leo_d05.png`, `leo_d11.png`,
  `leo_d12.png`; compare a real hide, `leo_b03_press15.png`.
- **Suggested direction** (owner decides): a picture state, not words. For example, the sign shows a pot or
  table icon and is dimmed or shakes, with the pumpkin wobbling "no!", when there is no cover. The pressed
  highlight should appear only when a hide actually happens. A pictorial nudge toward the nearest spot (an
  arrow or a glow on the furniture) would also help.

### Minor

**m1. Visits outlast a 6-year-old's press, and "PHEW!" mid-search invites letting go.** Grandma's visits ran
3.5-6 s (about 8 s with a spot check first). "PHEW!" appears whenever the beam passes, then "HMM?" again as it
swings back. Leo let go after 3 s during the search and was caught 0.9 s later. The rule "keep holding while
Grandma is in the room" is visible (she stands there), and Leo learned it by round 4, so this is a learning
curve rather than a wall. Consider a clearer "still here / all clear" cue, such as no "PHEW!" until she turns
to leave, or a visible "she's leaving" beat. Repro: `leo-serve-trace.txt` line 179 (`[t=43040] down 70 142`,
held 3 s, released), then "QUICK! HIDE!" and `THERE YOU ARE!` by t≈46.9 s (line 196). Shots: `leo_b28.png`,
`leo_b33.png`, `leo_b34.png`, `leo_b35.png`. Concern: pacing / comprehension.

**m2. A thumb that slides onto the sign does not hide, with no feedback.** Leo's resting thumb on the scenery
(correctly ignored) slid onto the sign when Grandma came. The sign stayed unlit, the pumpkin stayed out, and he
was caught. This is probably intentional (it guards against a resting hand), but the kid only sees "I'm
touching hide and it doesn't work". Lifting and pressing again would have worked. Repro: `leo-serve-trace.txt`
line 224 (`[t=52640] down 28 122`), tilt right, then line 241 (`move 75 138`, no lift). Caught at about
t=56.6 s. Shots: `leo_c10.png`, `leo_c11_slid.png`, `leo_c12.png`, `leo_c13.png`. Concern: feedback /
recovery. Hardware may differ. See gaps.

**m3. Hiding in the spot Grandma names is a reading-only rule.** "THE TABLE?" plus a "?" bubble, then "RUN
AWAY!" while the sign still says HIDING, then caught. Zoe read it and found it fair. Leo never met a spot
check of his own spot, so this is untested for a non-reader, but he would have no cue except the small "?"
bubble, and the HIDING sign contradicts the warning. Repro: `zoe-serve-trace.txt` line 341
(`[t=68670] down 80 137` at the table during "THE TABLE?"), held. Caught at about t=72.7 s. Shots:
`zoe_x27.png`, `zoe_x28.png`, `zoe_x35.png`, `zoe_x36.png`, `zoe_x37.png`. Concern: comprehension (non-reader).

**m4. Rustle: tilting while hidden calls Grandma, explained only in words.** "SHH!", then "SHE HEARD YOU!",
summons a sweep. A small wobble mark shows at the spot. Leo tilts constantly with both hands, so he will
summon extra visits without knowing why. It didn't cost Zoe the round, since the hidden pumpkin stays safe.
Repro: `zoe-serve-trace.txt` lines 424-430 (`[t=87870] down 80 137` at the table, then tilt -0.5 g and
+0.6 g while held). Shots:
`zoe_v01.png`, `zoe_v02.png`, `zoe_v05.png`. Concern: comprehension / pacing.

**m5. Repetition: same candy spots every round, and the ghost trivializes the first visit.** All ten rounds
across both personas used the same three candy positions. With the ghost spent on Grandma's first entrance,
Zoe's round 4 took 9 s. Fine for a few rounds, thin for an 8-year-old beyond about five. Repro: compare
`sheets/leo_r1.png`, `leo_r3.png`, `zoe_r2.png`, `zoe_r4_r5.png` (first frames). Concern: boredom /
meaningful choice.

**m6. "YOU LOST" plus the baked-pumpkin pie (unchanged from the previous report's m5).** The pie captions vary
("SWEET PUMPKIN!", "OH MY! PIE!", "PIE TIME!", "YUM YUM!"), but the large "YOU LOST" is the one harsh phrase
in a game whose audience rule is "failure is gentle". Shots: `leo_a31.png`, `zoe_x41_lost.png`. Concern:
comprehension / tone.

**m7. Grandma sometimes returns about 0.5-1 s after leaving.** Seen in Leo's round 4 (`leo_c46.png`,
`leo_c48.png`) and Zoe's round 1 (`zoe_z58.png`). Combined with the slow pumpkin (about 2-3 s across half
the rug), the dash to the door often meets her again. Zoe enjoyed the near miss. For Leo it means a second
5 s hold right after the first. Concern: pacing / challenge.

### Nit

- **n1.** The bubble stays stale after Grandma leaves ("HIDDEN!" or "SHE HEARD YOU!" with nobody there,
  `zoe_v14.png`, `leo_c41.png`), so there is no "all clear" cue. Concern: feedback.
- **n2.** A candy can sit hidden under the pumpkin or Grandma's shoe beside the pot (`zoe_z14.png`). The shelf
  says fewer than she can see on the floor. Concern: feedback.
- **n3.** The dark (used) ghost ignores taps silently (`zoe_y20.png`), and nothing says whether it comes back.
  Concern: feedback.
- **n4.** TRY AGAIN looks live but ignores taps for about 0.8 s after the pie card appears (Zoe's first tap,
  `zoe_x41_lost.png`, `zoe_w00.png`). Supplementary timing in `traces/supp-tryagain-guard-script.txt`: taps
  at +100/+300 ms land during the icing wipe, +500/+700 ms on the pie are ignored, +900 ms on start a round.
  Concern: feedback.
- **n5.** The win card's "GRANDMA IS PUZZLED!" box covers Grandma's face. The pumpkin is shown back at the
  table rather than out the door (`zoe_z59.png`, `leo_c68_win.png`). Concern: overlap / feedback.
- **n6.** The pumpkin feels sluggish to a quick 8-year-old (`sheets/zoe_r2.png`, y33-y42: about 4 s pot to
  door at 0.7 g). Fine for Leo. Concern: pacing.
- **n7.** The exit is a framed arrow picture, not the orange home button used elsewhere. In play, a short tap
  does nothing silently and a hold of under a second leaves (`leo_d21_tapframe.png`, `leo_d22_hold04.png`).
  Concern: consistency (carried over from the previous report's n2).

### Positives worth keeping

- The hide happens on press, holds through drift, and never toggles on a double tap. There is a late-press
  grace under the beam.
- The yellow HIDING sign and the pumpkin tucked into the furniture are clear feedback when it works.
- A resting thumb on the scenery is ignored.
- The ghost's mug crash is delightful cause and effect, and it ends a search in about 1.2 s.
- The door arrow cue is clear, the candy shelf counter works, and there is always a way out.

## Gesture and interruption matrix

| Gesture / case | Persona | Result |
| --- | --- | --- |
| Hold HIDE at a spot, 1.5-5.5 s | Leo R2, R4; Zoe R1-R5 | hides on press, stays hidden while held |
| Hold released mid-search (3 s) | Leo R2 | caught 0.9 s later: m1 |
| Hold at a spot Grandma names | Zoe R3 | "RUN AWAY!", caught: m3 |
| Press only after "QUICK! HIDE!" (about 0.9 s in) | Zoe R3 | safe (grace) |
| Quick 120 ms tap on HIDE | Zoe R1 | blink-hide, harmless |
| Double tap on HIDE | Leo R2 | harmless, no toggle |
| Hold FIND COVER in the open | Leo R2 (while rolling to cover), R5 | hides on reaching cover / caught in the open: M1 |
| Finger drifts 15 px off the sign while hiding | Zoe R3 | still hidden |
| Thumb resting on scenery | Leo R3 | ignored |
| Thumb slides from scenery onto the sign | Leo R3 | no hide, no feedback: m2 |
| Tilt while hiding | Zoe R5 | "SHH!", "SHE HEARD YOU!", sweep: m4 |
| Ghost long press (1.5 s) / quick tap | Leo R4 / Zoe R1, R2, R4 | acts on lift, Grandma leaves about 1.2 s later |
| Dark ghost tap | Zoe R2 | nothing: n3 |
| Hold HIDE and tap the ghost at once | - | **gap**: single-touch sim |
| START / READY / TRY AGAIN held 1-1.5 s | Leo | act on lift |
| TRY AGAIN quick tap right after the pie | Zoe | ignored about 0.8 s: n4 |
| Arrow picture: tap on lost / title; tap and hold in play | Leo | leaves / leaves; play tap ignored, hold of under 1 s leaves: n7 |
| Leave mid-round and reopen | Leo | title, fresh round (no saved state) |
| Changing grip between rounds (TRY AGAIN skips calibrate) | - | **gap** |
| Process restart mid-round | - | **gap** (no persisted state, low risk) |
| Session rest / daily cap mid-round, night tint, shake, flat-on-table grip | - | **gap** |
| Physical board, real multi-touch, real children | - | **gap**: emulator roleplay only |

## Supplementary QA (not part of the black-box pass)

- `make -C apps/porthole playtest` at `088bf6c`: **94 of 94 scenarios pass, 0 UI FAIL, 0 UI WARN.** That
  includes `70_pumpkin_win`, `71_pumpkin_loss`, `72_pumpkin_tilt`, `73_pumpkin_kid_taps`,
  `74_pumpkin_monkey` (ASan/UBSan), `75_pumpkin_navigation`, `76_pumpkin_ghost`, `77_pumpkin_rustle` and
  `78_pumpkin_hold`. I read `build/playtest/report.md` in full; its UI findings table is empty. I did not open
  the scenario files.
- An ad hoc `ui` audit (`traces/supp-ui-audit-script.txt`, script mode with `profile`) of `pumpkin_intro`,
  `pumpkin_calibrate`, `pumpkin_play` (out, hiding, ghost used) and `pumpkin_lost` measured the sign at
  80x24, the arrow at 24x22 and the ghost at 24x24 (its region is removed once used). All text was within the
  glass: the lowest, "GET CANDY", has a worst corner 77.8 px from the center. All text was cream or dark on a
  dark or cream box. No finding.
- No new scenario was added. A regression scenario for M1 would need the expected fix's behavior defined
  first.

## Comparison with the previous report (`pumpkin-hide-playtest.md`, `d9a3f90`)

- **Fixed:** old M1 (hide acted on lift, so a 1-3 s press left the pumpkin exposed) and old M2 (a double tap
  hid and un-hid). Now the pumpkin hides on press, stays hidden while held, and double taps are harmless. Old
  m1 (lifting a resting thumb toggles) is gone in its old form. A resting thumb on the sign now simply hides.
  Old m2 (no pressed look) is fixed: the sign turns yellow while held.
- **Changed:** the risk moved from "pressing too long" to "not holding long enough" (m1 here) and to the
  pressed look being shown even when no hide happens (M1 here, the same root as old m2's "FIND COVER does
  nothing silently", now worse because it looks like success).
- **Unchanged:** "YOU LOST" plus pie (old m5, here m6), the win card not picturing the escape (old n3, here
  n5), the arrow-picture exit (old n2, here n7), and Grandma's quick returns (old m4, here m7).
- **Outcomes:** the previous Leo went 1 win, 4 losses in 7 rounds and would stop after round 3. This Leo went
  1 win, 4 losses in 5 rounds, but the cause of his losses shifted from the button betraying a correct press
  to not knowing how long to hold or where hiding works. He learned the fix himself by round 4.

## Iteration history and verdict

- Iteration 1 (this run, `088bf6c`, snap `e28928e3...`): the findings above. No game changes have been made
  since, so there is no retest.
- **Verdict: needs fixes.** M1 remains Major for the non-reading 6-year-old: the sign's pressed look reports
  a hide that isn't happening, and he loses while doing what he believes is right. m1 (hold length and
  "PHEW!") and m3/m4 (reading-only rules) are the next most important for Leo. For Zoe the hold mechanic is
  fair, understandable and fun, with repetition (m5) the main limit. After a fix, a fresh independent agent
  should replay with a non-reader persona: holds in the open, a spot check on his own spot, constant tilting
  while hidden, and the slide-on thumb.
- **Suggested brief improvements:**
  - Add a non-reader probe for each text-only rule: Grandma naming the kid's own spot, and tilting while
    hidden. Random seeds may not produce these in four rounds, so ask the tester to play until each has
    happened once or to report it as a gap.
  - Ask for the hold duration needed per Grandma visit, measured from frames.
  - Note that the sim is single-touch, so "hold hide and use the ghost" can only be judged on hardware.
