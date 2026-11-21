# Times Cards: game brief

The times tables as flash cards, one deck per kid, scheduled by spaced repetition. A card shows "7 × 8"; the kid says
the answer out loud and taps the card; the card turns to four answers and the kid taps one. The game grades the pick
itself, from right or wrong and how long it took against this kid's own usual time, and decides when each card comes
back, when new facts come in and when to try the next table, at each kid's own pace. Built for two kids practising a
few minutes a day: a 6-year-old starts on 2×, an 8-year-old on 4×.

The shared app brief (`apps/porthole/AGENTS.md`) covers what every game shares. This file has Times Cards' pages, its
rules and its save. The design and its sources: `docs/design/adaptive-practice.md` (Times Cards: chosen answers, timed
grades).

## Pages

Every page has the house at the top: from a card it goes back to the deck, from the deck out to the launcher.

- `cards_home`, the deck: the table the kid is on, big on an index card, a dot for each of its ten facts (green known,
  amber learning, pale new), "Say it, then tap the card" (or "Every table known!") and **Go!**; a tap on the card deals
  too.
- `cards_ask`: "k of n" (or "One more try" on a missed card's second look) and the card, "a × b" or "n ÷ d", with "Say
  it out loud, then tap!". The card sinks under a finger; a tap turns it. The time counts from here.
- `cards_answer`: "a × b =" (or "n ÷ d =") on a smaller card and four answers in a 2 × 2 grid. A tap picks one: the
  right answer turns green (and a wrong pick red, the others grey). The next card follows 0.6 s after a right pick and
  2 s after a wrong one, so the right answer is seen. Taps between the answers, on the card or after a pick do nothing.
- `cards_done`: "Perfect round!" (nothing missed), "Great job!", "Nice try!" (more than half missed), or "Next up: N
  ×!" when the round reached a new table; "+2 stars", two discs ("Right" green, "Missed" red), the table's dots and
  **Again**, which deals the next round.
- `cards_stats`, for a grown-up: hold the "Times Cards" title on the deck for 2 s (not a kid target, not in the
  audit). "How it's going", the step and how many jumps, then four rows: right answers of all tries on met cards,
  the usual time, the "fast" line for a mid-size fact (3 × 4) and how many facts are being learned. The house goes
  back to the deck.

Every finished round pays `ROUND_STARS` (2) into the kid's shared stars (`os/stars.h`, `AppEnter::stars`, spent on
Pets Club's hats), whatever was missed. A round left half-way pays none. The shell keeps and saves the balance; this
game never stores it.

No sound (`soundOn` is always false), no countdown shown, nothing lost: a missed card only comes back sooner. The
time is measured, never displayed: a slow right answer is still right.

## Rules (`deck.h`, header-only, checked by `host/test_times_cards.cpp`)

- A card is one fact: table × n, tables 1..10, n 1..10 (100 cards), or a division, n ÷ d for divisors 2..10 and
  quotients 1..10 (90 more; ÷ 1 is left out). Its Leitner box says how well the kid knows it: 0 never shown, 1
  learning, up to 7.
- **The answers** (`choices`): the right one and three near misses a kid really makes: the neighbouring facts (7 × 8
  shows 49 and 64 next to 56: a × (b ± 1), (a ± 1) × b), for a division the quotient ± 1 and ± 2, then small numbers
  close by. Shuffled, so the right answer's place tells nothing.
- **The grade** (`gradeOf`): a wrong pick is **Wrong**. A right pick is **Easy** when faster than `fastMs` for that
  card, **Hard** when slower. `fastMs` is the kid's own: a running mean μ and spread σ of log(ms ÷ size) over right
  answers on known facts (box 3 up), weight `TIME_WEIGHT` (0.1, about the last ten answers), each answer clipped to μ
  ± 2.5σ, times capped at 10 s. Easy means under exp(μ + σ) × size, kept within 3 to 8 s. The size: × 1, × 2, × 5, × 10
  and squares 0.8; both factors 6..9 (not a square) and every division 1.25; the rest 1. The first guess before any
  known answer comes from the starting table: 5 s for 1× and 2×, 4.5 s for 3×, 4 s for 4×, σ 0.35.
- **Half-days.** Time runs in halves of the local day (midnight to noon, noon to midnight). A box waits `INTERVAL`
  halves before its card is due again: box 1 a half, box 2 a day, then 4, 7, 14, 30, 60 days.
- **Easy** on a new or due card moves it one box up (a new card known at once goes to box 2). A card still being
  learned also moves up on Easy after enough of a gap: box 1 in a later half, box 2 on a later day (after a night's
  sleep). **Hard** keeps its box and brings it back the next half. **Wrong** puts it back in box 1, due the next half.
  A known card answered before it was due never moves up.
- **Mastery:** a fact is known at `MASTER_BOX` (3): Easy in two halves and then on another day.
- **Put back in the round:** a Wrong card comes back three cards later, a Hard one five later, once each, headed "One
  more try" and not counted in "k of n"; a round grows by five such cards at most.
- **The step** (a staircase, −2..+2, `noteStep`): two fast right answers in a row move it up, a wrong one down, a slow
  right one starts the count again. Only met cards and probes count; a card come back does not. A new session starts
  where the last left off, at most +1. Aimed at about 85% right.
- **A round** is ten cards dealt one at a time (`choose`), so a miss changes the very next card:
  - −2: easy wins first (known facts, due ones before others, not seen this half);
  - a new fact: at +2 a probe (below) or a new fact every card, at +1 every other card, at 0 one card in three, never
    below 0;
  - then due cards (oldest first, at most `DUE_MOST`, 5, a round), then facts being learned (least known first), then
    easy wins, and a new fact only when nothing else is left.
  At most `NEW_PER_ROUND` (4) new facts a round and `WORKING_SET` (6) facts being learned at once (probes aside).
  New facts come in table order (4×1, 4×2, … then 5×1).
- **The jump** (`probeCard`, `jump`): at +2 the deck probes the first table from the level up that the kid has not
  started, on its hardest facts (× 6, × 7, × 8). Three fast right first answers there mark the whole table known: box
  3, due again over the next few days (two to eight halves apart), so a lucky guess is soon found. A probe missed or
  slow marks its table (`probeMiss`, a bit a table): that table is never jumped and is learned the usual way, and no
  table past it is probed until then.
- **Division (3.OA.7):** from level 4 (1× to 3× known), each known multiplication fact opens its two divisions as new
  cards: 6 × 7 known opens 42 ÷ 6 and 42 ÷ 7. They share the working set: a new division comes whenever the divisions
  being learned are fewer than half the multiplications, so divisions hold about a third of the places. Divisions
  come by fact family in table order (2× first, 1× last, its families being n ÷ n). They never move the level.
- **Level:** the first table from the last level up not fully known. It moves on (and the done page says "Next up")
  once all ten facts are known, past any jumped table. It never moves down: a known fact missed again is learned again
  from box 1 while the level stays.
- **Start:** from the profile's age, `startTable(age) = clamp(age - 4, 1, 4)`. The tables below start in box 5, spread
  over the next two weeks, so a few come back for review each day.
- Days are Porthole's local wall clock (`now / 43200` halves). Being away punishes nothing: an overdue card is simply
  due. A clock set back is clamped so no card waits longer than its box says.

## Telemetry

While Usage stats is on (`os/telemetry.h`, on by default), each pick sends `times_cards_answer` (`fact` "7x8" or "56:7", `grade`, `correct`, `ms` from the card showing to the pick, `box`, `retry`) and a finished round sends `level_finished` (`easy`, `hard`, `wrong`, `level_up`). Nothing changes in play or in the save.

## Save

One blob per kid (`timescards/s<id>`, written by the shell, at most every 5 s). In memory the deck is the `Save` struct
(1020 bytes): magic, version, size, level, start table, answers ever, the old pace (0 since version 5), box, due half
and last-seen half per multiplication card, then (version 2) the same three per division card, a spare zero, then
(version 6, where version 3's grading check was) the timing (`logMu` in 1/1024, `logVar` in 1/4096, 0 = no answer
yet), the step, the streak, jumps, tries and rights, the tables whose probe was missed, spare zeros, crc.

Versions: 1 to 3 stored the struct whole in days (520, 972, 1020 bytes); 4 bit-packed in days; 5 the struct whole in
halves (only if a deck could ever not be packed); 6 bit-packed in halves (`pack` / `packDeck`, on `os/bits.h`): about
160 bytes new, 330 four months in, under 400 over half a year. The whole version is numbered below the packed one on
purpose, so the shell's "newer version" check never refuses it. Every older version loads: its days become halves
(`toHalves`), the old pace and grading counters are dropped, and the timing starts from the first guess. `takeSave`
hands the packed blob only after reading it back as byte for byte the deck in memory. `host/fixtures/
times_cards_save_v3.h` and `times_cards_save_v4.h` are real saves; the tests hold their migration byte for byte.
Never regenerate them.

The struct stays the same size: a new field takes room from `spare6` (its zero must be a sensible default), joins
`packDeck`, and `SAVE_VERSION` goes up; nothing is reordered or resized. A stored blob this build cannot read (wrong
size or crc, or a newer build's after a rollback) starts a fresh deck on screen but is left in storage as it is: the
fresh deck replaces it only once the kid answers a card on it, never just because the game was opened.
`store()` is `"timescards"` and is never renamed.

## Look and files

- Its own look (rule 7): a cream index card with a red margin line and blue rules on a periwinkle desk; Fredoka
  SemiBold (OFL, `art/fonts/`) at 28 and 40 px (`generated/fonts.h`) and a digits-and-× cut at 110 px
  (`generated/big.h`), both with × and ÷, from `tools/fontconv.py` (`--game cards`, `--game cardsbig`). Button fills are dark enough
  for white text at the audit's contrast.
- `game.h` / `game.cpp` (`class Game`, `namespace cards`): pages, input and drawing on the RGB565 surface. Repaints
  only when what the page shows changes. `debugCmd("level<N>")` starts a fresh deck on table N;
  `debugCmd("card<N>")` deals a round of card N alone (screenshots); `debugCmd("right")` / `("wrong")` pick the right
  answer or a wrong one on the answer page, as a tap would (playtests). A card too wide for the index card (100 ÷ 10)
  drops the spaces round its sign.
- `art/launcher.png` -> `launcher_icon.h` (`tools/launcher_icons.py`).
- In `games/apps.h` (id 12). That fills `MAX_APPS` (12): the next game needs it raised.
- Playtests: `tests/playtests/c0_times_cards.txt` (a first week of a fast 6-year-old: a miss and its colours, a double
  tap, the house mid-round, leaving and coming back, jumps over the tables they know, "Every table known!", the
  grown-up page, and an 8-year-old's own deck answered slowly, so no jump) and `c1_times_cards_monkey.txt` (the last
  table to "Every table known!", then random input under the sanitizers).
