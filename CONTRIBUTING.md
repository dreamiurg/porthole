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
  app's `make check`. The pre-push hook adds the complexity gate and the app's
  `make ci`, which covers playtests and coverage. If a hook fails, fix the cause
  rather than skipping the hook.
- CI runs the same gates plus both firmware builds. One aggregate check,
  **Required Checks**, must pass before merging.
- PRs are squash-merged, so the PR title becomes the commit message. Use
  [Conventional Commits](https://www.conventionalcommits.org): `feat:`, `fix:`,
  `docs:`, `chore:`, `refactor:`, `test:`, `ci:`. Scope by app when it helps:
  `feat(paw-street): add a digging trick`. Release notes and version bumps are
  generated from these titles.

## Where to start

- Each app's `README.md` explains how to play and build it. Its `CLAUDE.md`
  lists the constraints its code must respect.
- `docs/hardware.md` covers the board. `docs/new-app.md` explains how to add an app.
- No board yet? Both apps run on a computer. Paw Street has a browser emulator
  and Biscuit started as a web page.
