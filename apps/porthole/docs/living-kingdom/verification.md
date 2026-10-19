# Chapter One v0.1 verification

Implementation branch: `feat/living-kingdom-chapter-one`, based on design commit
`48e1bbb9700190147eb06e9989dcf0ad77c7eb56`. Design PR #79 was squash-merged
as `96209b1`; implementation PR #81 now targets `main`. The user authorized
merging and releasing after the original review handoff.

The results below were recorded on 2026-10-19 against the frozen implementation
using the approved polished Chapter One artwork, reference-based generated
additions, textured panels and Atkinson typography. The ordinary host emulator
at `build/host/snap` has SHA-256:

`2bb76fc45af7902d4ed30629304380ba04f2aa15ecb1a8efd18c6003941b2d3b`

This is the integrated main build at `2116f91`. The earlier copy-corrected
Kingdom build was `780fd0ad…`; merging PR #80 changed only shared/admin code.

## Passed automated gates

- `make -C apps/porthole ci` completed with exit status 0. Content and art
  freshness checks, compiled tests, full playtests and coverage all passed with
  warnings treated as errors. All **67 of 67** playtest scenarios passed,
  including sanitized monkey runs, with zero UI warnings or failures. The
  coverage replay also passed all **58 of 58** non-monkey scenarios.
- Repository coverage is **98.6% of lines** (7,006 / 7,102) and **99.3% of
  functions** (1,102 / 1,110). Kingdom covers **99.6% of lines** (847 / 850)
  and **100% of functions** (134 / 134). Reports are generated at
  `build/playtest/report.md`, `build/coverage/playtest/report.md` and
  `build/coverage/coverage.xml`.
- Kingdom's engine suite executes **167,866 checks**, including all four
  complete outcomes across 128 seeds, item-free progress, scheduled effects,
  atomic choices, corrupt snapshots and profile isolation.
- The compiled manuscript contains **40 scenes, 7,590 narrative words,
  314 measured story pages and 83 choices**, plus authored choice results.
  Glyph support, complete paragraph pagination, journal headings and choice
  cards fit the actual compiled fonts. The illustrated opening has eight
  reader pages before its separate choice screen.
- `80_kingdom_reader` passed **76 checks** with zero UI findings. It covers
  full-scene reading, backwards navigation, rereading to the same choices,
  exactly one commitment, Party and Inventory overview/detail navigation,
  Journal, Save & quit, and immediate reboot to the exact saved scene and page.
  The separate focused automated report is
  `build/kingdom-board08-regression/report.md`.
- Dedicated UI and art suites also passed under AddressSanitizer and
  UndefinedBehaviorSanitizer. They check all 14 UI modes, circular text and
  touch bounds, unchanged campaign state during reading, exact resume,
  profile isolation and alternating framebuffer equality. Art checks cover
  source-derived RGB565 reconstruction, all 70 scene/map state combinations,
  declared physical sizes, title, camp, portraits, items, nine-slice texture
  identity and clipping, malformed streams and 5,000 deterministic fuzz cases.
- Versioned `make -C apps/porthole factory` built firmware **0.27.0** with
  the pinned Xtensa toolchain. RAM is **218,632 / 327,680 bytes (66.7%)**;
  flash is **8,288,010 / 8,323,072 bytes (99.6%)** after PR #80 integration.
  The actual padded OTA binary is **8,288,416 bytes**, leaving **34,656 bytes**
  in the app slot. Future expansion needs a flash-budget review. The binary is
  `apps/porthole/.pio/build/firmware/firmware.bin` from the repository root.

No test expectation, sanitizer gate, hook or coverage threshold was weakened.

## Repository gates and test resources

Normal pre-commit checks passed, including Semgrep, full-tree mypy, Ruff,
gitleaks, freshness checks and `make check`. `make complexity` passes. No hook
has been bypassed; publication uses the required clean-tree pre-push hooks.

The coverage runner initially timed out two existing shell scenarios under
parallel load. Both passed together in 11.3/11.7 seconds. The worker pool is
now capped at four; every scenario, assertion, sanitizer and original 30/120
second timeout remains unchanged. The subsequent full run passed 67/67 and
58/58. One push attempt was correctly refused because an evidence document
changed during its hook; its actual CI and firmware checks passed. The subsequent clean-tree push at `580cd22` passed every normal pre-push gate:
static checks on all files, complexity, full CI and firmware. No gate was bypassed.

## Manual evidence and scope

The [mill campaign](blackbox/mill-board08/README.md) and
[marsh campaign](blackbox/marsh-board08/README.md) were watched through full
endings and post-ending exploration on visual baseline `6f40297…`, with fresh
218/218 and 269/269 identical-frame replays. Four later copy edits clarify the
mill return destination and remove unearned Pippa attribution. They change no
IDs, effects, conditions, controls or artwork.

On the copy-corrected `780fd0ad…` binary, the
[clue/return-instruction pass](blackbox/marsh-board08/copy-addendum/README.md)
and [both seeded woods paths](blackbox/copy-woods/README.md) were watched with
Back/Next, save/quit, restart and abrupt-kill recovery. These use disclosed
legitimate prior-play fixtures, reproducible from fresh ordinary-input traces.
Their fresh narrow replays match 22/22, 17/17, 10/10 and 17/17 frames.
The full mill replay remains 218/218 identical; full marsh replay changes only
its expected corrected return instruction, inspected in the addendum.

The [source-free channel campaign](blackbox/channel-board08/README.md) also
completed its paid-work ending, post-ending deferred bridge and exact saved-map
recovery: 242/242 baseline frames replay identically. On the current build,
241 match and only the expected journal attribution differs, inspected and
linked in that report. The [evidence index](blackbox/README.md) separates these
full watched baselines from the narrow current-copy checks. Earlier rejected art
and font treatments remain explicitly historical. Engine simulations and
scripted regression fixtures do not replace the watched player runs.

The final [main integration pass](blackbox/main-integration/README.md) uses
the combined `2bb76fc4…` binary. It traverses launcher, reader, native adult
Info, Party/Inventory and exact page recovery after both forced kill and
Save & quit. Its fresh replay matches 23/23 frames. Full mill, marsh and channel
replays match all 218/218, 269/269 and 242/242 copy-corrected frames; main's
adult typography replay matches 51/51. Kingdom source and assets are unchanged.

[Implementation PR #81](https://github.com/dreamiurg/porthole/pull/81) originally
targeted the design branch, which did not trigger hosted CI. It now targets
`main`, with the newer PR #80 admin typography integrated. GitHub Required
Checks must pass before its squash merge. All four
actual PR screenshot embeds rendered at natural 480×480 on GitHub, with exact
remote PNG/capture hashes verified; see [the rendering record](pr81-image-verification.json).

Testing covers the host emulator and compiled board firmware; no physical
device was played. Any approximate 2.1-inch typography comparison is a monitor
approximation, not a hardware readability measurement.

Future chapters have a documented append-only identity/save contract and
reserved quest/NPC capacity, but their content is outside this version. A
future version must retain and migrate the frozen 935-byte v1 snapshot before
changing its schema. No earlier Kingdom save format was shipped.
