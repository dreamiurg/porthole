# Porthole shell: profiles and an app launcher

Status: approved in conversation 2026-09-24. Owner: Dmytro (PM). Device firmware name: **Porthole**.

## Goal

One device, several kids, several games. Power on -> "Who's playing?" -> that kid's launcher -> a game.
Each kid has a profile (name, avatar, age, optional 4-digit PIN). Every game keeps separate progress per
profile. Biscuit will be rewritten later on the same runtime; this spec covers the shell and Pets Club.

## Decisions

- **One firmware image.** Games are C++ modules compiled into one binary on a shared runtime (the
  Pets Club renderer: 160x160 indexed framebuffer, 32-color palette, 8x8 ASCII font, `InputTracker`).
  Switching games or profiles is a screen change, no reboot.
- **Neutral profile picker.** Paw Street stays inside Pets Club as the neighbourhood view: it shows a
  house for every profile on the device, the kid enters only their own; other houses show that
  profile's dog in the window (if that profile has adopted one). House switching, house creation and
  per-house PINs leave Pets Club.
- **`MAX_PROFILES = 4`.** One persisted record per profile, so raising it later needs no migration.
- **Rest timer is per profile, across all games.** Same numbers as Pets Club today (`SESSION_SEC`
  6 min play, `REST_SEC` 10 min rest), same rule that it only applies when the device has 2+ profiles.
- **Partition table and NVS namespaces do not change.** Pets Club keeps namespace `"crago"`, keys
  `s0..s3`. Flashing Porthole over an existing Pets Club device must keep every kid's pet.
- Out of scope: parent area, daily limits, bedtime, Biscuit rewrite, network.

## Layout

`apps/pets-club/` moves (git mv, history kept) to `apps/porthole/`, which is now the unit the monorepo
contract (Makefile targets, CI job, pre-commit hooks, release tags `porthole-vX.Y.Z`, installer card)
applies to. Pets Club has no standalone firmware anymore.

```
apps/porthole/
  os/                 shared runtime every game compiles against (platform-agnostic)
    gfx.* input.h palette.h font8x8_basic.h app.h
  shell/              profiles store + migration, picker, profile creation, launcher, rest screen
  games/pets-club/    pet.* game.* content*.h sprites.h (+ its tools: art.py, check_content.py)
  firmware/           board.* main.cpp (only code that includes Arduino headers)
  host/               sim.cpp (runs the whole shell), test_pet.cpp, test_shell.cpp
  tools/              playtest.py webemu.py
  tests/playtests/
  Makefile platformio.ini app.json README.md CLAUDE.md
```

Games include `os/` headers only; `os/` includes nothing from `shell/` or `games/`. The shell knows
games only through `app.h` and a static array in the firmware/sim entry point.

## Contract (`os/app.h`)

```cpp
struct Profile { uint8_t id; char name[12]; uint8_t avatar, age, muted; };   // what games see
struct SaveSlot { const void* data; size_t len; };                             // len 0 = none

class App {
 public:
  virtual const char* name() const = 0;          // launcher label
  virtual const gfx::Sprite& icon() const = 0;   // launcher icon, from the game's own sprites
  virtual const char* store() const = 0;         // NVS namespace; keys are "s<profile id>"
  // saves[i] belongs to all[i]; the game may read other profiles' saves (Paw Street) but only writes `who`'s.
  virtual void enter(const Profile& who, const Profile* all, const SaveSlot* saves, int n, uint32_t nowSec, uint32_t ms) = 0;
  virtual void update(uint32_t nowSec, uint32_t ms, const Input& in) = 0;
  virtual void render() = 0;
  virtual Tint tint() const = 0;
  virtual bool asleep() const = 0;               // backlight dimming policy
  virtual bool soundOn(uint32_t ms) = 0;         // the shell gates this with the profile's mute
  virtual bool takeSave(const void** data, size_t* len, bool allowed) = 0;
  virtual bool wantsHome() = 0;                  // back pressed on the game's top screen
  virtual void leave() = 0;                      // flush state; next takeSave returns the final save
};
```

The shell owns persistence: it calls `takeSave` every frame (throttled to once per 5 s, flushed on
`leave`) and writes `<store>/s<id>`. On profile delete it erases `<store>/s<id>` for every app.

## Profiles

Persisted as `porthole/p<id>`, one blob per profile, append-only like `Save`:
`magic, version, size, name[12], avatar, age, muted, reserved, pin(u16), reserved, restUntil, playSec,
lastPlayed, crc` (44 bytes, `static_assert`ed). `pin` is the typed code + 1, so 0 means none and 0000 is a
real code. Ids are stable slots `0..3`; a deleted id is reused by the next new profile. Delete erases every
app's `s<id>` first and the record last; create erases any `s<id>` left behind for the id it takes.

**Migration (first boot of Porthole on a Pets Club device):** runs on every boot until the marker key
`porthole/m` exists. House `crago/s<n>` becomes profile *n* (the id is the original slot: no compaction,
no save moves, gaps stay gaps): `name=kidName`, `age=kidAge`, `pin` (Pets Club's stored value *v* > 0
becomes code *v*; Pets Club stored 0000 as 1, so a kid whose code was 0000 types 0001 afterwards),
`muted`, `restUntil`, `playSec`, `avatar=n`. Only when no `s*` house loads, the pre-house `crago/save`
key is copied to `s0` (before its record). Order: every `p` record, then the marker, then erase
`crago/save` unconditionally. Every write is idempotent, so power lost anywhere re-runs the same
migration on the next boot; once the marker exists nothing migrates again (deleting every profile, or
corrupt records, never resurrects the houses). The old fields stay in `Save` (append-only) but Pets
Club no longer reads them for anything but migration. Covered by `test_shell` with real v1 and v2
`Save` blobs, a gap (`s0`, `s2` -> `p0`, `p2`), power loss after each write, delete-all, and corrupt
records with the marker present.

## Screens (shell)

All screens obey the round-screen rules (targets >= 24x22 logical, fully inside the circle, text inside
the chord) and have a visible way back.

- **Who's playing?** A vertical list: one row per profile (avatar + name) and a "+" row while fewer
  than 4 profiles. Tap a row -> PIN pad if the profile has a PIN -> launcher (or Rest screen if
  resting). Long-press a row -> PIN if set -> "Delete <name>?" confirm. Boot always lands here; with
  zero profiles it opens creation directly.
- **New profile:** name (existing keyboard) -> avatar grid -> age (existing age picker) -> optional
  PIN (set twice or Skip). Back on each step; back on the first step returns to the picker.
- **Launcher:** header with the kid's avatar + name (tap -> picker), one icon per game, a mute toggle.
  With one game it still shows the launcher.
- **Rest:** the existing Pets Club rest screen, generalised: "<name> is resting", minutes left, back to
  picker. When the session budget runs out mid-game the shell calls `leave()`, saves, and shows Rest.
- Leaving a game: `wantsHome()` -> `leave()` -> save -> launcher.

Avatars: 8 pixel-art portraits drawn with `tools/art.py`-style generation into a generated header
(never hand-edited), size chosen to fit a list row (about 20x20). Pets Club launcher icon: about 32x32.

## Pets Club changes

- Drop `SC_STREET` switching/creation, `SC_PIN_SET`, `SC_PIN_ENTER`, `SC_NAME_KID`, `SC_AGE` and
  `SC_REST` from the game; adoption becomes intro -> name pet -> theme. Kid name/age come from `Profile`.
- Paw Street becomes read-only neighbourhood view reachable from home (as today), back returns home.
- Back on `SC_HOME` -> `wantsHome()`. The stats-screen mute toggle is removed; mute lives on the
  launcher and the shell gates the buzzer.
- Functions touched that are listed in `whitelizard.txt` are split and their lines deleted
  (repo rule); moved files get their paths updated, no new entries.

## Serial commands (firmware)

`S` stats (+ active profile/app), `T<epoch>` clock, `R` erase every profile and every app store and
reboot, `P<n>` clear profile n's PIN, `D` touch log. Same semantics as today, profile instead of house.

## Verification

- `make -C apps/porthole check ci` green: `test_pet`, `test_shell` (profiles, migration, delete),
  lint (content gate, `art.py --check`, `-Werror` build), playtests, coverage (threshold not lowered).
- Playtests: existing Pets Club scenarios gain a preamble (`profile NAME AGE` + `app pets-club`
  sim commands, or `--profile`/`--app` flags); `10_houses` becomes profile scenarios: create 3,
  PIN gate, delete, rest timer across games, 4-profile limit. UI audit: zero FAIL, zero target-size WARN
  on new screens.
- `pio run -e firmware` builds. On-device check (when the board is free): migration of the kids' real
  saves, picker -> Pets Club -> back, power cycle lands on picker.

## Delivery stages

1. **Move + extract (no behaviour change).** `apps/pets-club` -> `apps/porthole` layout above; board
   storage generalised to (namespace, key); CI, pre-commit, `whitelizard.txt` paths, `.claude` agents
   and skills, README/CONTRIBUTING/docs updated. Playtest snapshots identical to before.
2. **Shell + contract + Pets Club as an App**, with placeholder avatars.
   In parallel: **art** (8 avatars, Pets Club icon) by pixel-artist once stage 1 lands.
3. **Playtest + review**, fix findings, PR.
