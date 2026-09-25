# Biscuit: game brief

Biscuit's rewrite onto the Porthole runtime is complete: content and its generated art (#33), the rules, save
and legacy migration (#35, `pet.h`), the RGB565 surface (#36), the first half of the screens (7a, #39: Home, care,
fetch, World, Tricks and Training, the scrapbook, pet naming) and the second (7b: the Library and stories,
discoveries, Today and its pocket word, stickers) -- full content parity with the standalone firmware. The plan it
was built from, with rationale and delivery order, is
`docs/superpowers/specs/2026-09-25-biscuit-on-porthole-design.md`.

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
  for the child ("Friend" when the profile has none) and `{pet}` for the dog ("Biscuit"
  until it is named). Never hardcode a specific child's name into game text, a test
  fixture, or a migration default.

## Module map

Everything lives under `namespace biscuit` -- Pets Club owns the global `Game` class and
`namespace pet`, and two `pet.h` headers in one link would collide.

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
  winning source's keys, and only after the marker `porthole/mb`. `shell::loadAll` runs it
  at every boot after the Pets Club migration (a no-op once the marker exists). Profile
  delete and create, and serial `R`, erase `biscuit/s<id>` like every game's store in `APPS`.
- `host/test_biscuit.cpp` (shared `host/` directory, #35): `pet.h`'s rules self-check --
  distinct from `host/test_biscuit_content.cpp` (content/art plumbing, #33).
- Content: `content_stories.h` (7 branching stories, `Story{id, title, subtitle,
  unlockDay, pages[10], prompt, choices[2]{label, ending[4]}}`), `content_discoveries.h`
  (96 `Discovery{id, topic, title, pages[2], wonder, sourceName, sourceUrl}` across 12
  topics), `content_daily.h` (7 daily adventures, 6 trick names, 12 sticker names; which
  activities a day asks for, the trick lessons and unlock days are rules, in `pet.h`) -- `namespace biscuit`, `inline constexpr`, converted once from
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
  fields, fixed counts, unique ids, discovery order vs. `DiscoveryId`. Fit is the pixel
  gate's job (below). See the `content` skill for exactly what each checks.
- `layout.h`: the label box (font, width, most height, spacing, alignment, position) of
  every screen that draws content, from the LVGL firmware's layouts. The screens draw with
  these constants and the pixel gate measures against them: re-tune a box here.
- `host/test_biscuit_content.cpp` (shared `host/` directory, not inside this game's own):
  `personalize()`, the generated art tables, and the pixel gate: every string, tokens
  filled with the widest names, measured and drawn by `os/font.cpp` in its `layout.h` box
  and inside the round glass.

- `game.h` / `game.cpp` (`class Game : public App`, in `namespace biscuit`, 7a): App
  plumbing, `enter` (decodes its own profile's blob only; an unnamed pup is never saved),
  the speech line and activity timers (a line 4 s, a room one-shot 800 ms), needs ticking
  every 30 s with a save checkpoint every 5 min or at a new day, screen dispatch.
  `surface()` is `SURFACE_RGB565` everywhere except the two naming screens, which reuse the
  shell's indexed `ui::keyboard` (8 letters). `store()` is `"biscuit"`, never renamed.
  `soundOn()` is always false: Biscuit has no buzzer sounds (shared app brief constraint 3).
  `tint()` follows the scenes' light: evening from 17:00, night from 20:00 to 6:00. Screen
  names are prefixed `biscuit_` (the playtest runner fails on a name two games share).
  `debugCmd`: `hungry unlock young grown tricks practiced read` (`read`: every story finished and every
  discovery kept).
- Screens, split by file to stay under the complexity gate: `screens_home.cpp` (Home:
  scene frames every 250 ms, name and day, needs, speech bubble, hotspots for the shelf
  (the pup pulls out a book, and 800 ms later the Library opens; any other tap first
  cancels it), window (nap), fern and the pup (a tap pets it), Feed/Play/Read/More, fetch,
  and the orange home button to the launcher; World: Learn tricks, Cozy nap, Our
  scrapbook, Today's adventure), `screens_train.cpp` (Tricks, Training watch/do),
  `screens_read.cpp` (Library: Discoveries, Notebook and the seven stories, two a page,
  "Day N: <title>" until they open, "<title> *" once read; Story, Choice, Ending; and the
  page reader the discoveries share), `screens_learn.cpp` (Discoveries: today's three,
  a topic's eight or the notebook of kept ones; Topics; a discovery's cover, pages and
  wonder page, Source; Today and its Word), `screens_profile.cpp` (the scrapbook with Our
  stickers and Rename pup, the sticker album, SetupPet, RenamePet).
  - Reading: each story, ending or discovery page is filled with the names (a 256-byte
    buffer) and split by `font::pageBreaks` into at most `PAGE_SCREENS` screens; the
    counter between Previous and Next counts screens across the whole story (a story is
    about 20, an ending 8). Next becomes Choose on a story's last screen and The end on an
    ending's; an ending's first Previous goes back to the choice, the choice's Back to the
    story's last screen. The end brings the pup home (`finishStory`: today's reading, and
    the first finish of a story its three stars). A discovery's cover, pages and wonder
    page, then Keep (`discover`: today's reading, and the notebook) opens the notebook on
    its page; Source shows the name and address as text. Every reader turn waits for the
    fresh-screen pause, since Next's meaning changes at the ends.
  - Today: the day's adventure (`ADVENTURES[day % 7]`), its three activities (from
    `dailyMask`) as buttons that go and do them (a snack, fetch, a cuddle, the Library,
    the tricks, a nap), marked `* ` once done, and A lovely word. The third earns the
    sticker (`recordActivity`, once a day); World's tile then says "Sticker earned!".
  - Today's three discoveries follow the old firmware: one from each third of the topics,
    `topic = family * 4 + day % 4`, the `(day / 4) % 8`-th of that topic.
  Screens the old standalone firmware had that Porthole's shell now owns instead: Settings,
  Clock, ResetConfirm, SetupChild, the idle cover, and Rest.
- `ui565.h` / `ui565.cpp` (7a): the widgets on the RGB565 surface: `button`,
  `actionButton`, `tile`, `hotspot`, `roundButton`, `home`, `top`, `nav`, `need`, `bubble`,
  12x12 ASCII icons and the tennis ball at 3x. Split in two like nothing in `os/ui.h`:
  `update()` asks `tapped()`, `render()` draws; never draw from `update()`, where on the
  device the RGB565 target is the buffer on the glass. Hit boxes are logical px.
  Game-local for now; promote to `os/` if a second RGB565 game shows up.
- `tools/icon.py` -> `generated/icon.h`: the launcher icon, a 32x32 indexed sprite (the
  launcher is the shell's), with a `--check` in `make lint`.

## Content limits (first release)

Full parity with the standalone firmware's content: 7 branching stories, 96 illustrated
sourced discoveries across 12 topics, 6 tricks (three lessons each to mastery), 12
stickers, 7 daily adventures with a pocket word each -- all landed with #33. The limit is
the box each string is drawn in (`layout.h`), measured in pixels by `os/font.cpp` with the
widest names filled in: a story, ending or discovery page may take at most `PAGE_SCREENS`
(2) screens of the 352x176 font24 page box, every other string its own box, all of it
inside the round glass. See the `content` skill for the authoring workflow.

## Layout: re-tuned, not copied

The old firmware's controls ran as small as 56 logical px tall (6.2 mm); Porthole's floor
is 24x22 logical (8 mm) -- see the shared app brief's constraint 1. Every Biscuit screen's
layout gets re-tuned to that floor, not ported as-is (physical px). 7a: Back 86x56 -> 90x66,
Previous/Next 128x56 -> 120x66 with the page number between them ("n/m" when "n / m" is too
wide), trick rows 316x56 -> 312x66, Home's actions 68x72 -> 72x66, the cue pad five 90x66
keys with the lesson count in the title slot, World's back disc 56 -> 66, the fern 64x46 ->
75x66, and the ball's box 80x64 -> 81x72. 7b: list rows 56/60/64 -> 312x66 (three a page,
or a pair of 153x66 buttons and two rows), the choice rows 324x68 -> 312x66 (lower: nothing
else is at the bottom), Today's activities two by two (153x66) with A lovely word as the
fourth, Source / Keep and the scrapbook's Our stickers / Rename pup where Previous / Next
are, and the wonder question and the pocket word's meaning moved up to clear the bottom
buttons (with the widest names they take six and seven lines). The UI audit (`make
playtest`) at zero FAIL and zero WARN is the acceptance test for this, not a visual
comparison to the old screenshots. A box that holds content lives in `layout.h`: re-tune it
there, and the content gate measures the new box.
