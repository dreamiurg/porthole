# Pets Club farm and shelter: child playtest, round 3 (narrow retest)

Emulator roleplay by an independent agent. This is not a study with a real child and not a test on the physical device.

## Build and setup

- Revision: `2778979` (clean tree). `make -C apps/porthole snap` reported it was already up to date.
- `build/host/snap` sha256: `81ee22b289f8eadf5b88acb5bd927d5f1731de809447b30c8e9919c0b3bda0ea` (the same binary before and after `make playtest`).
- Fixture: `.superpowers/sdd/2026-11-08-pets-club-shelter/fixture/*.sav`, copied over `build/host/` before every fresh start. Hashes:
  - crago-s0 `a048df7b...`
  - porthole-m and porthole-mb `4bf5122f...`
  - porthole-p0 `845f86e0...`
  - sys-ran `ef260e9a...`
- The fixture holds one profile, "Mia" (age 8 on Stats). Its dog Biscuit shows "Grown dog - day 31". Roleplay treats the profile as Ava's own.
- Harness: `snap --serve` without `--fresh`, from `apps/porthole/`, at the default clock (Tuesday 16:00). It was driven through a small socket relay that logs every line.
  - The serve protocol works in logical px. Every `down`, `move`, `up` and `tick` frame is 33 ms.
  - `tap X Y [ms]` in the trace expands to `down`, then `tick ms-33`, then `up`.
  - Only ordinary touches and elapsed `tick`s were used: no `dbg`, `app`, `skip` or hidden-state queries.
  - Relay and helper scripts are saved as `.txt` in the evidence folder.
- Evidence: native 480x480 frames and `input-trace.txt` (six runs, with process restarts marked) are in `docs/pets-club/shelter-playtest-3/`.

### Disclosure

- I did not read game, os or shell source, the `tests/playtests` scripts or `docs/superpowers/`.
- To learn the `--serve` wire protocol, I read the serve loop in `host/sim.cpp` (about 40 lines) and `tools/webemu.py`'s input mapping.
- **Leak after play:** reading a file under `apps/porthole/` auto-loaded `apps/porthole/CLAUDE.md`. It describes the farm rule, the 5 s save cadence with an `App::saveUrgent()` checkpoint, `FreshGate` (450 ms) and the 600 ms long press. This came after I had already observed all of these from the screen; I used it only to explain findings m1 and m2.
- I read the round 1 and round 2 reports only after play.
- The `ui` geometry dump and the `b0_pets_farm_shelter` contact sheet are supplementary QA and were done after the black-box play.

## Persona and brief (verbatim scope)

**Persona:** Ava, 6, a kindergartener who reads very little: a few words and her dog's name. She has played Pets Club with Biscuit. She taps hard and fast, often mashes, presses too long or too short, and loves petting animals. This is a simulated perspective.

**Brief as dispatched (scope):**

1. The "send Biscuit to the farm" question that a grown-up or older kid would reach from Biscuit's Stats page.
   - Can a mashing 6-year-old send Biscuit away by accident?
   - Try fast taps, double taps, mashing all over, a short press on Yes, a press that slides off Yes, and a press that starts on No.
   - Can a kid who really means it (and is told by a grown-up "hold Yes") do it?
   - Is the feedback while holding clear without reading?
2. On the farm after saying goodbye: what does Ava do with the dog there? Does patting feel nice? Is "Go home" findable?
3. Later, the farm friends Ava can find from the room: can she find them, visit one, pat it and get back? Try a fast double tap on the little pictures on the room wall.

No brief revisions in this round.

## Paths played

### Run 1: first play, curiosity-led (`001`-`045`)

1. Picker → Mia → Pets Club tile → "tap to start" → room (`004`).
2. Ava pats Biscuit first. She gets hearts and a happy dog (`005`).
3. She taps the paw (tricks) and the bookshelf heart (stories), then the stat bars → Stats (`010`). She cannot read "To the farm", but it is a big light-blue button, so she taps it.
4. Confirm card (`011`): green **No!**, brown **Yes**, "Hold Yes" in yellow, and a sad-looking sitting Biscuit.
   - A quick tap on Yes does nothing except flash "Hold Yes" white (`012`).
   - 20 mashed taps on Yes (66 ms each) and a fast double tap do nothing (`014`).
5. Then one firm press on Yes. A plum fill sweeps over Yes from the left (`016`, about 300 ms). At about 630 ms, **while the finger is still down**, the farm appears (`017`, `018`). Biscuit was gone before I meant to let go.
6. Farm (`018`): "Biscuit lives here now!", a "Biscuit" sign, a fence, sun, a cloud and a big green **Go home**.
   - Tapping Biscuit makes him jump with his tongue out and drops two hearts (`021`, `022`).
   - Tapping the sign, sun or grass does nothing (`028`).
7. Go home → empty room with a big purple "Shelter" sign and a little framed Biscuit on the wall (`031`, `032`).
8. A fast double tap on the picture opens "Farm friends" with one framed Biscuit (`034`). The second tap does not chain.
   - Tapping the frame opens the visit: the same farm with a home button and no Go home (`035`). A tap pats him (`036`).
   - Home → Farm friends (`037`) → home → room (`038`).
9. Shelter → Pick a pup → next → "Pick me!" → name keyboard → OK → welcome → room with Boxy and Biscuit's picture still on the wall (`039`-`044`).
10. Offset taps on the picture:
    - (42..64, 42..64) all open Farm friends (`run1_probe_pic_64_53`).
    - (38,53) opens Paw Street through the door (`run1_probe_pic_38_53_door`).
    - (68,53) does nothing.

### Run 2: measuring the hold frame by frame (`110_h*`)

- The fill starts at about 66 ms and grows about 9 physical px per 33 ms frame. It reaches 141/146 px at 627 ms.
- The farm shows at **660 ms**, without a release.

### Run 3: cancel and mash probes (`201`-`213`, `mash_seed*`)

- **C:** two 400 ms presses on Yes, 100 ms apart. Progress resets on release (`201`); there is no accumulation.
- **B:** press Yes for 400 ms, slide down off it and hold 1 s, then release. Cancelled; the bar empties (`203`).
- **Slide off and back on:** the hold restarts from zero (`205`) and completes 650 ms after returning (`206`).
- **E:** press on No, slide onto Yes and hold 1.4 s. Nothing happens (`210`), and No does not fire on release either.
- Press on the text or below Yes, then slide onto Yes and hold 1.2 s: nothing.
- **Yes hold area:** a 1 s hold commits at (90,102) and (115,90). It does nothing at (86,102), (115,86), (115,116), (115,120), (140,102) and (144,102). This matches the visible button.
- **Mash all over:** 3 seeds × 120 random taps inside the circle, 40-300 ms each, starting on the confirm. They wandered through No, Stats, Hats, Spelling and back. **Biscuit stayed home in all three runs** (`mash_seed1..3`).
  - Mash scripts: `mash-seed*.txt`.

### Run 4: process restart, which differs from leave/resume (`300`-`33x`)

- Yes, then restart within 0.3-3 s on the farm, or 1.3 s after Go home: **Biscuit is back in the room** with bars and buttons, and there is no wall picture (`303`, `311`, `33x_restart_farm_3000`).
- Restart after 5 s or 60 s: empty room plus wall picture (`32x_restart_farm_5000`, `310`).

### Run 5: long presses and double taps in the room (`440`-`481`)

- **Double tap on Go home:** stops in the empty room (`440`).
- **Wall picture:** presses of 150-500 ms open Farm friends. Presses of **600 ms to 2 s do nothing** (`46x_pic_600`, `441`). The same is true for the Shelter sign (`470`) and the paw button with a dog present (`481`), so this is game-wide behaviour, not new to the wall.

### Run 6: petting gestures on the farm (`500`-`504`)

- Holding Biscuit for 300 or 900 ms, or rubbing back and forth across him: no hearts and no reaction (`500`-`503`). Only a short tap pats.
- A double tap on the visit's home button stops at Farm friends (`504`).

## Gesture and interruption matrix

| Control | Offset | Fast/double tap | Mash | Long press | Drag off / onto | Carried touch | Notes |
| --- | --- | --- | --- | --- | --- | --- | --- |
| Confirm: Yes (hold) | Hold area = visible button | No effect (`012`, `014`) | Safe in 3 × 120 random taps | **Commits at ≈650 ms, finger still down** (`110_h660`) | Off: cancels (`203`). Back on: restarts and commits (`206`) | Starting on No/text and sliding on: never commits (`210`) | M1 |
| Confirm: No | Not offset-tested | Exits to Stats | Exits often during mash | Gap | Off: no fire (`210`) | Gap | |
| Farm: Biscuit | Tap anywhere on the body pats | Each tap pats | n/a | No reaction (`501`) | Rub: no reaction (`503`) | n/a | m3 |
| Farm: Go home | Not offset-tested (round 2 covered it) | Stops in the room (`440`) | n/a | Gap | Gap | Commit-carry lands on the dog, not Go home | |
| Wall picture | ±11 px hits; (38,53) is the door | Opens Farm friends once (`034`) | n/a | **≥600 ms ignored** (`441`) | Gap | n/a | m2 (game-wide) |
| Farm friends card | Centre | Second tap becomes a pat in the visit | n/a | Gap | Gap | n/a | |
| Visit home | n/a | Stops at Farm friends (`504`) | n/a | Gap | Gap | n/a | |
| Process restart within 3 s of Yes | n/a | n/a | n/a | n/a | n/a | n/a | m1: move lost, Biscuit back |
| Process restart ≥5 s after Yes | n/a | n/a | n/a | n/a | n/a | n/a | Kept |
| Ordinary leave/resume mid-farm | Not possible from the farewell (no home button, only Go home) | | | | | | Gap |

Untested gaps:

- Several farm friends. This needs more grown dogs and a clock skip, so it is outside natural play. The automated contact sheet shows four frames in a 2×2 Farm friends grid.
- Night farm.
- Tilt.
- Physical-device touch timing.

## Findings (prioritized)

### Blocker

None.

### Major

**M1. A single firm press on Yes sends Biscuit away in about 0.65 s, with no release needed**

- **Screen:** `confirm_farm`
- **Issue:** The "hold" is so short that a 6-year-old's ordinary firm or slow press counts as consent. Ava cannot read "Hold Yes".
  - The only cue on a short tap is that the text flashes white (`012`), so the way she discovers holding is by pressing longer, which is exactly the accident.
  - The plum fill over brown is visible, but it lasts about 0.6 s and commits mid-press with no "done" beat. Ava has no time to notice and lift.
  - Elsewhere in this game, presses of 600 ms or more are ignored (wall, Shelter, paw: `441`, `470`, `481`). So the one control that rewards a long press is the hardest to undo.
  - What does work: quick taps, double taps and random mashing are all safe (360 random taps, 0 sends), and drag-off cancels.
- **Concern:** recovery / comprehension / feedback
- **Repro:**
  1. Fresh fixture, picker (80,67), tile (36,72), title (80,100), bars (45,22), "To the farm" (80,77).
  2. `down 115 102`, then `tick 627`: the bar is full and you are still on the confirm.
  3. One more frame: the farm.
  - Trace: `input-trace.txt` run2.
- **Screenshot:** `docs/pets-club/shelter-playtest-3/016_hold300.png`, `110_h594.png`, `110_h660.png`, `017_hold600.png`
- **Suggestion for the owner:**
  - Make the hold long enough that it cannot be an ordinary press: roughly 1.5-2 s, so the fill visibly takes time.
  - Draw a picture cue a non-reader understands, for example a finger with a ring that fills.
  - Possibly end with a short "Bye, Biscuit!" beat after release.
  - The coordinator should decide the exact length. The current behaviour does not meet "only on the kid's confirmed choice" for this persona.

### Minor

**m1. The farm move survives a restart only after the 5 s autosave**

- **Screen:** `farm`, then `home`
- **Issue:** If the process restarts within about 3 s of committing, the move is lost and Biscuit is home again with no wall picture. This happens whether the restart comes on the farewell or right after Go home. A choice this big should be checkpointed immediately.
  - The result is gentle (the dog comes back), but it contradicts "Biscuit lives here now!".
  - On a battery or USB unplug it is rare.
- **Concern:** consistency / recovery
- **Repro:**
  1. Fresh fixture to the confirm.
  2. `down 115 102`, `tick 1000`, `up`, `tick 3000`.
  3. Kill `snap` and start it again without restoring the fixture.
  4. Mia → Pets Club → tap: Biscuit is in the room. With `tick 5000`, the empty room shows instead.
- **Screenshot:** `docs/pets-club/shelter-playtest-3/33x_restart_farm_3000.png`, `311_restart_after_gohome.png`, `32x_restart_farm_5000.png`

**m2. Long presses on the wall picture (and every room button) do nothing, with no feedback**

- **Screen:** `home` (the wall picture), `wall`
- **Issue:** Ava "presses too long". A press of 600 ms or more on Biscuit's picture fails silently, and a short tap works. This is pre-existing and game-wide, not new to this change, but it makes Farm friends feel unreliable to a firm presser.
- **Concern:** feedback / consistency
- **Repro:** empty room or a room with a dog; `down 53 53`, `tick 600`, `up` → nothing. Then `tap 53 53` → Farm friends.
- **Screenshot:** `docs/pets-club/shelter-playtest-3/441_pic_long.png`, `46x_pic_600.png`, `481_paw_long.png`

**m3. On the farm, only a quick tap pats Biscuit; holding or rubbing does nothing**

- **Screen:** `farm` (the farewell and the visit)
- **Issue:** Round 2's m1 is fixed for taps: he jumps, sticks his tongue out and drops hearts (`021`, `036`). But stroking and holding a finger on the dog are what a 6-year-old who "loves petting" does first, and they get nothing.
- **Concern:** feedback
- **Repro:** farm after Yes; `down 80 98`, `tick 900`, `up` → nothing. Then a drag from 60,98 to 100,98 and back → nothing.
- **Screenshot:** `docs/pets-club/shelter-playtest-3/501_farm_hold900.png`, `503_farm_rub.png`

### Nits

- **n1. Pat hearts fall downward** from the tap point toward Go home, rather than rising (`024_h0`-`026_h2`, `022_farm_pat3`). This is cosmetic, and the room does the same.
- **n2. The confirm still has no pictorial meaning for a non-reader.** Only "Biscuit" and the green/brown colours read. The sitting dog does not show a farm. The green "No!" is the more attractive button, which is the safe side (`011`).
- **n3. Out of scope:** "Read to me!" on the book shelf touches its button's left and right borders (`009_heart`).

## Qualitative assessment

**Can Ava recognize the goal and actions, and tell what changed?**

- **Room and farm:** yes.
  - She pats Biscuit on the farm and sees hearts.
  - "Go home" is the only button, big, green and at the bottom; she finds it at once (`018`).
  - The empty room's wall picture is the one small "Biscuit" thing she recognises. She taps it out of curiosity, and Farm friends and the visit work with a clear way back (home → Farm friends → room, `037`, `038`).
- **Confirm:** no.
  - She reads only "Biscuit". She does not know what Yes does or that it must be held.
  - What changed after a commit is clear (the farm), but by then it is done.

**Decisions, feedback, pictures vs reading**

- The farm and wall are picture-led: the sign, Biscuit and framed photos.
- The confirm is entirely reading-led. Its only motion feedback, the fill, is too brief to act on (M1).
- Patting feedback is nice for taps but absent for strokes (m3).

**Where play becomes repetitive or frustrating, and where Ava stops**

- Run 1, after about three pats on the farm and one visit, is Ava's natural stopping point. There is little else to do there: the scenery is inert and the dog only reacts to taps. A visit lasts about 10-15 s before she goes home.
- If she had sent Biscuit by accident, she would be upset at the empty room. The big Shelter sign redirects her, but she would likely look for a way to bring Biscuit back. There is none, only visits.
- Everything after run 1 was coverage-driven, not play she would choose.

**Recovery from imprecise input, wrong guesses or interruption**

- Mash, double taps, drag-off and carried touches are all handled well.
- Long press is the outlier: it does nothing everywhere except on the one irreversible control, where it commits.
- A restart soon after the commit silently undoes it (m1).

## Supplementary QA (separate from first play)

**`make -C apps/porthole playtest`**

- Result: 90 of 90 scenarios PASS, 0 UI FAIL / 0 UI WARN. `build/playtest/report.md` was read in full, including the scenario table, the empty UI findings table and the contact sheet list.
- `b0_pets_farm_shelter`: 113 checks pass.
- Its wall sheet shows four farm friends in a 2×2 grid with names fitting.

**`ui` dump** (`ui-tour-supplementary*.txt`)

- confirm_farm: No (20,90,50×24) and Yes (90,90,50×24).
- farm: Go home 64×22; the dog's pat area is 72×54.
- Room: wall picture 25×25, next to the door 24×24 with a 2 px gap.
- Farm friends: card 46×30.
- All texts are inside the glass. The widest is "Biscuit will live there." at x 9-152, y 54, which is 75.6 px from the centre.

## Iteration history

- **Round 1** (`shelter-playtest.md`) and **round 2** (`shelter-playtest-2.md`, `8a05c93`): see those reports. Round 2's m3 (a masher can send the dog with three taps) and m1 (no patting on the farm) led to this round's hold-Yes and farm patting.
- **Round 3** (this report, `2778979`):
  - Mash safety is fixed: 0 of 3 × 120 random-tap runs sent the dog.
  - Tap patting is fixed.
  - The new problem is that the hold commits too fast for this persona (M1).
  - No code changed during this round, and I fixed nothing.

## Verdict

**Needs fixes.** M1 is a Major for a 6-year-old who presses firmly. Retest M1 and m3 on the fixed build with a fresh agent. m1 (immediate checkpoint) and m2 (long press game-wide) can be fixed or deferred with rationale.
