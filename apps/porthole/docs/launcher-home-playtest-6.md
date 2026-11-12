# Launcher Home / Library star mode: kid playtest 6 (narrow retest)

Emulator roleplay by an independent playtester agent. This is not a real-child study and not a physical-device test.

## Build

- Revision: `811293c` (clean tree)
- Binary: `apps/porthole/build/host/snap` copied to a private scratch folder and run from there,
  sha256 `2dc4c237c2dd8f75ac3f718c099f860f009e026bde44d5804389c8a8fe5927d7`
- Mode: `snap --serve --fresh`, default clock. A small private socket driver sent the serve protocol's
  `down X Y` / `tick MS` / `up` / `tilt X Y Z` / `frame` lines. Frames were saved as native 480x480 PNGs.
  The sim steps whole frames, so press and gap lengths below are nominal (±40 ms).

## Persona

Leo, 6. Can't read. Taps firmly (presses of 200-400 ms). Double-taps and triple-taps when impatient. Loves the gold
stars. This is a simulated perspective only.

## Dispatch brief (verbatim, no revisions)

> You are an independent child-perspective playtester for Porthole, a round 480x480 touchscreen with no buttons,
> emulated on the host. Follow apps/porthole/docs/playtesting.md and the `playtest` skill. Play source-free: don't
> read source (apps/porthole/games/, os/, shell/, host/), tests/playtests scripts or docs/superpowers/. Disclose
> anything that loaded automatically. Decide from the screen. Run the sim from your own copy of the snap binary in a
> scratch folder, so other processes can't disturb its saves.
>
> ## Persona
> Leo, 6, can't read, taps firmly (presses of 200-400 ms), double-taps and triple-taps when impatient, and loves the
> gold stars. Simulated perspective only.
>
> ## Setup
> - `make -C apps/porthole snap`. Record `git rev-parse --short HEAD` and `sha256sum` of the snap you run.
> - `snap --serve --fresh`, default clock. Create Leo's profile (age 6, no code) through the screens; ordinary
>   touches only; no dbg, shortcuts or clock skips.
>
> ## Scope (narrow retest)
> In All games, in the mode where games get gold stars:
> - Firm double and triple taps on one game's star. Does it end up the way Leo meant (on after a double tap's first
>   tap), or flip back?
> - Quick taps on two different games. Do both count?
> - Triple taps on the green check button, and on the house. Does Leo land back in the starring mode by accident?
> - After leaving the mode, how soon can he deliberately get back in? Does it feel broken?
> Then a minute of normal play from his first screen.
>
> ## Output
> Write apps/porthole/docs/launcher-home-playtest-6.md, with screenshots and the input trace in
> apps/porthole/docs/launcher-home-playtest-6/ (no .py files; save scripts as .txt). Include the revision and hash,
> the persona, this brief, findings ranked with repro steps, and gaps. Don't fix code or commit.

## Disclosures (what I knew beyond the screen)

- **Auto-loaded:** `apps/porthole/AGENTS.md` (via its `CLAUDE.md` symlink) loaded automatically. It describes the
  intended star-mode rules, including the 400 ms merge of repeat presses on one tile and the 1 s window in which the
  star button ignores presses after the check. So I knew those numbers before play. I used them to choose probe
  cadences around the boundaries. I did not use them to decide what Leo would do.
- **Read on purpose:** `docs/playtesting.md`, the `playtest` and `tour` skills, and the stdin protocol lines of
  `tools/webemu.py` (to drive `--serve`). I read no game, shell, host or test-scenario source.
- **Hidden state:** Run A has one stray `debug` line (trace line 69). Its output was not shown to me, so I learned
  nothing from it. The repro scripts in `repro/` use `profile` and `debug` to confirm `star_mode` in a fresh process.
  They are supplementary QA, not part of the black-box pass.
- **Run A save failure (test setup, not the game):** My first scratch folder had no `build/host/`, so the sim
  could not write saves. Every star edit and check worked on screen. But after going through "Who's playing?" and
  back, Leo's Home had returned to the default games. Run B used a folder with `build/host/`, and its stars
  survived the same trip. I treat Run A's lost stars as caused by my setup. The input-timing results matched in
  both runs. Run A is kept as evidence of what I tried first.

## Paths played

1. Fresh device, then name keyboard (L, e, N-Z page, o, OK), then face (frog), age 6, code skip, then Home.
   Run B repeated this setup.
2. Home, then the "All games" tile, then the Library, then the star button (the caption "Pick games" shows while
   pressed). Star mode: lavender sky, tiles jiggle, five gold stars in the header, green check.
3. Tapped Radar (white star) while Home was full. "Home full! Tap a star" showed and the gold stars hopped. Unstarred
   Pumpkin, then double-tapped and triple-tapped Radar. Tapped two games quickly, one after the other. Probed slow
   double taps.
4. Check: single tap, fast triple tap, slow triple tap, fast quadruple tap. Star button: re-entry at 600, 700, 900,
   1000 and 1300 ms after the check. Star button: double and triple tap.
5. House: triple tap from star mode and from the Library. Picker, then Leo, then Home kept the stars he chose
   (Run B).
6. About a minute of normal play: opened Pumpkin from Home, got caught after two candies ("YOU LOST", pie), went
   back to Home via the top sign. Opened Radar, tapped it (the range changed), watched planes, back to Home, then
   All games.
7. Extra input probes: a 1 s long press on the star button and on a tile, a drag off the check, and an off-center
   tap on the check.

## Results against the brief

| Question | Result | Evidence |
| --- | --- | --- |
| Firm double tap on one star (300 ms presses, 150 ms gap) | It acts once. A white star turns gold and stays gold. | `runB/B13_radar_dbl.png`, `runA/11_dbl_*.png` |
| Firm triple tap on one star (250-350 ms presses, 120-150 ms gaps) | It acts once each time (gold to white, and white to gold). | `runB/B14_*`, `runB/B15_*`, `runA/12_*`, `runA/13_*` |
| Slow double tap (350 ms gap) | It acts once. | `runA/15_gap350.png` |
| Slow double tap (450 ms gap) | **It counts twice and flips back** (white, gold, white). | `runA/15_gap450.png` (F4) |
| Quick taps on two different games (150 ms and 60 ms gaps) | Both count. At 60 ms the second was refused only because Home was full ("Home full!"). | `runB/B16_two_quick.png`, `runA/14_*`, `runA/15_two_games_gap60.png` |
| Fast triple tap on the check (300/150) | Leaves star mode and stays in the Library. | `runA/16_check_*.png`, `runB/B17_check_triple.png` |
| Slow triple tap on the check (400/300) | **Leaves, then the third press re-enters star mode.** | `sheet-F2-slow-triple-check.png` (F2) |
| Fast quadruple tap on the check (300/150) | **Re-enters star mode.** | `runB/B21_quad_check.png` (F2) |
| Triple tap on the house in star mode | Goes to the Library (unsaved edit dropped), then Home. Not back in star mode. | `runA/20_house_*.png` |
| Triple tap on the house in the Library | Goes to Home, then "Who's playing?". Not star mode. | `runA/21_*`, `runB/B24_house_triple.png` (F5) |
| Re-entry after leaving | A star press that starts 600, 700 or 900 ms after the check's lift is ignored. One that starts at 1000 or 1300 ms enters. | `runA/17_*`, `runA/19_*`, `runA/18_ignored_*` (F3) |
| Triple tap on the star button (300/150) | **Enters star mode, then the third press lands on the check and leaves.** | `sheet-F1-triple-tap-star-button.png` (F1) |

## Findings (ranked)

### F1 (Major): an impatient triple tap on the star button enters star mode, then leaves it again

- **Screen**: library (star button) / star mode (check)
- **Issue**: The green check sits exactly where the star button was. The 450 ms fresh-screen guard drops the second
  press, but the third press (about 600 ms after entry) lands on the check. The screen flashes lavender for about
  0.7 s and goes back to the plain Library. Every triple tap at Leo's cadence did this.
- **Concern**: feedback / recovery / consistency. Leo's habit on the control he likes best looks broken. He sees
  "nothing happens" (just a flash) and will probably tap harder and faster, which repeats it. A single or double tap
  works, but nothing on screen tells a non-reader to tap fewer times. This is the mirror of the 1 s guard the star
  button already has after the check. The check has no matching guard after entry, only the 450 ms one.
- **Repro**: `docs/launcher-home-playtest-6/repro/r1_triple_tap_star_button.txt` (fresh process: `star_mode=1` after
  the first tap, `star_mode=0` at the end). By hand: Home, All games, then three presses of 300 ms on the star
  button (126,42) with 150 ms gaps.
- **Screenshot**: `apps/porthole/docs/launcher-home-playtest-6/sheet-F1-triple-tap-star-button.png`
  (`runB/B52_t1` to `B52_settled`)
- **Suggestion**: Guard both directions with the same rule the tiles use. After either screen change, ignore
  presses on that shared spot while they keep arriving within about 400 ms of the last lift (or 1 s), and stop
  ignoring them once the finger has been off for that long. The aim is that "a burst acts once", whichever
  direction the burst goes.

### F2 (Minor): a slow triple tap, or a fast quadruple tap, on the check puts Leo back in star mode

- **Screen**: star mode, then library
- **Issue**: With 400 ms presses and 300 ms gaps, the third press starts about 1000 ms after the first lift, just
  past the star button's 1 s guard, so star mode re-opens. A fast four-tap burst (300/150) reaches the same point.
  His stars were already saved, so nothing is lost. He simply ends up back in the lavender mode he tried to leave.
  If he keeps that rhythm, every burst repeats it.
- **Concern**: recovery / consistency
- **Repro**: `docs/launcher-home-playtest-6/repro/r2_slow_triple_tap_check.txt` (`star_mode=0` after the first
  press, `star_mode=1` at the end). By hand: in star mode, three 400 ms presses on (126,42) with 300 ms gaps.
- **Screenshot**: `apps/porthole/docs/launcher-home-playtest-6/sheet-F2-slow-triple-check.png`,
  `runB/B21_quad_check.png`
- **Suggestion**: The burst rule in F1 covers this too: the guard extends while presses keep coming.

### F3 (Minor): an ignored star-button press still says "Pick games" and looks pressed

- **Screen**: library, within 1 s after the check
- **Issue**: A deliberate press 600-900 ms after leaving shows the yellow "Pick games" caption and the pressed
  button, then nothing happens. A second try works. For Leo (who can't read the caption) the button visibly
  reacted and then failed, which is the "feels broken" case the brief asks about. It only happens when he wants
  back in straight away, for example after regretting a check.
- **Concern**: feedback
- **Repro**: in star mode, tap the check (300 ms). Start a 300 ms press on the star button 500 ms after the lift.
- **Screenshot**: `apps/porthole/docs/launcher-home-playtest-6/runA/18_ignored_pressing.png`, then
  `runA/18_ignored_after.png`
- **Suggestion**: While the guard holds, don't draw the pressed state or the caption, so an ignored press looks
  ignored. Or use the burst rule above, which ends the guard as soon as the finger has rested.

### F4 (Minor): a slow, firm double tap on a tile flips the star back

- **Screen**: star mode tile
- **Issue**: Presses of 300 ms with a 450 ms gap count as two taps, so the star goes gold and then back to white.
  At 350 ms it acts once. A 6-year-old's "firm" double tap can easily have 400-500 ms between lift and the next
  press.
- **Concern**: recovery (Leo sees his star go back to white). Recovery is easy: one more tap.
- **Repro**: in star mode, two 300 ms presses on a white-star tile with a 450 ms gap.
- **Screenshot**: `apps/porthole/docs/launcher-home-playtest-6/runA/15_gap450.png`
- **Note**: This is the documented 400 ms rule working as written. Widening it to about 500 ms trades against
  quick deliberate on/off. I'm recording it so the threshold is chosen knowingly.

### F5 (Nit): a triple tap on the house from the Library lands on "Who's playing?"

- **Screen**: library, then launcher, then picker
- **Issue**: The third press is past the 450 ms guard on Home, where the house means "switch kid". Leo can get
  back with one tap on his frog. He doesn't land in star mode.
- **Concern**: consistency / recovery. This is a shell-wide fresh-gate effect, not specific to star mode.
- **Repro**: Library, then three 300 ms presses on the house (80,12) with 150 ms gaps.
- **Screenshot**: `apps/porthole/docs/launcher-home-playtest-6/runB/B24_house_triple.png`

### Out of scope (for the Pumpkin owner, not this change)

- The Pumpkin loss screen reads "YOU LOST" (`runB/B42.png`). Leo can't read it, and the pie picture and TRY AGAIN
  read as gentle. It's still worth checking against the "failure is gentle" rule.

## Behavior that worked (positive cases)

- Entering star mode is unmistakable without reading: the sky turns lavender, the tiles jiggle and the green check
  appears (`runB/B10_starmode.png`).
- Double and triple taps on a tile act exactly once at normal cadence. Two different games tapped 60-150 ms apart
  both count.
- "Home full!" is unreadable to Leo, but the gold stars hopping point him at what to do. Tapping a gold star (making
  room) is what he did next.
- A long press (1 s) on the star button enters star mode. A 1 s press on a tile toggles it. Dragging off the check
  cancels. A tap 6 px off-center on the check still counts (`runB/B60`-`B63`).
- The house in star mode drops the visit's edits. The check keeps them, and they survive a trip through the picker
  (Run B, `runB/B25_home_after_picker.png`). The newest star shows first on Home.

## Qualitative assessment

- **Goal, actions and feedback:** Yes, mostly. The big star button looks like the thing to press, and the lavender
  jiggle plus white and gold stars on each game make "tap to make it gold" obvious to a non-reader. The header's
  five little stars show how many places are left. The weak spots are the shared star/check spot. A triple tap
  makes the mode vanish (F1), or come back after he pressed the check (F2), with no visible reason.
- **Decisions and feedback:** The choice is real and personal (which games live on Home), and the results show
  instantly on Home, newest first. Pictures carry everything. The only text that matters ("Home full! Tap a
  star") is backed by the hopping stars.
- **Repetition, boredom and stopping point:** Leo would star and unstar for about a minute, because gold stars are
  fun. Once Home showed Pumpkin and Radar he would stop and go and play. That is a good natural stopping point.
  He would only get annoyed if the star button "didn't work" (F1); at that point he would probably give up on star
  mode rather than keep trying.
- **Recovery from mis-taps and interruptions:** No probe lost saved stars on a correctly saving device. A wrong flip
  is one tap to undo. Leaving with the house drops unsaved edits safely. Ending up on the picker is one tap on his
  frog.
- **Normal play (about a minute):** Pumpkin opened from Home. Tilt moved the pumpkin, candies counted, and Grandma
  caught him after two candies. He left with one tap on the top sign and was back on Home. Radar showed planes
  moving, and a tap changed the range. It was pleasant but text-heavy for a non-reader. The house returned him to
  Home, and Home then led to All games without trouble.

## Gesture and interruption matrix

| Gesture | Tile (star mode) | Star button | Check | House |
| --- | --- | --- | --- | --- |
| Single firm tap (200-400 ms) | ok | ok | ok | ok |
| Double tap, 120-150 ms gaps | acts once | ok (2nd dropped) | ok | ok |
| Double tap, 450 ms gap | **flips back** (F4) | not tested | not tested | not tested |
| Triple tap, fast | acts once | **enters then leaves** (F1) | ok, stays out | lands 2 screens up (F5) |
| Triple tap, slow (400/300) | not tested | not tested | **re-enters** (F2) | not tested |
| Quadruple tap, fast | not tested | not tested | **re-enters** (F2) | not tested |
| Long press 1 s | toggles once | enters | not tested | not tested |
| Drag off before release | not tested | not tested | cancels | not tested |
| Off-center by about 6 px | ok (tile area) | ok | ok | not tested |
| Leave and resume (picker and back) | stars kept (Run B) | n/a | n/a | n/a |
| Process restart / `reboot` | **gap** | **gap** | **gap** | **gap** |

## Gaps

- No process restart or `reboot` check of saved stars (only the picker round-trip).
- I didn't test the second Library page (only six games on page 1 were used), or a full Home with a star refused
  on another page ("Stars: page N").
- Slow double and triple taps on the star button and the house, and drags off tiles, were not tested.
- Run A ran without a writable save folder. Whether the shell gives any sign when a favorites write fails was not
  judged: it showed none, but this is a test setup problem and was not confirmed as a device behavior.
- Emulator only. Real finger timing on the capacitive panel, and real children's tap rhythm, were not measured.

## Automated suite

`make -C apps/porthole playtest` at `811293c` (snap sha256 as above): 93 of 93 scenarios pass, 0 UI FAIL, 0 UI
WARN. I read `build/playtest/report.md` in full; it has no findings. The suite does not cover F1 or F2. The repro
scripts in `repro/` are candidates for `b2_home_stars`-style regression checks; I did not add them to
`tests/playtests/`, because they would fail on this build until it is fixed.

## Evidence

- `launcher-home-playtest-6/runA/`, `runB/`: native 480x480 frames, named in play order.
- `launcher-home-playtest-6/runA-input-trace.txt`, `runB-input-trace.txt`: every serve command sent (`down`,
  `tick`, `up`, `tilt`, `shot`).
- `launcher-home-playtest-6/sheet-*.png`: half-size contact sheets of the key sequences.
- `launcher-home-playtest-6/repro/*.txt`: raw `snap --script` repros for F1 and F2, each confirmed in a fresh
  process.

## Verdict

**Needs fixes.** Double and triple taps on a game's star, and quick taps on two games, now behave the way Leo
means. But because the star button and the check share one spot, a triple tap on the star button bounces him
straight back out of star mode (F1, Major). A slow or four-tap burst on the check bounces him back in (F2). Both
need a burst-aware guard in both directions, followed by a retest of this flow.
