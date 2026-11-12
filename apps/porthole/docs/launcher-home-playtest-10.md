# Launcher Home / Library: kid playtest 10 (final pre-release check)

Independent, child-perspective emulator roleplay. **This is not a study with real children and was not run on a physical device.** Every observation below comes from the host simulator (`snap --serve`) and its rendered 480x480 frames.

## Build and setup

- Revision: `b208c63` ("fix(porthole): the house asks before dropping star edits; "again!" taps open nothing"), clean working tree.
- Executable: `make -C apps/porthole snap` (already up to date), copied to a private scratch folder. sha256 of the binary that was run: `241c860e80863bb8b641b11959ab37257d9d7dd74db223b12cd251e7547e8042`.
- Run: `snap --serve --fresh` with the default clock (Tuesday 16:00). It was driven by a small local driver that sends `down X Y` / `tick MS` / `up` and saves each `frame` as a PNG. One finger tap is `down`, a hold of H ms, `up`, then 40 ms. The driver logged every input, with simulated time, to `launcher-home-playtest-10/traces/`.
- Profiles: Zoe (bunny, age 8, no code) and "Loe" (fox, age 6, no code). The 6-year-old's name came out as "Loe" because he can't spell, and I left it that way. Both profiles were created on a fresh device through the name/face/age/code screens using ordinary taps. No `dbg`, `app`, `profile`, clock skips or hidden-state queries were used during the two black-box passes.
- Process restart: the driver killed the sim and started it again without `--fresh` (Zoe, after her goal).
- Supplementary QA, kept separate and listed below: raw `--script` runs using `profile`, `screen`, `debug` and `ui` for timing sweeps, plus `make -C apps/porthole playtest`.

### Disclosure

- These files loaded automatically before play: the repository `CLAUDE.md` and `apps/porthole/CLAUDE.md` (= `AGENTS.md`). The app brief describes the Home/Library/star-mode design in detail: newest star first, the check keeps, the house asks "Tap [check] to keep" and a second tap drops, and the Library's 800 ms / 2.5 s guard after "All games". So I knew the intended rules before playing. I made choices from the frames, but I can't claim first-time comprehension for myself. Zoe's success depended on reading the on-screen prompt, which she can do. For a true first-play claim, a fresh agent is needed (see the brief suggestions).
- I read `apps/porthole/docs/playtesting.md`, the `playtest` and `tour` skills, and `apps/porthole/tools/webemu.py` (only for the `--serve` stdin protocol). I read no game, os, shell or host source, no `tests/playtests` scripts, no design specs and no earlier launcher reports.

## Personas and dispatch brief (verbatim)

> You are a fresh, independent child-perspective playtester for Porthole, a round 480x480 touchscreen with no buttons, emulated on the host. Follow apps/porthole/docs/playtesting.md and the `playtest` skill. Play source-free: don't read source (apps/porthole/games/, os/, shell/, host/), tests/playtests scripts, docs/superpowers/, or earlier launcher playtest reports until you finish. Disclose anything that loaded automatically. Decide from the screen. Run the sim from your own copy of the snap binary in a private scratch subfolder.
>
> ## Personas (two passes)
> A. Leo, 6, can't read, taps firmly (200-400 ms presses), mashes 3-6 times, taps "again!" about once a second, and loves gold stars.
> B. Zoe, 8, a confident reader who wants Rally and Pets Club first on her screen and one game off it.
> Simulated perspectives only.
>
> ## Setup
> - `make -C apps/porthole snap`. Record `git rev-parse --short HEAD` and `sha256sum` of the snap you run.
> - `snap --serve --fresh`, default clock. Create the profile (age 6 or 8, no code) through the screens; ordinary touches only; no dbg, shortcuts or clock skips.
>
> ## Scope (final check of the launcher before release)
> - Zoe: her goal end to end, including leaving the star mode the way she thinks of first. Time-to-goal and hesitations. Does she ever lose her changes without understanding why?
> - Leo: several minutes of his play: mashing the stars, the check, the house and All games. Does he end up somewhere he didn't mean to, open a game he didn't choose, or change his first screen by accident? Can he always get back?
> - Anything confusing, broken-looking or boring.
>
> ## Output
> Write apps/porthole/docs/launcher-home-playtest-10.md, with screenshots and the input trace in apps/porthole/docs/launcher-home-playtest-10/ (no .py files; save scripts as .txt). Include the revision and hash, the personas, this brief, findings ranked with repro steps, the assessment and gaps. Don't fix code or commit. Reply with ONLY (under 6 lines): a verdict (acceptable / needs fixes), the top findings, and the report path.

No revisions were made to the brief during this run.

## Pass B: Zoe, 8, first play (`traces/trace-zoe.txt`, `shots/z*.png`, `zoe-all-frames.png`)

| t (sim) | What Zoe sees / thinks | Action | Result |
| --- | --- | --- | --- |
| 0-7.8 s | Name keyboard, faces, ages, "Pick 4 numbers" + skip | Z-O-E, OK, bunny, 8, skip | Home (`z06_home_first`): Pets Club, Biscuit, Marble Kick, Tilt FC, Pumpkin, "All games". No Rally. |
| 7.8 s | "Phones let me hold an icon to edit it" | Holds Biscuit 1.2 s | Yellow "Biscuit" caption, no edit mode (`z08`). Hesitation 1. |
| 9.0 s | "Don't open it" | Slides off to the grass and lifts | Nothing opens (`z09`). Good cancel. |
| 10.4 s | "More games -> pick there" | Taps All games | Library page 1: gold corner stars on her 5 Home games, a big star button, an arrow (`z10`). |
| 12.4 s | "The little gold star = on my Home, tap it to take Biscuit off" | Taps Biscuit's corner star | **Biscuit opens** (its name keyboard, `z12`). Hesitation 2 (Finding m2). |
| 12.9 s | "Oops" | House | Back to Library page 1 (`z13`). |
| 14.6 s | "The big star must be for picking" | Taps the star button | Lavender sky, jiggling tiles, green check, five gold counter stars (`z14`). Clear. |
| 16.4 s | | Taps Biscuit | Its star turns white and the counter shows 4 gold + 1 white (`z16`). Clear. |
| 17.7 s | "Where's Rally?" | Arrow | Page 2: the red car. Pressing it shows "Rally" (`z18`); lifting turns its star gold (`z19`). |
| 21.0 s | "Done, go to my **Home**" (the house *is* Home to her) | Taps the house | "Tap [check] to keep" in yellow while the check hops (`z20`). Hesitation 3, but she reads it. |
| 22.9 s | | Taps the check | Library page 2, normal mode, Rally gold (`z22`). |
| 24.6 s | | House | **Home: Rally, Pets Club, Marble, Tilt FC, Pumpkin, All games** (`z23`). Goal met. |
| 27-29 s | | Opens Rally, then Rally's own "Home" | Back on Home (`z25`). |

**Time-to-goal:** 17.6 s of simulated time from the first Home frame, with 10 taps, 1 hold and 1 detour into Biscuit. My waits were short (0.4-1.5 s), so a real 8-year-old would plausibly take about 45-90 s. She had 3 hesitations, and only the corner-star detour was a real mistake.

**Exiting star mode the way she thought of first:** the house. The ask worked: she read it, tapped the check, and lost nothing. She would have lost nothing on a *single* house tap either.

**Did she lose changes without understanding why?** Yes, in the follow-up probes (Finding m1). (a) She wanted Pets Club in slot 1, so she unstarred and restarred it, then double-tapped the house about 0.25 s apart, like a quick "home-home". The ask was on screen for less than 0.2 s. Back on Home, Rally was still first (`z28`, `z31`). (b) Same with a real edit: unstar Tilt FC, then double-tap the house. The ask flashes (`z33`), Tilt FC's star is gold again (`z34`), and Home is unchanged (`z35`). Nothing on screen says the edit was dropped.

**Ordering:** her goal was met partly by luck. Biscuit was the one she removed, and "newest star first" put Rally ahead of Pets Club. Nothing on screen says Home order = the order you starred. She found that unstar + restar moves a game to the front only by trying it (Nit n1).

**Voluntary stop:** right after `z23`. She has her Home and wants to race. Everything after that was coverage.

**Process restart:** after a kill and a relaunch without `--fresh`, "Who's playing?" appeared, and tapping Zoe showed the same Home (`z38`, `z39`). Kept.

## Pass A: Leo, 6, non-reader, masher (`traces/trace-leo.txt`, `shots/l*.png`, `leo-all-frames.png`)

About 113 s of simulated input (roughly 4-5 minutes of a real 6-year-old, counting looking time). Presses were 300-400 ms at about 1 per second, and taps were slightly off-center.

| Probe | What happened | Frames |
| --- | --- | --- |
| "Again!" on **All games** x5 (300 ms, 1/s) | Taps 2-3 did nothing. **Tap 4 opened Radar**, a game he never chose, from the Library tile under his finger. Tap 5, inside Radar, changed its range from 12 mi to 25 mi. | `l05`-`l09` |
| House x3 from Radar | Radar's house -> Library. The other two taps landed on grass: nothing. | `l10`-`l12` |
| **Big gold star** x5 | Entered star mode once. The other 4 presses landed on the empty spot: nothing. Good. | `l13`-`l16` |
| Radar's white star x4 (Home full) | "Home full! Tap a star" in yellow, and the gold stars hop. He can't read it, but the hopping draws him. Nothing changed. | `l17`-`l21` |
| Hopping Pumpkin star x3, then Radar x2, then Radar again | Every 1/s tap toggles: Pumpkin gold -> white -> gold -> white, Radar gold -> white -> gold. The result depends on how many times he tapped. | `l22`-`l27` |
| **Check** x4 | Kept once (Pumpkin off, Radar on), back to the Library. The other 3 presses landed on the "All games" title: nothing. Good. | `l28`-`l30` |
| House on Home x4 | Home -> "Who's playing?". The rest did nothing there. | `l31`-`l35` |
| His fox row x4 (aimed slightly high, at y=40) | Nothing (between the title and the row). | `l36`-`l39` |
| His fox x4 (on the row) | Tap 1 -> Home. **Tap 2 opened Radar** (his Home tile 1) in the same spot. Taps 3-4 changed Radar's range twice (3 mi, 6 mi). | `l40`-`l43` |
| Star mode: unstar Marble, then house x3 at 1/s | House 1: the ask. House 2: dropped and back to the Library. House 3: Home, unchanged. Good protection; he never sees a loss. | `l45`-`l48` |
| Pets Club, then house x2 | Home, then "Who's playing?" | `l49`-`l51` |
| Arrow x4 | Page 2 once. The other taps landed on nothing (the back arrow is elsewhere). Good. | `l53`-`l55` |
| Spaceship star on page 2 x3 (Home full) | "Home full! Stars: page 1", and the back arrow hops. Nothing changed. He gives up. | `l56`-`l58` |
| House (no edits) | Leaves star mode with no ask. | `l59` |
| Starfighter from page 2, then EXIT x3 (finger a bit left) | EXIT -> Library page 2. **The next tap carried onto the back arrow -> page 1**. "Where's my spaceship?" | `l62`-`l65` |

**Ends somewhere he didn't mean to / opens a game he didn't choose:** yes, twice, and both are deterministic at his cadence (Findings M1, M2). **First screen changed by accident:** no. The house protects it, and the only change he kept (Pumpkin off, Radar on) came from his own taps followed by the check. He did unstar Pumpkin only because its star hopped at him. **Can he always get back:** yes. Every screen he reached had the orange house (or Radar's house), and the fox row brings him back from "Who's playing?". The cost is that his "again!" taps there open a game (M2).

**Voluntary stop:** about 1.5 min in, when the spaceship star was refused ("Home full!") and he couldn't read why. The star game stops being fun at that point, and he goes to play a game.

## Findings (prioritized)

### M1 (Major): "again!" taps after All games still open the 6th Library game (Radar)
- **Screen:** library (opened from Home's All games)
- **Issue:** at Leo's cadence, one of his repeat taps opens the Library tile under his finger. Commit `b208c63` says "again!" taps open nothing.
- **Concern:** recovery / feedback (opens an unchosen game, and later taps change that game's settings)
- **Repro:** fresh profile, Home. Tap All games at (124,104) 4 times, 300 ms presses, 1 per second. Tap 4 opens Radar. The timing sweep in `launcher-home-playtest-10/scripts/results.txt` shows that with presses of 120-250 ms at 1 per second (`sweep_1000_250.txt`: down 250, up, 750 ms gap), **the 2nd tap already opens Radar**. With 300-400 ms presses at 1 per second, tap 4 opens it. At 0.7 per second, tap 3-5 opens it. At 1.2-1.5 per second, it is always the 2nd tap. The guard seems to cover only short release-to-press gaps and ends about 2.5 s after opening. A kid's "again!" falls outside both.
- **Screenshot:** `apps/porthole/docs/launcher-home-playtest-10/shots/l08_ag4.png` (Radar opened), `l09_ag5.png` (range changed)
- **Suggestion (owner's call):** keep tiles inert until a press starts **at least 800 ms after the last lift anywhere**, not just within 2.5 s of opening. Or require the first Library tile press to begin after the finger has been off the glass for about 1 s.

### M2 (Major): "again!" taps on the picker row open Home's first game
- **Screen:** picker -> launcher (Home)
- **Issue:** the first Home tile (36,70) sits where the kid's name row is (36,68). A repeat tap on your own name opens tile 1: Pets Club by default, Radar for Leo. Leo reaches the picker by mashing the house on Home, so "house, house, fox, fox" ends in an unchosen game.
- **Concern:** recovery / consistency (Home has no carry-over guard; the Library does)
- **Repro:** `scripts/pick_1000_300.txt` (profile, house to the picker, then 4 taps on (36,68), 300 ms at 1/s): picker -> launcher -> **splash (Pets Club)** -> intro. Live: `traces/trace-leo.txt`, "Leo taps on the fox itself 4x".
- **Screenshot:** `apps/porthole/docs/launcher-home-playtest-10/shots/l41_f2.png`

### m1 (Minor): a quick double tap on the house drops star edits with almost no explanation
- **Screen:** library, star mode
- **Issue:** with edits pending, house - house at any gap from 100 to 900 ms drops them (`scripts/house_*.txt`). The "Tap [check] to keep" ask is visible for only the gap between the taps. The star simply turns gold again. Everywhere else, the rule is that a double tap acts once (the stars, FreshGate).
- **Concern:** recovery / feedback (Zoe, a reader, lost a reorder and an unstar without seeing why)
- **Repro:** All games, star button, tap Tilt FC (it turns white), then tap the house twice about 0.2 s apart. Library shows Tilt FC gold, and Home is unchanged.
- **Screenshot:** `shots/z33_between_taps.png`, `shots/z34_after_double.png`, `shots/z35_home_after_double2.png`
- **Suggestion:** treat a second house press that begins within 600 ms of the first lift as part of the same tap, as star taps are. That keeps the protection for Leo's 1/s "house, house" while letting a reader's double tap only ask.

### m2 (Minor): the Library's corner gold star looks tappable but opens the game
- **Screen:** library (normal mode)
- **Issue:** Zoe tapped Biscuit's corner star to "take it off" and Biscuit opened (its new-pup name keyboard).
- **Concern:** comprehension (the decoration reads as a control)
- **Repro:** All games, wait 2 s, tap (94,64), which is Biscuit's star.
- **Screenshot:** `shots/z11_press_corner_star.png` (the caption says "Biscuit"), `shots/z12_after_corner_star.png`
- **Suggestion:** a press on a corner star outside star mode could start star mode or show "Pick games" instead of opening the game. Or leave it as is, since recovery is one house tap.

### m3 (Minor): a carried tap from a game's exit flips the Library page
- **Screen:** library page 2 after Starfighter's EXIT
- **Issue:** Leo's repeat tap on EXIT (finger a little left) lands on the Library's back arrow, and page 1 replaces the page he came from.
- **Concern:** consistency / recovery (harmless but disorienting: "where did my spaceship go?")
- **Repro:** Library page 2, open Starfighter, then tap (72,143) twice at 1/s, 300 ms presses.
- **Screenshot:** `shots/l63_e1.png`, `shots/l64_e2.png`

### n1 (Nit): Home order is invisible
- **Screen:** library, star mode
- **Issue:** "Newest star first" is not shown anywhere. Zoe's "Rally and Pets Club first" was met because Biscuit happened to be the one she removed. Putting Pets Club in slot 1 needed unstar + restar, found only by trying.
- **Concern:** comprehension
- **Repro:** fresh profile, goal "Pets Club first, then Rally": star Rally, uncheck Biscuit, check. Home shows Rally first.
- **Screenshot:** `shots/z23_home_after.png`

### n2 (Nit, for the Radar owner): accidental taps change Radar's range
- Carried "again!" taps from M1/M2 changed Radar's per-kid range (12 -> 25 mi, 25 -> 3 -> 6 mi) with no confirmation. It's harmless, but it is a persistent setting changed by mistake. Screenshots: `shots/l09_ag5.png`, `shots/l42_f3.png`.

### Observation (design trade-off, not filed)
At Leo's 1/s cadence each star tap toggles, so how a mash ends depends on how many taps there were. Together with "Home full!" making the gold stars hop, this led Leo to unstar Pumpkin. Nothing was lost by accident, because only the check keeps edits, and the check was his own choice.

## What worked (positive evidence)
- Star mode is unmistakable for a non-reader: lavender sky, jiggle, white vs gold stars and the 5-star counter (`l13`, `z14`). Leo saw it change.
- Mashing the star button (5x), the check (4x) and the arrow (4x) each acted once. The spare presses landed on nothing (`l16`, `l30`, `l55`).
- The house ask protects Home from Leo's "house, house" (`l46`-`l48`), and Zoe understood it from the words (`z20` -> `z22`).
- "Home full!" is clear for a reader, and the hops point a non-reader to the gold stars or the arrow (`l17`, `l56`).
- Holding a tile shows its name, and sliding off cancels (`z08`, `z09`). Dragging off the check cancels too (supplementary `shots/qa_check_dragoff.png`).
- A game opened from Home returns to Home, and one opened from the Library returns to its page (`z25`, `l62`). Edits survive a process restart (`z39`).

## Qualitative assessment

- **Goal, actions and feedback:** Zoe recognized the goal and the tools in about 4 actions: All games, then the big star, then the stars. Each tap changed something visible: star color, counter, caption. Leo understood that "the star makes stars" from the pictures alone. What he can't read is "Home full!" and "Tap [check] to keep", and the hops only partly replace the words. For him, the second message is just noise before the house drops the edits, which is safe.
- **Decisions, skill, consequences:** the choice is meaningful (5 places), and the counter makes the limit visible. Its consequence (Home order) is not shown (n1). Recovery is always one tap. The unfair moments come from carry-over: M1/M2 open a game the child didn't pick.
- **Repetitive, confusing or frustrating moments:** Zoe: holding a tile does nothing (phone habit), the corner star opens a game (m2), and a double tap on the house loses her edit silently (m1). Leo: "Home full!" refusals with no readable reason (the spaceship) end his interest after about 1.5 min. Being dropped into Radar twice is confusing but not upsetting. The natural stopping point for both is "Home looks how I want, now play".
- **Imprecise input, wrong guesses and interruptions:** slide-off cancel, off-center taps and firm 300-400 ms presses all behave. The house ask and the check-only save mean Home never changes by accident. Mis-taps and wrong guesses cost one tap to undo. The weakness is carried "again!" taps across screen changes (M1, M2, m3).

## Gesture / interruption matrix

| Gesture / interruption | Home | Library | Star mode | Picker | Notes |
| --- | --- | --- | --- | --- | --- |
| Single firm tap (200-400 ms) | ok | ok | ok | ok | |
| Off-center tap | ok | ok | ok | missed row (y=40), expected | |
| Hold > 1 s then lift | caption, opens on lift (z08) | not tested | not tested | **gap** (3 s delete hold not tested) | |
| Drag off before lift | cancels (z09) | not tested | check: cancels | not tested | |
| Mash at 1/s carried into next screen | **opens tile 1 after picker (M2)** | **opens tile 6 (M1)**; EXIT -> arrow (m3) | star/check/arrow: once | house mash: nothing | |
| Quick double tap (<300 ms) | not tested | not tested | **house drops edits (m1)**; star once | not tested | |
| Leave via game and return | ok | ok (same page) | n/a | | |
| Process restart | edits kept (z39) | | edits in progress: **gap** | ok | |
| Rest screen / daily cap during star mode | **gap** (needs a clock skip; excluded from first play) | | | | |
| Tilt / board move | **gap** | | | | |

## Supplementary QA (separate from first play)

- `make -C apps/porthole playtest` at b208c63: **93 of 93 scenarios pass, 0 UI FAIL, 0 UI WARN**. That includes `a2_launcher_pages` (16 checks) and `b2_home_stars` (72 checks). I read the report in full; there are no failures or findings to file. The automated suite does not catch M1 or M2: its scenarios evidently don't model 1/s carried taps past the guard window.
- The `ui` dump of star mode after a check drag-off: house 32x32, check 24x24, tiles 40x30, arrow 24x22. All are at or above the 24x22 floor.
- Timing sweeps and the house double-tap sweep are in `launcher-home-playtest-10/scripts/` (raw `--script` files) with outputs in `scripts/results.txt`. Rerun with `snap --script scripts/<name>.txt` from a scratch folder.
- Proposed regression (not added to `tests/playtests/`, because it would currently fail the suite; the owner/coordinator should add it alongside the fix): `profile Leo 6`, `wait 1500`, then 4x (`down 124 104`, `wait 250`, `up`, `wait 750`), then `expect-screen library`. A second scenario: the same from the picker on (36,68), then `expect-screen launcher`.

## Suggested brief improvements
- Exclude or warn about the auto-loaded `apps/porthole/AGENTS.md` launcher paragraph for first-play agents, or state up front that it will load. It gives away the star-mode rules and guard timings, so this run can't certify first-time comprehension.
- Specify the persona's press-to-press cadence *and* hold length for "again!" taps (e.g., 1/s with 120-400 ms presses), and ask for carry-over across **every** screen change (picker -> Home, game exit -> Library), not only All games. M2 and m3 were found that way.
- Add "a reader's quick double tap on the house" to Zoe's probes. A reader's double tap and a masher's 1/s taps need different protection.

## Verdict: **needs fixes**
M1 contradicts the fix this revision claims, and it reproduces for the target persona on the 2nd-4th "again!" tap. M2 is the same carry-over from the picker into Home's first tile. Both open an unchosen game. Recovery is one tap, but they are deterministic. Zoe's goal works end to end, star mode is clear, and Home is never changed by accident. m1 is a real but minor silent loss for readers. After fixes, replay Leo's All-games and picker mashing and Zoe's house double tap on the new build.

Gaps: no real children and no physical device. Rest screen or daily cap while in star mode, tilt, and the picker's 3 s delete hold were not tested. Only one fresh-device setup was used.
