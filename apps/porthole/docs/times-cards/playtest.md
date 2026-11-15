# Times Cards: black-box playtest

Emulator only (headless `snap`), three independent source-free playtester runs. No real child, no board.

| Run | Build | Persona | Play |
| --- | --- | --- | --- |
| 1 | first build (`snap` 9849f025…) | Zach, 6, first grade, reads short words slowly, imprecise finger | 10 days, honest mistakes, every finger probe |
| 2 | first build (`snap` a32a0359…) | Zoe, 8, third grade, knows 1x–3x, learning 4x–6x | 7 days, 143 cards, every finger probe |
| 3 | second build (`snap` ba2501c8…) | Zoe, 8, with a per-fact knowledge map; Zach, 6, short pass | 7 days (181 cards) and 3 days |

Each run found the game from the launcher (All games, page 2), made its profile through the visible screens, and left
and came back through the launcher between days (`skip 86400`). No `dbg`, no `app` shortcut, no source. Run 1 and 2
shared a scratch directory by mistake and disturbed each other's driver once each; both reran the affected steps from
their own traces. Later briefs give each run its own directory.

## Dispatch brief (run 3; runs 1 and 2 had the same shape without the knowledge map)

Persona and ordinary goal: find "Times Cards" yourself and practise the way a parent asked, a few rounds a day for
about a week, saying each answer out loud and grading honestly; count how often each fact comes back; note when a new
table opens and where you would stop each day. Plus the brief's probes: offset taps, taps between buttons, double taps
on the card and on grades, a fast grade right after the answer shows, a press sliding off, a touch carried across a
page change, leaving mid-round, switching profiles. Changes from run 2: a per-fact knowledge map (so grading is
honest and repeat counts mean something) and a private working directory.

## Findings and what changed

| Finding (run) | Resolution |
| --- | --- |
| Known facts kept coming back: 4x1 and 4x2 twelve times in a week, 1x3 every day; rounds padded with older tables (1, 2) | Practice is now only facts still being learned and not yet seen today (a missed one gets its second look in its own round); a known fact comes only when due, so a round can be short (a first day is four new cards). Run 3: no padding from older tables. |
| Slow progress: nothing new until day 9 (1); a learning fact graded Easy did not move unless due (1) | A fact still being learned moves up on Easy any day after the one it was last seen (`seen[]` per card in the save): known after Easy on two separate days. Run 3: 5x opened on day 3, 6x on day 5, 7x after day 7. |
| Progress only on the deck page, which Again skips; same "Great job!" every time (1, 2) | The round's end shows the table and its ten dots; titles "Perfect round!", "Great job!", "Nice try!" (mostly Wrong) and "Next up: N ×!". |
| "Say it, then tap the card" but the deck's card did nothing (1, 2) | A tap on the deck's card deals, like Go!. |
| A grade within 450 ms of the answer showing was ignored (1, 2) | The answer page ignores only the finger that turned the card, by place: a quick grade counts, a double tap on the card still acts once. Every other page keeps the full 450 ms gate, so a double tap on the last grade cannot hit Again. |
| No pressed look on the card (1, 2) | The card sinks under a finger. |
| An all-Wrong round grew to 30 cards (2); Wrong only makes the round longer, which teaches pressing Easy (3, Major) | A missed card comes back once, headed "One more try", and does not change "k of n"; a round grows by five such cards at most. The deeper fix (a reward for honest grading) is deferred: see below. |
| Hard's amber read 4.49:1 under white (1) | Darker amber, 5.1:1. |
| Shell scenarios failed after a twelfth game (1, 2) | Fixed in the PR: Rally's tile moved on the Library's now-full second page, and the admin test switches one more game off. |

## Deferred, with the player impact

- **Honest grading has no reward of its own** (run 3's Major). After the change above, Wrong costs one extra card,
  shown as a second look rather than a longer round. A reward (for example Biscuit stars for finished rounds, which do
  not depend on the grades) is the next step and is planned with the shared star balance.
- **No "done for today"**: Again always deals; once everything due is done, a round is extra practice of the facts
  being learned, then of everything. A kid can stop at any round's end; nothing is lost by stopping.
- **Leaving mid-round restarts the count** at "1 of n"; grades already given are kept.
- **Finishing a table** gets the "Next up" title and an empty dot row, no view of tables already learned.
- **The house overlaps "k of n"** (the shell's standard home button place); leaving keeps every grade.
- **Times Cards is on page 2 of All games**: each kid can star it onto Home.

## What held up (all runs)

Missed taps, taps in the gaps, slide-off, long holds and double taps on the card, the grades, Go! and Again all acted
once or not at all. Each profile keeps its own deck; progress survives leaving and a process restart. Text contrast
4.5–7.4:1 and grade buttons 34x22 logical px pass the UI audit.

## Where they would stop

Zach (6): after one round of 6–8 cards. Zoe (8): after the first round on days with nothing new, longer on days a new
table opens; the first day held her for four rounds.

Screenshots (final build): [deck](home.png), [card](ask.png), [answer](answer.png), [round end](done.png),
[one more try](one_more_try.png), [next table](next_table.png). Scripted replay: `tests/playtests/c0_times_cards.txt`.
