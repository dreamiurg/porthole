# Launcher Home / All games: kid playtest 9 (star mode mashing, double taps, Zoe's Home)

Emulator roleplay by an independent playtester agent. These are simulated children, not real ones, and this was not run on a physical board.

## Build and setup

- Revision: `3d64856` (working tree clean at start). Binary: `apps/porthole/build/host/snap` copied into a private scratch folder; the run used that copy. sha256 `96f895b37c0c5dbebb39af5f2847a8dfb9af80a7e7c4c1632ce820075adfbea6`.
- Mode: `snap --serve --fresh`, default clock (Tuesday 16:00). Each probe ran in a fresh process with the same profile-creation prefix, then one burst or path. All input was `down`/`move`/`up`/`tick`, and I chose it from frames. No `dbg`, `app`, `profile`, clock skips or `ui` queries were used in play.
- Profiles were made through the screens: Leo (L-E-O, frog, age 6, skip code) and Zoe (Z-O-E, cat, age 8, skip code).
- Driver: a small stdin/stdout client of `--serve`. Its source is saved as `launcher-home-playtest-9/traces/driver-source.txt`, the burst generator as `burst-generator-source.txt`, and the frame classifier as `classifier-source.txt`. The classifier compares the header and arrow bands against reference frames I captured during play.
- Evidence: `apps/porthole/docs/launcher-home-playtest-9/shots/*.png` are labeled native 480x480 sheets. `apps/porthole/docs/launcher-home-playtest-9/traces/*.txt` holds every trace. `burst-results.txt` has the per-tap classification of every burst.

## Dispatch brief (verbatim)

> You are an independent child-perspective playtester for Porthole, a round 480x480 touchscreen with no buttons, emulated on the host. Follow apps/porthole/docs/playtesting.md and the `playtest` skill. Play source-free: don't read source (apps/porthole/games/, os/, shell/, host/), tests/playtests scripts, docs/superpowers/, or earlier launcher playtest reports until you finish. Disclose anything that loaded automatically. Decide from the screen. Run the sim from your own copy of the snap binary in a private scratch subfolder.
>
> ## Personas (two short passes)
> A. Leo, 6, can't read, taps firmly (200-400 ms presses), mashes 3-6 times, and taps "again!" about once a second.
> B. Zoe, 8, a confident reader who wants Rally and Pets Club first on her screen.
> Simulated perspectives only.
>
> ## Setup
> - `make -C apps/porthole snap`. Record `git rev-parse --short HEAD` and `sha256sum` of the snap you run.
> - `snap --serve --fresh`, default clock. Create the profile (age 6 or 8, no code) through the screens; ordinary touches only; no dbg, shortcuts or clock skips.
>
> ## Scope
> - In All games, the mode where games get gold stars (entered and left with the buttons at the top): mash the enter button and the done button in every rhythm Leo uses. Does each burst end where it was heading? Is the done button findable, and does Leo understand how to get out?
> - Double taps from the first screen into All games: does a game open that Leo didn't choose?
> - Zoe's goal: Rally and Pets Club first, one game removed. Time-to-goal and hesitations.
>
> ## Output
> Write apps/porthole/docs/launcher-home-playtest-9.md, with screenshots and the input trace in apps/porthole/docs/launcher-home-playtest-9/ (no .py files; save scripts as .txt). Include the revision and hash, the personas, this brief, findings ranked with repro steps, the assessment and gaps. Don't fix code or commit. Reply with ONLY (under 6 lines): a verdict (acceptable / needs fixes), the top findings, and the report path.

No revisions were made to the brief.

### Disclosures (what I read or saw)

- I read `apps/porthole/docs/playtesting.md` and the `playtest` and `tour` skills. I also read `apps/porthole/AGENTS.md` lines 110-120 (the snap modes) and the protocol part of `apps/porthole/tools/webemu.py` (lines 20-100: `down`/`move`/`up`/`tick`/`frame`), which I needed to write a driver. I read no game, shell, os or host source, no playtest scenarios and no earlier launcher reports.
- Auto-loaded: the repo-root `CLAUDE.md` loaded at the start; it is generic and has no launcher details. `apps/porthole/CLAUDE.md` loaded automatically when I opened `build/playtest/report.md`. That file documents the Library and star-mode design: newest star first, the 600 ms tile repeat, the 800 ms Library tile delay after arriving from Home, the house button restoring Home, and the stars hopping on "Home full". By then every burst probe and the Zoe path were already done and recorded. Three checks came after it, and I knew the design while running them: the pixel check that the stars hop (F1), the Pets Club re-star ordering (Z5), and the time sum. Comprehension claims in this report rest on the pre-exposure observations.

## What the screens are (as seen)

- **Home:** the kid's name bar, a sun/moon tile, five game pictures, and a sixth cream tile of tiny pictures ("All games"). Tiles have no captions; a finger on one shows the name in a yellow bar.
- **All games:** the title "All games", a star button top-right, six tiles per page with gold stars on the Home games, and a yellow arrow for page 2 (Starfighter, a princess game, a girl's face, Rally).
- **Star mode** (the star button; holding it says "Pick games"): the sky turns lavender and the tiles jiggle. A green check sits top-left (holding it says "Done") with five small stars beside it counting Home's places, and every tile has a gold or white star. The orange house stays at the top. Sheet: `shots/02_library_and_star_mode.png`.

## Pass A: Leo (6, non-reader, firm presses, mashes)

### Enter-button bursts (star button at logical 126,42)

Each burst started on All games after a 1.5 s look. Jitter was up to ±4 px between presses. A frame was taken during every press and after every lift.

| Burst | Presses | Press / gap (ms) | Page | Result after each lift | End |
|---|---|---|---|---|---|
| E1 | 6 | 250 / 120 | 1 | star mode from tap 1; taps 2-6 land on empty sky | star mode |
| E2 | 4 | 400 / 200 | 1 | same | star mode |
| E3 | 3 | 200 / 450 | 1 | same | star mode |
| E4 | 5 | 300 / 700 ("again!" 1/s) | 1 | same | star mode |
| E5 | 6 | 350 / 650 | 1 | same | star mode |
| E6 | 3 | 200 / 80 | 1 | same | star mode |
| E7 | 5 | 300 / 700 | 2 | same, stays on page 2 | star mode p2 |
| E8 | 6 | 250 / 120 | 2 | same | star mode p2 |
| P4 | 1 hold 1.2 s | — | 1 | — | star mode |
| P3 | press, drag off to centre | — | 1 | cancelled | All games (no change) |

### Done-button bursts (green check at logical 33,42)

These started in star mode after 1.5 s.

| Burst | Presses | Press / gap (ms) | Page | End |
|---|---|---|---|---|
| D1 | 6 | 250 / 120 | 1 | All games (normal) |
| D2 | 4 | 400 / 200 | 1 | All games |
| D3 | 3 | 200 / 450 | 1 | All games |
| D4 | 5 | 300 / 700 | 1 | All games |
| D5 | 6 | 350 / 650 | 1 | All games |
| D6 | 3 | 200 / 80 | 1 | All games |
| D7 | 5 | 300 / 700 | 2 | All games p2 |
| D8 | 6 | 250 / 120 | 2 | All games p2 |
| P2 | 1 hold 1.2 s | — | 1 | All games |
| P1 | press, drag off | — | 1 | stays in star mode (cancelled) |

All 16 enter/done bursts ended where they were heading, with no ping-pong. After the mode changes, the extra presses land on an empty spot: the sky where the star was, or the "All games" title where the check was. Sheets: `shots/03_enter_mash_E4_1persec.png`, `shots/04_done_mash_D4_1persec.png`.

### Double taps from Home's "All games" tile (logical 124,103)

The Library's sixth tile, Radar, sits exactly under the finger.

| Burst | Presses | Press / gap (ms) | End |
|---|---|---|---|
| H1 | 2 | 120 / 80 | All games |
| H2 | 2 | 250 / 150 | All games |
| H3 | 2 | 300 / 300 | All games |
| H4 | 2 | 300 / 450 | All games ("Radar" caption flashes on press 2, nothing opens) |
| H5 | 2 | 300 / 700 | All games (same caption flash) |
| H6 | 4 | 250 / 120 | **Radar opens** on press 4 |
| H7 | 3 | 350 / 650 ("again!" 1/s) | **Radar opens** on press 3 |

A single later press on a Library tile was ignored when it began 500 or 700 ms after arrival. It opened the game at 800 ms or later (G_* traces).

So a double tap never opens a game. Leo's own rhythm of three or more "again!" taps does open Radar, a game he never chose. Sheets: `shots/05_home_tile_triple_tap_H7_opens_radar.png`, `shots/06_home_tile_double_tap_H5_safe.png`. Radar's own small house returns to All games (R1).

### Leo's comprehension

- **Getting in:** the big star tile is the most attractive thing on All games, so Leo presses it. The lavender sky and jiggling tiles clearly say "something changed".
- **Inside:** tapping a tile flips its star between gold and white at once (`shots/08_leo_star_mode_explore.png`), which is understandable cause and effect. Mashing one tile acts once when the presses are under about 0.6 s apart (M1). At 1/s each press toggles, so 4 presses bring it back (M2) (`shots/14_star_mode_tile_mash_M1_M2.png`).
- **Refused star:** tapping a white star when Home is full shows "Home full! Tap a star", which Leo cannot read. The gold stars on the tiles hop by a few pixels, but that is hard to tell apart from the jiggle all the tiles already do (`shots/13_home_full_frames_leo.png`). Leo sees "nothing happened".
- **Getting out:** the button he came in on is gone, and pressing its old spot does nothing. The two exits are a big green check (a familiar "OK" mark) and the house. Both are visible, large and at the top. Leo would find one within a few tries, and either one gets him out. The check is findable. Whether Leo knows the check *keeps* his stars and the house *undoes* them is a different matter (see M1 below); he cannot.
- **House mashing** climbs one level per press: star mode, then All games, then Home, then "Who's playing?" (K1, K3, K4; `shots/07_house_mash_K4_to_picker.png`). Two fast presses stay on All games (K5). The picker has no house, but it shows Leo's frog, which he recognizes.
- **Where Leo stops:** after a minute or two of flipping stars, or after the first "Home full" refusal he can't decode. He would leave by the check or the house and tap a game picture. That is a fine natural stopping point: nothing is lost, and the games are still there.

## Pass B: Zoe (8, reader): Rally and Pets Club first, one game removed

Path (`traces/Z1.txt` to `Z3.txt`; `shots/09_zoe_path.png`), starting on Zoe's Home:

1. She taps the tile of tiny pictures (she spots a red car in it), which opens All games p1. Rally isn't there.
2. She taps the arrow to p2 and sees the red car (Rally), still without a star.
3. She taps the star button (gold stars clearly mean "on Home"), which enters star mode on p2.
4. She taps Rally, which shows "Home full! Stars: page 1". **Hesitation 1:** she reads it and understands she must take a star off first. The page hint helps.
5. Back to p1, she taps Pumpkin (star turns white; counter 4/5). That is "one game removed".
6. Arrow to p2, then Rally (gold, counter 5/5).
7. She taps the green check. **Hesitation 2:** check or house? I took the check.
8. She taps the house. Home shows **Rally, Pets Club**, Biscuit, Marble, Tilt FC, All games. Goal reached.

That was 10 taps and 16.2 s of simulated time with short looking pauses; a real 8-year-old reading the captions would take perhaps 30-45 s. Seven taps are enough if she knows to unstar first. She did not have to learn the newest-first order, because Rally landed first anyway. To put Pets Club in front, she would have to unstar and re-star it (Z5, `shots/12_zoe_restar_pets_first.png`), and nothing on screen tells her that.

Wrong choices tried:

- **Long-pressing Rally in normal mode**, as on a phone, to "pin" it: the caption shows while held, then Rally opens on lift (`shots/11_zoe_long_press_rally.png`). Recoverable with Rally's own Home button.
- **Tapping the house instead of the check after her edits** (Z4, `shots/10_zoe_house_discards_edit.png`): she lands on All games with Rally unstarred and Pumpkin starred again, and Home is unchanged. Holding the house shows no caption, while the star and the check both have one. Nothing says the edit was dropped.

## Findings (ranked)

### Major

**M1. The house in star mode silently throws away the kid's edits**
- **Screen:** All games, star mode.
- **Issue:** after starring Rally and unstarring Pumpkin, pressing the house returns to All games with every star restored, and Home is unchanged. There is no caption on the house press and no message.
- **Concern:** comprehension / recovery / feedback.
- **Repro:** `launcher-home-playtest-9/traces/Z4.txt` (Zoe path to Rally starred, then `down 80 18`, `tick 150`, `up`, then the house again).
- **Screenshot:** `apps/porthole/docs/launcher-home-playtest-9/shots/10_zoe_house_discards_edit.png`.
- **Note:** the house is the "I'm done" button on every other screen. An 8-year-old who just did her task will plausibly press it and conclude "it didn't work". `apps/porthole/CLAUDE.md` (auto-loaded after the observation) says this is intended ("the home button ... puts Home back as it was"), so this is a design call. From the child's seat, though, it is a silent loss of the only task in this mode.
- **Suggested fix:** keep the stars on the house too, or at least show a caption or a brief "not saved" cue on the house press in star mode.

### Minor

**m1. Leo's "again!" mash on Home's "All games" tile opens Radar**
- **Screen:** Home → All games.
- **Issue:** the 800 ms Library-tile guard covers double taps, but a third 1/s tap (H7) or a fourth fast tap (H6) on the same spot opens Radar, the Library tile under that spot. Leo never chose it. Press 2 also flashes a "Radar" caption that Leo can't read.
- **Concern:** recovery / feedback.
- **Repro:** `traces/H7.txt` (setup prefix; `tap 124 103` ×3, 350 ms presses, 650 ms gaps) and `traces/H6.txt`.
- **Screenshot:** `apps/porthole/docs/launcher-home-playtest-9/shots/05_home_tile_triple_tap_H7_opens_radar.png`.
- **Impact:** recoverable in one tap (Radar's house returns to All games). On a device, opening Radar starts its network feed. A timing guard can't tell this apart from a deliberate tap, so a layout fix would work better: don't put a game tile under Home's "All games" spot on Library page 1.

**m2. "Home full!" is text-only for a non-reader**
- **Screen:** star mode.
- **Issue:** a refused star shows a yellow text box. The gold stars hop, but by a few pixels while every tile already jiggles, so Leo sees no reason the star refused.
- **Concern:** comprehension / feedback.
- **Repro:** `traces/F1.txt` (star mode p1 on a fresh profile, tap Radar 124,103).
- **Screenshot:** `apps/porthole/docs/launcher-home-playtest-9/shots/13_home_full_frames_leo.png`.

**m3. House mash in star mode climbs to "Who's playing?"**
- **Screen:** star mode → All games → Home → picker.
- **Issue:** each house press goes up a level, so a 3-6 press mash lands on the profile picker, which has no house.
- **Concern:** recovery.
- **Repro:** `traces/K4.txt` (star mode, `tap 80 18` ×5, 300 / 700 ms), also K1 and K3.
- **Screenshot:** `apps/porthole/docs/launcher-home-playtest-9/shots/07_house_mash_K4_to_picker.png`.
- **Impact:** one tap on the kid's own face recovers. Consistent with the house elsewhere, so this is low.

### Nit

**n1. Leo can't toggle star mode off where he turned it on**
- **Screen:** star mode.
- **Issue:** the star's spot is empty in star mode, so pressing there does nothing.
- **Concern:** comprehension.
- **Repro:** E1-E8.
- **Screenshot:** `apps/porthole/docs/launcher-home-playtest-9/shots/03_enter_mash_E4_1persec.png`.
- This is the right trade-off for mash safety; the check and the house are both visible exits.

**n2. Home order (newest star first) is invisible**
- **Screen:** star mode / Home.
- **Issue:** putting Pets Club ahead of Rally needs an unstar and re-star, with no cue that this works.
- **Concern:** meaningful choice / comprehension.
- **Repro:** `traces/Z5.txt`.
- **Screenshot:** `apps/porthole/docs/launcher-home-playtest-9/shots/12_zoe_restar_pets_first.png`.

**n3. Long-pressing a Library tile opens the game on lift**
- **Screen:** All games.
- **Issue:** a phone habit ("hold to edit") opens the game instead.
- **Concern:** consistency.
- **Repro:** `traces/Z1a.txt`.
- **Screenshot:** `apps/porthole/docs/launcher-home-playtest-9/shots/11_zoe_long_press_rally.png`.
- The caption during the hold is good feedback; recovery takes one tap.

Positives worth keeping:

- The press captions ("Pick games", "Done", game names).
- The lavender sky and jiggle.
- The check and star are on opposite sides, so mashing either acts once.
- Within-600 ms tile repeats count once.
- The refusal names the page that holds the stars.
- Drag-off cancels both buttons.

## Assessment

- **Can they recognize the goal, the actions and what changed?** Leo can tell he entered a different mode (colour plus jiggle), and that tapping a tile flips its star. He cannot understand refusals (m2) or which exit keeps his stars (M1). Zoe reads everything. Her one stumbling block is the check versus the house (M1); the refusal message itself helped her.
- **Choices, feedback, pictures vs reading:** star flips and the counter stars are pictorial and immediate. Refusal and ordering depend on reading (m2, n2).
- **Repetitive, confusing or frustrating moments:** for Leo, the refusal: he would press Radar's white star several times and then give up. For Zoe, a silent undo if she uses the house. Neither persona hits any waits.
- **Recovery from imprecise input and interruptions:** every mash ended somewhere sensible, every screen reached has a way back, and nothing persistent is lost by a mistake. A cancelled edit can be redone. A Radar that Leo didn't choose takes one tap to leave.

**Gesture / interruption matrix**

| Case | Status |
|---|---|
| Tap, ±4 px jitter | tested |
| Fast mash (80-200 ms gaps) | tested |
| 1/s "again!" | tested |
| Long press 1.2 s (enter, done, tile) | tested |
| Drag-off (enter, done) | tested |
| Double/triple tap Home → Library | tested |
| House mash from star mode | tested |
| Tile mash in star mode | tested |
| Slide onto a button from elsewhere | gap |
| Page-arrow mashing | gap |
| Leave mid-star-mode via rest screen / profile switch | gap |
| Process restart mid-star-mode | gap |
| Offsets > 4 px around button edges | gap |
| Multi-touch | gap |
| Physical device | gap |

## Automated suite

I ran `make -C apps/porthole playtest` at `3d64856`: 93 of 93 scenarios pass, with 0 UI FAIL and 0 UI WARN. I read `build/playtest/report.md` in full; its findings table is empty. I did not dump `ui` geometry for the star-mode screens.

## Verdict

**Needs fixes, or a recorded design decision on M1.** Every enter and done burst in Leo's rhythms ended where it was heading, the done button is findable, and double taps from Home never open a game. Zoe reached her goal in 10 taps. M1 (the house silently undoing star edits) is Major from the child's seat. m1 (a 3-tap mash opens Radar) and m2 (a text-only refusal for non-readers) should be fixed or explicitly deferred.

## Iteration history

None in this pass (single run on `3d64856`). No game fixes were requested or applied, and the brief was not revised.
