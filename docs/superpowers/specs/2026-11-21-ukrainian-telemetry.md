# Pets Club's Ukrainian words: telemetry and tuning

Date: 2026-11-21. Code: `apps/porthole/games/pets-club/ukrainian.h` (the rating, the calibration, the picking),
`ukrainian.cpp` (the screens and the events), `content_uk.h` (written by `tools/uk_words.py`, which sets every word's step).

The events go out only when a grown-up has turned on **Usage stats** (Admin, page 2; off by default), with every other
event of the device ([telemetry design](2026-11-16-telemetry-design.md)). They carry the hashed device id and the
hashed player id, never a name, and the word only as its English meaning.

## The events

| Event | When | Properties |
|-------|------|-----------|
| `uk_answer` | every answer | `kind` (`meaning`, `lookalike`, `syllables`), `word` (the English meaning), `step` (1..10, the word's), `rating` (the kid's, after the answer), `correct` (1/0), `ms` (from the question shown to the answer) |
| `uk_calibrated` | the calibration ends | `rating` (step × 100), `answers` (how many it took) |
| `level_finished` | a round of five ends | `game` = `Pets Club`, `what` = `ukrainian`, `right`, `asked`, `rating`, `calibrating` |

## What each knob is and what tells us to move it

| Knob (ukrainian.h / uk_words.py) | Now | Data that moves it |
|---|---|---|
| A word's step (`step()` in uk_words.py: syllables, letters, і ї є ґ щ ь ʼ, clusters, band, Russian-like, false friend) | formula | Per word, right rate against the kid's rating: a word answered right far more (or less) often than its step predicts is mis-stepped. Fit the formula's weights on all words once there are a few hundred answers; fix single words by hand before that. |
| `KIND_OFFSET` (meaning 0, lookalike +100, syllables +50) | 0 / 100 / 50 | Right rate per kind at the same rating minus step. |
| `TARGET_BELOW` (85: 80% right) | 85 | Right rate per round after calibration: aim for 75–85%. |
| K (160 down to 40) | `160 / (1 + n/5)` | Rating jitter round to round once settled: if it swings more than a step a week, lower the floor. |
| `GUESS_MS` / `SLOW_MS` | 700 / 900 / 1500, 10 s / 10 s / 15 s | The `ms` distribution of right answers per kind: guesses are the fast hump; slow is the long tail. |
| Calibration (1, 2, 4… then halving; stop at two misses, the top or 15) | as the page | `uk_calibrated.answers` (should be 6–10) and whether the first rounds after it run near 80%. |

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
