# Pets Club farm and shelter: child playtest, round 5

This is emulator roleplay by an independent agent. It is not a study with a real child and not a test on the physical device.

## Build and setup

- Revision `f02cf5b` ("fix(porthole): Pets Club farm buttons work on a long press, a barn on the question"), clean tree.
- `apps/porthole/build/host/snap` sha256 `4a784082d96aa3e9922f27e8d3c733c8bc0f89bc84defb3f3d5ef4519bb95d95`. A forced rebuild (`make -B snap`) and `make playtest` both left the hash unchanged.
- Fixture: `.superpowers/sdd/2026-11-08-pets-club-shelter/fixture/*.sav`, copied over `apps/porthole/build/host/` before runs 1, 2 and 3. The hashes are in `shelter-playtest-5/fixture-sha256.txt` and match round 4: crago-s0 `a048df7b...`, porthole-m/mb `4bf5122f...`, porthole-p0 `845f86e0...`, sys-ran `ef260e9a...`.
- The profile is "Mia" ("Mia, age 8"). The dog is Biscuit, "Grown dog - day 31", with 0 books, 0 tricks and a 1-day streak. Roleplay treats the profile as Nora's own.
- Harness: `build/host/snap --serve` without `--fresh`, from `apps/porthole/`, at the default clock.
  - A small daemon (`shelter-playtest-5/serve-driver.txt`, saved as .txt) owns the process and appends every input line it receives to `shelter-playtest-5/trace.txt`.
  - It sends only `down X Y` / `move X Y` / `up` / `tick MS` in logical px, plus `frame` for native 480x480 screenshots (`shot NAME` in the trace is a saved frame).
  - `quit` closes stdin, which restarts the process; the saves are kept.
- During play I used no `dbg`, `app`, `skip`, `ui`, `debug` or `screen` commands.

### Disclosure

- I read no game, os, shell or host source, no `tests/playtests` scripts and nothing under `docs/superpowers/`. Under `.superpowers/` I read only the fixture directory.
- To learn the serve wire protocol I read `apps/porthole/tools/webemu.py`: `tick`, `frame`, `down`/`move`/`up`.
- **Leak before play:** running in the repo auto-loaded `apps/porthole/CLAUDE.md`. It says that Pets Club's farm Yes uses "a longer hold", that the long press is 600 ms, that `FreshGate` is 450 ms, and that the farm move is checkpointed within a second. It gives no hold length. Every timing below was measured from frames.
- I read the earlier reports (`shelter-playtest*.md`) only after play, while writing this report.
- **My own mistake, kept in the trace:** in run 1 I read button positions off down-scaled thumbnails. I pressed "Go home" at logical y=91, which is Biscuit's body; the button is at y≈137. For about a minute I believed the farm was a dead end. That cost a process restart (run 1b). Run 2 shows that Go home works. This was my mis-tap, not a game bug.

## Persona and brief (verbatim)

> You are an independent child-perspective playtester for Pets Club, a dog-care game on Porthole: a round 480x480 touchscreen with no buttons, emulated on the host. Follow apps/porthole/docs/playtesting.md and the `playtest` skill. Play source-free: don't read game source (apps/porthole/games/, os/, shell/, host/), the tests/playtests scripts, docs/superpowers/ or .superpowers/ (except the fixture dir). Decide from the screen. Read the earlier reports in apps/porthole/docs/pets-club/ only after you finish playing.
>
> ## Persona
> Nora, 6, can't read yet, presses firmly and often too long, taps again when something seems slow, and loves her dog Biscuit. Her 9-year-old brother sometimes plays on her profile and reads the words out. Simulated perspective only.
>
> ## Setup (disclosed fixture)
> - `make -C apps/porthole snap`. Record `git rev-parse --short HEAD` and `sha256sum apps/porthole/build/host/snap`.
> - Before each fresh start, copy /home/user/porthole/.superpowers/sdd/2026-11-08-pets-club-shelter/fixture/*.sav into apps/porthole/build/host/ (overwrite). The profile is "Mia" in the picker; treat it as Nora's own. Biscuit is 30 days old.
> - Run `apps/porthole/build/host/snap --serve` WITHOUT --fresh, from apps/porthole/, at the default clock. Use ordinary touches only (taps, presses of different lengths, slides, rubs). No dbg, app shortcuts, clock skips or hidden-state queries.
>
> ## Scope
> Everything new around Biscuit, played as Nora would, plus the brother's deliberate run:
> 1. The question about sending Biscuit away: every button there pressed short, medium (0.7-1.5 s) and long (2+ s). Does any button ever seem broken? Does Biscuit ever leave without someone meaning it? Can a non-reader tell from the picture what Yes does?
> 2. The brother deliberately sends Biscuit to the farm. Then the farm: pat, rub and hold Biscuit, and Go home pressed long.
> 3. The empty room and the shelter: long presses on every button and arrow, picking a pup, keeping or changing its name.
> 4. Later: finding Biscuit on the wall, visiting, rubbing, and getting back, with long presses on everything.
> Also the brief's imprecise-finger probes on these screens.
>
> ## Output
> Write apps/porthole/docs/pets-club/shelter-playtest-5.md, with screenshots and the input trace in apps/porthole/docs/pets-club/shelter-playtest-5/ (no .py files; save scripts as .txt). Include the revision and hash, the persona and this brief, the fixture, findings ranked with repro steps, the qualitative assessment and gaps. Don't fix code or commit. Reply with ONLY (under 12 lines): a verdict (acceptable / needs fixes), the top findings ranked, and the report path.

There were no brief revisions.

## Runs

| Run | Start | What it covered | Outcome |
| --- | --- | --- | --- |
| 1 | Fresh fixture | Nora alone: room, tricks, Stats, the question (every button short, medium and long, plus mash and slides); her 2.5 s press on Yes; the farm | **Biscuit left on Nora's 2.5 s Yes press.** I then mis-tapped Go home (see Disclosure). |
| 1b | Restart, saves kept | Empty room, shelter, wall, visit, naming (keyboard long presses), welcome, the room with the new pup, then the wall and visit again | Adopted "Mochi" the corgi. Biscuit is on the wall and can be visited. |
| 2 | Fresh fixture | The brother: a deliberate hold measured frame by frame; farm pat, rub and hold; offset taps; Go home long; empty room; leave and resume; shelter medium presses; keyboard back-out; renaming to "Rex" | Clean, complete flow |
| 3, 3b-3d | Fresh fixture, then restarts | Restart during a 1.6 s Yes hold; restart 200 ms after the commit; restart from the name keyboard | Biscuit stays / the move is kept / an empty room with the Shelter sign |

All PNGs are native 480x480 frames in `apps/porthole/docs/pets-club/shelter-playtest-5/`, named `r<run>_<nn>_<what>.png`. `sheet_*.png` are 2/3-scale contact sheets.

## First play: Nora on her own (run 1)

1. **Picker, launcher, room.** Nora taps the dog-face "Mia" row. In the launcher she picks the orange cartoon dog; the finger shows a "Pets Club" caption (`r1_02`). She taps through the splash. Biscuit stands on the rug (`r1_04`).
   - She pats him: hearts and a jump (`r1_05`). Happy.
2. **Exploring.** The paw button opens tricks; she backs out with the orange home button. The little bars at the top open Stats (`r1_09`).
   - She can read only "Biscuit". The big light-blue button is the most tappable thing on the card, so she taps it. It reads "To the farm", which she can't read.
3. **The question** (`r1_11`): dark blue, white and yellow text, a bright green **No!**, a brown **Yes**, yellow "Hold Yes", a sitting Biscuit and a small red barn.
   - She taps the dog picture and the barn. Neither reacts, which is fine (`r1_12`, `r1_13`).
   - A quick tap on Yes: the button sinks, "Hold Yes" flashes white, and nothing happens (`r1_14`).
4. **Holding Yes** (`sheet_yes_hold_commit.png`, `r1_16`-`r1_19`, `r2_04`-`r2_08`):
   - A plum fill sweeps across Yes, and Biscuit stands and walks toward the barn.
   - Releasing at 1.48 s empties the fill and sits Biscuit back down (`r1_20`, `r1_21`).
   - Nora: "Biscuit is walking to the red house!" To a pre-reader the barn reads as a house or doghouse. Nothing pictures "for good" or "a new puppy comes".
5. **Nora's long press (2.5 s) on Yes** (`r1_45`-`r1_51`): at about 1.9 s, with her finger still down, the farm appears: "Biscuit lives here now!", a fence, a sign and a big green Go home.
   - Her release does nothing there (no carried tap).
   - **This is Biscuit leaving on Nora's ordinary long press.** It is her habit, not a decision she understood (M1).
6. **Farm:** she pats, rubs and holds Biscuit. Every one gets hearts and a tongue (`r2_12`-`r2_14`). This is the best moment of the flow for her.
7. **Go home** leads to an empty room. There is no dog on the rug, Biscuit's little framed picture is on the wall, and a big plum "Shelter" sign covers the rug (`r2_22`).
   - Nora: "Where's Biscuit?"

**First voluntary stopping point (Nora alone):** on the farm, after patting Biscuit for a while. If she presses Go home, the empty room is a sad moment. She would either stop, or tap the only big thing, the Shelter sign, because it is there.

- Inside the shelter, the pups in the kennel and the arrows carry her along. She looked at all seven.
- She would pick the corgi. The naming keyboard means nothing to her (she can't read). Her long press on green OK does nothing (m1), so she taps again, gets the welcome card, and taps once more.
- Her natural end is the new pup on the rug (`r1b_70`). It is a satisfying close, but she got there by accident.
- Everything after this point was coverage.

## The brother's deliberate run (run 2)

- He reads "Move Biscuit to the farm? Biscuit will live there. Then pick a new pup." and "Hold Yes".
- He holds Yes and watches the fill. It is about 95% full at 1.8 s, and the farm appears at about 1.9 s (`r2_06`, `r2_07`).
- On the farm he pats, rubs and holds Biscuit: hearts every time (`r2_12`-`r2_15`).
- Taps between the dog and Go home (80,122), and left of Go home (44,137), do nothing (`r2_16`, `r2_17`).
- **Go home pressed for 2.5 s:** the button sinks while held, and on release he gets the empty room (`r2_18`-`r2_22`). This is round 4's M1, and it is fixed.
- Empty room, then the home button to the launcher, then Pets Club again: still the empty room with Shelter (`r2_25`-`r2_27`).
- In the shelter: arrows held 1.0, 0.7 and 1.5 s each step once on release. Pick me! held 1 s opens the keyboard. The keyboard's home **tapped** goes back to the shelter with the same pup (`r2_33`). He picks Choco.
- He clears "Choco" with six short backspaces. The box shows "Pup's name" and OK greys out (`r2_36`). He types R, E, X across the two pages (`r2_38`), taps OK, and gets "Welcome home, Rex!" (`r2_39`). A tap brings the room with Rex the beagle and Biscuit still on the wall (`r2_40`).
- Rex's Stats: "Puppy - day 1", with the bars and no farm button (`r2_41`). Farm friends still shows Biscuit (`r2_42`).

## Gesture and interruption matrix

Short is under 0.3 s, medium is 0.7-1.5 s, long is 2.5 s.

| Screen / control | Short | Medium | Long | Offset / near miss | Slide | Mash / double | Evidence |
| --- | --- | --- | --- | --- | --- | --- | --- |
| Stats: To the farm | opens the question | gap | 1.5 s opens on release | gap | gap | gap | `r1_33` |
| Question: No! | to Stats | 1.0 s: sinks, to Stats on release | 2.5 s: to Stats | gap | Yes→No release: No looks pressed, **does nothing** (m2) | n/a | `r1_22`-`r1_32`, `r1_40`, `r1_41` |
| Question: Yes | "Hold Yes" flashes; stays | 0.36-1.48 s fill, then cancel on release | **leaves at about 1.9 s** (M1) | dog / barn taps inert | off cancels; No→Yes never fills | 6 × 0.4 s presses: no build-up | `r1_14`-`r1_21`, `r1_34`-`r1_51`, `r2_04`-`r2_08` |
| Farm: Biscuit | hearts + jump | rub: hearts | hold 2.5 s: hearts | sign and post inert | rub works | gap | `r2_12`-`r2_15` |
| Farm: Go home | gap (round 4 covered) | 1.0 s: sunk | 2.5 s: to empty room | above / left: inert | gap | gap | `r2_16`-`r2_22` |
| Empty room: Shelter | works | 1.0 s: sunk | 2.5 s: opens on release | gap | gap | gap | `r1b_06`-`r1b_09` |
| Empty room: ball, bowl | inert | inert | inert | n/a | n/a | n/a | `r1b_03`-`r1b_05` |
| Room: Biscuit's photo | opens Farm friends | **1.0 s: nothing** | **2.5 s: nothing** (m3) | gap | gap | gap | `r2_23`, `r2_24`, `r1b_71`-`r1b_76` |
| Shelter: arrows | step | 0.7 / 1.0 / 1.5 s: one step on release, no feedback while held | 2.5 s: one step on release | y=67 (above the drawn circle) still hits | gap | 7 taps wrap around | `r1b_10`-`r1b_15`, `r2_28`-`r2_30` |
| Shelter: Pick me! | keyboard | 1.0 s: keyboard | 2.5 s: sunk, keyboard on release | gap | gap | gap | `r1b_38`-`r1b_41`, `r2_31` |
| Shelter: home | to the room | gap | 2.5 s: to the room | n/a | n/a | n/a | `r1b_18`, `r1b_19` |
| Shelter: pup in the kennel | inert | n/a | n/a | n/a | n/a | n/a | `r1b_16` |
| Keyboard: letters | type (0.5 s still types) | **0.75 / 1.0 / 1.5 s: nothing** | nothing | gap | gap | gap | `r1b_51`, `r1b_54`-`r1b_57` |
| Keyboard: <, N-Z | delete / page | gap | **turns yellow, nothing** | gap | gap | gap | `r1b_49`, `r1b_50`, `r1b_52` |
| Keyboard: ? | random name (Noodle, Cocoa, Mochi) | gap | **nothing** | gap | gap | gap | `r1b_42`-`r1b_48` |
| Keyboard: OK | welcome | gap | **turns yellow, nothing** (m1) | gap | gap | gap | `r1b_58`-`r1b_64` |
| Keyboard: home | to the shelter, pup kept | gap | **2.5 s: nothing** | n/a | n/a | n/a | `r2_32`, `r2_33` |
| Welcome card | to the room | gap | **2.5 s: nothing** ("tap" hides while held) | n/a | n/a | n/a | `r1b_66`-`r1b_70` |
| Farm friends: Biscuit's frame | visit | 0.9 s: works | 2.5 s: visit on release | gap | gap | gap | `r1b_77`-`r1b_79` |
| Visit: Biscuit | hearts | hold 1.2 s: hearts | gap | sign post inert | rub: hearts | gap | `r1b_80`-`r1b_82` |
| Visit / Farm friends: home | back | gap | 2.5 s: back | gap | gap | gap | `r1b_26`-`r1b_28`, `r1b_83`, `r1b_84` |

| Interruption | Result | Evidence |
| --- | --- | --- |
| Ordinary leave: empty room → launcher → reopen | Empty room kept | `r2_25`-`r2_27` |
| Ordinary leave: shelter home / keyboard home | Back one level, the pup choice kept | `r1b_19`, `r2_33` |
| Process restart while holding Yes (1.6 s) | Biscuit still home | `r3_01`, `r3_02` |
| Process restart 200 ms after the commit | Move kept: empty room with Shelter | `r3_03`, `r3_04` |
| Process restart from the name keyboard | Empty room with Shelter; no half-adopted pup | `r3_05`, `r3_06` |
| Process restart from the farm (run 1 → 1b) | Empty room | `r1b_02` |

Gaps: night or evening tint on these screens; SIGKILL (only stdin-close restarts were tested; round 4 tested SIGKILL); offset taps on No; a 1.85 s Yes hold; the longest names on the wall; mashing on the shelter and keyboard; real touch pressure and timing on the device. The fixture has 0 books and 0 tricks, so "progress is kept across the move" could not be seen beyond the wall and the profile.

## Findings (prioritized)

### Blocker

None.

### Major

**M1. Nora's ordinary long press on Yes sends Biscuit away, and the picture does not tell a non-reader that he is leaving for good**

- **Screen:** `confirm_farm`
- **Issue:** Yes commits at about 1.9 s of continuous press (95% at 1.8 s, the farm at 1.9 s), while the finger is still down.
  - The persona "presses firmly and often too long". Her 2.5 s press on the brown button, which she cannot read, moved Biscuit to the farm.
  - She also had to tap the text-only "To the farm" button first, which she would do out of curiosity because it is the biggest button on Stats.
  - The new picture cue (Biscuit walking toward a small red barn while the fill grows) is a real improvement over round 4. Seen by a 6-year-old, though, the barn reads as "a red house" or "his doghouse". Nothing pictures "he stays there" or "a new pup comes", so the walk looks like a fun trip, not a goodbye.
  - The result is gentle: Biscuit is on the wall and can be visited. But it cannot be undone, and losing the dog she loves from her room by accident is exactly what the confirm exists to prevent.
  - If the parent/coordinator decide that about 1.9 s of visible walking is enough warning, record that decision with this player impact. Otherwise the threshold sits below this persona's habit.
- **Concern:** comprehension / recovery
- **Repro:** fresh fixture; tap (75,68), tap (37,74), wait 1.5 s, tap (80,100), wait 1.5 s, tap the bars (45,25), tap (80,77), then `down 115 101`, ticks for 2.5 s, `up`. Run 1 in `shelter-playtest-5/trace.txt`; timing measured in run 2.
- **Screenshot:** `apps/porthole/docs/pets-club/shelter-playtest-5/r1_45_yes_long_1600.png`, `r1_46_yes_long_1800.png`, `r1_47_yes_long_2000.png`, `sheet_yes_hold_commit.png`, `r2_06_yes_1800.png`, `r2_07_yes_1900.png`
- **Suggestions for the owner and designer (any one helps):**
  - Lengthen the hold well past a habitual long press (for example 3-4 s).
  - Make the hold picture say "leaving": Biscuit goes through the barn door and out of sight, and a new pup appears.
  - Add a picture to the To the farm button.
  - Commit on release after the fill is full rather than mid-press, so a child who sees the barn has a final chance to let go. This changes the "full = go" model the brother used, so it would need a design decision.

### Minor

**m1. The name keyboard ignores every press of about 0.6 s or more: OK, home, ?, <, N-Z and letters**

- **Screen:** `name_pet` (the shared kids' name keyboard)
- **Issue:** A 0.5 s press types a letter; 0.75, 1.0, 1.5 and 2.5 s presses do nothing.
  - OK and < turn yellow while held, then do nothing on release.
  - The home button held 2.5 s does nothing, while every other home button in this flow works on a long press.
  - For Nora, the green OK is the only meaningful key, and her usual press there "seems broken". She recovers by tapping again, which her persona does, so this is friction rather than a block.
  - Round 4 fixed this on the farm buttons, so the flow now trains "hold works" and then stops honouring it on the next screens.
  - This is a carry-over of round 2's m2 (OK ignores a slow press), now seen on every key.
- **Concern:** feedback / consistency
- **Repro:** run 1b: on the keyboard, `down 118 125`, tick 2500, `up` → still the keyboard. `down 41 52`, tick 1000, `up` → no letter. `down 80 12`, tick 2500, `up` → still the keyboard (run 2).
- **Screenshot:** `apps/porthole/docs/pets-club/shelter-playtest-5/sheet_keyboard_long_presses.png`, `r1b_59_ok_hold_2500.png`, `r1b_61_ok_after.png`, `r1b_55_N_750.png`, `r2_32_kb_home_long.png`

**m2. Holding Yes and sliding onto No shows No pressed, but releasing does nothing**

- **Screen:** `confirm_farm`
- **Issue:** A child who changes her mind mid-hold ("no, don't go!") and slides over to No sees No sink under her finger. On release nothing happens, and she stays on the question. It is safe (the Yes fill cancels), but the safe answer looks broken at the moment it matters. A second tap on No works.
- **Concern:** feedback / recovery
- **Repro:** on the question: `down 115 101`, tick 800, `move 80 101`, `move 45 101`, tick 300, `up` → still the question.
- **Screenshot:** `apps/porthole/docs/pets-club/shelter-playtest-5/sheet_slide_yes_to_no.png`, `r1_40_yes_slid_to_no.png`, `r1_41_yes_slid_to_no_release.png`

**m3. Biscuit's photo on the room wall ignores presses of about 0.6 s or more (carried over from round 3's m2)**

- **Screen:** `home` (the empty room and the room with a pup)
- **Issue:** Presses of 0.9, 1.0 and 2.5 s on the photo do nothing; a tap opens Farm friends.
  - The photo is the only way to Nora's dog now. In the room with a pup, a long press slightly low lands on the new pup instead.
  - The Farm friends frame does honour long presses, so the two steps of the same path behave differently.
- **Concern:** feedback / consistency
- **Repro:** empty room: `down 53 53`, tick 2500, `up` → still the room; then `down 53 53`, tick 150, `up` → Farm friends.
- **Screenshot:** `apps/porthole/docs/pets-club/shelter-playtest-5/sheet_photo_long_press.png`, `r2_23_empty_photo_long.png`, `r1b_72_photo_hold_release.png`

**m4. The welcome card ignores a long press**

- **Screen:** the adoption welcome ("Welcome home, Mochi!")
- **Issue:** A 2.5 s press does not advance, and the blinking "tap" hint hides while the finger is down. A short tap works. This is the same pattern as m1.
- **Concern:** feedback
- **Repro:** after OK on the keyboard: `down 80 120`, tick 2500, `up` → still the card.
- **Screenshot:** `apps/porthole/docs/pets-club/shelter-playtest-5/r1b_66_welcome_hold.png`, `r1b_68_welcome_after.png`

**m5. Shelter arrows give no feedback while pressed**

- **Screen:** `shelter`
- **Issue:** While an arrow is held (1-2.5 s), the frame is pixel-identical to before the press. The pup changes only on release.
  - It works, but for 2 s Nora sees nothing, which invites a harder re-press.
  - Pick me! and Shelter sink while pressed, so this is also inconsistent.
- **Concern:** feedback
- **Repro:** `down 134 81`, tick 1000, frame (identical to the frame before), `up` → next pup.
- **Screenshot:** `apps/porthole/docs/pets-club/shelter-playtest-5/r1b_10_right_hold_1000.png`, `r1b_12_right_release.png`

**m6. "To the farm" is text only (carried over from round 4's m1, the remaining part)**

- **Screen:** `stats`
- **Issue:** The question now has a barn picture, but the button that leads to it does not, so a pre-reader presses it blind.
- **Concern:** comprehension
- **Repro:** fresh fixture → Stats.
- **Screenshot:** `apps/porthole/docs/pets-club/shelter-playtest-5/r1_09_stats.png`

### Nits

- **n1.** "?" on the keyboard replaces the shelter's name (Waffles) with a random one, and a non-reader can't get the original back. Seen in round 2 as fun; still a mystery button (`r1b_46`-`r1b_48`).
- **n2.** "N-Z" fills its key edge to edge (`r1b_41`). The UI audit passes.
- **n3.** Tapping the pup in the shelter kennel does nothing. A wag or a bark-hop would reward the most attractive thing on the screen (`r1b_16`).
- **n4.** In the empty room, the ball and the bowl are inert to every press. That is reasonable, but a non-reader may not see that the Shelter sign is the next step (`r1b_03`-`r1b_05`).

## Resolved since round 4 (verified on this build)

- **Round 4's M1:** No works at 1.0 and 2.5 s, and Go home works at 2.5 s. **Fixed.** (`r1_25`-`r1_32`, `r2_18`-`r2_22`)
- **Round 4's m1:** a barn now sits on the question and Biscuit walks toward it while Yes fills. **Partly fixed**; see M1 and m6.
- **Round 4's m3:** holding and rubbing Biscuit on the farm and on a visit now give hearts. **Fixed.** (`r2_13`, `r2_14`, `r1b_80`, `r1b_81`)
- Mash, slide-off and No-to-Yes slides stay safe. Restarts mid-hold and right after the commit behave correctly.

## Qualitative assessment

**Can Nora recognize the goal and actions, and tell what changed?**

- The room, patting, the farm, the shelter kennel with arrows and dots, and the welcome all work from pictures. She can browse and pick a pup with no reading, and the hearts make patting clearly rewarding.
- The question and Stats are reading-led. For her, "Yes" is a brown button and the barn is a red house. She can see that something is happening during the hold (the fill and the walking dog), but not that it means "Biscuit leaves your room for good" (M1).
- After the move the change is unmistakable: the farm, then an empty rug with the photo on the wall.

**Decisions, feedback, pictures versus reading.**

- For the brother, the decision is informed and deliberate. The text is clear, and "hold until full" matches what he sees: about 1.9 s with a growing fill.
- For Nora, the only real decision in the flow is which pup, and it is a good one: seven distinct pups, a pleasing wrap-around browse and a prefilled name.
- Naming is reading-only. Nora would accept the suggestion by tapping green OK, which works only on a short press (m1).

**Where it becomes repetitive, confusing or frustrating, and where she stops.**

- **Frustrating:** long presses that do nothing on the keyboard, the photo and the welcome card (m1, m3, m4); arrows that give no feedback while pressed (m5); No that looks pressed after a slide but doesn't act (m2).
- **Confusing:** the question (reading), and the empty room right after (where did Biscuit go?).
- **Stopping points:** farm patting is the warmest moment. The empty room is an emotional low point, but the Shelter sign gives an immediate next step. Her natural end is the new pup on the rug. The whole farm-to-new-pup flow is about 15-20 presses and under 3 minutes, with no waits.

**Recovery from imprecise input, a wrong guess or an interruption.**

- Good: offset taps around Go home and on the scenery are inert. Slide-off cancels Yes, mashing never builds progress, a carried finger after the commit does nothing, and the shelter and keyboard back-outs keep the chosen pup. Restarts at every point tested left a sensible state.
- Weak: Nora's own wrong guess on the question (pressing Yes long) cannot be undone. That is M1.

## Supplementary QA (after play, separate from the black-box pass)

- `make -C apps/porthole playtest` at `f02cf5b`: **90 of 90 scenarios pass, 0 UI FAIL and 0 UI WARN.** `b0_pets_farm_shelter` has 132 checks.
- I read `build/playtest/report.md` in full: the scenario table, the empty UI findings table and the contact-sheet list.
- An ad hoc `ui` dump of the new screens (`shelter-playtest-5/supp_ui_dump.txt`, output in `supp_ui_dump.out.txt`) shows:
  - The question's No and Yes are 50x24 each, 20 px apart.
  - Go home is 64x22, and the farm dog region is 72x54.
  - The empty room's photo is 25x25, and the wall frame is 46x30.
  - The shelter arrows are 32x32, and the keyboard keys are 24x22.
- All of these are at or above the 24x22 floor. The audit cannot see press-duration behaviour, which is what M1 and m1-m5 are about.

## Iteration history

- Rounds 1-4: see `shelter-playtest.md` through `shelter-playtest-4.md`.
- **Round 5** (this report, `f02cf5b`): round 4's M1 and m3 are fixed, and m1 is partly fixed.
  - New: M1 (persona-specific: the 1.9 s threshold sits under a 2+ s habitual press, with an ambiguous picture), m2, m4 and m5.
  - Carried over: m1 (round 2's m2, now on every key), m3 (round 3's m2) and m6.
- I changed no code and committed nothing.

## Verdict

**Needs fixes.**

- M1 should be fixed, or explicitly accepted by the coordinator/parent with the player impact above. A fresh agent should then replay the question with a non-reading long-presser.
- m1-m5 share one cause: presses past about 0.6 s are dropped on the keyboard, the photo and the welcome card, and arrows show nothing while pressed. Making every control in this flow act on release, as the farm buttons now do, would fix them together.
