# Pets Club shelter, final pass: playtest 8 (Leo, 6, and Ana, 9)

This is an independent child-perspective black-box pass on the emulator. Leo and Ana are simulated personas, not real children. Nothing here was checked on the physical board.

## Build and setup

- Revision `033a859`. The tree was clean apart from this report's evidence directory.
- `apps/porthole/build/host/snap` sha256 `7c8da21a637cc847b1f646b96579a80216fd241bcd290a5a628273cd2e98d168`. `make -C apps/porthole snap` had nothing to rebuild, and the hash was the same after `make playtest`.
- Fixture: before each fresh start, I copied `.superpowers/sdd/2026-11-08-pets-club-shelter/fixture/*.sav` (`crago-s0`, `porthole-m`, `porthole-mb`, `porthole-p0`, `sys-ran`) over `apps/porthole/build/host/`. It has one profile, "Mia", with no kid code and no admin code. Her Pets Club dog Biscuit shows "Grown dog - day 31" on Stats.
- Driver: `build/host/snap --serve`, run from `apps/porthole/` without `--fresh` and at the default clock. I used only `down`, `move`, `up` and `tick`, plus `frame` to save native 480x480 PNGs. For the one Pumpkin calibration I also sent `tilt 0 0 -1000`, which means the board is lying flat. I never used `dbg`, `skip`, `app`, `ui`, `screen` or `debug`. The driver is a small persistent Python process kept in the session scratchpad, outside the repo. It logs every command it sends to the trace files.
- Input traces (coordinates are logical px, 0-159; `# ...` lines are my notes):
  - `shelter-playtest-8/trace-leo.txt`: Leo's whole session, one process.
  - `shelter-playtest-8/trace-ana.txt`: Ana's session, a process restart, then Pumpkin.
  - `shelter-playtest-8/trace-interrupt.txt`: process restarts mid-farm and mid-naming.
  - `shelter-playtest-8/trace-supplementary.txt`: checks made after reading report 7.

  Line 182 of `trace-leo.txt` is a stray note that was sent as a command. The sim answered `ok` and nothing happened.
- Screenshots are in `shelter-playtest-8/`. `L*` files are Leo, `A*` Ana, `P*` Pumpkin, `I*`/`U*`/`T*` the interruption session, `Q*` the supplementary checks, and `W*`, `R2_*`, `R3_*`, `R4_*` the trick-lesson frames.

**Disclosure.** I did not open game, OS, shell or host source, the playtest scenarios, the specs, or `.superpowers/` apart from the fixture. I read `tools/webemu.py` only to learn the `--serve` protocol. The session loaded `apps/porthole/AGENTS.md` into my context automatically. From it I knew that long presses count as taps, that profile delete needs a code, and that farm Yes takes a longer hold. It did not tell me where anything sits on screen. I read the earlier shelter reports only after both personas had finished. The `Q*` checks were made after reading report 7 and are supplementary, not first play.

## Personas and brief (verbatim)

> You are an independent child-perspective playtester for Porthole, a round 480x480 touchscreen with no buttons (emulated on the host) that holds several kids' games behind a "Who's playing?" picker and a launcher. Follow apps/porthole/docs/playtesting.md and the `playtest` skill. Play source-free: don't read source (apps/porthole/games/, os/, shell/, host/), tests/playtests scripts, docs/superpowers/ or .superpowers/ (except the fixture dir). Decide from the screen. Read the earlier reports in apps/porthole/docs/pets-club/ only after you finish playing.
>
> This is the final pass before release. Two personas, short passes:
> A. Leo, 6, can't read, presses everything firmly for 1-3 s, mashes, double-taps impatiently, and loves his dog Biscuit.
> B. Ana, 9, reads well and plays on purpose: she sends her old dog to the farm, picks a new pup, names it, and later visits the old dog.
> Simulated perspectives only.
>
> ## Setup (disclosed fixture)
> - `make -C apps/porthole snap`. Record `git rev-parse --short HEAD` and `sha256sum apps/porthole/build/host/snap`.
> - Before each fresh start, copy /home/user/porthole/.superpowers/sdd/2026-11-08-pets-club-shelter/fixture/*.sav into apps/porthole/build/host/ (overwrite). There's one profile, "Mia" (treat it as each persona's own), with no secret code, and no grown-up code is set. The Pets Club dog Biscuit is 30 days old.
> - Run `apps/porthole/build/host/snap --serve` WITHOUT --fresh, from apps/porthole/, at the default clock. Ordinary touches only. No dbg, app shortcuts, clock skips or hidden-state queries.
>
> ## Scope
> - Leo: the picker (his name held long), the launcher, Pets Club's room, Stats (including long presses anywhere on it, especially near the bottom), the farm question, the farm, the empty room and its buttons (double taps arriving from the street or the splash), the shelter, naming, Train and Fetch (holding a finger down while things change). Can he lose his dog or his profile, or anything important, by accident without a way back? Does any button feel broken?
> - Ana: the whole deliberate path, end to end, plus a visit. Is it clear, gentle and fun? Where would she stop?
> - One quick look at Pumpkin Hide & Seek from the launcher: play a little, resting a thumb on its on-screen signs.
>
> ## Output
> Write apps/porthole/docs/pets-club/shelter-playtest-8.md, with screenshots and the input trace in apps/porthole/docs/pets-club/shelter-playtest-8/ (no .py files; save scripts as .txt). [...] Don't fix code or commit. [...]

The brief was not revised during this pass.

## Leo (6, pre-reader, 1-3 s presses): what happened

1. **Picker.** Leo holds "Mia" for 2.5 s. A card with his dog face, a padlock, four lines of text and a green OK appears at about 0.7 s (`L02`-`L05`). He cannot read it. He holds OK for 1.5 s and is back at the picker (`L06`). He holds his name again for 1.2 s and gets the same card (`L07`). Only a quick tap (150 ms) opens the launcher (`L09`). **Nothing was deleted**, but his normal press on his own name never lets him play. See M1.
2. **Launcher.** His first press (my coordinate slip) landed on the "Mia" header strip and went back to the picker (`L10`-`L12`). That is harmless. Holding the orange dog tile shows the yellow "Pets Club" caption while his finger is down, and the game opens when he lifts (`L13`-`L15`). The splash moves on by itself after about 1.7 s (`T1`-`T14`).
3. **Room.** Holding the dog pets him: hearts and a happy sit (`L16`-`L18`). Holding the shelf opens the book carousel, and home brings him back (`L19`, `L20`). Holding the stat pips opens **Stats** (`L21`).
4. **Stats, long presses.** He makes 2-2.5 s holds on "Mia, age 8" (y 139), under it (y 152), on the hearts row and on the panel. Nothing happens (`L22`, `L23`, `L26`). A hold at (30,130) is on the pink Stickers button and opens the sticker grid (`L24`), which is correct. Home from the stickers goes back to Stats (`L27`). The supplementary sweep (below) confirms that report 7's hidden Start over strip is gone.
5. **Farm question.** He holds the light-blue "To the farm" for 2 s (`L29`). A navy screen shows "Move Biscuit to the farm?", a green "No!", a brown "Yes", "Hold Yes" and a dog beside a red barn. He has no home button here; No! is the way out.
   - A 1 s hold on Yes fills the button in plum, the dog walks toward the barn, and lifting cancels (`L30`-`L32`). That is good, readable feedback.
   - A 3 s hold, his ordinary press, **moves Biscuit to the farm** (`L33`-`L35`). The threshold is 2.0 s: 1.8 s and 1.9 s do nothing (`Q03`-`Q05`). See M2.
   - Sliding off Yes onto No! and lifting cancels, and No! does not fire (`L135`-`L137`). A 2 s hold on No! goes back to Stats (`L134`).
6. **Farm.** "Biscuit lives here now!" shows a sign with his name, a big Biscuit, an orange "Bring back" and a green "Go home" (`L37`). Leo cannot read either word. He holds the dog, then holds the green button, because green has meant "go" everywhere else, and lands in the **empty room** (`L39`, `L40`).
7. **Empty room.** He sees the framed Biscuit on the wall, an orange "Bring back" **with Biscuit's face on it**, and a plum "Shelter" with no picture. The face is the one cue a non-reader can use.
   - Holding the wall frame opens "Farm friends" with Biscuit's portrait (`L41`). Holding the portrait opens a farm visit (`L43`). Holding the dog there gives hearts and a tongue (`L44`, `L45`). Home steps back one screen each time.
   - A short tap on Bring back brings him home with "Biscuit is home!" (`L79`, `L80`).
8. **Double taps arriving at the empty room.**
   - *From Paw Street*: he double-taps his house. The house sits over the Shelter/Bring back seam.
     - With a 150 ms or 300 ms gap, the second tap is swallowed (`L52`, `L68`).
     - **With a 400 ms gap, the second tap opens the Shelter** (`L54`, `L69`-`L70`). It is harmless, because home goes back. See N1.
   - *From the splash*: double taps on the splash dog with 300, 400 and 600 ms gaps, landing on Bring back, are all ignored (`L73`-`L78`).
     - A deliberate tap about 0.9 s after the room appears is ignored too (`I04`, `I05`).
     - One at about 1.2 s works (`U1`, `U2`).

     That guard is a safe default.
   - Unreliable attempts are kept in the trace and labeled. My loop `L57`-`L59` pressed home in the room, which leaves Pets Club, so its later taps hit the picker and the profile keyboard. My loop `L63`-`L64` sent its first door tap during the splash-to-room guard, so its later taps all landed in the Shelter. No profile was created in either case.
9. **Fetch.** Holding the ball button for 2 s opens Fetch / Word Fetch (`L81`, `L82`). Holding Fetch for 3 s opens the game only when he lifts, with no early fire (`L83`-`L85`).
   - Sliding the finger moves the dog. Catching bones gives hearts and a point. A bee gives "Ouch, a bee!" and takes a point away (`L92`-`L96`).
   - He kept his finger down through "Time's up!", and nothing fired. The card says "7 bones!", but a last-second bee's "Ouch, a bee!" toast covers the line under it for about 2 s (`L98`-`L102`). See N3.
   - Holding OK for 2 s goes back to the room (`L103`).
10. **Train.** The paw button opens the trick list, and Sit is the only open tile (`L104`). Holding Sit for 3 s opens the lesson when he lifts (`L105`-`L107`).
    - "Watch me glow, then tap the spots!", then "tap to start". He held the dog for 1.6 s and the lesson started when he lifted (`L109`-`L113`).
    - The glow demo runs straight after. A child staring at the screen catches it, but my first round's frames missed it (`W1`-`W12`).
    - A wrong first spot gives "Oops! Watch again." and the demo replays (`L121`-`L123`), which is gentle.
    - Rounds 2 and 3 were done by watching the glows (head, tummy, tail, then head again; `R3_*`, `R4_*`). Each spot counts **when he lifts**. While he holds a spot for 1-3 s, nothing visible changes (`L126`-`L128`). See N2.
    - "Lesson 1 of 3 - Practice Sit again tomorrow!" waits for a tap and does not take the held finger (`L130`-`L132`).
11. **Shelter and naming.**
    - "Pick a pup!" shows seven breeds. A 2 s hold on an arrow moves one pup, and a double tap moves two (`L141`, `L142`).
    - Holding the pup does nothing visible (`L143`, `L144`).
    - Holding "Pick me!" for 3 s opens the name keyboard, prefilled with the pup's name, when he lifts (`L145`-`L147`).
    - Holding letters types one each. A double tap on C typed one "c" (`L151`). The pink "?" rolls a name ("Pip", `L152`). OK turns yellow while held (`L153`, `L154`).
    - "Welcome home, Pip! Mia and Pip, best friends." needs a fresh tap (`L155`-`L157`). The room then has Pip and Biscuit's frame on the wall, and Stats says "Pip, Puppy - day 1" (`L158`, `L159`).
    - One of my imprecise taps (73,26), just under the home button on the name screen, went back to the Shelter and dropped the typing (`L148`, `L149`). The home button's touch area reaches a few px below its drawing. That is acceptable.

**Can Leo lose his dog by accident?** He can move Biscuit to the farm with one ordinary 2-3 s press, after he has already pressed the pips and "To the farm". Until he adopts, Bring back (with Biscuit's face) undoes it on the farm and in the empty room. After the four more presses that adoption takes (Go home or Shelter, Pick me, OK, the welcome tap), Biscuit stays on the farm for good. He is still on the wall and can be visited. **He cannot lose his profile**: every hold on his name gives the safe card. **He cannot reset his dog**: no Start over was reachable. **Does any button feel broken?**
- His own name on the picker, held as he always holds, never starts play (M1).
- Pumpkin's HIDE sign ignores a resting press (M4).
- The trick spots do nothing until he lifts (N2).

## Ana (9, reads, deliberate): what happened

1. She taps "Mia" and holds a tile to read its name ("Pets Club", `A02`). In the room she feeds Biscuit (`A04`, `A05`).
2. She looks for something about Biscuit's age and taps the stat pips. Stats shows "Grown dog - day 31" and "To the farm" (`A06`). That is a guess for a first-timer: the room has no picture of a barn or of "Biscuit's page". Once she is on Stats, it is clear.
3. Farm question (`A07`). She taps Yes quickly first. "Hold Yes" flashes white (`A08`), she understands, and she holds Yes for about 2 s. "Biscuit lives here now!" (`A10`). The words are kind and nothing feels like losing him. "Bring back" is a reassuring safety net.
4. Go home leads to the empty room. She taps Shelter and browses all seven pups (`A12`-`A19`): Connor the Golden pup, Boxy the Dalmatian, Romeo the Husky, Choco the Beagle, Marsha the Poodle, Waffles the Corgi and Charcoal the Black Lab. The list wraps around. The dots show where she is. Every breed looks different.
5. She picks Waffles, erases the name with `<` (the grey "Pup's name" placeholder and a greyed OK appear, `A22`), and types "Toast" across the A-M and N-Z pages (`A24`). She goes back to the shelter to compare once more. **Her typed "Toast" is gone** when she picks Waffles again; it is back to "Waffles" (`A25`, `A26`, N4). She types it again and taps OK (`A27`). "Welcome home, Toast! Mia and Toast, best friends." (`A28`). The room now has the corgi (`A29`).
6. **Visit.** She taps the wall frame. "Farm friends" shows Biscuit's portrait, and that opens the farm with "Biscuit lives here now!". Holding Biscuit gives hearts (`A30`-`A32`). Home steps back to Farm friends, then to the room (`A33`, `A34`).
7. **Leave and resume** (same process). Home goes to the launcher, and reopening shows Toast in the room (`A35`, `A36`). **After a process restart** (new `snap --serve`, no fixture copy), Toast and Biscuit's frame are still there (`A37`, `A38`).

**Where Ana stops.** She would stop voluntarily right after the welcome card and one visit, about 4-6 minutes in. That is a satisfying natural end: she made a real choice, named it, and saw the old dog safe. She might visit Biscuit once more the next day. The visit is thin (a sign, the dog, petting), so she would not come back for it often. Toast as a day-1 puppy gives her a reason to return to the room.

## Interruptions (fresh fixture, `trace-interrupt.txt`)

- Restart about 1.5 s after the farm Yes: the empty room comes back with Bring back (`I01`, `I02`).
- Restart during naming, before OK: the empty room comes back with no pup adopted, and Bring back still works (`I03`-`I07`).
- Leave via home and reopen the empty room: unchanged (`I08`-`I11`), and Bring back works (`I12`).

## Pumpkin Hide & Seek (quick look)

- Title screen: "TILT TO ROLL / START / GET CANDY", with an exit sign (a framed return arrow) at the top (`P03`). Then "HOLD IT COMFY AND STILL / READY" (`P05`), then play with a big HIDE sign (`P07`).
- **Thumb resting on HIDE for 3 s:** nothing happens and the sign does not change. Grandma comes ("GRANDMA!", "HMM?", "QUICK! HIDE!"), finds the pumpkin, and the screen says "YOU LOST - SWEET PUMPKIN!" with a pie and TRY AGAIN (`P08`-`P14`). A quick tap on HIDE hides the pumpkin and the sign turns to "COME OUT" (`P39`-`P41`). Ignoring a resting thumb is by design, but a firm presser gets no hint that the press didn't count (M4).
- **Thumb resting on the exit sign during play:** a quick tap is ignored (`P37`, `P38`), which is good. A rest of about 0.5 s **leaves the game for the launcher** (`P19`, `P23`-`P28`). The only hold indicator is a thin yellow bar along the sign's bottom edge (`crop_pumpkin_exit_bar.png`). See M3.
- On the result screen, a 3 s rest on the exit sign leaves when the thumb lifts (`P15`-`P17`), which is fine.

## Gesture and interruption matrix

| Target | Tap | Hold 1-3 s | Double tap / mash | Offset / edge | Drag off / slide onto | Finger held over a screen change |
| --- | --- | --- | --- | --- | --- | --- |
| Picker name | launcher | `no_delete` card (every time) | mash x5: launcher, then the Biscuit game's keyboard (`Q00`) | gap | gap | not tested |
| `no_delete` OK | picker | picker | gap | gap | gap | nothing fired |
| Launcher tile | opens | caption, opens on lift | gap | header strip: picker | gap | nothing fired |
| Room dog / buttons / shelf / pips | open | open once / pet | gap | gap | gap | gap |
| Stats bottom strip (y 139-154, x 55-100) | - | nothing (35 holds, pixel-identical frames) | - | - | - | - |
| Farm Yes | "Hold Yes" flashes | 1.0-1.9 s fills and cancels; 2.0 s+ commits | gap | - | off Yes cancels; onto No! does not fire | commit happens on hold, no carry |
| Farm Bring back / Go home | works | works | Go home x2: empty room, no extra | gap | gap | gap |
| Empty room Bring back / Shelter | works | works | street 400 ms: Shelter; splash 300-600 ms: ignored | gap | gap | gap |
| Shelter arrows / Pick me | +1 / opens | +1 / opens on lift | +2 | gap | gap | nothing fired |
| Name keys / ? / < / OK | one each | one each, OK yellow | C x2 = one c | (73,26) hits home | gap | nothing fired |
| Fetch | - | - | - | - | slide steers | held through Time's up: nothing fired |
| Trick spots | counts | counts on lift, no feedback while held | gap | gap | gap | held through "Good dog!": nothing fired |
| Pumpkin HIDE | hides | ignored, no feedback | gap | gap | gap | gap |
| Pumpkin exit (play) | ignored | leaves after about 0.5 s | gap | gap | gap | - |
| Leave / resume | ok (Ana) | | | | | |
| Process restart | ok after adoption, mid-farm, mid-naming | | | | | |

## Findings, ranked

No Blocker. No Major.

### Minor

**M1. Leo's ordinary press on his own name never starts play.**
- **Screen**: picker, then the `no_delete` card.
- **Issue**: A press past about 0.6 s on a profile row always opens the all-text "To delete Mia, ask a grown-up..." card. Leo holds 1-3 s by habit, so he loops picker, card, OK, picker. Only a quick tap or mashing gets him in. It is safe, but the first screen of the device teaches him that his name "does the wrong thing".
- **Concern**: comprehension, feedback.
- **Repro**: `trace-leo.txt` lines for L02-L08: `down 70 67; tick 2500; up`, then OK, then `down 70 67; tick 1200; up`.
- **Screenshot**: `docs/pets-club/shelter-playtest-8/L04_hold2500.png`, `L07_hold_again.png`.
- **Owner's call**: Opening the game on release, and showing the card only after a much longer hold (or with a picture that says "not this"), would keep delete safe without blocking the normal press. Report 7 logged the card as N1.

**M2. Farm Yes commits at 2.0 s, inside a 6-year-old's normal press.** This carries over from report 7's M1, unchanged.
- **Screen**: the farm question.
- **Issue**: 1.8 s and 1.9 s cancel, and 2.0 s commits. Recovery is good until adoption: Bring back shows the dog's face. After adoption only visits remain. The dog-walks-to-the-barn animation is a good hint for readers and non-readers alike.
- **Concern**: comprehension, recovery.
- **Repro**: fresh fixture, then tap Mia, the Pets Club tile, the pips (120,30) and To the farm (80,77); then `down 115 101; tick 2000; up` (`trace-supplementary.txt`).
- **Screenshot**: `Q03_yes1800.png`, `Q05_yes2000.png`, `L35_yes3000.png`.

**M3. Pumpkin: a thumb resting on the exit sign ends the round in about 0.5 s.**
- **Screen**: Pumpkin, in play.
- **Issue**: A tap is ignored, which is good. A rest of about 0.5 s leaves for the launcher, and the round is lost. The only hold cue is a thin bar along the bottom edge of the sign. For a tilt game held by its rim, a thumb near the top bezel is likely. Report 7 measured about 1.2 s with a ring for Marble Kick's in-play exit.
- **Concern**: recovery, consistency.
- **Repro**: open Pumpkin, START (80,130), READY (80,130), then `down 80 11; tick 500; up`.
- **Screenshot**: `P23_e100.png` … `P27_e500.png`, `crop_pumpkin_exit_bar.png`.

**M4. Pumpkin: a firm press on HIDE does nothing and shows nothing, so the pumpkin gets caught.**
- **Screen**: Pumpkin, in play.
- **Issue**: Ignoring a resting thumb is intended. But a child who presses HIDE firmly (Leo-style) sees no change and then loses ("YOU LOST"). A pressed look, or a nudge such as "tap!", would tell him why.
- **Concern**: feedback, comprehension.
- **Repro**: in play, `down 80 132; tick 3000; up`.
- **Screenshot**: `P08_rest1.png`, `P09_rest3.png`, `P13.png`.

**M5. Mashing his name carries Leo into the other dog game, "Biscuit".** This carries over from report 7's M2, unchanged.
- **Screen**: picker, then launcher, then the Biscuit game's name keyboard (prefilled "Biscuit").
- **Issue**: Leo's own Pets Club dog is called Biscuit, and he cannot read which dog tile is which. It is harmless: home goes back to the launcher.
- **Concern**: comprehension, consistency.
- **Repro**: fresh fixture, then five taps at (80,67) of 80 ms each, 120 ms apart.
- **Screenshot**: `Q00_mash_name.png`.

### Nit

**N1. A 400 ms double tap from Paw Street lands on Shelter.**
- **Screen**: Paw Street, then the empty room.
- **Issue**: The house sits over the Shelter/Bring back seam. A slow second tap at 400 ms passes the room's guard, while the splash path guards about 1 s. Shelter is harmless and home goes back.
- **Concern**: consistency.
- **Repro**: empty room, door (27,52), then `down 82 95; tick 100; up; tick 400; down 82 95; tick 120; up`.
- **Screenshot**: `L69_street.png`, `L70_gap400.png`.

**N2. Trick spots give no feedback while held.**
- **Screen**: trick lesson.
- **Issue**: A spot counts only when the finger lifts. During a 1-3 s hold the circle and the "3 of 4" counter don't change.
- **Concern**: feedback.
- **Repro**: in the Sit lesson, `down 96 77; tick 3000; up`.
- **Screenshot**: `L126_last_held.png`, `L128_last_held3.png`, `L129_lift.png`.

**N3. A bee toast covers the Fetch result card.**
- **Screen**: Fetch, "Time's up!".
- **Issue**: A bee hit in the last second puts "Ouch, a bee!" over the card's "Great job!" line, and the "7 bones!" line, for about 2 s.
- **Concern**: overlap.
- **Repro**: `trace-leo.txt`, L97-L102.
- **Screenshot**: `crop_bee_toast.png`, `L101_end_after.png`, `L102_end_4s.png`.

**N4. Backing out of naming forgets the typed name.**
- **Screen**: shelter name keyboard.
- **Issue**: Ana typed "Toast", went back to the shelter to compare, picked the same pup again, and the name was back to "Waffles". This is a reasonable "cancel" meaning, but it surprised her.
- **Concern**: recovery.
- **Repro**: `trace-ana.txt`, A24-A26.
- **Screenshot**: `A24_toast.png`, `A26_name_again.png`.

**N5. Fetch menu: icons overlap their labels.** This carries over from report 7's N2.
- **Screen**: the ball menu.
- **Issue**: The lock icon covers the "h" of "Word Fetch", and the bone touches "Fetch".
- **Concern**: overlap.
- **Repro**: room, `down 66 128; tick 2000; up`.
- **Screenshot**: `crop_fetch_menu.png`.

**N6. The farm and the empty room are all words for a non-reader.**
- **Screen**: farm, empty room.
- **Issue**: Bring back has the dog's face (good). "Go home" is the green "go" button and Shelter has no picture, so Leo's natural choice on the farm walks him one step further from Biscuit. Recovery is still there.
- **Concern**: comprehension.
- **Repro**: `trace-leo.txt`, L37-L40.
- **Screenshot**: `L37_after2.png`, `L40_empty.png`.

**N7. Pumpkin's loss screen says "YOU LOST" in big capitals.**
- **Screen**: Pumpkin, result.
- **Issue**: The pie and "SWEET PUMPKIN!" are cute, but the headline is blunter than the house tone ("failure is gentle").
- **Concern**: comprehension.
- **Repro**: in play, wait for Grandma without hiding.
- **Screenshot**: `P13.png`.

### Verified fixed or fine

- **Report 7's B1, the hidden Start over strip on Stats, is gone.** I made 35 holds of 2.5 s at x 55-100, y 141-154, plus Leo's holds at y 139 and 152. Every frame was pixel-identical to Stats (`Q02_stats.png`, kept sample `Qs_75_147.png`). Leo could not reset his dog.
- The farm question's hold feedback is clear: the button fills, the dog walks to the barn, and "Hold Yes" flashes on a tap. Drag-off cancels.
- Bring back with the dog's face works from the farm, from the empty room, after leaving and reopening, and after process restarts.
- Holds fire once, on release, across tiles, room buttons, arrows, keys, OK and Pick me. No held finger fired anything on the next screen.
- The trick-lesson wrong answer ("Oops! Watch again.") and the Fetch bee are gentle.

## Qualitative assessment

- **Goal, actions and change.**
  - *Ana*: yes, once she finds Stats. Every step of the farm, shelter, name, welcome and visit flow says what it does and what happened. Finding "To the farm" behind the stat pips is the one guess.
  - *Leo*: the room is all pictures, and he can pet, fetch and train with clear hearts and points. On the screens that matter most (the delete card, the farm question, the farm, the empty room) he is guessing from colour, and green "Go home" leads him away from Biscuit (N6). The dog-walks-to-barn animation and Biscuit's face on Bring back are the two pictures that help him.
- **Decisions, feedback, fairness.** Choosing among seven distinct pups and naming one is a real, enjoyable choice for Ana; the "?" name roll is fun for Leo. Feedback is consistent: pressed colours, a yellow OK, captions, hearts. The two weak spots are the trick spots while held (N2) and Pumpkin's HIDE (M4). Recovery is fair: the farm move is undoable until adoption.
- **Boredom, repetition and where they stop.**
  - *Leo* stops after petting and one Fetch round, about 3-4 minutes. Fetch is 30 s with frequent catches, his best activity.
  - The Sit lesson is three rounds of watch-then-repeat. Round 3 needs four spots, so he would likely quit or lose the sequence there; that is long for a 6-year-old who can't read the "Lesson 1 of 3 - again tomorrow" card.
  - *Ana* stops after the welcome card and a visit, about 5 minutes in. That is a good natural stopping point. The visit has only one interaction (petting), which is fine as a "say hello" but not a reason to return often.
- **Recovery without an adult.** Mis-taps, wrong trick spots, double taps, sliding off Yes, leaving and process restarts were all recovered without help, and no progress was lost. Leo could not delete his profile or reset his dog. The one path that is not undoable is adopting a new pup after an accidental farm move. That needs four more presses, and the old dog stays on the wall and on the farm.

## Automated suite (supplementary)

`make -C apps/porthole playtest` at `033a859`: **91 of 91 scenarios pass, UI 0 FAIL / 0 WARN**. That includes `12_no_start_over`, `b0_pets_farm_shelter` (179 checks) and `b1_long_presses` (12 checks). I read the full `build/playtest/report.md`: no failed directives, no sanitizer reports, no UI findings. I added no scenario. M1-M5 are product decisions that would need the owner's choice before a test can assert them.

## Gaps

- Picker drag-off and slide-onto, and the profile keyboard.
- The launcher's second page and other games beyond Pumpkin's first round.
- Several real days with the new pup, the farm over time, and the remaining Sit lessons. Clock skips were not allowed.
- Profiles with kid or admin codes, and more than one profile.
- The physical board, real touch noise, real tilt and real children. This was emulator roleplay only.

## Earlier reports

I read reports 1-7 only after play. Report 7's Blocker (the hidden Start over) is fixed. Its M1 (2.0 s farm Yes), M2 (mashing into the Biscuit game) and N2 (Fetch menu overlap) are unchanged and appear above as M2, M5 and N5.

## Verdict

**Acceptable**, with the Minor findings above deferred to their owners. No Blocker or Major remains:
- The destructive Start over is gone.
- Profile delete is safe.
- The farm move is undoable until adoption.
- Ana's path is clear, gentle and satisfying.

M1 (a held name never starts play) and M3/M4 (Pumpkin's resting-thumb behaviour) are the ones most worth fixing before many more pre-readers use the device.
