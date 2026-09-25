# Adding an app

An app is a directory under `apps/` with its own `Makefile`, `platformio.ini`
and `README.md`. A new game for the Porthole firmware is not an app: it is a
directory under `apps/porthole/games/`, built into the same image and added to
`APPS[]` (see `apps/porthole/CLAUDE.md`). The root `Makefile`, the pre-commit hooks and CI find apps by
their Makefile, so a new app only has to follow this contract.

## Makefile targets

Run from the repository root as `make -C apps/<app> <target>`.

| Target | Must do | Runs in |
| --- | --- | --- |
| `lint` | App-specific static checks. Ruff, shellcheck, actionlint, gitleaks and whitespace fixes already run repo-wide. | `check` |
| `test` | Fast unit or host tests, well under a minute. | `check` |
| `check` | `lint` + `test`. | pre-commit, when the app's files change |
| `coverage` | Tests with coverage. Fail below the app's threshold. Write `build/coverage/coverage.xml` (Cobertura) or `build/coverage/lcov.info`. | `ci` |
| `ci` | Everything CI runs for the app except the firmware build. | pre-push, CI |
| `firmware` | `pio run -e firmware`. | CI |
| `flash` | Build and upload; honor an optional `PORT=`. | by hand |
| `factory` | `pio run -e firmware -t factory`: a merged image to write at `0x0`, plus the separate flash parts. | release workflow |

## PlatformIO

Extend the shared board section instead of repeating it:

```ini
[platformio]
extra_configs = ../../platform/waveshare-round.ini

[env:firmware]
extends = waveshare_round
build_flags = ${waveshare_round.build_flags}
build_src_flags = ${waveshare_round.src_warnings}
extra_scripts = post:../../platform/factory_image.py
```

Warnings are errors: `src_warnings` is `-Wall -Wextra -Werror` for your own
sources (framework and library code are untouched). Host builds and tests use
`-Werror` too, and `lint` must pass with zero warnings.

Paths are relative to the PlatformIO project directory. If the project lives in
a subdirectory (say `apps/<app>/firmware`), add another `../`.

## Screenshots

Every app ships two images in `apps/<app>/docs/`, built from 480x480 panel
captures (host simulator snapshots, device framebuffer dumps, or a browser
capture of the round screen):

```sh
python3 tools/gallery.py --cols 4 apps/<app>/docs/preview.png     home.png a.png b.png c.png
python3 tools/gallery.py --cols 3 apps/<app>/docs/screenshots.png home.png a.png b.png c.png d.png e.png
```

`preview.png` is the four-tile strip in the root README's catalog.
`screenshots.png` is the six-tile gallery at the top of the app's own README.
Add `--pixel` for pixel art so it stays crisp. Re-shoot them when the screens change.

## Wiring it in

1. In `.github/workflows/ci.yml`, add the app's two change-detection lines to
   the `static` job (app and firmware paths), a job for the app copied from an
   existing one, and the job to `needs:` of `Required Checks`.
2. Add `<app>-check`, `<app>-ci` and `<app>-firmware` hooks to
   `.pre-commit-config.yaml`, copied from an existing app.
3. Releases need no wiring: `tools/release.py` finds every `apps/*/Makefile`.
   The app's first `feat:` or `fix:` after merge releases `<app>-v0.1.0`.
4. In the root `README.md`, add a row to the app table and a section under
   **Apps**: a `### Name` heading, the linked `preview.png`, one paragraph on what
   it is, one on how it is built, and a "Read more" link plus the one command that
   runs it without a board.
