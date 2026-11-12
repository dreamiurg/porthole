# Launcher Home and Library: independent child-perspective playtest 2

Emulator roleplay by an independent `playtester` agent (simulated children, not
a study with real children, not a physical device).

## Build and setup

- Revision: `f86ee45` (clean tree except this report).
- Executable: `apps/porthole/build/host/snap`, sha256
  `476340e198bdb191edf503b7b9fbbec74b13af113bac13a63c1e27b0b6e3f71e`
  (`make -C apps/porthole snap`: up to date at this revision).
- Run: `apps/porthole/build/host/snap --serve --fresh` from `apps/porthole/`,
  default clock (Tuesday 16:00, day tint). A small stdin driver
  (`launcher-home-playtest-2/serve-driver.txt`, a Python script saved as text)
  sends only the `--serve` protocol's ordinary inputs (`down`/`move`/`up`,
  `tick MS`) and `frame` to save native 480x480 PNGs. No `dbg`, `app`,
  `profile`, `screen`, `debug` or clock skips in the first-play traces.
- Profiles created through the screens: Zoe (fox, age 8, code skipped), Leo
  (frog, age 6, code skipped). Every trace starts from `--fresh`.
- Fresh-process replays of both traces reproduced the saved final frames
  byte-for-byte (`zoe/z24_home_with_rally.png`, `leo/l21_home_after_star_mash.png`).
- Process restart: `zoe-restart-trace.txt` run without `--fresh` after the Zoe trace.

### Source exposure (disclosure)

I did not read game, shell, os or host source, playtest scenarios, specs or
earlier reports before playing. Two briefs loaded automatically at start: the
repo-root `CLAUDE.md` and `apps/porthole/CLAUDE.md`. The latter describes the
Home/Library design in detail (favorites, "All games" tile, star mode, "Pick
games", "Home full! / Tap a star", the counter, newest star first). So this
agent was **not naive** about how star mode works; comprehension judgments
below are what the screen shows a child, but a fresh agent without that brief
would be a stronger first-comprehension check. I also read
`tools/webemu.py`'s protocol lines to drive `--serve`. After play I used `ui`
geometry dumps and `debug` once (supplementary only), and after writing the
findings I read `launcher-home-playtest.md` (see the end).

## Personas

- **A. Zoe, 8**: confident reader, plays most days; Rally and Pets Club are her
  favorites. Taps a little off-centre at times and sometimes holds (0.5-0.7 s).
- **B. Leo, 6**: cannot read; presses firmly for 1-3 s, mashes and double-taps.

## Dispatch brief (verbatim)

> You are a fresh, independent child-perspective playtester for Porthole, a round 480x480 touchscreen with no buttons (emulated on the host). Follow apps/porthole/docs/playtesting.md and the `playtest` skill. Play SOURCE-FREE: don't read source (apps/porthole/games/, os/, shell/, host/), tests/playtests scripts, docs/superpowers/, or any earlier playtest reports in apps/porthole/docs/ until you've finished playing. Avoid reading AGENTS.md/CLAUDE.md files, and disclose anything that loaded automatically. Decide from the screen.
>
> ## Personas (two passes)
> A. Zoe, 8, a confident reader who plays most days. Rally and Pets Club are her favorites. Her taps are a bit off sometimes, and she sometimes holds them.
> B. Leo, 6, can't read, presses firmly for 1-3 s, mashes and double-taps.
> Simulated perspectives only.
>
> ## Setup
> - `make -C apps/porthole snap`. Record `git rev-parse --short HEAD` and `sha256sum apps/porthole/build/host/snap`.
> - `apps/porthole/build/host/snap --serve --fresh` from apps/porthole/, at the default clock. Create the kid's profile through the screens (age 6-8, no code), then use ordinary touches only. No dbg, app shortcuts or clock skips.
>
> ## Scope (first play of the launcher)
> - Zoe: what does she understand on the first screen after picking her profile? Can she reach Rally, and can she make Rally appear first, right where she starts? Can she take a game she doesn't play off her first screen? How long does it take, and where does she hesitate?
> - Leo: can he always reach his games and get back? Does he ever get stuck in a mode where games don't open, or change things without meaning to? Does anything look broken?
> - Can anything be lost? Is anything confusing or boring?
> Also the brief's imprecise-finger probes on the new controls.
>
> ## Output
> Write apps/porthole/docs/launcher-home-playtest-2.md, with screenshots and the input trace in apps/porthole/docs/launcher-home-playtest-2/ (no .py files; save scripts as .txt). Include the revision and hash, the personas, this brief, findings ranked with repro steps, the assessment (with time-to-goal for Zoe) and gaps. After you finish, you may read apps/porthole/docs/launcher-home-playtest.md and say which of its findings you saw again. Don't fix code or commit. Reply with ONLY (under 10 lines): a verdict (acceptable / needs fixes), the top findings, and the report path.

No brief revisions in this pass.

## Paths played (first play)

Coordinates are logical px (0-160). Full traces: `launcher-home-playtest-2/zoe-trace.txt`, `leo-trace.txt`.
Screenshots: `launcher-home-playtest-2/zoe/`, `leo/`, `supplementary/`.

### Zoe

1. Name keyboard (A-M / N-Z pages) -> "Zoe" -> OK -> fox -> 8 -> skip (z01-z05).
2. **First Home** (`z06_first_home.png`): her fox and name in a bar, a sun/moon
   button, five big game pictures (two dogs, a ball tray, a footballer, a
   pumpkin) and a cream tile of six tiny pictures. No names under tiles. No car.
   Zoe reads "my games", recognises Pets Club's dog. She spots a tiny red car in
   the cream tile and guesses "that's where the other games are".
3. Presses the cream tile for 0.6 s, a little off-centre: a yellow caption
   "All games" appears over her name while pressing (`z07`), then the Library
   (`z08`): "All games", a big gold-star button, six games with small gold stars
   on five of them (the ones from her Home), Radar without, and an arrow.
4. No car: taps the arrow (off-centre) -> page 2 (`z09`): Star Fighter, Kingdom,
   Toy Town, **Rally**. Taps Rally -> Rally's track picker (`z10`). Rally's own
   Home button returns her to Library page 2, not to the start (`z11`): good.
5. "How do I make it be on my first screen?" Nothing on Home or the Library says
   so. Curious about the big gold star, she holds it ~0.7 s: caption
   "Pick games" (`z12`); on release, star mode (`z13`): the star became a green
   check, every tile got a small white star, and the title reads **"Pick 5·0"**.
   Hesitation: she reads "Pick 5 point 0"? She guesses "pick 5 games".
6. Taps Rally's white star: **"Home full! / Tap a star"** in yellow over the
   title, the arrow hops (`z14`). On this page every star is white, so "a star"
   is unclear. She taps Rally's star again, then the Rally picture (`z17`,
   `z18`): the same message. Then she notices the hopping arrow and follows it
   (`z19`): page 1 with gold stars hopping. Now "Tap a star" makes sense.
7. Taps the ball-tray game she never plays (tile, not the exact star): its star
   turns white, counter "Pick 5·1" (`z21`). Arrow -> page 2 -> taps Rally: gold
   star, "Pick 5·0" (`z22`). Green check -> plain Library (`z23`). Orange house ->
   **Home with Rally first** (`z24`). Goal met.
8. Takes the pumpkin off as well: All games -> quick tap on the star (straight
   into star mode, no caption, `z25`) -> taps pumpkin (`z26`). Presses the
   orange house expecting Home: it only leaves star mode and stays in the
   Library (`z27`); a second press reaches Home: four games + All games,
   centred second row (`z28`).
9. Orange house on Home -> "Who's playing?" (`z29`). Process restart -> tap Zoe
   -> the same Home (`zr02`).

**Time to goal (Rally first on Home):** 13 taps after first seeing Home
(All games, arrow, Rally, Rally's Home, star, 3 refused/uncertain taps, arrow,
unstar, arrow, star Rally, check, house), 19 s of input/animation time in the
sim. With an 8-year-old's looking and reading pauses, my estimate is
**60-90 s**, about 20-30 s of it spent at "Home full! / Tap a star" on a page
with no gold star, and a few seconds at "Pick 5·0". Removing a game she does
not play: 4 taps once she knows star mode (All games, star, tile, house x2).

### Leo

1. Types L-E-O with 0.8-1.5 s presses on every key, frog, 6, skip: every slow
   press worked (`l01`-`l04`).
2. Home: presses the pumpkin 2 s (caption "Spooky" while held, `l05`); the
   pumpkin game opens (`l06`). Its framed back-arrow picture brings him back to
   Home (`l07`).
3. Double-taps the cream tile: Library once, no stray second tap (`l08`).
4. Holds the big gold star 2.5 s ("Pick games" caption he cannot read, `l09`):
   **star mode** (`l10`). The only visible changes: the gold star became a green
   check, Radar got a white star, the title text changed.
5. Wants the dog: presses Pets Club 2 s -> it does **not** open; its star turns
   white (`l11`). Presses again 1.5 s -> star back to gold (`l12`; this also
   moves Pets Club to the front of his Home). Presses pumpkin (unstarred) and
   Radar (starred) (`l14`). Then mashes seven tiles 150 ms apart: every tap
   toggles a star (`l15`); after it calms, Home holds only Pets Club and the
   pumpkin (`l16`, "Pick 5·3").
6. Presses the orange house 1.5 s: star mode ends, plain Library (`l17`).
   Presses the dog 1.8 s: Pets Club opens (`l18`). Its house returns to the
   Library page 1 (`l20`); house again -> Home with **two games and All games**
   (`l21`). He never meant to change anything; Biscuit, the ball tray and the
   footballer are gone from his first screen (still in All games).
7. Holds his frog name bar 2 s: no visible press feedback, then "Who's playing?"
   on release (`l22`, `l23`). Holds his own row 2.5 s: back to his Home (`l25`,
   no delete prompt before 3 s).
8. Holds the sun 2 s: "How bright?" (`l26`); house back (`l27`).
9. Drag-off: presses the dog and slides onto the grass before lifting: nothing
   opens, caption gone (`l28`, `l29`). Double-taps the pumpkin: opens once, the
   second tap does not press START (`l30`).
10. Library: double-tap the star button -> star mode once (`l32`); double-tap
    Biscuit in star mode -> no visible change (`l33`, see finding 4); double-tap
    the arrow -> one page (`l34`); hold the arrow 2 s -> one page back (`l35`);
    double-tap the green check -> plain Library, not back into star mode (`l36`).

## Findings (prioritised)

### 1. Major: a non-reader can enter star mode by accident, then every press edits Home instead of opening a game

- **Screen**: library (star mode), then launcher.
- **Issue**: Holding or tapping the big gold star (the most eye-catching control
  on the Library) switches to a mode where tile presses toggle stars. For a
  6-year-old the mode looks almost the same as before (same tiles, same layout;
  only small corner stars, a check instead of a star, and text). Leo pressed
  "his dog" twice, then mashed; nothing opened, and he silently rearranged and
  shrank his Home from 5 games to 2 (and moved the dog to the front).
  He recovered by pressing the orange house, which he would try after several
  failed presses; an impatient 6-year-old might instead decide the games are
  broken and give up or call an adult. Nothing is truly lost (all games stay in
  All games), but his first screen changed without his meaning to, and stays
  changed after a restart.
- **Concern**: comprehension / recovery / feedback.
- **Repro**: `launcher-home-playtest-2/leo-trace.txt`, lines from
  "double tap on the many-pictures tile" through "presses the orange house"
  (fresh device; profile Leo 6; All games; hold star 2.5 s at (127,42);
  press Pets Club (36,72) 2 s twice; tap (80,104), (124,106); mash the six tiles).
- **Screenshot**: `launcher-home-playtest-2/leo/l10_after_star_hold.png`,
  `l11_dog_press1.png`, `l15_after_mash.png`, `l21_home_after_star_mash.png`.
- **Suggestions for the owner** (not prescriptive): make the mode impossible to
  miss without reading (tiles wiggle or tint, a different background, the
  green check pulsing); or enter it only through a deliberate hold with a
  filling bar like the picker's delete hold; or let only the corner star
  toggle while a press on the picture shows the hopping check instead of
  editing; or drop out of star mode after a few seconds without taps.

### 2. Minor: "Home full! / Tap a star" shown on a page where every star is white

- **Screen**: library page 2 (star mode).
- **Issue**: Starring Rally when Home is full says "Tap a star", but the stars
  to tap (gold) are on the other page; the only hint is the arrow's hop. Zoe
  tapped Rally's white star and the Rally picture again (same message) before
  following the arrow. Leo cannot read it at all.
- **Concern**: comprehension.
- **Repro**: `zoe-trace.txt` from "Curious about the big star button" to "follows the arrow back".
- **Screenshot**: `launcher-home-playtest-2/zoe/z14_rally_star_tap.png`, `z18_tile_tap.png`, `z19_lib_p1_starmode.png`.

### 3. Minor: the counter "Pick 5·N" reads like a decimal and has no picture form

- **Screen**: library (star mode).
- **Issue**: "Pick 5·0" / "Pick 5·1" (`zoe/crop_pick.png`) reads as "5.0" /
  "5.1" to an 8-year-old; it does not say "places left". A non-reader gets
  nothing. Five little slot icons (filled / empty) would serve both.
- **Concern**: comprehension.
- **Repro**: `zoe-trace.txt`, the star-button hold and the unstar of the ball-tray game.
- **Screenshot**: `launcher-home-playtest-2/zoe/z13_star_mode.png`, `z21_unstar_marble.png`, `crop_pick.png`.

### 4. Minor: a double tap on a tile in star mode toggles twice (no visible change)

- **Screen**: library (star mode).
- **Issue**: Two taps 80 ms apart star then unstar the game; the star flickers
  and ends where it started. Everywhere a screen changes, a double tap acts once;
  here it does not, so Leo's double tap "does nothing". Rapid mashing (150 ms
  apart) also acts on every tap, which is how finding 1 strips Home so fast.
- **Concern**: feedback / consistency.
- **Repro**: `leo-double-tap-probe-trace.txt` (the last block: star mode, `tap 80 74`, 80 ms, `tap 80 74`).
- **Screenshot**: `launcher-home-playtest-2/supplementary/starmode_double_tap_strip.png` (after first tap: gold; after second: white; settled: white).

### 5. Minor: nothing on Home hints that Home can be changed

- **Screen**: launcher, library.
- **Issue**: Zoe only found star mode by curiosity; the hint "Pick games" shows
  only while the star is held, and a quick tap goes straight into the mode with
  the cryptic counter (`z25`). A confident 8-year-old manages; it was not
  obvious. (Kept separate from 1-3 because it is about finding the feature, not
  understanding it.)
- **Concern**: comprehension.
- **Repro**: `zoe-trace.txt`, first Home through the star-button hold.
- **Screenshot**: `launcher-home-playtest-2/zoe/z06_first_home.png`, `z08_after_allgames.png`, `z12_press_star.png`.

### 6. Nit: the orange house in star mode only leaves star mode

- **Screen**: library (star mode).
- **Issue**: Zoe pressed the house expecting Home and stayed in the Library
  (now plain); a second press went Home. Safe, and it is what got Leo out of
  star mode, but it is one extra press against expectation.
- **Concern**: consistency.
- **Repro**: `zoe-trace.txt`, "leaves with the orange home button instead of the check".
- **Screenshot**: `launcher-home-playtest-2/zoe/z27_orange_from_starmode.png`.

### 7. Nit: the pumpkin game's caption says "Spooky"

- **Screen**: launcher / library caption.
- **Issue**: Its tile caption is "Spooky" while the game's title screen says
  "PUMPKIN HIDE & SEEK"; every other caption matches its game's name.
- **Concern**: consistency.
- **Repro**: hold the pumpkin tile (80,104) on Home.
- **Screenshot**: `launcher-home-playtest-2/leo/l05_holding_pumpkin.png`, `supplementary/captions.png`.

### 8. Nit: the name bar on Home gives no press feedback

- **Screen**: launcher.
- **Issue**: Holding the name bar 2 s shows nothing (tiles show a caption, the
  star shows "Pick games"); on release the kid is on "Who's playing?". Easy to
  recover (tap own row), but a held press has no visible response.
- **Concern**: feedback.
- **Repro**: `leo-trace.txt`, "holds his frog name bar 2 s".
- **Screenshot**: `launcher-home-playtest-2/leo/l22_hold_header.png`, `l23_after_header.png`.

No layout defects seen: all tiles, captions and buttons sit inside the circle,
nothing overlaps or clips, the cream tile's tiny pictures are legible enough to
spot a car, the centred short rows look intentional (`z28`, `l21`).

## Assessment

**Can the persona recognise the goal and actions and see what changed?**
Zoe: yes for playing. Her first Home is clear (her face and name, big
pictures); the cream tile of tiny pictures read as "more games" at once, and
the caption on press confirmed it ("All games"). Pictures carry finding a game:
she found Rally by its car in two taps. Changing Home is harder: no hint on
Home, a cryptic counter, and "Tap a star" on the wrong page (findings 2, 3, 5).
Once she saw the gold stars hop on page 1 she understood "gold = on my first
screen" and the rest was quick; seeing Rally first on Home was a clear payoff.
Leo: yes for playing (every press length opens a game; captions are unread but
harmless), no for star mode, which looks like the normal Library to him
(finding 1).

**Interesting decisions, consequences, feedback?** Choosing which five games
are "mine" is a meaningful, personal choice for Zoe; each star tap gives an
immediate change (gold/white, counter) and the refusal is gentle (no sound, a
hop). Pictures, not reading, carry play; reading carries the editing.

**Repetition, confusion, frustration; where would they stop?** Zoe's natural
stopping point comes right after `z24`: Rally is first, she taps it and plays.
She would not keep curating. The only frustration was the 3-tap loop on
"Home full!" before finding the arrow. Leo would stop exploring star mode after
two or three presses that do not open the dog; the risk is that he decides the
dog is broken rather than pressing the house. Nothing here is boring; it is a
short task.

**Recovery without losing progress or adult help?** Every screen has the orange
house; game exits return to where the kid came from (Library page for Rally and
Pets Club). Unstarred games are never lost (All games always lists them), stars
survive leaving to the picker and a process restart (`zr02`). The weak spot is
the silent Home edit in finding 1: recoverable, but a 6-year-old would need to
learn star mode, or an adult, to put his games back.

## Gesture and interruption matrix

| Control | Quick tap | Off-centre | Hold 0.6-0.7 s | Hold 1-3 s | Double tap | Drag off | Mash |
| --- | --- | --- | --- | --- | --- | --- | --- |
| Home game tile | opens (z10 via Library) | opens (z08) | opens, caption while held (z07) | opens (l06) | opens once, no stray tap (l30) | cancels (l29) | not tested on Home |
| All games tile | opens (z25) | opens | opens (z08) | opens (l31) | opens once (l08) | gap | gap |
| Library star button | star mode (z25) | works | "Pick games" caption, then star mode (z12-z13) | same (l10) | enters once (l32) | gap | gap |
| Star-mode tile | toggles (z21) | toggles from picture body (z21) | gap | toggles (l11) | toggles twice, net none (finding 4) | tile-to-tile drag: nothing (supplementary/carry_and_drag.png) | every tap toggles (l15) |
| Arrow | pages (z09) | pages (z09, 93,138) | gap | pages once (l35) | pages once (l34) | gap | gap |
| Green check | ends star mode (z23) | works | gap | gap | ends once, does not re-enter (l36) | gap | gap |
| Orange house | Library -> Home, game -> origin, star mode -> Library | works | gap | works (l17, l20) | gap | gap | gap |
| Name bar | -> picker (z29) | gap | gap | -> picker, no feedback (l23) | gap | gap | gap |
| Early press inside 450 ms of a screen change | ignored (supplementary/carry.txt) | | | | | | |

| Interruption | Result |
| --- | --- |
| Open a game from Home, leave | back to Home (l07) |
| Open a game from Library page 2, leave | back to Library page 2 (z11) |
| Leave to the picker, pick self again | same Home (l25) |
| Process restart (no `--fresh`) | stars and order kept (zr02) |
| Process restart while still in star mode (after Leo's mash) | every toggle kept: Home shows his accidental 2-game Home (`leo/lr01_home_after_restart_mid_starmode.png`; `leo-midstar-cut-trace.txt` then `leo-midstar-restart-trace.txt` without `--fresh`) |
| Rest screen / daily cap while in star mode | **gap** (needs a clock skip, out of scope for first play) |

## Supplementary QA (after first play)

- `make -C apps/porthole playtest` at `f86ee45`: **93 of 93 scenarios pass,
  0 UI FAIL, 0 UI WARN**; report read in full (`apps/porthole/build/playtest/report.md`).
  The launcher scenarios that ran: `a2_launcher_pages`, `b2_home_stars`,
  `b3_admin_games`, `b1_long_presses`, `14_shell_edges`, `15_shell_taps`, plus
  the monkeys `20`-`23` (sanitizers clean). No new scenario added: `b2_home_stars`
  already covers star toggling, and finding 1 is a comprehension issue, not an
  assertable state bug.
- `ui` geometry (`supplementary/ui-geometry.txt`): tiles 40x30 with 4 px gaps,
  header 91x24, sun/star 24x24, arrow 24x22 (exactly the 24x22 floor), house
  32x32. All within the audit's rules.
- Captions for all ten games (`supplementary/captions.png`, `captions.txt`).
- Carried touch: a press begun right after a screen change and held 1 s is
  ignored; in star mode a press dragged from one tile onto another toggles
  nothing (`supplementary/carry.txt`, `carry_and_drag.png`).

## Gaps

- Evening and night tints, the rest screen and the daily cap while on Home or
  in star mode (need clock skips).
- The red "new game" dot (needs a game new to an existing profile).
- Several kids on one device; admin switching games off while stars exist;
  fewer than seven games switched on (no Library).
- Leo would probably type numbers on "Pick 4 numbers" rather than "skip"; the
  brief fixed "no code", so this pass skipped it.
- Mashing on Home itself, drag-off on the arrow/star/check.
- Only one emulator, no physical panel (touch noise, real finger size), no real
  children. The agent had the app brief's design description loaded (see
  disclosure).

## Verdict

**Needs fixes.** Zoe reaches Rally and puts it first unaided (an estimated
60-90 s, with a hesitation at "Home full! / Tap a star" and the counter), and
nothing can be lost. But Leo, a non-reader, can enter star mode by pressing the
most attractive button and then silently strip his Home while nothing opens
(finding 1, Major). After a fix, a fresh agent should replay the Leo path and
Zoe's "Home full" path on the new build.

## Earlier report (read after finishing)

Read `launcher-home-playtest.md` (build `50558dc`) only after the findings
above were written. Against its findings:

- **M1 (star mode reads as "switched off", refusal is a one-frame jiggle):
  largely fixed, part seen again.** Tiles are no longer dimmed (white empty
  stars instead), and a refused star now says "Home full! / Tap a star" with
  hopping stars and arrow. Seen again: the refusal still lands on a page with no
  gold star (my finding 2), and the "Pick 5·N" counter still reads as a decimal
  (my finding 3; their suggestion of a star counter is still open).
- **m1 (Home order is star order; a double tap in star mode reorders): order
  fixed, reorder seen again in a new form.** A new star now goes first, so
  Zoe's Rally led her Home with one star. But an unstar + re-star still moves a
  game, now to the front (Leo's two presses on the dog, `l11`/`l12`), and a
  double tap still toggles twice (my finding 4). The larger issue I saw is that
  Leo's mashing in star mode removes games from Home altogether (my finding 1).
- **m2 (All games tile looks like a painting game): fixed.** The tile now shows
  six tiny game pictures; Zoe spotted the car inside it.
- **n1 (star button gives no hint): partly fixed.** A press shows "Pick games";
  a quick tap still gives no hint, and Home has none (my finding 5).
- **n2 (star-mode edits lost at a restart): fixed.** Edits save at once
  (`lr01`). Note the flip side: Leo's accidental edits persist too.
- **n3 (no swipe paging), picker "+ New" mis-hit:** not retested (gaps).
