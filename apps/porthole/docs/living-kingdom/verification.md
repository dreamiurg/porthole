# Chapter One v0.1 verification

Implementation branch: `feat/living-kingdom-chapter-one`, based on design commit
`48e1bbb9700190147eb06e9989dcf0ad77c7eb56`. The intended PR base is
`design/living-kingdom-chapter-one`; do not merge automatically.

The results below were recorded on 2026-10-19 against the frozen implementation
using the approved polished Chapter One artwork, reference-based generated
additions, textured panels and Atkinson typography. The ordinary host emulator
at `build/host/snap` has SHA-256:

`6f40297c36fccf7d067c7f37aeeeacf2f29f0fa6416d5cca19832efcf33e87ce`

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
  flash is **8,245,850 / 8,323,072 bytes (99.1%)**, leaving **77,222 bytes**
  in the app slot. The firmware binary is
  `apps/porthole/.pio/build/firmware/firmware.bin` from the repository root.

No test expectation, sanitizer gate, hook or coverage threshold was weakened.

## Pending final gates

The final repository pre-commit hooks and the ordinary-input manual review of
the frozen build are pending. This record does not yet certify those gates,
an implementation commit, or an opened implementation PR. No hooks have been
bypassed.

## Manual evidence and scope

**Final frozen-build ordinary-input report: pending.** Its report and
screenshots will be linked here when the watched final-build run is complete.
The [black-box evidence index](blackbox/README.md) currently preserves
historical campaigns and explicitly identifies their rejected, pre-revision
presentation. Those traces establish their recorded gameplay observations;
they do not certify the current artwork or replace final-build manual review.
Automated scenarios, engine simulations and staged fixtures remain separate
from ordinary-input player runs.

Testing covers the host emulator and compiled board firmware; no physical
device was played. Any approximate 2.1-inch typography comparison is a monitor
approximation, not a hardware readability measurement.

Future chapters have a documented append-only identity/save contract and
reserved quest/NPC capacity, but their content is outside this version. A
future version must retain and migrate the frozen 935-byte v1 snapshot before
changing its schema. No earlier Kingdom save format was shipped.
