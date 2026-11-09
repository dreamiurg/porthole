# Pets Club farm and shelter: black-box child playtest

Independent playtester pass, emulator roleplay only. Nobody tested this with a real child or on a physical board.

## Build and setup

- Revision: `91c5385` (refactor(porthole): Pets Club farm and wall cleanups). Clean tree apart from this report.
- Executable: `apps/porthole/build/host/snap` sha256 `b94ce1799e0f09849dba93b8386e226ff020e820c921aaaa445807883d428167` (`make -C apps/porthole snap` was up to date).
- Fixture: `.superpowers/sdd/2026-11-08-pets-club-shelter/fixture/*.sav` (crago-s0, porthole-m, porthole-mb, porthole-p0, sys-ran). It holds one profile, Mia (8), with a Pets Club dog Biscuit adopted 30 days ago (Stats read "Grown dog - day 31"). The fixture was copied fresh before every run.
- Driver: `snap --serve` without `--fresh`, default clock, from `apps/porthole/` (later from an isolated copy of `build/host/`; see the Traces section). Input was only `down/move/up/tick` plus `frame`, and a process restart where noted. No `dbg`, `skip`, `app` or hidden-state queries were used in the black-box pass. A tap is `down`, `tick 66`, `up` (about 100 ms). Coordinates are logical (0-159). The frames are native 480x480 PNGs.

### Disclosure: what I knew before play

I read no game source (`games/`, `os/`, `shell/`), no playtest scenario (`b0_pets_farm_shelter.txt` exists but stayed closed), no spec, plan or `.superpowers/` file other than the fixture. I read `host/sim.cpp`'s header and `runServe()`, plus `tools/webemu.py`, to learn the serve protocol. However, two documents gave away parts of the feature before play:

- The session's auto-loaded `apps/porthole/CLAUDE.md` (the app brief) says that an old dog (day 30+) can move to the farm, stays on the room's memory wall, and the kid then picks a pup at the shelter.
- `docs/playtesting.md`'s sim table mentions `dbg old` ("the farm button shows") and `wall5` ("shelter pups, Romeo to Charcoal").

So I knew that a farm button and a memory wall existed, but not where they were, what they looked like or what they said. I wrote down Mia's expectations from the screens, not from that knowledge. A fully fresh agent should confirm the comprehension claims below (see Brief improvements).

## Persona and dispatch brief (verbatim scope)

Mia, 8, a third grader. She reads short sentences confidently and skims long ones. She has played Pets Club for a month with her dog Biscuit and knows the home screen (food, play, books, tricks buttons, and the stat pips that open Stats). She has never seen anything this update added. She is curious and impatient with reading, taps things that look fun, sometimes taps beside a button, double-taps when nothing seems to happen, and sometimes holds too long. Her goal: "Play with Biscuit like usual."

In scope:
- Anything new on Biscuit's pages.
- Sending Biscuit somewhere, and the room afterwards.
- Getting and naming a new dog: browsing every choice, keeping a suggested name, changing it, backing out halfway.
- Whether Biscuit is still remembered and whether Mia can find that.
- Leaving and reopening at awkward moments (mid-choice, empty room).
- A few days of waiting with the new pup.
- Imprecise-finger probes on every new screen: offset taps, double taps, long hold, drag-off, carried touches.

Brief revisions: none were made during this pass.

## Paths played (first play, in order)

Trace: `shelter-playtest/traces/main.txt`, then `nameC.txt`. Frames: `shelter-playtest/firstplay/`.

1. Picker, Mia, launcher, Pets Club tile (top-left orange dog), then home (`00`-`02`). Ordinary play: food, cookie ("Yum! +10"), a "First Meal" sticker card, then play (`03`-`07`). The sticker on day 31 is probably a fixture artifact.
2. Tapped the stat pips to open Stats. **New: a big green "To the farm" button** where the food/fun/sleep/clean bars used to be (`08_stats`). Mia's expectation: "a trip to a farm, fun, Biscuit comes back." Nothing on the button says otherwise.
3. Tap: a dark-blue card reads "Move Biscuit to the farm? / It will be happy there!" with a green **No!** on the left, a brown **Yes** on the right, and a droopy Biscuit below (`09`). Patting the dog did nothing (`11`). No! returned to Stats (`12`).
4. To the farm again, then Yes. The farm scene shows Biscuit big and wagging behind a fence, a "Biscuit" sign, a drifting cloud and a single **Go home** button (`13_yes_*`, `14_anim_*`). There is no text, no goodbye line and no wave. Patting Biscuit does nothing (`14_pat`).
5. Go home. The room is now **empty**: no stat pips, no four buttons, a small framed Biscuit portrait on the wall (where a pink heart sat before), and one big purple **Shelter** button on the rug (`15_home_*`). The bowl, ball, shelf, heart and window do nothing (`28_*`).
6. Tapped the little portrait: a **"Farm friends"** page with Biscuit's framed picture and name (`16`). Tapping the portrait does nothing; there is no visiting (`17`). The home button leads back to the room (`18`).
7. Shelter: **"Pick a pup!"**, a red-roofed kennel, round left/right arrows, a name plate (name and breed), 7 dots and **Pick me!** (`19`). The right arrow went through all 7 and wrapped: Connor/Golden pup, Boxy/Dalmatian, Romeo/Husky, Choco/Beagle, Marsha/Poodle, Waffles/Corgi, Charcoal/Black Lab (`20_pup1-8`). Then left x3 to Waffles (`21`).
8. Pick me! opened the letter keyboard prefilled with "Waffles", with a pink **?** beside it (`22`). My taps on "?" landed on the name box's right end (logical 110,28) and did nothing (`23_q1/2`). Home led back to the shelter with Waffles still showing, a clean back-out (`24`).
9. Home from the shelter returned to the empty room, home again went to the launcher (`25`, `26`). Reopening showed the splash, which still shows a golden dog, and then the empty room (`27`).
10. Back in, Pick me!, then name changes (`nameB`/`nameC`). Seven backspaces showed the grey placeholder "Pup's name", and OK turned grey (`30_bs_all`). N-Z page (`31`). The real ? (logical 123,29) gave a random name, "Maple" (`32_q_empty`). Typed T-A-C-O across both pages (`33_taco`). OK showed "Welcome home, Taco! / Mia and Taco, best friends." on a starry card with a blinking "tap" (`34_ok_*`). Tap: home with Taco the corgi, the bars back, and Biscuit's portrait still on the wall (`35`). Stats show "Taco / Puppy - day 1" with the bars (`36`). Stickers kept "First Meal" and gained "All Grown Up" (`37`, `38`).
11. Keeping the suggested name: Pick me!, then OK, showed "Welcome home, Waffles!" (`probes/keep.png`).
12. Days: from the launcher, three plain `tick 86400000` waits. The device dropped back to "Who's playing?" each time; I tapped Mia, opened Pets Club, then Stats, fed, played and left (`days/d1_*`-`d3_*`). Taco reached Puppy day 2, 3 and 4 with streaks of 2, 3 and 4 days and a star. Each day showed "Taco missed you!", gift boxes and poop. Biscuit's portrait stayed on the wall. Nothing stale or broken. On day 1 a tap on the shelf opened the library: "Biscuit's Big Day", book 1 of 3, NEW!, with Taco reading along at the bottom (`days/x1`-`x7`).
13. Paw Street from Taco's room shows Taco beside Mia's house (`probes/door.png`). From the empty room it shows the house with no dog (`probes/empty_door.png`). The farm and Biscuit do not appear on Paw Street.

## Gesture and interruption matrix

Probe traces: `traces/p_<name>.txt`. Frames: `probes/<name>.png`.

| Screen / control | Offset tap | Double tap | Long hold (1.2 s) | Drag off | Carried / quick follow-up |
| --- | --- | --- | --- | --- | --- |
| Stats: To the farm | bottom edge hits (`farm_edge_out`) | opens confirm once; 2nd tap lands on empty space (`farm_dbl`) | nothing (`farm_hold`) | cancels (`farm_drag`) | no button under the 2nd tap |
| Confirm: Yes | top-left corner hits (`yes_edge`); gap between buttons nothing (`gap`); below nothing (`outside`); top nothing (`top`, no home button; No! is the exit) | farm once, no carry into Go home (`yes_dbl`) | **nothing** (`yes_hold`) | cancels (`yes_drag`) | - |
| Confirm: No! | bottom-right corner hits (`no_edge`) | - | - | - | - |
| Farm: Go home | side/below nothing (`gh_side`, `gh_below`) | 2nd tap at 120/300 ms ignored (`gh_dbl`, `gh_tap_then_300`); held 2nd press ignored (`gh_carry`) | nothing (`gh_hold`) | cancels (`gh_drag`) | **2nd tap at 600 ms opens the Shelter** (`gh_tap_then_600`) |
| Empty room: Shelter | 3 px outside each side nothing (`o_left/right/above/below`); inner corners hit (`in_tl`, `in_br`) | 2nd tap at 120 ms ignored (`sh_dbl`) | nothing (`hold`) | cancels (`dragoff`) | **2nd tap at 600 ms hits Pick me! and opens naming with Connor** (`sh_600`) |
| Wall portrait (12 px frame) | 9-10 px off in each direction still opens it (`pic_l/r/u/d`) | - | - | - | - |
| Shelter arrows | about 14 px outside the left arrow still steps (`l_out`); between kennel and arrow nothing | steps twice (`r_dbl`) | nothing (`r_hold`) | - | - |
| Shelter: dog / plate / dots | nothing (`dog`, `plate`, `dots`) | | | | |
| Naming: OK | - | one welcome, not skipped (`keep_dbl`) | nothing (`ok_hold`) | - | - |
| Naming: ? | 13 px left of center lands on the name box: nothing (`23_q*`) | - | - | - | - |

For comparison, a long hold on the old home's food button and on the stat pips also does nothing (`hold_food`, `hold_stats`), so the hold behaviour matches the rest of the game.

| Interruption | Result |
| --- | --- |
| Home x2 from the shelter, launcher, reopen | empty room; the shelter choice resets to Connor (`25`-`27`) |
| Back out of naming (home) | shelter, same pup selected (`24`) |
| Process restart on the confirm card | Biscuit still home (`rs_confirm`) |
| Process restart 0.3 s after Yes | Biscuit at the farm, empty room: the choice is saved at once (`rs_farm_fast`) |
| Process restart 6 s after Yes | same (`rs_farm_6s`) |
| Process restart mid-naming, after 6 s | empty room with Shelter; no half-adopted pup (`rs_name`) |
| Leave from the launcher, a day passes, reopen | the picker shows first; Taco is fine (`days/`) |
| Process restart with a named pup | **untested gap** |
| Leaving from the farm scene | not possible: no home button, only Go home |

## Findings (prioritized)

### Major

**M1. The farm confirmation does not tell an 8-year-old that this is a permanent goodbye**
- **Screen**: `confirm_farm` (and its entry, Stats "To the farm").
- **Issue**: The card reads "Move Biscuit to the farm? It will be happy there!" It never says that Biscuit stays there, that he won't come home or that she'll pick a new pup. The cheerful second line pushes towards Yes. One tap on Yes is final, and nothing in the game brings Biscuit back. Mia read "To the farm" as a trip: on a green call-to-action button on her dog's page, it looks like an activity. She only learned the truth from the empty room. For a child with a month-long attachment, this is the moment that can turn a "fun trip" into "I gave my dog away by mistake".
- **Concern**: comprehension / meaningful choice / recovery.
- **Repro**: fixture; tap 80 67, 36 72 (Pets Club), 42 25 (pips), 80 77 (To the farm), 115 102 (Yes). Trace `traces/base_confirm.txt` + `tap 115 102`.
- **Screenshot**: `docs/pets-club/shelter-playtest/firstplay/08_stats.png`, `firstplay/09_farm1.png`, `firstplay/15_home_c.png`.
- **Suggestion (for the owner)**: say what happens in words and pictures before Yes, for example "Biscuit will live on the farm. You can pick a new pup!", and show the farm. Consider a second step or a goodbye moment, and a button label that is not a trip.

### Minor

**m1. No goodbye moment, and "Go home" is ambiguous**
- **Screen**: `farm`, then the empty `home`.
- **Issue**: The farm scene has no words. "Go home" can read as "Biscuit and I go home". The empty room then shows Biscuit missing with no line explaining it (for example "Biscuit is happy on the farm"). Mia works out where he went from a 12 px portrait. The scene is gentle and not scary, but abrupt.
- **Concern**: comprehension / feedback.
- **Repro**: as M1, then `tap 80 137`.
- **Screenshot**: `firstplay/13_yes_d.png`, `firstplay/15_home_c.png`.

**m2. Biscuit cannot be visited or petted after he leaves**
- **Screen**: `farm`, `wall` ("Farm friends").
- **Issue**: Tapping Biscuit on the farm, or his portrait on Farm friends, does nothing. Mia's first wish after the empty room is "can I go see him?" The wall answers "remembered" but gives no feedback to a tap.
- **Concern**: feedback.
- **Repro**: `traces/base_empty0.txt` + `tap 53 53`, `tap 80 77`. On the farm: `traces/base_farm.txt` + `tap 80 95`.
- **Screenshot**: `firstplay/17_portrait_tap2.png`, `firstplay/14_pat2.png`.

**m3. Go home, Shelter and Pick me! sit on the same spot, so steady repeat taps skip the empty room and the browsing**
- **Screen**: `farm`, then `home` (empty), then `shelter`.
- **Issue**: The three buttons overlap at about (80,128-148). Taps more than 450 ms apart (an impatient "is it working?" rhythm) go farm, empty room, Shelter, Pick me!, then naming the first pup, Connor. Mia never sees the empty room's portrait or the other six pups. Home on the naming screen recovers.
- **Concern**: consistency / pacing.
- **Repro**: `traces/p_gh_tap_then_600.txt` (farm: tap 80 135, tick 600, tap 80 135 lands in the shelter); `traces/p_sh_600.txt` (empty room: same, lands in naming with Connor).
- **Screenshot**: `probes/gh_tap_then_600.png`, `probes/sh_600.png`.

**m4. The naming screen gives no context, and "?" is a mystery**
- **Screen**: `name_pet`.
- **Issue**: With a suggested name there is no prompt ("Name your pup!") and no picture of the chosen pup. The pink "?" means "help" to most kids, but it actually picks a random name; that is fun once found, but it gets found by accident. A tap 13 px left of the "?" center lands on the name box and does nothing.
- **Concern**: comprehension.
- **Repro**: `traces/base_name.txt`; tap 110 28 does nothing (`23_q1`); tap 123 29 gives "Maple" (`32_q_empty`).
- **Screenshot**: `firstplay/22_pick_b.png`, `firstplay/32_q_empty.png`.

**m5. On an old dog's Stats page the farm button replaces the care bars**
- **Screen**: `stats`.
- **Issue**: From day 30, every Stats visit shows the big green "To the farm" button where the food/fun/sleep/clean bars were (compare `36_taco_stats`). Mia loses that readout for her own dog, and the page's most prominent control is now the one irreversible action. This makes M1 more likely to happen.
- **Concern**: consistency / meaningful choice.
- **Repro**: fixture, then the stat pips.
- **Screenshot**: `firstplay/08_stats.png` vs `firstplay/36_taco_stats.png`.

### Nits

- **n1. Confetti over the welcome title.** Confetti specks sit on "Taco!" in the welcome card's first frames. `firstplay/34_ok_a.png`.
- **n2. The splash still shows a golden dog after Biscuit left.** Reopening with an empty room shows a golden pup on the splash, which can read as "Biscuit's back!" just before the empty room. `firstplay/27_reopen_a.png`.
- **n3. The shelter forgets its place.** Leaving the game resets the shelter to Connor. `probes/in_tl.png`.
- **n4. "It" for Biscuit.** The confirm card calls Biscuit "It"; Mia would say "he/she".
- **n5. Library still stars "Biscuit" (pre-existing content, not new).** "Biscuit's Big Day" is a fixed character (a fresh dog named Pip gets the same title; supplementary check). After Biscuit leaves, Mia reads a "Biscuit" story with Taco at the bottom. That can be confusing or poignant. It is a coincidence of the fixture's name, not a bug. `days/x1.png`-`x7.png`.

## Qualitative assessment

**Can Mia recognize the goal, the actions and what changed?**
- **Shelter and naming: yes.** "Pick a pup!", the arrows, the dots and "Pick me!" are clear from the pictures, and the welcome card confirms the result.
- **Empty room: obvious.** Only Shelter and the portrait respond, and Shelter is the one big button.
- **Farm button: no.** "To the farm" does not tell her it is permanent (M1). She understands where Biscuit went only after the fact, from the empty room and the portrait. The words, not pictures, carry the decision, and the words are soft.

**Is the goodbye gentle, sad or scary?**
- Gentle and not scary: a sunny farm, a wagging dog, no crying.
- But there is no goodbye at all (m1). The sadness lands later, in a silent empty room.
- For a child who chose it knowingly, it is fine. For one who thought it was a trip, it is the most upsetting moment in the game.

**Does the empty room make the next step obvious?** Yes. One big Shelter button, nothing else live, and the portrait sits where the eye goes.

**Is browsing fun or tedious?**
- Fun. Seven clearly different dogs (spots, husky eyes, corgi, black lab), each with a name and breed, wrap-around arrows and dots.
- One lap is about 8 taps and well within Mia's patience. She browsed the whole set and went back for the corgi.
- The kennel dog does not react to a tap; a small missed delight.

**Is naming clear?**
- Mostly. The prefilled name, backspace, the grey "Pup's name" placeholder when empty, OK greying out and the welcome card all work.
- The "?" is a happy accident, and there is no prompt or pup picture (m4).

**Can she find Biscuit afterwards?**
- Yes, if she notices the 12 px portrait. It has a generous tap area, about 25x25, and opens "Farm friends", which she can read.
- He is not on Paw Street, and he cannot be visited (m2).

**Pacing and boredom**
- The new flow is short: Stats, confirm, farm, room, shelter, naming, welcome is about 8 deliberate taps.
- The waits are short, with no forced animations to sit through.
- The days that follow look like ordinary Pets Club: "missed you", gifts, poop, streaks.

**First voluntary stopping point**
- After "Welcome home, Taco!" and a minute of playing with the corgi at home: a satisfying close, about 4-5 minutes in.
- The risky earlier stop is the silent empty room right after Yes, if Mia did not mean to say goodbye. She might put the device down upset rather than go to the shelter.
- Everything after the welcome card (re-entering with every name variant, probing every edge, the three days) was coverage, not Mia's choice.

**Recovery from imprecise input and interruptions**
- Good. Holds and drag-offs never commit, a double tap never carries into the next screen within 450 ms, and No! is easy to hit.
- The farm choice is saved instantly. A restart mid-naming falls back cleanly to the empty room.
- One weak spot: the stacked buttons with slower repeat taps (m3).
- The only irrecoverable outcome is the intended one, which is why its wording matters (M1).

## Supplementary QA (separate from first play)

- `make -C apps/porthole playtest` at `91c5385`: **90 of 90 scenarios pass, 0 UI FAIL / 0 UI WARN**, including `b0_pets_farm_shelter` (92 checks), the monkeys (no sanitizer reports), `30_ui_audit` and `11_stats`. I read the whole `build/playtest/report.md`: the summary table, an empty findings table and the contact-sheet list.
- A raw `ui` dump over the fixture flow (`traces/ui.txt`, output `traces/ui.out`) gave these regions in logical px:
  - Stats: farm button 80x22.
  - Confirm: No! and Yes, each 50x24.
  - Farm: Go home 64x22.
  - Empty home: portrait 25x25, door 24x24, Shelter 72x24.
  - Shelter: arrows 32x32, Pick me! 64x22.
  - Naming: ? 24x24, keys 24x22.
  - Every target is at least 24x22. The widest text, "It will be happy there!" (x 8-152, y 66-74), stays inside the glass.
- One supplementary script run (`newgame Sam Pip` + `dbg books`) checked n5 only. No black-box decision used it.

## Brief improvements suggested

1. **Fix the first-play leak.** The auto-loaded `apps/porthole/CLAUDE.md` and `docs/playtesting.md`'s `dbg` table describe the farm, wall and shelter. A truly source-free first-play agent needs a working directory or brief that does not auto-inject the app brief, or the coordinator should accept and record this leak. A fresh agent with no such knowledge should re-verify the M1 and m1 comprehension claims after the fixes.
2. **Model Mia's understanding explicitly.** Say whether she knows the "sent to a farm" idiom (most 8-year-olds read "farm" literally) and how attached she is. The judgement on M1 depends on it.
3. **Ask for a process restart with a named pup** (an untested gap here). Allow plain multi-day ticking explicitly. It takes about 2.5 minutes of wall time per day, and the shell returns to the picker after the wait.
4. **Ask for the "did not mean it" path:** a child who taps Yes and then wants Biscuit back. The game offers no way, so the report should judge how that feels.

## Untested gaps

- A process restart after a pup is named and played with.
- Night or evening tint on the farm, shelter and wall screens.
- More than one farm friend on the wall (only Biscuit went this run).
- Two or more profiles: whether another kid's Paw Street or wall shows Biscuit.
- A dog younger than day 30 (does the farm button stay hidden?). Only Taco on day 1-4 was seen; it was hidden.
- Session rest or daily cap hitting mid-shelter.
- Physical device touch and a real child.

## Traces and reproduction

- `shelter-playtest/traces/drive.py TRACE OUTDIR` replays a trace against a fresh fixture copy through `snap --serve` and writes PNGs. Set `WD=<dir>` to run in an isolated save directory. `batch.py` runs the probes.
- Main flow: `main.txt`. Naming: `nameB.txt`, `nameB2.txt`, `nameC.txt`. Days: `days.txt`. Each probe: `p_<name>.txt`, self-contained (base plus probe lines).
- The early probes and the first days run shared `apps/porthole/build/host/` with other runs. The first days run desynced: it tapped the picker as if it were the launcher, so it was discarded. I reran the affected probes in isolated directories (`WD`) and got identical results. The days evidence comes from the isolated `days.txt` run (`days/`).

## Verdict

**Needs another pass.** The shelter, naming, wall and persistence work, are robust to imprecise fingers and are fun to browse. The automated suite is fully green. M1 (the irreversible farm choice is not explained to the child) is a Major comprehension problem and should be fixed. A fresh agent should then replay the Stats, confirm, farm and empty-room flow. m1-m5 are smaller improvements; fix them or defer them with a reason.
