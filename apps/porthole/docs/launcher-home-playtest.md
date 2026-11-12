# Launcher Home / All games / star mode: first-play black-box playtest

Emulator roleplay by an independent agent. This is not a study with real children and was not run on a physical device.

## Build and setup

- Revision: `50558dc` (clean tree apart from an unrelated untracked `docs/admin-games-playtest/`).
- `build/host/snap` sha256: `91f7ab4b37746985d58f0c57f8456a365014f19236e2ee72538c228954df180d`. `make -C apps/porthole snap` was already up to date. `make playtest` left the same hash.
- Harness: a copy of that binary run as `snap --serve --fresh` (later restarts were plain `--serve`, which keeps the saves) in its own scratch working directory, so saves never touched `build/host/`. It was driven only with `down` / `move` / `up` / `tick` and `frame` for native 480x480 PNGs. A "tap" is `down`, `tick 100`, `up`. A "press N" is `down`, `tick N`, `up`. The clock was the default (Tuesday 16:00). I used no `dbg`, `app`, `skip`, `debug` or `screen` queries during play.
- Profiles were made through the screens. Zoe: name keyboard, fox face, age 8, code skipped. Leo: in the persona, a grown-up typed "Leo" for him; then frog face, age 6, code skipped.
- Evidence is in [`launcher-home-playtest/`](launcher-home-playtest/):
  - the native frames (`a*` Zoe, `b*` restart, `c*`/`d*` Zoe's finger probes, `e*`-`j*` Leo and other probes, `strip_*` sequences, `z_pick_title.png` the zoomed header);
  - `input-trace.jsonl`, every command in order, with each process start marked;
  - `ui-probe.txt`, the supplementary `ui` geometry script.

### Disclosure (what I saw beyond the screen)

- `git log` titles: "Home of favorites, the Library and star mode" and "Admin -> Games switches games on and off per device". These are names only, with no mechanics.
- The shared scratch directory listing showed earlier file names such as `home-star-mode.png` and `home-library-p2.png`. I did not open them and worked in a fresh subdirectory.
- I read `tools/webemu.py` (dev tooling) only for the `--serve` wire protocol.
- **After the first-play and probe passes**, reading `build/playtest/report.md` auto-loaded `apps/porthole/AGENTS.md`. That file describes the Home/Library design: star order, "Pick 5 . N", the red "new" dot and switched-off favorites. Everything in "First play" below was recorded before that. The `ui` geometry dump was also used only afterwards, as supplementary QA. A fresh agent would be needed for any further first-time-comprehension claim.

## Personas and brief

Simulated perspectives only.

- **A. Zoe, 8.** A confident reader who plays Porthole most days; Pets Club and Rally are her favorites. Her taps are a bit off sometimes, and she sometimes holds them.
- **B. Leo, 6.** Can't read, presses firmly for 1-3 s, mashes and double-taps.

Exact dispatch brief (unrevised):

> You are an independent child-perspective playtester for Porthole, a round 480x480 touchscreen with no buttons (emulated on the host): a "Who's playing?" picker, then a launcher. Follow apps/porthole/docs/playtesting.md and the `playtest` skill. Play SOURCE-FREE: don't read source (apps/porthole/games/, os/, shell/, host/), tests/playtests scripts, docs/superpowers/, or AGENTS.md/CLAUDE.md files beyond what loads automatically (disclose anything that did). Decide from the screen. This is a first-play pass of a new launcher.
>
> ## Personas (two short passes)
> A. Zoe, 8, a confident reader who plays Porthole most days. Pets Club and Rally are her favorites. Her taps are a bit off sometimes, and she sometimes holds them.
> B. Leo, 6, can't read, presses firmly for 1-3 s, mashes and double-taps.
> Simulated perspectives only.
>
> ## Setup
> - `make -C apps/porthole snap`. Record `git rev-parse --short HEAD` and `sha256sum apps/porthole/build/host/snap`.
> - Run `apps/porthole/build/host/snap --serve --fresh` from apps/porthole/, at the default clock. Create the kid's profile through the screens (any age 6-8, no code), then use ordinary touches only. No dbg, app shortcuts or clock skips.
>
> ## Scope
> The launcher changed. Without being told how it works:
> - Does Zoe understand what she sees after picking her profile? Can she get to Rally? Does she discover she can choose which games appear first, and can she put Rally there? Can she take a game off?
> - Does Leo get to his games, or get stuck in a mode he doesn't understand (e.g. a picking mode where games don't open)? Can he always get back?
> - Can anything be lost? Is anything confusing, slow or boring?
> Also the brief's imprecise-finger probes on the new controls.
>
> ## Output
> Write apps/porthole/docs/launcher-home-playtest.md, with screenshots and the input trace in apps/porthole/docs/launcher-home-playtest/ (no .py files; save scripts as .txt). Include the revision and hash, the personas, this brief, findings ranked with repro steps, the qualitative assessment and gaps. Don't fix code or commit. Reply with ONLY (under 12 lines): a verdict (acceptable / needs fixes), the top findings, and the report path.

Deviation: I ran a copy of the binary from a scratch working directory instead of `apps/porthole/`. The behavior is the same and the developer's saves were not touched.

## First play: Zoe (8), what happened

1. **Home** (`a07_afterskip.png`). The top row has her fox and "Zoe" on a card, plus a sun/moon button. Below are six picture tiles: dog, puppy, green ball tray, soccer kid, pumpkin, and a cream tile with six colored squares. There are no names and no Rally.
   - *Zoe:* "Where's my car game?" The colored squares look like a painting game or "more stuff". She tapped it out of curiosity because nothing else looked like Rally.
2. **All games** (`a08_grid.png`). The header reads "All games", with a star button where the sun was. The same five games carry small gold stars, plus a Radar tile without one, and a yellow arrow sits underneath.
   - *Zoe:* "The starred ones are the ones on my home." She read that link correctly.
   - The arrow led to page 2 (`a09_page2.png`): jet, princess, girl and a red car.
3. **Tapped the car** and Rally opened (`a10_rally.png`). Rally's Home returned to All games page 2 (`a11_back_from_rally.png`). Reaching Rally took four taps and no confusion.
4. **Star button** (`a12_star.png`). The header changed to "**Pick 5·0**" with a green check, and every tile on page 2 went checkered/dim. *Zoe:* "Pick 5... dot zero? Pick five what?"
   - She tapped the dim car to star it. Nothing happened except a one-frame jiggle of the header text (`a13_car_in_pick_150.png` vs `a13_car_in_pick.png`). *Zoe:* "It's grey, so I can't pick it?"
5. She went back a page (`a14_pick_p1.png`) and saw five starred, bright tiles and a dim Radar. *Zoe:* "Oh, five stars are already used up." She tapped Marble Kick (green tray), which dimmed and lost its star while the header became "Pick 5·1" (`a15_unstar.png`).
   - Only then did "·N" read as "how many left", and dim read as "not starred" rather than "can't".
6. Page 2, tap the car: it gets a star and the header returns to "Pick 5·0" (`a16_star_rally.png`). Green check, then All games (`a17_check.png`). Orange home leads to Home with Rally on it, **in the last place** (`a18_home.png`).
7. **Taking a game off** worked the same way (`a24`-`a26`): Pets Club unstarred, the orange home tapped mid-star-mode, and the change was kept. With all five unstarred, Home is just the "All games" tile (`a31_home_zero.png`), which is still usable.
8. **Rally first.** I unstarred everything and starred Rally first. Home then showed Rally first, then the jet, dog and pumpkin in the order picked (`a35_home_new.png`).
   - Zoe would never guess the order rule. She would only notice it by luck after redoing her stars.
   - Rally opened from Home returned to Home (`a37_back_to.png`).
9. Other buttons: the name card and the orange home button on Home both lead to "Who's playing?" (`a19`, `a23`). Picking her row again lands on her Home. The sun/moon tile is "How bright?" (`a21`).

**Zoe's first voluntary stop:** right after `a18_home.png`, about 15 taps from her first Home. Rally is on her Home, so she would go race. Unstarring everything to put Rally *first* was coverage, not something she would do on her own. She was not bored, but steps 4-5 were the one moment she might have given up ("I can't pick Rally") if she hadn't gone back to page 1.

## First play: Leo (6), what happened

- **Profile.** Leo's Home has the default five games (`e05_leo_home.png`). Zoe's stars did not carry over to him.
- **Holds open games.** A 2 s press on the colored-squares tile opened All games. A 1.5 s press on the star entered star mode.
  - While a finger is on a tile, its name shows in a yellow bar over the header ("Rally", "Pets Club", "Radar", "All games"; `strip_c01.png`, `strip_e11.png`, `strip_f01.png`). Leo can't read it, but the screen visibly reacts.
- **Star mode** (`strip_e06.png`).
  - Firm presses on Radar did nothing, apart from the header jiggle he would not notice. Star mode was full.
  - A double tap on the dog unstarred and re-starred it (`strip_e11.png`), so it looked like nothing happened, but the dog moved to the end of his Home (`g05_leo_home.png`: puppy, tray, soccer, pumpkin, dog).
  - *Leo:* "Games don't open, everything's grey." He has two familiar ways out, and both work:
    - holding the green check for 2.5 s returns to All games (`f02_hold_check`);
    - the orange home button also leaves star mode.
  - A 2 s press on the dog then opened Pets Club (`f03_leo_pets`). Its home returned to All games (`f05_back_from_pets`).
  - He was never trapped. The longest "nothing happens" stretch was 3-4 presses.
- **Mashing home.** Four taps at 250 ms intervals took him All games → Home → "Who's playing?" (`strip_f06.png`). He presses his frog and is back.
  - An imprecise press at the boundary below his row hit "+ New" (`strip_g01.png`); this is outside launcher scope. The orange home button took him back.
- **Rally.** He reached it by picture: colored tile, arrow, car, opened with a 2.5 s press (`strip_g05.png`).

**Leo's first voluntary stop:** in star mode after a few presses that open nothing. He would press home and go play the dog game. He gets to his games without help, but his Home rearranges itself when he double-taps in star mode.

## Gesture / interruption matrix

| Probe | Where | Result | Evidence |
| --- | --- | --- | --- |
| Tap | Home tile, All-games tile, arrows, star, check, home | Acts once | `a*` |
| Hold 1-2.5 s | Home tile, star, check, Library tile | Acts on release; caption shows while held | `strip_c01`, `f02` |
| Drag off before release | Home tile to empty sky, and onto another tile | Cancels; neither opens | `strip_c02` |
| Swipe left across Library | Page 1 | No page change, nothing opens (arrows only) | `i02_swipe_left` |
| Double tap 120 ms | Star button | Enters star mode once | `strip_d05` |
| Double tap 300 ms | Check | Leaves once; second tap swallowed | `strip_d05` |
| Double tap | Starred tile in star mode | Toggles twice: net no star change, but the game moves to the end of Home | `strip_e11`, `g05` |
| Off-target taps | Header text, gap between header and star, 1 px outside the star, 4 px gap between tiles | Inert | `strip_d01`, `i01_gap_tap` |
| Sixth star | Star mode full | Refused; header text jiggles one frame | `strip_f01` |
| Home button mid star mode | Library | Leaves star mode, keeps changes | `a24`, `a25` |
| Ordinary leave/resume | Game → its Home → launcher | Returns to the page it came from (Library page or Home) | `a11`, `a37`, `f05` |
| Process restart (stdin closed, new `--serve`) | Home | Favorites and order kept | `b02_restart_home` |
| Process restart mid star mode | Unstarred puppy, then quit | Change dropped; old favorites kept, nothing else lost | `strip_h2` |
| Profile switch | Zoe ↔ Leo | Favorites are per kid | `e05`, `a20` |

**Not tested (gaps):**
- the red "new game" dot (it needs a game new to an existing kid, which ordinary input on one build cannot create);
- Admin → Games switching a favorite off (grown-up flow);
- six or fewer games switched on (no Library);
- rest/daily-cap expiry or idle dimming while in star mode;
- SIGKILL or power-cut timing;
- tilt (not relevant);
- a physical panel and real children.

## Findings (prioritized)

### M1 (Major): star mode reads as "these games are switched off", and the first try always fails
- **Screen**: library (star mode).
- **Issue**: Default favorites fill all five places, so a kid's first star-mode tap on a game they want is always refused. The refusal shows only as a one-frame jiggle of "Pick 5·0". Every unstarred tile is drawn checkered/dim, which looks disabled. That is literally true while the stars are full, and it still looks the same once a place frees up.
- **Concern**: comprehension / feedback.
  - Zoe, a fluent reader, read "Pick 5·0" as "pick five ... zero?" and concluded "It's grey, I can't pick Rally". She recovered only by going back to page 1 and inferring the limit from five bright starred tiles.
  - Leo gets no understandable signal at all.
  - This is the main path in the brief ("can she put Rally there?").
- **Suggestions (for the owner)**:
  - Make the refusal point at what to do: wiggle or flash the existing stars, or briefly show "Take a star off first".
  - Show the counter as stars (filled/empty) rather than "5·N".
  - Draw unstarred tiles as tappable with an empty star outline, rather than dithered like a disabled control.
- **Repro**: fresh profile (defaults) → tap the cream colored-squares tile (124,104) → tap the arrow (95,135) → tap the star (126,42) → tap the car (80,104).
- **Screenshot**: `docs/launcher-home-playtest/a12_star.png`, `a13_car_in_pick_150.png`, `a13_car_in_pick.png`, `z_pick_title.png`, `strip_f01.png`.

### m1 (Minor): Home order is star order, invisible and easy to disturb
- **Screen**: launcher (Home) / library star mode.
- **Issue**: A newly starred game always goes last. The only way to put Rally first is to unstar everything and re-star in order. An accidental double tap on a starred tile in star mode leaves the stars unchanged but moves that game to the end of Home.
- **Concern**: consistency / meaningful choice. Non-readers find games by position: Leo's dog moved from first to last after he mashed it.
- **Repro**: Leo's default Home → All games → star → double-tap the dog (36,72) 150 ms apart → check → home.
- **Screenshot**: `docs/launcher-home-playtest/e05_leo_home.png` vs `g05_leo_home.png`; `strip_e11.png`.

### m2 (Minor): the "All games" tile does not look like "more games"
- **Screen**: launcher (Home).
- **Issue**: Six colored squares on cream read as a painting/colors game. Its caption ("All games") appears only while a finger is on it. Zoe found it only because Rally was missing and nothing else fit.
- **Concern**: comprehension.
- **Repro**: any Home.
- **Screenshot**: `docs/launcher-home-playtest/a07_afterskip.png`, `strip_j.png` (third frame: the caption while pressed).

### n1 (Nit): the star button gives no hint of what it does
- **Screen**: library.
- **Issue**: Game tiles show a name caption while pressed; the star button (and the sun on Home) show nothing. It is discoverable by curiosity only.
- **Concern**: comprehension.
- **Repro**: All games → press and hold the star, then slide off.
- **Screenshot**: `docs/launcher-home-playtest/j01_press_star.png`.

### n2 (Nit): star-mode edits are not saved until you leave star mode
- **Screen**: library star mode.
- **Issue**: A restart mid-star-mode drops the unsaved toggles. Nothing else is lost: the previous favorites stay.
- **Concern**: recovery.
- **Repro**: All games → star → tap the puppy (80,72) → kill the process → restart → Leo.
- **Screenshot**: `docs/launcher-home-playtest/strip_h2.png`.

### n3 (Nit): the Library does not page on a swipe
- **Screen**: library.
- **Issue**: A sideways swipe does nothing, so kids used to tablets may try it. It is harmless (nothing opens) and the arrows work.
- **Concern**: consistency.
- **Repro**: All games → drag (130,100) → (30,100).
- **Screenshot**: `docs/launcher-home-playtest/i02_swipe_left.png`.

Out of scope, noted only: on "Who's playing?" with two kids, a press at y≈93 (between Leo's row and "+ New") hit New (`strip_g01.png`). This is a pre-existing screen; the home button recovers.

### Positives observed
- The press caption gives readers names on hover-like presses.
- Holds of any length open on release, and drag-off cancels cleanly.
- The return goes to where the kid came from (Library page or Home).
- Favorites are per kid and survive a restart.
- Zero favorites still leaves a usable Home.
- Every screen has the orange home button, and in star mode the home button is a safe exit.
- No beeps on navigation.
- No game, save or profile could be lost through the launcher.

## Qualitative assessment

- **Can the persona recognize the goal, the actions and what changed?**
  - Home and All games: yes. Zoe tied the small gold stars to her Home immediately, and pictures carry Leo to every game, Rally included.
  - Star mode: only partly (M1). "Pick 5·0" and the dimmed tiles misled a fluent reader on the first try; she understood the counter only after a successful unstar changed it to "5·1".
  - What changed after acting is clear: stars appear and disappear and Home updates.
- **Interesting choices, feedback, pictures vs reading?**
  - Choosing her five games is a meaningful, personal choice Zoe would enjoy.
  - Pictures do the work for opening games. Star mode leans on reading ("Pick 5·N"), and its refusal feedback is too faint for either persona.
  - The ordering rule is invisible (m1).
- **Repetitive, confusing or frustrating moments; where they stop?**
  - The only friction is star mode when full: 2-4 presses that do nothing.
  - Zoe stops once Rally is on Home (~15 taps). Leo stops fiddling in star mode after 3-4 presses and leaves through home.
  - There are no waits; screen changes feel immediate (input is ignored for about 450 ms after each one).
  - The natural stopping point is "my game is on Home".
- **Recovery without losing progress?**
  - Yes. Every star-mode state exits by check or home, refused taps are harmless, and mashing home lands at most on "Who's playing?", where his frog brings him back.
  - The only "loss" is unsaved star toggles at a restart (n2) and Leo's reordered Home (m1).

## Automated suite (supplementary)

`make -C apps/porthole playtest` at 50558dc: **93 of 93 scenarios pass, 0 UI FAIL / 0 UI WARN** (including `a2_launcher_pages`, `b2_home_stars`, `b3_admin_games`, `b1_long_presses` and the monkeys). I read the report in full; there are no findings to file.

The supplementary `ui` probe (`ui-probe.txt`) shows:
- Home/Library tiles of 40x30, 4 px apart;
- the star/sun button at 24x24 and the arrows at 24x22, both at the 24x22 floor;
- the home button at 32x32.

The audit has no geometry findings.

## Iteration history

One pass on build 50558dc / `91f7ab4b…`. No game changes were made and the brief was not revised.

Suggested brief improvement: state whether a fixture with a "new" game may be used, so the red-dot behavior can be checked source-free.

## Verdict

**Needs fixes**: M1 (star-mode refusal and the dimmed "disabled" look) is the core path of the change and confused a fluent 8-year-old persona. Retest star mode with a fresh agent after a fix; m1 and m2 are recommended. Emulator roleplay only; no real-child or physical-device validation.
