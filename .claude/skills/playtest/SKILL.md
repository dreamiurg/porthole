---
name: playtest
description: Paw Street (apps/paw-street). Use before calling any Paw Street gameplay or UI change done. Runs the playtest suite (`make -C apps/paw-street playtest`), explains how to read apps/paw-street/build/playtest/report.md and its FAIL/WARN severities, and says when to escalate to the playtester agent instead of reading it yourself.
---

1. `make -C apps/paw-street playtest` builds `snap` and `snap-asan`, then runs `python3 apps/paw-street/tools/playtest.py` against every scenario in `apps/paw-street/tests/playtests/*.txt`. It writes `apps/paw-street/build/playtest/report.md` and one contact-sheet PNG per scenario under `apps/paw-street/build/playtest/`.
2. To iterate on one scenario instead of the whole suite: `python3 apps/paw-street/tools/playtest.py 05_read` (matches file names in `apps/paw-street/tests/playtests/` by substring; space-separate several substrings, e.g. `06 monkey`).
3. Read `apps/paw-street/build/playtest/report.md` top to bottom, not just the summary line:
   - **Scenario table**: pass/fail, check count, and UI FAIL/WARN counts per scenario. The report's own header states the rule: a scenario fails on a failed directive, a crash, a sanitizer report, or a hard (FAIL) UI finding — **WARN findings never fail a run**.
   - **UI findings table**: one row per (severity, rule, screen), with example findings and which scenario saw them. `<details>` blocks below hold the complete list when there are more than a few.
4. Know the rules the audit applies (from `apps/paw-street/tools/playtest.py`'s `audit()` — check there if a threshold matters and this drifts):
   - **target size** — FAIL if the smaller dimension is under 18 logical px (6 mm); WARN if under 24 px (8 mm) and area under 480 px². The repo's real floor (apps/paw-street/CLAUDE.md hard constraint 1) is 24x22 — a WARN here is still a constraint violation, not optional polish.
   - **round edge** — FAIL if less than 85% of a region's area is inside the circle, or its center is more than 70 px from (80,80).
   - **overlap** — FAIL if two regions share more than 4x4 px (WARN instead if one fully nests inside the other, e.g. a badge on a button).
   - **spacing** — WARN if two regions are under 2 px apart without overlapping.
   - **clipped text** — FAIL if any corner of a text box is more than 79 px from (80,80).
   - **contrast** — FAIL under 2.5:1 (WCAG luminance ratio), WARN under 4:1.
   - **dead screen** — WARN if a screen has no tappable region at all (screens where the whole glass is one button — splash, celebrate, intro, gift — are exempt).
5. For a quick ad hoc check of one screen outside the formal suite, write a short raw script instead (see the `tour` skill) and use its `ui` command — faster, but it prints raw hit-region/text-box numbers; it does not apply the FAIL/WARN rules above for you.
6. To add a new formal scenario, add a file under `apps/paw-street/tests/playtests/` using the directive language (`expect`, `expect-screen`, `ui-check`, `sheet`, `follow-glow`, `solve-word`, `answer-book`, `repeat N ... end`, plus raw sim commands from the `tour` skill) — read an existing scenario file and `apps/paw-street/tools/playtest.py`'s `play()` function first; directives are compiled into a raw sim script under the hood, they are not understood by `apps/paw-street/host/sim.cpp` directly.
7. Escalate to the `playtester` agent when the change touches more than one screen, adds a new interactive control, produces a FAIL you're not confident how to fix, or a monkey scenario times out or trips the sanitizer.
8. Never call a UI or gameplay-feel change done from `make -C apps/paw-street test` alone — `apps/paw-street/host/test_pet.cpp` never renders a frame, so it cannot catch a layout, contrast, or feedback problem.
