# Times Cards: game brief

The times tables as flash cards, one deck per kid, scheduled by spaced repetition. A card shows "7 × 8"; the kid says
the answer out loud and taps the card; the answer shows, and the kid says how it went: **Easy**, **Hard** or
**Wrong**. The deck decides when each card comes back and when the next table opens. Built for two kids practising a
few minutes a day: a 6-year-old starts on 2×, an 8-year-old on 4×.

The shared app brief (`apps/porthole/AGENTS.md`) covers what every game shares. This file has Times Cards' pages, its
rules and its save.

## Pages

Every page has the house at the top: from a card it goes back to the deck, from the deck out to the launcher.

- `cards_home`, the deck: the table the kid is on, big on an index card, a dot for each of its ten facts (green known,
  amber learning, pale new), "Say it, then tap the card" (or "Every table known!") and **Go!**; a tap on the card deals
  too.
- `cards_ask`: "k of n" (or "One more try" on a missed card's second look) and the card, "a × b". The card sinks under a finger; a tap turns it.
- `cards_answer`: "a × b =" small and the answer big, with **Easy**, **Hard** and **Wrong**.
- `cards_done`: "Perfect round!" (no Hard or Wrong), "Great job!", or "Next up: N ×!" when the round opened a new
  table; "+2 stars", the round's three tallies, the table's dots and
  **Again**, which deals the next round.

Every finished round pays `ROUND_STARS` (2) into the kid's shared stars (`os/stars.h`, `AppEnter::stars`, spent in
Biscuit's doghouse), whatever the grades, so an honest Wrong costs nothing. A round left half-way pays none. The shell
keeps and saves the balance; this game never stores it.

No sound (`soundOn` is always false), no timer, nothing lost: a Wrong card only comes back sooner.

## Rules (`deck.h`, header-only, checked by `host/test_times_cards.cpp`)

- A card is one fact, table × n, tables 1..10, n 1..10: 100 cards. Its Leitner box says how well the kid knows it: 0
  never shown, 1 learning, up to 7. A box waits `INTERVAL` days before the card is due again (1, 2, 4, 7, 14, 30, 60).
- **Easy** on a card that is new or due moves it one box up (a new card known at once goes to box 2). **Hard** keeps
  its box and brings it back tomorrow. **Wrong** puts it back in box 1, due tomorrow, and shows it again three cards
  later in the same round, once (headed "One more try", not counted in "k of n"; a round grows by five such cards
  at most). A fact still being learned (box 1 or 2) also moves up on Easy any day after
  the one it was last seen; a known card answered before it was due never moves up.
- **Mastery:** a fact is known once it reaches `MASTER_BOX` (3): Easy on two separate days. **Level-up:** when all ten
  facts of the newest table are known, the next table opens and its facts start coming in as new cards; the older ones
  keep coming back for review.
- **Start:** from the profile's age, `startTable(age) = clamp(age - 4, 1, 4)`. The tables below start in box 5, spread
  over the next two weeks, so a few come back for review each day.
- **A round** is up to ten cards: due cards first (oldest first), leaving room for up to four new ones from the newest
  table, then practice from the newest table and any fact still being learned. A known older fact comes only when due,
  so a round can be short (a first day is four new cards). With every table known and nothing due, a round is practice
  from all of them. Shuffled.
- Days are Porthole's local wall clock (`now / 86400`). Being away punishes nothing: an overdue card is simply due.
  A clock set back is clamped so no card waits longer than its box says.

## Save

One 520-byte blob per kid (`timescards/s<id>`, written by the shell, at most every 5 s): magic, version, size, level,
start table, answers ever, box, due day and last-seen day per card, crc. Append-only: a new field goes right before `crc` with a
zero that is a sensible default, `SAVE_VERSION` goes up, nothing is reordered or resized. A blob of the wrong size or
crc starts a fresh deck. `store()` is `"timescards"` and is never renamed.

## Look and files

- Its own look (rule 7): a cream index card with a red margin line and blue rules on a periwinkle desk; Fredoka
  SemiBold (OFL, `art/fonts/`) at 28 and 40 px (`generated/fonts.h`) and a digits-and-× cut at 110 px
  (`generated/big.h`), both from `tools/fontconv.py` (`--game cards`, `--game cardsbig`). Button fills are dark enough
  for white text at the audit's contrast.
- `game.h` / `game.cpp` (`class Game`, `namespace cards`): pages, input and drawing on the RGB565 surface. Repaints
  only when what the page shows changes. `debugCmd("level<N>")` starts a fresh deck on table N.
- `art/launcher.png` -> `launcher_icon.h` (`tools/launcher_icons.py`).
- In `games/apps.h` (id 12). That fills `MAX_APPS` (12): the next game needs it raised.
- Playtests: `tests/playtests/c0_times_cards.txt` (a natural first week: Wrong and Hard, the house mid-round, leaving
  and coming back, two days on to the 3× table, a second kid's own deck) and `c1_times_cards_monkey.txt` (the last
  table to "Every table known!", then random input under the sanitizers).
