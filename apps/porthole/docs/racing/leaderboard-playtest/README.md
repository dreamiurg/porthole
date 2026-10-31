# Rally leaderboard: black-box playthrough (emulator roleplay)

This is emulator roleplay by an independent playtester agent. No real child played it and no physical board was
used. The playtester did not read `games/racing/`, `host/test_racing*` or the racing playtest scenarios, and used no
`dbg`, `ui`, `app` or hidden-state queries. Every action was chosen from rendered 480x480 frames.

## Build and setup

- Build: commit `3fa7529` (feat(porthole): Rally leaderboard of every kid best times), frozen emulator `snap3` (the commit is the coordinator's statement and was not verified by the playtester),
  sha256 `2bd27d052ea29a601da748eb4eded10df3aa6b78832aa78bc79e5e6f91d82dc4`.
- Fresh working directory. Fixture (`--script`): `reset`, `tilt 0 1000 0`, `profile Zoe 9`, `profile Sam 6`. Then
  `snap3 --serve` without `--fresh`, driven through the `down/move/up/tick/tilt/frame` protocol. There was one
  process restart (reboot) before Zoe's Night Run.
- Steering helper: it read only the rendered pixels (the road's extent at two rows ahead and the red car's x) and
  turned the error into a wheel angle `θ` (`tilt 1000·sinθ 1000·cosθ 0`, with a lean-back `φ` scaled in). The
  playtester chose every boost and rocket tap, lean, flat and oversteer phase, and every menu tap. A frame was saved
  and inspected at least every 2 s of play (contact sheets of every 2 s frame, plus native frames at key moments).
- Harness note: the first race used `tick 100`. The emulator steps whole 33 ms frames, so each tick ran about 133 ms
  of game time and the helper steered a little more coarsely. Every later race used `tick 99`. In race 1 the 2 s inspection cadence was therefore about 2.7 s of game time. The game clocks
  themselves are consistent (Sam's paused race: 1:32.5 shown, about 98 s ticked minus about 6 s paused).
- Input trace: `sam-desert-canyon-race.jsonl` covers Sam's whole session, from the "Who's playing?" tap through
  Rally, the grip page, a full 3-lap Desert Canyon race (leans of 45-70°, oversteer, repeated Boost/Rocket taps,
  one flat-pause and Resume) to the results page. `t` is the cumulative requested tick in ms, and `frame` lines mark
  inspection points. It is an input record. It was not replayed in a fresh process, so no claim is made that it
  reproduces the outcome. It also needs Zoe's earlier Desert Canyon save to show the same "To beat".

## Persona and dispatch brief (as received, unchanged)

Two siblings share one board:

- Zoe, 9: reads well and is competitive. She tries to beat records and steers smoothly but sometimes too hard.
- Sam, 6: an early reader and impatient. He taps things repeatedly, leans the puck back and forth, sometimes holds
  it flat-ish and mis-taps near buttons.

The brief asked for judgments on comprehension, boredom, whether the goal ("beat the best time") is clear and
motivating, and playability. It required covering: Zoe's full 3-lap Desert Canyon race and a replay to beat it;
Sam's Desert Canyon race with the grip page showing the time to beat, sloppy leaning (pause only when nearly flat),
one flat lay mid-race and the board with both kids; a race on another track whose board starts empty; the speed arc
and text legibility; and the exits Results → Tracks → Home → launcher. No solutions or winning inputs were given.

## What was played (results as shown)

| # | Kid | Track | Outcome on screen |
| --- | --- | --- | --- |
| 1 | Zoe | Desert Canyon | 1st, `1:30.4  New best!`, board `1 Zoe 1:30.4  lap 29.2` |
| 2 | Zoe (Replay) | Desert Canyon | 1st, `Your time 1:32.3` (did not beat it), board unchanged |
| 3 | Sam | Desert Canyon | grip page `To beat: Zoe 1:30.4`; 1st, `1:32.5  New best!`, board `1 Zoe 1:30.4 lap 29.2` (white), `2 Sam 1:32.5 lap 29.6` (yellow) |
| 4 | Sam | Snow Track | 1st, `1:54.3  New best!`, board `1 Sam 1:54.3  lap 37.1` only (started empty) |
| 5 | Sam (Replay) | Snow Track | paused, Restart → `Start over?` → No; Home → `Go home?` → Yes → launcher |
| 6 | Zoe after reboot | Desert Canyon grip | `Your record 1:30.4` (persisted) |
| 7 | Zoe | Night Run | about 24 s of lap 1 (HUD legibility), then hold-pause → Tracks → `Pick a new track?` → Yes → Home → launcher |

There were no losses in the ordinary sense. Every finished race placed 1st. Sam's sloppy inputs cost only about
2 s, which is partly a limit of the pixel helper (see "Not covered").

## Qualitative assessment

**Can the persona recognise the goal and actions?** Mostly yes. The red car tile reads as racing on the second
launcher page. The track picker is four big picture cards (`01`). "How to race" uses pictures plus three short lines,
and the grip page lets a kid turn and watch the car move before Go, which is a good wordless check. On Sam's grip
page `To beat: Zoe 1:30.4` (`02`) states the goal plainly and names a sibling, which is motivating. After a reboot,
Zoe sees `Your record 1:30.4` (`09`). There is some friction:

- `Hold 1/3 to pause` is cryptic before a kid has seen the HUD. Sam (6) would not connect "1/3" to the lap counter.
  In-race, the `II Hold` label under `1/3` makes it discoverable.
- "Grab boosts and rockets" never says to tap the round icons to use them. Zoe would find out by tapping. Sam taps
  everything anyway.
- `Recenter` on the pause page is not a 6-year-old's word.

**Feedback, skill and decisions.** The speed arc reads as speed (`03`, `04`). It fills green→yellow→orange with the
mph number at 90, turns cyan and full with a cyan number during a boost, and turns red with a red number and a
debris puff after a hit (e.g. `59 mph`, `81 mph`). Traps clearly cost speed. Place (`4th`→`1st`) and lap (`1/3`) are
large, outlined and legible on Desert, Snow and Night (`10`). Boosts are a real choice early on, when rivals are
close. Rockets are less meaningful: a rocket tapped while 1st with nobody in view is spent with nothing to hit (Snow
lap 2, 0 → spent at ~51 s).

**Repetition, boredom and stopping.** Each race is long for Sam: Desert about 92 s, Snow about 114 s. On every
track the player reached 1st by about 10 s, and in races 1, 4 and 7 no rival was visible afterwards. That leaves
80-100 s of solo driving, broken only by pickups, ramps, mud and ice. For Zoe the clock is the real opponent and the
results page gives her that (`New best!` or `Your time 1:32.3` under the board). She would plausibly replay 2-3 times
to chase 1:30.4. Sam's first voluntary stop is likely the results page of his first race. He "won" (1st, green "New
best!"), and a second 2-minute solo lap-fest on Snow is where a 6-year-old drifts. The natural stopping point (the
results page with Replay/Tracks/Home) is clear and calm.

**Recovery and imprecise input.** This is good. Leaning back 45-70° never paused. Lying flat paused within about
0.5 s with `Hold like a wheel` and a disabled Resume (`05`). Tilting back up showed `Paused` with Resume enabled, and
the race resumed with a `Ready` count. Restart, Tracks and Home on the pause page each ask `... This race ends.
Yes / No`, so a boost-mashing tap that lands on Restart (tested at logical (130,80)) cannot lose a race outright. A
tap within about 0.5 s of the pause page appearing was ignored. A quick tap on `1/3` does not pause, and a 2-3 s
hold does. A double tap on the Rally tile and on Go did not skip or double-activate. A mis-tap beside Sam's name
still picked Sam. Best times survived a process restart.

## Findings

No Blocker or Major issues were found.

### Minor

1. **Results: "New best!" right after "To beat: Zoe" reads as beating Zoe.**
   - Screen: results page.
   - Concern: comprehension.
   - Repro: as Sam, open Desert Canyon (grip page shows `To beat: Zoe 1:30.4`), finish slower than Zoe (1:32.5).
     Results show green `1:32.5  New best!` while Zoe is still `1` on the board.
   - Detail: a 6-year-old told "to beat Zoe" who then sees "New best!" in celebratory green will think he beat her.
     The board's row order is the only correction, and Sam cannot read it quickly.
   - Suggestion for the owner: say "Your best!" or show the gap, e.g. "Zoe is 2.1 s faster".
   - Screenshot: `07-results-board-zoe-sam.png`, `02-grip-to-beat-sam.png`.
2. **Rivals stop mattering about 10 s in, leaving 80-100 s of solo driving.**
   - Screen: race.
   - Concern: boredom / challenge.
   - Repro: any race with smooth steering and one boost. Desert run 1 was 1st at 10.1 s with no rivals visible to
     the end. Snow was 1st at 10.3 s and alone to 1:54. Night was 1st at 8.3 s.
   - Detail: Zoe has the clock to chase. Sam has nothing on screen to race for the last 2 laps, and Snow's 3 laps
     take nearly 2 minutes.
   - Screenshot: contact sheets in the playtester's scratch (`sheet-snow-laps2-3-every-2s.png` (a contact sheet at a 2 s cadence, downscaled, not a native frame)). Representative frame:
     `10-night-run-hud.png` (1st, empty road).
3. **"Hold 1/3 to pause" is cryptic on the how-to page.**
   - Screen: How to race.
   - Concern: comprehension.
   - Repro: open any track for the first time in a session.
   - Detail: the line refers to a HUD element the kid has not yet seen. An early reader cannot decode "1/3".
     In-race `II Hold` helps.
   - Screenshot: `12-how-to-race.png`.
4. **The track picker shows no records.**
   - Screen: Pick a track.
   - Concern: meaningful choice / motivation.
   - Repro: after Zoe and Sam set Desert and Snow times, open Rally. All four cards look the same as on first launch
     (`01`).
   - Detail: a record-chasing 9-year-old cannot see which tracks she holds or where her sibling beat her until she
     opens the grip page.
   - Screenshot: `01-track-picker.png`.

5. **The race HUD shows no clock, and Replay skips the time to beat.**
   - Screen: race HUD / Replay.
   - Concern: feedback / motivation.
   - Repro: results → Replay. The race starts straight at the countdown (`13`), so the grip page with `Your record`
     or `To beat` is skipped. During the race the HUD shows mph, lap, place and charges but no elapsed or lap time
     (`03`, `10`).
   - Detail: on Zoe's "beat 1:30.4" attempt the target appeared nowhere until the results page, so she cannot tell
     mid-race whether she is ahead or behind. The goal is clear before the first race and after each race, but not
     while chasing it. Finding 10 is the results-page side of the same gap.
   - Screenshot: `13-replay-starts-without-target.png`, `10-night-run-hud.png`.

### Nit

6. **`Ready` overlaps a lap banner after Resume.**
   - Screen: race, after Resume.
   - Concern: overlap.
   - Repro: lay the board flat right as a lap completes (Sam's Desert race, about 34 s, lap 1 → 2), tilt up and tap
     Resume. `Ready` draws on top of the `Lap 2` banner text, with the banner's lower half showing under it.
   - Screenshot: `06-ready-overlaps-lap-banner.png`.
7. **A rocket can be spent with no target.**
   - Screen: race.
   - Concern: feedback.
   - Repro: be 1st with no rival in view and tap the rocket. The count drops and nothing visible is hit.
   - Detail: Sam will waste them. Either keep the charge or show it flying off.
   - Screenshot: the 2 s scratch sheet `/private/tmp/claude-501/-Users-dreamiurg-src-dreamiurg-porthole--claude-worktrees-position-sensor-sand-app-a148d0/a98258df-f1ce-4e38-8571-de57bc45c177/scratchpad/blackbox/lb1/n_b.png` (r0051.8: rocket 1 → 0 while alone). It was not copied, so the link dies with the session..
8. **`Recenter` is an adult word on the pause page.**
   - Screen: Paused.
   - Concern: comprehension.
   - Repro: pause any race.
   - Screenshot: `05-flat-pause-hold-like-wheel.png`.
9. **"How to race" reappears after a reboot.**
   - Screen: How to race.
   - Concern: pacing.
   - Repro: restart the process and open a track as Zoe, who has raced before. It costs one tap and is harmless.
   - Screenshot: `12-how-to-race.png` (same page).
10. **Results don't say how close a non-record run was.**
   - Screen: results.
   - Concern: feedback.
   - Repro: Zoe's replay shows `Your time 1:32.3` with board `1 Zoe 1:30.4`. A competitive 9-year-old would like
     "+1.9 s".
   - Screenshot: `11-zoe-replay-result-not-beaten.png`.

## Gesture and interruption matrix

| Gesture | Where | Result |
| --- | --- | --- |
| Double tap | Launcher home button, Rally tile, grip-page Go | One activation. The second tap was ignored by the new page. |
| Off-centre tap | `Sam` row (far right of the row) | Picked Sam |
| Tap while Go is grey | Grip page within about 1 s of opening (`Hold it steady` bar filling) | Ignored. Go lights after the bar fills. Visible, so not a finding. |
| Lean back 45°, 55°, 60°, 65°, 70° | Mid-race | Keeps racing, no pause |
| Lean back 80°, 85° | Mid-race | Pauses with `Hold like a wheel` (`14`) |
| Lean back 60° | Grip page | `Lift it up`, Go disabled |
| Flat | Mid-race | Pauses within about 0.5 s, Resume disabled until held up (`05`) |
| Tilt back up, tap Resume | Pause page | `Ready` count, then the race continues. The clock excludes the pause. |
| Quick tap on `1/3` | Mid-race | No pause |
| Hold `1/3` 2-3 s | Mid-race | Pauses |
| Tap within about 0.5 s of the pause page appearing | Pause page (boost spot) | Ignored |
| Boost-spot mis-tap (130,80) | Pause page → lands on Restart | `Start over? This race ends.` → No returns to the pause page |
| Repeated Boost/Rocket taps with 0 charges | Mid-race | Nothing happens. No error, no harm. |
| Home / Tracks | Pause page | Confirmation, then Yes → launcher / track picker |
| Process restart | After races | Profiles and best times kept (`09`) |

Untested: dragging a finger off a Rally button before release; long-press on Go, Replay or Tracks; a touch carried
from the race into the pause page; lean between 70° and 80°; shaking; and Industrial Ruins.

## What worked

- `To beat: Zoe 1:30.4` on the grip page and `Your record 1:30.4` for the holder: a clear, sibling-named goal.
- The per-track board lists both kids in time order, highlights the current kid's row in yellow and the other in
  white (`07`). Snow's board started empty and showed only Sam (`08`). Records persist across a restart.
- The speed arc and mph number communicate boost (cyan) and hit loss (red) without words.
- Lean-back tolerance is generous. Flat-pause is quick and explains itself, and Resume is disabled until the board
  is held up again.
- Every destructive exit (Restart, Tracks, Home) is confirmed. FreshGate ignored carried and double taps.
- Exits are complete: Results → Home goes straight to the launcher; Results → Tracks → (Snow) and Tracks → Home →
  launcher; Pause → Tracks/Home → confirm → picker/launcher.

## Not covered and limits

- Industrial Ruins was not raced. Night Run got only about 24 s, not a full race.
- Steering was a pixel P-controller with scripted lean and oversteer multipliers. It recovers from oversteer faster
  than a real 6-year-old, so Sam's 1:32.5 is optimistic. Hard crashes into walls, sustained off-road driving and
  last-place finishes were not reached, so a frustrating losing race was not observed.
- Physical wheel feel, accelerometer noise, hand fatigue over a 2-minute race and real reading ability are untested.
  This is emulator roleplay, not a child study or a device test.
- The input trace was not replayed in a fresh process.

## Automated suite (supplementary)

`make -C apps/porthole playtest` was run in this worktree at `dca4176` (the branch HEAD at test time: a merge of main
on top of `3fa7529`; it changes nothing under `games/racing/`, only shell/os/host code from main). The result was 79
of 79 scenarios passing, with 0 UI FAIL and 0 UI WARN, including `a0_racing_flow`, `a1_racing_monkey`,
`a2_launcher_pages` and `a4_racing_board`. The black-box pass used the frozen `snap3` (`3fa7529`), not the
`dca4176` build, so the shell/os changes merged from main were not black-box played here.

## Judgment

Ready from a playability standpoint for the leaderboard change. No Blocker or Major issue was found. Finding 1
("New best!" vs "To beat") is the one most worth fixing for the 6-year-old. Finding 2 (rivals irrelevant after
about 10 s, long solo laps) is a pacing concern for game-designer. It is not about the leaderboard itself, and this pass cannot tell whether it is new.

## Resolutions (coordinator, after this pass)

| # | Finding | Resolution |
| --- | --- | --- |
| 1 | "New best!" after "To beat: Zoe" reads as beating Zoe | Fixed: "Your best yet!" for an own best race, "Best lap yet!" for a lap only, "Track record!" only when the kid tops a board of two or more (`record()` now reports which improved; `host/test_racing.cpp`) |
| 2 | Rivals stop mattering about 10 s in | Open: a rival pace/balance decision, raised with the user |
| 3 | "Hold 1/3 to pause" is cryptic | Fixed: "Hold 1/3 at the top to pause" |
| 4 | Track picker shows no records | Open: the cards have no room for a line; the grip page shows the time to beat |
| 5 | Replay skips the time to beat; no race clock | Partly fixed: the countdown shows "To beat ..." / "Your record ..."; no in-race clock (HUD space) |
| 6 | "Ready" over the "Lap 2" banner | Accepted (both readable, rare) |
| 7 | A rocket can be spent with nothing to hit | Accepted: rockets can miss by design |
| 8 | "Recenter" is a grown-up word | Fixed: "New grip" |
| 9 | How to race reappears after a restart | Accepted: shown once per visit by design |
| 10 | No gap shown on a slower run | Fixed: "1:32.3  (+1.9)" |

The fixes were checked in the emulator (own best, Replay countdown target, How to race wording) and by `make check`.
