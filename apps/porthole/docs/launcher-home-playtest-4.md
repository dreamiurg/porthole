# Launcher star mode: independent child-perspective playtest 4

Emulator roleplay by an AI playtester. These are simulated perspectives only. No real child and no physical device
were involved.

## Builds and setup

The binary changed twice during the session because the coordinator was committing fixes in parallel. Each build was
played, and its evidence is labelled with a prefix (`b1_`, `b2_`, `b3_`) in `launcher-home-playtest-4/`.

| Build | Source | `snap` sha256 | Played |
| --- | --- | --- | --- |
| 1 | `b4b0596` (the binary already in `build/host/` at start) | `4b3b21c324e95457e207afc4d31f42afc319a02101362b94243373f088edde81` | First play. Run from `apps/porthole/` against the shared `build/host/` saves. |
| 2 | `b4b0596` + uncommitted shell edits | `d12ad53c5f58d351f975c80f63f46ddc0d316678ffdd9238fb4ec0b6166d82fd` | Full scope again, in an isolated copy |
| 3 (final) | **`c1c5059`** "fix(porthole): star mode edits the kid's own list, never the fallback" (the tree I copied had a staged diff whose sha matches `git diff b4b0596 c1c5059`) | **`4afd480273544d6f758d42e5851f2e013f586e12ccb30c1be78acb7fc5dee2b5`** | Full scope again, in an isolated copy, plus the automated suite |

- Started with `make -C apps/porthole snap` and `snap --serve --fresh` at the default clock (Tuesday 16:00).
- I created Leo through the screens: name keyboard "Leo", frog face, age 6, "skip" for the code.
- Driver: a small socket relay around `--serve` (`down`/`move`/`up`/`tick`/`frame`). It is saved as
  `launcher-home-playtest-4/harness-*.txt`.
  - A "tap" is down, 100 ms, up.
  - A "press" is down, 1-3 s, up.
  - A "restart" closes stdin and starts a new `--serve` without `--fresh` on the same saves.
- No `dbg`, `app`, `profile`, `skip` or hidden-state queries were used during play.
- **Deviation from the brief:** during build 1, a concurrent `make ci` replaced the shared saves in
  `apps/porthole/build/host/`. My first process restart came back as someone else's profile ("Sam"), and the binary
  had been rebuilt underneath me.
  - From build 2 on, I ran a copy of the binary from a scratch working directory laid out like `apps/porthole/`
    (`<wd>/build/host/snap`), so the saves were isolated.
  - Build 1's process-restart result is therefore void.

### Source exposure (disclosure)

- The repo-root `CLAUDE.md` loaded at the start. It mentions the launcher but not star mode.
- I read `tools/webemu.py` lines 60-100 for the wire protocol. I also grepped `tools/playtest.py` only for its
  cwd/env handling.
- `git status` and `git log` showed me file names and two commit titles:
  - "Home falls back to the default without losing hidden stars"
  - "star mode edits the kid's own list, never the fallback"
- After play, while saving evidence, `apps/porthole/CLAUDE.md` (= `AGENTS.md`) loaded automatically. It describes the
  intended star-mode rules. I used it only to label M1 below as a deliberate rule. No play decision used it.
- I read `docs/launcher-home-playtest-3.md` only after play.
- I played build 1 first. So on builds 2 and 3 I was no longer a first-time player. First-time comprehension claims
  rest on build 1 and on the parts of the flow that did not change.

## Persona and dispatch brief (verbatim)

> You are an independent child-perspective playtester for Porthole, a round 480x480 touchscreen with no buttons, emulated on the host. Follow apps/porthole/docs/playtesting.md and the `playtest` skill. Play source-free: don't read source (apps/porthole/games/, os/, shell/, host/), tests/playtests scripts or docs/superpowers/. Disclose anything that loaded automatically. Decide from the screen. Afterwards you may read apps/porthole/docs/launcher-home-playtest-3.md.
>
> ## Persona
> Leo, 6, can't read, presses firmly for 1-3 s, mashes, double-taps, and loves gold stars. His big sister (9) sometimes helps. Simulated perspective only.
>
> ## Setup
> - `make -C apps/porthole snap`. Record `git rev-parse --short HEAD` and `sha256sum apps/porthole/build/host/snap`.
> - `apps/porthole/build/host/snap --serve --fresh` from apps/porthole/, at the default clock. Create Leo's profile (age 6, no code) through the screens. Ordinary touches only; no dbg, app shortcuts or clock skips.
>
> ## Scope (narrow retest of the picking-games mode on the launcher)
> - Leo enters the mode where games get gold stars (the screen turns a different color). He takes every gold star off, mashes, and presses the green check. What does his first screen look like afterwards? Can he still play his games? Is anything scary, empty or broken-looking?
> - He takes all stars off, then leaves with the house instead of the check. Then he tries again with the check.
> - The sister uses the stars to put Rally and Pets Club first. Does it work, and can she tell what she's doing?
> - Leaving and restarting at awkward moments.
>
> ## Output
> Write apps/porthole/docs/launcher-home-playtest-4.md, with screenshots and the input trace in apps/porthole/docs/launcher-home-playtest-4/ (no .py files; save scripts as .txt). Include the revision and hash, the persona, this brief, findings ranked with repro steps, the assessment and gaps. Don't fix code or commit. Reply with ONLY (under 8 lines): a verdict (acceptable / needs fixes), the top findings, and the report path.

The brief was not revised during the session.

## Paths played

### First play (build 1)

1. **Finding star mode.** Home had five tiles. Leo pressed the one with lots of little pictures and got "All games"
   with gold stars in tile corners (`b1_a06_alltile`). He pressed the big gold star at the top right because he
   loves gold stars. The sky turned purple, five stars appeared at the top, and a green check appeared
   (`b1_a07_starbtn`). The colour change and the star row made it obvious that something had changed.
2. **Taking stars off.** A tap or a 2 s press on a tile turned its star white, and one top star turned white too
   (`b1_a08_tap_dog`). A fast double tap counted once.
3. **Last star.** When he removed the last star, all five came back instantly with no sign of why
   (`b1_b03` -> `b1_b04` -> `b1_b05_t50`). Leo would read this as "the stars came back by magic" and keep mashing.
   Build 2 changed this (see the iteration history).
4. **Sixth star on page 2.** Starring a sixth game showed the text "Home full! Stars: page 1" (`b1_a13_star6`).
   Leo can't read it. The sister can.
5. **Check.** Pressing the check returned to "All games" with his stars. Home then showed his three games plus the
   All-games tile, and a game opened and came back fine (`b1_c09_home3`, `b1_c10_game_open`).
6. **Corner habit.** With fewer favourites, the All-games tile had moved. Leo's habit of pressing the top-right gold
   corner opened "How bright?" on Home instead. His next presses set the brightness to Medium, and a house press
   went to "Who's playing?" (`b1_d01_edit`, `b1_d04_house`).
7. **House in star mode.** The house returned to "All games" with the stars as they were before the visit
   (`b1_e04_house`).

**Leo's first voluntary stopping point:** step 5. His Home showed the games he picked, and he would tap one and play.
He would not come back to star mode on his own. The magic snap-back in step 3 was the moment he would ask his sister
"why do they come back?".

### Retest on build 2, then final on build 3 (same results on both unless noted)

- **Every star off, mash, check** (`b3_X03_zero` .. `b3_X07_home`).
  - Star mode now allows zero gold stars: all five top stars are white.
  - Mashing the header, sky and grass does nothing. Mashing near tiles toggles them, which is expected.
  - After the check with no stars, "All games" and Home show the five default games with gold stars again. Home looks
    normal, not empty or scary (`b3_X07_home`), and Pets Club opens and returns fine (`b2_A14_game`).
- **Re-entering star mode after that** (M1, `b3_X08_all`, `b3_X09_edit_reenter`).
  - "All games" shows five gold stars, but pressing the star button turns all of them white.
  - Starring one game (Radar) and pressing the check leaves Home with only Radar and the All-games tile
    (`b3_X12_home`). The four games Leo had just been playing vanished from Home after one tap.
- **All off, then the house** (`b3_Y01_zero` .. `b3_Y03_home`). The visit is discarded and Home is unchanged. Doing it
  again with the check gives the default Home (`b3_Y05_home`).
- **Sister: Rally and Pets Club first** (`b3_Z00_edit` .. `b3_Z05_restart_home`).
  - A finger on a tile shows its name in a yellow bar ("Rally", `b3_Z01_rally_label`), so she can read what she is
    starring.
  - Starring Pets Club, then Rally, then the check gives Home = Rally, Pets Club, All games. That order survives a
    restart.
  - On build 2, starting from a full Home of five (`b2_C07_full_toast` "Home full! Tap a star"), she had to un-star and
    re-star both games to move them to the front. That worked (`b2_C15_home` = Pets Club, Rally, ...), but nothing in
    star mode shows Home's order. She found the rule only by going back to Home.
  - Dragging Rally on Home showed its name and did nothing else: no launch, no reorder (`b2_C09_drag_start`,
    `b2_C11_drag_end`).

## Gesture and interruption matrix (build 3 unless marked)

| Case | Result | Evidence |
| --- | --- | --- |
| Tap / 1-3 s press on a tile in star mode | Toggles once, on release | `b3_X03_zero` |
| Fast double tap on a tile (33 ms gap) | Counts once | `b1_a10_double_football` (trace) |
| Double tap on star button (33 ms and 250 ms gaps) | Enters star mode; the second tap does not hit the check underneath | `b2_R7b_dbl_star_fast` |
| Press tile, drag off, release | No toggle; name caption while held | `b1_g01`, `b1_g02_dragoff_tile` |
| Press check, drag off, release | Stays in star mode | `b1_g03` (trace) |
| Hold star button 2.8 s | "Pick games" caption, enters on release | `b1_g05_star_down` |
| Mash page arrows 24x | Ends on a page, no stuck state | `b3_Z08_arrow_mash` |
| House in star mode (page 1 and page 2) | Discards visit, back to All games | `b3_Y02_house_all`, `b2_R6c_home` |
| Process restart with unsaved star-mode edits (page 1, page 2) | Edits discarded; picker, then saved Home intact | `b2_R1a_unsaved`/`b2_R1c_home`, `b3_Z06`/`b3_Z07` |
| Restart immediately (0 ms) after the check | Saved | `b2_R2b_home` |
| Zero stars saved, then restart | Home = default; star mode = all white (M1 persists) | `b2_R3b_home`, `b2_R3d_edit` |
| Star mode idle 5 and 17 min, then tap | Still in star mode; the tap toggles a star | `b2_R4d_wake_tap` |
| Build 1 restart from shared `build/host` | **Void** (concurrent CI replaced saves) | `b1` trace |
| Screen dimming/dark and a wake tap on hardware | **Gap**: frames do not show idle dimming | - |
| Rest screen / daily cap during star mode | **Gap**: launcher idle time did not trigger it | - |
| Sound | **Gap**: not observable; none expected | - |

## Findings (prioritised, final build 3)

### M1. Major: the gold stars on "All games" vanish when Leo opens star mode, and one tap then empties most of Home

- **Screen**: library, star mode, launcher.
- **Issue**: After a check with no stars, Home and "All games" show the five default games with gold stars. Pressing
  the star button shows every star white. Starring any single game and pressing the check leaves Home with only that
  game. The other four, including the ones Leo just played, disappear from Home.
- **Concern**: consistency / comprehension / recovery.
- **Player impact**:
  - For Leo, opening the gold-star mode looks like his stars were taken away. This is the "broken-looking" moment
    the brief asks about.
  - One later tap shrinks his Home from five games to one.
  - Nothing is lost, because every game is still one press away through All games. But getting the old Home back
    means re-starring four specific games in order, which he cannot work out without help.
  - `AGENTS.md` describes this as the deliberate rule ("star mode edits the kid's own list: it can show no gold
    star"). The rule is consistent inside the code but not on the screen: two adjacent screens disagree about which
    games are starred.
- **Suggested direction (owner's call)**: either show the fallback games as gold when star mode opens over a default
  Home, or do not draw fallback games with gold badges in "All games".
- **Repro**: `launcher-home-playtest-4/repro-m1-zero-then-star.txt` (raw `--script`). I replayed it in a fresh
  process on build 3. Black-box path: trace build 3, lines for `X03`-`X12`.
- **Screenshot**: `docs/launcher-home-playtest-4/repro-m1-sheet.png`, `b3_X08_all.png`, `b3_X09_edit_reenter.png`,
  `b3_X12_home.png`.

### m1. Minor: Home's order rule is invisible while picking

- **Screen**: library (star mode).
- **Issue**: The newest star goes first on Home, but star mode shows no order. To put Rally and Pets Club first from a
  full Home, the sister must un-star and re-star them in reverse order and then check Home to see what happened.
- **Concern**: comprehension.
- **Repro**: build 2 trace `C12`-`C15`: from Home (kicker, football, Biscuit, Rally, Pets Club), star mode, then:
  1. Page 2: tap Rally twice, more than 400 ms apart.
  2. Page 1: tap Pets Club twice, more than 400 ms apart.
  3. Check, then home.
- **Screenshot**: `docs/launcher-home-playtest-4/b2_C07_full_toast.png`, `b2_C15_home.png`.

### m2. Minor: the gold corner at top right is brightness on Home and star mode on "All games"

- **Screen**: launcher / library. This repeats round 3's finding 7, and it happened to me.
- **Issue**: Leo learned "top-right gold = stars". On Home, the same press opened "How bright?", and his follow-up
  presses changed the brightness to Medium.
- **Concern**: consistency.
- **Repro**: build 1 trace around frames `d01`-`d06`: Home after favourites changed, press (126,42) 1 s, then press
  (80,72) 1.5 s.
- **Screenshot**: `docs/launcher-home-playtest-4/b1_d01_edit.png` (the "How bright?" screen opened instead of star mode).

### n1. Nit: the All-games tile moves when the number of favourites changes

- **Screen**: launcher.
- **Issue**: A remembered tap at (125,104) lands on empty sky once Home has three favourites instead of five.
- **Concern**: consistency.
- **Repro**: build 1 trace after `c09`.
- **Screenshot**: `docs/launcher-home-playtest-4/b1_c09_home3.png`.

### n2. Nit: "Home full!" is text-only for a non-reader

- **Screen**: library (star mode).
- **Issue**: The refusal is yellow text for about 2 s. In my frames, the only picture cue was the five gold header
  stars.
- **Concern**: feedback.
- **Repro**: build 2: full Home, star mode page 2, tap Star Fighter.
- **Screenshot**: `docs/launcher-home-playtest-4/b2_T2_up.png`, `b2_T5_2s.png`.

### n3. Nit: "check with zero stars" silently gives the five default games back

- **Screen**: library.
- **Issue**: Leo likely enjoys the gold stars returning. His sister, though, would see her "take everything off"
  undone with no explanation.
- **Concern**: feedback.
- **Repro**: build 3 `X03`-`X06`.
- **Screenshot**: `docs/launcher-home-playtest-4/b3_X06_all.png`.

## Qualitative assessment

- **Goal and actions from the screen.**
  - Positive: Leo recognised star mode from the purple sky and the white and gold stars. Taps changed a star and the
    header count at once, and pressing a tile shows the game's name for the sister.
  - Friction: the order rule (m1) and the vanishing stars (M1) can only be understood by going back to Home.
- **Decisions, feedback and pictures.**
  - The interaction is almost entirely pictures and it works without reading. The exceptions are the "Home full!"
    and "Pick games" text.
  - The consequence of a tap in star mode is clear. The consequence for Home is not visible until he leaves.
- **Repetition, frustration and stopping.**
  - Star mode is a short task, not something to play with. Leo stops once his Home shows "his" games.
  - The frustrating moment is M1: stars he saw as gold turn white.
  - The natural stopping point is the check followed by Home.
- **Recovery.**
  - The house, an unsaved restart and drag-offs all cancel safely.
  - Double taps and long presses behave.
  - An emptied Home never looks empty: it falls back to the defaults.
  - What remains is M1, which a 6-year-old cannot undo alone.

## Iteration history

- **Build 1**: removing the last star instantly re-starred the five defaults (`b1_b03`-`b1_b05_t50`), so Leo could
  never take every star off. That was already changed in build 2: zero stars allowed in star mode, with the default
  on Home after the check.
- **Build 2**: M1 found.
- **Build 3** (`c1c5059`, titled "star mode edits the kid's own list, never the fallback"): M1 is unchanged. It is the
  stated rule.
- I made no code changes and no commits. I did not edit the brief.

## Automated suite (build 3)

- I ran `PLAYTEST_SNAP=<build-3 copy> PLAYTEST_OUT=<scratch>/playtest python3 -W error tools/playtest.py` from
  `apps/porthole/`. This is equivalent to `make playtest` without the rebuild, which I skipped to avoid racing the
  coordinator's concurrent build.
- Monkeys used `build/host/snap-asan`, built at 00:10:23 from the same tree.
- Result: **93 of 93 scenarios pass, 0 UI FAIL, 0 UI WARN**, including `a2_launcher_pages`, `b1_long_presses`,
  `b2_home_stars`, `b3_admin_games`, and monkeys `20`-`23` with no sanitizer report. I read the report in full.
- `ui` dump of All games and star mode (`ui-star-mode-output.txt`):
  - Check/star button: 24x24.
  - Page arrows: 24x22, exactly at the floor.
  - Tiles: 40x30.
  - Home button: region at y = -4, which the audit accepts.

## Remaining limitations

- Emulator only, with no physical device. Idle dimming, the wake tap on a dark screen and the rest screen during star
  mode are untested gaps.
- I was not a fresh agent for builds 2 and 3. A fresh agent should judge first-time comprehension of the M1 rule if it
  is kept.
- **Suggested brief improvements**:
  - Tell the playtester to run a frozen copy of the binary in an isolated working directory.
  - Tell the coordinator not to rebuild or commit during a playtest, or to say which build is final.
  - Spell out what "takes every gold star off" should mean once a fallback exists.

## Judgment

**Needs another pass (needs fixes):** M1 is Major for this persona. Everything else in scope (the check, the house,
mashing, double taps, drag-offs, restarts, the sister's reordering) works and recovers safely on build 3.
