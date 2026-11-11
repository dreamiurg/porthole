# Porthole launcher: Home, Library and per-device games

Status: approved by the parent on 2026-11-11 ("follow your recommendations"). Proposal page with mockups:
https://claude.ai/artifact/V4MSJvsG7mLS2NqL4Hr441

## Goal

Each kid gets a Home screen with only their favorite games; every game lives in a Library; a grown-up can switch a
game off for the whole device. One official firmware serves everyone (OTA ships one image), so the choice of games
lives on the device, not in the build. This is what makes Porthole configurable per device for open-source users.

## Decisions (the four questions, all as recommended)

1. The kid picks their own favorites (star mode in the Library). No holds anywhere.
2. Home holds up to 5 favorites + an "All games" tile (one page, no arrows).
3. Admin -> Games (per-device on/off) ships in the same change.
4. Home order = the order the stars were set; no drag reordering (unstar and star again to move).

## Screens

- **Home** (`launcher`, the kid's): the existing header (face + name, sun button), up to 5 favorite tiles in the
  existing 3x2 grid geometry (`launcher::tile`), the 6th slot always "All games" (a cream tile with a 3x2 grid of
  small colored squares, drawn in the shell palette). Fewer than 5 favorites: the row is centered, as the short last
  row is today. No page arrows. Pressing a tile shows its name in the caption, as today. If 6 or fewer games are
  enabled on the device, there is no Library: Home shows all enabled games, exactly as today's single page.
- **Library** (`library`): today's launcher grid of every enabled game, 6 a page, page arrows. A gold star in a tile's
  corner marks a favorite (decoration, not a target). The header row shows the caption "All games"; the sun button's
  place holds a star button that enters star mode. The top home button returns to Home. A new game (an id the kid
  has never seen) shows a small "new" dot until first opened.
- **Star mode** (same screen, `library` with a mode): header caption "Pick 5 · N" (N = places left); the star button
  becomes a green check (Done). Tapping a tile toggles its star; a 6th star is refused with a gentle wiggle of the
  counter; tiles without a star are dimmed. Games don't open in star mode. Done (or the home button) saves and
  returns to the Library.
- **Admin -> Games** (`sys_games`, grown-ups, native adult typography like the rest of Admin): one row per game in
  the firmware with its picture, name and an on/off switch; pages of 4. Off = hidden from every kid's Home and Library;
  saves untouched. At least one game must stay on (the last switch can't turn off). Saves only on change, read back.

## Data (append-only rules hold)

- **Stable game id:** each `App` gets a 1-byte id that never changes or gets reused (Pets Club 1, Biscuit 2, ...),
  kept next to `APPS[]` in `games/apps.h`; a host test asserts ids are unique and non-zero.
- **Favorites per kid:** NVS `porthole/f<profile id>`, a versioned blob {magic, version, size, count, ids[5],
  crc}. Missing or invalid = default. Profile delete and create erase it (like the game saves).
- **Device games:** NVS `sys/games`, a versioned blob with a 32-bit "off" mask by id + crc. Missing = all on.
- **Default favorites:** the first 5 enabled games in launcher order, so an updated device's Home looks like today's
  page 1 with "All games" in the 6th place.
- Unknown ids (a game removed from the firmware) are skipped silently. Disabled favorites are hidden, not removed, so
  switching the game back on restores the star.

## Rules carried over

Round screen and 24x22 targets; the shell palette and 8x8 font on kid screens; no sounds; every screen has a way back;
a long press acts like a tap (os/input.h); FreshGate on every screen change; profile `Record` (52 B) unchanged.

## Verification

Host tests (favorites blob, games mask, defaults, unknown ids, all-off refused, delete erases f<id>), layout tests
for Home with 0-5 favorites, playtest scenarios (Home, Library paging, star mode, Admin Games), `make ci`,
`make complexity`, firmware build, /simplify + /code-review, an independent kid playtest (8-year-old) of Home,
Library and star mode, and a grown-up playtest of Admin -> Games.
