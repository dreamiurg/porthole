# Contributing

Fork it, play with it, send changes back. Here is how the repo expects work to land.

## Setup

You need Python 3.11+, Node 22, a C++ compiler (clang or gcc) and
[pre-commit](https://pre-commit.com). Then:

```sh
pip install -r platform/requirements.txt   # PlatformIO, only needed for firmware
make hooks                                 # install the pre-commit and pre-push hooks
make check                                 # lint + fast tests for every app
```

## Workflow

- `main` is protected. Work on a branch and open a pull request. The hooks refuse
  commits made directly on `main`.
- The pre-commit hook runs formatters, secret scanning, SAST and the changed
  app's `make check`. The pre-push hook runs everything CI runs: every
  pre-commit hook again on every file (CI's static job), the complexity gate,
  the changed app's `make ci` (playtests and coverage) and its firmware build.
  If a hook fails, fix the cause rather than skipping the hook.
- Pushing needs PlatformIO (`pip install -r platform/requirements.txt`) and
  Node 22+ (for Biscuit's art check), as CI does. The hooks bring their own
  Python packages. By the time you push, you have run everything CI will.
- CI is a backstop. It runs once per PR, cheapest checks first, and only for the
  apps the PR touches. One aggregate check, **Required Checks**, must pass
  before merging. If CI ever catches something your hooks didn't, that's a bug in
  the hooks: fix them too.
- PRs are squash-merged, so the PR title becomes the commit message. Use
  [Conventional Commits](https://www.conventionalcommits.org): `feat:`, `fix:`,
  `docs:`, `chore:`, `refactor:`, `test:`, `ci:`. Scope by app when it helps:
  `feat(porthole): add a digging trick`. Releases are automatic: after a merge,
  any app with a new `feat:` (minor) or `fix:` (patch) gets a new version, release
  notes built from these titles, and a flashable image.

## Where to start

- Each app's `README.md` explains how to play and build it. Its `CLAUDE.md`
  lists the constraints its code must respect.
- `docs/hardware.md` covers the board. `docs/new-app.md` explains how to add an app.
- No board yet? Porthole runs on a computer: a host simulator and a browser
  emulator (`make -C apps/porthole webemu`).
