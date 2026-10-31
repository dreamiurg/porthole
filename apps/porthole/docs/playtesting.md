# Playtesting

Commands run from `apps/porthole/` (or `make -C apps/porthole <target>` from the repo root).

## Required black-box playthrough

Every gameplay or UI change must pass this development gate before it is called
done, its PR is marked ready, or it is merged. This is required in addition to
the automated suite below. Documentation-only and non-player-visible changes do
not require another playthrough.

Game and kids' shell flows use the representative child persona below. For a
grown-up-only settings change, use a representative adult instead; the same
independence, ordinary-input, evidence and iteration requirements apply. This
exception does not apply to games. Major changes include new mechanics, controls,
progression, difficulty/economy, onboarding or a substantial interaction redesign.

1. Build the current emulator and dispatch an independent `playtester` agent to
   play the affected flow as its representative target user, using the brief
   and iteration procedure below. Use the SDL
   simulator, browser emulator, or `snap --serve` with inspected rendered frames
   and ordinary touch/tilt inputs. Base each decision on what is visible.
   Independence is required for every game's gameplay/UI changes; new games and
   major changes require a source-free first-play pass. An implementer can do
   additional checks but cannot certify this gate alone. If delegation or the
   emulator is unavailable, report the blocked gate and leave the work incomplete.
2. Enter through the visible launcher and instructions. For a new finite game or
   changes to controls, combat or progression, complete a full natural run. For
   an ongoing game, complete the affected activity and return to its normal
   resting screen. For a shell-only change, complete the affected task. Narrow
   presentation changes require traversing the affected screens and their
   relevant recovery/exit paths, not every level in the game. Also exercise
   relevant failure/recovery, replay, back/exit, mis-taps, carried touches and
   canceled holds. Do not substitute a staged ending for a required natural run.
3. During this pass, do not use `dbg`, hidden-state queries, source-derived
   solutions, stage jumps, `app` shortcuts or state-aware bots to choose actions
   or force success. Ordinary elapsed-time stepping (`tick`/`wait`) is allowed;
   wall-clock jumps and progression cheats are not natural play. A documented
   profile/save fixture may prepare existing user progress needed to reach the
   affected activity; disclose it and traverse the changed flow normally.
   Keep debug tours, clock probes and automated state-aware tests separate.
4. Inspect native-size frames and the sequence in motion: round-screen layout,
   HUD footprint, sprite scale, clipping, contrast, text and instructions, input
   discoverability, response and feedback, collision readability, animation,
   pacing, difficulty and dead ends. Judge the experience, not just whether the
   state machine eventually reaches an ending. A blind replay of a known trace
   without watching the current experience does not replace this pass.
5. Record prioritized findings with a repro and screenshots. Use the iteration
   loop below to improve both the game and the playtester's instructions, then
   play again; any further gameplay/UI edit invalidates the earlier evidence for
   its affected flow. Verify the final build, including a fresh complete run
   when controls, combat or progression changed. Add focused regression tests
   where they can meaningfully catch the observed bug.
6. Save a concise report under `docs/<game>/` (or `docs/` for shared flows), with
   the tested revision and executable hash, persona and exact dispatch brief
   (including revisions), setup/fixtures, paths played, outcomes including
   losses, findings and resolutions across iterations, remaining limitations,
   native screenshots, and an input trace or exact reproducible steps.
   Include the qualitative assessment below. If claiming a trace reproduces
   an outcome, replay it in a fresh process. Link the evidence in the PR or final
   handoff. State emulator versus physical-device scope accurately.
7. Run the required automated checks and obtain code review. The reviewer must
   verify that the evidence covers the final changed experience. Missing or
   blocked playthroughs leave gameplay/UI work incomplete; report the gap rather
   than treating green CI or fixture screenshots as a substitute. Document any
   accepted limitation explicitly instead of silently dropping a finding.

The [Starfighter report](starfighter/blackbox/README.md) is an example: ordinary
input drove the complete game, observed issues led to refinements, and the final
victory trace reproduced the saved frame. These qualitative checks are a required
development/review gate; CI alone cannot certify that someone inspected play.

## Child-perspective playtester brief

The coordinator must give the independent agent this contract before play. The
playtester must not have implemented the changed experience. Use the game's own
brief to choose a representative age and abilities; do not apply one generic
child to every game. For a new game or major change, use a fresh agent without
implementation history, walkthroughs, solution traces or expected correct choices.
Provide only the build/start commands, input/frame protocol, audience, ordinary
player goal, scope and disclosed setup fixture. Keep implementation review and
hidden-state QA separate from this first-play session.

Record the following in the dispatch brief and report:

- **Persona:** target age, reading fluency, familiarity with this kind of game,
  patience for reading/waiting and touch/tilt precision. This is a simulated
  perspective, not evidence from a real child or a measured attention span.
- **First play:** enter through the launcher and on-screen instructions without
  coaching. Say what looks tappable, what you expect a tap to do and what you
  think the next goal is. Follow curiosity rather than a known solution. If lost,
  record the confusion and attempted recovery before trying another visible path.
- **Exploration and mistakes:** try plausible wrong choices and alternative
  strategies; tap appealing scenery and ambiguous controls; back out, retry and
  leave/resume. Check whether feedback explains the consequence and recovery.
  Finish the natural run when possible; inability to finish unaided is a finding,
  not a reason to consult the solution or force an ending.
- **Imprecise fingers:** deliberately offset taps around visible targets, try
  either side of close boundaries, double-tap after uncertain feedback, hold
  slightly too long, drag off before release, and carry a touch across a screen
  change. Try input during animations and modest tilt over/under-correction where
  relevant. Observe actual frames after each action. Judge intentional hits,
  safe cancellation and accidental activation separately; outside taps should
  not all activate a button. Random mashing alone does not cover this behavior.
- **Experience:** assess the qualitative questions below as play unfolds, with
  screenshots/steps and observed waits or repetition supporting each criticism.

The first-play discovery and deliberate mistake probes can be separate passes.
Use the required scope above: new finite games and major control/progression
changes need a complete natural run; narrow presentation changes need the full
affected activity and recovery/exit paths, not every unchanged level.

## Qualitative assessment and iteration

The report must answer these questions with observations, including a positive
case and any friction; a bare "fun" or "PASS" is not an assessment:

- Can this persona recognize the goal and available actions from the screen,
  understand the instructions, and tell what changed after acting?
- Does play offer interesting decisions or skill, understandable consequences
  and useful feedback? Do pictures help the child act, or is reading carrying
  the whole interaction? Is challenge fair and recovery understandable?
- Where does play become repetitive, passive, confusing or frustrating? Record
  specific waits, repeated actions and reading burden. At what point would this
  persona want to stop, and why? Is there a satisfying natural stopping point?
- Can imprecise input, a wrong guess or an interruption be recovered from without
  losing progress, adult coaching or pressure to keep playing?

Keep the ordinary child goal separate from coverage requirements. Record the
first voluntary stopping point before continuing for coverage; completing the
checklist does not demonstrate that a child would naturally keep playing. Use an
explicit gesture/interruption matrix and distinguish ordinary save/leave/resume
from process restart. Label untested cases as gaps rather than inferring broad
input robustness from a completed story.

Iterate until the coordinator and independent reviewer are satisfied by evidence:

1. The playtester reports prioritized findings, including mechanics/pacing and
   boredom concerns as well as layout/input bugs. Preserve unsuccessful attempts.
2. The coordinator routes fixes to the right owner: mechanics/difficulty/pacing
   to game-designer/game-engineer, controls/feedback to game-engineer, and art or
   player-facing instructions to their owners. Fix the underlying experience;
   rewriting tutorial text alone cannot excuse a dull or confusing mechanic.
3. Also revise the playtester's brief when it misses a relevant behavior, models
   the audience poorly or produces an uninformative assessment. Record what was
   changed and why. Improve persona, exploration and reporting instructions;
   never add solution hints, prescribe winning taps, remove hard cases or weaken
   acceptance merely to obtain a pass. New coverage may be refined even when the
   first brief was adequate; do not force artificial edits every round.
   Promote lessons that apply across games into `.claude/agents/playtester.md`
   or this procedure; keep game-specific personas and probes in that game's
   brief/report. Preserve the reason for each instruction change.
4. Rebuild and have the independent agent replay the final affected flow, verify
   fixes and seek new problems. Repeat as needed. A previously coached or
   source-exposed agent cannot certify first-time comprehension; use a fresh
   independent agent for that claim. Keep earlier evidence labeled by build.
5. Accept only when required paths and mistake/recovery probes are complete on
   the final build, no Blocker or Major usability/playability finding remains,
   and each actionable lesser finding is fixed or explicitly deferred with its
   player impact and rationale. Both coordinator and reviewer must state why
   the target persona can understand, control and enjoy the affected activity.
   Check the prompt revisions and remaining boredom/pacing concerns in review.
   A blocked or unsatisfactory run stays incomplete despite green CI. Do not
   claim real-child or physical-device validation from emulator roleplay.

## Automated suite

`make playtest` builds the headless simulator twice (plain, and with AddressSanitizer + UBSan) and runs every
scenario in `tests/playtests/`. The suite has three parts:

* **Playthroughs** (`01`-`13`, the shell's profiles in `10` and `13`): scripted sessions a kid would play, with assertions on the game state. They
  include the things kids do by accident: double taps, a finger that slides off a button, taps during animations.
* **Chaos monkeys** (`20`-`23`): 600 random gestures per seed from several starting points (home, the profile screens, the picker, Paw Street, the mini-games, Biscuit's home, fetch and training pad), under the sanitizers. A monkey may leave through the launcher into the other game; `debug` prints every opened game's stats, the open one last.
* **Biscuit** (`40`-`45`): naming, care, fetch, tricks to mastery, the daily cap across both games, and an audited tour of every Biscuit screen (day, evening, night, the three stages, the widest names).
* **UI audit** (`30`, plus the creation screens in `01` and the profile screens in `10` and `13`): every reachable screen is measured against the rules
  below.

```bash
make playtest                         # build both sims, run everything; exit 1 on any failure
python3 tools/playtest.py             # the same, without rebuilding
python3 tools/playtest.py 06 monkey   # only scenarios whose file name contains "06" or "monkey"
```

Output:

* one line per scenario on the console, with each failing directive and the debug output around it;
* `build/playtest/report.md`: a summary table, the failures, and every UI finding grouped by rule and screen (a
  filtered run reports only the scenarios it ran);
* `build/playtest/<scenario>-<name>.png`: labeled contact sheets of the snapshots (needs Pillow; skipped without).

A scenario fails on a failed directive, a crash or non-zero exit, any sanitizer report (`ERROR: AddressSanitizer`,
`ERROR: LeakSanitizer`, `runtime error:`), an unknown sim command, a timeout (30 s, or 120 s for monkeys and the thirty-minute family-control expiry scenario), or a
UI finding of either severity: FAIL and WARN both fail the run (warnings are errors). `make ci` (what CI runs for this app)
includes `make playtest`, so an open FAIL finding fails CI.

`make coverage` reruns the non-monkey scenarios against a coverage-instrumented sim (`PLAYTEST_SNAP` points the
runner at it, `PLAYTEST_OUT` keeps its report in `build/coverage/playtest/` instead of overwriting yours).

## How it works

The sim is deterministic: a fixed clock (a Tuesday, 16:00), a seeded pet RNG and fixed 40 ms frames. The runner
turns a scenario into a sim script, runs `snap --fresh --script` in a temporary directory (so saves and snapshots
never touch your `build/host/`, and scenarios run in parallel) and reads stdout. Each query is wrapped in
`echo @@<id>` / `echo @@` markers so its output can be found.

Three directives need the live game state to decide what to tap (`follow-glow`, `solve-word`, `answer-book`).
For those the runner runs the script so far, reads the answer, appends the taps and carries on. Because every run
replays the same game, the final run sees exactly what the earlier ones saw. Scenarios whose name contains
`monkey` use `build/host/snap-asan`; the rest use `build/host/snap`.

## Scenario syntax

A scenario is a text file, one command per line; `#` starts a comment. Coordinates are logical pixels (160×160,
the glass is the circle of radius 80 around (80,80)); times are milliseconds of game time.

### Sim commands (host/sim.cpp)

| Command | Effect |
| --- | --- |
| `tap X Y` | finger down for 2 frames, up on the 3rd (120 ms) |
| `hold X Y` | finger down for 1 s (a long press fires at 600 ms), then up |
| `down X Y` / `move X Y` / `up` | one frame each; drags are `down`, several `move`s, `up` |
| `wait MS` | step frames for MS ms with the finger as it is |
| `skip SEC` | move the wall clock forward (one frame) |
| `reset` | wipe every save: a fresh device, on the "new profile" screen |
| `profile NAME AGE [PIN]` | create a profile (face = its slot) and select it: the launcher (or the rest screen) |
| `app NAME` | open a game from the launcher (`app pets-club`) |
| `newgame KID PET` | `reset`, then profile KID (age 8) with pup PET already adopted, opened in Pets Club (its splash) |
| `snap NAME` | write a snapshot (for `sheet`) |
| `dbg CMD` | test hook: `tired bedtime rested younger older` (the profile's rest budget and age; `tired` rests at the next second, `bedtime` uses up today's play so the next second shows "Back tomorrow"), then to the open game: Pets Club `dirty poop hungry hearts books tricks hats grown dog sleepy younger older butterfly` (`butterfly` sends one across the room from the left), Biscuit `hungry` (needs to their floor) `unlock` (7 days: every trick) `young` `grown` (the stages) `tricks` (all mastered) `practiced` (all on their last lesson) |
| `debug` / `screen` / `ui` | print the state / the screen name / one audited frame (used by the directives) |
| `watch MS` | step like `wait`, printing `glow X Y` whenever a trick-lesson glow appears |
| `monkey N SEED` | N random gestures, see below |
| `echo WORD` | print WORD (the runner's markers) |

### Runner directives

| Directive | Meaning |
| --- | --- |
| `expect KEY OP VALUE` | runs `debug` and compares the latest `KEY=value` it printed. OP is `= != < <= > >=` (`==` too). Numbers compare as numbers, anything else as text (`=`/`!=` only). `books` and `profile` compare the first number of `a/b` unless VALUE contains `/` (`expect profile=0/1`); `play` ignores its trailing `s`. |
| `expect-screen NAME` | the current screen is NAME; `a\|b` accepts either, `*` any real screen |
| `ui-check [TAG]` | audits one frame against the UI rules; TAG names the state in the report (default: the screen name; the real screen is appended when the tag does not start with it) |
| `snap NAME` + `sheet NAME` | `sheet` puts every snapshot since the previous `sheet` into `build/playtest/<scenario>-NAME.png` |
| `follow-glow` | `watch 6000`, then taps every reported glow with 250 ms gaps (one lesson round) |
| `solve-word` | reads `word`, `cols`, `tiles` from `debug` and taps the tiles that spell the word (call it at the start of a word) |
| `answer-book` | reads `correct=` from `debug` and taps that answer button |
| `repeat N` … `end` | unrolls the lines in between N times (no nesting) |

Keys printed by `debug`: the shell's `profile` (active id / count, -1 on the picker) `profiles muted play today rest tint
screen` (`today`: seconds played today, `rest`: seconds until the profile may play) (`tint`: `day`, `evening` or `night`, what the glass shows), then the open (or last opened) game's: `food fun energy clean bond hearts streak day stage asleep poop dirty gift age
books screen tricks hat stickers`, and on the matching screens `word cols typed tiles`, `book title pages page
correct`, `score left` (fetch), `spots` (bath), `trick phase round len seq input` (lesson). `screen` is the shell's
screen, or the game's while one is open.

### Timing guards worth knowing

Several screens ignore taps for a moment after they open, so a scenario has to wait before tapping: a trick
lesson 400 ms, fetch and Word Fetch results 600 ms, a celebration 700 ms, a gift 300 ms (700 ms once open), the
adoption parcel 900 ms after it opens. On top of those, every screen change (shell and game) ignores touches until
a press begins 450 ms after the new screen appeared (`ui::FreshGate`), so a tap right after a screen change needs
`wait 450` first, and a double tap acts once: `05_read`, `11_stats` and `15_shell_taps` check that.

## UI rules

1 logical px = 3 panel px = 0.33 mm on the 2.1-inch round panel. All rules work on the hit regions the game tests
(`UiAudit`, filled by `Input::hit`, `tapIn` and `tapInCircle`) and on the text boxes it draws (`gfx::textLog`).

| Rule | Severity | Check | Why |
| --- | --- | --- | --- |
| target size | FAIL | a region's width or height < 18 px | 6 mm: the hard floor for this device; smaller targets get missed |
| target size | WARN | width < 24 px or height < 22 px | 8 mm: the device's floor (constraint 1), about the minimum touch target in the Apple (44 pt) and Material (48 dp) guidelines, which are written for adults |
| overlap | FAIL | two regions share more than 4 px in both axes | one touch would mean two things; up to 4 px (1.3 mm) is the hit padding `button()` adds |
| overlap | WARN | as above, but one region contains the other | nested hotspots (the dog vs. objects on the floor) work by test order, worth knowing |
| round edge | FAIL | under 85% of a region lies inside radius 80, or its center is more than 70 px from (80,80) | the bezel hides the rim; a target has to be reachable and visibly whole |
| spacing | WARN | two regions that do not intersect are less than 2 px apart | 0.7 mm between separate targets invites mis-taps |
| clipped text | FAIL | a corner of a text box is more than 79 px from (80,80) | the bezel cuts the glyphs |
| contrast | FAIL / WARN | WCAG 2 contrast of the text color on the dominant color under the box, below 2.5 / below 4.0 | new readers need crisp letters; the palette comes from `os/palette.h` (day colors) |
| dead screen | WARN | nothing tappable, on any screen but splash, intro, celebrate and gift | a kid has no way forward or back |

Contrast is skipped when the sampled background equals the text color (the sampler cannot tell). A text drawn
with `textShadow` is reported with its shadow color noted; the rule still measures the letter color itself.
Text on the RGB565 surface (`os/font.h`) is logged per line as the logical box that covers it, with real colors: the
`ui` dump prints `text X Y W H rgb=RRGGBB bg=RRGGBB` instead of `color=I bg=J`, and contrast is computed from those.
Contact sheets keep 3x-upscaled indexed frames crisp and scale true 480 px frames down smoothly.

What the audit cannot see: hit tests that do not go through `Input` (the trick-lesson zones in
`trainHitZone`, rubbing in the bath, page turns in a story, dragging in fetch), text that overflows its button but
stays on screen, text drawn over other text or under a button, and the evening and night tints (`16_night` checks
the tint by name). The contact sheets are there for those; look at them.

## Chaos monkey

`monkey N SEED` performs N gestures at random points inside radius 78, with 40-500 ms pauses between them:

| Share | Gesture |
| --- | --- |
| 65% | tap (finger down 80 ms) |
| 10% | double tap: two taps on the same point, presses 120 ms apart |
| 10% | drag: down, 4-8 moves toward another random point, up |
| 10% | slow tap: finger down 30-200 ms |
| 5% | long press: 700 ms |

It prints `monkey <n> screen=<name>` every 100 gestures and `monkey done` at the end. Gestures come from the
sim's own LCG, so a seed replays the same session on every machine; a failing seed is a reproducible bug report.
After each run the monkey scenarios check that the game is on a real screen and that food, fun, energy and clean
are within 0..100, while ASan and UBSan watch every frame.

## Adding a scenario

1. Create `tests/playtests/NN_name.txt`. Files run in name order; put `monkey` in the name to run under the
   sanitizers with the longer timeout.
2. Start from a known state: `newgame Sam Biscuit` + `wait 2000` lands on Pets Club's home; `reset` on the "new
   profile" screen; `profile Sam 8` on the launcher, then `app pets-club` + `wait 2000` on the adoption intro.
   `dbg` hooks set up the rest (`dbg hungry`, `dbg dirty`, `skip 86400` for the next day).
3. Find coordinates with a probe: a script ending in `ui` prints every region of the current screen, and
   `snap` + `sheet` show what the kid sees.
4. Assert outcomes, not steps: `expect` the stat that should change, `expect-screen` where the kid should be.
   When the game is wrong, keep the expectation that describes the right behavior and let it fail.
5. Run it alone (`python3 tools/playtest.py NN`) until it does what you meant, then check its sheet.
