# Launcher All games: star/check button, kid playtest 7 (narrow retest)

This is emulator roleplay by an independent playtester agent. It is not a study with a real child, and it was not run on a physical device.

## Build

- Revision: `950e105` ("one rule for the star/check spot, so a mash flips star mode once"). The tree was clean apart from this report.
- Binary: `apps/porthole/build/host/snap`. `make -C apps/porthole snap` said it was up to date. I copied it to a private scratch folder (with its own `build/host/` for saves) and ran it from there.
  - sha256 `4e01025ed451bf5465b42f5f3cf73c6fb1e4d051584448672cc9fd4baa1589ba`
- Mode: `snap --serve --fresh`, default clock.
  - A small driver sent `down X Y` / `move X Y` / `up` / `tick MS` / `frame` and saved native 480x480 PNGs. Its source is saved as `launcher-home-playtest-7/traces/driver-source.txt`.
  - Each probe is a fresh process: the same setup prefix plus one burst. The sim steps whole 33 ms frames, so press and gap lengths are nominal (about ±33 ms).
- Automated suite: `make -C apps/porthole playtest` at `950e105`. 93 of 93 scenarios passed, with 0 UI FAIL and 0 UI WARN. I read `build/playtest/report.md` in full; its findings table is empty.

## Persona

- **Leo, 6.** Can't read. Taps firmly (200-400 ms presses). Double-taps, triple-taps and mashes when impatient. Loves the gold stars.
- **His sister, 9.** Sometimes does the same thing slowly and on purpose.

This is a simulated perspective only.

## Dispatch brief (verbatim, no revisions)

> You are an independent child-perspective playtester for Porthole, a round 480x480 touchscreen with no buttons, emulated on the host. Follow apps/porthole/docs/playtesting.md and the `playtest` skill. Play source-free: don't read source (apps/porthole/games/, os/, shell/, host/), tests/playtests scripts or docs/superpowers/. Disclose anything that loaded automatically. Decide from the screen. Run the sim from your own copy of the snap binary in a scratch folder.
>
> ## Persona
> Leo, 6, can't read, taps firmly (200-400 ms presses), double-, triple- and mash-taps when impatient, and loves the gold stars. His sister (9) sometimes does it slowly and on purpose. Simulated perspective only.
>
> ## Setup
> - `make -C apps/porthole snap`. Record `git rev-parse --short HEAD` and `sha256sum` of the snap you run.
> - `snap --serve --fresh`, default clock. Create Leo's profile (age 6, no code) through the screens; ordinary touches only; no dbg, shortcuts or clock skips.
>
> ## Scope (narrow retest of the top-right star/check button in All games)
> - Leo: firm double and triple taps and 5-tap mashes on the star button, then on the green check. Does each burst end in the mode he started toward (into star mode, or out of it), or does it flip back? Does anything look pressed without doing anything?
> - The sister: one deliberate tap to enter, a few stars, one deliberate tap on the check, then the star button again after a second or two. Does every deliberate tap work? Does she ever think the button is broken?
> - Double taps on one game's star (gaps of 200-500 ms), and quick taps on two different games.
>
> ## Output
> Write apps/porthole/docs/launcher-home-playtest-7.md, with screenshots and the input trace in apps/porthole/docs/launcher-home-playtest-7/ (no .py files; save scripts as .txt). Include the revision and hash, the persona, this brief, findings ranked with repro steps, and gaps. Don't fix code or commit.

## Disclosures (what I knew beyond the screen)

- **Auto-loaded:** the repo `CLAUDE.md`, and partway through the session `apps/porthole/AGENTS.md` (through its `CLAUDE.md` symlink).
  - The second file states the intended rules. A repeat press on one tile within 600 ms of the last lift counts once. A press on the star/check spot is ignored if it begins within 1 s of star mode starting or ending, or within 1 s of the lift of a press it already ignored.
  - I knew these rules before most probes, and I used them to pick probe cadences around the boundaries. I did not use them to decide what Leo would do. Finding M1 comes from the last clause of the rule, and I confirmed it on screen.
- **Read on purpose:**
  - `docs/playtesting.md`, the `playtest` and `tour` skills.
  - The protocol lines of `tools/webemu.py`, to learn how to drive `--serve`.
  - The setup section and disclosures of `launcher-home-playtest-6.md`, for the protocol and the save-folder pitfall. That section also mentions the 400 ms / 1 s numbers.
  - I read no game, shell, host or test-scenario source.
- **Scratch folder:** the session scratchpad already held files from earlier agents, including copies of shell source. I did not open them.
  - Before noticing this, I overwrote an existing `drive.py` and `setup.txt` there with my own.
  - All my evidence was then re-run from a private subfolder (`lhp7/`) with its own snap copy (same hash).
- **No hidden state:** no `dbg`, `debug`, `screen`, `ui` or `app` commands were used. The mode was judged from the sky colour (lavender = star mode, light blue = normal) and from what the frames show.

## Setup path (ordinary touches, `traces/tr_*.txt` lines 1-21)

1. Fresh device: name keyboard. Typed L, E, then N-Z, o, OK.
2. Picked the frog face, age 6, then "skip" the code.
3. Home, then the "All games" tile (bottom right, the six tiny pictures).

In All games, the top-right square button shows a gold star. While a finger is on it, a yellow "Pick games" caption covers the title. Its centre is at logical (126,43).

## What happened

### Leo on the star button (start: normal mode; aim: get into star mode)

Presses were 300 ms with 150 ms gaps (double, triple) and 200 ms with 100 ms gaps (mash of 5).

| Burst | Ends in | Extra presses look pressed? | Evidence |
| --- | --- | --- | --- |
| Double | star mode | no | `sheet_A1_double_star.png` |
| Triple | star mode | no | `sheet_A2_triple_star.png` |
| Mash of 5 | star mode | no | `sheet_A3_mash5_star.png` |

- The first press shows the yellow "Pick games" caption and a pressed star. The sky then turns lavender and the tiles jiggle.
- The later presses change nothing in the header. A pixel diff of the header/button area of every mid-press frame against the settled frame is identical; only the tile jiggle differs.

### Leo on the green check (start: star mode, entered 1.5 s earlier; aim: get out)

| Burst | Ends in | Extra presses look pressed? | Evidence |
| --- | --- | --- | --- |
| Double | normal | no | `sheet_B1_double_check.png` |
| Triple | normal | no | `sheet_B2_triple_check.png` |
| Mash of 5 | normal | no | `sheet_B3_mash5_check.png` |

- The first press shows the yellow "Done" caption over the stars and a pressed check.
- The later presses land on the star button, which is already back, and show nothing.
- None of these bursts flips star mode back on.

### The sister (deliberate 400 ms presses, `sheet_S1_sister.png`)

She did these steps in order:

1. One tap: star mode.
2. 1.5 s later she tapped the pumpkin: its star went white.
3. She tapped the dog: white.
4. She tapped the radar: gold. The header showed 4 gold stars and 1 white.
5. She tapped the check: "Done" caption, back to the blue sky.
6. 1.5 s later she tapped the star again: "Pick games", star mode. Her picks were kept.

Every deliberate tap worked.

Gap sweep (fresh process each, 400 ms presses, gap measured from the previous lift):

| Gap after lift | Enter, then check: exits? | Check, then star: re-enters? |
| --- | --- | --- |
| 500 / 700 / 800 / 900 ms | no (silently ignored) | no (silently ignored) |
| 1000 / 1100 / 1300 / 2000 ms | yes | yes |

At "a second or two" she is always fine. A sister who changes her mind at once ("oops, one more star", 0.5-0.9 s later) loses that one tap with no sign of why. If she then waits, the next tap works.

### Double taps on one game, two games quickly (`sheet_DT.png`)

- Double tap on the pumpkin's gold star, 300 ms presses, gaps of 150 / 200 / 300 / 400 / 500 ms: each toggled once (gold to white). Triple taps (gaps 250 and 400 ms) also toggled once.
- With gaps of 600 / 700 ms the second press counts as a new tap and the star goes back to gold. That is outside the brief's 200-500 ms range. A slow "on purpose" double tap at 600 ms+ is two taps, which seems fair.
- Pumpkin then radar, 150 ms apart: both counted. The pumpkin went white and the radar gold.
- Radar first, then pumpkin, 150 ms apart: the radar star was refused, because Home was full. The pumpkin unstar counted, leaving 4 gold.
- A double tap on each game in turn, at 200 ms gaps: both toggled once.
- A tile tap 200 ms after a 5-mash on the star button counted. The button guard does not leak onto the tiles.

### Extra imprecise-finger probes (`sheet_P.png`)

- **Off-target taps:**
  - About 2 px left and right of the button, and 15 px above it (between the button and the house): nothing.
  - Inside the corners: enters.
  - At (126,58), just below the button: lands on the top-right game tile and opens that game. That is a correct tile hit, but it shows how close the tile row is.
- **Drag off:** a finger slid off the button to the title before lifting cancels it, and the button does not look pressed while the finger is off it. A deliberate tap 100 ms later works, so a cancelled press does not lock the button.
- **Long press:** a 1.5 s hold on the star enters star mode, and a 1.5 s hold on the check leaves it.
- **The house in star mode:** it goes back to normal All games. A star tap 0.7 s later is ignored; one 1.8 s later works.

## Findings (ranked)

### M1 (Minor, close to Major for Leo): steady one-a-second tapping keeps the star button dead indefinitely, with no sign why

- **Screen**: `library` (All games), star/check spot.
- **Issue**: each ignored press restarts the 1 s guard from its own lift. A child tapping about once a second never gets a working tap until he pauses for a full second, and nothing on screen shows the button is resting.
- **Concern**: feedback / recovery (the "is it broken?" feeling).
- **Repro**: `launcher-home-playtest-7/traces/tr_K1_steady_1s_taps.txt`. After setup:
  1. Tap the star (126,43) for 300 ms. Wait 1.5 s: star mode.
  2. Tap the check for 300 ms: it exits, correct.
  3. Make 7 more 300 ms taps on the star, each 700 ms after the last lift. All 7 are ignored, the sky stays blue, and the button never looks pressed.
  4. Pause 1.1 s, then tap again: it enters at once.

  I replayed this in a second fresh process with the same result.
- **Screenshot**:
  - `launcher-home-playtest-7/sheet_K1_steady_1s_taps.png`
  - `launcher-home-playtest-7/frames/K1_steady_1s_taps/k2_p.png` … `k8_p.png` (pressed frames, identical to the idle frame)
  - `kz_after_pause1100.png`
- **Player impact**:
  - The fast bursts in scope work well.
  - At one tap a second, though, each tap is visibly separate and deliberate, and that is exactly how a 6-year-old reacts to "nothing happened": try again, and again, a little faster. Leo would see 3-7 dead taps in a row.
  - Most likely he walks away, or decides the star button is broken. He would not discover that he has to stop for a second.
  - The sister, tapping at "a second or two", never hits this.
- **Suggestions for the owner** (not prescriptive):
  - Restart the guard only for presses at mash cadence, for example when the ignored press began within about 400-500 ms of the last lift, so the third or fourth separate tap works.
  - Or give the resting button a visible "not yet" state, such as briefly dimmed, so a non-reader learns to wait.

### N1 (Nit): a quick change of mind in the first second is silently dropped

- **Screen**: `library`, star/check spot.
- **Issue**: from the gap sweep, an enter followed by a check 0.5-0.9 s later is ignored, and so is a check followed by a star 0.5-0.9 s later. A single deliberate tap is lost with no feedback; the next tap after a pause works.
- **Concern**: feedback.
- **Repro**:
  - `traces/tr_G_exit_700.txt`: enter, then a check 700 ms after the lift. It stays in star mode.
  - `traces/tr_G_reenter_700.txt`: check, then star 700 ms later. It stays normal.
- **Screenshot**: `frames/G_exit_700/r_end.png`, `frames/G_reenter_700/r_end.png`.
- **Player impact**: small. This is the intended cost of making a mash flip the mode only once, and one retry fixes it. It shares M1's root (no visible resting state).

### What is now fixed relative to the scope questions

- Every double tap, triple tap and 5-mash on the star ends in star mode, and every one on the check ends out of it. None flips back.
- No ignored press looks pressed or shows its caption.
- The sister's deliberate path works, re-entry after 1 s or more included.
- Double taps at 150-500 ms gaps on one game act once, and quick taps on two different games both count.

## Qualitative assessment (narrow)

- **Recognize goal and actions:**
  - Leo cannot read "Pick games" or "Done", but the change is unmistakable: the lavender sky, the jiggling tiles, a star on every game, and five little header stars.
  - The green check reads as "finished" without words.
  - A tap on a game's star visibly flips it gold or white, and the header count changes with it. This is the part he loves.
- **Feedback and consequences:**
  - Good for every press that acts.
  - For a press that is ignored there is deliberately no feedback. That is right for the extra presses in a mash, but it reads as "broken" when the taps are spaced out (M1).
- **Where it gets frustrating, and where Leo would stop:**
  - The one realistic frustration is M1: "Again! Again!" taps at about one a second after a check.
  - Otherwise star mode is short and satisfying. Leo would naturally stop after starring two or three games and tapping the check, or after opening a game. The check plus the return to the blue sky is a clear stopping point.
- **Recovery:**
  - Wrong taps on stars are undone by tapping again.
  - The house leaves star mode, and a drag-off cancels.
  - Nothing is lost. M1 recovers by itself once the child pauses for 1 s, which needs patience rather than coaching.

## Gesture / interruption matrix

| Gesture | Star button (enter) | Check (exit) | Game star in star mode |
| --- | --- | --- | --- |
| Single firm tap | ok | ok | ok |
| Double 150 ms gap | ok, once | ok, once | ok, once (to 500 ms) |
| Triple | ok, once | ok, once | ok, once |
| Mash 5 (100 ms gaps) | ok, once | ok, once | not tested |
| One tap a second, repeated | **dead until a 1 s pause (M1)** | first ok, rest dead | not tested |
| Long press 1.5 s | ok | ok | not tested |
| Drag off before lift | cancels, next tap ok | not tested | not tested |
| Off-centre taps | edges ok; outside ignored | not tested | not tested |
| House during star mode | n/a | leaves star mode; star dead for 1 s | n/a |
| Process restart / reboot | **gap** | **gap** | **gap** |
| Rest screen / daily cap mid star mode | **gap** | **gap** | **gap** |
| Library page 2 (arrows) in star mode | **gap** | **gap** | **gap** |

## Gaps

- Star mode on Library page 2 was not tested; with six games there was only one page here.
- Not tested: interruption by the rest screen or a restart during star mode; the "Home full!" refusal feedback beyond its end state; and whether stars persist across a reboot.
- I did not add a formal playtest scenario: the brief kept me out of `tests/playtests/`, so I cannot check what `b2_home_stars` already covers. The K1 trace is a ready repro for one.
- Timings are nominal: the sim steps in whole 33 ms frames.
- This was not a real child and not a physical device. Real capacitive touch may split a firm press, or add jitter, in ways the emulator does not.

## Evidence index (`launcher-home-playtest-7/`)

- `traces/tr_*.txt`: the exact input of every probe, in the driver grammar.
  - `tap X Y HOLD [NAME]` = down, hold, up. A NAME saves a mid-press frame `NAME_p`.
  - `shot NAME` saves a frame.
  - Logical coordinates, 0-159.
- `traces/driver-source.txt`: the driver (Python, saved as text).
- `frames/<probe>/*.png`: native 480x480 frames. `frames/setup/` holds the profile creation screens.
- Sheets:
  - `sheet_A*`: Leo on the star.
  - `sheet_B*`: Leo on the check.
  - `sheet_S1_sister.png`: the sister's flow.
  - `sheet_DT.png`: double taps and two games.
  - `sheet_K1_steady_1s_taps.png`: M1.
  - `sheet_P.png`: off-target, drag, long press.

## Verdict

**Acceptable for the scoped bursts**: every scoped double, triple and mash now ends in the intended mode, nothing looks pressed without acting, and the sister's deliberate flow works. One Minor remains (M1): spaced "again, again" taps at about one a second keep the button dead until a 1 s pause, with no visible cue. I recommend fixing it, or explicitly accepting it, before calling the star button done for a 6-year-old. Nothing was fixed or committed by me.
