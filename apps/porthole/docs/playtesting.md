# Playtesting

Commands run from `apps/porthole/` (or `make -C apps/porthole <target>` from the repo root).

`make playtest` builds the headless simulator twice (plain, and with AddressSanitizer + UBSan) and runs every
scenario in `tests/playtests/`. The suite has three parts:

* **Playthroughs** (`01`-`13`, the shell's profiles in `10` and `13`): scripted sessions a kid would play, with assertions on the game state. They
  include the things kids do by accident: double taps, a finger that slides off a button, taps during animations.
* **Chaos monkeys** (`20`-`22`): 600 random gestures per seed from several starting points (home, the profile screens, the picker, Paw Street, the mini-games), under the sanitizers.
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
`ERROR: LeakSanitizer`, `runtime error:`), an unknown sim command, a timeout (30 s, or 120 s for monkeys), or a
hard (FAIL) UI finding. WARN findings are reported but never fail a run. `make ci` (what CI runs for this app)
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
| `dbg CMD` | test hook: `tired rested younger older` (the profile's rest budget and age; `tired` rests at the next second), then to the open game: `dirty poop hungry hearts books tricks hats grown dog sleepy younger older` |
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

Keys printed by `debug`: the shell's `profile` (active id / count, -1 on the picker) `profiles muted play rest screen`,
then the open (or last opened) game's: `food fun energy clean bond hearts streak day stage asleep poop dirty gift age
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

What the audit cannot see: hit tests that do not go through `Input` (the trick-lesson zones in
`trainHitZone`, rubbing in the bath, page turns in a story, dragging in fetch), text that overflows its button but
stays on screen, text drawn over other text or under a button, and the evening and night tints. The contact
sheets are there for those; look at them.

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
