# Chapter One v0.1 verification

Implementation branch: `feat/living-kingdom-chapter-one`, based on design commit
`48e1bbb9700190147eb06e9989dcf0ad77c7eb56`. The intended PR base is
`design/living-kingdom-chapter-one`; do not merge automatically.

The results below were recorded on 2026-10-19 against the frozen implementation
using the approved polished Chapter One artwork, reference-based generated
additions, textured panels and Atkinson typography. The ordinary host emulator
at `build/host/snap` has SHA-256:

`780fd0add1892afa4af35e29a9d1acc266d5baf7798e6bcb0ab3a8c2bd4eb246`

## Passed automated gates

- `make -C apps/porthole ci` completed with exit status 0. Content and art
  freshness checks, compiled tests, full playtests and coverage all passed with
  warnings treated as errors. All **67 of 67** playtest scenarios passed,
  including sanitized monkey runs, with zero UI warnings or failures. The
  coverage replay also passed all **58 of 58** non-monkey scenarios.
- Repository coverage is **98.6% of lines** (6,980 / 7,076) and **99.3% of
  functions** (1,093 / 1,101). Kingdom covers **99.6% of lines** (847 / 850)
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
- `make -C apps/porthole firmware` built the ESP32-S3 release firmware with
  the pinned Xtensa toolchain. RAM is **218,632 / 327,680 bytes (66.7%)**;
  flash is **8,245,882 / 8,323,072 bytes (99.1%)**, leaving **77,190 bytes**
  in the app slot. The firmware binary is
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
changed during its hook; its actual CI and firmware checks passed. Reports are
committed before the clean-tree publication attempt.

## Manual evidence and scope

The [mill campaign](blackbox/mill-board08/README.md) and
[marsh campaign](blackbox/marsh-board08/README.md) were watched through full
endings and post-ending exploration on visual baseline `6f40297…`, with fresh
218/218 and 269/269 identical-frame replays. Four later copy edits clarify the
mill return destination and remove unearned Pippa attribution. They change no
IDs, effects, conditions, controls or artwork.

On the current `780fd0ad…` binary, the
[clue/return-instruction pass](blackbox/marsh-board08/copy-addendum/README.md)
and [both seeded woods paths](blackbox/copy-woods/README.md) were watched with
Back/Next, save/quit, restart and abrupt-kill recovery. These use disclosed
legitimate prior-play fixtures, reproducible from fresh ordinary-input traces.
Their fresh narrow replays match 22/22, 17/17, 10/10 and 17/17 frames.
The full mill replay remains 218/218 identical; full marsh replay changes only
its expected corrected return instruction, inspected in the addendum.

A separate source-free tester is completing the channel campaign; its report
will be linked in the [evidence index](blackbox/README.md). Earlier rejected art
and font treatments remain explicitly historical. Engine simulations and
scripted regression fixtures do not replace the watched player runs.

Testing covers the host emulator and compiled board firmware; no physical
device was played. Any approximate 2.1-inch typography comparison is a monitor
approximation, not a hardware readability measurement.

Future chapters have a documented append-only identity/save contract and
reserved quest/NPC capacity, but their content is outside this version. A
future version must retain and migrate the frozen 935-byte v1 snapshot before
changing its schema. No earlier Kingdom save format was shipped.
