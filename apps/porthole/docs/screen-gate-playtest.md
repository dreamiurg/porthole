# Screen-change tap gate and Pumpkin signs: black-box kid playtest

Emulator roleplay by an independent playtester agent (`playtester`, opus). Simulated perspectives only:
no real child played and no physical board was used.

## Build

- Revision: `6a72c89` (`fix(porthole): a steady retrier gets past the fresh gate within 2.5 s`), clean tree.
- Executable: `apps/porthole/build/host/snap` copied to a private scratch folder, sha256
  `bf787c9d41d1f2b0313999e56a5927f03732e32b97c93a8912800ef82211fb3f` (the same hash as the suite's snap after `make playtest`).
- Mode: `snap --serve --fresh`, default clock (Tuesday 16:00), fresh device each replay. Profiles were created
  through the screens (Leo, age 6, frog, no code; Zoe, age 8, fox, no code). No `dbg`, `app`, `profile`, clock skips or
  state queries during play. Tilt went through the serve protocol's `tilt X Y Z` (milli-g).

## Dispatch brief (verbatim, as received)

> You are an independent child-perspective playtester for Porthole, a round 480x480 touchscreen with no buttons, emulated on the host. Follow apps/porthole/docs/playtesting.md and the `playtest` skill. Play source-free: don't read source (apps/porthole/games/, os/, shell/, host/), tests/playtests scripts, docs/superpowers/ or earlier playtest reports until you finish. Disclose anything that loaded automatically. Decide from the screen. Run the sim from your own copy of the snap binary in a private scratch subfolder.
>
> Personas (two passes)
> A. Leo, 6, can't read, taps firmly (200-400 ms presses), mashes 3-6 times, taps "again!" about once a second when nothing seems to happen, and rests a thumb on the screen while tilting.
> B. Zoe, 8, a confident reader who taps deliberately and quickly and gets impatient with slow buttons.
> Simulated perspectives only.
>
> Setup
> - `make -C apps/porthole snap`. Record `git rev-parse --short HEAD` and `sha256sum` of the snap you run.
> - `snap --serve --fresh`, default clock. Create the profile (age 6 or 8, no code) through the screens; ordinary touches only; no dbg, shortcuts or clock skips. Tilt inputs through the serve protocol are allowed for the tilt game.
>
> Scope
> This build changes how every screen treats taps right after it appears, and how Pumpkin Hide & Seek's in-game signs respond.
> - Leo: mash through the picker, Home, All games, a game and back. Does a tap ever open or change something on the NEXT screen that he didn't aim at? When he taps "again!" once a second, how long until something happens, and does anything feel broken or dead?
> - Zoe: quick deliberate navigation through several screens and games (Pets Club, Pumpkin, a tilt game, the star picking in All games). Does anything feel slow or ignore her?
> - Pumpkin: play two rounds. Press HIDE short, long (2-3 s), double-tap it, and rest a thumb on it from before the round starts. Does hiding feel immediate and fair? Use the ghost too.
>
> Output
> Write apps/porthole/docs/screen-gate-playtest.md, with screenshots and the input trace in apps/porthole/docs/screen-gate-playtest/ (no .py files; save scripts as .txt). Include the revision and hash, the personas, this brief, findings ranked with repro steps, the assessment and gaps. Don't fix code or commit. Reply with ONLY (under 8 lines): a verdict (acceptable / needs fixes), the top findings, and the report path.

Brief revisions: none in this pass.

## Disclosure (what I knew before playing)

- Loaded automatically: the root `CLAUDE.md` and `apps/porthole/CLAUDE.md`. The latter describes the change's intent in
  detail: `FreshGate` ignores a press that begins within 800 ms of a screen change, and within 800 ms of the lift of
  any ignored press, until 2.5 s after the change; a finger held across a change does not re-arm it; Pumpkin's action
  sign and ghost act as a press lands. I therefore knew the timing rules and designed some probes (cadence, resting
  thumb) to test their edges. It contained no Pumpkin solution.
- Read on purpose: `apps/porthole/docs/playtesting.md`, the `playtest` and `tour` skills, and a grep of
  `apps/porthole/tools/webemu.py` for the serve protocol (`down/move/up/tick/tilt/frame`, `ok` replies, raw 480x480
  RGB frames).
- Not read: any source under `games/`, `os/`, `shell/`, `host/`, any `tests/playtests/*.txt`, `docs/superpowers/`,
  or any earlier playtest report. The automated suite ran only after the black-box passes; I read its report, not its
  scenarios.
- Supplementary (not black-box) timing probes used `snap --script` with the `screen` query; they are labeled below.

## Method and input evidence

A small private driver (not committed; no `.py` here) replays a trace into `snap --serve --fresh` and saves frames.
Every trace under `screen-gate-playtest/traces/` is a plain list of serve commands plus three macros:
`tap X Y MS` (down, `tick MS`, up; 120 ms if omitted), `shot NAME` (save the 480x480 frame) and
`strip NAME MS STEP` (a frame every STEP ms). Coordinates are logical px (0-160). Each decision was made by looking at
the latest frames; a trace was then extended and replayed from a fresh device. The traces were replayed once more
in fresh processes to build the sheets in `screen-gate-playtest/shots/`, and they reproduced every outcome described below.

| Trace | What it plays |
| --- | --- |
| `leo_base1.txt` | Leo: name, face mash, age mash, skip, Home, house mash to the picker, name-row mash, All games mash, Pumpkin tile mash, START, READY, a first passive loss, TRY AGAIN, a hide under the table, the table check loss |
| `leo.txt`, `leo_rest.txt` | Leo's thumb lands on HIDE 200 ms after the round starts: lifted and pressed again later (`leo.txt`), and kept there (`leo_rest.txt`) |
| `pk.txt` (saved as `pk_dbl.txt`), `pk_mash.txt`, `pk_1hz.txt`, `pk_ghost.txt` | Calibrated round: double-tap HIDE, a 5-press mash, taps at 1 Hz, the ghost |
| `pk_full.txt` | Full natural Pumpkin run: losses, a long press, the ghost, three candies, the escape win |
| `pk_exit.txt`, `again.txt`, `hm.txt`, `ag6.txt`, `ta2.txt` | Exit mash, "again!" taps from Home to the Pumpkin tile, a 6-press house mash, a 6-press All-games mash, TRY AGAIN with "again!" taps |
| `zoe.txt` | Zoe: name, face, age, skip, Pets Club (adopt, name, house, feed, sticker), Home, All games, star mode, Marble Kick, Pumpkin START |
| `gate_leo.txt`, `gate_zoe.txt` | Supplementary: time until a retrier gets through a new screen (All games, then a Library tile), by press length, cadence and first-tap delay |

## What happened (first play, by persona)

### Leo, 6 (non-reader, firm 200-400 ms presses, mashes, "again!" taps at 1 Hz, thumb resting on the screen)

- Profile creation: four firm presses on the frog face carried nothing onto "How old are you?". Four presses on "6"
  landed on the code pad's "2" key and typed no digit (`shots/leo_01_creation_and_picker_mash.png`).
- Five presses on Home's house left him on "Who's playing?" with nothing selected. Five presses on his name row opened
  Home and did not open the dog tile under his finger. A four-press mash on "All games" stopped at the Library with
  Radar unopened, and a five-press mash on the Pumpkin tile stopped at Pumpkin's title (`leo_01`, `leo_02`).
- Exit: a five-press mash on Pumpkin's back picture returned to the Library without touching its house
  (`leo_11_exit_mash_to_library.png`).
- Longer mashes do leak: six presses about 0.55 s apart on Home's "All games" opened **Radar**
  (`leo_13_six_tap_mash_opens_radar.png`), and six on the Library's house went past Home to the **picker**
  (`leo_12_house_mash_reaches_picker.png`). See m2.
- "Again!" taps (300 ms presses, 1 Hz): Home's Pumpkin tile opened on his third tap, about 2.3 s after Home appeared
  (`traces/again.txt`). The ignored taps showed no pressed look and no name caption, so the tile looked dead for two
  seconds. In the supplementary table, a 1 Hz retrier with 300-400 ms presses gets through on tap 2-4, 1.1-3.1 s after
  the change.
- Pumpkin, first rounds: he lost the first round by not acting, the second because he stayed under the table while
  Grandma said "THE TABLE?" / "RUN AWAY!" (text he cannot read), and the third because he pressed the sign while it read
  "FIND COVER" (away from a hiding spot), which did nothing. He won the fifth round by escaping through the glowing door
  with three candies (`pumpkin_10_escape_win.png`).
- Thumb resting on HIDE: his thumb landed on HIDE 200 ms after TRY AGAIN started the round and stayed there. The
  pumpkin never hid. Grandma came, said "QUICK! HIDE!" and caught him **while his thumb was on HIDE**
  (`leo_03_thumb_resting_on_hide_caught.png`, `leo_03b_quick_hide_with_thumb_on_hide.png`). The sign looks exactly
  as it does when nothing touches it (`leo_03c_hide_sign_no_pressed_state.png`, a pixel-identical crop). When he
  lifted and pressed again, the pumpkin hid at once (`leo_04_thumb_lift_and_repress_hides.png`). See M1.

### Zoe, 8 (reads everything, quick 100-120 ms deliberate taps, retaps after about 0.5 s when ignored)

- Within one screen everything was instant: name keys and page switches, Pets Club's food menu, the cookie, the
  sticker card, unstarring and starring inside star mode, Marble Kick's play and level-up.
- At almost every **screen change** where she tapped her next target within 0.8 s, she was ignored three times and got
  in on the fourth tap, 2.3-2.8 s later:
  - "How old are you?": taps on 8 at about 0.6, 1.2 and 1.8 s were ignored; the fourth worked (`zoe_01_age_three_taps_ignored.png`).
  - Pets Club "Pick your house!": taps at 0.7, 1.4 and 2.1 s were ignored; the fourth, at 2.8 s, worked (`zoe_02_house_three_taps_ignored.png`).
  - Home to "All games": taps at 0.7, 1.3 and 1.9 s were ignored; the one at 2.5 s worked (`zoe_03_home_to_allgames_three_taps_ignored.png`).
  - Star mode (the Library's star button): Radar's star ignored three times before "Home full! Tap a star" appeared
    (`zoe_04_star_mode_taps_ignored_then_stars.png`). Nothing sits under the star button in star mode, so there was no
    mash to protect against here.
  - After the green check, back in the Library: the stadium tile was ignored three times before Marble Kick opened (`zoe_05_after_check_three_taps_ignored.png`).
  - Pumpkin's round start: HIDE ignored three times at 0.6 s spacing (`zoe_07_pumpkin_start.png`).
  - When she waited about a second before tapping (the code pad's skip, Library star button, Pets Club package,
    puppy, READY), her first tap worked.
- Supplementary table (`gate_zoe.txt`): if the first tap comes before 800 ms and she retaps every 0.45-0.8 s, the
  screen ignores 2-4 taps and she gets through only at 2.2-3.0 s. One tap after 800 ms always works on the first try.
- Marble Kick's home button ignored her quick double tap; it needs a hold (a dotted ring fills), and her quick tap
  showed no hint of that (`zoe_06_marble_kick_and_hold_home.png`). This was not part of the change. See n1.

### Pumpkin sign probes (both personas)

| Probe | Result | Evidence |
| --- | --- | --- |
| Short tap (150 ms) at a spot | Hides on the press; sign reads COME OUT at once | `traces/pk_full.txt` (p12 area), `leo_04` |
| Long press 2.5 s | Hides as the press lands; the lift does not come back out | `pumpkin_09_long_press_hide.png` |
| Double tap (150 ms apart) | Hides once and stays hidden | `pumpkin_05_double_tap_hide.png` |
| Mash, 5 presses | Hides once and stays hidden | `pumpkin_06_mash_hide.png` |
| Taps at 1 Hz | Toggles hide / come out / hide each second | `pumpkin_07_1hz_taps_toggle.png` |
| "Again!" taps through TRY AGAIN | TRY AGAIN on tap 2; taps 3-4 on HIDE ignored; tap 5 hides; **tap 6 comes out in front of Grandma** | `leo_14_try_again_again_taps_come_out.png` |
| Thumb resting from round start | Never hides; caught with the thumb on HIDE | `leo_03*` |
| Press while sign says FIND COVER | Pressed look only, nothing happens, no other cue | `pumpkin_11_find_cover_press_does_nothing.png` (`pk_full.txt`, p08) |
| Ghost | Immediate: "WHAT WAS THAT?", a sparkle across the room, Grandma leaves, the ghost dims (once per round) | `pumpkin_08_ghost.png` |

## Findings (prioritized)

### M1 (Major): a thumb resting on HIDE from the round's start never hides the pumpkin
- **Screen**: Pumpkin Hide & Seek, play.
- **Issue**: a press that lands on HIDE in the first 0.8 s of a round is ignored for as long as it is held. Grandma
  arrives about 3 s in and catches the pumpkin while the child's thumb is on HIDE, and the sign shows no pressed state.
- **Concern**: challenge / feedback / recovery (unfair loss; the child did the right thing and cannot see why it failed).
- **Repro**: `docs/screen-gate-playtest/traces/leo_rest.txt`: lose a round, tap TRY AGAIN (300 ms press), `tick 200`,
  `down 80 130` and hold for 12 s. Frames 3-8 show "GRANDMA!", "HMM?", "QUICK! HIDE!", then the pie.
- **Screenshot**: `apps/porthole/docs/screen-gate-playtest/shots/leo_03_thumb_resting_on_hide_caught.png`,
  `leo_03b_quick_hide_with_thumb_on_hide.png`, `leo_03c_hide_sign_no_pressed_state.png`.
- Suggested direction (owner's call): honor a press still held on HIDE once the gate's window ends (the brief's
  "rest a thumb on it from before the round starts" case), or at least draw it pressed and act when it is still down at
  the gate's end. Re-pressing worked at once (`leo_04`), so only the held case is affected.

### M2 (Major): a quick reader is ignored for 2-3 s after most screen changes
- **Screen**: every screen change tested: age, Pets Club house, Home to All games, star mode entry, back from star
  mode, Pumpkin round start.
- **Issue**: a first tap within 0.8 s is ignored, and each quick retry (lifted less than 0.8 s before the next)
  re-arms the gate, so 3 deliberate taps die and the 4th works at 2.3-2.8 s. The dead taps get no pressed look or name caption.
- **Concern**: pacing / feedback / consistency ("slow buttons", "it's ignoring me" for exactly the impatient reader in the brief).
- **Repro**: `traces/zoe.txt` (age: `tap 37 102 120` at 0.6 s after the age screen, then every 0.62 s), or from Home
  with any profile: tap All games, then tap a Library tile at 0.7 s and every 0.6 s with 120 ms presses. Table:
  `traces/gate_zoe.txt` (e.g. `press=120 every=600 first=400: acted on tap #5 (pressed at 2800 ms)`).
- **Screenshot**: `apps/porthole/docs/screen-gate-playtest/shots/zoe_01_age_three_taps_ignored.png`, `zoe_02_...`,
  `zoe_03_...`, `zoe_04_star_mode_taps_ignored_then_stars.png`, `zoe_05_...`.
- Suggested directions (owner's call): re-arm only for presses near where the change-causing press was (a mash
  repeats one spot, while a reader's next tap usually goes somewhere else); don't gate in-place mode switches with
  nothing new under the triggering finger (star mode on and off); and give a gated press the normal pressed look or
  caption so it reads as "wait" rather than "broken". Re-test Leo's carry cases after any change.

### m1 (Minor): "again!" taps at 1 Hz wait up to 3.1 s with no visible response
- **Screen**: any new screen (measured from All games to a Library tile; seen on Home to the Pumpkin tile).
- **Issue**: with 300-400 ms presses at 1 Hz and a first tap within 0.3 s, the tap that acts is the 3rd or 4th
  (2.3-3.1 s). This is 0.6 s past the documented 2.5 s, because the cap only lets in a press that *begins* after 2.5 s.
  Ignored presses show nothing.
- **Concern**: feedback / pacing.
- **Repro**: `traces/again.txt` (tail), `traces/gate_leo.txt` (`press=300 every=1000 first=100: acted on tap #4 (pressed at 3100 ms)`).
- **Screenshot**: `apps/porthole/docs/screen-gate-playtest/shots/leo_15_again_taps_home_to_pumpkin.png` (Home stays put for two taps, then Pumpkin's title).

### m2 (Minor): a sustained mash still reaches the next screen
- **Screen**: Home to Library (All games), Library to Home to picker (house).
- **Issue**: six firm presses about 0.55 s apart (or five about 0.7 s apart) cross the 2.5 s cap. "All games"
  opened Radar, a game Leo never aimed at, and the Library's house went two screens back, to "Who's playing?".
  Both were recoverable with one tap.
- **Concern**: recovery / meaningful choice (the documented trade-off; six presses is within the persona's 3-6).
- **Repro**: `traces/ag6.txt`, `traces/hm.txt`.
- **Screenshot**: `apps/porthole/docs/screen-gate-playtest/shots/leo_13_six_tap_mash_opens_radar.png`, `leo_12_house_mash_reaches_picker.png`.

### m3 (Minor): the HIDE / COME OUT toggle lets "again!" taps expose a hidden pumpkin
- **Screen**: Pumpkin, play.
- **Issue**: taps once a second alternate hide and come out. After TRY AGAIN, Leo's fifth "again!" tap hid him as
  Grandma arrived and his sixth brought him out in her flashlight. He cannot read HIDE vs COME OUT.
- **Concern**: comprehension / feedback (non-reader).
- **Repro**: `traces/ta2.txt` (shots u5 to u6), `traces/pk_1hz.txt`.
- **Screenshot**: `apps/porthole/docs/screen-gate-playtest/shots/leo_14_try_again_again_taps_come_out.png`, `pumpkin_07_1hz_taps_toggle.png`.
- Possible directions: a short guard after hiding before COME OUT acts, or a picture on the sign (pumpkin under cover vs
  out) so a non-reader sees the state.

### m4 (Minor): pressing FIND COVER does nothing, with no non-reading cue
- **Screen**: Pumpkin, play, away from a hiding spot.
- **Issue**: the sign changes to FIND COVER and a press only shows the pressed look. Leo lost a round pressing it while
  standing in the open.
- **Concern**: comprehension (this was not the gate change, but it is part of how the signs respond).
- **Repro**: `traces/pk_full.txt`, after the ghost and the right tilt: `tap 80 130 300` while the sign reads FIND COVER (shot p08).
- **Screenshot**: `apps/porthole/docs/screen-gate-playtest/shots/pumpkin_11_find_cover_press_does_nothing.png`.

### Nits
- n1: Marble Kick's home button needs a hold. A quick double tap does nothing and shows no ring or hint
  (`zoe_06_marble_kick_and_hold_home.png`). This predates the change.
- n2: Pumpkin's way out is an arrow in a wall picture, which a 6-year-old may not read as "back". On the win screen
  "YOU ESCAPED!" shows the pumpkin back in the room beside Grandma (`pumpkin_10_escape_win.png`).
- n3: Docs drift: `docs/playtesting.md` ("Timing guards worth knowing") still says `ui::FreshGate` waits 450 ms, while
  `apps/porthole/AGENTS.md` says 800 ms with a 2.5 s cap.

## Qualitative assessment

- **Goals, actions and feedback.** Leo finds the launcher by pictures, and Pumpkin's goal (roll to candy, duck when
  Grandma comes, door glows with an arrow once all three are on the shelf) is mostly visual. Hiding is immediate and
  plain to see: the pumpkin squashes under the furniture on the press. Grandma's spot checks ("THE TABLE?"), "RUN
  AWAY!" and FIND COVER, though, are carried only by text. The "?" bubble over the table helps a little. The gate's
  ignored taps give no feedback at all, so to both kids a just-opened screen looks frozen for up to 2-3 s.
- **Decisions, skill and fairness.** Pumpkin has real choices: which spot, when to come out, and when to spend the
  once-per-round ghost. The ghost felt great: instant, funny, and it rescued an exposed pumpkin. Fairness breaks in
  M1 (caught with a thumb on HIDE) and m3 (the toggle). The losses (a pie, "OH MY! PIE!", TRY AGAIN in one tap) are
  gentle and quick to retry.
- **Repetition, frustration and stopping points.** Grandma comes back every 2-5 s, hidden windows last 2.4-4.8 s, and
  crossing the room takes about 3.5 s at a firm tilt. The tension works but is busy for a 6-year-old. Leo lost three
  rounds in a row, each for a reason he could not read, and would probably have stopped or asked for help after the
  third; with persistence the escape on round five is a satisfying natural end. Zoe would not quit over the gate, but
  three dead taps on most navigations would make the device feel "laggy" to her. She would stop at a Marble Kick goal
  or the Pumpkin escape.
- **Recovery and interruption.** Every mis-tap and mash was recoverable without adult help. Back exists on every screen
  visited, and the exit mash lands safely in the Library. The worst carries (m2) cost one tap.

## Gesture / interruption matrix

| Gesture | Shell screens | Pets Club | Pumpkin | Marble Kick |
| --- | --- | --- | --- | --- |
| Firm tap 200-400 ms | ok | ok | ok | ok |
| Quick tap 100-120 ms | ok (gated <0.8 s, M2) | ok | ok | home needs hold (n1) |
| Double tap | ok, no carry | not tested | ok | home: nothing (n1) |
| Mash 4-5 | ok, no carry | not tested | ok | not tested |
| Mash 6 | leaks (m2) | not tested | ok | not tested |
| Retry at 1 Hz | 1.1-3.1 s (m1) | not tested | toggles (m3) | not tested |
| Long press 2-3 s | not tested (picker hold-to-delete out of scope) | not tested | ok | home hold ok |
| Held across screen change | ok (Marble Kick hold-exit lift, no action) | not tested | READY/TRY AGAIN act on lift; held = nothing | ok |
| Thumb resting on a target | not tested | not tested | **fails (M1)** | not tested |
| Off-center tap | a 15 px miss below TRY AGAIN did nothing (fair) | not tested | ok | not tested |
| Tilt over/under | n/a | n/a | slow, momentum carries past spots | goal in 1.5 s |
| Leave and resume (home / back) | ok | ok | ok (round not kept; no save by design) | ok |
| Process restart | **gap** | **gap** | **gap** | **gap** |

## Gaps

- Two-finger input (a thumb resting on the floor while another finger taps HIDE) cannot be modeled: the sim takes a
  single touch, and the real touch controller's behavior is unverified. This needs a physical-device check.
- Not played: Biscuit, Tilt FC, Rally, Radar beyond the accidental open, Kingdom, Starfighter; profiles with a code
  (the code pad's gate); the rest screen; the night tint; process restart and resume.
- Timing tables are supplementary (script mode with the `screen` query), from one start screen (All games to a Library
  tile); other screens were sampled black-box only.
- No real child and no physical board.

## Automated suite (supplementary)

`make -C apps/porthole playtest` at `6a72c89`: **93 of 93 scenarios pass, 0 UI FAIL, 0 UI WARN**
(`apps/porthole/build/playtest/report.md`, read in full: an all-PASS table, an empty findings table, and contact-sheet links).
The suite does not cover M1 or M2. The repros above would make regression scenarios once the owner settles the intended behavior.

## Verdict

**Needs fixes.** For the 3-5 press mashes the gate is meant to stop, it does its job: no carry through the picker,
Home, All games, a game or back, and Pumpkin's HIDE now acts on the press for short, long, double and mashed presses.
Two Major issues remain: M1 (a resting thumb on HIDE is ignored and the pumpkin is caught) and M2 (a quick reader
loses 3 taps and 2-3 s on most screen changes). Re-test both flows with a fresh playtester after a fix.
