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
- Watched final-build mill and marsh campaigns completed with reading,
  rereading, rest breaks, exact resume, consequences, endings and post-ending
  play. Fresh replay matched **218/218** and **269/269** frames respectively.
  A source-free independent channel campaign is in progress.
- Normal repository hooks and final `/code-review` are being completed before
  marking this PR ready. [Detailed verification](https://github.com/dreamiurg/porthole/blob/feat/living-kingdom-chapter-one/apps/porthole/docs/living-kingdom/verification.md)
  and [manual evidence](https://github.com/dreamiurg/porthole/blob/feat/living-kingdom-chapter-one/apps/porthole/docs/living-kingdom/blackbox/README.md)
  record the precise scope.
- ESP32-S3 firmware builds: **218,632 bytes RAM (66.7%)**, **8,245,850 bytes
  flash (99.1%)**, leaving **77,222 bytes** in the app slot. Future content/art
  expansion needs a flash-budget review. No physical-device test is claimed.

Minor known polish: the marsh route's return instruction could explicitly name
the mill. It remains discoverable and the full route completed naturally.
Actual 2.1-inch reading comfort still needs hardware playtesting; the supplied
201px comparison is only a monitor approximation.

Targets `design/living-kingdom-chapter-one`. **Do not merge automatically.**

![Original-art title](https://github.com/dreamiurg/porthole/blob/feat/living-kingdom-chapter-one/apps/porthole/docs/living-kingdom/blackbox/mill-board08/08-title.png?raw=true)
![Readable illustrated story](https://github.com/dreamiurg/porthole/blob/feat/living-kingdom-chapter-one/apps/porthole/docs/living-kingdom/blackbox/mill-board08/20-road-first.png?raw=true)
![Original portraits and RPG skills](https://github.com/dreamiurg/porthole/blob/feat/living-kingdom-chapter-one/apps/porthole/docs/living-kingdom/blackbox/mill-board08/10-party.png?raw=true)
![Persistent completed world](https://github.com/dreamiurg/porthole/blob/feat/living-kingdom-chapter-one/apps/porthole/docs/living-kingdom/blackbox/mill-board08/217-ending-exact-resume.png?raw=true)
