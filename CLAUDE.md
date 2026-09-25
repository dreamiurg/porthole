# porthole: agent brief

## What this is

A monorepo of small apps (so far, games) for one board: the Waveshare
ESP32-S3-Touch-LCD-2.1, a round 480x480 capacitive touchscreen with no physical
buttons. Each app lives in `apps/<name>/` and owns its code, content, tests and
docs. Build, test, release and flashing infrastructure is shared at the root.

| Path | What it holds |
| --- | --- |
| `apps/porthole/` | Porthole: the device firmware (shared runtime in `os/`, board layer in `firmware/`) and its games in `games/`. Profiles (up to four kids) and a launcher in `shell/`. First game: Pets Club (`games/pets-club/`): pixel dog, tricks, stories, spelling. C++ core + host simulator + browser emulator. A second game, Biscuit (`games/biscuit/`, landing with its own PRs -- design: `docs/superpowers/specs/2026-09-25-biscuit-on-porthole-design.md`), is a warm reading-and-discovery companion drawn full-color at native resolution on the same runtime. App brief: `apps/porthole/CLAUDE.md`; each game also has its own `CLAUDE.md` under `games/<game>/`. |
| `platform/` | Shared PlatformIO base (`waveshare-round.ini`), the factory-image script, pinned PlatformIO requirements. |
| `tools/gallery.py` | Builds the README screenshot strips (`docs/preview.png`, `docs/screenshots.png`) every app ships. |
| `docs/` | `hardware.md` (board, pins, round-screen rules), `new-app.md` (the app contract), `superpowers/specs/` (approved design docs). |
| `.claude/` | Agent roster and skills, generalized across every game under `apps/porthole/games/<game>/`; paths inside them are `apps/porthole/...`. |

When you work inside an app, read that app's brief first, then the specific
game's brief if you're touching one. Their constraints (round-screen geometry,
a flat/limited color and font set, append-only save layout, nothing in the
game ever punishes the kid for being away) are hard rules, not style.

## Shared rules for every app

1. **Round screen, no buttons.** Everything tappable and every glyph must sit
   inside the circle. Every screen needs a visible way back. See `docs/hardware.md`.
2. **The buzzer is harsh.** Never beep on routine taps or navigation. Keep sound
   for rare moments and give every app a mute toggle.
3. **Saves survive firmware updates.** Never reorder or resize a persisted
   struct, and never rename an NVS namespace that shipped. Pets Club keeps the
   namespace `"crago"` for exactly this reason.
4. **Kids 5-11 are the audience.** Failure is gentle, nothing dies, nothing is
   lost. No personal data: no real names, no analytics, no network calls during play.
5. **Verify on the host.** The board is often unplugged. That does not exempt a
   change from verification: run the app's `make check`, and `make ci` for
   anything user-visible.
6. **Warnings are errors.** Every hook and CI job fails on any warning: `-Werror`
   on host and firmware builds (firmware: our sources via `build_src_flags`),
   biome `--error-on-warnings`, Python `-W error`, and `tools/fail-on-warning.sh`
   for tools with no fail switch. Fix the cause; never silence or skip it.

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
- Conventional commit titles (`feat(porthole): ...`, `fix(porthole): ...`).
  After each merge, `.github/workflows/release.yml` runs `tools/release.py`: every
  app with new `feat`/`fix` commits under its folder (or `platform/`) gets a
  `<app>-vX.Y.Z` tag, a GitHub release with generated notes and a flashable
  `factory.bin`. No one merges anything to release.
- Shift left. The hooks run everything CI runs for what you changed; CI is a
  backstop that runs once per PR and only for touched apps. If CI catches
  something a hook missed, fix the hook, not just the code.
- Complexity is ratcheted. `whitelizard.txt` lists functions that were already
  over the thresholds when the gate was added. Don't add entries to it. When you
  touch a listed function, split it up and delete its line.
- Coverage thresholds live in each app's Makefile. Don't lower them. Raise them
  when coverage grows.

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
