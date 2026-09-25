# Biscuit: game brief

**None of this exists in code yet.** Biscuit is being rewritten onto the Porthole runtime;
this file describes the intended design so every agent building toward it works from the
same contract. The full plan, with rationale and delivery order, is
`docs/superpowers/specs/2026-09-25-biscuit-on-porthole-design.md` -- read it before
implementing anything referenced here as "landing with the Biscuit PRs." Once a PR lands,
update the paragraph it makes real and drop the qualifier for that part.

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
- **Warm, doglike voice.** Biscuit speaks in short, natural sentences. He notices smells,
  sounds, naps, paws, snacks, closeness, and the child's mood. He may ask a sharp question,
  but he never sounds like an investigator, teacher, mission controller, or corporate
  assistant. Good: "You read the tiny marks. I'll keep your toes warm." Bad: "Supplies
  secured. What are we investigating?" Not every discovery needs a Biscuit line -- warmth
  can come from the surrounding interaction; don't force dialogue where it doesn't belong.
- **Sourced facts, with a claim ledger.** Every discovery's factual claim traces to an
  authoritative source (museum, university, science agency, standards body, original
  research) that supports that specific claim, not just its general topic. While drafting,
  keep a compact ledger (claim, exact URL plus page/section, supporting passage, confidence,
  visual implication) under ignored `build/content-review/` until review is done -- don't
  copy long source passages into it. Separate established fact from interpretation,
  reconstruction, legend and open scholarly question; avoid unsupported superlatives
  ("first," "oldest," "proved"). Automated checks validate structure and URLs, not truth --
  reopen every source and compare it against the final text by hand before shipping.
- **Stable ids, stable array order.** Story and discovery ids and their array order are
  persisted -- a save encodes progress by index. Never reorder or resize an existing entry;
  append, and add explicit migration coverage for anything that isn't a pure append.
- **Personalization, not a hardcoded name.** The companion is for whichever profile is
  playing. Copy in content files uses a `{name}` token that `personalize()` fills from the
  profile at render time; never hardcode a specific child's name into game text, a test
  fixture, or a migration default. Use "friend" as the fallback when there is no name yet.

## Module map (landing with the Biscuit PRs)

Everything lives under `namespace biscuit` -- Pets Club owns the global `Game` class and
`namespace pet`, and two `pet.h` headers in one link would collide.

- `pet.h` / `pet.cpp`: the rules -- decay, floors, stages, trick/adventure/sticker unlock
  cycles, `valid()`. Save struct: `magic, version, size` header; `createdAt, updatedAt,
  lastVisitDay, fullness/happiness/energy, friendship, daysTogether, careCounts[3],
  discoveries[3], stickers, stories, tricks[6], dailyCompleted, dailyClaimed, sleeping,
  petName[13], named`; `crc` last. Under `BLOB_MAX` (256 bytes), same append-only rule as
  every other game -- see the shared app brief's constraint 4. `fromLegacy(const
  LegacySave&, Save&)` accepts only the old standalone firmware's save layout (FNV-1a
  checksum verified) and converts it once, from `shell/migrate.cpp`, into a profile's
  Biscuit save.
- `game.h` / `game.cpp` (`class Game : public App`, in `namespace biscuit`): App plumbing,
  `enter` (decodes its own profile's blob only), state, command dispatch. `surface()`
  returns `SURFACE_RGB565` everywhere except the naming screens, which reuse the shell's
  indexed `ui::keyboard` rather than porting a second keyboard. `store()` returns
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
- Content: `content_stories.h` (7 branching stories), `content_discoveries.h` (96
  discoveries across 12 topics), `content_daily.h` (7 daily adventures, 6 tricks, 12
  sticker names) -- see Content limits below and the `content` skill.
- Generated art and fonts, never hand-edited: `generated/scenes.h`, `generated/
  discovery_art.h` (from `tools/art/{art.js, discovery-art*.js, export-assets.mjs}`, run by
  `make -C apps/porthole art`) and `generated/fonts.h` (from the four bundled LVGL font
  files via `tools/fontconv.py`, regenerated only if a glyph is ever added). Regenerating
  needs Node 22+ and `npx lv_font_conv@1.5.3` on the dev machine only -- neither is a hook
  or CI dependency, since the generated headers are committed.
- `tools/check_content.py`: the content gate. Reads `generated/fonts.h` for advance widths
  and kerning, applies the same page-fill rule as `font::pageBreaks` (352x176 box, font24,
  4px spacing) and the round-screen rule to every string in every box the game draws it in,
  rejects any glyph outside the font's 98, and checks `content_discoveries.h`'s ids match
  `generated/discovery_art.h`'s enum order.

## Content limits (first release)

Full parity with the standalone firmware's content: 7 branching stories, 96 illustrated
sourced discoveries across 12 topics, 6 tricks (three lessons each to mastery), 12
stickers, 7 daily adventures with a pocket word each. See the `content` skill for the
authoring workflow and the exact per-box pixel limits `check_content.py` enforces.

## Layout: re-tuned, not copied

The old firmware's controls ran as small as 56 logical px tall (6.2 mm); Porthole's floor
is 24x22 logical (8 mm) -- see the shared app brief's constraint 1. Every Biscuit screen's
layout gets re-tuned to that floor, not ported as-is: Back grows 86x56 -> 86x66,
Previous/Next 128x56 -> 128x66, list rows 56/60 -> 66, the training cue pad and room
hotspots re-flowed to fit. The UI audit (`make playtest`) at zero FAIL and zero WARN is
the acceptance test for this, not a visual comparison to the old screenshots.
