# Biscuit: game brief

**Part of this does not exist in code yet.** Biscuit is being rewritten onto the Porthole
runtime, PR by PR. Landed so far: content and its generated art (#33), and the rules,
save and legacy migration (#35, `pet.h`). Still to come: the RGB565 surface itself, and
the screens (`game.h`/`game.cpp`) that put the two together into something playable. This
file describes the intended design for what isn't real yet so every agent building toward
it works from the same contract. The full plan, with rationale and delivery order, is
`docs/superpowers/specs/2026-09-25-biscuit-on-porthole-design.md` -- read it before
implementing anything referenced here as "landing with the Biscuit PRs" (or a specific
PR number, where known). Once a PR lands, update the paragraph it makes real and drop the
qualifier for that part.

Biscuit previously shipped as a standalone LVGL firmware (source still at
`~/src/dreamiurg/zoegotchi`, and in porthole history as `biscuit-v0.1.0`); it was removed
from the monorepo in PR #24 because it shared nothing with the runtime. This is that
rewrite: same look and content, sharing Porthole's shell, sim, playtests, build, CI,
release and installer.

The shared app brief (`apps/porthole/CLAUDE.md`) covers what every game shares. This file
has Biscuit's product contract, its module map, and its content limits.

## Product contract

Biscuit is a companion, not a collection of mini-games. A new feature should strengthen
care, shared moments, curiosity, or Biscuit's visible growth -- not add an isolated menu
activity.

- **No death, no guilt, no streaks, no countdowns, no lost progress.** No pet death, no
  guilt for time away, no expiring friendship, no attendance streaks, no countdown
  challenges inside reading or training. Care stats decay toward floors and stop, the same
  pattern as Pets Club's `pet::FLOOR_*` constants -- see the shared app brief's constraint
  5. A wrong answer or a miss shows the right thing and moves on; it never blocks progress
  or resets state.
- **Warm, doglike voice.** Biscuit speaks in short, natural sentences, never like an
  investigator, teacher, mission controller, or corporate assistant. See the `content`
  skill's Biscuit section for the full voice guide with good/bad examples -- this brief
  states the principle, not the how.
- **Sourced facts, with a claim ledger.** Every discovery's factual claim traces to an
  authoritative source that supports that specific claim, not just its general topic. See
  the `content` skill's Biscuit section for the ledger format and the review workflow --
  this brief states the principle, not the how.
- **Stable ids, stable array order.** Story and discovery ids and their array order are
  persisted -- a save encodes progress by index. Never reorder or resize an existing entry;
  append, and add explicit migration coverage for anything that isn't a pure append.
- **Personalization, not a hardcoded name.** The companion is for whichever profile is
  playing. Content copy (in fields the game personalizes) uses two tokens that
  `personalize()` (`personalize.h`/`.cpp`, landed with #33) fills at render time: `{name}`
  for the child ("friend" when the profile has none) and `{pet}` for the dog ("Biscuit"
  until it is named). Never hardcode a specific child's name into game text, a test
  fixture, or a migration default.

## Module map

Everything lives under `namespace biscuit` -- Pets Club owns the global `Game` class and
`namespace pet`, and two `pet.h` headers in one link would collide.

Landed:

- `pet.h` (#35): the rules -- decay to the `NEED_FLOOR` (20), the 8-hour `ELAPSED_CAP_SEC`
  elapsed cap, stages (`Puppy`/`YoungPup`/`StoryDog`), the 7-day daily-adventure and 12-day
  sticker cycles, `TRICK_UNLOCK_DAY` and lessons, `valid()`. Header-only (no `pet.cpp`) so
  the shell's migration can use it without a second `.o` colliding with Pets Club's in the
  coverage build. `namespace biscuit` throughout -- Pets Club owns the global `Save` and
  `namespace pet`.
  - Save: 96 bytes (`SAVE_VERSION = 1`, magic `0x54435342` "BSCT"), NVS namespace
    `"biscuit"` (`STORE`, never renamed once it ships), key `s<profile id>`. Fields:
    `magic, version, size` header; `createdAt, updatedAt, lastVisitDay,
    fullness/happiness/energy, friendship, daysTogether, careCounts[3], discoveries[3],
    stickers, stories, tricks[6], dailyCompleted, dailyClaimed, sleeping, petName[13],
    named, reserved[2]`; `crc` last -- append-only like every other game (shared app brief
    constraint 4). The child's name and the daily play budget live in the shell's profile,
    not here. `named` gates everything: nothing decays, no visit counts, no progress until
    the pup has a name.
  - `legacy::fromLegacy(data, n, out, player)`: converts the old standalone firmware's save
    (three frozen generations, magic `0x5a4f4531/2/3`, FNV-1a checksum over the payload;
    v1 had no names, v2 added kid/pup names, v3 added a play-seconds field Porthole doesn't
    use) into a `Save`, plus the kid's name it held. The old firmware's clock was UTC with
    a hardcoded Pacific-time day boundary; conversion applies a fixed 8-hour offset
    (`ponytail:` a real zone table would be needed if a device ever migrates from
    elsewhere -- not needed today, since every known board is Pacific).
- `shell::migrateBiscuit` (`shell/migrate.cpp`, #35): reads whichever of
  `zoegotchi/pet3|pet2|pet1` or `biscuit/pet1` (from `biscuit-v0.1.0`) is newer, converts
  with `fromLegacy`, and writes it to the profile named like the saved kid (first 8
  letters, case-insensitive), else the first profile, else a new one; erases only the
  winning source's keys, and only after the marker `porthole/mb`. **Not wired yet**: it is
  not called from `loadAll` until Biscuit joins `APPS` (`TODO(biscuit screens PR)` in
  `shell/profiles.h`) -- so no unattended release moves a save that nothing reads yet, and
  profile delete or serial `R` can't erase it prematurely.
- `host/test_biscuit.cpp` (shared `host/` directory, #35): `pet.h`'s rules self-check --
  distinct from `host/test_biscuit_content.cpp` (content/art plumbing, #33).
- Content: `content_stories.h` (7 branching stories, `Story{id, title, subtitle,
  unlockDay, pages[10], prompt, choices[2]{label, ending[4]}}`), `content_discoveries.h`
  (96 `Discovery{id, topic, title, pages[2], wonder, sourceName, sourceUrl}` across 12
  topics), `content_daily.h` (7 daily adventures, 6 tricks with 3 cue-pattern lessons
  each, 12 sticker names) -- `namespace biscuit`, `inline constexpr`, converted once from
  the legacy JS with ids and array order unchanged. See the `content` skill for the full
  authoring workflow and its gate.
- `personalize.h` / `personalize.cpp`: fills the `{name}`/`{pet}` tokens (see Product
  contract above) into a caller-supplied buffer.
- Generated art, never hand-edited: `generated/scenes.h` (`SCENES[stage][time][activity]
  [frame]`, RLE RGB565), `generated/discovery_art.h` (`enum DiscoveryId` plus 96
  `Image565`), the pixel data itself in `generated/art_data.inc`, and
  `generated/manifest.json` (a SHA-256 per scene/picture, all matching the legacy
  firmware's). `art.cpp` is the one translation unit that includes `art_data.inc`. See the
  `art` skill for the generator and its `--check`.
- `tools/check_content.py`: the content gate -- glyph set, token validity, required
  fields, fixed counts, unique ids, discovery order vs. `DiscoveryId`, and a per-field
  character-count ceiling that stands in for a real pixel-fit check until `generated/
  fonts.h` exists. See the `content` skill for exactly what it checks.
- `host/test_biscuit_content.cpp` (shared `host/` directory, not inside this game's own):
  `personalize()` and the generated art tables. Wording, glyphs and id order are the
  content gate's job, not this test's.

Still landing:

- `game.h` / `game.cpp` (`class Game : public App`, in `namespace biscuit`): App plumbing,
  `enter` (decodes its own profile's blob only), state, command dispatch. `surface()` will
  return `SURFACE_RGB565` everywhere except the naming screens, which reuse the shell's
  indexed `ui::keyboard` rather than porting a second keyboard. `store()` will return
  `"biscuit"`; that string is never renamed once it ships. `soundOn()` always false --
  Biscuit has no buzzer sounds, the old firmware never drove it and the buzzer is harsh
  (shared app brief constraint 3).
- Screens, split by file to stay under the complexity gate: `screens_home.cpp` (Home with
  scene, needs, speech bubble, hotspots, Feed/Read/Play/More, fetch ball; the World view),
  `screens_read.cpp` (Library, Story, Choice, Ending), `screens_learn.cpp` (Discoveries,
  Topics, Discovery cover/pages/wonder, Source, Today, Word), `screens_train.cpp` (Tricks,
  Training watch/do), `screens_profile.cpp` (Profile, Stickers, SetupPet, RenamePet). 17
  views total. Screens the old standalone firmware had that Porthole's shell now owns
  instead: Settings, Clock, ResetConfirm, SetupChild, the idle cover, and Rest -- profiles,
  backlight dimming, the serial clock command, profile delete, and the shell's rest screen
  cover these.
- `ui565.h` / `ui565.cpp`: plain-function UI helpers on the RGB565 surface (`button`,
  `iconButton`, `top`, `nav`, `need`, `speechBubble`, `worldButton`, `picture`, small ASCII
  icons, the tennis ball at 3x scale). Hit boxes are logical px, same convention as the
  indexed games. Game-local for now; promote to `os/` if a second RGB565 game shows up.
- `generated/fonts.h`, and the pixel-accurate version of `tools/check_content.py` that
  reads it -- see the `art` and `content` skills for what each will do once it lands.

## Content limits (first release)

Full parity with the standalone firmware's content: 7 branching stories, 96 illustrated
sourced discoveries across 12 topics, 6 tricks (three lessons each to mastery), 12
stickers, 7 daily adventures with a pocket word each -- all landed with #33. Today
`tools/check_content.py` enforces a per-field character-count ceiling as a stand-in; the
real limit, measured against the runtime's actual page box (352x176, font24, 4px line
spacing) plus the round-screen rule for every other box, lands once `generated/fonts.h`
exists. See the `content` skill for the authoring workflow and exactly what the gate
checks today versus later.

## Layout: re-tuned, not copied

The old firmware's controls ran as small as 56 logical px tall (6.2 mm); Porthole's floor
is 24x22 logical (8 mm) -- see the shared app brief's constraint 1. Every Biscuit screen's
layout gets re-tuned to that floor, not ported as-is: Back grows 86x56 -> 86x66,
Previous/Next 128x56 -> 128x66, list rows 56/60 -> 66, the training cue pad and room
hotspots re-flowed to fit. The UI audit (`make playtest`) at zero FAIL and zero WARN is
the acceptance test for this, not a visual comparison to the old screenshots.
