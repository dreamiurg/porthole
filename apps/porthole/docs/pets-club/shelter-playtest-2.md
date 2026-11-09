# Pets Club farm and shelter: child playtest, round 2

This is an independent playtester pass, done as emulator roleplay only. No real child played it, and nobody tested it on a physical board.

## Build and setup

- Revision: `8a05c93` (docs(porthole): Pets Club farm visits, plain confirm and empty room). The tree was clean, and `make -C apps/porthole snap` was already up to date.
- Executable: `apps/porthole/build/host/snap`, sha256 `0b1418133aed49322ef592ccadfc95c2bf9aa3a51ad4b6988e3afab0dcd82a85`. The hash was the same after `make playtest`.
- Fixture: `.superpowers/sdd/2026-11-08-pets-club-shelter/fixture/*.sav`. The files are crago-s0 `a048df7b...`, porthole-m and porthole-mb `4bf5122f...`, porthole-p0 `845f86e0...` and sys-ran `ef260e9a...`. It holds one profile, "Mia" (age 8), whose dog Biscuit stats show as "Grown dog - day 31". Roleplay treats this profile as Leo's.
  - `build/host/` also held three leftover saves from other sessions: `crago-s1`, `porthole-p1` and `sys-window`. They would have added a second profile and a play window, so I moved them aside for my runs and put them back afterwards.
  - Before each of runs 1-4, I copied in a fresh set of fixture files.
- Driver: `snap --serve` from `apps/porthole/`, without `--fresh`, at the default clock (Tuesday 16:00). Input was only `down`, `move`, `up` and `tick`, plus `frame` for screenshots and a process quit and restart where noted. I used no `dbg`, `skip`, `app` or hidden-state query in the black-box pass.
  - A tap is `down`, `tick 100`, `up`, `tick 66`.
  - A double tap is two presses about 130 ms apart.
  - Coordinates are logical px (0-159).
  - Frames are native 480x480 PNGs.
- Evidence is in `docs/pets-club/shelter-playtest-2/`.
  - `NN_*.png` are the frames.
  - `input-trace.txt` is every command batch in order, with each run marked.
  - `drv.py.txt` is the driver, kept as .txt.
  - `supp_*.txt` are the supplementary monkey scripts.

### Disclosure: what I knew before play

- I read no game source (`games/`, `os/`, `shell/`), no playtest scenario, no spec and no `.superpowers/` file other than the fixture. I also left the round-1 report closed until my play was done.
- To learn the serve protocol, I read `host/sim.cpp`'s `runServe()` and `tools/webemu.py`.
- The same leak as round 1 happened: the session auto-loaded `apps/porthole/CLAUDE.md`. It says that an old dog (day 30+) can move to the farm on the kid's confirmed choice, stays on the room's memory wall, and that the kid then picks a pup at the shelter.
- `docs/playtesting.md`, which the procedure requires me to read, names `dbg old` ("farm button shows") and `wall5` ("Romeo to Charcoal").

So I knew the feature existed, but not its screens, wording or placement. Leo's expectations below come from what was on screen. Any claim about first-time comprehension needs a fresh agent that does not get this injected (see Brief improvements).

## Persona and dispatch brief (verbatim)

> You are a fresh, independent child-perspective playtester for Pets Club, a dog-care game on Porthole: a round 480x480 touchscreen with no buttons, emulated on the host. Follow apps/porthole/docs/playtesting.md (the required black-box playthrough, the child brief, the qualitative assessment) and the `playtest` skill. This is a source-free first-play pass. Do NOT read game source (apps/porthole/games/, os/, shell/), the playtest scripts in apps/porthole/tests/playtests/, docs/superpowers/, .superpowers/ (except the fixture dir below), or the previous playtest report in apps/porthole/docs/pets-club/ until your own play is finished. Base decisions on what you see on screen.
>
> ## Persona
> Leo, 7 years old, a second grader who reads short sentences slowly and skips anything long. He has played Pets Club for a month with his dog Biscuit and knows the home screen and the Stats page. He doesn't know anything new was added. He's impulsive, taps fast and sometimes twice, aims a little off, and loves animals. Simulated perspective only.
>
> ## Setup (disclosed fixture)
> - `make -C apps/porthole snap`. Record `git rev-parse --short HEAD` and `sha256sum apps/porthole/build/host/snap`.
> - Copy /home/user/porthole/.superpowers/sdd/2026-11-08-pets-club-shelter/fixture/*.sav into apps/porthole/build/host/ (overwrite) before each fresh start. It holds one profile whose Pets Club dog Biscuit was adopted 30 days ago, standing in for a month of play. The profile is called "Mia" in the picker. Treat it as Leo's own profile.
> - Run `apps/porthole/build/host/snap --serve` WITHOUT --fresh, from apps/porthole/, at the default clock. Drive it with ordinary touches, and look at the rendered frames after every action. No dbg, no app shortcuts, no wall-clock skips, no hidden-state queries. Waiting is fine.
>
> ## Goal and scope
> Leo's goal: "Play with Biscuit." Enter from the picker and launcher, notice what's new, and follow his curiosity. Focus on comprehension, which is what the last round flagged:
> - Before Leo taps any new choice about Biscuit, what does he think will happen? After it, was he right?
> - Does he understand where Biscuit went, and that it is for good? Is it gentle, not sad or scary?
> - Is the next step obvious from the room?
> - Choosing and naming a new pup.
> - Can he find Biscuit again later, and does visiting feel nice?
> - Fast double taps through each new screen. Do they ever skip a choice he didn't mean to make?
> - Leave the game mid-way and come back.
> Also do the brief's imprecise-finger probes on the new screens.
>
> ## Output
> Write your report to apps/porthole/docs/pets-club/shelter-playtest-2.md, with screenshots and the input trace in apps/porthole/docs/pets-club/shelter-playtest-2/ (any helper scripts you write there must pass `ruff check` and `ruff format`, or keep them as .txt). Include the revision and hash, the persona and this brief, the fixture, paths played, findings ranked with repro steps, the qualitative assessment, the first voluntary stopping point and untested gaps. After you finish, you may read the round-1 report and say which of its findings you saw again. Don't fix code or commit. Reply with ONLY (under 20 lines): a verdict (acceptable / needs fixes), the top findings ranked, and the report path.

Brief revisions during this pass: none.

## Paths played (first play)

Frame names are in `shelter-playtest-2/`. Leo's thoughts are roleplay and are marked as his.

### Run 1 (fresh fixture): the main story

1. **Picker and launcher** (`01`, `02`). Leo taps "Mia" slightly left of center, then the orange cartoon dog tile, and lands in his room with Biscuit, the bars and the four buttons (`04`). He pats Biscuit and a heart floats up (`05`). He taps the paw button, expecting Stats, but it opens the trick list (`07`); he goes back home. He taps the bars and Stats opens (`09`).
2. **Stats: "To the farm"** (`09`). This is the new thing: a light-blue "To the farm" button sits where the care bars used to be.
   - **Leo's guess before tapping:** "We go to a farm! Biscuit can see cows, then we come back." He double-taps it, impulsively.
3. **Confirm** (`10`, `11`). A dark card reads "Move Biscuit to the farm? / Biscuit will live there. Then pick a new pup." under a green **No!** and a brown **Yes**, with Biscuit sitting with his head down.
   - The double tap stopped here; it did not chain into anything.
   - Leo reads slowly and gets "live there" and "new pup". His guess is now: "Biscuit stays at the farm and I get a new puppy." So the card corrected his wrong first guess before anything was committed.
   - He hesitates, because he likes Biscuit, and taps the green "No!" (the go-colour). That returns him to Stats with Biscuit still his (`12`).
4. **Yes.** Leo opens the confirm again, taps the gap between the buttons (nothing happens, `13`), then double-taps Yes.
   - **His guess:** "Biscuit goes to the farm and I get a puppy."
   - The farm scene appears (`14_yes_dbl_a..c`): "Biscuit lives here now!", a wooden "Biscuit" sign, a fence, sun, a drifting cloud, a big wagging Biscuit and **Go home**.
   - The second tap of the double tap did nothing.
   - **Was he right?** Yes. The headline says plainly where Biscuit is.
   - Leo pats Biscuit and taps the sign: there is no reaction beyond the idle tail wag (`15`, `16`, `17`).
5. **Go home.** **Leo's guess:** a little unsure ("Biscuit comes home with me?").
   - The room is now empty (`18`): no Biscuit, no bars and no care buttons. A small framed Biscuit picture hangs on the wall, and a big purple **Shelter** sign lies on the rug where the dog was.
   - The picture plus the farm headline tell him Biscuit stayed. The single big button tells him what to do next.
6. **Curiosity before the shelter.** Leo taps the wall picture and gets "Farm friends" with Biscuit's framed photo (`19`). He taps the photo and visits the farm (`20`). The visit screen is the same scene with a home button and no Go home.
   - He taps, rubs and holds Biscuit: nothing happens (`21`-`23`).
   - After 4 s nothing has changed except the cloud (`24`). Leo would leave here.
   - Home goes back to Farm friends, then home again goes back to the room (`25`, `26`).
   - The ball and bowl in the empty room do nothing (`27`, `28`). The door still opens Paw Street, which shows his house and no dog (`29`), and tapping the house goes back home (`30`).
7. **Shelter** (double tap, `31`): "Pick a pup!" with a red-roofed kennel, round arrows, a name plate (name and breed), seven dots and **Pick me!**.
   - **Leo's guess:** "I choose a puppy here." Right.
   - He browses with the arrows (`32`-`39`): Connor/Golden pup, Boxy/Dalmatian, Romeo/Husky, Choco/Beagle, Marsha/Poodle, Waffles/Corgi and Charcoal/Black Lab, and it wraps around.
   - A double tap on the right arrow stepped two pups (Romeo to Marsha, skipping Choco). That was visible from the dots and was not a choice.
   - Tapping the pup in the kennel does nothing (`42`).
8. **Leave mid-way.** Home from the shelter goes to the empty room (`43`). Reopening the shelter starts at Connor again (`44`).
   - Home again goes to the launcher, and reopening Pets Club shows the splash and then the same empty room (`45`-`47`).
   - **Friction:** Leo's first tap on Shelter, made just as the room appeared after the splash, did nothing. He tried again and it worked (`48`-`50`). This was reproduced: a tap about 2.0 s after opening is eaten (`51a`-`c`), and a tap at 4.0 s works (`52`).
9. **Pick and name.** He picks Choco the Beagle (`53`) and double-taps **Pick me!**. The letter keyboard opens with "Choco" filled in, and the second tap typed nothing (`54`).
   - Curious, he taps the pink **?**, which gives a random name, "Mochi" (`55`).
   - A double tap on backspace deleted two letters (`56`). Clearing everything showed the grey "Pup's name" placeholder and a grey OK (`57`).
   - He types M, A, then the N-Z page and X, for "Max" (`58`, `59`), with slightly off-centre taps.
   - He double-taps OK and gets "Welcome home, Max! / Mia and Max, best friends." with a blinking "tap" (`60_ok_a..d`). The welcome was not skipped.
   - A double tap on the welcome takes him to the room with Max the beagle, the bars and buttons back, and Biscuit's picture still on the wall (`61`).
10. **Playing with Max.** Patting Max shows a heart (`62`). Stats now reads "Max / Puppy - day 1" with the bars (`63`).
    - The Fetch menu opens (`64`). Fetch with Max works (`65`, `66`).
    - Home during fetch ended the round with "Time's up! 0 bones! Good try!" (`67b`). This is pre-existing and out of scope.
11. **Find Biscuit again.** The wall picture opens Farm friends, and the photo opens the visit (`68`-`70`). Missing slightly to the right of the frame (62,53) still opens it (`74`).
12. **Process restart.** After a 6 s wait for the autosave, I quit the process and started it again. Picker, Mia and Pets Club show Max at home and Biscuit on the wall, and Stats reads "Max, Puppy - day 1" (`75`, `76`).

### Run 2 (fresh fixture): interruptions and naming edges

- Yes, then the farm (`80`). I quit the process on the farm scene (after 6 s) and restarted: the empty room and Shelter show (`81`). The choice was already saved.
- Shelter, then left x2 to Waffles, then **Pick me!** and the keyboard (`82`, `83`). Home on the keyboard goes back to the shelter with Waffles still showing (`84`).
- Pick me!, type one backspace, wait 6 s, then quit the process (`85`). After restart: the empty room and Shelter, with no half-adopted pup (`86`).
- Pick me! (Connor), then clear the name. Tapping the grey OK does nothing (`87`).
  - Leo mis-types "Bisbgig" because he lost track of the A-M/N-Z pages (`88`), then fixes it to "Biscuit" (`90`). Mixing letters from both pages takes several page toggles.
  - **Holding OK for 900 ms or 650 ms does nothing** (`89_hold_ok`, `91_hold650`). A 450 ms press works.
  - The welcome then reads "Welcome home, Biscuit! / Mia and Biscuit, best friends." (`92`). So the new pup can have the old dog's name, and Farm friends still shows the old Biscuit (`93`, `94`).

### Run 3 (fresh fixture): impatient repeat taps

- Farm, then four taps at (80,137), 600 ms apart. They stop in the empty room (`95`) and never chain into the shelter or naming.

### Run 4 (fresh fixture): confirm-card finger probes

- Press on Yes and drag off downwards: cancelled (`96`).
- Hold Yes for 1.2 s: nothing (`97`).
- Hold "To the farm" for 700 ms, then slide onto where Yes will be and release: it stays on Stats (`98`).

## Gesture and interruption matrix

| Screen / control | Offset tap | Double tap | Long hold | Drag off | Carried touch |
| --- | --- | --- | --- | --- | --- |
| Stats: To the farm | centre only | confirm once; nothing chained (`10`) | not tested alone | not tested | held 700 ms, slid onto Yes: nothing (`98`) |
| Confirm: No! | left of centre (50,104) hits (`12`) | gap | gap | gap | - |
| Confirm: Yes | right-of-centre hits; the gap between buttons does nothing (`13`) | farm once (`14`) | 1.2 s: nothing (`97`) | cancels (`96`) | see above |
| Farm: Go home | lower-left (62,142) hits (`18`) | gap (repeat taps at 600 ms covered in run 3) | gap | gap | 600 ms repeats stop in the empty room (`95`) |
| Farm / visit: Biscuit | - | - | tap, rub and 900 ms hold: no reaction (`16`, `21`-`23`) | - | - |
| Empty room: Shelter | off-centre (76,104) hits | shelter once (`31`) | gap | gap | - |
| Wall picture | (62,53), right of the frame, hits (`74`) | gap | gap | gap | - |
| Farm friends photo | centre hits (`20`) | gap | gap | gap | - |
| Shelter arrows | (138,84) hits | steps two pups (`34`) | gap | gap | - |
| Shelter: Pick me! | centre | keyboard once, no key typed (`54`) | gap | gap | - |
| Naming: backspace | (95,127) hits | deletes two (`56`) | gap | - | - |
| Naming: ? | centre hits (`55`) | gap | gap | gap | - |
| Naming: OK | (119,127) hits | welcome not skipped (`60`) | **650 and 900 ms: nothing**; 450 ms works (`89`, `91`, `92`) | gap | - |
| Welcome card | - | goes to the room once (`61`) | - | - | - |

"Gap" means untested. Round 1 covered several of these cells at revision `91c5385`; its results are not carried over here.

| Interruption | Kind | Result |
| --- | --- | --- |
| Home from the shelter, home again, launcher, reopen | ordinary leave and resume | empty room; the shelter restarts at Connor (`43`-`47`) |
| Home from naming | ordinary back | shelter with the same pup (`84`) |
| Home during fetch with the new pup | ordinary leave | the round ends as "Time's up" (pre-existing) (`67b`) |
| Process restart on the farm scene | process restart | empty room; the choice is kept (`81`) |
| Process restart mid-naming | process restart | empty room; no pup adopted (`86`) |
| Process restart after adopting and playing | process restart | Max and the wall both kept (`75`, `76`) |
| Process restart on the confirm card | process restart | gap |

## Findings (prioritized)

Blocker: none. Major: none.

### Minor

**m1. Visiting Biscuit is a still picture: nothing responds and it gets dull within seconds**
- **Screen**: `farm` (the farewell and the visit from Farm friends).
- **Issue**: Patting, rubbing or holding Biscuit gives no heart, sound or animation. The visit reuses the farewell headline, "Biscuit lives here now!", and only the cloud and tail move. For a 7-year-old who just gave up his dog, "can I pet him?" is the first thing he tries, and it fails silently. Leo would leave after about 5 s.
- **Concern**: feedback / boredom.
- **Repro**: run 1 to the room, tap 53 53 (wall), tap 84 76 (photo), then tap 85 98, drag 70,95 to 100,95, and hold 85 98 900. Same on the farewell: tap 80 95.
- **Screenshot**: `docs/pets-club/shelter-playtest-2/21_visit_pet.png`, `23_visit_hold.png`, `24_visit_4s.png`, `16_farm_pet.png`.
- **Suggestion (owner)**: a heart or happy hop when patted, and perhaps a short visit line such as "Biscuit is happy to see you!".

**m2. The name keyboard's OK ignores a slow press**
- **Screen**: `name_pet` (the shared name keyboard).
- **Issue**: Holding OK for 650 ms or 900 ms and lifting inside it does nothing, with no feedback; 450 ms works. A 7-year-old who presses firmly on the final "I'm done" button is a plausible case. He can retry, but nothing tells him to. Hold also does nothing on the confirm's Yes, which is the safe side for an irreversible choice, so this matters for OK, not Yes.
- **Concern**: feedback / recovery.
- **Repro**: run 2; shelter, Pick me!, then hold 119 125 for 650 ms. Trace: `input-trace.txt`, RUN 2 section.
- **Screenshot**: `docs/pets-club/shelter-playtest-2/91_hold650.png`, `89_hold_ok.png`.

**m3. A random masher can send the dog to the farm (supplementary QA, not first-play evidence)**
- **Screen**: `stats`, then `confirm_farm`.
- **Issue**: The irreversible choice is three ordinary taps: bars, To the farm, Yes. From an old dog's home screen, 4 of 40 seeded 60-gesture monkeys (`snap`, `newgame` + `dbg old`) ended with the dog on the farm (3) or already replaced (1). Starting on the confirm card, 12 of 20 30-gesture monkeys did.
  - A younger sibling mashing could do this. The dog stays visitable and progress is kept, so this is gentle, but it cannot be undone.
- **Concern**: meaningful choice / recovery.
- **Repro**: `docs/pets-club/shelter-playtest-2/supp_monkey_home.txt` (replace SEED with 1-40) and `supp_monkey_confirm.txt`, each run with `./build/host/snap --fresh --script`.
- **Screenshot**: n/a (scripted). The confirm is `11_farm_wait.png`.
- **Suggestion**: the coordinator should decide whether a press-and-hold Yes or a second "Bye, Biscuit!" step is wanted. The product decision says the choice must be the kid's.

**m4. The farm button still replaces the care bars on an old dog's Stats page** (round 1 m5, seen again)
- **Screen**: `stats`.
- **Issue**: Leo, who knows Stats, finds his food, fun, sleep and clean readout gone. The page's most prominent new control is the one irreversible action. Compare Max's Stats page, which has the bars.
- **Concern**: consistency / meaningful choice.
- **Repro**: fixture; tap 60 68, 35 74, wait 4 s, then tap 44 26.
- **Screenshot**: `docs/pets-club/shelter-playtest-2/09_bars.png` vs `63_max_stats.png`.

### Nits

- **n1. The first tap right after the splash hands over to the room is eaten.** This is the 450 ms fresh-screen gate, a general shell rule rather than a new one. Leo's Shelter tap did nothing once, and he retried. Repro: launcher, Pets Club tile, wait 2.0 s, tap 76 104. Screenshot: `51b_open.png`, `51c_first_tap.png`.
- **n2. The wall picture is small and not signposted.** Leo found it by curiosity, because the empty room had little else to tap. With the new pup back, the room is busier and a kid may never notice that Biscuit can be visited. Its tap area is generous. Screenshot: `18_gohome_b.png`, `61_after_welcome_b.png`.
- **n3. The "Shelter" sign is words only.** It is the only live button, so Leo still found it. A pup or kennel picture would help a slow reader who cannot read "Shelter". Screenshot: `18_gohome_b.png`.
- **n4. The naming screen has no prompt, and "?" is a mystery until tapped** (round 1 m4, partly seen again). Leo tapped "?" out of curiosity and liked it. Switching between the A-M and N-Z pages made him mistype "Bisbgig". Screenshot: `54_pick_c.png`, `88_biscuit_typed.png`.
- **n5. "No!" with an exclamation mark reads like shouting** next to a plain "Yes". Screenshot: `11_farm_wait.png`.
- **n6. The splash still shows a golden dog when the room is empty** (round 1 n2, seen again). Screenshot: `51a_open.png`.
- **n7. The shelter forgets its place on leaving and restarts at Connor** (round 1 n3, seen again). Screenshot: `44_shelter_reopen.png`.
- **n8. The new pup can take the old dog's name.** Then there are two "Biscuit"s: one in the room and one on the farm whose visit says "Biscuit lives here now!". This is the kid's choice; it could confuse. Screenshot: `92_hold450.png`, `94_wall_two_biscuits.png`.
- **n9. The farm is sunny at night** (supplementary; seen in `build/playtest/b0_pets_farm_shelter-wall.png` frame `15_farm_charcoal`): it shows a sun under the night tint. Not seen in black-box play, which ran at 16:00.
- **n10. The kennel pup does not react to a tap.** It is a small missed delight. Screenshot: `42_tap_kennel_pup.png`.

Pre-existing and out of scope, noted only: in the Fetch menu the icons sit over the "Fetch" and "Word Fetch" labels (`64_ball_btn.png`), and home during fetch ends the round as "Time's up!" (`67b.png`).

## Qualitative assessment

**Can Leo recognize the goal, the actions and what changed?**
- **Yes for the farewell.** This is the round-1 problem, now fixed.
  - Leo's guess for "To the farm" was wrong (a trip).
  - The confirm card fixed that before anything was committed. "Biscuit will live there. Then pick a new pup." is two short sentences he can sound out, and the sitting, head-down Biscuit sets a gentle tone.
  - The farm headline "Biscuit lives here now!" confirms it in plain words.
- **"For good."** "Live there" and "lives here now" imply it, and Leo understands Biscuit stays on the farm. Nothing says "can't come back", but the visit makes that softer rather than harsher.
- **"Go home"** was briefly ambiguous ("does Biscuit come too?"). The empty room with his picture answered it straight away.
- **Shelter, browsing and naming** are clear from pictures: kennel, arrows, dots, "Pick me!", a prefilled name and the welcome card.
- **The tricky part** is reading-heavy: the confirm card is the one place reading carries the decision.

**Is it gentle, sad or scary?** Gentle and not scary: sunny farm, wagging dog, name sign, no crying. There is a small pang at the empty room, but the Shelter sign gives an immediate happy next step. The one disappointing note is m1: Leo wants to pet Biscuit goodbye and can't.

**Interesting decisions and feedback.**
- Picking from seven distinct dogs is a real, enjoyable choice. Leo browsed the full lap (about 8 taps) and went back for the beagle.
- Renaming, "?" and the welcome card give good feedback.
- The farm decision has a clear No! that recovers fully.
- The visit (m1) is the weak spot: a picture, not an activity.

**Where it gets repetitive, passive or frustrating.**
- The visit screen after about 5 s.
- Page-toggling to type a name with letters from both halves (the "Bisbgig" moment), which costs about 10 extra taps.
- One eaten tap after the splash (n1).
- No long waits: the whole farm-to-welcome flow is about 15 taps and 2-3 minutes for Leo.

**First voluntary stopping point.**
- About 4 minutes in, Leo would close the visit to Biscuit within seconds (`24`).
- His real stopping point comes after "Welcome home, Max!" and a round of fetch with Max. That is a satisfying close: new pup, named by him, playing.
- Everything after that (revisits, restarts, hold and drag probes, the "Biscuit" rename, runs 2-4) was coverage, not Leo's choice.

**Recovery from imprecise input and interruptions.**
- Good. Double taps never skipped or made a choice: To the farm, Yes, Shelter, Pick me!, OK and the welcome each acted once. The arrow and backspace double taps acted twice, which is what Leo meant.
- Repeat taps at 600 ms on the Go-home spot stop in the empty room.
- Drag-off and a hold on Yes never commit, and a touch carried from To the farm does not land on Yes.
- Back from naming keeps the pup.
- Process restarts keep the choice and never half-adopt.
- The only weak input spot is the slow press on OK (m2).

## Round 1 findings: which I saw again

| Round 1 | This round |
| --- | --- |
| M1: the confirm does not say it is permanent | **Fixed.** The card now says "Biscuit will live there. Then pick a new pup." Comprehension was good. |
| m1: no goodbye; "Go home" ambiguous | **Partly fixed.** The farm now says "Biscuit lives here now!". "Go home" is still briefly ambiguous, and there is still no goodbye pat (folded into m1 here). |
| m2: cannot visit or pet | **Visit fixed; petting not.** Now m1. |
| m3: stacked buttons chain repeat taps into naming | **Fixed.** Shelter sits higher, and 600 ms repeats stop in the empty room. |
| m4: naming has no prompt; "?" a mystery | **Seen again** (n4). |
| m5: the farm button replaces the Stats bars | **Seen again** (m4). |
| n1: confetti over the welcome title | **Not seen.** The specks were above the card (`60_ok_a`). |
| n2: the splash shows a golden dog | **Seen again** (n6). |
| n3: the shelter resets to Connor | **Seen again** (n7). |
| n4: "It" for Biscuit | **Fixed.** The card uses the name. |
| n5: library stars "Biscuit" | Not checked this round. |

## Supplementary QA (separate from first play)

- `make -C apps/porthole playtest` at `8a05c93`: **90 of 90 scenarios pass, 0 UI FAIL / 0 UI WARN.** That includes `b0_pets_farm_shelter` (107 checks, with ui-checks on the stats, confirm, farm, empty home, wall and shelter screens) and the monkeys (no sanitizer reports). I read the whole `build/playtest/report.md`: the summary table, the empty findings table and the contact-sheet list. I also looked at the `b0` night and wall sheets (n9).
- Ad hoc `snap-asan` monkeys: 600 gestures from the confirm card and 600 from the shelter. There were no ASan or UBSan reports.
- Plain `snap` monkeys: 40 seeds from an old dog's home and 20 from the confirm (m3).

## Brief improvements suggested

1. **The first-play leak is still open.** The auto-loaded `apps/porthole/CLAUDE.md` describes the farm, wall and shelter. `docs/playtesting.md`'s `dbg` table names `old` and `wall5`, and the procedure makes the playtester read it. A truly fresh comprehension claim needs one of three things:
   - a working directory that does not auto-inject the app brief,
   - redacted copies of those two documents,
   - or an explicit coordinator acceptance of the leak.
2. **Ask for a slow-press probe on every final button** (OK, Yes, Pick me!). It found m2, which double-tap probes miss.
3. **Ask for the "I want Biscuit back" moment explicitly.** That means judging the visit as an activity, not only whether it exists, and asking how long the persona would stay.
4. **Add a night or evening session** (`--now` at 20:00 counts as a disclosed setting, not a skip) so tinted farm, wall and shelter frames get black-box eyes.
5. **Mention the sibling-mash risk** as a coordinator question, so the brief does not leave m3 to supplementary QA by chance.

## Untested gaps

- Multi-day play with the new pup. Round 1 covered three days on an older build.
- Evening and night tint in black-box play. It was seen only in the supplementary contact sheet.
- Two or more profiles: whether another kid's Paw Street or wall shows Biscuit.
- Session rest or the daily cap arriving mid-shelter or mid-naming.
- More than one dog on the farm through natural play (`b0` covers it scripted).
- A process restart on the confirm card. Offset and drag probes on Go home, the arrows' outer edges, "?" and Pick me!.
- The longest typable name on the welcome card and the wall.
- A physical device, real touch noise and a real child.

## Verdict

**Acceptable**, with lesser findings for the owner to fix or defer.
- No Blocker or Major remains. Round 1's Major (the farewell was not understood) is fixed: Leo's wrong first guess was corrected by the confirm card before he committed, and the farm and the empty room made the outcome and the next step plain.
- m1 (a passive, non-reactive visit) is the most player-visible gap and should be fixed or explicitly deferred.
- m2-m4 and the nits each say what they cost the player, so the coordinator can accept or defer them.
- Because of the disclosed brief leak, a fresh, leak-free agent should confirm the first-time comprehension claim.
- All of this is emulator roleplay, not real-child or physical-device evidence.
