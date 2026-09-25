# Biscuit on the Porthole runtime: port plan

## Context

The owner built two games for the same board (Waveshare ESP32-S3-Touch-LCD-2.1: 480x480 round touch
panel, no buttons). Pets Club (ex-Crago) is C++ on a home-grown 160x160 indexed renderer and now lives
in the `porthole` monorepo as the first game on a shared runtime (`apps/porthole/os` + `shell`) with
profiles, a launcher, hooks, CI, unattended releases and a web installer. The old standalone firmware
(Codex-built; repo and NVS namespace `zoegotchi`) is a browser JS prototype plus an Arduino+LVGL 8.3
firmware drawing at native 480x480 RGB565 with smooth Montserrat text and full-color pre-rendered
scenes. It was copied into porthole verbatim as `apps/biscuit`, then removed in PR #24 because it
shared nothing with the runtime; the approved launcher spec says "Biscuit will be rewritten later on
the same runtime". This is that rewrite plus the harness merge.

Goal: Biscuit is the second game inside the single porthole firmware image, on the shared runtime,
sharing emulator, playtests, build, CI, release and installer, and keeping the look it has on the
device today. The old standalone firmware's harness (two Codex skills, product contract, verification
discipline) merges into porthole's skills and agents, which are generalized from "Pets Club only" to
"any game".

Where: `~/src/dreamiurg/porthole`, feature branches off `origin/main`. The old code stays at
`~/src/dreamiurg/zoegotchi` (12 commits, clean) and in porthole history (`biscuit-v0.1.0`, commit
`1870956`).

## Decisions taken with the owner (2026-09-25)

| Question | Decision |
| --- | --- |
| The old standalone firmware's 25-minute daily play cap | Lives in the shell, per profile, counted across every game. Pets Club gets it too. |
| The old standalone firmware's save on the board (NVS namespace `zoegotchi`, key `pet3`) | Migrate it into one profile, the way Pets Club houses became profiles. |
| Biscuit's look (smooth text, full color) vs the Pets Club pixel look | Keep Biscuit's look. The shared runtime grows a full-resolution RGB565 surface; Biscuit's generated fonts and images carry over. Pets Club is untouched. |
| Content in the first release | Full parity: 7 branching stories, 96 illustrated sourced discoveries, 6 tricks, 12 stickers, 7 daily adventures with pocket words. |

Assumptions not validated with the owner (state them at the start of implementation):

- One firmware image, two games, picked from the launcher. Not a separate `apps/biscuit` firmware.
- The browser prototype is not kept. One implementation, C++ on the runtime; `make webemu` replaces it.
- Kid name and age come from the shell profile. The old standalone firmware's SetupChild,
  Settings/brightness, Clock, ResetConfirm, idle cover and in-game Rest screens are dropped: the shell
  and firmware own those (profiles, backlight dimming, serial `T` for the clock, profile delete, rest
  screens). Pet naming and renaming stay in Biscuit.
- Biscuit has no buzzer sounds (the old firmware never drove it; the buzzer is harsh).
- Porthole's 8 mm tap-target rule (24x22 logical = 72x66 physical px) applies to Biscuit. Most of the
  old standalone firmware's controls are 56 px tall (6.2 mm) and would fail the UI audit, so Biscuit's
  layouts get re-tuned, not copied.
- De-identification: the kid's real name is in the old standalone firmware's source, tests, migration
  defaults and dialogue fallbacks. Every occurrence becomes the profile name, or "friend" when there
  is none. Test fixtures use a neutral name.

## Inventory that drives the design

Porthole today (all under `apps/porthole/`):

- `os/gfx.*`: one global `uint8_t fb[160*160]`, palette-index primitives, 8x8 font, `textLog` audit.
  `os/palette.h`: 32 colors, `Tint`, palette built per tint at flip time. `os/input.h`: `InputTracker`,
  `UiAudit` hit regions in 160-logical px. `os/ui.h`: indexed widgets incl. a keyboard. `os/app.h`: the
  `App` interface (`icon()` returns an indexed `gfx::Sprite`, `render()` takes no surface argument).
- `firmware/board.cpp`: the RGB panel already has two 480x480 RGB565 frame buffers in PSRAM
  (`num_fbs=2`, `fb_in_psram`, 10-line bounce buffer, 16 MHz pclk). `present(fb160, pal565)` upscales
  3x nearest into the back buffer and calls `esp_lcd_panel_draw_bitmap` + swap + vsync wait. A native
  480 buffer can go through the same call. Touch is physical px; `main.cpp` divides by 3.
- `host/sim.cpp`: `writeBMP` and `writePPM` are near-duplicates that sample `gfx::fb` through the
  palette at 3x with a round mask; SDL window uses a 160 texture GPU-scaled; `--serve` already ships
  flat 480x480 RGB, so `tools/webemu.py` needs no change. Script DSL and `down/move` are logical px.
- `tools/playtest.py`: audit rules are all in logical px and never read pixels, except contrast, which
  maps `textLog` palette indices through `os/palette.h`. `make_sheet()` downsamples every snapshot to
  160 then up to 320 (would wreck true 480 content). `SCREENS` scans exactly two source files.
- `tools/devctl.py`: parses the serial `F` dump as `FB w h` + w*h index bytes + 64 palette bytes.
- `Makefile`: `GAME := games/pets-club`, `SRC`, `INCLUDES`, `lint`, `art`, test sources are all
  single-game. `platformio.ini`: `build_src_filter` already takes all of `games/`; include path is
  `-Igames/pets-club`; partitions are PlatformIO's `default_16MB.csv` (app slot 6.25 MB, two OTA slots).
- `shell/profiles.h`: `Record` is 44 bytes, append-before-`crc`, with `restUntil, playSec, lastPlayed`;
  `play()` gives 6 min play / 10 min rest only with 2+ profiles; `resting()` clamps to `REST_SEC`.
  `shell/migrate.cpp` is the one-time idempotent namespace-to-profile migration pattern (38 lines).
  `BLOB_MAX = 256`. `Shell::begin` already takes an app array.
- Both `firmware/main.cpp` and `host/sim.cpp` hold `static Game g_pets; static App* const APPS[] = {&g_pets};`.

The old standalone firmware (under `~/src/dreamiurg/zoegotchi/firmware/`):

- Rules: `include/pet.h`, header-only `namespace pet`, `Pet` POD 112 bytes v3, decay 4/2/3 per hour
  awake, 8-hour elapsed cap, needs floor 20, stages
  Puppy/YoungPup/StoryDog, trick unlock days {1,1,2,3,5,7}, 7-day adventure and 12-day sticker cycles.
  Save wrapper `{magic 0x5a4f4533, Pet, FNV-1a over Pet}`, 120 bytes, NVS `zoegotchi/pet3`.
- UI: `src/main.cpp` 955 lines, 23 views, LVGL widget helpers, layout numbers per view quoted in the
  report; `paginate()` fills a 352x176 box at (64,146) with font24, 4 px line spacing (5 lines/page);
  4-frame scene animation at 250 ms; time of day day/evening/night at 17:00 and 20:00.
- Assets: `generated/assets.cpp` (21 MB of source, ~5.2 MB of rodata): 369 unique 160x160 RLE RGB565
  scenes (4.26 MB) indexed [3 stages][3 times][14 activities][4 frames], 96 uncompressed 96x48
  discovery pictures (885 KB), text 75 KB. Generated by `tools/export-assets.mjs` (Node 22, zero npm
  deps) from `src/art.js`, `src/discovery-art*.js`, `src/stories.js`, `src/discoveries*.js`,
  `src/game.js`; `manifest.json` records per-scene SHA-256.
- Fonts: `generated/font{16,20,24,28}.c` from `lv_font_conv@1.5.3`, 4 bpp uncompressed, 98 glyphs
  (ASCII 0x20-0x7e plus U+00B7 U+00E9 U+00F6), class kerning tables, line heights 17/22/26/29.
  Plain C arrays; a ~150-line renderer reads them without LVGL.
- Last build: 4.6 MB image, IRAM 99.99% used (with LVGL). Custom `partitions.csv`: nvs 0x9000/0x5000,
  otadata 0xe000, factory app 12 MB, coredump.

## Architecture

### A. Runtime: an RGB565 surface in `os/`

New `os/gfx565.h/.cpp` and `os/font.h`. The indexed path is untouched.

- Surface: `namespace gfx565 { constexpr int W = 480, H = 480; extern uint16_t* fb; void target(uint16_t*); }`.
  The host sets the target once per frame: firmware hands the panel's back buffer (no extra PSRAM, same
  bandwidth pattern as today's `present()`); the sim hands a static 460 KB array.
- Primitives, RGB565 colors: `clear`, `rect`, `roundRect(x,y,w,h,r,c)`, `frame`, `circle`, `hline`,
  `blit(const Image565&, x, y, scale)` (scale 1 or 3, nearest), `blitRle(const RleImage&, x, y, scale)`
  decoding runs straight into the target at scale (no scene scratch buffer), plus `inCircle(x, y)` for
  radius 240 around (240,240).
- Text: `font::Font` = flat tables `{bitmap, glyphs[], kernLeft[], kernRight[], kernValues[], lineHeight,
  baseLine}` with `Glyph {bitmapIndex, advW, boxW, boxH, ofsX, ofsY}`; `glyphIndex(cp)` maps ASCII
  32..126 and the three extras (UTF-8 decoded), anything else to a visible placeholder.
  `textWidth(font, s)`, `text(font, s, x, y, fg)` blending 4-bit coverage against the pixel already in
  the target, `wrap(font, s, width, spacing, out)` returning line breaks, `pageBreaks` = the port of
  `paginate()` (fill a box by measured height; the same function is what the content gate calls).
  Kerning included (class tables are ~20 lines to read).
- Audit: `gfx565::text` logs the box into the shared `gfx::textLog` in logical px (divide by 3) with a
  new RGB form; the sim's `ui` dump prints `text x y w h rgb=RRGGBB bg=RRGGBB` for those, and
  `tools/playtest.py` parses both forms and computes contrast from RGB directly. Hit regions are
  unchanged: Biscuit hit-tests with `Input::hit` on logical boxes.
- Fonts live with the first consumer: `games/biscuit/generated/fonts.h`, produced once from the four
  LVGL `.c` files by `games/biscuit/tools/fontconv.py` (regex over the C arrays). Regeneration needs
  `npx lv_font_conv@1.5.3` only if a glyph is ever added; the content gate rejects glyphs outside the
  set, so that is rare. Node and lv_font_conv are not part of hooks or CI.

### B. Contract: which surface an app draws on

- `os/app.h`: `enum Surface : uint8_t { SURFACE_INDEXED, SURFACE_RGB565 };` and
  `virtual Surface surface() const { return SURFACE_INDEXED; }`. Non-pure, so Pets Club does not change.
  Polled every frame, so a game may switch per screen.
- `Shell::surface()` returns the active app's surface on `SH_APP`, else `SURFACE_INDEXED`. Launcher,
  picker, rest and the shared keyboard stay indexed.
- Hosts branch on it once per frame:
  - `firmware/main.cpp`: `present(gfx::fb, pal)` or `board::presentHires()` (new: `draw_bitmap` on the
    back buffer + swap + vsync, ~10 lines next to `present()`). Before `render()`, hand the back buffer
    to `gfx565::target()`.
  - `host/sim.cpp`: one `framePixel(x, y)` helper (indexed: `pal[fb[(y/3)*160 + x/3]]`; RGB565:
    `fb480[y*480+x]` widened) feeding `writeBMP`, `writePPM` and the SDL texture, which becomes a
    480x480 texture updated CPU-side (the window then previews the panel's exact nearest 3x instead of
    GPU bilinear). `writeBMP`/`writePPM` collapse to one pixel source and two encoders.
  - Serial `F` gains a third header token: `FB 160 160 idx` or `FB 480 480 565`; `tools/devctl.py`
    reads the token (absent = idx) and skips palette and upscale for 565.
  - `tools/playtest.py make_sheet()`: keep the 160 downsample only when the snapshot is an exact 3x
    replicate (`img.resize(160, NEAREST).resize(480, NEAREST) == img`), else LANCZOS 480 to 320.
- Launcher icon: `App::icon()` stays an indexed `gfx::Sprite` (48x44 slot). Biscuit's is a 32x32 ASCII
  sprite generated by `games/biscuit/tools/icon.py` into `generated/icon.h`, same style as
  `tools/shell_art.py`.

### C. Shell: daily play cap per profile

- `shell::Record`: append `uint32_t dayPlaySec; uint32_t playDay;` before `crc`, bump `version`, extend
  the loader to zero-fill older records (the `size` field already exists for this). Add the round-trip
  case to `host/test_shell.cpp`.
- `shell::play()`: `DAILY_SEC = 25*60`. Reset `dayPlaySec` when `now/86400 != playDay` (porthole's clock
  is local wall-clock seconds, so no timezone rule; the old standalone firmware's hardcoded Pacific rule goes away).
  Accumulate; at the cap set `restUntil = (playDay+1)*86400` (next local midnight). Unconditional (1+
  profiles), unlike the 6/10 turn-taking rule which stays 2+.
- `shell::resting()`: the backward-clock clamp becomes `restUntil - now <= 86400 + REST_SEC`.
- Idle time does not count: the shell skips `play()` while no touch for 60 s (matches the backlight
  dimming step). Fixes both budgets.
- Rest screen: when `restUntil - now > REST_SEC`, draw "<name> played today. Back tomorrow" with no
  countdown (the old standalone firmware's bedtime rule: the child sees bedtime as part of the game). Otherwise the
  existing "Back in N min".
- `Shell::debugCmd`: add `bedtime` (fill today's budget) for playtests.
- Playtest scenario: play to the cap in Pets Club, switch to Biscuit, expect the rest screen; new day,
  expect the launcher.

### D. Biscuit game module: `apps/porthole/games/biscuit/`

Everything in `namespace biscuit` (Pets Club owns global `Game` and `namespace pet`; two `pet.h` in one
link would collide).

- `pet.h/.cpp`: the rules port, kept mechanical: same decay, floors, stages, unlock days, cycles,
  `valid()`. New save struct following porthole conventions (`magic, version, size`, fields, `crc`,
  append-only, `loadBlob` accepts older sizes): `createdAt, updatedAt (uint32 local sec), lastVisitDay,
  fullness/happiness/energy, friendship, daysTogether, careCounts[3], discoveries[3], stickers,
  stories, tricks[6], dailyCompleted, dailyClaimed, sleeping, petName[13], named`. Dropped:
  `playerName` (profile), `playSeconds` (shell), `setupComplete` (becomes `named`), in-struct version.
  Under 256 bytes (`BLOB_MAX`). `fromLegacy(const LegacySave&, Save&)`: accepts only the ZOE3 layout
  with the FNV-1a check and `valid()`.
- `game.h/.cpp` (`class Game : public App` in `namespace biscuit`): App plumbing, `enter` (decode
  own profile's blob; ignore other profiles' saves), state, `command()` dispatch as in the old
  `main.cpp`, `surface()` = RGB565 except on the naming screens, which use the shell's indexed
  `ui::keyboard` (saves porting a 28-key keyboard and it already passes the audit). `store()` returns
  `"biscuit"`, never renamed after the first release. `tint()` from local hour (17:00, 20:00), used
  both for scene selection and returned to the shell. `soundOn` false. `asleep()` = napping.
- Screens split by file to stay under the complexity gate: `screens_home.cpp` (Home with scene,
  needs, speech bubble, hotspots, Feed/Read/Play/More, fetch ball; World), `screens_read.cpp`
  (Library, Story, Choice, Ending), `screens_learn.cpp` (Discoveries, Topics, Discovery cover/pages/
  wonder, Source, Today, Word), `screens_train.cpp` (Tricks, Training watch/do), `screens_profile.cpp`
  (Profile, Stickers, SetupPet, RenamePet). 17 views; Settings, Clock, ResetConfirm, SetupChild, idle
  cover and Rest are gone.
- `ui565.h/.cpp`: the old LVGL helpers as plain functions on the surface: `button`, `iconButton`,
  `top`, `nav`, `need`, `speechBubble`, `worldButton`, `picture`, 12x12 ASCII icons and the tennis
  ball drawn at 3x. Hit boxes are logical. Game-local for now; promote to `os/` when a second RGB565
  app appears.
- Layout re-tune to the 8 mm floor: every tappable control at least 72x66 physical (24x22 logical) and
  inside the circle; Back 86x56 -> 86x66, Previous/Next 128x56 -> 128x66, list rows 56/60 -> 66,
  training cue pad and "Peek again" re-flowed to fit five 88x66 keys above the bottom chord, room
  hotspots (fern 64x46, shelf, nap) grown or merged. The UI audit is the acceptance test, at zero
  FAIL and zero WARN.
- Time and animation: scene frame every 250 ms from `ms`, shelf/fern 4-frame one-shots, bubble text
  for 4 s, needs decay tick every 30 s, save via `takeSave` (dirty flag; the shell throttles to 5 s).
- Migration of the old save, in `shell/migrate.cpp` next to the Pets Club one (it is "the only shell
  file that knows a game's save layout"): once, marker `porthole/mb`; read `zoegotchi/pet3`; convert
  with `biscuit::fromLegacy`; write `biscuit/s<id>` for the profile whose name equals the blob's
  `playerName` (case-insensitive), else the lowest id; with zero profiles create one from
  `playerName` with age 0 like `adoptHouse`; erase `pet3` only after the marker. `host/test_shell.cpp`
  gets a synthetic ZOE3 blob case (FNV-1a computed in the test).

### E. Content pipeline

- Text is hand-authored C++ like Pets Club: `content_stories.h` (7 `Story {id, title, subtitle,
  unlockDay, pages[10], prompt, choices[2]{label, ending[4]}}`), `content_discoveries.h` (96
  `Discovery {id, topic, title, pages[2], wonder, sourceName, sourceUrl}`, 12 topics),
  `content_daily.h` (7 adventures with word and meaning, 6 tricks with cue patterns and lessons, 12
  sticker names). Converted once from the JS sources by a throwaway script; the kid's name in copy
  becomes a `{name}` token that `personalize()` fills from the profile.
- Images stay generated by the JS art tool, moved as-is: `games/biscuit/tools/art/{art.js,
  discovery-art*.js, export-assets.mjs}` emitting `generated/scenes.h` (RLE) and
  `generated/discovery_art.h` (96 `Image565` in an explicit id order that the tool also writes as an
  `enum DiscoveryId`). `make -C apps/porthole art` runs it with `node` (22+, zero deps) alongside
  `art.py`; `--check` reproduces byte-for-byte using the manifest hashes. Node is a dev-machine tool
  only; the generated headers are committed and excluded from pre-commit, semgrep and lizard like
  `sprites.h`. `ponytail:` two languages for art tooling; port `art.js` to Python if Biscuit's art
  starts changing often (the manifest SHA-256 per scene is an exact oracle for such a port).
- `games/biscuit/tools/check_content.py`: the pixel-accurate gate, reading `generated/fonts.h` for
  advance widths and kerning, applying the same fill rule as `font::pageBreaks` (352x176 box, font24,
  spacing 4) and the circle rule for every string in every box the game draws it in (titles, rows,
  buttons, bubbles, discovery pages, story pages, wonder, source names). Fails on any glyph outside
  the font's 98. Also checks `content_discoveries.h` ids equal `generated/discovery_art.h`'s enum
  order. Runs in `lint`.
- Flash: ~5.2 MB of assets plus code does not fit the 6.25 MB OTA slot. `apps/porthole/partitions.csv`
  = the old standalone firmware's proven table (nvs 0x9000/0x5000 unchanged so every save survives, otadata kept for
  the uploader, one 12 MB factory app, coredump), `board_build.partitions` and
  `board_upload.maximum_size = 12582912` in `platformio.ini`. `platform/factory_image.py` and the web
  installer read offsets from PlatformIO, so they follow. If flash ever gets tight, scenes can be
  stored palette-indexed (Biscuit's flat palette is small) for roughly 4x less; not needed now.

### F. Multi-game build

- `Makefile`: `GAMES := $(wildcard games/*)`; `SRC` = `os/*.cpp shell/*.cpp games/*/*.cpp`;
  `INCLUDES := -Ios -Ishell -I.`; games include their own headers by bare name (same-directory
  lookup), hosts and tests by path (`games/pets-club/game.h`, `games/biscuit/game.h`); `test` builds
  `host/test_pet.cpp`, `host/test_biscuit.cpp`, `host/test_shell.cpp`; `lint` and `art` list both
  games' tools explicitly (two games, no loop machinery); `coverage` covers `games/*/`.
- `platformio.ini`: include path `-I.` instead of `-Igames/pets-club`; partitions as in E.
- `firmware/main.cpp`, `host/sim.cpp`: `static biscuit::Game g_biscuit;` appended to `APPS[]`.
- `tools/playtest.py`: `SCREENS` scans `games/*/game.cpp`; `expect-screen` names get a game prefix
  where they clash.
- Root: `.pre-commit-config.yaml` and `.semgrepignore` exclude `apps/porthole/games/biscuit/generated/`;
  `whitelizard.txt` gets no new entries; CI, release and installer are app-level and unchanged.

## Harness merge

| Result | Built from | Notes |
| --- | --- | --- |
| Root `CLAUDE.md` and `apps/porthole/CLAUDE.md` | porthole's | Architecture map gains `games/biscuit/`, the RGB565 surface, `Surface`, the partition table, the daily cap. Hard rule 2 becomes per surface: indexed games keep 32 colors + 8x8 ASCII; RGB565 games keep flat fills, the bundled font's glyph set, and the same round-screen and 8 mm rules. Delegation table lists both games. |
| `apps/porthole/games/biscuit/CLAUDE.md`, `games/pets-club/CLAUDE.md` | new | Per-game briefs. Biscuit's carries the old standalone firmware's product contract (companion, not mini-games; no death, guilt, streaks, countdowns, lost progress; warm doglike voice; sourced facts; stable ids and array order), its module map and its content limits. Pets Club's takes the reading-level tables and Save notes out of the app brief. |
| skill `feature` | `zoegotchi-feature` | Generalized to any game: state the feature contract before editing (entry, play, completion, replay, sleep, Back; one-time vs repeat rewards; midnight, clock rollback, duplicate action; saved fields, stable ids, migration), failing rule-level test first, the proof list (rules and migration tests, restart/replay/time changes, every screen reachable and escapable by touch, playtest at zero findings). Browser parity, LVGL and `device.py` steps dropped. |
| skill `content` | `new-story` + `zoegotchi-content` | One skill with a game section each: Pets Club books and spelling words with `check_content.py`; Biscuit stories, discoveries, adventures and words with theirs; the claim ledger for factual entries (kept under ignored `build/content-review/`), duplicate and diversity review, the voice guide with the good/bad lines, "regenerate, never hand-edit generated", append-only ids. |
| skills `playtest`, `tour`, `flash`, `art` | porthole's | Frontmatter and prose stop saying "Pets Club"; `tour` documents `app biscuit`; `art` covers both generators and the RGB565 rules; `playtest` documents the RGB text audit line and the sheet rule. |
| agents (7) | porthole's | Descriptions become "any game under apps/porthole/games/<game>/", owned paths listed per game, game specifics in each game's CLAUDE.md. `story-writer` gains discoveries and the claim ledger; `pixel-artist` gains the JS art tool and the RGB565 style; `playtester` gains the RGB text-fit check; `reviewer` gains `Surface` and the shell budget in its checklist. |
| dropped | `device.py`, `check-layout.sh`, `partitions_test.py`, `export-fonts.mjs --check`, `device_flow.py` | Covered by `devctl.py`, the new content gate, `factory_image.py`, the committed fonts. `device.py`'s `test-begin/test-end` (RAM-only state for on-device tests) is not needed while `--fresh` and profiles exist. |

Skill format: Codex `.agents/skills/<name>/SKILL.md` and Claude `.claude/skills/<name>/SKILL.md` have
the same shape (YAML `name`/`description`, Markdown body); bodies port with path edits.

## Delivery order

PRs, stacked where dependent. Ship a device build after PR 6a per the "playable early" preference.

1. `feat(porthole): daily play cap in the shell` (C). Independent. Value for Pets Club now.
2. `docs: generalize skills and agents for two games; feature and content skills; per-game briefs`
   (Harness). Independent; land early so the agents building 4-6 use it.
3. `build: two games in one image` (F, without Biscuit sources: Makefile, includes, partition table,
   playtest screens, excludes). Independent.
4. `feat(porthole): RGB565 surface, fonts, hi-res present and snapshots` (A, B, plus
   `games/biscuit/generated/fonts.h` and `fontconv.py`, a host test that renders and measures text
   and checks `pageBreaks` on a known page). Independent of 1-3.
5. `feat(biscuit): rules, save, legacy migration` (D `pet.*`, `fromLegacy`, `host/test_biscuit.cpp`,
   `migrate.cpp` case). Independent of 4.
6. `feat(biscuit): content and generated art` (E). Needs 4 for the gate's font tables.
7. `feat(biscuit): the game` (D screens). Needs 3-6. Split: 7a Home, care, fetch, naming, tricks;
   7b Library, stories, discoveries, Today, Profile, stickers. Playtests, screenshots, README and
   catalog updates with 7b.

Parallel: 1, 2, 3, 4, 5 at once (five agents); 6 after 4; 7a after 3-6; 7b after 7a. Critical path:
4 -> 6 -> 7a -> 7b.

Model routing: game-engineer (opus) for 1, 3, 4, 5, 7; pixel-artist (opus) for the icon and any art
change in 6; story-writer (sonnet) for the content conversion in 6; the harness PR 2 is prose
(sonnet) with an opus review; reviewer (opus) before every push; playtester (opus) on 7a and 7b.

## Verification

- Gates stay `make -C apps/porthole check` (lint at zero warnings, `test` = both games' self-checks
  and the shell tests) and `ci` (playtests, coverage floor 91 on `os/`, `shell/`, `games/*/`).
- `host/test_biscuit.cpp` ports the old standalone firmware's `pet_test.cpp` cases: names and setup, rename preserves
  the companion, legacy ZOE3 migration, care and elapsed time (decay, 8-hour cap, floors, sleep),
  permanent reading progress (idempotent stars, 96 discovery bits, out-of-range no-op), adventures,
  tricks and growth (unlock days, mastery saturation, 12-day sticker loop, stage thresholds, negative
  day wraparound), rejects broken snapshots.
- `host/test_shell.cpp` adds: record round-trip with the new fields, daily cap reached, reset at the
  next local day, idle seconds not counted, backward clock clamp, Biscuit legacy blob migration to the
  name-matched profile and to a new profile.
- Runtime test for the RGB565 layer: text width of a known string equals the sum of advances plus
  kerning; a story page breaks where the old firmware broke it (spot check three pages against the
  device screenshots in `zoegotchi/output/device-qa/`).
- Playtests: `20_biscuit_care`, `21_biscuit_fetch`, `22_biscuit_story_both_branches`,
  `23_biscuit_discoveries_keep`, `24_biscuit_tricks_mastery`, `25_biscuit_today_sticker`,
  `26_daily_cap_across_games`, `27_biscuit_naming`, a Biscuit monkey under ASan, and an
  `expect-screen` tour of all 17 views. UI audit at zero FAIL and zero WARN, `make playtest` green.
- Device: `make -C apps/porthole flash`; `tools/devctl.py` tour of both games with `F` dumps; on the
  board that still holds `zoegotchi/pet3`, confirm the migration lands on the right profile and
  `S` shows free heap and PSRAM headroom; check IRAM in the build output (the old standalone firmware's LVGL build sat
  at 99.99%); look at scene animation and text on the panel with eyes (tearing, color).
- Screenshots: `tools/gallery.py` (no `--pixel` for Biscuit) into `apps/porthole/docs/preview.png` and
  `screenshots.png` showing both games; root README catalog section updated.

## Changes during implementation

Kept here rather than edited into the sections above, so the original plan and what actually shipped
stay distinguishable.

- The daily play cap (PR #31) is kept in `shell::Record` as `(playDay, dayPlaySec)`, not as the
  `restUntil`-deadline scheme section C sketched -- a new day or a clock set back just resets the
  counter, instead of needing a special backward-clock clamp.
- Two legacy save locations need migrating, not one: the pre-porthole standalone firmware's NVS
  `zoegotchi/pet3` (section D), and the short-lived in-monorepo `apps/biscuit` (`biscuit-v0.1.0`,
  removed in PR #24), whose own save lived at NVS `biscuit/pet1`.
- The content PR (#33) adds a second personalization token, `{pet}` (the dog's own name, falling back
  to "Biscuit"), alongside `{name}` (section E).

## Risks and open points

- Layout re-tune for 8 mm targets changes most Biscuit screens; the Training pad and the top bar are
  the tight ones. Budget a playtester pass, not a copy.
- IRAM and PSRAM: proven by both firmwares separately, not together. Check the first firmware build of
  PR 4 before building screens on it.
- The old blob may not be on the board (nobody knows if the kid played the LVGL build). The migration
  is small and tested either way.
- Content volume on a new text renderer: 96 discoveries x 3 pages plus 7 stories x 12 pages have to
  pass the gate; expect a batch of one-word edits.
- Not decided: whether Biscuit's World screen keeps a "Settings" entry once brightness and clock are
  gone (recommend: drop, mute lives on the launcher).
