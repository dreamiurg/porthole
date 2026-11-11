# Launcher playtest: default build without Biscuit

Emulator roleplay by an independent playtester agent. This is not a real-child
study and not a physical-device test.

## Build identity

- Revision: `3414b4b` (clean tree; HEAD is "docs: say Biscuit is switched off
  on the device for now", on top of `e98cb50` "feat(porthole): per-game on/off
  switch, Biscuit off by default").
- Executable: `apps/porthole/build/host/snap` (default device build, `make -C apps/porthole snap`),
  sha256 `45e0d52ac4e569024455d6d86d4691d8d1ab564896b3b7f82ab8329c7e1d2b6e`.
- Mode: `snap --serve --fresh`, default clock (fixed Tuesday 16:00). Driven over the
  stdin protocol with `down X Y` / `tick MS` / `up` and `frame` (480x480 native
  frames saved as PNG). Restart check: a second `snap --serve` process without `--fresh`.

## Persona

Zoe, 8, a confident reader who plays Porthole most days, mostly Pets Club. Her taps
are sometimes a bit off, and she sometimes holds them. Simulated perspective only.

## Dispatch brief (verbatim, as received)

> You are an independent child-perspective playtester for Porthole, a round 480x480
> touchscreen with no buttons (emulated on the host): a "Who's playing?" picker, then a
> launcher of game tiles. Follow apps/porthole/docs/playtesting.md and the `playtest`
> skill. Play source-free: don't read source (apps/porthole/games/, os/, shell/, host/),
> tests/playtests scripts or docs/superpowers/. Decide from the screen.
>
> Persona: Zoe, 8, a confident reader who plays Porthole most days, mostly Pets Club.
> Her taps are sometimes a bit off, and she sometimes holds them. Simulated perspective only.
>
> Setup: `make -C apps/porthole snap` (the DEFAULT device build, not snap-all). Record
> `git rev-parse --short HEAD` and `sha256sum apps/porthole/build/host/snap`. Run
> `apps/porthole/build/host/snap --serve --fresh` from apps/porthole/, at the default
> clock. Make a profile for Zoe through the picker's + (age 8, no code), then use
> ordinary touches only. No dbg, app shortcuts or clock skips.
>
> Scope (narrow: the launcher changed): This build ships one fewer game. Check the
> launcher as Zoe: Can she find and open every game on both pages, and come back? Is the
> second page's short row laid out well inside the circle? Do the page arrows, captions
> and the brightness/sun button behave? Any game whose tile opens the wrong game or
> nothing at all? Open Pets Club and adopt the puppy, to confirm it still starts fine.
>
> Write apps/porthole/docs/launcher-playtest-no-biscuit.md, with screenshots and the
> input trace in apps/porthole/docs/launcher-playtest-no-biscuit/ (no .py files; save
> scripts as .txt). Include the revision and hash, the persona, this brief, findings
> ranked with repro steps, and gaps. Don't fix code or commit.

No brief revisions were made during this pass (single iteration).

### Suggested brief improvements

1. "Make a profile for Zoe through the picker's +" does not match the fresh device:
   `--fresh` opens straight on "Your name?" (`001_boot.png`); the picker with **+ New**
   only appears once a profile exists (`068_launcher_home.png`). Say "create Zoe on the
   first-run screens, then later use the picker".
2. State whether a profile created on the default build should also be checked for a
   leftover Biscuit save/tile (e.g. a fixture profile that played Biscuit on an earlier
   firmware). That upgrade path is the riskiest part of removing a game and is untested here.
3. Name the time budget for the persona (e.g. "Zoe gives a launcher about a minute
   before opening Pets Club") so voluntary-stop judgments are anchored.

## Source exposure disclosure

No game, shell, os or host source was read. To learn the `--serve` protocol I read
`apps/porthole/AGENTS.md` (the dev-target lines) and `apps/porthole/tools/webemu.py`
(the browser client). After play I ran one supplementary `--script` geometry probe that
used the `profile` shortcut (`launcher-playtest-no-biscuit/supplementary-ui-probe.txt`);
it did not inform any black-box decision.

## Setup and paths played

Session 1 (`launcher-playtest-no-biscuit/trace-session1.txt`, shots `001`-`071`):

1. Fresh device: "Your name?" -> typed Z-o-e (N-Z page switch, slightly off-center taps)
   -> OK -> fox face -> age 8 -> code screen "skip" -> launcher page 1 (`007_skip.png`).
2. Page arrow -> page 2 (`008_page2.png`): three tiles, Kingdom, Toy Town, Rally.
3. Each page-2 tile pressed (caption captured mid-press), opened, and exited with the
   game's own visible way back: Kingdom "Exit", Toy Town "<", Rally "Home" (held 700 ms
   before release). Each return lands on page 2.
4. Taps on empty sky where a second row would be (three spots): nothing happens.
5. Sun/moon button -> "How bright?" -> dimmest -> back on page 2, screen dimmed, icon
   changed; again -> home button (cancel) -> unchanged; again -> Auto.
6. Drag off the Rally tile before release: cancels. Swipe right across page 2: nothing.
7. Back arrow tapped near its corner -> page 1.
8. Each page-1 tile opened and exited: Marble Kick (home coin), Tilt FC (left arrow),
   Pumpkin Hide & Seek (framed arrow at top), Radar (home), Starfighter (EXIT).
9. Pets Club: splash -> "Knock knock!" package -> tap -> "A puppy! It needs a home."
   -> tap -> named the pup Bea -> picked the green house -> "Welcome home! Zoe and Bea,
   best friends." -> home room. Fed kibble ("First Meal" sticker). Home button -> launcher.
10. Double-tap next arrow, double-tap previous arrow; tap just outside the next arrow;
    hold the next arrow 600 ms then drag up onto the Radar tile and release.
11. Launcher home -> "Who's playing?" -> Zoe -> page 1 -> Pets Club resumes straight into
    the room with Bea (`070_pets_resume.png`).

Session 2 (`trace-session2.txt`, shots `r072`-`r075`): new process without `--fresh`
(process restart): picker with Zoe -> Pets Club resumes with Bea -> page 2 intact ->
tapping the Zoe name bar returns to the picker.

## Tile -> game map (all correct)

| Page | Tile art | Press caption | Opens | Way back used |
| --- | --- | --- | --- | --- |
| 1 | Dog face | Pets Club | Pets Club splash / room | top home button |
| 1 | Wooden tray | Marble Kick | Marble Kick start | home coin |
| 1 | Kicker on pitch | Tilt FC | Tilt FC kickoff | left arrow |
| 1 | Pumpkin | Spooky | Pumpkin Hide & Seek title | framed arrow, top |
| 1 | Radar scope | Radar | Radar | home button |
| 1 | Starship | Star Fighter | STARFIGHTER title | EXIT |
| 2 | Princess knight | Kingdom | Princess Pulverizer: The Living Kingdom | Exit |
| 2 | Doll face | Toy Town | Toy Town | "<" |
| 2 | Red car | Rally | Rally track picker | Home |

Nine tiles, no blank, duplicated or Biscuit tile, no tile that opens nothing.

## Gesture and interruption matrix

| Probe | Where | Result | Evidence |
| --- | --- | --- | --- |
| Clean tap | every tile, both arrows, sun | works | `029`-`044`, `008`, `028`, `021` |
| Off-center tap | Toy Town tile low-right, back arrow corner, keyboard keys | works | `013`, `028`, `003` |
| Tap just outside target | left of next arrow; empty sky on page 2 | nothing (correct) | `065`, `019` |
| Hold then release | Rally tile 700 ms; next arrow 600 ms | tile opens on release; arrow shows pressed | `016`/`017`, `066` |
| Drag off before release | Rally tile -> sky; next arrow -> Radar tile | cancels, nothing opens | `025`/`026`, `067` |
| Double tap | next arrow, previous arrow | one page change, no overshoot | `063`, `064` |
| Swipe | right across page 2 | nothing (no swipe paging) | `027` |
| Back out of every game | 9 games | all return to launcher, on the page they came from | `012`, `015`, `018`, `031`, `034`, `037`, `040`, `043`, `062` |
| Brightness cancel | home on "How bright?" | unchanged, back on page 2 | `023` |
| Ordinary leave/resume | Pets Club -> launcher -> picker -> Zoe -> Pets Club | resumes with Bea | `070` |
| Process restart | new `snap --serve` without `--fresh` | picker, Zoe, Bea kept | `r072`-`r074` |
| Carried touch across a screen change | not tested on a tile/arrow that changes screen on press (all act on release) | gap | - |
| Upgrade from a save that had Biscuit | not tested (no fixture) | gap | - |
| Night/evening launcher, multiple profiles | not tested | gap | - |

## Qualitative assessment

**Goal and actions recognizable?** Yes. Page 1 shows six picture tiles and a big yellow
arrow; Zoe goes for the dog (her game) or the arrow ("more games!"). Pressing a tile
pops its name into the yellow bar where her name was (`044_press_pets.png`), which is a
good confirmation for a reader and costs nothing for a non-reader. Page 2's arrow points
the other way and sits on the left, so "go back" reads clearly (`008_page2.png`).

**Feedback and consequences?** Every tap that should respond does: caption on press,
pressed arrow (`066`), game opens on release, dimmed screen and new sun icon after a
brightness pick (`022_dim.png`). Some captions are shorter than the game's title
("Spooky" opens "Pumpkin Hide & Seek", "Kingdom" opens "Princess Pulverizer"), but the
tile art carries the match, so Zoe isn't misled.

**Friction / boredom.** Little. The launcher is a hub, not an activity; Zoe would spend
seconds here. The one wasted gesture was a swipe on page 2 (`027_swipe.png`); she would
then tap the arrow, which is big and obvious. Page 2's single row leaves the lower sky
empty (tiles end at y=87, arrow at y=124 logical), but it looks like "fewer games here"
rather than "something missing". Voluntary stop: in an ordinary session Zoe would leave
the launcher as soon as Pets Club opened (step 9); visiting the other eight games was
coverage, not her choice. In Pets Club she'd stay to feed and play with Bea.

**Recovery.** Every game had a visible way back that returned to the same launcher page,
brightness can be cancelled, drag-off cancels a tile, and nothing was lost across
leave/resume or process restart.

## Findings (prioritized)

No Blocker or Major.

**1. Minor**
- **Screen**: launcher page 2 (default build).
- **Issue**: the automated suite never renders the shipping 9-tile launcher; it runs
  `snap-all`, which still includes Biscuit, so the 3-tile page 2 is only covered by this
  manual pass.
- **Concern**: consistency (regression coverage).
- **Repro**: `make -C apps/porthole playtest` builds/runs only `snap-all` and `snap-asan`
  (both every game); `tests/playtests/a2_launcher_pages.txt` therefore audits a different
  page 2 than devices ship. Owner: whoever owns `tools/playtest.py`/the Makefile (a
  default-build launcher scenario needs runner support; adding a `.txt` alone cannot
  select the `snap` binary).
- **Screenshot**: `apps/porthole/docs/launcher-playtest-no-biscuit/008_page2.png`

**2. Nit**
- **Screen**: launcher page 2.
- **Issue**: swiping does not change page; only the arrow does.
- **Concern**: feedback / consistency with phones and tablets kids know.
- **Repro**: on page 2, `down 40 105`, move to `125 105`, `up` (trace-session1.txt, the
  block before `shot 027_swipe`). Not a regression; recovery is immediate via the arrow.
- **Screenshot**: `apps/porthole/docs/launcher-playtest-no-biscuit/027_swipe.png`

**3. Nit**
- **Screen**: launcher page 2.
- **Issue**: the single row sits at the top of the grid, leaving a 37 px empty band above
  the arrow. Inside the circle and consistent with page 1's first row; vertical
  centering is an option, not a defect.
- **Concern**: spacing.
- **Repro**: launcher -> next arrow.
- **Screenshot**: `apps/porthole/docs/launcher-playtest-no-biscuit/008_page2.png`

**4. Nit**
- **Screen**: launcher caption vs. game title.
- **Issue**: caption "Star Fighter" (two words) vs. title "STARFIGHTER" (one word).
- **Concern**: consistency.
- **Repro**: press the starship tile on page 1, release.
- **Screenshot**: `.../041_press_star.png`, `.../042_star.png`

Observations outside scope (Pets Club, unchanged by this build; for the Pets Club owner
to triage, not counted against this launcher change):

- "A puppy! It needs a home." shows no "tap" hint after 5 s, unlike the screens before
  and after it ("tap to open", "tap"). Zoe knows to tap; a first-timer might wait.
  `049_box_5s.png`.
- The "Bone" choice on "What's for dinner?" renders as an outline that reads like "C Ɔ"
  rather than a bone. `058_feed.png`.
- A sparkle particle draws over the "needs a" text for a frame after opening the box.
  `047_open_box.png`.

## Supplementary QA

- `make -C apps/porthole playtest` at `3414b4b`: **91 of 91 scenarios pass, 0 UI FAIL,
  0 UI WARN** (report `apps/porthole/build/playtest/report.md`, read in full). Monkeys
  `20`-`23`, `52`, `63`, `74`, `81`, `94`, `98`, `a1` ran under ASan/UBSan with no reports.
  Caveat: all on `snap-all`, see finding 1.
- Geometry probe on the default `snap` (`supplementary-ui-probe.txt`): page 1 has 10
  regions (home, name bar, sun 24x24, next arrow 24x22 at (84,124), six 40x30 tiles);
  page 2 has 7 (same minus three tiles, previous arrow 24x22 at (52,124)). Arrows meet
  the 24x22 floor exactly; the farthest tile corner on page 2 is 68 px from the centre,
  well inside the glass.

## Gaps

- Upgrade path: a profile that had Biscuit progress on an earlier firmware, opened on
  this build (no fixture available to a source-free tester).
- Evening/night launcher art, more than one profile, a PIN profile.
- Carried touch into a game's first screen from a tile press (tiles act on release, so
  it was not reachable by ordinary taps).
- Physical device and real children: none. Emulator roleplay only.

## Iteration history

Single pass on `3414b4b`; no game fixes requested or retested.

## Verdict

**Acceptable** for the launcher change: every one of nine games is reachable on its
page, opens the right game and returns to the same page; page 2's row is inside the
circle; arrows, captions and brightness behave; Pets Club adopts and resumes. Finding 1
(coverage of the shipping launcher) should be routed to the playtest-runner owner.
