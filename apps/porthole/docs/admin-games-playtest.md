# Admin > Games: grown-up black-box playtest

Date: 2026-11-11. Tester: independent `playtester` agent (Claude, opus). This is emulator roleplay of a grown-up,
not a test with a real parent and not a test on a physical device.

## Build

- Revision: `50558dc` (clean tree; the only untracked files are this report and its evidence folder).
- Executable: `apps/porthole/build/host/snap`, sha256
  `91f7ab4b37746985d58f0c57f8456a365014f19236e2ee72538c228954df180d` (`make -C apps/porthole snap` said
  "Nothing to be done". The hash was the same after `make playtest`).
- The build reports its version as `dev` on the picker and Info.

## Persona

A parent, 40, comfortable with phones, setting up Porthole for two kids. They want to hide a game they don't want on
the device, and then check the kids' screens. They don't read manuals. They use ordinary taps, sometimes off-center,
and sometimes tap twice when they aren't sure a tap registered.

## Dispatch brief (verbatim, as received)

> You are an independent playtester for Porthole's grown-ups' settings (a round 480x480 touchscreen with no buttons,
> emulated on the host). Follow apps/porthole/docs/playtesting.md (the grown-up persona variant) and the `playtest`
> skill. Play source-free: don't read source (apps/porthole/games/, os/, shell/, host/), tests/playtests scripts or
> docs/superpowers/. Decide from the screen. Disclose anything that loaded automatically.
>
> ## Persona
> A parent, 40, comfortable with phones, setting up Porthole for two kids. They want to hide a game they don't want
> on the device, and to check the kids' screens afterwards. They don't read manuals.
>
> ## Setup
> - `make -C apps/porthole snap`. Record `git rev-parse --short HEAD` and `sha256sum apps/porthole/build/host/snap`.
> - Run `apps/porthole/build/host/snap --serve --fresh` from apps/porthole/. Create two kid profiles through the
>   screens. The grown-ups' menu is reached by holding the "Who's playing?" title for 5 seconds (this is the
>   documented way in, and the only hint you get). Ordinary touches only. No dbg or shortcuts.
>
> ## Scope
> Find the place to choose which games are on the device. Switch two games off (one of them a kid's first-page game)
> and one back on. Try to switch every game off. Leave and check both kids' launchers, including any "all games"
> view. Reboot the sim (`reboot` in serve mode, if the protocol has it, or restart without --fresh) and check that
> the choice stuck. Judge discoverability, wording, feedback, mis-taps on the small adult controls, and whether
> anything a kid had (favorites, saves) is lost.
>
> ## Output
> Write apps/porthole/docs/admin-games-playtest.md, with screenshots and the input trace in
> apps/porthole/docs/admin-games-playtest/ (no .py files; save scripts as .txt). Include the revision and hash, the
> persona, this brief, findings ranked with repro steps, and gaps. Don't fix code or commit. Reply with ONLY (under
> 10 lines): a verdict (acceptable / needs fixes), the top findings, and the report path.

Brief revisions: none in this round.

### Disclosure: what loaded automatically or was read

- **Loaded automatically:** the repo-root `CLAUDE.md` and `apps/porthole/CLAUDE.md` (the app brief). The app brief
  describes this feature in one paragraph: Games is on Admin page 2, rows with on/off switches four to a page, the
  last game on cannot go off ("Keep one game on"), switches save immediately, and favorites/saves are kept. It also
  describes Home/Library favorite rules (five favorites, a hidden favorite keeps its place until a new star needs
  it). So I am **not a clean first-time-discovery witness for where Games lives.** I still recorded what the screens
  themselves offer a parent who hasn't read anything (see "Discoverability" below). A fresh agent without the app
  brief in context would be needed to back up a first-time discoverability claim.
- **Read on purpose:** `apps/porthole/docs/playtesting.md`, the `playtest` and `tour` skills, and lines 40-140 of
  `apps/porthole/tools/webemu.py`, only for the `--serve` stdin protocol (`down/move/up/tick/frame`).
- **Not read:** anything under `games/`, `os/`, `shell/`, `host/`, `tests/playtests/` or `docs/superpowers/`.
- **Used:** no `dbg`, `profile`, `app` or other shortcuts during the black-box pass. The one supplementary `ui`
  geometry probe (below) was run after the black-box pass and is labelled.

## Setup and method

`snap --serve --fresh` was driven from `apps/porthole/` through a small socket broker kept in the session scratch
directory (not committed). Each finger action was a `down X Y`, then `tick 80`, then `up` and a 600 ms settle, all in
logical px. Holds used `down`, a long `tick` and `up`. Frames were captured with the protocol's `frame` command and
saved as native 480x480 PNGs. Every command sent is logged in order in `admin-games-playtest/input-trace.txt`, with
`# shot NAME` lines where each screenshot was taken and `# start` / `# quit` lines around the two processes. A few
shot names were reused when I redid a step after a mis-aimed tap of my own (55-57, 59-60, 80-81). The later
capture overwrote the earlier one, and both attempts remain in the trace.

## Paths played (black-box)

1. **Fresh device, first profile (Ada):** I typed the name "Ada", picked a fox face, age 7, and skipped the code.
   That opened Ada's Home: Pets Club, Biscuit, Marble Kick, Tilt FC, Spooky and an "All games" tile (01-06). I held
   each tile to read its game's name in the yellow caption (07). All games has 10 games over two pages: Radar, Star
   Fighter, Kingdom, Toy Town and Rally are on top of the Home five (08-09).
2. **Second profile (Ben):** from the picker, New, then "Ben", bear face, age 7, skip (12-14). Ben used star mode to
   unstar Tilt FC and star Rally, which is on Library page 2 (15-19). Ben's Home was then Pets Club, Biscuit, Marble
   Kick, Spooky, Rally and All games.
3. **A save to protect:** Ada opened Biscuit and named the pup "Max" (22-25). I left through the home button.
4. **Finding the setting:** first, plausible wrong guesses. I tapped "dev" and held it 2 s: nothing (28-29). I held
   the title 1.5 s and 3 s, and released at 3 s: nothing, and no sign that holding does anything (30-32). Holding
   5.2 s opened Info (33-35), which has no code yet. I tried "Home" on Info, which turned out to be Radar's home
   location (37), and went back. Then Admin (39): page 1 is Set admin code, Screen time and Player PINs. I tapped
   just left of the small ">" (40), which did nothing, then tapped ">" itself. Admin 2/2, "Device settings", has
   Clock, Sound and Games (41). Games is 10 rows over three pages (42, 49, 50).
5. **Switching:** I tapped the "Biscuit" *name*, not the switch. The whole row toggled to a grey row with the switch
   off (43). Probes: a tap on the 2 px seam between two rows did nothing (44). Pressing on Marble Kick and sliding
   off before lifting changed nothing (45). A double tap on Tilt FC toggled it twice (off, then back on: 47-48). A
   1 s long press on Radar toggled on release (86-87). A tap on the Kingdom picture toggled it (88). I switched
   Rally off (51). Biscuit is a first-page game for both kids and Rally is Ben's starred favorite.
6. **Leaving and checking kids** (Biscuit and Rally off):
   - Ada's Home shows Pets Club, Marble Kick, Tilt FC, Spooky and **Radar**, plus All games. Radar filled
     Biscuit's place and wears a star in the Library (55-57).
   - Ben's Home shows **three** tiles plus All games: his picked favorites, without the hidden Biscuit and Rally
     (58).
   - Both Libraries list 8 games: Biscuit and Rally are gone (56-57, 59-60). In Ben's star mode the counter reads
     "Pick 5 · 2" (61).
7. **Back on:** I turned Biscuit back on (65-66).
8. **Trying to switch every game off:** I turned off every row on pages 1 and 2 (67-68), then tried Toy Town, the
   last one on. A toast said "Keep one game on" and the switch stayed on (69). The toast had gone 1.5 s later (70).
   Then I re-enabled eight games one at a time (71-72).
9. **Final check** (only Rally off): Ada's Home is back to her original five, Biscuit included, and Radar is off
   her Home (73-75). Ben's Home has four favorites plus All games (76). Ada's Biscuit still says "MAX, Day 1 ·
   puppy" (77).
10. **Restarts:**
    - `reboot` in serve mode, then the picker. Ben's Home and Library still have no Rally (78-81).
    - A full process restart without `--fresh`: Admin > Games page 3 still shows Rally off (82-83). Turning Rally
      back on puts it back on Ben's Home with his star (84-85). His pick survived it being hidden and a restart.
11. **Six or fewer on:** I switched off Radar, Kingdom, Toy Town and Rally, leaving 6 games on. Both kids' Home
    then shows all six, with no All games tile and no star button (90-91). The kids can't edit favorites in this
    state. That is expected with nothing to choose between.

## Qualitative assessment (grown-up persona)

- **Can the parent recognize the goal and actions, and tell what changed?** On the Games screen itself, yes, at once.
  Each row has the game's own launcher picture (the same one the kids see), its name and a phone-style green/grey
  switch. An off row turns grey with a grey switch (43, 51). This is immediately legible to a phone user. Getting
  there is the weak part. The 5 s hold has no feedback before it fires (30-32), and the parent has to tell the
  "Home" button on Info (Radar's location) from the orange home button (37). Games is on page 2 of Admin behind a
  small ">" next to the heading, while page 1 has an empty band between Screen time and Player PINs (39, 41).
- **Feedback and consequences.** Each toggle changes the row instantly. The only refusal ("Keep one game on") is clear
  and short (69). What the screen does *not* say is what "off" means: hidden from the kids, or deleted along with
  their progress? A parent who cares about a kid's pet will hesitate here. I only learned saves were kept by
  checking (77, 85).
- **Friction and stopping point.** The parent's real task (hide one game, glance at the kids) takes about 5 steps
  into the menu, 1-2 taps and 3 taps back out. A parent would stop happily there. Undoing the "all off" experiment
  took 8 separate taps over two pages with no "all on" option. That is tolerable for 10 games but tedious. Exiting
  from Games to the picker is three home-button taps (Games, Admin, Info, picker; 52-54). Each step is clear.
- **Recovery from imprecise input.** Good:
  - a seam tap does nothing;
  - a slide-off cancels;
  - toggles act on release;
  - a near miss beside ">" does nothing;
  - every mistake is one tap to undo, with immediate visual feedback.

  A double tap from uncertainty toggles twice and ends where it started (47-48). That is normal phone-switch
  behavior, and the visible state makes it obvious. Nothing a kid had was lost: Ada's named pup survived, and Ben's
  starred Rally came back with its star after being hidden, rebooted and restarted.

## Findings (ranked)

No Blocker or Major findings.

### Minor

**M1. The Games screen doesn't say that "off" only hides the game and keeps the kids' progress.**
- Screen: `sys_games`.
- Concern: comprehension/trust.
- Repro: picker, hold the title 5 s, Admin, ">", Games. The rows show only picture, name and switch. There's no
  heading or note such as "Off: hidden from kids. Saves kept."
- Impact: a cautious parent may not switch a game off for fear of wiping a pet, or may assume an off game is deleted
  and its progress gone.
- Screenshot: `apps/porthole/docs/admin-games-playtest/42-games-p1.png`

**M2. Games is buried: Admin page 2, behind a small ">", with a decoy "Home" on Info.**
- Screens: `sys_info` / `sys_admin`.
- Concern: discoverability.
- Repro: Info shows Wifi / Updates / Home / Admin. "Home" opens Radar's location (37). Admin 1/2 lists only code,
  screen time and PINs, with empty space above Player PINs (39). Games is only on 2/2 (41).
- Impact: a parent looking for "which games" scans page 1, doesn't find it, and has to notice "1/2". A parent who
  hasn't read the brief might give up or try every button.
- Screenshots: `apps/porthole/docs/admin-games-playtest/37-info-home.png`, `39-admin.png`, `41-admin-p2.png`
  (same folder).

**M3. Default-favorites kids and kids who picked favorites behave differently when a Home game is switched off.**
- Screen: `launcher` (kid's Home).
- Concern: consistency.
- Repro: Ada never starred anything; Ben did. Switch Biscuit off.
  - Ada's Home pulls in Radar, starred, as a fifth tile (55, 56).
  - Ben's Home just shrinks to three tiles (58).
  - Switch Biscuit back on and Radar silently leaves Ada's Home and loses its star (73-74).
- Impact: probably fine for kids (they keep a full or near-full Home), but the parent checking "what will the kids
  see" sees two different rules. A default kid can see a "new favorite" appear and vanish without touching
  anything. Recorded as a design question for the owner (game-designer/shell owner), not necessarily a bug.
- Screenshots: `apps/porthole/docs/admin-games-playtest/55-ada-home-2off.png`, `58-ben-home-2off.png`,
  `73-ada-home-final.png` (same folder).

### Nit

**N1. The 5-second title hold gives no feedback.**
- Screen: `picker`.
- Concern: feedback.
- Repro: hold "Who's playing?" 3 s and release. Nothing changes at 1.5 s or 3 s (30-32).
- Impact: intentionally hidden from kids. A parent who was told "hold 5 s" can't tell whether a slightly short hold
  even counted. A subtle progress cue only after about 3 s would help grown-ups without inviting kids. Owner's call.
- Screenshot: `apps/porthole/docs/admin-games-playtest/31-hold-title-3s.png`

**N2. There's no bulk "all on" or reset.**
- Screen: `sys_games`.
- Concern: pacing.
- Repro: after exploring "all off", restoring takes 8 single-row taps across pages 1-2 (71-72).
- Impact: low with 10 games.
- Screenshot: `apps/porthole/docs/admin-games-playtest/72-p1-on.png`

**N3. The "Keep one game on" toast covers the neighboring row.**
- Screen: `sys_games`.
- Concern: overlap.
- Repro: with only Toy Town on, tap Toy Town. The toast covers the Rally row for about 1 s (69).
- Impact: brief and harmless. Rally can't be read or confirmed for that second.
- Screenshot: `apps/porthole/docs/admin-games-playtest/69-last-game-tap.png`

**N4. The rows are 21 logical px tall with 2 px seams.**
- Screen: `sys_games`.
- Concern: target size/spacing.
- Repro: the supplementary `ui` probe gives rows `22 30 116 21`, `22 53 116 21` and so on (exactly at the 2 px
  spacing rule).
- Impact: within the grown-ups' 18 px floor and passes the audit. Seam taps safely do nothing (44). An adult
  fingertip aimed at the edge of a row could flip its neighbor, but the result is visible and one tap to undo. No
  action needed unless real-device use shows mis-flips.
- Screenshot: `apps/porthole/docs/admin-games-playtest/44-boundary-tap.png`

### Positives worth keeping

- Whole-row toggle, on release, with drag-off cancel.
- The real launcher art in each row.
- An immediate grey "off" state.
- The clear last-game refusal.
- Survives `reboot` and a process restart.
- Kids' saves (the Biscuit pup's name) and picked favorites (Ben's Rally star) are restored when a game comes back.

## Gesture / interruption matrix

| Case | Result |
| --- | --- |
| Tap row name / picture / switch | toggles (43, 88, 51) |
| Tap 2 px seam between rows | nothing (44) |
| Press, slide off, release | nothing (45) |
| Double tap | two toggles, net unchanged (47-48) |
| Long press 1 s | toggles on release (86-87) |
| Near miss beside small ">" | nothing (40) |
| Last game off | refused, toast (69) |
| Leave via home x3 to picker | works (52-54) |
| `reboot` (serve) | settings kept (78-81) |
| Process restart, no `--fresh` | settings kept (82-83) |
| Restart while a toggle's save is in flight | **gap** |
| Admin code set, then Games | **gap** (no admin code was set in this run) |
| Save failure ("Could not save") | **gap** (needs a sim hook, out of black-box scope) |
| Kid inside a game when it's switched off | n/a: Admin is reached only from the picker |
| A game switched off while it was a kid's *last opened* / "come back to Library page" target | **gap** |
| Kids' launcher with only one game on | **gap**: I saw the refusal but didn't leave to view a one-game Home |
| Evening/night tint of the Admin screens | **gap** |

## Supplementary QA (not part of the black-box pass)

- `make -C apps/porthole playtest` at `50558dc`: **93 of 93 scenarios pass, 0 UI FAIL, 0 UI WARN**. I read
  `apps/porthole/build/playtest/report.md` in full. It includes `b3_admin_games` (26 checks), `b2_home_stars` and
  `a2_launcher_pages`. There are no automated findings to file.
- A `ui` geometry probe of `sys_games` page 1 (`admin-games-playtest/supplementary-ui-games.txt`, which uses the
  `profile` shortcut) gives:
  - the rows as 116x21 at y = 30/53/76/99;
  - the ">" arrow as 22x18;
  - the disabled "<" as not registered as a target;
  - text contrast: navy `182852` on `FFF3EF`.

  I measured the off-row label separately from a screenshot: `5A554A` on `C6C3C6` is about 4.2:1.
- I added no new scenario: `b3_admin_games` already covers this screen in the suite.

## Evidence

- Screenshots, native 480x480: `apps/porthole/docs/admin-games-playtest/NN-*.png`. The numbers match the steps above.
- Input trace: `apps/porthole/docs/admin-games-playtest/input-trace.txt`.
- Supplementary probe script: `apps/porthole/docs/admin-games-playtest/supplementary-ui-games.txt`.

## Suggested brief improvements

- Give the next tester the brief *without* the app `AGENTS.md`/`CLAUDE.md` auto-loaded, or ask for a fresh agent in a
  context that doesn't contain it. The app brief describes where Games lives and its rules, which weakens any claim
  about first-time discoverability.
- Ask explicitly for an admin code to be set first. That is the realistic parent setup, and it puts the Games screen
  behind the code.
- Ask the tester to view a kid's Home with exactly one game on.

## Verdict

**Acceptable**, with the Minor findings above routed to the shell owner/game-designer for a decision. No Blocker or
Major remains. The core task (hide a game, see it vanish from both kids' Home and All games, have it stick across a
reboot and a restart, lose nothing) works, and its feedback is clear. M1 (no "hidden, not deleted" reassurance) and
M2 (buried on page 2) are the most worth fixing. A fresh agent should confirm discoverability, because the
auto-loaded app brief compromised that part of this run.
