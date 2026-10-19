# Codex Cloud handoff — implement the playable first chapter

**Repository:** `dreamiurg/porthole`  
**Starting branch:** `design/living-kingdom-chapter-one`  
**Implementation PR target:** this design branch while open; if it has merged, target `main`.  
**Game folder:** `apps/porthole/games/living-kingdom/`  
**Launcher label:** `Kingdom` (full internal title: Princess Pulverizer: The Living Kingdom).

## Mission

Build **Version 0.1 of The Living Kingdom** as a *genuinely playable* persistent, story-first RPG. The gameplay mechanics must form a stable foundation for many future chapters, while story content initially covers **Chapter One: Trouble at Willowbrook**. The user will playtest the host/browser emulator and provide direction before expanding further. You own implementation, art in the constraints of the board, testing, bug fixes, and a PR; do not stop at design wireframes or a one-scene proof of concept.

The story is for an advanced third-grade reader. Do not oversimplify prose. **Several pages of text before choices are desired.** The critical UX contract is: **choice screen → READ AGAIN → any previous scene page → back to choice → commit exactly once**. Resuming mid-reading after quitting or shell-imposed break must preserve the same page and pending decision.

## Read these in order

1. `AGENTS.md` and `CLAUDE.md` at root; `apps/porthole/AGENTS.md`; `docs/hardware.md`; existing game briefs.
2. `docs/superpowers/specs/living-kingdom/README.md` (handoff index).
3. `v0.1-game-design.md` — scope, design, hardware, acceptance criteria.
4. `ux-flow.md` — critical reading, choices, and accessibility flow.
5. `chapter-01-story.md` — authored opener, quest flow, NPC voices and ending variants. Treat as **starter narrative material to expand**, not as a complete 4-day manuscript.
6. `content-contract.md` and `chapter-01-world.json` — stable IDs / sample world graph, initial procedural variation, save and evolution.
7. `starter-screens.svg` — nine **schematic** color/styling/layout references. Use as visual direction only; produce real in-engine sprites/fonts/screens.

**Launcher is being redesigned separately by the repo owner.** Do not treat the present `MAX_APPS=8` limit as a product requirement or a reason to constrain future games. Integrate the game with the existing shell, profiles and launcher using the smallest safe compatibility change needed for prototype access; avoid undertaking a broader launcher redesign or interfering with the owner's work. If capacity or layout must change for the prototype, make a focused change and verify other games still launch.

## Functional core to build now

- One persistent world per child, seeded variations chosen once; six explorable Chapter One locations and world-day changes.
- Original in-game font and colorful pixel-art visual language, readable on actual round 2.1-inch 480×480 display, not giant crowded screenshot art. Use existing Porthole RGB565 font/layout and frame abstractions as appropriate, but own visual identity.
- Multi-page 150–350 word scenes (longer where the story needs it); short-to-medium paragraph chunks; free backwards/forwards traversal; button to reread before any commitment.
- Choices that support evidence, companion input and item-dependent options, with precise atomic state effects; no moral alignment scoreboard.
- Scene/story data separate from renderer/UI so Chapter Two can be added mostly as story data; stable IDs and migration strategy; deterministic seeded encounter variation.
- Inventory (six useful slots), three skills, companions, journal recording story decisions and promises, three quests intertwining with bridge/mill/marsh.
- At least three meaningful central conflict resolutions (mill-first, marsh-first, time-share), plus optional difficult-to-unlock infrastructure solution with a real labor/supplies tradeoff.
- At least two delayed consequences actually alter revisits, NPC dialogue and available quests after travel/rest advances days.
- An ending for Chapter One after actual follow-through, with a deed recognized, incomplete optional objectives retained, and a Chapter Two hook. Don't make the player reach a dead "coming soon" page before a resolution.
- Save exact reading/campaign state, isolate profiles, and maintain forward-compatible persistence.

## Quality / playtesting

Implement host-level and simulator tests for:
1. `scene pages → choice → reread pages → return → decide`: journal/effects exactly once.
2. Mid-page shutdown / reboot / profile switching then resume same page and choices.
3. World seed determinism, seed variations and no unwinnable quest seeds.
4. Each of three principal outcomes is reachable and meaningfully different.
5. Delayed world changes appear on later in-game days, and are not applied repeatedly.
6. Choice skill/item availability is explained and no impossible dead-end paths.
7. Circular bezel clipping, minimum tap target size, readable body font; screenshots at 480×480 for map, story, choice, party, journal, inventory, consequence and title.
8. Complete 3–5 representative play-session traces in emulator with realistic pacing and saved intermissions.

Use existing tests/harnesses and full `make -C apps/porthole check`, `make -C apps/porthole ci`, `make -C apps/porthole playtest`, and a firmware build when available. Never claim physical-device testing if only host emulator was used. Inspect screenshots instead of trusting passing pixel audits alone. Revisit text legibility with the entire image scaled to approximately 2.1 inches.

## Content quality target

The supplied manuscript is the **opening core, dialogue samples, and causal constraints**. Write enough coherent original additional scene content to make Chapter One replayable for multiple 15–20 minute sittings (roughly 20–30+ encounters/variants, more only if worthwhile), not merely a few button taps. Keep writing humorous, age-appropriate but intellectually honest, with real conflicts among people of good intentions. Avoid arbitrary failures, or an obvious perfect answer in every dilemma. Preserve player agency.

The JSON design fixture is not guaranteed complete enough to compile or test as-is. **Normalize and expand** it to a production validated graph, document any changes, ensure all referenced clues and outcomes are reachable, and keep ID stability after release. Engine design should balance testability, flash/RAM budget and later content-only expansions.

## Working method

1. Explore repo and produce brief implementation plan grounded in its real interfaces/limits.
2. Implement first vertical loop: title → story pagination → rereadable choice → persistence → journal → map.
3. Extend state machine to quests, inventory, relations, seeded encounters and delayed world consequences.
4. Integrate first complete chapter with meaningful multiple routes and visually polished scenes.
5. Test, capture/screenshoot every UX state, refine and correct shortcomings autonomously.
6. Open implementation PR with screenshots, summary, actual checks and known remaining design questions. Do not merge.

**Do not block waiting for artwork approvals.** The visual direction and starter assets are sufficient for autonomous first implementation. Use original in-game pixel art assets rather than relying on the generated image boards themselves.
