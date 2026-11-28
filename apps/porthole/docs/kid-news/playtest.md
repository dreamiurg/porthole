# Kid news: playtest

Independent playtester pass, emulator roleplay only (no real child, no physical device).
Build: HEAD `971b611` plus the uncommitted Kid news working tree; host `build/host/snap` sha256 `ed84268761683aa6…`
(not rebuilt for this pass). `make playtest` was not run (coordinator asked for no rebuild), so no automated FAIL/WARN
counts are claimed here; `ui` dumps were read for the screens below.

## Persona and brief

- Main: a 6-year-old early reader (short words; guesses longer ones from pictures), has used the device for weeks,
  impatient, taps fast and 3–6 px off, double-taps, pokes things that are not buttons, taps right after a screen changes.
- Second quick pass: a 9-year-old fluent reader.
- Goal given: pick yourself after an update, find out what is new, go find it. No solutions or tap coordinates beyond the
  sim's entry steps (header 60,42 to the picker; first picker row 80,66).

## What the kid saw

Screen 1 "New for Zoe!": "Learn times tables!" (3x4 card icon), "Tilt sand and water!" (sand picture), yellow arrow.
Screen 2: "Puzzles win hats!" (Pets Club pup), "More! Look for dots" (big red dot), green OK.
Launcher page 1: red dots on Pets Club, Biscuit and the page arrow; page 2: Rally, Sand, Times tables, and the back
arrow dotted while Biscuit still is. See `playtest-news-screens.png`, `playtest-launcher-dots.png`.

## Assessment

- **Understanding.** Short, picture-led cards; the 6-year-old reads "Tilt sand and water!" and "More! Look for dots",
  and gets "times tables" from the 3x4 icon. The red-dot card is the key that explains the launcher dots. The 9-year-old
  reads it all at a glance. The sand icon reads as coloured stripes at this size (nit).
- **Finding the new things.** Dots lead straight to the games. In Pets Club the Play button's dot, then the Puzzles
  dot in the Play menu, lead to Puzzles in two taps; both dots clear on tapping Puzzles, and leaving the Play menu
  without picking keeps the Play dot (`playtest-pets-club-dots.png`). The card promises hats, but the Puzzles screen
  shows "0 stars" and no hat, so a 6-year-old who came for hats may be puzzled (minor).
- **Boring or annoying?** Two screens, four cards, about 10 s of reading. Not boring; it is the kid's own name and
  it ends fast. It does not come back after OK, after picking again, or after a reboot.
- **Recovery.** Taps on cards and on the title do nothing and give no feedback (the kid may poke a card 2–3 times
  expecting it to open the game), but they do no harm. There is no way back from screen 2 to screen 1.
- **Where the kid stops.** After OK, at the first dotted tile, which is the intended outcome.

## Findings

1. **Major: a double tap on the arrow skips screen 2 unseen.** The arrow (68,117 24x22) sits inside where OK appears
   (45,116 70x26), and screen 2 has no tap guard: a second tap 0–600 ms after the first lands on OK and opens the
   launcher. The kid never sees "Puzzles win hats!" or the "Look for dots" explainer, and the update counts as seen.
   Screen 1 does have a guard (a double tap on the picker row that ends on the arrow stays on screen 1 at 60 and 200 ms
   gaps, and only advances at about 530 ms). Concern: recovery / comprehension.
   Repro: `reset` / `profile Zoe 6` / `dbg news` / `tap 60 42` / `wait 450` / `tap 80 66` / `wait 450` / `tap 80 128` /
   `wait 120` / `tap 81 129` / `wait 450` / `screen` → `launcher`. Screenshot: `playtest-double-tap-skip.png`.
2. **Minor: "Puzzles win hats!" but Puzzles shows stars and no hat.** Concern: comprehension. Repro: as above with
   `newgame Zoe Rex` / `wait 2400` / `dbg news`, then Play, then Puzzles. Screenshot: `build/host/pt_puzzle.bmp`.
3. **Minor: cards look tappable but do nothing.** No feedback on a tap; an impatient kid may think it is broken.
   Concern: feedback. Repro: on screen 1, `tap 50 55`, `tap 100 90`.
4. **Nit: the Play dot sits on the red-and-white ball icon**, red on red; the white rim keeps it readable. Concern:
   contrast. Screenshot: `playtest-pets-club-dots.png` (first frame).
5. **Nit: the sand card's picture is hard to read at card size.** Concern: comprehension.

## Checked and fine

- A fresh kid (`profile` without `dbg news`) gets no screen and no dots on either launcher page.
- News is per kid: with two old kids and one new one, each old kid gets their own "New for <name>!", and the new kid
  gets nothing.
- Tile dots clear on opening the game (Pets Club, Biscuit) and stay cleared after a reboot; page-arrow dots follow
  the other page.
- A double tap on OK does not also flip the launcher page (the launcher has an entry guard).
- Off-target taps (4–5 px) on the arrow, Play and Puzzles all register.

## Gaps

Rally's "together" dot was not tested (it needs a second device). Sand, Times and Rally were not opened. There was no
process kill partway through the news screens (only a reboot after OK), and no `make playtest` run.

## Verdict

**Needs another pass.** Finding 1 is a Major: with this persona's normal double taps, the second screen and the dot
explainer are skipped. Giving screen 2 the same tap guard as screen 1 should fix it; then replay the double-tap
repro. Everything else is acceptable to ship, with 2–5 deferred or fixed at the owner's choice.

## Resolutions (implementer)

- Finding 1 (double tap on the arrow skipped screen 2): fixed. Turning to screen 2 now re-arms the shell's fresh-screen gate (`Shell::updateNews`), so taps in the first 450 ms after it appears are ignored, as on every other screen. `tests/playtests/c2_kid_news.txt` replays the double tap (120 ms apart) and expects screen 2.
- Finding 2 (Puzzles shows 0 stars and no hat): kept. Hats are bought with stars won in Puzzles, so a new player starts at 0; the line says what the kid can do there.
- Findings 3 to 5: kept as they are (cards are not buttons; the dot's white rim keeps it readable on the ball; the sand icon is the launcher's own).
