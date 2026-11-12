# Launcher All games: star/check button, kid playtest 8 (final narrow retest)

This is emulator roleplay by an independent playtester agent. It is not a study with a real child, and it was not run on a physical device.

## Build

- Revision: `7ca8c01` ("fix(porthole): cap the star/check mashing guard at 3 s so a 1 Hz retry gets in"). The tree was clean apart from this report.
- Binary: `apps/porthole/build/host/snap`. `make -C apps/porthole snap` said it was up to date. I copied it to a private scratch subfolder (`lhp8/run/`, with its own `build/host/` for saves) and ran only that copy.
  - sha256 `93ae36fe2f7f7251e1df36066b4806eb11bd34addc4f99416b3a697d89a5d79a` (the copy and the build output are identical).
- Mode: `snap --serve --fresh`, default clock. Each probe is a fresh process: the same setup prefix (profile creation through the screens) plus one burst.
  - The driver sends `down X Y` / `up` / `tick MS` / `frame` and saves native 480x480 PNGs. While "watching", it steps 40 ms at a time and logs every change in the sky colour (light blue = normal, lavender = star mode), the yellow caption over the header, the button icon and a hash of the button's pixels.
  - Timings are nominal, to about ±40 ms.
- Automated suite: `make -C apps/porthole playtest` at `7ca8c01`. 93 of 93 scenarios passed, with 0 UI FAIL and 0 UI WARN. I read `build/playtest/report.md` in full; its findings table is empty.

## Persona

- **Leo, 6.** Can't read. Taps firmly (200-400 ms presses) and mashes. When something doesn't respond, he taps "again!" about once a second.
- **His sister, 9.** Taps slowly and on purpose.

This is a simulated perspective only.

## Dispatch brief (verbatim, no revisions)

> You are an independent child-perspective playtester for Porthole, a round 480x480 touchscreen with no buttons, emulated on the host. Follow apps/porthole/docs/playtesting.md and the `playtest` skill. Play source-free: don't read source (apps/porthole/games/, os/, shell/, host/), tests/playtests scripts or docs/superpowers/. Disclose anything that loaded automatically. Decide from the screen. Run the sim from your own copy of the snap binary in a private scratch subfolder.
>
> ## Persona
> Leo, 6, can't read, taps firmly (200-400 ms presses), mashes, and taps "again!" about once a second when something doesn't respond. His sister (9) taps slowly and on purpose. Simulated perspective only.
>
> ## Setup
> - `make -C apps/porthole snap`. Record `git rev-parse --short HEAD` and `sha256sum` of the snap you run.
> - `snap --serve --fresh`, default clock. Create Leo's profile (age 6, no code) through the screens; ordinary touches only; no dbg, shortcuts or clock skips.
>
> ## Scope (final narrow retest of the top-right star/check button in All games)
> - Bursts of double and triple taps and 5-tap mashes on the star, then on the check. Does each burst end where it was heading?
> - Leo's "again!" taps about once a second on the button right after a mode change: how long until something happens, and does it ever feel broken?
> - The sister's single deliberate taps, including a quick change of mind (about 0.5 s after entering).
> - A minute of normal play afterwards.
>
> ## Output
> Write apps/porthole/docs/launcher-home-playtest-8.md, with screenshots and the input trace in apps/porthole/docs/launcher-home-playtest-8/ (no .py files; save scripts as .txt). Include the revision and hash, the persona, this brief, findings ranked with repro steps, and gaps. Don't fix code or commit.

## Disclosures (what I knew beyond the screen)

- **Auto-loaded:** the repo `CLAUDE.md`, and `apps/porthole/CLAUDE.md` (the `AGENTS.md` brief).
  - The app brief describes the intended star/check rule: a press there is ignored if it begins within 1 s of a mode change or of the lift of a press it already ignored, and within 3 s of that change.
  - It also describes the 600 ms tile double-tap rule and `FreshGate` (450 ms after a screen change).
  - The HEAD commit title says the guard is now capped at 3 s.
  - I used this knowledge to choose probe cadences around the boundaries. I did not use it to decide what Leo or his sister would do. Every result below was measured on screen, and two of them contradict the documented numbers.
- **Read on purpose:**
  - `docs/playtesting.md`, the `playtest` and `tour` skills.
  - `launcher-home-playtest-7.md`, including its driver (`traces/driver-source.txt`), to learn the `--serve` protocol. I wrote my own driver from it.
  - I read no game, shell, host, tool or test-scenario source.
- **Scratch folder:** the session scratchpad holds many files from earlier agents, including copies of shell source. I did not open any of them and worked only in my own `lhp8/` subfolder.
- **No hidden state:** no `dbg`, `debug`, `screen`, `ui` or `app` commands. The mode was judged from the frames only.

## Setup path (ordinary touches; `traces/setup.txt`, `sheets/setup.png`)

1. Fresh device: name keyboard. Typed L, E, then N-Z, O, OK ("Leo").
2. Picked the frog face, age 6, then "skip" the code.
3. Home showed five games and the "All games" tile (bottom right, six tiny pictures). Leo tapped it.

In All games, the top-right square button shows a gold star (logical centre about (126,43)). This build has 10 games, so the Library has a second page behind the yellow arrow.

## What happened

### 1. Short bursts on the star (normal mode: aim is to get into star mode)

| Burst (press / gap) | Ends in | Extra presses look pressed? | Evidence |
| --- | --- | --- | --- |
| Double 300/150, 300/400, 400/200 | star mode | no | `sheets/A1_star_double.png`, `logs/A4_*`, `logs/A5_*` |
| Triple 300/150, 300/500 | star mode | no | `sheets/A2_star_triple.png`, `logs/A6_*` |
| Mash of 5, 200/100 | star mode | no | `sheets/A3_star_mash5.png` |

- The first press shows the yellow "Pick games" caption and a pressed star. On the lift the sky turns lavender and the tiles jiggle.
- During the ignored presses the button's pixels are byte-identical to the settled frame (same hash in the logs).

### 2. Short bursts on the check (star mode, entered 1.5 s earlier: aim is to get out)

| Burst | Ends in | Extra presses look pressed? | Evidence |
| --- | --- | --- | --- |
| Double 300/150, 300/400, 400/200 | normal | no | `sheets/B1_check_double.png`, `logs/B4_*`, `logs/B5_*` |
| Triple 300/150, 300/500 | normal | no | `sheets/B2_check_triple.png`, `logs/B6_*` |
| Mash of 5, 200/100 | normal | no | `sheets/B3_check_mash5.png` |

### 3. Longer or firmer mashes: the mode flips back (finding M1)

These are fresh processes, with timings measured from the lift that changed the mode. Results were the same on the star and on the check (`logs/E_star_*` mirror `logs/E_normal_*`).

| Mash (press / gap, presses) | Total length | Ends where heading? |
| --- | --- | --- |
| 5 x 200/100, 300/100, 300/150, 300/200, 350/150, 400/100 | 1.3-2.2 s | yes |
| 5 x 400/150 | 2.6 s | **no**: the 5th press (1.8 s after the change) acts |
| 5 x 400/200, 5 x 350/200 | 2.7-2.9 s | **no** (5th press at 1.85-2.0 s) |
| 6 x 250/150, 6 x 300/150 | 2.2-2.6 s | **no** (6th press at 1.75-1.95 s) |
| 7 or 8 x 200/100, 250/100; 8 x 150/100 | 2.0-2.6 s | **no** (the press at 1.6-1.85 s acts) |
| 12 presses, any cadence (`logs/F_*`) | 4-8 s | flips again every 1.5-2.2 s; the end mode is a coin toss |

- In a continuous mash, the first press beginning about 1.6-2.1 s after the change acts. It shows "Done" (or "Pick games") and flips the mode, and the guard starts over.
- The acting press is indistinguishable to Leo from the ignored ones before it. Only the result differs.

### 4. Leo's "again!" at about one tap a second (the previous round's M1)

- **Right after a change** (`logs/C_*`): taps at 1 Hz, with presses of 200-400 ms and 600-900 ms between lift and the next press.
  - **Every tap acts.** The first retry 600-900 ms after the change already works, so nothing ever feels dead. The previous round's "dead until you pause" problem is gone.
  - In both directions (star and check), each such tap toggles the mode. That is the honest result of separate deliberate taps.
- **After an ignored tap** (`logs/H_*`): Leo taps the check (exit), then the star 200-500 ms later (ignored), then "again!" 700 ms after that. The "again!" works every time: one dead tap, then success about 1.2-1.5 s after the change.
- **A faster "again!" at 0.8 s** (300 ms press, 500 ms gap; `logs/H8_*`): two dead taps, then success about 1.9-2.1 s after the change.
- **Jittered 1 Hz** (`logs/J_seed*`): 3 seeds of 10 taps, with presses of 200-400 ms and gaps of 400-900 ms. Taps whose gap after the last lift is under 600 ms are dropped silently. At most two dropped in a row, and the next tap always worked.
- **Gap threshold** (`logs/G_*`): a second press beginning 450-550 ms after the change's lift is ignored; one at 600 ms or more acts. This held for presses of 200, 300 and 400 ms and in both directions.
  - The app brief says 1 s; the observed value is 600 ms.

**Verdict on "does it ever feel broken":** at 1 Hz, no. Only a dropped tap under 600 ms after a change goes unanswered, and the very next one works.

### 5. The sister (deliberate 450 ms presses)

- **Enter, then the check 500 ms later** (her quick change of mind; `logs/S1_*`, `sheets/S1_1000.png`): ignored silently, with no pressed look and no caption.
  - Her next deliberate tap works, whether it comes 600, 800, 1000 or 1500 ms after that.
  - The same holds for exit, then the star 500 ms later (`logs/S4_*`).
- **Change of mind 600 or 700 ms after entering:** it works at once (`logs/S3_*`).
- **Other routes:**
  - The house button during star mode returns to normal All games, on the page she was on.
  - A star tap 300-500 ms after that is ignored; 600 ms or more works (`traces/HS_*.txt`).
  - Page 2 in star mode works (arrow, "Home full!" and the check; `sheets/R1_routes.png`).

### 6. A minute of normal play (`traces/N5_minute.txt`, `sheets/N2.png`, `sheets/N4.png`, `sheets/N5.png`)

This covers about 41 s of play after the 11 s setup, all on one fresh process.

1. Leo entered star mode and took the yellow arrow to page 2 to star the red car. He got "Home full! Stars: page 1" in yellow, which he cannot read.
2. He double-tapped the car star again (nothing new), then followed the hopping arrow back to page 1.
3. With his sister's help he unstarred the pumpkin (the header went to 4 gold and 1 white), went to page 2 and starred the car (5 gold).
4. One tap on the check showed "Done" and returned to the blue sky. The house took him Home: the car is first.
5. He aimed at the car and missed. The tap opened Marble Kick (the tile below it). Its house button brought him straight back to Home.
6. He tapped the car: Rally's track picker. Rally's "Home" took him back to the launcher.
7. All games, then the star: he double-tapped the soccer kid's gold star off (once), tapped the radar's star gold, and triple-tapped the check. It exited once, with nothing flipped back.
8. The house took him Home: radar first. All games again showed the edits kept.

Every step responded and nothing got stuck. The only friction was the unreadable "Home full!" text, and the hopping arrow plus the white-and-gold stars carried it.

### 7. Side probe: double tap on Home's "All games" tile (finding m2)

The All games tile on Home and the radar tile in the Library sit in the same place (bottom right).

| Gap between taps (300 ms presses) | Result |
| --- | --- |
| 100-380 ms | the second press is ignored; Leo stays in All games |
| 400-500 ms | the second press opens **Radar** |

Evidence: `traces/DT_allgames_*.txt`, filmed in `sheets/DT400film.png`, where the "Radar" caption appears under the second press.

## Findings (ranked)

### M1 (Major): a firm 5-tap mash, or any mash past about 1.6-2 s, flips star mode back

- **Screen**: `library` (All games), star/check spot.
- **Issue**: during a continuous mash, the press beginning about 1.6-2.1 s after the change acts again. It shows "Done" / "Pick games" and flips the mode, so the burst ends opposite to where it was heading (and a longer mash ends at random).
- **Concern**: feedback / recovery (the exact behaviour the guard exists to prevent).
- **Repro**: `launcher-home-playtest-8/traces/E_normal_mash5_400_150.txt`. After setup, in All games:
  1. Make 5 taps on the star (126,43): 400 ms presses, 150 ms apart.
  2. Press 1 enters star mode. Presses 2-4 are ignored. Press 5 (1.8 s after the change) shows "Done" and returns to the blue sky.
  3. The check mirrors this: `traces/E_star_mash5_400_150.txt` ends back in star mode.
  4. Replayed in a second fresh process (`frames/E_replay`) with the same result.
- **Screenshot**:
  - `launcher-home-playtest-8/sheets/E_normal_mash5_400_150.png` (`frames/E_normal_mash5_400_150/p5_p.png` shows the "Done" flash)
  - `sheets/E_star_mash5_400_150.png`
  - `sheets/A7_star_mash8.png`
  - `sheets/B7_check_mash8.png` (a mash of 8 x 250/100 exits and then re-enters)
- **Player impact**:
  - Leo's own firm presses (350-400 ms) put a 5-tap mash at 2.6-2.9 s. That alone triggers it.
  - An excited 6-year-old who keeps mashing for a couple of seconds after the sky changes sees lavender, then blue again.
  - Mashing the check, he ends up still in star mode. Mashing the star, he ends up out of it.
  - Nothing is lost: the check's stars are saved, and one more tap recovers. But it reads as "the button undid itself".
- **Likely cause** (black-box; not verified on the old build): the new 3 s cap appears to apply as about 1.6-2.1 s inside a mash. The app brief documents "within 3 s".
- **Suggestions for the owner** (not prescriptive):
  - Keep ignoring mash-cadence presses (a gap under 600 ms after the last lift) for the whole of an unbroken mash, or for a true 3 s.
  - Let only a press after a pause of 600 ms or more act, which is what already makes the 1 Hz retry work.

### n1 (Nit, carried over from round 7 and narrower now): a change of mind under 600 ms is dropped silently

- **Screen**: `library`, star/check spot.
- **Issue**: a deliberate tap beginning less than 600 ms after the mode change's lift does nothing and shows nothing. The window was under 1 s in round 7.
- **Concern**: feedback.
- **Repro**: `traces/S1_1000.txt`. Enter star mode with a 450 ms press; tap the check 500 ms later (ignored); tap again 1 s later (exits).
- **Screenshot**: `sheets/S1_1000.png`.
- **Player impact**: small. The sister's next deliberate tap always works. This is the accepted cost of the mash guard.

### m2 (Minor, outside the narrow scope; found in normal play): a slow double tap on Home's "All games" opens Radar

- **Screen**: `launcher` (Home), into `library`.
- **Issue**: the second press of a double tap with a gap of 400 ms or more lands on the Library's bottom-right tile and opens that game. The app brief says a press within 450 ms of a screen change is ignored; the observed cut-off is about 390 ms.
- **Concern**: recovery / consistency.
- **Repro**: `traces/DT_allgames_400.txt`. On Home, tap (124,105) for 300 ms, wait 400 ms, tap (124,105) for 300 ms. Radar opens.
- **Screenshot**: `sheets/DT400film.png` (the `b_p` frame shows the "Radar" caption; `after_*` frames show Radar).
- **Player impact**: Leo lands in a game he didn't pick (Radar today, whichever game is sixth in the Library tomorrow). Its home button brings him back, so nothing is lost.
- **Suggestion**: check whether `FreshGate`'s 450 ms is measured from the frame the new screen first shows. The sim may be measuring it from earlier.

## Qualitative assessment (narrow)

- **Recognize goal and actions:**
  - The star button's effect is unmistakable without reading: the lavender sky, the jiggling tiles, a white or gold star on every game, and five header stars counting Home's places.
  - The green check reads as "finished".
  - "Home full! Stars: page 1" is unreadable for Leo. The hopping arrow and gold stars carried him to the right page, and his sister read it.
- **Feedback and consequences:**
  - Every acting press shows a caption and a pressed look; ignored presses show nothing. Star taps flip gold and white immediately, and Home reorders with the newest star first, which Leo noticed straight away (the car, then the radar).
  - The weak spot is M1: within a long mash, the one press that acts looks like all the others, and the result undoes the first one.
- **Where it gets frustrating, and where Leo would stop:**
  - At 1 Hz there is no frustration any more (round 7's M1 is fixed).
  - Firm mashing past about 2 s is the remaining frustration (M1).
  - Leo's natural stopping point was after starring the car and seeing it first on Home. He then wanted to play Rally, not to keep editing. Star mode is a short, satisfying side trip.
- **Recovery:**
  - Every wrong state is one deliberate tap away from right.
  - The house leaves star mode, the games' own home buttons return to the launcher, and edits are kept after the check. Nothing is lost in any probe.

## Gesture / interruption matrix

| Gesture | Star (enter) | Check (exit) | Game star in star mode |
| --- | --- | --- | --- |
| Single firm tap | ok | ok | ok |
| Double, 150-400 ms gap | ok, once | ok, once | ok, once (200 ms gap) |
| Double, 600 ms+ gap | acts twice (two taps) | acts twice | not tested |
| Triple, 150-500 ms gap | ok, once | ok, once | not tested |
| Mash of 5, short presses (200-300 ms) | ok, once | ok, once | not tested |
| Mash of 5, firm (350-400 ms), or more than 1.6-2 s of mashing | **flips back (M1)** | **flips back (M1)** | not tested |
| 1 Hz "again!" after a change | every tap acts | every tap acts | n/a |
| 0.8 Hz "again!" after an ignored tap | 2 dead, then ok | 2 dead, then ok | n/a |
| Change of mind under 600 ms | dropped silently (n1) | dropped silently (n1) | n/a |
| House during star mode, then star | dead under 600 ms, then ok | n/a | n/a |
| Page 2 in star mode (arrow, Home full) | ok | ok | ok (refused, then accepted after unstar) |
| Off-centre taps, drag-off, long press | **gap** (covered in round 7, not retested) | **gap** | **gap** |
| Process restart / reboot during star mode | **gap** | **gap** | **gap** |
| Rest screen / daily cap during star mode | **gap** | **gap** | **gap** |

## Gaps

- No restart, rest-screen or daily-cap interruption during star mode.
- No off-centre, drag-off or long-press retest on this build (they passed in round 7; the rule change was timing-only).
- The previous build was not re-run, so "M1 is new in `7ca8c01`" is inferred, not verified.
- No formal scenario added: the brief kept me out of `tests/playtests/`. `traces/E_normal_mash5_400_150.txt` is a ready repro for one.
- Timings are nominal (40 ms frames).
- This was not a real child and not a physical device. A real capacitive panel may split or merge firm presses differently.

## Evidence index (`launcher-home-playtest-8/`)

- `traces/*.txt`: the exact input of every probe.
  - Grammar: `tap X Y HOLD [NAME]`, `tick MS`, `shot NAME`, `watch on/off`, `film MS STEP NAME`, in logical coordinates.
  - `setup.txt` is the shared prefix.
  - `driver-source.txt`, `gen-source.txt`, `sweep-source.txt`, `seq-source.txt`, `sheet-source.txt`: the tools, saved as text.
- `logs/*.txt`: per-probe input timeline with each on-screen state change (sky / caption / icon / button-pixel hash).
- `frames/<probe>/*.png`: native 480x480 frames for the key probes. `frames/setup/` holds the profile creation screens.
- `sheets/*.png`: contact sheets for every probe named above.

## Automated suite

`make -C apps/porthole playtest` at `7ca8c01`: 93 of 93 scenarios PASS, 0 UI FAIL, 0 UI WARN (`b2_home_stars`: 67 checks, `a2_launcher_pages` included). The report was read in full; it has no failures and no UI findings.

## Verdict

**Needs fixes.**

- Fixed since round 7: the 1 Hz "again!" problem (every retry at 600 ms or more now works). Short double taps, triple taps and short-press 5-tap mashes end where they were heading, nothing looks pressed without acting, and the sister's deliberate flow works.
- Still open: the cap that fixed 1 Hz now lets a firm 5-tap mash, or any mash past about 1.6-2 s, flip the mode back (M1, Major for this persona, recoverable with one tap).
- Minor m2 (a 400 ms double tap on "All games" opens Radar) and Nit n1 can be deferred with the impact stated above.

Nothing was fixed or committed by me.
