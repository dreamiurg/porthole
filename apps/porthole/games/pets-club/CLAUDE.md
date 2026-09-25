# Pets Club: game brief

The shared app brief (`apps/porthole/CLAUDE.md`) covers what every game shares: the
round-screen and tap-target rules, the append-only save-layout rule, the buzzer, and the
per-app workflow. This file has what's specific to Pets Club: the `Save` layout and the
age-based content tables.

## Save layout

`Save` (`pet.h`) is a raw byte blob, one per profile, NVS namespace `"crago"`, key
`s<profile id>`, written by the shell. Magic `0x4F475243` ("CRGO"), `version`, `size`,
fields, then `crc` last -- the append-only rule in the app brief applies exactly here.

`SAVE_VERSION` is currently 2. The v1 -> v2 migration is the reference pattern for any
future bump: `SAVE_V1_SIZE = 140` marks the pre-v2 blob size; `pet::loadBlob` checks for
that exact size, reads the v1 fields (everything up to `SAVE_V1_SIZE - 4`), validates the
v1 CRC, and zero-fills the new fields (v2 added houses, reading level and turn-taking --
zero means "not asked yet" for `kidAge`, "none" for `pin`, and so on). `host/test_pet.cpp`
has the round-trip case for this; copy its shape for the next bump. To add a field: put
it immediately before `crc`, bump `SAVE_VERSION`, extend `loadBlob`, add a test case.
Never touch `SAVE_V1_SIZE` or reorder/resize an existing field.

Since Porthole, the shell's profile owns the kid's name, age, secret code, mute and rest
budget; `Save`'s own `kidAge`, `pin`, `muted`, `restUntil` and `playSec` fields are only
read once, by the shell's migration (`shell/migrate.cpp`) converting an old Pets Club
house into a profile. They stay in the struct because the layout is append-only, but
`game.cpp` should treat the profile, not these fields, as the source of truth going
forward.

## Reading level and word length by age

`pet::levelRange(age, lo, hi)` and `pet::wordLenRange(age, lo, hi)` (`pet.cpp`) drive both
the story library (`pet::bookListFor`) and the spelling round:

| Age | Story levels | Spelling word length |
| --- | --- | --- |
| <= 6 | 1 only | 3-4 |
| 7 | 1-2 | 4-5 |
| 8 | 2-3 | 5-7 |
| >= 9 | 3 only | 6-8 |

`bookListFor` widens down a level if the exact band doesn't have at least
`STARTER_BOOKS + 3` stories yet, so a new level's content doesn't need to hit that count
immediately -- but keep it in mind when deciding how many stories a new level needs
before it stops falling back.

See the `content` skill for how new `Book`/`WordClue` entries are written and checked
against these bands.
