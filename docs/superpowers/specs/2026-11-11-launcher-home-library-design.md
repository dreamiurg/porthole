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
3. Info -> Games (per-device on/off; first on Admin, moved after the grown-up playtest, M2) ships in the same change.
4. Home order = the newest star first; no drag reordering (unstar and star again to bring a game to the front).
   Amended 2026-11-11 after the kid playtest (`apps/porthole/docs/launcher-home-playtest.md`, m1): first-starred-first
   buried a re-starred game at the end and an accidental double tap moved a game out of place; "what I just picked
   shows first" is the rule a kid can see.

## Screens

- **Home** (`launcher`, the kid's): the existing header (face + name, sun button), up to 5 favorite tiles in the
  existing 3x2 grid geometry (`launcher::tile`), the 6th slot always "All games" (a cream tile with a 3x2 grid of
  tiny game pictures, the launcher icons downscaled in code by nearest neighbor, the games not on Home first; the
  first design's colored squares read as a painting game in the kid playtest, m2). Fewer than 5 favorites: the row is centered, as the short last
  row is today. No page arrows. Pressing a tile shows its name in the caption, as today. If 6 or fewer games are
  enabled on the device, there is no Library: Home shows all enabled games, exactly as today's single page.
- **Library** (`library`): today's launcher grid of every enabled game, 6 a page, page arrows. A gold star in a tile's
  corner marks a favorite (decoration, not a target). The header row shows a plain "All games" title (no border, so
  it does not look like Home's switch-kid button); the sun button's place holds a star button that enters star mode,
  and a finger on it shows "Pick games" over the header, as a game's name shows. The top home button returns to
  Home. A new game (an id the kid has never seen) shows a small "new" dot until first opened; "All games" wears one
  while a game that is not on Home is new (one on Home already wears its own).
- **Star mode** (same screen, `library` with a mode): the sky behind the grid turns lavender and every tile jiggles
  (1 px right and back, neighbors in turn, like a phone's edit mode), so a non-reader sees that something is different.
  Five small stars in the header count Home's places (gold = taken, white = free; no number to read); the star
  button becomes a green check (Done). Tapping a tile toggles its star; repeated taps on one tile within 400 ms of
  each other count once (a double tap or mashing). Every tile wears a star: gold on Home, an empty one to pick (nothing is dimmed:
  dimmed tiles read as switched off). A new kid's Home is already full (the defaults), so a 6th star is refused with
  a lasting cue: "Home full!" in yellow over the header for 2 s, with "Tap a star", or "Stars: page N" when the page
  shown has no gold star, while the gold stars hop and so does the arrow toward a page that holds one, so a
  non-reader sees where to tap. No sound. Games don't open in star mode. Changes apply only on Done, which saves
  them (only if anything changed, read back) and returns to the Library; with no star left Home shows the
  default favorites again (see Data), never an empty Home; leaving any other way (the home button,
  which returns to the Library, the rest screen, a restart) discards that visit's edits. (Amended after the kid
  playtests. Round 1 (`apps/porthole/docs/launcher-home-playtest.md`), M1, n1, n2: star mode read as "switched off",
  so nothing is dimmed and a refused star got a lasting cue. Round 2 (`launcher-home-playtest-2.md`), 1-4: a
  6-year-old entered star mode unnoticed and mashed his Home from 5 games to 2, which stuck after a restart; hence
  the lavender sky and jiggle, edits kept only by Done, the star counter, "Stars: page N" and the double-tap guard.
  Round 3 (`launcher-home-playtest-3.md`), 1: a non-reader emptied his Home and could not refill it; hence Home's
  default when none of the kid's favorites shows. Round 4 (`launcher-home-playtest-4.md`), M1: after a check with no
  stars Home and the Library showed the defaults gold while star mode showed them white, and one new star cut Home
  to that game; resolved by the Library's gold stars always being the kid's own list and the check with no star left
  and none hidden resetting the kid to "not picked", so all three show the default gold.)
- **Info -> Games** (`sys_games`, grown-ups, native adult typography like the rest of Admin): a Games button on Info,
  between Wifi and Updates (the grown-up playtest, M2, found it buried on Admin page 2; Info's Radar button now reads
  "Radar home" so it is not taken for the kids' Home). One row per game in the firmware with its picture, name and an
  on/off switch; pages of 3, with the line "Off only hides a game; saves stay." (M1). Off = hidden from every kid's
  Home and Library; saves untouched. At least one game must stay on (the last switch can't turn off). Saves only on change, read back.

## Data (append-only rules hold)

- **Stable game id:** each `App` gets a 1-byte id that never changes or gets reused (Pets Club 1, Biscuit 2, ...),
  kept next to `APPS[]` in `games/apps.h`; a host test asserts ids are unique and non-zero.
- **Favorites per kid:** NVS `porthole/f<profile id>`, a versioned 24-byte blob {magic u32, version u16 (1), size
  u16, count u8, picked u8, ids[5] u8, reserved u8, seen u32, crc u32}. `picked` 0 = the kid never starred anything
  (Home shows the default); `ids` is Home's order, the newest star first; `seen` is a bit per game id the kid has
  seen. Missing or invalid = default. A refused star never changes the blob, not even turning the default into a
  list. Profile delete and create erase it (like the game saves).
- **Seen baseline on first Home:** when a kid's `f<id>` is missing, their first Home writes it with `seen` = every
  game in the image, so only games added by a later update wear a "new" dot. It is written only when the key is
  missing: a damaged blob or a newer firmware's (after a USB rollback) is kept as it is and the default shows in
  RAM, so going forward again finds the newer favorites; nothing writes it that session (star mode works in RAM and
  its check saves nothing; a game opened is marked seen in RAM only). Opening a game marks it seen.
- **Device games:** NVS `sys/games`, a versioned blob with a 32-bit "off" mask by id + crc. Missing = all on.
- **Default favorites:** the first 5 enabled games in launcher order, so an updated device's Home looks like today's
  page 1 with "All games" in the 6th place.
- Unknown ids (a game removed from the firmware) are skipped silently. Disabled favorites are hidden, not removed, so
  switching the game back on restores the star; when a new star needs the place, the oldest hidden one gives it up.
  When none of a kid's favorites shows (all switched off or gone, or every star taken off), Home shows the default
  and the list is kept as it is, so the hidden ones come back with their games (the Library then shows no gold
  star: its stars are always the kid's own list; Done with none left and none hidden resets the list to "not
  picked", the default). Star mode never uses that fallback:
  it edits the kid's own list, so it can show no gold star at all and a new star is then accepted (a swap by
  unstarring then starring works), and hidden favorites give their places up only to new stars, oldest first.

## Rules carried over

Round screen and 24x22 targets; the shell palette and 8x8 font on kid screens; no sounds; every screen has a way back;
a long press acts like a tap (os/input.h); FreshGate on every screen change; profile `Record` (52 B) unchanged.

## Verification

Host tests (favorites blob, games mask, defaults, unknown ids, all-off refused, delete erases f<id>), layout tests
for Home with 0-5 favorites, playtest scenarios (Home, Library paging, star mode, Info Games), `make ci`,
`make complexity`, firmware build, /simplify + /code-review, an independent kid playtest (8-year-old) of Home,
Library and star mode, and a grown-up playtest of Info -> Games. After the fixes: `b2_home_stars` replays the kid
report's path (fresh profile -> All games -> arrow -> star -> Rally: "Home full! / Stars: page 1"), the newest-first
order, a double tap acting once, the check keeping a picked Home (3 favorites, a tap opens a game) and the home
button dropping star mode's edits, and an emptied Home coming back as the default; `starMode` the same with the
double tap and a restart, `emptyHomeIsDefault` the check with no star, `hiddenFavorites` favorites switched off and back on, `swapAndHidden` star mode on the kid's own list; `starRules` covers a refused
star on the default and the oldest of two hidden favorites giving its place up, `favoritesKept` the baseline written
only when missing; kid playtests 3 and 4 (`apps/porthole/docs/launcher-home-playtest-3.md`, `-4.md`) replayed
star mode, and b2 asserts `home_gold` (the Library's gold stars) at 0 in star mode and 5 after a check with none.
