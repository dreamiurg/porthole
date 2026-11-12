# Launcher Home and Library: child-perspective playtest 5 (final check)

Emulator roleplay by an independent agent. This is not a study with real children and not a test on a physical
device. Leo and Zoe are simulated perspectives.

## Build and setup

- Revision `169261b` (`git rev-parse --short HEAD`). The working tree was clean.
- `make -C apps/porthole snap` reported that the binary was up to date.
- I copied the binary into a private scratch folder and ran it there. Its sha256 is
  `3bd29b0f871622ebb84114897c37612d782b36f2d6e9645125fc37a752abaae5`. `build/host/snap` has the same hash after
  `make playtest`.
- I ran one long-lived `snap --serve --fresh` with the default clock (Tuesday 16:00).
  - A small driver (`launcher-home-playtest-5/serve-driver.py.txt`) sent `down`/`move`/`up`/`tick` commands and saved
    `frame` output as native 480x480 PNGs.
  - Restart checks killed the process and started `snap --serve` again without `--fresh`, so the saves were kept.
- I created both profiles through the screens. I typed names on the letter pads, picked faces, picked ages and tapped
  "skip" for the code.
  - Leo is 6, with the frog face.
  - Zoe was meant to be 8. My imprecise tap landed on **9** (bear face). The launcher shows nothing that depends on
    age, so I kept it. This is disclosed as a deviation from the brief.
- First play used no `dbg`, `app`, clock skips, hidden-state queries or profile commands. The supplementary QA below
  used `snap --script` with `debug`. It is kept separate and is labeled.
- Input trace: `launcher-home-playtest-5/input-trace.txt`. Driver shorthand: `tap X Y [MS]` means `down X Y`, then
  `tick MS` (default 120 ms), then `up`, then `tick 40`. `shot NAME` saves `shots/NAME.png`. `kill`/`restart` are
  the process restarts.
- Screens: `launcher-home-playtest-5/shots/`. Contact strips are in `launcher-home-playtest-5/strips/`. Strips are
  scaled to 360 px; tap coordinates were always taken from the native frames.

### Independence disclosure

- These files loaded automatically before or during play, and I did not ask for them:
  - the repository's root `CLAUDE.md`
  - `apps/porthole/CLAUDE.md`, which loaded when I read the tools folder
- `apps/porthole/CLAUDE.md` describes the Home/Library/star-mode rules in detail: newest star first, the 400 ms
  repeat rule, the default when no star is left, and the "Home full!" toasts. I therefore knew the intended rules
  before play.
- Because of that, I cannot certify **first-time comprehension**. The comprehension judgments below are roleplay
  informed by that knowledge. A fresh agent without it should confirm them if that claim matters.
- I chose every action from the frames. The two timing findings below contradict the documented rules; I found
  them from the screen, not from the docs.
- I also read the following before play:
  - `docs/playtesting.md` and the `playtest`/`tour` skills
  - `tools/webemu.py`, only to learn the `--serve` protocol
- I read no source in `games/`, `os/`, `shell/` or `host/`, no `tests/playtests` scripts and no `docs/superpowers/`
  files.
- I read the earlier reports only after play. Of those, I read only playtest 4's M1.

## Dispatch brief (verbatim)

> You are a fresh, independent child-perspective playtester for Porthole, a round 480x480 touchscreen with no buttons, emulated on the host. Follow apps/porthole/docs/playtesting.md and the `playtest` skill. Play source-free: don't read source (apps/porthole/games/, os/, shell/, host/), tests/playtests scripts, docs/superpowers/, or earlier playtest reports in apps/porthole/docs/ until you've finished playing. Avoid AGENTS.md/CLAUDE.md and disclose anything that loaded automatically. Decide from the screen. Run the sim in your OWN copy of the build directory if another process might be using apps/porthole/build/host (copy the snap binary into a scratch folder and run it there).
>
> ## Personas (two passes)
> A. Leo, 6, can't read, presses firmly for 1-3 s, mashes, double-taps, and loves gold stars.
> B. Zoe, 8, a confident reader. Rally and Pets Club are her favorites, and she wants them first on her screen.
> Simulated perspectives only.
>
> ## Setup
> - `make -C apps/porthole snap`. Record `git rev-parse --short HEAD` and `sha256sum` of the snap you run.
> - `snap --serve --fresh`, default clock. Create the kid's profile (age 6-8, no code) through the screens; ordinary touches only; no dbg, app shortcuts or clock skips.
>
> ## Scope (final check of the launcher)
> - Leo: play for several minutes, holding and mashing the gold things. In the mode where games get stars, take every star off and press the green check, then go back in. Do the screens agree with each other about which games are starred? Can he always reach his games? Does anything look broken or empty?
> - Zoe: put Rally and Pets Club first and take one game off her first screen. Time-to-goal, hesitations.
> - Leaving and restarting at awkward moments.
>
> ## Output
> Write apps/porthole/docs/launcher-home-playtest-5.md, with screenshots and the input trace in apps/porthole/docs/launcher-home-playtest-5/ (no .py files; save scripts as .txt). Include the revision and hash, the personas, this brief, findings ranked with repro steps, the assessment and gaps. Afterwards you may read apps/porthole/docs/launcher-home-playtest-4.md and say whether its M1 is fixed. Don't fix code or commit. Reply with ONLY (under 8 lines): a verdict (acceptable / needs fixes), the top findings, and the report path.

## What was played (first play, black box)

### Leo (6, non-reader, firm 1-3 s presses, mashing)

1. **Home after the profile** (`005_home`).
   - Five big pictures and a cream tile of six tiny pictures. No gold anywhere yet.
   - He held the tiny-pictures tile for 2 s. A yellow "All games" caption showed (`006`), and on release it opened
     "All games" (`007`).
   - Five tiles there wear gold stars. A big gold star button sits top right.
2. **The gold star button**, held for 2 s.
   - "Pick games" showed while held (`008`). Release turned the sky lavender: five gold stars in the header, gold
     corner stars, a white star on Radar, and a green check (`009`).
   - The change is obvious without reading.
3. **Taking every star off.**
   - A 1.5 s press turned the dog's star white, and one header star went white (`010`).
   - A double tap on the second star did **not** change it (`011`). A later single tap did (`012`).
   - A quick 110 ms double tap on the third star counted once, as it should (`013`-`015`).
   - Repeating the first pattern (a hold on one tile, then a ~310 ms double tap on another) flipped the star off and
     back on (`016`-`019`, `strips/rep_strip.png`). See m1.
   - A 1.2 s press took the last star off, leaving all white (`020`).
4. **Green check** with a 1.5 s press.
   - Back on "All games", **all five default stars are gold again** (`021`).
   - Going back in shows five gold stars (`022`). Home shows the same five games (`023`).
   - Home, "All games" and star mode agree everywhere. Nothing looks empty or broken.
5. **Played a game and came back.**
   - A 2 s hold on the dog opened Pets Club. The splash leads to the parcel, which has a house button (`024`-`025`).
     That house button returned him to Home (`026`).
6. **Mashing in star mode.**
   - A triple tap on Radar's white star gave "Home full! Tap a star" (`027`). The gold stars hop (`066_full_a`-`e`)
     and the message clears after about 2 s.
   - A quick sweep across four tiles took four stars off (`028`). He pressed the check.
   - Home is Pumpkin plus "All games", centred (`029`-`030`). Every game is still reachable through "All games".
7. **Header and picker.**
   - Holding his name bar for 3 s showed no pressed look. Release went to "Who's playing?" (`031`-`032`).
   - Holding "Leo" there for 2.2 s showed a small plum bar under the name (`036`). Release went back to his Home,
     unchanged (`037`).
8. **Page 2.**
   - He starred Starfighter, Kingdom and the girl game. Rally was refused with "Home full! Tap a star"
     (`045`-`047`).
   - A triple tap on the check put him **back in star mode** (`048`, frame by frame in `050`-`054`). See m2.
   - The home button took him back to "All games" (`055`). Home showed the newest stars first (`056`).
9. **Rally from page 2.**
   - His first tap missed. The "All games" tile had moved, because Home now had four games and the rows were
     re-centred (`057`). See n1.
   - The second try worked. Rally's "Home" button returned to "All games" page 2 (`058`-`059`).
10. **Every star off across both pages, then the check, then a process restart** (`060`-`065`).
    - After the restart, Home is the default five. "All games" shows the same five gold, and star mode shows five
      gold.

**Where Leo stops by himself.** After roughly two to three minutes of starring and unstarring, he wants to play the
dog or the car, and he leaves star mode for a game. The stars are a short, satisfying fiddle, not a game.

For Leo, the gold stars "coming back" after he took them all off reads as magic, not as an error. He has no way to
end up with an empty Home, which is good. He never needed help to reach a game.

### Zoe (8 in the brief; the profile says 9; confident reader)

Goal: Rally and Pets Club first on Home, and one game taken off.

| Step | Action | What she saw | Shot |
|---|---|---|---|
| 1 | Finger on the first dog, then slides off | Caption "Pets Club"; the slide-off cancels | `074`-`076` |
| 2 | Hesitation: no edit button on Home. She tries a phone-style 3 s hold on Pets Club | Pets Club opens (wrong guess) | `077`-`078` |
| 3 | Taps the splash, then the house button | Back to Home | `079` |
| 4 | Taps the "All games" tile, reads it, sees gold stars on her Home games | "All games" | `080` |
| 5 | Taps the star button | Star mode | `081` |
| 6 | Arrow to page 2, taps Rally | "Home full! Stars: page 1" (clear to a reader) | `082`-`083b` |
| 7 | Back arrow, unstars Pumpkin (the game she takes off), forward arrow, stars Rally | 5/5 stars | `084`-`085` |
| 8 | Green check, then the house button | Home: **Rally, Pets Club**, Biscuit, Tilt FC, Marble Kick, All games | `086`-`087` |

- She reached the goal with about 10 taps after her one wrong guess, roughly 12 s of game time plus reading.
- The only real hesitation was step 2: Home has no edit button.
- The "Home full!" text told her exactly what to do.
- She then wanted Pets Club before Rally. Unstarring and re-starring Pets Club did that (`088`-`090`), but nothing on
  screen says the newest star goes first (n3). She found it by trying.
- Her setup survived a kill during Rally (`091`-`093`).
- Leo's Home was unaffected by her changes (`094`).

## Gesture and interruption matrix

| Case | Result | Evidence |
|---|---|---|
| Tap / firm 1-2 s press on tiles, the star button and the check | Acts once on release | `006`-`010`, `021` |
| 3 s hold on a Home tile | Opens the game (no edit mode) | `077`-`078` |
| Finger on a tile, then slide off | Caption only, cancelled | `074`-`076` |
| Quick double tap on a star (80-120 ms presses, ≤160 ms gap) | Counts once | `013`-`015`, QA sweep |
| Double tap on a star with firmer presses (≥250 ms) or a ~200 ms gap | **Toggles twice: no net change** (m1) | `016`-`019`, QA sweep |
| Triple tap on a star while Home is full | "Home full!", stars hop | `027`, `066_*` |
| Fast sweep over four tiles | Four toggles | `028` |
| Double tap on the green check | Second press ignored | QA sweep |
| Triple tap on the green check | **Re-enters star mode** (m2); changes already saved | `050`-`054` |
| House button in star mode | Back to "All games", visit discarded | `055`, `095` |
| House button while "Home full!" shows | Back to "All games", no leftovers | `095a`-`096` |
| Tap off-target (sky; the old place of a moved tile) | Nothing | `057` |
| Hold the Home header 3 s | Picker on release, with no feedback while held (n2) | `031`-`032` |
| Hold own name on the picker 2.2 s | Plays (bar shows, no delete) | `036`-`037` |
| Kill in star mode **before** the check, then restart | Edits discarded; Home as before | `038`-`040` |
| Kill right after the check | Edits kept | `041` |
| Kill after a check with zero stars, then restart | Default everywhere, and the screens agree | `063`-`065` |
| Kill inside a game (Rally) | Home intact after restart | `091`-`093` |
| Back from games | Pets Club → Home; Rally from page 2 → "All games" page 2 | `026`, `059` |
| Idle dimming, rest screen or daily cap during star mode | **Gap**: not tested | - |
| Evening/night tint of star mode | **Gap**: default clock only | - |
| Physical touch panel, real fingers | **Gap**: emulator only | - |
| Ordinary leave and resume through the picker (no restart) | Kept | `036`-`037`, `094` |

## Findings (ranked)

No Blocker. No Major.

### m1. Minor: a firm double tap on a star flips it twice, so it looks like the tap did nothing

- **Screen**: library, star mode.
- **Issue**: Two presses on the same star can arrive within the documented 400 ms repeat window and still both act
  when the presses are firm or a little slow. The star goes white and back to gold, so nothing seems to change.
- **Concern**: feedback / recovery.
- **Repro** (sweep; all `snap --script`, file `launcher-home-playtest-5/qa-double-tap-sweep.txt`):
  1. `reset`, `profile Leo 6`, tap All games (124,104), tap the star button (126,42).
  2. Press the second tile's star at (94,66) for D ms, lift, wait G ms, and press again for D ms.
  3. With D=80, gaps up to 200 ms count once. With D=120, gaps up to 160 ms count once. With D=250, every gap
     from 120 to 320 ms toggles twice.
  4. Interactively: a hold on one tile, then a 120 ms press, a 190 ms gap and a 120 ms press on another tile toggled
     twice. A 270 ms press-to-press pair counted once.
- **Player impact**: Leo double-taps when unsure. His second tap undoes his first, which is the "it didn't work"
  loop the repeat rule was meant to stop. Nothing is lost, and a third tap fixes it.
- **Screenshots**: `launcher-home-playtest-5/strips/rep_strip.png` (`shots/016_rep_a.png`-`019_rep_d.png`),
  `shots/011_unstar2_dbl.png`.

### m2. Minor: a triple tap on the green check goes back into star mode

- **Screen**: library and star mode. The check and the star button are the same spot.
- **Issue**:
  - The first press saves and leaves star mode, and the second is ignored. A third press about 400 ms after the
    first lands on the star button and enters star mode again.
  - Two presses are always safe (gaps 100-400 ms, presses 80-300 ms).
- **Concern**: feedback / consistency.
- **Repro**:
  - `launcher-home-playtest-5/qa-check-triple-tap.txt`: three 100 ms presses with 100 ms gaps → `star_mode=1`. A 60 ms
    gap gives 0, and 100, 150 and 250 ms gaps give 1.
  - Interactive: trace lines after `shot 050_dc_0`.
- **Player impact**: a masher ends up still on lavender and may think the check failed. His edit was saved, and the
  house button or the check leaves. A gate like the 450 ms screen-change gate on the mode switch would cover it.
- **Screenshot**: `launcher-home-playtest-5/strips/s10.png` (`shots/050_dc_0.png`-`054_dc_4.png`).

### n1. Nit: the "All games" tile moves when the number of Home games changes

- **Screen**: launcher.
- **Concern**: consistency (target memory).
- **Repro**:
  - With five favorites, "All games" is at bottom-right (124,104).
  - With four, it is at (102,105). With one, it is beside the single game (`030`).
  - Leo's remembered tap at (124,104) missed (`057`).
- **Note**: it is harmless and visible, and his second tap worked. Centred rows are a deliberate design choice.
- **Screenshot**: `shots/057_rally.png`.

### n2. Nit: holding the Home header shows no pressed state

- **Screen**: launcher.
- **Concern**: feedback.
- **Repro**: hold (66,42) on Home for 3 s. The bar looks the same until release, which then opens the picker.
- **Note**: the tiles show a caption and the picker rows a bar, but the header shows nothing. Minor for Leo; he
  rarely goes there.
- **Screenshot**: `shots/031_hold_header.png`.

### n3. Nit: the Home order rule (newest star first) is invisible

- **Screen**: library, star mode → launcher.
- **Concern**: comprehension.
- **Repro**: to put Pets Club before Rally, unstar Pets Club, star it again, press the check, then go Home
  (`088`-`090`). Nothing hints at this.
- **Note**: Zoe's stated goal was met without it, because Rally, newest, came first and Pets Club, the first
  default, came next.
- **Screenshot**: `shots/090_z_home_pc_first.png`.

### n4. Nit: "all stars off" comes back as all gold

- **Screen**: library, star mode.
- **Concern**: comprehension.
- **Repro**: unstar everything, then press the check (`020`-`022`).
- **Note**: this is the intended fix for the old empty Home, and every screen agrees. To a 6-year-old the stars just
  "came back"; Leo likes that. I list it only so the behavior is a recorded decision.
- **Screenshot**: `shots/021_after_check.png`.

## Qualitative assessment

- **Goal and actions recognisable?**
  - Leo: yes.
    - Pictures carry Home and "All games".
    - The lavender sky and the hopping stars tell him he is in a different mode.
    - The white-to-gold star change is the feedback.
    - The header stars count Home's places.
  - He cannot read "Home full!", but the hopping gold stars and the refused white star tell him something said no.
  - Zoe: yes, after one wrong guess (a phone-style hold on Home). The names under a finger and the "Home full! Stars:
    page 1" text guided her straight to the fix.
- **Decisions and feedback.**
  - Starring is a real choice: which five games are "mine".
  - Consequences show on Home immediately after the check.
  - The weak spots are m1 (a double tap undoes itself) and m2 (mashing the check re-enters the mode).
  - Reading carries only the "Home full!" variants and the "Pick games" caption.
- **Repetition, frustration, stopping.**
  - Star mode is a short tidy-up task.
  - Leo stops after two or three minutes to play a game.
  - Zoe stops once Home shows Rally and Pets Club, under a minute.
  - Nothing repeats tediously. The natural stopping point is the check followed by Home.
- **Recovery.**
  - Every screen has the orange house button.
  - Leaving or killing before the check discards the visit, and the check saves at once.
  - Zero stars falls back to the default, so Home is never empty.
  - Every game is reachable from "All games" at all times.
  - Interruptions never lost a saved Home. Kids' Homes stayed separate.

## Playtest 4's M1: fixed

Playtest 4's M1:

- "All games" showed gold after a zero-star check, but star mode then showed every star white.
- One new star then emptied Home to a single game.

On this build:

- After a zero-star check, "All games" (`021`, `064`), star mode (`022`, `065`) and Home (`023`, `063`) all show the
  same five default games gold, before and after a process restart.
- A new star in that state is refused with "Home full!" (`066_*`) instead of wiping Home.

## Automated suite

- `make -C apps/porthole playtest` at `169261b`: **93 of 93 scenarios pass, 0 UI FAIL, 0 UI WARN**. The launcher
  scenarios `a2_launcher_pages`, `b2_home_stars`, `b3_admin_games` and the monkeys `20`-`23` are included.
- I read the full report (`apps/porthole/build/playtest/report.md`).
- I added no new formal scenario. m1 and m2 contradict the current behavior, so a regression scenario belongs with
  the fix. The two raw repro scripts above are ready to become `expect` checks.

## Gaps

- First-time comprehension is not certified (see the independence disclosure).
- Idle dimming, the rest screen and the daily cap during star mode were not tested.
- Only the default clock was used, so evening and night tints were not seen.
- The tile jiggle was judged from single frames, not motion.
- Zoe's profile age is 9, not 8.
- Emulator only: no physical panel and no real children.

## Verdict

**Acceptable.** There is no Blocker or Major. M1 from playtest 4 is fixed. m1 and m2 are small timing polish that
mostly affects mashers. Each is recoverable with no loss, and both are worth fixing.
