# porthole: agent brief

## What this is

A monorepo of small apps (so far, games) for one board: the Waveshare
ESP32-S3-Touch-LCD-2.1, a round 480x480 capacitive touchscreen with no physical
buttons. Each app lives in `apps/<name>/` and owns its code, content, tests and
docs. Build, test, release and flashing infrastructure is shared at the root.

| Path | What it holds |
| --- | --- |
| `apps/porthole/` | Porthole: the device firmware (shared runtime in `os/`, board layer in `firmware/`) and its games in `games/`. First game: Pets Club (`games/pets-club/`): pixel dog, up to three kids per device, tricks, stories, spelling. C++ core + host simulator + browser emulator. Its own brief: `apps/porthole/CLAUDE.md`. |
| `apps/biscuit/` | Biscuit: story dog with branching mysteries and illustrated discoveries. Browser game (plain JS) + LVGL firmware in `firmware/`. See `apps/biscuit/README.md` and `apps/biscuit/firmware/README.md`. |
| `platform/` | Shared PlatformIO base (`waveshare-round.ini`), the factory-image script, pinned PlatformIO requirements. |
| `site/`, `tools/build_site.py` | The web installer on GitHub Pages (`https://dreamiurg.net/porthole/`), built from each app's latest release and its `app.json`. ESP Web Tools does the flashing. |
| `tools/gallery.py` | Builds the README screenshot strips (`docs/preview.png`, `docs/screenshots.png`) every app ships. |
| `docs/` | `hardware.md` (board, pins, round-screen rules), `new-app.md` (the app contract). |
| `.claude/` | Agent roster and skills. They currently target Pets Club; paths inside them are `apps/porthole/...`. |

When you work inside an app, read that app's brief first. Its constraints (for
Pets Club: round-screen geometry, 32 colors and ASCII only, append-only save
layout, the pet never dies) are hard rules, not style.

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
- Conventional commit titles (`feat(porthole): ...`, `fix(biscuit): ...`).
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
| New stories or spelling words | `story-writer` | sonnet |
| UX/QA pass on a screen or flow | `playtester` | opus |
| Review before a push | `reviewer` | opus |

The roster was written for Pets Club. For Biscuit, brief the same roles with
Biscuit's paths and read its README first.
