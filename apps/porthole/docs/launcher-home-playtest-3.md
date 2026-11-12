# Launcher Home and Library: independent child-perspective playtest 3

Emulator roleplay by an AI playtester (simulated perspectives only). No real
child and no physical device were involved.

## Build and setup

- Revision: `86cacf1` (clean tree).
- Executable: `apps/porthole/build/host/snap`, sha256
  `64c340aeae1fc7f1ee996724df86572dc465c61be373019071267d621796d335`.
- Started `build/host/snap --serve --fresh` from `apps/porthole/` at the default
  clock (Tuesday 16:00). Both profiles were created through the screens
  (name keyboard, face, age, "skip" for the code): Leo (fox, 6), then Zoe
  (owl, 8). No `dbg`, `app`, `profile` or clock skips during first play.
- Driver: a throwaway socket wrapper (scratchpad, not committed) around the
  `--serve` protocol (`down`/`move`/`up`/`tick`/`frame`). Every input is logged
  in `launcher-home-playtest-3/input-trace.txt`, in this grammar: `tap X Y` =
  finger down 120 ms then up (+40 ms); `hold X Y MS` = down MS then up;
  `dtap` = two 80 ms presses 80 ms apart; `drag` = down, a move every 40 ms,
  up; `wait MS` = elapsed frames; `restart` = the process closed and started
  again without `--fresh` (same saves); `shot NAME` = the native 480x480 frame,
  saved as `launcher-home-playtest-3/NAME.png`. Coordinates are logical px.

### Source exposure (disclosure)

Two briefs loaded automatically into my context before play: the repo-root
`CLAUDE.md` and `apps/porthole/CLAUDE.md`. The second describes the Home /
Library design in detail (star mode, lavender sky and jiggle, the five header
stars, "Home full!", only the green check keeps a visit's stars, the home
button / rest / restart discard, newest star first, an empty Home showing only
"All games"). So my first-play **comprehension** judgments are not those of a
truly naive agent; I made each decision from the frame and the persona, but a
fresh agent without that context would be needed to certify first-time
comprehension. To learn the `--serve` protocol I read lines 50-110 of
`tools/webemu.py` (no game logic). I did not read game/shell/host source,
playtest scenarios, specs or earlier reports until play was over; afterwards
I read the header comment of `tests/playtests/b2_home_stars.txt` and
`docs/launcher-home-playtest-2.md`.

## Personas

- **A. Leo, 6**: cannot read; presses firmly for 1-3 s, mashes, double-taps,
  holds anything shiny and gold.
- **B. Zoe, 8**: confident reader, plays most days; Rally and Pets Club are her
  favorites; her taps are a bit off sometimes.

## Dispatch brief (verbatim)

> You are a fresh, independent child-perspective playtester for Porthole, a round 480x480 touchscreen with no buttons (emulated on the host). Follow apps/porthole/docs/playtesting.md and the `playtest` skill. Play SOURCE-FREE: don't read source (apps/porthole/games/, os/, shell/, host/), tests/playtests scripts, docs/superpowers/, or earlier playtest reports in apps/porthole/docs/ until you've finished playing. Avoid AGENTS.md/CLAUDE.md, and disclose anything that loaded automatically. Decide from the screen.
>
> ## Personas (two passes)
> A. Leo, 6, can't read, presses firmly for 1-3 s, mashes, double-taps, holds anything shiny and gold.
> B. Zoe, 8, a confident reader who plays most days. Rally and Pets Club are her favorites. Her taps are a bit off sometimes.
> Simulated perspectives only.
>
> ## Setup
> - `make -C apps/porthole snap`. Record `git rev-parse --short HEAD` and `sha256sum apps/porthole/build/host/snap`.
> - `apps/porthole/build/host/snap --serve --fresh` from apps/porthole/, at the default clock. Create the kid's profile through the screens (age 6-8, no code), then use ordinary touches only. No dbg, app shortcuts or clock skips.
>
> ## Scope (first play of the launcher)
> - Leo: play his way for several minutes, especially holding and mashing the gold/yellow things. Does he notice when the screen is in a different mode? Can he change his first screen's games without meaning to, and does it stay changed after he leaves (the house, a restart)? Can he always reach his games and get back?
> - Zoe: can she make Rally appear first on her first screen and take a game she doesn't play off it? Time-to-goal, where she hesitates, and whether she understands the stars at the top.
> - Can anything be lost? Is anything confusing or boring?
> Also the brief's imprecise-finger probes on the new controls.
>
> ## Output
> Write apps/porthole/docs/launcher-home-playtest-3.md, with screenshots and the input trace in apps/porthole/docs/launcher-home-playtest-3/ (no .py files; save scripts as .txt). Include the revision and hash, the personas, this brief, findings ranked with repro steps, the assessment (Zoe's time-to-goal) and gaps. Afterwards you may read apps/porthole/docs/launcher-home-playtest-2.md and say which findings recurred. Don't fix code or commit. Reply with ONLY (under 10 lines): a verdict (acceptable / needs fixes), the top findings, and the report path.

Brief revision suggestion (not applied): the brief cannot stop
`apps/porthole/CLAUDE.md` from auto-loading in a Claude Code agent whose cwd is
the repo, and that file now spells out the whole star-mode design. For a truly
source-free comprehension claim, start the agent with a cwd outside the repo
(or a harness setting that skips project memory) and pass absolute paths.

## Paths played (first play)

All screenshots below are in `launcher-home-playtest-3/`.

### Leo (trace lines 1-163)

1. Profile through the screens (`00`-`04`): name keyboard, fox face, age 6,
   skip the code. First Home (`05-first-home`): fox + "Leo" bar, a sun/moon
   button, five big pictures and a cream tile of tiny pictures.
2. Holds the dog 2 s (`06`): a yellow "Pets Club" caption over the header while
   the finger is down; on release Pets Club opens (`07`, `08`). Holds the
   orange house 1.5 s: back to Home (`09`). Hold-to-play works for him.
3. Holds the tiny-pictures tile 2 s (`10`, caption "All games"): the Library,
   page 1 (`11`), now with gold stars in tile corners, a big gold star button
   and a yellow arrow.
4. **Holds the big gold star 2 s** (`12`: caption "Pick games"; `13`): the sky
   turns lavender, the tiles jiggle, five gold stars appear at the top and a
   green check replaces the gold star. *Leo notices*: the whole background
   colour changed and things wiggle. He does not know what it means.
5. Holds the header stars (nothing, `14`), then holds the gold corner star on
   the dog 2 s: no game opens, the star goes white and one header star goes
   white (`15`). He mashes the other gold stars with 1-2 s presses until every
   star is white (`16`, `17`).
6. **Presses the big green check** (most attractive thing left; `18`): back to
   the normal Library, no stars anywhere. Home (`19-home-after-zero`) is now
   only the small "All games" tile in an empty sky.
7. Leaves the house (taps his name bar, "Who's playing?", `20`), comes back
   (`21`): still empty. **Process restart** (`22`, `23`): still empty. He can
   still reach every game through the tiny-pictures tile (`24`).
8. Tries to put games back: star button, stars dog and Biscuit (`25`), then
   presses the orange house: the Library with no stars (`26`) -- his two stars
   were thrown away silently, and the house did not take him Home.
9. Arrow to page 2 (`27`), holds the gold crown game (Kingdom, `28`, `29`),
   "Exit" brings him back to page 2 (`30`). Drags from the star button onto a
   tile: nothing happens (`31`). Double-taps the star button: star mode once
   (`32`); double-taps Kingdom's tile in star mode: starred once (`33`). Arrow
   back (`34`), stars the dog and the pumpkin, presses the check (`36`), and
   Home shows pumpkin, dog, Kingdom (`37-home-leo`).

**Leo's first voluntary stopping point**: after step 2 he had what he wants
(press the dog, play). The launcher itself is not a game to him; he would
leave star mode within a few presses once tiles stop opening games. Steps 4-9
were driven by his attraction to the gold/green items plus coverage.

### Zoe (trace lines 164-221, the timed run)

1. New profile through the screens (`40`, `41`), owl, 8. Her Home (`42`) is the
   default five + All games (Leo's changes did not touch it).
2. Rally is not on Home. She spots the red car in the tiny-pictures tile and
   taps it off-centre (117,99): Library page 1 (`43`). Reads "All games",
   follows the arrow (tapped off-centre at (90,140)): page 2 with the car (`44`).
3. **Hesitation 1**: phone habit, she long-presses the car (0.9 s) expecting an
   edit menu (`45`, caption "Rally"); on release Rally opens (`46`). She taps
   Rally's "Home" and lands back on Library page 2 (`47`).
4. Taps the gold star button off-centre (121,47): star mode (`48`), every tile
   with a white star, five gold stars at the top.
5. Taps the car's star: **"Home full! / Stars: page 1"** over the header for
   about 2 s (`49`, `49b`, `49c`). **Hesitation 2**: reads it, connects "five
   gold stars at the top = Home is full", goes back with the arrow (`50`).
6. Presses the pumpkin ("Spooky" caption under her finger, `51`), it goes white
   and one top star goes white (`52`). Arrow to page 2, taps the car's star:
   gold, all five top stars gold (`53`). Taps the green check (`54`).
7. Taps the house: **Home shows Rally first**, the pumpkin gone, Pets Club
   second (`55-zoe-home-goal`).

**Zoe's time-to-goal**: 12 touches and 30.1 s of simulated time from first
seeing Home to the finished Home (my waits stand in for reading/thinking and
are estimates; a real 8-year-old would plausibly take 30-60 s). About 4 s went
to the accidental Rally launch and about 3 s to the "Home full!" refusal.
**The top stars**: she understood them only once "Home full!" appeared next to
five gold stars; after that the gold/white count was clear to her and she
used it to confirm (`52`, `53`). Before the refusal they read as decoration.
**Stopping point**: she would tap Rally on her new Home and play; she would not
keep curating.

## Supplementary QA (after first play)

- Off-centre taps in star mode (`64`): just left of the check (112,42), just
  below it (126,56), right of it (140,42), the 4 px gap between tiles (58,72)
  and the 2 px gap between rows (80,88) all do nothing. Header taps in star mode
  do nothing (`61`).
- Restart in star mode with an unsaved change (`60`-`63`): Home unchanged.
- 26 activity taps over ~17 minutes in star mode (`65`, `66`): no rest screen
  appeared and star mode never timed out (the launcher seems not to count toward
  the session budget); the house then discarded the change (`67`, `68`).
- Tap star then tap the same spot 600 ms later (`69`): in and straight out of
  star mode, nothing saved. A tap within ~450 ms of leaving star mode is ignored
  (`70`: my next "house" tap was dropped and a hold entered star mode again).
- Same corner, different meaning: the sun on Home opens "How bright?" (`72`),
  the star on the Library opens star mode.
- Geometry (`geometry-probe.txt`, `ui`): tiles 40x30, 4 px apart in a row and
  2 px between rows; star/check button 24x24; arrow 24x22. "Home full!" texts
  at (35,33,62,8) and (33,43,67,8) sit inside the glass.
- Random Leo-style mashing (`leo-mash-seed1..8.txt`: 40 random 0.3-2.5 s presses
  from the Library, not state-aware): in 8 seeds Home never changed (the presses
  mostly wander into games); seed 7 ended in star mode. Accidental commits need
  the deliberate attraction to the gold star and green check seen above.

### Gesture and interruption matrix

| Control | Quick tap | Off-centre | Hold 1-3 s | Double tap | Drag off | Mash |
| --- | --- | --- | --- | --- | --- | --- |
| Home game tile | opens (Zoe 47 via Library) | opens | caption, opens on release (06-07) | gap on Home | gap on Home | gap on Home |
| All games tile | opens (43) | opens (117,99) | opens (10-11) | gap | gap | gap |
| Library star button | star mode (48) | works (121,47) | "Pick games" then star mode (12-13) | enters once (32) | cancels (31) | random mash: seed 7 |
| Star-mode tile | toggles (52) | toggles from picture body (51) | toggles once (15) | acts once (33) | gap | every press toggles (17) |
| Green check | saves (54) | edges outside do nothing (64) | saves (18, 36) | gap | gap | - |
| House in star mode | discards, Library (26, 67) | - | discards (26) | - | - | - |
| Arrow | pages (44) | works (90,140), (70,140) | pages (27) | gap | gap | - |

| Interruption | Result |
| --- | --- |
| Leave to picker and back (committed) | kept (21) |
| Process restart (committed) | kept (23, 63) |
| Process restart in star mode (uncommitted) | discarded (63) |
| Game opened from Library, exit | back to that Library page (30, 47) |
| Rest screen during star mode | **gap**: not reached after 17 min of activity in the launcher |
| New-game red dot | **gap**: needs a firmware with a new game (or `dbg unseen`) |
| Arrow/star "hop" animation | **gap**: only stills inspected, motion not judged |

## Findings (prioritised)

No Blocker. No Major on this build in my judgment; finding 1 is borderline and
is the coordinator's call.

### 1. Minor (borderline Major): a non-reader can empty his Home and it stays empty

- **Screen**: library (star mode), then launcher.
- **Issue**: Holding the gold star, pressing every gold star (they turn white)
  and then the big green check leaves Home as one small "All games" tile in an
  empty sky; it survives leaving the house and a restart, with no warning,
  confirmation or undo.
- **Concern**: recovery / comprehension / feedback.
- **Repro**: fresh device, Leo 6; Home -> hold (124,105); hold star (126,42)
  2 s; hold (50,64), (94,64), (138,64), (50,98), (94,98) 1-2 s each; hold the
  check (126,42) 1.5 s; house; restart. Trace lines 40-89.
- **Screenshot**: `docs/launcher-home-playtest-3/17-mash-all-unstarred.png`,
  `18-check-with-zero.png`, `19-home-after-zero.png`, `23-home-after-restart.png`.
- **Impact**: nothing is lost (every game is one extra press away through the
  tiny-pictures tile, saves untouched), but a 6-year-old who did this by
  attraction would likely live with the empty Home until an adult notices; his
  restore attempt in finding 2 failed. Suggestions (owner's choice): refuse or
  ignore a commit that leaves zero stars (or show the default), or show the
  check only once something changed and make it the thing that hops.

### 2. Minor: the house in star mode silently throws away the visit's stars and does not go Home

- **Screen**: library (star mode).
- **Issue**: Leo starred two games, pressed the house to "go home", and landed
  on the normal Library with his stars gone; nothing showed they were dropped,
  and a second house press is needed to reach Home.
- **Concern**: feedback / consistency / recovery.
- **Repro**: trace lines 90-102: from the Library, star (126,42); hold (50,64),
  (94,64); hold the house (80,13).
- **Screenshot**: `docs/launcher-home-playtest-3/25-starred-two.png`,
  `26-after-home-btn-in-starmode.png`.
- Note: discarding is the safer default for accidental entry (it protects
  Leo); the problem is only that the loss is silent when changes exist.

### 3. Minor: nothing on Home says Home can be changed; the phone-style long press opens the game

- **Screen**: launcher, library.
- **Issue**: Zoe's first attempt to move Rally (0.9 s press on its tile) launched
  Rally; the only way to edit is the Library's star button, whose "Pick games"
  hint shows only under a finger.
- **Concern**: comprehension.
- **Repro**: trace lines 164-183 (hold (82,104) 900 ms on Library page 2).
- **Screenshot**: `docs/launcher-home-playtest-3/45-zoe-hold-car.png`,
  `46-zoe-rally-opened.png`.

### 4. Nit: the top stars only make sense after a refusal

- **Screen**: library (star mode).
- **Issue**: five gold stars with no label read as decoration until "Home full!"
  appears; "Stars: page 1" is understandable to a reader but terse ("Gold stars
  on page 1"); Leo never connects them to Home.
- **Concern**: comprehension.
- **Repro**: trace lines 184-195 (star mode on page 2, tap (95,97)).
- **Screenshot**: `docs/launcher-home-playtest-3/48-zoe-starmode-p2.png`,
  `49-zoe-full-a.png`.

### 5. Nit: an empty Home looks broken

- **Screen**: launcher.
- **Issue**: with no favorites a single small tile floats under the header with
  a large empty sky beneath (finding 1's end state).
- **Concern**: consistency / comprehension.
- **Repro**: as finding 1.
- **Screenshot**: `docs/launcher-home-playtest-3/19-home-after-zero.png`.

### 6. Nit: the pumpkin game's caption is "Spooky"

- **Screen**: launcher / library caption.
- **Issue**: the caption under a finger says "Spooky", not the game's title.
- **Concern**: consistency.
- **Repro**: press (80,104) on Library page 1.
- **Screenshot**: `docs/launcher-home-playtest-3/51-zoe-press-pumpkin.png`.

### 7. Nit: the same gold corner means brightness on Home and star mode on the Library

- **Screen**: launcher / library.
- **Issue**: Leo's habit of holding the gold thing at top right gives "How
  bright?" on Home and star mode on the Library.
- **Concern**: consistency.
- **Repro**: Home, hold (126,42) 2 s; Library, hold (126,42) 2 s.
- **Screenshot**: `docs/launcher-home-playtest-3/72-sun-held.png`,
  `13-after-star-hold.png`.

No layout defects: every tile, caption, header star, check and arrow sits
inside the circle; "Home full!" fits its yellow bar; nothing overlaps.

## Assessment

**Can the persona recognise the goal and actions and see what changed?**
Zoe: yes. Pictures find games (the red car inside the tiny-pictures tile), the
caption under her finger names them, and star mode is unmistakable (lavender,
jiggle, every tile with a star). Each press shows its result at once (gold or
white corner star, a top star changing). She needed the "Home full!" refusal to
understand the top stars; it then did its job. Leo: he clearly notices the mode
change (the sky changes colour and the pictures wiggle, `13`), an improvement
he could not miss, but he cannot tell what it means; to him it is "the games
stopped opening and the stars turn white".

**Decisions, feedback, pictures vs reading?** Picking "my five" is a real,
personal choice for Zoe, with immediate visual results and a gentle refusal
(no sound, a yellow note). Reading carries only the refusal text; pictures and
colours carry the rest. For Leo it is not a decision but a toy.

**Repetition, confusion, frustration; where would they stop?** Short task, no
boredom: Zoe is done in about 30-60 s and goes to play Rally. Her two
hesitations cost a few seconds each. Leo's frustration point is star mode: after
two or three presses that do not open the dog he presses the most attractive
remaining thing (green check or house). Via the house he is safe; via the check
he can lock in whatever he mashed (finding 1).

**Recovery without losing progress or adult help?** Nothing a kid owns is lost:
saves are untouched, every game stays in All games, Home edits are per kid
(Leo's did not reach Zoe), and a restart or the house in star mode drops
uncommitted edits. Every screen has the orange house, and games return to the
page they were opened from. The weak spot is putting an emptied Home back
without reading: possible (Leo did it, `37`), but only after learning that the
check, not the house, keeps stars (finding 2).

## Automated suite

`make -C apps/porthole playtest` at `86cacf1`: **93 of 93 scenarios pass, 0 UI
FAIL, 0 UI WARN** (report read in full: `build/playtest/report.md`; it includes
`a2_launcher_pages`, `b2_home_stars`, `b3_admin_games` and the four monkeys).
No new scenario added: `b2_home_stars` already covers star mode, the refusal,
double taps, discard paths and the "none on Home" end state; finding 1 is a
design decision rather than a regression to pin down.

## Comparison with playtest 2 (read after play)

- P2 #1 (Major, a non-reader enters star mode unnoticed and every press edits
  Home): **largely fixed**. The mode is now obvious (lavender + jiggle), and the
  house now discards instead of keeping the mashed stars. It **recurs in reduced
  form** as finding 1 here: the green check still commits whatever was mashed,
  down to an empty Home.
- P2 #2 ("Tap a star" on a page with only white stars): **fixed** ("Stars: page 1").
- P2 #3 (the "Pick 5·N" counter): **fixed**, replaced by five header stars
  (which have their own small comprehension gap, finding 4).
- P2 #4 (a double tap toggles twice): **fixed** (acts once, `33`).
- P2 #5 (nothing on Home hints at editing): **recurs** (finding 3).
- P2 #6 (the house in star mode stays in the Library): **recurs**, now also
  silently discarding (finding 2).
- P2 #7 ("Spooky" caption): **recurs** (finding 6).
- P2 #8 (name bar has no press feedback): not re-checked (I held it 1.5 s
  without a mid-hold frame); gap.

## Gaps and limitations

- Emulator roleplay only; no real children and no physical panel (finger size,
  bezel, glare, real press timing all untested).
- First-time comprehension is not certified: the design was in auto-loaded
  project memory (see disclosure).
- Rest screen during star mode, the new-game red dot, the hop animations in
  motion, mash and double tap on Home tiles, Admin's games-off interacting with
  stars: not tested.
- Reading/thinking pauses in Zoe's time-to-goal are my estimates.

## Verdict

**Acceptable, with fixes recommended** for findings 1-2 (the empty-Home commit
and the silent discard). Zoe reaches her goal quickly and enjoys the result;
Leo can always reach his games and get back, and the mode change is now visible
to him. If the coordinator rates finding 1 Major, this needs another pass after
the fix.
