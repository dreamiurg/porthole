# Porthole: agent brief

## What this is

Porthole is the device firmware: a shared runtime (`os/`), the board layer (`firmware/`), the shell (`shell/`: "Who's playing?", up to four profiles, the launcher, the rest timer) and the games compiled into one image (`games/`). Its first game is Pets Club. A second game, Biscuit, is being rewritten onto this same runtime (it previously shipped as a standalone LVGL firmware and was removed in PR #24); it is landing PR by PR (rules, save and legacy migration are in; the RGB565 surface and its screens are not) -- where this brief or `games/biscuit/CLAUDE.md` calls something "landing," read that literally, not as a blanket disclaimer. Design: `docs/superpowers/specs/2026-09-24-launcher-profiles-design.md` (shell/profiles) and `docs/superpowers/specs/2026-09-25-biscuit-on-porthole-design.md` (Biscuit) at the repo root.

Pets Club is a touch-only, Tamagotchi-style dog companion for the Waveshare ESP32-S3-Touch-LCD-2.1, a round 480x480 capacitive-touch panel with no physical buttons. It is built for kids aged 5-10 who like reading and dogs: the pet grows over real calendar days, learns eight tricks over three lessons each, and is read twenty-plus original stories from a library that unlocks with daily gifts. Each profile on the device has its own pet, room and reading level (from the profile's age); Paw Street shows every profile's house. Everything renders into a 160x160 indexed-color framebuffer (32-color palette) that gets upscaled 3x to the physical panel. The pet never dies, never runs away, and never resets: stats decay toward floors, not through them. Its own brief, with the Save layout and reading-level tables: `games/pets-club/CLAUDE.md`.

Biscuit (landing with the Biscuit PRs) is a warm, doglike reading and discovery companion on the same panel, drawn full-color at native 480x480 instead of Pets Club's indexed look. Its product contract, module map and content limits: `games/biscuit/CLAUDE.md`.

It lives at `apps/porthole/` in the porthole monorepo (apps for this board sharing build/test/deploy infrastructure). Every path below is relative to `apps/porthole/`, and every command runs from there (or as `make -C apps/porthole <target>` from the repo root). The shared PlatformIO board base is `../../platform/waveshare-round.ini`. Read a game's own `CLAUDE.md` before touching it; this file covers what every game shares.

## Architecture map

`os/`, `shell/` and each game under `games/<game>/` are the platform-agnostic core. They compile unchanged into both the firmware and the host simulator (`-DHOST_BUILD` only changes what `main()` exists, not the game logic). The include path is `-Ios -Ishell -I.`: `os/` and `shell/` headers by bare name, a game's own headers by bare name (same directory), and a game's headers from anywhere else by path (`games/pets-club/game.h`). `os/` never includes anything from `shell/`, `games/` or `firmware/`; games include only `os/`; the shell knows games only through `os/app.h` (one exception: `shell/migrate.cpp` reads a game's old save layout once, for a one-time migration -- Pets Club's old houses today, Biscuit's old NVS blob once its migration lands).

In `shell/`:

- `profiles.h` / `profiles.cpp`: the profile record (`porthole/p<id>`, append-only with magic/version/size/crc, `MAX_PROFILES = 4`, currently version 2 / 52 bytes, `REC_V1_SIZE = 44` still loads), create/delete (delete erases `s<id>` in every game's namespace), the turn-taking rest budget (`SESSION_SEC`/`REST_SEC`, only with 2+ profiles), the daily play cap (`DAILY_SEC` = 25 min per profile per local day, across every game, kept as `(playDay, dayPlaySec)` rather than a deadline so a new day or a clock set back just resets it -- `playedToday()`/`restLeft()` read it, `shell::play()` updates it), `IDLE_MS` (a minute without a touch counts toward neither budget; also the firmware's first backlight-dimming step), `liftCaps` (called from `Shell::clockRestored()` when the firmware had to guess the clock after losing the RTC: a capped kid's `lastPlayed` never moves, so "today" can't be trusted -- every cap lifts, like a clock set back), and the `Store` interface both platforms implement. `host/test_shell.cpp` exercises it directly.
- `migrate.cpp`: first boot on a Pets Club device: house `crago/s<n>` becomes profile `n` (same slot, no save moves; the older `save` key is copied to `s0` only when no house exists). Runs until the marker `porthole/m` is written, so power loss mid-migration just re-runs it. `migrateBiscuit` does the same once for Biscuit's pre-Porthole save (`zoegotchi/pet3|pet2|pet1` or `biscuit/pet1` -> `biscuit/s<id>`, marker `porthole/mb`); it is not called until Biscuit is in `APPS` (landing with a later PR).
- `shell.h` / `shell.cpp`: the `Shell` class: picker, new profile (name, face, age, code), code pad, delete, launcher (mute lives here), rest screen ("Back in N min" for a turn-taking rest, or "Back tomorrow" once `playedToday()`, with no countdown), and hosting the open game (`App`): it writes the game's save at most every 5 s and on leaving, and gates the buzzer with the profile's mute. `dbg bedtime` (see the `tour` skill) uses up today's budget for testing; `debugPrint()`'s `today=` key reports seconds played today.
- `shell_sprites.h`: AUTO-GENERATED by `tools/shell_art.py` (avatars, plus, lock, sound icons).

Each game lives at `games/<game>/` and owns its own `CLAUDE.md` with its Save layout, content limits and any game-specific detail; this file only covers what every game shares.

A game is a directory under `games/`: the Makefile and PlatformIO build every `games/*/*.cpp`, so adding one means the directory, an `App` subclass, and a `static` instance appended to `APPS[]` in both `firmware/main.cpp` and `host/sim.cpp`; its `screenName()` values must not repeat another game's or the shell's (`tools/playtest.py` fails on a clash; prefix them). A host test is `apps/porthole/host/test_<name>.cpp` plus one `test_<name>_SRC :=` line in the Makefile -- host tests live in the shared `host/` directory, named per game, not inside the game's own directory.

- `games/pets-club/`: the shipped indexed-color game. Brief: `games/pets-club/CLAUDE.md`.
- `games/biscuit/`: the RGB565 game. Its rules (`pet.h`), save and legacy migration are landed; the screens, `game.h`/`game.cpp` and the RGB565 surface itself are still landing. Brief: `games/biscuit/CLAUDE.md`.

In `os/`, the shared runtime:

- `app.h`: the contract between the shell and a game (`App`, `Profile`, `SaveSlot`, `AppEnter`, `MAX_PROFILES`, and -- landing with Biscuit -- `enum Surface { SURFACE_INDEXED, SURFACE_RGB565 }` plus `App::surface()`, defaulted to `SURFACE_INDEXED` so Pets Club needs no change; polled every frame, so a game may switch per screen).
- `ui.h` / `ui.cpp`: the shared widgets (panel, button, icon button, the orange back button, toast, the name keyboard, `FreshGate`: every screen change ignores touches for 450 ms). Stay indexed even on an RGB565 game: launcher, picker, rest screen and this keyboard are always drawn by the shell.
- `crc32.h`: the checksum every persisted blob uses.
- `gfx.h` / `gfx.cpp`: the indexed-color renderer. A 160x160 indexed-color framebuffer (`gfx::fb`), drawing primitives, the 8x8 font, the round-mask test (`gfx::inCircle`), and the `UiAudit` hit-region log / text-box log that the `ui` sim command and the playtester agent read.
- `gfx565.h` / `gfx565.cpp` and `font.h` (landing with the Biscuit PRs): the native 480x480 RGB565 renderer for a `SURFACE_RGB565` game -- a `uint16_t*` target the host sets once per frame, flat-fill/blit/RLE-blit primitives, and a flat-table bitmap-font renderer (advance widths, kerning, word wrap) ported from Biscuit's bundled fonts. `gfx565::text` logs into the same `gfx::textLog` in logical px so `tools/playtest.py`'s UI audit covers both surfaces. See the spec's section A.
- `input.h`: turns raw touch samples into tap/long-press/drag edges (`InputTracker`); shared verbatim by firmware and host.
- `palette.h`: the fixed 32-color palette and the day/evening/night tint tables. Indexed games only; an RGB565 game picks its own flat colors.
- `font8x8_basic.h`: third-party public-domain font table, not normally touched.

`firmware/board.cpp`, `firmware/board.h`, `firmware/main.cpp` are the firmware, the only code that includes Arduino headers. `board.h` is the hardware contract (display present/init, touch, RTC, backlight, buzzer, NVS blobs addressed by namespace and key); `board.cpp` implements it for this specific board; `main.cpp` is the Arduino `setup()`/`loop()` that runs the `Shell` with the static `APPS` array over an NVS `Store`, restores the clock, dims the backlight, and parses serial commands. The 12 MB single-factory-app partition table (see below) already has room for Biscuit's assets. Landing with Biscuit: `board::presentHires()` draws a native 480x480 buffer straight to the panel for a `SURFACE_RGB565` game, alongside today's `present()` (160x160, upscaled 3x) for indexed games.

`host/` is the simulator and the tests: `sim.cpp` (the whole shell; SDL2 window, or headless `--script`/`--serve` modes; saves go to `build/host/<namespace>-<key>.sav`), `test_pet.cpp` (the Pets Club simulation self-check), `test_shell.cpp` (profiles, migration, delete, rest budget, daily cap) and `test_biscuit.cpp` (Biscuit's rules self-check) -- a new game's host test always lives here, as `host/test_<name>.cpp`, not inside the game's own directory. None of them touches `firmware/`.

`tools/` is dev tooling: `webemu.py` (serves `build/host/snap --serve` in a browser) and `playtest.py` (the playtest runner).

## Hard constraints

Every change must respect all seven. None of these are stylistic; each one maps to a real failure mode (a save that bricks on the next boot, a button a kid can't reach, a dog that "dies").

1. **Round screen.** The visible area is the circle of radius 80 around (80,80) in logical px (`gfx::CX`, `gfx::CY`, `gfx::R` in `os/gfx.h`; test with `gfx::inCircle(x, y[, margin])`). 1 logical px = 3 physical px = 0.33 mm (160x160 logical -> 480x480 physical). This applies at native 480x480 too: an RGB565 game's own `inCircle` is the same radius-240 circle around (240,240), just unscaled. Anything tappable must be at least 24x22 logical (8 mm) and lie entirely inside the circle: that is the exact size of the four home-screen buttons (`BTN_W=24, BTN_H=22` in `game.cpp`), not a rounded-up guideline. Text must never be cut by the bezel: before placing text at row `y`, compute the chord half-width `sqrt(80^2 - (y-80)^2)` and keep the text's full drawn width inside `[80 - halfwidth, 80 + halfwidth]`.
2. **Flat colors, and only the game's own font's glyphs.** This is per surface. An indexed (`SURFACE_INDEXED`) game: every framebuffer pixel is a palette index 0-31 (`Col` enum, `os/palette.h`) or `C_T` (255, transparent, sprites only) -- never a 33rd color or an out-of-palette blend -- and all text renders through the 8x8 font (`font8x8_basic.h`), ASCII 0-127 only: no curly quotes, no em/en dashes, no accented letters, anywhere a kid sees it. An RGB565 (`SURFACE_RGB565`) game (landing with Biscuit): its art is flat fills and pre-rendered scenes, with no runtime blending beyond what its art tool already baked in -- text is the one exception, blending each glyph's 4-bit coverage against the pixel underneath at draw time -- and every drawn string is limited to its bundled font's actual glyph set -- a content gate rejects anything outside it, the same discipline as ASCII-only, just against a bigger table. Both keep the round-screen and 8 mm tap-target rules above.
3. **The buzzer is harsh.** `board::buzzer()` / `Shell::soundOn()` (the open game's `soundOn()`, gated by the profile's mute) drive a single-tone piezo. Never wire a sound to routine input (taps, navigation): only to rare, meaningful moments (growth, a trick learned, a gift, a sticker, end of a minigame), and only through the existing `Sound` enum in `game.h`. There is a per-profile mute toggle on the launcher; every sound path goes through the shell, which respects it. (Landing with Biscuit: it ships with no sounds at all, by design -- see its own brief.)
4. **A persisted layout is append-only.** Every game's `Save` struct and the shell's profile `Record` (`shell/profiles.h`, `porthole/p<id>`) are raw byte blobs with a magic/version/size/crc header, one per profile, written by the shell (NVS namespace per game, key `s<profile id>`). To add a field: put it immediately before `crc`, bump the version constant, and extend the loader so every older blob size/version still loads. Never reorder or resize an existing field: that corrupts every save already written to a device, silently. Each game's own `CLAUDE.md` names its exact fields and the migration pattern already in use there.
5. **No game punishes absence.** Pets Club: every stat decays toward a floor and stops (`pet::FLOOR_FOOD`, `FLOOR_FUN`, `FLOOR_ENERGY`, `FLOOR_CLEAN` in `games/pets-club/pet.h`; currently 20/25/20/20), no game over, no running away, no forced reset; time away is capped at `pet::OFFLINE_CAP_SEC` (12 hours) of decay, and overnight hours count as sleep. Biscuit's contract (see its brief) is the same idea stated more broadly: no death, no guilt for absence, no expiring friendship or streaks, no lost progress. Neither game reads a weekend offline as neglect.
6. **Built for a 5-10 year old.** Short, common words in whichever glyph set the surface allows (constraint 2). Failure is gentle: a wrong answer or a miss shows the right thing and moves on; it never blocks progress or resets state. No dead ends: every screen needs an always-reachable way back (`Game::drawBackButton`/`backButton` or an equivalent visible exit); check this for any new screen.
7. **Verify in the simulator before calling it done.** The physical device may be unplugged: that is not an exemption. Every gameplay or UI change needs at least one of: `make test` (each game's simulation self-check), `make snap` plus a script (see the `tour` skill), or `make playtest` (see Workflows below). No verification behind it means it is not done.

Note on constraint 1 and automated checks: `make playtest`'s UI audit fails a run on any finding. It still labels a target under 18 logical px (6 mm) FAIL and one under the real 24x22 (8 mm) box WARN, but warnings are errors: a WARN fails the run just like a FAIL.

Note on constraint 2 and content: Pets Club's `games/pets-club/tools/check_content.py` re-renders every string through the real 8x8 font and word-wraps it into the exact box the game draws it in -- the source of truth, not a character count. Biscuit's `games/biscuit/tools/check_content.py` (landed with #33) checks the glyph set, token validity, counts and ids today; it is a character-count stand-in for the real pixel-fit check until `generated/fonts.h` lands with a later PR. See the `content` skill.

## Workflows

### make targets (Makefile)

The repo root and CI call these by name; keep them working:

- `make lint`: app-specific static checks: `games/pets-club/tools/check_content.py`, `games/pets-club/tools/art.py --check`, `tools/shell_art.py --check`, `games/biscuit/tools/check_content.py`, and a `-Werror` host build of the shell, the games and the sim. (ruff and shellcheck run from the repo-root pre-commit, not here.) Biscuit's art tool's `--check` also runs here, skipped with a message where `node` is missing (CI's runner always has it).
- `make check`: `lint` + `test`. What pre-commit runs; a few seconds.
- `make ci`: `check` + `playtest` + `coverage`, everything CI runs for this app except the firmware build.
- `make coverage`: builds every `test_*` binary and `snap` with `--coverage` under `build/coverage/`, runs all the tests plus the non-monkey playtests, and writes line coverage of `os/*.cpp` and `games/*/*.cpp` to `build/coverage/coverage.xml` (Cobertura, via gcovr). Fails below `COV_MIN` in the Makefile. On macOS gcovr uses `xcrun llvm-cov gcov`; elsewhere `gcov`. Uses `gcovr` from PATH, else `uvx gcovr`.
- `make firmware` / `make flash [PORT=...]` / `make monitor [PORT=...]`: `pio run -e firmware`, upload, and the 115200-baud serial monitor. PlatformIO auto-detects the port when `PORT` is not given.

Development targets:

- `make sim`: SDL2 window. Mouse = finger. Keys: `h` +1 hour, `n` +8 hours, `d` +1 day, `r` reset save, `s` screenshot to `build/host/shot.bmp`, `p` print stats to stdout, `q`/Esc quit.
- `make snap`: headless binary, no SDL, two modes:
  - `./build/host/snap --script FILE` runs a command script (see the `tour` skill) and writes named `.bmp` files to `build/host/`.
  - `./build/host/snap --serve` speaks the stdin/stdout protocol `tools/webemu.py` drives.
  - Both accept `--now EPOCH` (fixed clock) and `--fresh` (wipe saves first); the default clock is a fixed Tuesday 16:00 local time, so unscripted snapshots are still deterministic.
- `make snap-asan`: the same headless binary built with AddressSanitizer + UBSan. Used by the chaos-monkey playtest scenarios; slower, build it only when you need sanitizer coverage.
- `make test`: builds and runs every `host/test_*.cpp` the Makefile auto-discovers, each against its own `test_<name>_SRC :=` source list: `test_pet` (`os/*.cpp`, Pets Club's `pet.cpp`), `test_shell` (`shell/profiles.cpp`, `shell/migrate.cpp`, Pets Club's `pet.cpp`), `test_biscuit` (Biscuit's rules -- header-only, no extra sources), `test_biscuit_content` (`games/biscuit/personalize.cpp`, `games/biscuit/art.cpp`: `personalize()` and the generated art tables, not content wording -- that's the content gate's job). Pure logic self-checks, no rendering. Must stay green.
- `make webemu`: depends on `snap`; runs `tools/webemu.py`, serves the game at `http://127.0.0.1:8765` (`python3 tools/webemu.py 8766` for another port). The buttons under the canvas skip time and force debug states for testing.
- `make art`: runs `games/pets-club/tools/art.py`, `tools/shell_art.py`, and Biscuit's Node-based art tool (`node games/biscuit/tools/art/export-assets.mjs`, Node 22+, zero npm deps), regenerating `games/pets-club/sprites.h`, `shell/shell_sprites.h` (and, if Pillow is installed, preview sheets under `build/art/`), and `games/biscuit/generated/{scenes.h,discovery_art.h,art_data.inc,manifest.json}` from committed JS sources. All have `--check`, which `make lint` runs (Biscuit's skipped with a message if `node` is missing) -- see the `art` skill.
- `make playtest`: depends on `snap` and `snap-asan`; runs `python3 tools/playtest.py` against every scenario in `tests/playtests/*.txt`, writes `build/playtest/report.md` plus a contact-sheet PNG per scenario. See the `playtest` skill for how to read the report.
- `make clean`: removes `build/` (not `.pio/`, the firmware build cache).

### Firmware build and flash

```bash
make firmware                                  # pio run -e firmware
make flash                                     # or: make flash PORT=/dev/cu.usbmodemXXXX (Linux: usually /dev/ttyACM0)
make monitor
```

`platformio.ini` extends `[waveshare_round]` from `../../platform/waveshare-round.ini` (platform pin, board, PSRAM and USB flags) and adds only the app specifics (source dirs, partition table, upload speed, warning flags, `-Ios -Ishell -I.`). The env is `firmware`. The partition table is `partitions.csv`: nvs at 0x9000 (unchanged from PlatformIO's default, so saves survive a reflash), otadata, one 12 MB factory app at 0x10000 (`board_upload.maximum_size`), coredump. No OTA slots.

See the `flash` skill for the PlatformIO-venv Python-dependency caveat and how to set the clock.

### Serial commands (`main.cpp` `loop()`, 115200 baud)

| Command | Effect |
| --- | --- |
| `S` | Print the active profile and the open (or last) game's stats (`Shell::debugPrint()`) plus free heap and current epoch |
| `T<epoch>` | Set the RTC and running clock to `<epoch>` (local wall-clock seconds; ignored if <= 1,600,000,000) |
| `R` | Erase every profile and every game's saves (namespaces `porthole` and each app's store), then reboot |
| `P<n>` | Clear profile `n`'s 4-digit secret code (parent escape hatch) |
| `D` | Toggle touch-position logging |

### Hooks and CI

Hooks are managed by the repo-root pre-commit config, not by this app: it runs ruff and shellcheck repo-wide and `make -C apps/porthole check` for this app. CI runs `make ci` plus the firmware build. Run `make ci` yourself before pushing anything that touches gameplay or UI; a red CI run is a worse time to find out.

### Skills

Project skills live under the repo-root `.claude/skills/` (agents under `.claude/agents/`), with paths written as `apps/porthole/...`: `playtest`, `flash`, `art`, `content`, `feature`, `tour`. Each is a short, imperative how-to for its workflow, generalized across every game; a game-specific detail (reading levels, a content gate's exact box sizes) lives in that game's own `CLAUDE.md`, which the skill points to. Load the matching skill instead of re-deriving the steps.

## Delegation table

| Task | Agent | Model |
| --- | --- | --- |
| Mechanics, progression, daily hooks, tuning numbers | `game-designer` | opus |
| Implement a screen or mechanic in `os/`, `games/` or `host/` | `game-engineer` | opus |
| Board bring-up, pins, PlatformIO, flashing | `firmware-engineer` | opus |
| New or changed pixel art, icons, sprite poses | `pixel-artist` | opus |
| A new story, spelling-word batch, or (Biscuit) discovery | `story-writer` | sonnet |
| UX/QA pass on a screen or flow | `playtester` | opus |
| Review before a push | `reviewer` | opus |

## Model policy

**opus** for anything that writes or reviews code, or judges design/polish: `game-designer`, `game-engineer`, `firmware-engineer`, `pixel-artist`, `playtester`, `reviewer`.
**sonnet** for prose and bounded research: `story-writer`.
**haiku** for mechanical sweeps (renames, formatting, single-file lookups): no standing agent above is haiku; reach for it directly, ad hoc, for that class of task.

State the model when dispatching an agent (for example: "dispatching `game-engineer` (opus) to implement this"). Between sonnet and opus for anything code-shaped, take opus.
