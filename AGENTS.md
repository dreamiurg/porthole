# porthole: agent brief

## What this is

A monorepo of small apps (so far, games) for one board: the Waveshare
ESP32-S3-Touch-LCD-2.1, a round 480x480 capacitive touchscreen with no physical
buttons. Each app lives in `apps/<name>/` and owns its code, content, tests and
docs. Build, test, release and flashing infrastructure is shared at the root.

| Path | What it holds |
| --- | --- |
| `apps/porthole/` | Porthole: the device firmware (shared runtime in `os/`, board layer in `firmware/`) and its games in `games/`. Profiles (up to four kids) and a launcher in `shell/`. First game: Pets Club (`games/pets-club/`): pixel dog, tricks, stories, spelling. C++ core + host simulator + browser emulator. A second game, Biscuit (`games/biscuit/`; design: `docs/superpowers/specs/2026-09-25-biscuit-on-porthole-design.md`), is a warm reading-and-discovery companion drawn full-color at native resolution on the same runtime. Two tilt games follow: Marble Kick (`games/marble-kick/`), a labyrinth, and Tilt FC (`games/tilt-fc/`; design: `docs/superpowers/specs/2026-09-27-tilt-fc-design.md`), top-down street football steered by tilt. Pumpkin Hide & Seek (`games/pumpkin-hide-seek/`) is a cozy Halloween hide-and-seek: grab three candies and escape while Grandma searches with a flashlight. Rally (`games/racing/`; design: `docs/superpowers/specs/porthole-racing/`) is an arcade racer steered like a wheel over four tracks. App brief: `apps/porthole/AGENTS.md`; each game also has its own `AGENTS.md` under `games/<game>/`. |
| `platform/` | Shared PlatformIO base (`waveshare-round.ini`), the factory-image script, pinned PlatformIO requirements. |
| `tools/gallery.py` | Builds the README screenshot strips (`docs/preview.png`, `docs/screenshots.png`) every app ships. |
| `docs/` | `hardware.md` (board, pins, round-screen rules), `new-app.md` (the app contract), `superpowers/specs/` (approved design docs). |
| `.claude/` | Agent roster and skills, generalized across every game under `apps/porthole/games/<game>/`; paths inside them are `apps/porthole/...`. |

When you work inside an app, read that app's brief first, then the specific
game's brief if you're touching one. Their constraints (round-screen geometry,
a flat/limited color and font set, append-only save layout, nothing in the
game ever punishes the kid for being away) are hard rules, not style.

Every brief is an `AGENTS.md`; the `CLAUDE.md` beside it is a symlink to it, so
every agent reads the same file. Edit `AGENTS.md`, and give a new brief the same
symlink (`ln -s AGENTS.md CLAUDE.md`).

## Shared rules for every app

1. **Round screen, no buttons.** Everything tappable and every glyph must sit
   inside the circle. Every screen needs a visible way back. See `docs/hardware.md`.
2. **The buzzer is harsh.** Never beep on routine taps or navigation. Keep sound
   for rare moments. One device-wide switch in the grown-ups' Admin menu turns it off for every app.
3. **Saves survive firmware updates.** Never reorder or resize a persisted
   struct, and never rename an NVS namespace that shipped. Pets Club keeps the
   namespace `"crago"` for exactly this reason.
4. **Kids 5-11 are the audience.** Failure is gentle, nothing dies, nothing is
   lost. No personal data: no real names, no analytics. Nothing sent over the network carries anything
   about the kid. The only traffic is the daily firmware update check, its download, a clock sync, and an optional public-IP time-zone guess to ipapi.co;
   while Radar is open, home rounded to ~1 km goes to adsb.fi every 5 s and the callsigns it sees to
   adsbdb.com; a grown-up's typed address goes once to OpenStreetMap Nominatim when Home is set.
5. **Verify on the host.** The board is often unplugged. That does not exempt a
   change from verification: run the app's `make check`, and `make ci` for
   anything user-visible. Every gameplay or UI change also requires a black-box
   emulator playthrough of the final build: play through ordinary visible inputs,
   inspect the experience, fix observed problems, and replay after changes.
   Automated tests, bots and staged screenshots alone do not satisfy this gate.
6. **Warnings are errors.** Every hook and CI job fails on any warning: `-Werror`
   on host and firmware builds (firmware: our sources via `build_src_flags`),
   biome `--error-on-warnings`, Python `-W error`, and `tools/fail-on-warning.sh`
   for tools with no fail switch. Fix the cause; never silence or skip it.
7. **Every game looks like itself.** The device's limits are shared, the look is
   not. Each game gets its own palette, art style, typography and characters,
   and never borrows another game's sprites, palette, colors or visual idiom,
   even when reuse would be faster. Two games side by side in the launcher must
   never look like twins. Reusing runtime, shell widgets and tooling is fine;
   what the kid sees inside a game is not shared.

## Commands

From the repository root:

| Command | Does |
| --- | --- |
| `make check` | Every app's `lint` + `test`. Same as the pre-commit gate. |
| `make ci` | Every app's full CI suite, minus firmware builds. |
| `make complexity` | Lizard gate (CCN 15, 60 lines, 5 params). |
| `make firmware APP=<app>` / `make flash APP=<app> [PORT=...]` | Build or flash one app. |
| `make hooks` | Install pre-commit and pre-push hooks. |

Per-app targets are listed in `docs/new-app.md`.

## Workflow

- `main` is protected: branch, open a PR, squash-merge after **Required
  Checks** passes. Hooks block commits on `main`. Never skip hooks with `--no-verify`.
- Conventional commit titles, `type(scope): what changed`; see Commit titles below.
  After each merge, `.github/workflows/release.yml` runs `tools/release.py`: every
  app with new `feat`/`fix` commits under its folder (or `platform/`) gets the
  next patch `<app>-vX.Y.Z` tag, a beta (pre-release) with generated notes and a
  flashable `factory.bin`. `promote.yml` rebuilds a beta as the next minor,
  stable: daily once a beta is 3 days old and not yanked (`yank.yml`), or by
  hand. No one merges anything to release.
- Shift left. The hooks run everything CI runs for what you changed; CI is a
  backstop that runs once per PR and only for touched apps. If CI catches
  something a hook missed, fix the hook, not just the code.
- Complexity is ratcheted. `whitelizard.txt` lists functions that were already
  over the thresholds when the gate was added. Don't add entries to it. When you
  touch a listed function, split it up and delete its line.
- Coverage thresholds live in each app's Makefile. Don't lower them. Raise them
  when coverage grows.
- Done means reviewed. Before calling any change done, marking a PR ready or
  merging it, run `/simplify` on the diff and apply its cleanups, then
  `/code-review` on the diff (or the PR); fix every real finding or say why it
  is skipped, then re-run `make check` (and `make ci` for anything
  user-visible). Green hooks and CI are not a review.
- Done means played. Before calling gameplay or UI work done, marking its PR
  ready or merging it, dispatch an independent `playtester` agent to play the
  final emulator build as its representative audience: a target-age child for
  every game and child-facing flow, a grown-up for adult-only settings. New
  games and major changes require a source-free first-play pass.
  Test curiosity, plausible wrong choices and imprecise fingers, and assess
  boredom, comprehension and playability as well as correctness. Iterate on
  mechanics, controls, player-facing instructions and the playtester's brief;
  fix findings and retest until the final affected experience is satisfactory.
  Record the persona/brief, build, findings, resolutions and screenshot/input
  evidence. Follow `apps/porthole/docs/playtesting.md` for scope and acceptance.
  An implementer's own run is useful but does not replace the independent pass.
  Documentation-only and non-player-visible changes do not require a new run.

## Commit titles

PRs are squash-merged, so the PR title is the commit on `main` and a line in the
release notes. Write it as `type(scope): what changed`, in words a player or a
grown-up would use ("Rally rivals vary every race", not "add rivals() shuffle").

**Type** says what kind of change it is, and it decides releases: a `feat`, `fix`
or `perf` commit that touches `apps/<app>/` or `platform/` cuts a new firmware
beta (`tools/release.py` picks commits by path, never by scope).

| Type | Use for |
| --- | --- |
| `feat` | Something new a player or grown-up can see or do: a game, a page, a setting, a mechanic. |
| `fix` | Behavior that was wrong and now is right, including balance a playtest proved off. |
| `perf` | Faster, smaller or lighter on the board, same behavior. Releases like `fix`. |
| `refactor` | Code restructured with no behavior change (complexity splits included). |
| `test` | Tests, playtest scripts and test harnesses only. |
| `docs` | Briefs, READMEs, specs, playtest write-ups. |
| `build` | Makefiles, PlatformIO config, generators and dev tooling that build or check code. |
| `ci` | `.github/` workflows and actions, the release scripts they run. |
| `chore` | Housekeeping that fits nothing above: dependency bumps, gitignore, renames. |
| `revert` | Undoing an earlier commit; name it in the title. |

So a test or doc change inside `apps/porthole/` is `test` or `docs`, not `feat`:
it must not cut a release.

**Scope** says where the change lands: the narrowest area that covers all of it,
named after its folder.

| Scope | Covers |
| --- | --- |
| a game's folder name: `pets-club`, `biscuit`, `marble-kick`, `tilt-fc`, `plane-radar`, `pumpkin-hide-seek`, `racing`, `starfighter`, `living-kingdom`, `toy-town-life` | One game under `apps/porthole/games/<game>/`, with its host tests, playtests and docs. A new game takes its folder name. |
| `shell` | `apps/porthole/shell/`: profiles, picker, launcher, rest and daily caps, the grown-ups' menu (Admin, Wifi, Updates, Clock). |
| `os` | `apps/porthole/os/`: the shared runtime every game uses (drawing, fonts, input, tilt, UI widgets, time zones). |
| `firmware` | `apps/porthole/firmware/`, `platformio.ini`, `partitions.csv`: board drivers, display, OTA updates, serial commands. |
| `sim` | The host simulator and web emulator (`host/sim.cpp`, `tools/webemu.py`), the playtest runner. |
| `porthole` | A change across several of the scopes above, where none of them owns it (e.g. a rule every game follows). |
| `platform` | `platform/`: the shared PlatformIO base and factory image. |
| `hardware` | `hardware/`: the printed case and grip models. |
| `site` | `site/`: the public website. |
| `release` | `tools/release.py` and the release, promote and yank workflows. |
| `deps` | Dependency bumps (with `chore`, `build` or `ci`). |

Leave the scope out only for a repo-wide change (root docs, hooks, the root
Makefile). One scope per title: if a change needs two, it is usually two PRs;
otherwise pick the area where most of the change lands. Examples:
`feat(racing): rivals vary every race`, `fix(shell): admin code waits survive a reboot`,
`perf(firmware): double the indexed frame rate`, `test(biscuit): content gate covers endings`,
`ci(release): yank a beta by hand`, `docs: commit title guidance`.

## Delegation

| Task | Agent | Model |
| --- | --- | --- |
| Mechanics, progression, tuning numbers | `game-designer` | opus |
| Implement a screen or mechanic | `game-engineer` | opus |
| Board bring-up, pins, PlatformIO, flashing | `firmware-engineer` | opus |
| Pixel art, icons, sprite poses | `pixel-artist` | opus |
| New stories, spelling words, or (Biscuit) discoveries | `story-writer` | sonnet |
| UX/QA pass on a screen or flow | `playtester` | opus |
| Review before a push | `reviewer` | opus |
