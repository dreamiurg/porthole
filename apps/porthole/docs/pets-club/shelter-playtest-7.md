# Pets Club shelter, long presses and profile delete: playtest 7 (Ellie, 6)

Independent child-perspective black-box pass, emulator only. This is a roleplay of a 6-year-old, not a study with a real child. It does not cover the physical device.

## Build and setup

- Revision `48fbf75` (clean tree apart from this report's evidence directory).
- `apps/porthole/build/host/snap` sha256 `ac3a95d50506efdfc23cc9ef3c9987c99af74715bf9d8f8e3bd8213e05e99437`. `make -C apps/porthole snap` had nothing to rebuild, and `make playtest` left the same hash.
- Fixture: before every fresh start, `.superpowers/sdd/2026-11-08-pets-club-shelter/fixture/*.sav` was copied over `apps/porthole/build/host/*.sav`. Leftover `.sav` files were deleted first. The fixture has one profile, "Mia" (played as Ellie's own), with no kid code and no admin code. Her Pets Club dog Biscuit is on day 31, and the farm button shows.
- Driver: `snap --serve` from `apps/porthole/`, without `--fresh`, at the default clock. Input was only `down`, `move`, `up` and `tick`, plus `frame` to grab native 480x480 PNGs. There were no `dbg`, `skip`, `app` or hidden-state queries. Each probe replays one input trace in a new process from the fixture, so every trace is a reproducible recipe. The traces are in `shelter-playtest-7/traces/*.txt`, in this grammar: `tap X Y [ms]` = down, tick (120 ms by default), up. `hold X Y MS` = down, tick MS, up. `mash X Y N` = N taps of 80 ms, 120 ms apart. `drag` = down, 8 moves, up. `shot` = one frame. Coordinates are logical (0-159).
- The driver lives outside the repo (a Python file in the session scratchpad). The brief forbids `.py` files in the evidence directory.
- Screenshots: `shelter-playtest-7/shots/`. Overview sheets: `sheet_startover_hold.png`, `sheet_startover_zone.png`, `sheet_hold_thresholds.png`.

**Disclosure.** I did not open game, OS, shell or host source, the playtest scenarios, the design specs, or `.superpowers/` apart from the fixture. However, the session loaded the app brief (`apps/porthole/AGENTS.md`) into my context automatically. It describes the new long-press rule, the `no_delete` card, and the fact that Start over and farm Yes take a longer hold. I knew those mechanisms existed. I did not know where Start over is triggered, and found that by touch. I read `tools/webemu.py` to learn the serve protocol. I read the earlier shelter reports only after play. Supplementary checks after play: one raw `--script` repro, which printed `screen=` names, and `make playtest`.

## Persona and brief (verbatim)

> You are an independent child-perspective playtester for Porthole [...] Persona: Ellie, 6, can't read yet except her name. She presses everything firmly and often holds it 1-3 s, mashes when nothing seems to happen, wanders into menus, and loves her dog Biscuit in Pets Club. Simulated perspective only.
>
> Setup (disclosed fixture): `make -C apps/porthole snap`. Record `git rev-parse --short HEAD` and `sha256sum apps/porthole/build/host/snap`. Before each fresh start, copy [...]/fixture/*.sav into apps/porthole/build/host/ (overwrite). There's one profile, "Mia" (treat it as Ellie's own), with no secret code, and no grown-up code is set. Her Pets Club dog Biscuit is 30 days old. Run `apps/porthole/build/host/snap --serve` WITHOUT --fresh, from apps/porthole/, at the default clock. Ordinary touches only (taps, short and long presses, slides, mashing). No dbg, app shortcuts, clock skips or hidden-state queries.
>
> Scope: This build changed how every button reacts to long presses, how deleting a profile works, and several things in Pets Club. Play as Ellie: 1. The picker and launcher: long-press and mash everything, including her own name. Can Ellie delete her profile, or anything else important, by accident? Does anything look broken or ignore her? 2. Making a new profile from the picker's + (if it lets her): the name keyboard with long presses on every key, OK and the arrows. 3. Pets Club: the room buttons, the stat pips, Stats (including long presses near its bottom), the farm question if she finds it, the farm, the empty room, the shelter, naming, the wall and visits. Everything long-pressed. 4. Whether any button now does something she didn't want (a long press that fires twice, a hold that used to be harmless now acting). 5. Briefly open one tilt or tap game from the launcher (e.g. Marble Kick or Tilt FC) and check its start/exit buttons with long presses. Also the brief's imprecise-finger probes.
>
> Output: Write apps/porthole/docs/pets-club/shelter-playtest-7.md [...]

The brief was not revised during this pass.

## What Ellie did (first play, in order)

1. **Picker** (`b01`, `c0x`). She holds her name for 2 s, as she always does. A card appears with her dog face, a lot of words and a green OK (`b01_after_hold_mia.png`). She cannot read "To delete Mia, ask a grown-up to set a code in Admin." She presses the face (nothing happens), then holds OK: she is back at the picker. She holds her name again straight away for 3 s, but it began within 450 ms of the screen change and is ignored (`c03`). She mashes her name five times: the launcher opens, and the remaining mashes land on the middle tile, the cream "Biscuit" dog. She ends up in the Biscuit game's name keyboard (`c05_mash_mia.png`). Nothing is lost, but she is now in the wrong dog game.
2. **Launcher** (`d0x`). She holds the cream dog for 2 s. A yellow caption says "Biscuit" while her finger is down (`d02`, `d03`), and the game opens when she lets go (`d04`). For Ellie, whose own dog is called Biscuit, the second dog tile with the "Biscuit" caption is a lure. The Pets Club tile is the orange dog.
3. **New profile** (`e0x`-`l0x`). Holding the title for 3 s and holding "dev" do nothing (`e01`, `e02`). Holding New opens "Your name?" (`e03`). Holding OK with no name does nothing (`f01`). Each 1-3 s hold on a letter types exactly one letter (`f02` "A", `f03` "Aaa"). Each hold on `<` deletes one letter (`f04`), and each hold on N-Z flips the page once (`f05`). She spells E-L-L-I-E with holds and holds OK (`g01`). On "Pick a face!" she holds the frog: it turns yellow while held, and the next screen comes on release (`h01`, `h02`). She holds 6 on "How old are you?". On the code pad, four holds on 5 give 5555 (`j02`), then "Once more!" (`j03`). She cannot read that, presses 1 2 3 4, and gets "Not the same!" followed by a fresh "Pick 4 numbers" (`k01`-`k03`), which is gentle. She holds the blue "skip" for 2.5 s and lands on her new launcher with the frog (`l01`).
4. **Picker with two profiles** (`n0x`). Holding Ellie gives the frog card (`n01`). Holding OK returns to the picker (`n02`). Holding Ellie twice more, then the home button, then holding Mia for 3 s each give only the card or the picker (`n03`-`n05`). Mashing OK six times returns to the picker and stays there (`n06`). **No profile could be deleted.** This is good.
5. **Pets Club room** (`p0x`, `q0x`, `r0x`, `rr_*`). The splash moves on by itself. Holding the dog pets it, with hearts and a happy sit (`q01`-`q03`). Holding food opens "What's for dinner?". Holding Cookie shows "Yum! +10", then the "First Meal" sticker card (`r01`-`r03`). Her mashing taps clear the card and open the food menu again (`r04`), which is fine. Holding the home button in the food menu closes the menu only and does not leave the game (`s02`). Every room object she holds does one thing once:
   - ball: Fetch / Word Fetch
   - books, and the shelf: "Biscuit's Big Day"
   - paw: tricks
   - door: Paw Street
   - lamp: "Sweet dreams..."; holding it again gives "Good morning!" (`la01`)
   - bowl: food
   - window and heart: nothing
   - either row of stat pips: Stats

   While the dog sleeps, holding him does nothing harmful and food still opens (`la02`, `la03`).
6. **Stats** (`t02`). She sees the big name, the hearts, a light-blue "To the farm" button, pink Stickers and plum Hats. Holding Stickers opens the sticker grid. Holding Hats opens the hat picker (`st01`, `st02`). Holding the "Mia, age 8" line does nothing (`u01`).
7. **Stats, holding just under "Mia, age 8"** (`u02`, `w_*`). **A purple "Start over? Biscuit will forget you!" screen opens**, with No! and Yes and "Hold Yes". No control is drawn there: the hit spot is the blank strip just under the age line (y about 143-150, x about 60-80). She cannot read the warning. When she holds Yes, "Hold Yes" flashes white and the Yes button fills (`v_1000_mid`). **After about 2 s, Biscuit is gone:** "Knock knock! A package for you." (`v_2000_end`, `so_2000`). Opening the package gives a different puppy and "Pup's name" (`x01`, `x02`). Leaving with home and reopening Pets Club brings back the package, not Biscuit (`y02_reopen`). The Pets Club save file was removed. Biscuit is not on the wall and is not on the farm. Nothing on screen offers a way back. **Two ordinary Ellie holds have erased her 31-day dog for good.**
8. **The farm question** (`z02`, `fy_*`, `fm_*`). From Stats she holds "To the farm" and gets a navy screen with a dog and a red barn, "Move Biscuit to the farm?", No! and Yes. Mashing Yes 12 times does nothing (`qa01`). Holding No! for 2 s goes back to Stats (`qa02`). A hold of 1.2 s or 1.8 s on Yes does nothing. **A hold of 2.0 s or more moves Biscuit** (`fm_2000`). That is inside Ellie's ordinary 1-3 s press.
9. **Farm** (`fy_2500_a`). "Biscuit lives here now!" shows a big Biscuit, a sign, an orange "Bring back" and a green "Go home". Holding Bring back brings Biscuit home with "Biscuit is home!" (`qb01`). Holding Go home leads to the empty room (`fa01`).
10. **Empty room** (`fa0x`). It has the framed Biscuit on the wall, an orange "Bring back" with Biscuit's face, and a plum "Shelter". The dog's face on Bring back is the one cue a pre-reader can use, and holding it brings him home (`fc01`). Holding the frame opens "Farm friends" with Biscuit's portrait (`fb01`).
11. **Shelter** (`sa0x`, `sb0x`). "Pick a pup!" shows a kennel, two orange arrows, a name and breed, dots, and a green "Pick me!".
    - A 2 s or 3 s hold on an arrow moves exactly one pup (`sa01`, `sa02`).
    - Mashing left five times moves five pups and wraps round (`sa03`).
    - A tap 12 px below the right arrow's centre still moves (`sa04`).
    - Tapping the kennel does nothing (`sa05`). A drag that starts on the arrow and slides off cancels (`sa06`).
    - Holding "Pick me!" opens the name keyboard prefilled with the pup's name (`sb01`).
    - Each hold on the pink `?` gives one new name (`sc01` Mochi, `sc02` Maple). Each hold on `<` deletes one letter (`sc03`). Mashing `<` empties the name back to the grey "Pup's name" placeholder. A hold on OK with an empty name does nothing (`sc04`, `sc05`).
    - A hold on OK with a name gives "Welcome home, Boxy! Mia and Boxy, best friends." Her lifting finger does not dismiss it (`sd01`). A tap does, and she is in the room with Boxy and Biscuit's frame (`sd04`).
12. **Wall and visit** (`sf0x`, `sg0x`). Holding the frame opens "Farm friends". Holding Biscuit's picture opens the farm visit, where holding Biscuit makes a heart (`sf04`). On the visit, the wall and the room, a 2.5 s hold on home goes back exactly one step each time: wall, room, launcher (`sg01`-`sg03`).
13. **Marble Kick** (`mk*`). On the start page, holding Play starts the level. A tap on home leaves to the launcher (`mkb01`). During play a tap on home is ignored (`mkd01`). Holding it draws a dotted ring and leaves within about 1.2 s (`mkd02`, `mkd03`). The lifting finger does nothing on the launcher (`mkd04`). Holding the level badge does nothing (`mkc01`).
14. **Tilt FC** (`tf*`). On the calibrate page, holding Play starts the countdown (`tf02`). Holding the back arrow leaves (`tf05`). In the match a tap on back is ignored (`tf06`) and a 2.5 s hold leaves (`tf07`).

### Gesture and interruption matrix

| Target | Tap | Hold 1.5-3 s | Mash | Offset / edge | Drag off | Slide onto | Carried touch over a screen change |
| --- | --- | --- | --- | --- | --- | --- | --- |
| Picker name row | launcher | `no_delete` card, once | launcher, then the mashes hit a tile (c05) | gap between rows: nothing (m02) | not tested (gap) | not tested (gap) | ignored, < 450 ms (c03) |
| `no_delete` card OK / home | picker | picker | picker, stays (n06) | face: nothing | gap | gap | yes, nothing fired |
| Picker New / title / dev | name keyboard | New: keyboard; title (3 s) and dev: nothing | - | - | gap | gap | - |
| Name keyboard keys, `<`, N-Z, OK | one each | one each (f02-f05) | `<` one per tap (sc04) | left of A: nothing (e04) | gap | gap | OK hold onto the face screen: nothing fired |
| Face, age, code pad, skip | one | one, on release | - | - | gap | gap | yes, nothing fired |
| Launcher tile | opens on release | opens on release (d04) | - | - | gap | gap | yes |
| Room buttons | open | open once | reopen after a card (r04) | between food and ball: nothing (lb01); food's low edge: opens (lb02) | cancels (lc01) | does not fire (lc02) | - |
| Dog | - | pets | - | - | - | - | - |
| Stats bottom strip | nothing | **Start over (u02)** | - | 139: nothing; 143-150: Start over | gap | gap | - |
| Start over Yes | nothing | **1.0-1.8 s nothing; 2.0 s+ erases the dog** | gap | - | gap | gap | yes |
| Farm Yes | nothing (qa01) | 1.2-1.8 s nothing; 2.0 s+ moves the dog | nothing | - | gap | gap | yes |
| Shelter arrows | +1 | +1 per hold | +1 per tap | 12 px low: hits | cancels | gap | - |
| Shelter OK / ? / `<` | one | one | one each | - | gap | gap | yes |
| In-game home (menus, wall, visit) | one step | one step | gap | - | gap | gap | yes |
| Tilt games: exit during play | ignored | leaves (ring) | gap | gap | gap | gap | yes |

Leave/resume: leaving Pets Club with home and reopening it in the same process was tested (`y02`). **A process restart mid-flow (for example on the farm or the shelter) was not tested** in this pass. Each probe did start in a new process from the fixture.

## Findings, ranked

### Blocker

**B1. Two ordinary holds erase her dog: a hidden long press on Stats opens Start over, and about 2 s on Yes wipes Biscuit.**
- **Screen**: Stats, then `confirm_reset` (the "Start over?" screen), then the adoption intro.
- **Issue**: A long press on a blank strip just under "Mia, age 8" (logical x about 60-80, y about 143-150; y 139 and y 156 do nothing) opens Start over. A 2.0 s hold on Yes (1.8 s is not enough) erases the 31-day dog. He does not go to the wall or the farm, there is no undo, and the Pets Club save is removed. To a pre-reader, the "Hold Yes" fill is an invitation to keep holding.
- **Concern**: recovery, comprehension. "Nothing is lost" is a hard rule, and Ellie holds 1-3 s by habit. This is the same two-long-press accident the parent closed for profile delete on 2026-11-09. The reversible farm move takes the same ~2 s hold as this irreversible wipe, and its trigger at least has a visible button.
- **Repro**: fixture, then `traces/so2000.txt` (tap Mia, open Pets Club, tap the pips; `hold 80 146 2500`; `hold 115 101 2000`). The thresholds are in `traces/sx.txt` and `traces/fx.txt`, which hold the last swept value (2200 ms); edit the Yes hold to 1200, 1500, 1800 or 2000 to repeat the sweep. A headless one-shot is `traces/repro_start_over_two_holds.txt` (`snap --fresh --script`), which prints `screen=confirm_reset` and then `screen=intro`.
- **Screenshot**: `docs/pets-club/shelter-playtest-7/shots/u02_hold_very_bottom.png`, `shots/v_2000_end.png`, `shots/so_2000.png`, `shots/y02_reopen.png`, and the sheets `sheet_startover_zone.png` and `sheet_startover_hold.png`.
- **Suggested direction (owner's call)**: Gate Start over like profile delete (an admin code), or move it into the grown-ups' menu. Never give an invisible target to a destructive action.

### Minor

**M1. Farm Yes needs only about 2 s, inside a 6-year-old's normal press.**
- **Screen**: the farm question.
- **Issue**: 1.2-1.8 s does nothing and 2.0 s commits. Ellie cannot read "Move Biscuit to the farm?". The dog and barn picture hints at a move but not that it is a choice. Bring back on the farm and in the empty room recovers it, and the dog's face on that button is a good cue, so impact is limited until she picks a new pup at the shelter. After that the undo is gone and only visits remain.
- **Concern**: comprehension, recovery.
- **Repro**: `traces/fy2500.txt`; thresholds in `traces/fx.txt`.
- **Screenshot**: `shots/fm_1800.png`, `shots/fm_2000.png`.

**M2. Mashing her own name carries her into the other dog game, "Biscuit".**
- **Screen**: picker, then launcher, then the Biscuit game's name keyboard.
- **Issue**: After the 450 ms gate, the remaining mash taps hit the middle launcher tile, which is the cream dog game called "Biscuit", the same name as her Pets Club dog. A pre-reader cannot tell the two dog tiles apart, and the yellow caption only spells "Biscuit". No harm is done, because home leaves.
- **Concern**: comprehension, consistency.
- **Repro**: `traces/t3.txt` (`mash 80 67 5` on the picker).
- **Screenshot**: `shots/c05_mash_mia.png`, `shots/d02_finger_on_tile.png`.

### Nit

**N1. The `no_delete` card is all words for a pre-reader.**
- **Screen**: `no_delete`.
- **Issue**: Ellie sees her own face and a green OK, and does not know why holding her name stopped her from playing. The card is harmless and OK and home both return, so this is only a comprehension note.
- **Concern**: comprehension.
- **Repro**: `traces/t2.txt`.
- **Screenshot**: `shots/b01_after_hold_mia.png`.

**N2. "Word Fetch" label covered by its icon.**
- **Screen**: the ball menu.
- **Issue**: The A-tile icon covers the final "h" of "Word Fetch", and the bone icon touches "Fetch".
- **Concern**: overlap.
- **Repro**: `hold 66 128 2000` in the room.
- **Screenshot**: `shots/crop_wordfetch.png`, `shots/rr_ball.png`.

**N3. "Pup's name" placeholder is crowded.**
- **Screen**: the shelter's name keyboard.
- **Issue**: The grey "Pup's name" fills its box edge to edge, and the pink `?` overlaps the box's right border.
- **Concern**: spacing.
- **Repro**: `traces/s3.txt`.
- **Screenshot**: `shots/crop_pupname.png`, `shots/sc04_mash_back_empty.png`.

### Checked and fine (positives)

- **Profile delete is safe.** With no codes, every hold on a name gives only the kid-safe card. Repeated holds, mashing OK and holds carried across the screen change never deleted anything.
- **Holds fire exactly once.** Every key, arrow, room button, tile, OK and `?` that Ellie held fired once, on release, with a pressed look while held. No hold fired twice.
- **Fingers that move or arrive late are handled.** Dragging off cancels and sliding onto does not fire. Holding home in a menu backs out one step only.
- **Hold-to-commit choices resist mashing.** Mashing farm Yes does nothing.
- **Tilt games keep their exit safe during play.** Marble Kick and Tilt FC ignore a tap on exit during play and leave on a hold with a visible ring. On the start pages they act on a tap or a hold.

## Qualitative assessment

- **Can Ellie see the goal and the actions?** In the room, yes. The pictures carry everything: food bowl, ball, book, paw, the dog itself. Holding the dog pets him with hearts, a cookie gives "Yum!" and a sticker. She can tell what changed after each action. Text carries the important screens, though: the farm question, Start over, the `no_delete` card, "Once more!" and "skip" on the code pad. On exactly the two screens where a wrong choice matters most, she is guessing.
- **Decisions, feedback, fairness.** The shelter gives a real and pleasant choice. The pups' pictures differ clearly, the arrows step one at a time even when held, and `?` keeps rolling fun names. Feedback is consistent: pressed colours, captions, and the ring on the tilt-game exits. The farm move has good recovery (Bring back shows Biscuit's face). Start over has none (B1).
- **Boredom and where she would stop.** Her first natural stopping point is after petting, the cookie and the sticker, roughly the first 3-5 minutes. Fetch (sliding to catch bones) would hold her a few more minutes. Books, Word Fetch and the trick list are text-heavy, and she would drift away from those or back out with home. The farm and shelter flow would only matter to her because of "a new puppy!". Choosing a pup is the most engaging part of the new content. Visiting Biscuit is passive (one heart per hold), but fine as a "say hello" moment.
- **Recovery without an adult.** Misplaced taps, wrong code entries, mashing and the farm move are all recoverable without help. Start over is not: two holds lose the dog for good, which no 6-year-old can undo and which would genuinely upset this persona.

## Automated suite (supplementary)

I ran `make -C apps/porthole playtest` at `48fbf75`: **91 of 91 scenarios pass, 0 UI FAIL, 0 UI WARN.** That includes `b0_pets_farm_shelter` (175 checks) and `b1_long_presses` (12 checks). I read the full `build/playtest/report.md`: no failures, no sanitizer reports, no UI findings. None of the scenarios covers B1's hidden Start over hold. I did not add a scenario to `tests/playtests/`, because one that asserts the right behaviour would fail CI until the owner decides on the fix. The repro script for that scenario is `shelter-playtest-7/traces/repro_start_over_two_holds.txt`.

## Gaps

- Process restart mid-farm and mid-shelter (leave/resume was tested only within one process).
- Profiles with a kid code or with an admin code set, including the pad's Admin key and wrong-code waits. The fixture has neither.
- The launcher's second page and the remaining games (Biscuit beyond its name screen, Pumpkin, Radar, Starfighter, Kingdom, Rally).
- Several days of play with the new pup.
- Drag-off and slide-onto on the shell's keyboards and the picker.
- Real children and the physical board: neither was used.

## Earlier reports

I read playtests 1-6 only after play. None of them reports the hidden Start over hold. Report 1 recorded that a hold on "To the farm" did nothing; under this build's long-press rule it now opens the question, as the brief intended.

## Verdict

**Needs fixes.** B1 is a blocker for the stated persona. Everything else in scope (holds, mashing, delete safety, farm, shelter, naming, wall, visits, tilt-game exits) behaved safely.
