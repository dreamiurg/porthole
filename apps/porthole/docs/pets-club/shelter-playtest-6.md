# Pets Club farm and shelter: child playtest, round 6

Independent black-box playtest of the farm goodbye, "Bring back", the shelter, the new pup and visiting the old dog. Emulator roleplay only. No real child took part, and nothing ran on the physical device.

## Build and setup

- **Revision:** `0cd6906` ("fix(porthole): Pets Club farm offers Bring back, and long presses work on the shelter's screens"). The working tree was clean.
- **Executable:** `apps/porthole/build/host/snap`, sha256 `e0ac0f9552d35b3c69bc9ee39f7b30bdd2ab51c04cceffd5e5924dddb19d357a`. `make -C apps/porthole snap` reported it up to date.
- **Fixture (disclosed):** before each fresh start I deleted every `build/host/*.sav` and copied `/home/user/porthole/.superpowers/sdd/2026-11-08-pets-club-shelter/fixture/*.sav` into `build/host/`: `crago-s0`, `porthole-m`, `porthole-mb`, `porthole-p0` and `sys-ran`. The picker shows one profile, "Mia" (age 8). Biscuit is a grown dog; Stats says "day 31". He has 0 books, 0 tricks and a 1-day streak.
- **Run mode:** `snap --serve` from `apps/porthole/`, default clock, no `--fresh`. A small driver sent `down` / `tick` / `up` / `move` and grabbed native 480x480 frames (`frame`).
  - It used no `dbg`, `app`, clock skips or `debug` / `screen` queries during play.
  - The driver lives in the session scratch directory. Per the brief, no `.py` file is in the evidence folder.
- **Process restart:** the driver closed the sim's stdin and started a new `snap --serve` on the same saves. A crash or SIGKILL restart was not tested.
- **Source-free:** I read no game, os, shell or host source, no `tests/playtests` scripts and nothing in `docs/superpowers/`. From `.superpowers/` I used only the fixture directory.
  - I read `tools/webemu.py` (dev tooling, not game code), only to learn the `--serve` protocol.
  - I read the earlier shelter reports only after both persona passes, to check what was carried over.
  - One disclosure: the shared scratch directory already held files from earlier sessions. I did not open them and worked in a fresh subfolder.

## Personas and brief (verbatim from the coordinator)

> You are an independent child-perspective playtester for Pets Club, a dog-care game on Porthole: a round 480x480 touchscreen with no buttons, emulated on the host. Follow apps/porthole/docs/playtesting.md and the `playtest` skill. Play source-free: don't read game source (apps/porthole/games/, os/, shell/, host/), the tests/playtests scripts, docs/superpowers/ or .superpowers/ (except the fixture dir). Decide from the screen. Read the earlier reports in apps/porthole/docs/pets-club/ only after you finish playing.
>
> ## Personas (two short passes)
> A. Theo, 6, can't read yet, presses everything long and firmly (often 2-3 s), mashes when unsure, and loves his dog Biscuit. He doesn't mean to send Biscuit anywhere.
> B. Maya, 9, reads well and decides on purpose to send her old dog to the farm and get a new pup, then later visits him.
> Simulated perspectives only.
>
> ## Setup (disclosed fixture)
> - `make -C apps/porthole snap`. Record `git rev-parse --short HEAD` and `sha256sum apps/porthole/build/host/snap`.
> - Before each fresh start, copy /home/user/porthole/.superpowers/sdd/2026-11-08-pets-club-shelter/fixture/*.sav into apps/porthole/build/host/ (overwrite). The profile is "Mia" in the picker; treat it as each persona's own. Biscuit is 30 days old.
> - Run `apps/porthole/build/host/snap --serve` WITHOUT --fresh, from apps/porthole/, at the default clock. Ordinary touches only (taps, presses of every length, slides, rubs, mashing). No dbg, app shortcuts, clock skips or hidden-state queries.
>
> ## Scope
> - Theo: play his way around Biscuit's pages. If Biscuit ends up somewhere Theo didn't want, can Theo (or a grown-up nearby) get him back from what's on screen, before it's too late? What happens if Theo leaves that screen first? Is anything scary or sad?
> - Maya: the deliberate goodbye, the farm, going home, the shelter (try every pup, keep or change a name), the new pup, then finding Biscuit later and visiting him. Is anything confusing, and does the extra choice on the farm get in her way?
> - Both: long presses and offset taps on every new button.
>
> ## Output
> Write apps/porthole/docs/pets-club/shelter-playtest-6.md, with screenshots and the input trace in apps/porthole/docs/pets-club/shelter-playtest-6/ (no .py files; save scripts as .txt). Include the revision and hash, the personas and this brief, the fixture, findings ranked with repro steps, the qualitative assessment for each persona and gaps. Don't fix code or commit. Reply with ONLY (under 12 lines): a verdict (acceptable / needs fixes), the top findings ranked, and the report path.

## Evidence

All under `apps/porthole/docs/pets-club/shelter-playtest-6/`:

- **Native frames:** `t*` (Theo run 1), `u*` (Theo run 2, mistakes), `k*` (restart on the farm), `d*` (shell delete), `m*` / `n*` (Maya runs 1 and 2) and `q*` (re-checks of round 5's findings).
- **Input traces:** `trace-theo.txt` (run 1, then restart and resume), `trace-theo2.txt`, `trace-theo3.txt` (a long press on the bars that missed, the retry, then the restart), `trace-theo4.txt`, `trace-maya.txt` (run 1, then restart and resume), `trace-maya2.txt` and `trace-supp.txt`.
  - Coordinates are logical px (0-159).
  - `press X Y MS` means `down X Y`, `tick MS`, `up`. `tap X Y` means an 80 ms press.
  - `shot NAME` saves the current frame as `NAME.png`.
- **Contact sheets:** `sheet_theo_send_and_bring_back.png`, `sheet_theo_go_home_no_way_back.png`, `sheet_theo_adopts.png`, `sheet_maya_flow.png`, `sheet_shelter_pups.png`, `sheet_offset_taps_farm.png` and `sheet_shell_long_press_delete.png`.
- **Supplementary geometry (after play):** `ui_probe_script.txt` (raw `--script`) and `ui_probe_output.txt`.

## Runs

| Run | Start | What | Outcome |
| --- | --- | --- | --- |
| Theo 1 | fresh fixture | Picker long press, launcher, room, tricks, Paw Street, Stats; "To the farm" 2.5 s; Yes 2.5 s; farm; dog pressed; Bring back 2.5 s | **Biscuit sent by accident, then brought back.** Stats intact (`t33`). |
| Theo 1 (cont.) | same | To the farm again, Yes 2.6 s, **Go home** 2.2 s → empty room → photo → Farm friends → visit → room → launcher and back → restart → shelter → Pick me! → keyboard mash → OK → welcome | **Biscuit cannot be brought back** after Go home. Theo ended up with a pup he renamed "Waffles" with "?". |
| Theo 2 | fresh fixture | No pressed 2.5 s; 12-tap mash on Yes; Yes held 1.0 / 1.5 / 2.0 s; hold 1.8 s then slide off; offset taps around Bring back; Bring back at its left edge; Yes 6 s; Go home 3 s | No works long. Mashing and slide-off are safe. Yes commits at about 1.9-2.0 s. |
| Theo 3 | fresh fixture | Yes 2.5 s, wait 6 s on the farm, **process restart** | Resumes in the empty room. **Bring back is gone.** |
| Theo 4 | fresh fixture | Shell: long press on "Mia", then a 3 s press on the lower button of "Delete Mia?" | **Profile deleted** (see M2). |
| Maya 1 | fresh fixture | Tap, read and hold Yes; farm; Go home; Shelter; every pup both ways; tap the dots, pup and label; Charcoal, name kept; welcome; Stats; photo; visit; pet and rub; leave; restart; Paw Street; offset taps on the photo | Complete. Charcoal adopted; Biscuit visited before and after the restart. |
| Maya 2 | fresh fixture | Shelter → Choco → keyboard → home (back to Choco) → Pick me! → clear → type "Pepper" → OK → welcome → room → Farm friends | Complete. Renaming works. |
| Supp | fresh fixture | Round 5 re-checks: slide from Yes onto No; shelter arrow held 1 s | See "Round 5 findings re-checked" |

## Theo (6, pre-reader, long firm presses): first play

1. **Picker.** Theo holds his face for 2 s, and "Delete Mia?" opens. He can't read it.
   - His long presses on the green "No, keep!" (1 s and 2.5 s) do nothing (`t02`, `t03`). A short press goes back.
   - A 3 s press on the other button deletes his profile (`d02`; see M2). In run 1 he pressed the green one, so he survived.
2. **Launcher and room.** Long presses on the Pets Club tile, the room's paw button, the stat bars and the tricks page's home button do nothing (`t07`, `t12`, `t14`, `t18`).
   - Theo mashes, and short presses get through. He spends a lot of his first minute pressing things that don't answer.
   - Holding Biscuit makes him sit (`t10`), which is nice.
3. **Stats.** "To the farm" is the biggest button. It is text only, which Theo can't read. His 2.5 s press opens the question (`t21`).
4. **The question.** He sees a green and a brown button and a picture of Biscuit with a red barn. He presses brown for 2.5 s.
   - The button fills and Biscuit walks toward the barn (`t22`-`t24`). At about 2 s, **with his finger still down, Biscuit is on the farm** (`t25`).
   - The fill and the walk look like a game, not a goodbye, so the hold does not stop a child whose normal press is 2-3 s.
5. **The farm.** It is sunny, with a fence, a sign and Biscuit wagging. Nothing is scary.
   - Theo presses Biscuit. A long press and a tap get no reaction (`t28`, `t29`). He wanted to pet him.
   - Two buttons: an orange "Bring back" on the left, a big green "Go home" at the bottom. He can't read either.
6. **Bring back (a grown-up reads it to him).** A 2.5 s press brings Biscuit home: room normal, Stats day 31 (`t31`-`t33`). **The recovery works and is gentle.**
7. **Go home instead.** In a second try he presses the big green button, which is his most likely pick: green meant "keep" and "No!" on the earlier screens. The room is now empty (`t35`):
   - no dog on the rug;
   - a small framed photo of Biscuit on the wall;
   - a big plum "Shelter" sign.
   - This is the sad moment. To a 6-year-old the photo on the wall reads as "Biscuit is gone".
8. **Trying to get him back.** The photo, held 2.5 s, opens "Farm friends" (`t37`), and Biscuit's frame opens the farm visit (`t38`).
   - **The visit has no Bring back,** only the home button.
   - The shelter has no way to Biscuit either. Leaving to the launcher and coming back, or a restart, changes nothing (`t50`, `t52`).
   - From here, neither Theo nor a grown-up reading the screen can get Biscuit back (M1).
9. **Shelter.** Theo presses the arrows long (each press steps once), then "Pick me!" for 2.5 s (`t54`). He sees the keyboard with "Connor_".
   - His long presses on letters, "<" and OK do nothing (`t56`, `t58`, `t66`). Mashed short taps add "aa" to "Connor".
   - "?" swaps in random names: Luna, then Waffles. A short OK gives "Welcome home, Waffles!" (`t70`).
   - A 2.5 s press on the card works (`t74`). The new pup is on the rug.
   - The Golden pup "Connor" looks like a small Biscuit. That may comfort Theo, or confuse him ("Biscuit got small?").

**Where Theo would stop on his own:** in the empty room after Go home (`t35`). His dog is gone and the only big thing is a word he can't read. He would either stop and look for a grown-up, or press "Shelter" because it is there. In the Bring back branch, he would stop happily after Biscuit is back in the room.

## Maya (9, reads well, on purpose): first play

1. **Stats → "To the farm".** The question is clear to her: "Move Biscuit to the farm? Biscuit will live there. Then pick a new pup."
   - A short tap on Yes flashes "Hold Yes" white (`m04`), so she knows to hold.
   - The fill and Biscuit's walk to the barn take about 2 s (`m05`-`m09`), and the farm appears while her finger is still down (`m10`). Clear and calm.
2. **Farm.** "Biscuit lives here now!" She reads "Bring back" and "Go home".
   - She has already decided, so the extra choice does not slow her down. She taps Go home.
   - "Bring back" could make her briefly unsure ("did I do it wrong?"), but nothing on the screen pushes her to second-guess. No friction was seen.
3. **Empty room.** The Shelter sign is unmistakable.
4. **Shelter.** "Pick a pup!" shows seven pups behind the arrows, with a dot row showing where she is. The list wraps both ways (`m13`-`m21`):
   - Connor, Golden pup
   - Boxy, Dalmatian
   - Romeo, Husky
   - Choco, Beagle
   - Marsha, Poodle
   - Waffles, Corgi
   - Charcoal, Black Lab

   Each has a name and breed and a distinct sprite. Tapping the pup, its label or the dots does nothing (`m22`-`m24`). She would like the pup to wag or bark.
5. **Keeping a name.** Charcoal → Pick me! → the keyboard already says "Charcoal" → OK. A quick double tap on OK shows the welcome once and does not skip it (`m26`).
   - Then the room with Charcoal, and Stats as a day-1 puppy, still on a 1-day streak (`m27`, `m28`).
6. **Changing a name (run 2).** On the keyboard, home goes back to the same pup (`n03`).
   - Backspacing empties the field to a grey "Pup's name" placeholder with OK greyed out (`n04`).
   - "Pepper" needs 11 taps because letters are split across A-M and N-Z pages. It works, and the first letter is capitalized automatically (`n07`).
7. **Visiting later.** Biscuit's photo hangs next to the window (it is easy to miss on first look, but its hit area is generous: `m38`-`m41`).
   - The photo opens "Farm friends", and the frame opens the farm, with Biscuit wagging and walking a little.
   - A rub shows little hearts (`m34`). Taps get little reaction.
   - After leaving to the launcher and a process restart, Biscuit is still on the wall and the visit still works (`m36`). Paw Street shows only the new pup at Mia's house (`m37`).

**Where Maya would stop on her own:** right after the welcome card, with Charcoal or Pepper on the rug. That is a satisfying natural stop. She would come back to the visit once or twice, but it is a short, passive look (Biscuit stands and wags; a rub gives hearts), so it is not a reason to stay.

## Gesture and interruption matrix (new or changed controls)

| Control | Tap | ~1 s | 2-3 s | Offset / outside | Double / mash | Slide off | Evidence |
| --- | --- | --- | --- | --- | --- | --- | --- |
| Stats: To the farm | question | gap | question (on release) | gap | gap | gap | `t21`, `u01` |
| Question: No! | Stats | gap | Stats | gap | gap | gap | `u02` |
| Question: Yes (hold) | "Hold Yes" flashes | 1.0 / 1.5 s: no | **2.0 s: farm** | gap | 12-tap mash: no | 1.8 s then off: cancelled | `m04`, `u03`-`u07` |
| Yes held, slid onto No | n/a | n/a | n/a | n/a | n/a | No not shown pressed; release does nothing | `q01`, `q02` |
| Farm: Bring back | n/a (2.5 s tested) | gap | room, Biscuit home | right / below / gap: inert; left edge (15,103): works | gap | gap | `t31`, `u08`-`u11` |
| Farm: Go home | room | gap | room (2.2 s, 3 s) | gap below it | gap | gap | `t35`, `u13`, `m12` |
| Farm: dog | no reaction | n/a | no reaction | n/a | n/a | n/a | `t28`, `t29` |
| Room: photo | Farm friends | gap | Farm friends (2.5 s) | ±10 px all four sides: hits | gap | gap | `t37`, `m38`-`m41` |
| Farm friends: frame | visit | gap | visit (2.5 s) | gap | gap | gap | `t38`, `m30` |
| Visit: home | Farm friends | gap | Farm friends (2 s) | gap | gap | gap | `t40`, `t41` |
| Visit: dog | small tail move | n/a | none | n/a | n/a | rub: hearts | `m32`-`m34` |
| Shelter: arrows | next / previous pup | **no pressed look while held** | one step on release | gap | gap | gap | `t46`, `q03`-`q05` |
| Shelter: Pick me! | keyboard | gap | keyboard | gap | gap | gap | `t54`, `m25` |
| Shelter: home | room | gap | gap | gap | gap | gap | `t48` |
| Keyboard: letters, <, ?, OK | work | **ignored (0.5-1 s)** | **ignored** | gap | double OK: one welcome | gap | `t56`-`t66`, `m26` |
| Keyboard: home | back to the same pup | gap | gap | gap | gap | gap | `n03` |
| Welcome card | room | gap | room (2.5 s) | gap | gap | gap | `t74` |

| Interruption | Result | Evidence |
| --- | --- | --- |
| Leave to the launcher from the empty room, then return | Empty room, Biscuit on the farm | `t49`, `t50` |
| Process restart from the empty room | Same | `t52` |
| **Process restart while on the farm screen** | Empty room. **Bring back is gone.** | `k01`, `k02` |
| Leave to the launcher with the new pup, then restart | New pup in the room; Biscuit on the wall and visitable | `m35`-`m37` |
| Keyboard home | Back to the shelter, same pup | `n03` |

**Gaps:**
- the session rest timer or daily cap firing while the farm screen is up (likely the same as the restart case, not verified);
- SIGKILL restarts;
- evening and night tints;
- several farm and dog-visit Wall entries (the fixture has one);
- double taps on Bring back and Go home;
- dragging off Bring back;
- real touch pressure and timing on the device.

With 0 books and 0 tricks in the fixture, "progress is kept" could only be seen through the streak, the wall and the profile.

## Findings (prioritized)

### Blocker

None.

### Major

**M1. Once Theo leaves the farm screen, nothing on screen can bring Biscuit back, not even for a grown-up**

- **Screen:** `farm` (right after the move), `home` (the empty room), `wall`, the farm visit, `shelter`
- **Issue:** Bring back appears only on the farm screen shown right after the hold.
  - After Go home, a leave and return, or a process restart, the empty room, "Farm friends", the farm visit and the shelter offer no way back.
  - The undo disappears exactly when a pre-reader is most likely to lose it. Go home is the larger green button at the bottom, and on the screens before it, green meant "keep" and "No!".
  - Bring back is orange and text only, so Theo can't tell which one undoes.
  - Theo's ordinary 2-3 s press is what started the move (Yes commits at about 2 s; round 5's M1). Undo is now the only protection for him, and it lasts for one screen.
- **Concern:** recovery / comprehension
- **Repro:**
  1. Fresh fixture: tap the picker (80,67), the Pets Club tile (33,72), then the stat bars (45,27).
  2. Press (80,77) for 2 s, then (115,102) for 2.6 s.
  3. Press Go home (80,137).
  4. Look for a way back: the photo (53,53), the frame (80,104), the visit, the shelter (80,101). There is none.
  5. Restart variant: on the farm, wait 6 s, restart the sim, reopen. The room is empty.
  - Traces: `trace-theo.txt` (from line "press 80 77 2000" onward) and `trace-theo3.txt`.
- **Screenshot:** `apps/porthole/docs/pets-club/shelter-playtest-6/sheet_theo_go_home_no_way_back.png`, `t35_gohome_a.png`, `t38_visit.png`, `k02_after_restart.png`
- **Suggestions (designer's call):**
  - Keep "Bring back" available until a new pup is adopted, for example on the farm visit of the most recent dog while the room is empty, and/or on the empty room's Shelter screen.
  - Give Bring back a picture (Biscuit with an arrow to the house) so a pre-reader can tell it from Go home.
  - Resume onto the farm screen after an interruption until the child picks one of the two buttons.

**M2. (Shell, outside this flow, found on Theo's way in) Two long presses delete the whole profile, while the safe button ignores long presses**

- **Screen:** the shell picker → "Delete Mia?"
- **Issue:** Theo's 2 s press on his face opens "Delete Mia? Their pets and games will be gone."
  - The green "No, keep!" ignores presses of 1 s and 2.5 s. Only a short tap backs out.
  - The lower button reads "Hold to delete". A pre-reader cannot read it, and his ordinary 3 s press deletes the profile: Biscuit, the room, everything. The next frame is the "Your name?" keyboard.
  - No admin code was set in the fixture (a grown-up's code would guard this).
  - Not part of the shelter change, but it is the most destructive thing Theo's habit can do, and it sits one long press from where he starts.
- **Concern:** recovery / consistency
- **Repro:** fresh fixture; `press 80 67 2000`, `tick 1000`, `press 80 115 3000` → "Your name?". Trace: `trace-theo4.txt`.
- **Screenshot:** `apps/porthole/docs/pets-club/shelter-playtest-6/sheet_shell_long_press_delete.png`, `t02_after_keep.png`, `d02_after_hold_delete.png`
- **Route:** shell owner (game-engineer). For example, make "No, keep!" act on release at any length, as the farm buttons now do, and/or lengthen or picture the delete hold.

### Minor

**m1. The name keyboard ignores every press of about 0.5 s or more (carried over from round 5's m1)**

- **Screen:** `name_pet`
- **Issue:** Letters, "<", "?" and OK do nothing at 0.5, 0.65, 1, 2 and 2.5 s; short taps work. Theo is stuck until he mashes.
  - When the field is full at 8 letters, an extra tap is silently dropped too (`t60`-`t62`).
  - The commit title says long presses work on the shelter's screens; the keyboard is the one that still doesn't.
- **Concern:** feedback / consistency
- **Repro:** on the keyboard: `press 120 125 2500` → still the keyboard (`t58`). After one "<": `press 93 53 1000` → no "c" (`t65`).
- **Screenshot:** `apps/porthole/docs/pets-club/shelter-playtest-6/sheet_theo_adopts.png`, `t58_ok_hold.png`, `t66_bs_hold.png`

**m2. "Bring back" and "To the farm" are text only**

- **Screen:** `farm`, `stats`
- **Issue:**
  - "To the farm" (round 5's m6) is still text only, so Theo opens the question blind.
  - The new "Bring back" is also text only, sitting next to a bigger green "Go home". The one button that undoes the move is unreadable to the child who most needs it. This is part of M1's cause; it is listed separately because it is cheap to fix on its own.
- **Concern:** comprehension
- **Repro:** fresh fixture → Stats (`t19`); after Yes → farm (`t25`).
- **Screenshot:** `apps/porthole/docs/pets-club/shelter-playtest-6/t19_stats_tap.png`, `t25_yes_2500.png`

**m3. Shelter arrows show nothing while pressed (carried over from round 5's m5)**

- **Screen:** `shelter`
- **Issue:** A 1 s hold on the right arrow gives a frame pixel-identical to the frame before the press (checked with an image diff). The pup changes only on release. Pick me! sinks while pressed, so the arrows are inconsistent.
- **Concern:** feedback
- **Repro:** `down 134 81`, `tick 1000`, frame; `up` → next pup.
- **Screenshot:** `apps/porthole/docs/pets-club/shelter-playtest-6/q03_shelter_before.png`, `q04_arrow_held.png`, `q05_arrow_release.png`

**m4. A puppy's Stats row is cramped and its icons are cut**

- **Screen:** `stats` (the new pup, day 1)
- **Issue:** On the food / fun / sleep / clean row:
  - the bone icon is clipped at the left, so only "()" shows;
  - the sleep row has no moon icon;
  - the clean drop touches "sleep";
  - "clean" runs into the panel's right border.
  - Maya sees this right after adopting. It may be older than the shelter work; the audit did not flag it because these are drawings and text inside a panel, not targets.
- **Concern:** clipped text / overlap
- **Repro:** adopt any pup → tap the stat bars (45,27).
- **Screenshot:** `apps/porthole/docs/pets-club/shelter-playtest-6/m28_stats_charcoal.png`

**m5. (Pets Club and shell in general) Many ordinary controls ignore long presses**

- **Screen:** launcher tiles, the room's stat bars and paw/food buttons, the tricks page's home button
- **Issue:** Theo's 2-3 s presses do nothing there (`t07`, `t12`, `t14`, `t18`, and `k00`, where a 2 s press on the bars left him in the room). The farm visit's home button does honour a 2 s press (`t40`), so behavior differs from screen to screen.
  - Not new, but it shapes Theo's whole session: he learns that "pressing doesn't work" and mashes. Mashing is then what lands him on the keyboard and welcome screens.
- **Concern:** consistency / feedback
- **Repro:** room: `press 118 128 2200` → nothing (`t12`).
- **Screenshot:** `apps/porthole/docs/pets-club/shelter-playtest-6/t12_paw.png`, `t14_home_hold.png`

### Nits

- **n1.** On the farm (right after the move), Biscuit ignores taps and long presses (`t28`, `t29`). Theo's first wish there is to pet him. The visit later reacts a little: a small tail move and hearts on a rub.
- **n2.** The welcome card's top corners are cut by the round edge (`t70`). Text is inside.
- **n3.** Shelter pups don't react to a tap (`m23`). A wag or bark would make choosing more fun.
- **n4.** "?" replaces the shelter name with a random one, and a non-reader can't get it back (round 5's n1, still there; `t68`, `t69`).
- **n5.** In the empty room the only clue to Biscuit is a small photo. For Theo it reads like a memorial. Something that says "on the farm" (a tiny barn on the frame, or Biscuit in a field) would soften it.

## Round 5 findings re-checked on this build

| Round 5 | Now |
| --- | --- |
| M1: a 2.5 s Yes press sends Biscuit away | **Partly fixed.** Commit is still about 2 s, but Bring back now undoes it on the farm screen. The undo is lost on Go home or an interruption (this round's M1). |
| m1: keyboard ignores long presses | **Not fixed** (m1) |
| m2: slide from Yes to No shows No pressed, release does nothing | **Fixed.** No no longer looks pressed after a slide, and release is a silent cancel (`q01`, `q02`). |
| m3: photo ignores long presses | **Fixed.** 2.5 s opens Farm friends (`t37`). |
| m4: welcome card ignores long presses | **Fixed.** 2.5 s goes on (`t74`). |
| m5: arrows show nothing while held | **Not fixed** (m3) |
| m6: "To the farm" text only | **Not fixed** (m2) |

## Qualitative assessment

**Can the persona recognize the goal and actions, understand the instructions and see what changed?**

- **Maya: yes, throughout.**
  - The question names the consequence ("will live there", "pick a new pup").
  - "Hold Yes" flashes on a tap, and the fill plus Biscuit walking show progress.
  - Farm, empty room, Shelter, pick, name, welcome and new pup on the rug each make the change visible.
- **Theo: no, for the decision points.**
  - Every button he must choose between is text only.
  - Colors mislead him: green means "keep / No!" on the question and on the delete screen, but "leave for good" on the farm.
  - He can see what changed (Biscuit on a farm, then an empty room), but not why or how to undo it.

**Interesting decisions, consequences, feedback; do pictures carry it?**

- **For Maya,** choosing among seven distinct pups and naming one is a real, pleasant choice. Keeping the shelter name is one tap; renaming works but is long (11 taps for "Pepper").
- **For Theo,** pictures carry the shelter (pups are easy to compare) but not the farm decision or the undo.
- Bring back itself is gentle and instant when used: Biscuit is back on the rug, with day 31 and his stats intact.

**Where is it repetitive, passive, confusing or frustrating? Stopping points?**

- **Theo is frustrated by long presses that do nothing:** launcher, room buttons, keyboard and delete screen's "No, keep!". He ends up mashing, which is how he lands on things.
- **The empty room is the sad, confusing moment.** Theo would stop there and want a grown-up.
- **Maya's natural stop** is the welcome card and the new pup. The visit to Biscuit is short and passive (a look, a rub), fine as a "say hi" moment.
- **Nothing is scary.** The farm is bright and Biscuit wags.

**Can imprecise input, a wrong guess or an interruption be recovered from without losing progress or needing a grown-up?**

- **Imprecise input mostly yes:**
  - offset taps around Bring back are inert, and its edge counts;
  - mashing Yes and sliding off cancel safely;
  - the photo has a generous hit area;
  - keyboard home keeps the pup choice.
- **A wrong guess (Theo's long Yes) is recoverable only on the very next screen, and only by picking the orange text button over the green one.**
  - After Go home or an interruption, the move is permanent. Biscuit stays visitable on the wall, but he is gone from the room.
  - This does not meet the "nothing is lost" spirit for a child who didn't choose it, which is why it is Major.

## Supplementary QA (after play, separate from the black-box pass)

- **`make -C apps/porthole playtest`** at `0cd6906`: **90 of 90 scenarios pass, 0 UI FAIL, 0 UI WARN.** The report (`apps/porthole/build/playtest/report.md`) was read in full, and its UI findings table is empty.
  - `b0_pets_farm_shelter` passes with 149 checks, and its sheets include `b0_pets_farm_shelter-farm_shelter.png` and `-wall.png`.
  - Monkeys `20`-`23` and `a1` pass under the sanitizers.
- **Raw `ui` probe** (`ui_probe_script.txt` → `ui_probe_output.txt`):
  - Bring back is 44x26 at (14,90); Go home is 64x22 at (48,126); the farm dog is 72x54.
  - Question buttons are 50x24. The Farm friends frame is 46x30. The room photo is 25x25 next to the door's 24x24, 2 px apart.
  - Shelter arrows are 32x32; Pick me! is 64x22; keyboard keys are 24x22.
  - Every one is at or above the 24x22 floor.
  - The text "N-Z" is 22 px wide in a 24 px key (tight but inside).
- **No unsupported glyphs seen.**

## Iteration history

- Rounds 1-5: `shelter-playtest.md` through `shelter-playtest-5.md`.
- **Round 6** (this report, `0cd6906`, snap sha256 `e0ac0f95…d357a`):
  - Round 5's m2, m3 and m4 are fixed, and its M1 is partly fixed.
  - New: M1 (the undo is lost after Go home or an interruption), M2 (shell delete, outside the flow), m4 and the nits.
  - Carried over: m1, m2 (now including Bring back) and m3.
- I changed no code and committed nothing.

## Brief suggestions

- Add an explicit interruption probe on the farm screen: "Restart, or let the rest timer fire, while the farm screen is up." The undo is only on that screen, so this is now the riskiest moment.
- For a pre-reader persona, ask the tester to record which button they would pick on color and size alone before a grown-up reads it, so "would Theo find the undo" is judged separately from "does the undo work".
- Give the fixture a book or a trick so "progress is kept" can be seen across the move and Bring back.

## Verdict

**Needs fixes.**

- M1 should be fixed, or the coordinator/parent should explicitly accept that the undo lasts for one screen, recording the impact on a pre-reading long-presser.
- M2 is a shell issue outside this change and should be routed to the shell owner.
- A fresh agent should then replay Theo's path: long Yes, then Go home and an interruption on the farm.

This was emulator roleplay only: no real child or physical device took part.
