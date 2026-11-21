# Pets Club's Ukrainian words: telemetry and tuning

Date: 2026-11-21. Code: `apps/porthole/os/skills.h` (the shared engine: the rating, the calibration, the stars; skill
`WORDS_UK`), `apps/porthole/games/pets-club/ukrainian.h` (a word's difficulty, the picking), `ukrainian.cpp` (the
screens and the events), `content_uk.h` (written by `tools/uk_words.py`, which sets every word's step 1..7).

The events go out only while **Usage stats** is on (Admin, page 2; on by default, a grown-up can turn it off), with every other
event of the device ([telemetry design](2026-11-16-telemetry-design.md)). They carry the hashed device id and the
hashed player id, never a name, and the word only as its English meaning.

## The events

| Event | When | Properties |
|-------|------|-----------|
| `uk_answer` | every answer | `kind` (`meaning`, `lookalike`, `syllables`), `word` (the English meaning, unique per word), `step` (1..7, the word's), `rating` (the engine's theta after the answer; step s is about 100 × (s − 1)), `correct` (1/0), `ms` (from the question shown to the answer) |
| `uk_calibrated` | the engine's calibration ends | `rating` (theta), `answers` (how many it took) |
| `level_finished` | a round of five ends | `game` = `Pets Club`, `what` = `ukrainian`, `right`, `asked`, `rating`, `calibrating` |

## What each knob is and what tells us to move it

The engine's own knobs (K, the calibration's jumps, the 80% target, the pay) are shared with every learning game: a
change there goes to the stars thread's owner with the numbers, not into this game alone. This game's:

| Knob (ukrainian.h / uk_words.py) | Now | Data that moves it |
|---|---|---|
| A word's step (`step()` in uk_words.py: syllables, letters, і ї є ґ щ ь ʼ, clusters, band, Russian-like, false friend; capped at 7) | formula | Per word, right rate against the kid's rating: a word answered right far more (or less) often than its step predicts is mis-stepped. Fit the formula's weights on all words once there are a few hundred answers; fix single words by hand before that. |
| `KIND_OFFSET` (meaning 0, lookalike +100, syllables +50) | 0 / 100 / 50 | Right rate per kind at the same rating minus beta. |
| `READ_MS` / `SLOW_MS` | 700 / 900 / 1500, 10 s / 10 s / 15 s | The `ms` distribution of right answers per kind: guesses are the fast hump; slow is the long tail. |
| The ladder's words | 40 42 57 36 32 13 5 a step | Steps with few words repeat: a kid whose rating sits there sees the same words round after round. |

The calibration should take 6–10 answers (`uk_calibrated.answers`) and the first rounds after it run near 80% right.
If either is off, that is the engine's, and goes to its owner.

## Queries (PostHog project Porthole, HogQL)

Right rate and speed per word, against how far the kid is above it:

```sql
SELECT properties.word AS word, any(properties.step) AS step, count() AS n,
       avg(toInt(properties.correct)) AS right_rate,
       avg(toInt(properties.rating) - 100 * toInt(properties.step)) AS above,
       quantile(0.5)(toInt(properties.ms)) AS median_ms
FROM events WHERE event = 'uk_answer' AND timestamp > now() - INTERVAL 7 DAY
GROUP BY word HAVING n >= 5 ORDER BY right_rate
```

Right rate per kind, and per round after calibration:

```sql
SELECT properties.kind AS kind, count() AS n, avg(toInt(properties.correct)) AS right_rate
FROM events WHERE event = 'uk_answer' AND timestamp > now() - INTERVAL 7 DAY GROUP BY kind;

SELECT person_id, avg(toInt(properties.right) / 5) AS right_rate, count() AS rounds,
       argMax(properties.rating, timestamp) AS rating_now
FROM events WHERE event = 'level_finished' AND properties.what = 'ukrainian' AND properties.calibrating = '0'
  AND timestamp > now() - INTERVAL 7 DAY GROUP BY person_id
```

How the calibration went:

```sql
SELECT person_id, properties.rating AS rating, properties.answers AS answers, timestamp
FROM events WHERE event = 'uk_calibrated' ORDER BY timestamp DESC
```

## The weekly pass

Once a week while kids play: run the three queries, compare with the targets above, and change at most one knob or a
handful of words per pass in a pull request (a word's step through `uk_words.py`, so the header and the check stay in
step), with the numbers that moved it. Under 50 answers in the week: change nothing.
