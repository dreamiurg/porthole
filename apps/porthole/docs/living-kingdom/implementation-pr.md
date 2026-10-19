# Princess Pulverizer: playable Chapter One v0.1

Adds **Kingdom**, a complete persistent story RPG, from the Chapter One design
handoff. The expanded manuscript has 40 scenes, 7,590 narrative words, 314
measured pages and 83 choices. Six locations, companions, three skills,
six-slot inventory, quests, promises and scheduled consequences support
mill-first, marsh-first, timed-sharing and costly second-channel resolutions.
Each ending requires actual follow-through; optional work and exploration stay
available afterward.

The reader supports Back/Next and rereading before committing a decision once.
A CRC-protected per-profile save retains the exact page, pending choices,
world seed and tool return position across quitting, crashes and parent breaks.
Stable content IDs, declarative effects and an append-only save contract support
future chapters. Shell changes are limited to registering the game, increasing
blob capacity and allowing urgent Kingdom page/decision checkpoints.

Uses actual original concept-art pixels, following the user's selected **last
board, 08 — Polished UX**. Missing scenes and states use GPT-generated artwork
referenced to that board. Original design PNGs stay unchanged. Text uses the
exact PR #80 **Atkinson Hyperlegible** font, with serif headings, textured
parchment, indigo framing, gold actions and violet controls.
[Source versus implementation comparisons](https://github.com/dreamiurg/porthole/blob/feat/living-kingdom-chapter-one/apps/porthole/docs/living-kingdom/visual-comparison/README.md)
show the actual result and document layout/source-resolution differences.

Validation:

- Full `make ci REQUIRE_NODE=1`: **67/67 scenarios pass**, including sanitizer
  runs; coverage replay **58/58**. Repository coverage **98.6% lines / 99.3%
  functions**; Kingdom **99.6% / 100%**.
- Engine: **167,866 checks**, including all four outcomes across 128 seeds,
  atomic decisions, item-free progress, delayed effects, corrupt saves and
  profile isolation. Font/content/art freshness, geometry, decompression fuzz
  tests and all 14 UI modes pass.
- Watched mill, marsh and source-free independent channel campaigns completed with reading,
  rereading, rest breaks, exact resume, consequences, endings and post-ending
  play. Fresh baseline replays matched **218/218**, **269/269** and **242/242**
  frames. Four final copy fixes received narrow watched checks; current full
  replays retain all mill frames and change only the expected marsh return
  instruction and channel journal note.
- Normal pre-commit and clean-tree pre-push hooks pass: static checks,
  complexity, full CI and firmware. Code review found no remaining issues. [Detailed verification](https://github.com/dreamiurg/porthole/blob/feat/living-kingdom-chapter-one/apps/porthole/docs/living-kingdom/verification.md)
  and [manual evidence](https://github.com/dreamiurg/porthole/blob/feat/living-kingdom-chapter-one/apps/porthole/docs/living-kingdom/blackbox/README.md)
  record the precise scope.
- Integrated firmware **0.27.0** and factory build pass: **218,632 bytes RAM
  (66.7%)**, **8,288,010 bytes flash (99.6%)**. The padded OTA image leaves
  **34,656 bytes** in the app slot. Future content/art
  expansion needs a flash-budget review. No physical-device test is claimed.

Actual 2.1-inch reading comfort still needs hardware playtesting; the supplied
201px comparison is only a monitor approximation.

Try it with `make -C apps/porthole webemu`, then select a profile and open
**Kingdom** on the third launcher page.

Targets `main` after design PR #79 was merged. Includes the newer PR #80 admin
typography. The user authorized merging and cutting the next firmware release;
GitHub Required Checks must pass before squash merge.

The [final main integration check](https://github.com/dreamiurg/porthole/blob/feat/living-kingdom-chapter-one/apps/porthole/docs/living-kingdom/blackbox/main-integration/README.md)
adds a watched launcher/reader/tools/crash/save/restart pass (23/23 replay
frames). All three full campaign replays match the copy-corrected frames
(218/218, 269/269, 242/242); main's adult typography matches 51/51.

![Original-art title](https://github.com/dreamiurg/porthole/blob/feat/living-kingdom-chapter-one/apps/porthole/docs/living-kingdom/blackbox/mill-board08/08-title.png?raw=true)
![Readable illustrated story](https://github.com/dreamiurg/porthole/blob/feat/living-kingdom-chapter-one/apps/porthole/docs/living-kingdom/blackbox/mill-board08/20-road-first.png?raw=true)
![Original portraits and RPG skills](https://github.com/dreamiurg/porthole/blob/feat/living-kingdom-chapter-one/apps/porthole/docs/living-kingdom/blackbox/mill-board08/10-party.png?raw=true)
![Persistent completed world](https://github.com/dreamiurg/porthole/blob/feat/living-kingdom-chapter-one/apps/porthole/docs/living-kingdom/blackbox/mill-board08/217-ending-exact-resume.png?raw=true)
