# Princess Pulverizer: The Living Kingdom

`Kingdom` is a persistent story RPG for an advanced third-grade reader. Chapter
One, Trouble at Willowbrook, is a complete original fan-game manuscript. The
full design handoff is `docs/superpowers/specs/living-kingdom/` at the repo root.
The princess, Lucas and Dribble are familiar fan characters; local people,
prose and art are original. This remains an unofficial private prototype.

## Modules and invariants

- `engine.*`: pure requirements, deterministic encounter selection, transactional
  effects, choices, world days and delayed consequences. No renderer or shell
  dependency. Reading never changes facts, items, days or decisions.
- `save.h`: the fixed-width, pointer-free campaign snapshot (`Save`: how the
  game holds it, and version 1 as stored) and the stored version 2, the same
  fields bit-packed (`packSave`, in `engine.cpp`). UI cursors belong to the
  snapshot: scene, measured page, pending choice page, highlighted choice and
  overlay return cursor all survive interruption.
- `content/chapter-01.json`: prose and declarative world rules; runtime uses
  generated compact C++ tables through `content.*`. Never put story-specific
  prerequisites or rewards in rendering code.
  Human-readable identifiers use the `slug` authoring field; numeric `id`
  values and all existing identifier strings remain stable. The compiler also
  accepts legacy `key` fields when importing older authoring data.
- `tools/content.py`: compile and validate content. Run it after editing JSON;
  `--check` checks the committed tables and identity lock without changing files.
  `python3 tools/test_content.py` runs validator regression tests from this
  game's directory. Numeric references are checked against actual registered
  IDs; operators, effect types, status values and scheduled delays are bounded.
- `content/ids.json`: persisted identity lock. Registry IDs are immutable after
  release; append new entries. Explicit scene IDs are 1..127 and decision IDs
  1..511. Never reuse a retired ID. Removed prose must retain its old journal
  description so old choices remain readable.
- Choice positions within each scene are also persisted: `selectedChoice` is
  an array index in confirmation and result cursors. `choice_slots` in the
  identity lock freezes each scene's index-to-decision mapping. Append choices;
  never reorder or remove an existing slot, even when keeping its numeric ID.
- `game.*`, `reader.*`, `layout.h`: native RGB565 screens and sentence-aware
  pagination. All glyphs and hitboxes must fit the round glass. Font and art
  belong to Kingdom; do not borrow another game's visual identity.
- `art/`, `tools/kingdom_art.py`: use the original PNG concept artwork directly;
  board `08-chapter-one-polished-ux.png` is the selected style. The nine-screen
  SVG is only a schematic. Missing illustrations come from GPT Image Gen, with untouched
  sources and crop provenance; do not recreate them with code-drawn shapes.
  Cached lossless streams retain all source-derived RGB565 pixels. Firmware
  uses the shared `os/packed_image` ROM-miniz decoder; host builds require zlib.
  Games share its serial decoder workspace, never art assets or visual identity.
  See `art/README.md` for regeneration.
- Message text uses the exact PR #80 Atkinson Hyperlegible font for readability;
  decorative headings use Willowbook Serif. Keep their bundled licences and
  checksum-verified sources, and measure pagination with the generated faces.

## Player contract

Start or Continue opens the same scene and page. The reader permits backwards
and forwards movement; the choice screen can reread from page one, then return
to the same options. A confirmation applies exactly one choice atomically and
opens its saved result. `finishResult` follows the data-defined next scene or
returns to the map. Committing again, reloading the result, or visiting a
completed one-off cannot award anything again. Menus restore their complete
return cursor. Exit and shell rest keep the exact campaign cursor.

Travel to a different location advances one authored world day. Visiting the
current location is free; Rest advances one day. There is no real-clock decay,
absence penalty, combat loss or random failure. Campaign initialization fixes
four seeded variations. Eligible scenes prefer major story priorities, then a
deterministic seed/day/scene tiebreak; selecting or reading cannot reroll one.

Six inventory slots hold quantities of useful items. Necessary knowledge lives
in facts, independently of inventory. Capacity or missing supplies disable an
option with an explanation; effects are applied to a copy and either all
commit or none do. Skills are Courage, Ingenuity and Insight; relations are
per-NPC memory, never a morality score. Quest states are undiscovered,
available, active, blocked, deferred and resolved. Promise states include open,
kept and broken, with original creation day retained.

Four council outcomes have distinct costs and delayed events. Each event fires
once after its scheduled world day, updates world facts and the dated journal,
and unlocks actual follow-through. Chapter completion requires follow-through
and a recognized deed. Optional bridge/supply objectives remain in the journal
after the finale; do not silently mark everything resolved.

## Save evolution

The NVS namespace is `kingdom` and profile keys are shell-owned `s0`..`s3`.
`Save` begins with magic, version and exact byte length and ends with CRC32.
The CRC covers all preceding bytes. Loading verifies length, version, checksum,
profile identity and bounded fields before replacing the current state.
Writes serialize a complete snapshot, including committed choice identity.

The stored blob is version 2: magic, version, its own byte length, the fields of
`Save` bit-packed (`os/bits.h`), CRC32 over all of it. About 110 bytes for a new
campaign and under 300 for a finished chapter, where version 1 was always 935:
four kids' saves of every game share one 20 KB NVS partition. `packWorld` and
`packStory` in `engine.cpp` are the layout, one description for writing and
reading: a field `validState` bounds takes the bits its range needs, any other
is kept whole; days and journal ids take the width of the widest one stored;
only the journal's used entries are kept; `seen` and `decisions` are stored
XORed with what `lastVisit` and the journal imply (`fold`), which is all zeros
in a real campaign and still exact in any other. Every state `validState`
accepts packs and reads back byte for byte (`anyValidStatePacks`), and
`Game::takeSave` checks exactly that on every save before the packed blob may
replace the stored one; a blob that did not read back equal is not written,
the whole struct (version 1) is.

Version 1 is the `Save` struct itself, 935 bytes, and is still what the game
holds in memory. Never reorder, resize or change the meaning of its fields. A
stored version 1 loads as before and is written back as version 2 at the next
save, in place under the same key (NVS keeps the old blob if that write fails,
and the shell retries). `host/fixtures/kingdom_save_v1.h` is a real one;
`storedVersions` and `versionOneMigrates` hold every field of it through the
migration. A state that cannot be packed (the engine makes none) is stored as
version 1, whole.

A future field is appended to the struct immediately before CRC (a new struct
version, with the old struct frozen) and at the end of the packed layout under
a new stored version; the loader then reads a version 2 blob with that field
zero. Test each supported predecessor, initialize newly appended fields
explicitly, and preserve original IDs, seed and decision log. Unknown versions
and incomplete/corrupt writes are rejected, never interpreted as a valid old
snapshot. Do not rename the namespace. `reserved` bytes have no meaning and
must remain zero (version 2 does not store them).

Current bounds are 64 facts, 128 scene IDs, 512 decision IDs, 16 item types,
16 NPCs, 16 promises, 8 delayed events and 96 dated journal records. There are
six slots, three skills and 16 reserved quest tracks (four active in Chapter One). Major decisions additionally
remain in permanent bitsets even if a future chapter needs journal summaries.

## Verification

`host/test_kingdom.cpp` covers transactional decisions, exact snapshot/profile
validation, deterministic variations, route completion, delayed events,
requirements, rewards and revisits. The UI tests cover full reader geometry and
resume. Add failing rule tests before changing mechanics. Run `make check`,
`make ci` and a firmware build when available. Every gameplay/UI change also
requires ordinary-input final-build emulator playthroughs and screenshot review
under `apps/porthole/docs/playtesting.md`; scripts and state bots supplement
that gate. Never claim physical-device testing from emulator screenshots.
