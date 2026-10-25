# Codex Cloud implementation handoff — Toy Town Life v0.1

**Repository** `dreamiurg/porthole`  
**Base branch** `design/toy-town-life-anime`  
**Game** `apps/porthole/games/toy-town-life/` (inside Porthole's shared firmware, not standalone)  
**Goal** Working prototype of a gentle, creative, educational anime slice-of-life doll neighborhood for an eight-year-old.

## Read first

1. Root `AGENTS.md`, `apps/porthole/AGENTS.md`, `apps/porthole/docs/playtesting.md`
2. `.claude/agents/game-designer.md`, `.claude/skills/feature/SKILL.md`
3. `docs/superpowers/specs/toy-town-life/gameplay-brief.md` — player fantasy, all scenario details, learning outcomes and anti-compulsion principles
4. `docs/superpowers/specs/toy-town-life/design-review.md` — adversarial review, acceptance tests and revisions to the mechanics
5. `docs/superpowers/specs/toy-town-life/assets/README.md` — approved image references, transfer and use guidance
6. Original generated PNG boards available from the asset handoff (if not committed as binary assets yet, fetch via provided artifact location; do **not** replace with SVG or unrelated stock images).

## Implement a real first playable slice

Prioritize a polished loop over catalog breadth:
- Pick a doll and choose clothing/accessories; default selections valid.
- Home/street hub with School, Cafe, Park plus room decoration if capacity allows.
- School: support a friend facing an art presentation with real competing obligations and follow-up.
- Cafe: compare menu choices with transparent 10-coin pretend budget; show exact remaining amount; prevent over-budget purchase with revision, not punishment.
- Park: find a missing notebook using clues and revisable hypotheses; coherent seeded variants.
- One-page scrapbook/snapshot with optional caption to remember story decisions.
- Autosave per Porthole child profile and exact scene/choice resume. Remember chosen doll, outfit, decoration and short NPC context. No absence penalties.

**No** conventional consumption-driven collection loop, time-pressure, microtransactions, real brands, appearance-based praise, daily gifts, streaks, loot boxes, infinite needs decay, or social ranking. The educational experiences must feel like natural gameplay rather than quizzes inserted into dress-up.

Use cheerful anime slice-of-life visual identity rather than generic plastic 3D, with cardboards, miniature houses, doll materials and social locations. Treat generated multi-screen collages as **moodboards**, not native-resolution sprite sheets or text assets. Deliver true 480×480 screens with legible typesetting and round-safe hit targets, not a static slideshow.

Develop the smallest reusable state/events/choice engine for future scenarios. Keep authored prose/events data separate from the drawing code, with deterministic outcomes and idempotent actions. Document any new NVS persisted structure as versioned, size+CRC checked, append-only, and cover upgrade path in tests.

## Required real verification (not just compiling)

- Build host simulator/browser emulator. Do **actual ordinary-input, final-build black-box playthroughs** for School, Cafe and Park; view each frame and inspect ease of use.
- Verify math menu combinations, errors and recoverability; art-story choices and consequences; park clue state; no ambiguous "right answer" moral grading.
- Verify pick/style/save/reboot/reopen, interrupt midway, return/cancel/doubletap/slow input, multiple Porthole profiles.
- Screenshot title, picker, wardrobe, plan, school dilemma, cafe budget, park search, scrapbook, stop/resume, and real circle-safe readability. Where hardware is unavailable, explicitly mark emulator-only evidence.
- Run `make -C apps/porthole check`, `make -C apps/porthole ci`, `make -C apps/porthole playtest`, firmware build, and the prescribed review and black-box evidence.
- Create an implementation PR (against this branch while design PR is open; `main` if design PR has merged), with representative emulator PNG screenshots. No auto merge.

## What to optimize after first parent/child playtest

**Interest and reasoning**: Is the child inventing a story, choosing based on facts, using the calculator meaningfully, adjusting after a wrong inference? **Healthy stopping**: Is there a natural end after an outing and no anxiety about not returning? **Clarity**: Are choices readable at 2.1 inches and do children navigate without adult prompts? Tweak mechanics from observed evidence, not engagement-maximizing analytics.

Design this as something an eight-year-old wants to play again because it feels like her own town, not because it demands attention every day.
