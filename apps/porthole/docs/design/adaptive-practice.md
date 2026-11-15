# Adaptive practice and spaced review: Biscuit and Times Cards

This is the plan for applying `learning-principles.md` (principles 2 to 6 and 9) to the two games that teach
maths: Biscuit's puzzles and Times Cards. It says what each game does today, what changes, and how the work splits
into pieces that separate sessions can build at the same time without touching the same files.

## Where things stand (main, 2026-11-15)

| | Biscuit puzzles | Times Cards |
|---|---|---|
| Content | Six kinds: add within 10, take away within 20, two-digit, times tables 2 to 6, short and long missing-letter words | Times tables 1 to 10, 100 facts |
| Answer | Pick one of three | Say it out loud, turn the card, grade yourself Easy, Hard or Wrong |
| Difficulty | Fixed per kind, random items; the kid picks the kind | Pace of new facts adapts to a running score that starts at 0.85 |
| Review | None: a miss shows the answer and is gone | Leitner boxes (1, 2, 4, 7, 14, 30, 60 days); a miss comes back in the round and tomorrow |
| Mixing | A round is one kind | Due cards across tables, shuffled |
| Feedback on a miss | The right answer | The answer, then the kid's own grade |
| Stars | 1 per 5 right answers, paid into the shared balance | 2 per finished round, whatever the grades |

Times Cards already follows the principles; Biscuit's puzzles do not. Most of the work is in Biscuit.

## Biscuit: what changes

### 1. A level per strand that moves with the kid (principle 2)

Replace the six fixed kinds with three strands, each a ladder of levels tied to a grade standard:

- **Add and take away** (1.OA.6, 2.OA.2, 2.NBT.5):
  1. sums within 10;
  2. take away within 10;
  3. sums within 20 that cross ten (8 + 5);
  4. take away within 20 across ten (13 - 5);
  5. two-digit plus or minus one-digit, no regrouping;
  6. the same with regrouping (47 + 8);
  7. two-digit plus or minus two-digit.
- **Times in a story** (3.OA.3): "3 bowls with 4 biscuits each" and "12 biscuits shared by 3 pups". The plain fact
  drill moves to Times Cards (below), so Biscuit stops duplicating it and practises using the facts instead.
- **Spelling**: today's short and long missing-letter word lists, as levels 1 and 2.

Each strand keeps a level per kid in their Biscuit save. A new save starts each strand from the profile's age, as Times Cards
does: age 6 starts Add at level 1, age 8 at level 4. After every round of five:

- **5 of 5 right:** the level goes up one;
- **3 or 4 right:** it stays;
- **2 or fewer:** it goes down one.

This staircase keeps a kid near 70 to 90% correct, close to the 75 to 85% target. The kid still picks a strand; the
level is chosen for them and shown only as progress ("Level 4: across ten").

### 2. Missed items come back on a later day (principles 3, 4, 6)

Keep a small review queue in the save: up to 8 missed items, each with its strand, level, item key (the operands,
so it can be rebuilt exactly) and the day it is due. A miss is due the next day. The next round in that strand, on or
after that day, includes it. Right on review takes it off the queue; wrong makes it due the next day again. When the
queue is full, the oldest item drops. Being away punishes nothing: an overdue item is simply due.

### 3. Rounds mix (principle 5)

A round of five is three items at the current level, one from the level below, and one due review item (or a fourth
current-level item when nothing is due). Once Add is at level 3 or more, its rounds mix addition and subtraction.

### 4. A short "how" after a miss (principle 9)

The miss already shows the right answer. Add one line of strategy per level, for example "8 + 5: 8 + 2 = 10, then 3
more is 13", or for a story "3 bowls of 4 is 4 + 4 + 4". It must fit the existing feedback box (four lines).

### 5. Stars stay as they are

One star per five right answers, paid as the round ends, with no penalty for a miss (principle 10). A review item
answered right counts like any other.

### Save

Biscuit's save becomes v4: append three strand levels (`uint8_t`), the review queue (8 entries of 4 bytes) and its
count, before the crc. Saves stay append-only; a v3 save gets its levels from the profile's age and an empty queue.

## Times Cards: what changes

1. **Division facts (3.OA.7).** Once a multiplication fact is known (box 3), its division partners join the deck as
   new cards ("56 ÷ 8", "56 ÷ 7"), on the same Leitner schedule and the same pace rule. The deck grows from 100 to
   at most 190 cards; the save grows to match (append-only).
2. **Self-grading at age 6 (open question).** A 6-year-old may not judge their own recall honestly, and the research
   reviewed does not cover self-grading. Proposal: watch how the 6-year-old grades for a week before changing
   anything. If the grades look unreliable, ask the kid to say the answer and then tap which of three numbers they
   said, which keeps recall first and makes the grade checkable. The watching is built in: Times Cards counts first-sight
   Easy, Easy held the next day, quick turns and think time on a grown-up page (hold the title 2 s; see its brief).

## Work split

Each piece lists the files it owns. Pieces with no shared files can run at the same time.

| # | Piece | Owner files | Depends on |
|---|-------|-------------|------------|
| A | Biscuit strands and levels (1), mixed rounds (3), save v4 | `games/biscuit/puzzles.h`, `content_puzzles.h`, `pet.h`, `screens_puzzles.cpp`, `game.h`, Biscuit tests | none |
| B | Biscuit review queue (2) and "how" lines (4) | the same files as A | A |
| C | Times Cards division facts | `games/times-cards/*`, `host/test_times_cards.cpp` | none |
| D | Times Cards self-grading check at age 6: counters now, the decision after a week | `games/times-cards/*`, `host/test_times_cards.cpp` | C |
| E | Short decodable stories for the 6-year-old, with one question each (`learning-principles.md`, principle 11) | `games/biscuit/content_stories.h` and its content tests | none; coordinate with A only on `game.h` if a screen is needed |

A and B go to the same session, one after the other. C and E can each go to a separate session in parallel with
A. Every piece ships as its own PR with green CI, an independent kid playtest where it is visible, and screenshots
in the PR (`AGENTS.md`).

## Later mechanics (one a week)

From `learning-principles.md`, in the order we would build them:

1. **Number line** (principle 7): place a number on a line from 0 to 20 or 0 to 100 by counting on from a marker
   (age 6), and fractions on 0 to 1 with denominators 2, 3, 4, 6 and 8 (age 8, 3.NF.2).
2. **Doghouse shop with a budget**: "You have 23 stars: buy two things, how many are left?", with multiplication for
   the 8-year-old ("3 balls at 4 stars each"). Subtraction within 100 and 3.OA.3; money (2.MD.8) as review.
