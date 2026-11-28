# Build, test and make your own app

## Setup

You need Python 3.11+, Node 22, a C++23 compiler (GCC 11 or Clang 12 and newer, for `-std=c++2b`), and PlatformIO for firmware. Host builds also need zlib
(`zlib1g-dev` on Debian/Ubuntu, included in the macOS SDK).

```sh
git clone https://github.com/dreamiurg/porthole.git
cd porthole
pip install -r platform/requirements.txt          # PlatformIO Core and pinned builder deps
pip install pillow fonttools==4.61.1 'gcovr>=8.3'  # art and font checks, coverage
make hooks                                        # the CI checks, run on commit and push
make check                                        # lint and tests for every app
```

No board? `make -C apps/porthole webemu` runs the firmware in your browser, and `make -C apps/porthole sim`
opens it in a window (needs SDL2). [apps/porthole/README.md](../apps/porthole/README.md) has the details.

## Flash from source

Connect the board over USB-C. PlatformIO finds the port when you connect one board.

`make flash` first copies the board's saves to `~/porthole-nvs-backup/<mac>/`. It then stops if your checkout is older than
the release the board last ran or downloaded, because older firmware cannot read newer saves. Rebase on `main`, or
set `ALLOW_OLDER=1` if the board holds nothing you need.

```sh
make flash APP=porthole
make flash APP=porthole PORT=/dev/ttyUSB0
make monitor
```

## Make your own app

Copy the shape of an existing app into `apps/<your-app>/`. An app needs:

- a `Makefile` with the shared targets (`lint`, `test`, `check`, `coverage`, `ci`, `firmware`, `flash`,
  `factory`),
- a PlatformIO project that extends the shared board config,
- a README,
- two screenshot strips made with `tools/gallery.py`.

The full checklist is in [new-app.md](new-app.md).

What you get:

- **Pinned board config.** [`platform/waveshare-round.ini`](../platform/waveshare-round.ini) sets the
  PlatformIO platform, PSRAM and flash. [`platform/requirements.txt`](../platform/requirements.txt) pins
  PlatformIO.
- **The same commands for each app:** `make check`, `make ci`, `make firmware APP=...` and
  `make flash APP=...`.
- **Early checks.** Pre-commit hooks check formatting, secrets, SAST, mypy and the changed app's tests.
  Pre-push adds the complexity limit, playtests, coverage and the firmware build. CI runs the same checks
  on each PR, only for the apps it changes, and the full suite on each push to `main`.
- **Automatic releases.** Each merged `feat:` or `fix:` ships a beta; see [releases.md](releases.md).

## Layout

```
apps/<app>/         one complete project per directory
platform/           shared PlatformIO base, pinned requirements, factory-image script
hardware/           the 3D-printed case and the steering-wheel grip
tools/gallery.py    README screenshot strips from panel captures
docs/               hardware, install, releases, this page, new-app.md (the app contract)
.github/            CI, releases, Pages, Dependabot
site/               the project page and web demo at porthole.dreamiurg.net
.claude/            agents and skills for AI-assisted work (see AGENTS.md)
```

Contributing: [CONTRIBUTING.md](../CONTRIBUTING.md).
