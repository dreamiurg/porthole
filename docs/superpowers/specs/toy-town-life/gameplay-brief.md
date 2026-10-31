# Toy Town Life — Educational Gameplay Brief (v0.1)

**Status:** approved visual direction, proposed gameplay contract for Codex. **Audience:** primarily an eight-year-old, comfortable with reading; open to children 6–10. **Visual direction:** colorful **anime slice-of-life**, not plastic-toy 3D. **Device:** Porthole / Waveshare ESP32-S3-Touch-LCD-2.1, 480×480 round capacitive touchscreen, existing per-child profiles. No sound, no required tilt, and no networking during play.

## Vision and player fantasy

An imaginative neighborhood assembled from **cardboard houses, plastic playhouses, miniature streets and parks**, inhabited by dolls of several types: fashion dolls, small keychain figures, stuffed animals, and clay characters. A child can select and dress up dolls, decorate their home, choose where they go and what they do, and invent gentle stories about a community that remembers them.

The core is **not a dress-up simulator with decorative educational trivia added**. It is a *playable miniature society*: children practice planning, interpreting other people's needs, reading, estimating costs, comparing options and creating through small decisions with observable outcomes. An eight-year-old should be able to explain what they were trying to accomplish and what changed as a result.

An ideal session is **12–20 minutes** with 1–2 self-contained activities, a clear natural stop, easy resumability, and no penalty for leaving. The game must be fun for the story and creative expression alone; learning is embedded in play and not graded like homework.

## What the player actually does (three linked loops)

### A. Create and personalize (2–4 min, any time)

Choose among at least 4 distinct doll archetypes (fashion, plush, clay, keychain) with different *material/animation personalities*, not ability rankings, gender stereotypes or purchasable rarity. Change hair, skin tones, colors, outfits, shoes, accessories, and optional makeup (never required, never connected to approval, beauty, or success). Rename **in-world characters** with a local-only limited name keyboard, or use suggested names. Dress-ups and room decoration are available at no cost by default; optional challenge-unlocked variants should not gate self-expression.

Outfit selection can matter through **functional decisions** (raincoat on a rainy park visit, comfortable shoes for a long walk, apron for baking), but there is no "wrong-looking" outfit and never appearance-based social judgment. A doll may choose anything and explain why.

### B. Plan an outing / make choices (3–6 min)

Pick Home, School, Park, Cafe, Mall, or Street. A small, reusable "plan" screen shows intention, needs and 2–3 possible activities. The player chooses where, with whom and what to bring, sometimes allocating a finite amount of **play money, time or supplies**. There is no real currency, purchases, advertising, hidden microtransactions, external brand, or storefront.

One short outing is playable from beginning to end. No long time-locks. If a chosen activity can't proceed (e.g., cafe closed, forgot sketchbook), offer meaningful recovery choices; **no lost progress or permanent failure**.

### C. Story / creative resolution (5–9 min)

At a location, meet a situation with information, dialogue and two or three distinct approaches. The child may ask a friend, inspect a clue, use an item, propose an alternative or select an activity. Consequences are **locally visible**: different friend dialogue, new scene detail, journal observation, changed object placement or unfinished promise. Avoid crude "good choice +10 kindness" meters; characters should have plausible reasons and sometimes disagree.

Return home and optionally make a one-page scrapbook record: choose a scene snapshot, caption or sticker, and a reflection about what happened. The player can end without filling any form. Save automatically and reopen the exact state next time.

## Learning through mechanics, not quizzes

| Play activity | Authentic cognitive / social practice | Observable proof of learning *inside the game* |
| --- | --- | --- |
| Plan a day with a bag of 3 slots | Prioritize, anticipate needs, plan and revise | What player packed opens options, but missing items allow a fallback |
| Cafe order with a small pretend budget | Add simple amounts, compare costs and tradeoffs | Can assemble order and see leftover coins; can change order |
| Friend needs help with an art presentation | Perspective-taking, asking questions, emotion vocabulary | Listening reveals why friend worries; choices alter next dialogue |
| Lost park item | Observation, spatial reasoning, evidence before accusation | Examine locations and infer likely place, not guess a quiz answer |
| Decorate or craft | Creativity, design iteration, pattern/shape/space | Place/rearrange objects; adjust after seeing results |
| Mall visit | Wants vs needs, comparison, delayed gratification | Compare 2 items with real differences; can also browse without buying |
| Family/community planning | Fairness with limited supplies, cooperation | Two valid ways forward with different costs; no secret moral grade |

**Do not presume** these activities cause transferable skill growth merely because they contain numbers or feelings. Each mechanic must give genuine agency, understandable cause/effect, and a chance to adjust the plan. A good learning scenario makes the child think *within the fantasy*, not interrupt the fantasy for a test.

## Chapter / prototype: One Good Day

**Minimum playable slice**: persistent chosen doll, outfit/wardrobe, 3-slot bag, map with Home/School/Park/Cafe (Mall teaser optional), 3 authored event loops (School, Cafe, Park) with branches, visible changes and a scrapbook. These events are complete and replayable with changed details; no placeholder buttons or "coming soon" traps on core path. Later chapters can introduce mall shopping and broader life-sim routines.

### Scenario S1 — The Art Presentation (School)

Friend Mina asks whether the doll will sit nearby during her class art presentation. She feels nervous, but the player has promised to help another friend set up displays.
- Ask what Mina is worried about; may learn she's more afraid of forgetting her introduction than being watched.
- Sit beside her; support, but display setup is delayed.
- Help rehearse; creates a cue-card solution if the player has notebook/paper; another friend may need to improvise.
- Ask Lucas-like helper? **Do not import other game's characters.** Instead recruit classmate Theo and explain the tradeoff.

**Learning:** listening before advice, keeping promises, alternatives under limited time. **Not a morality test:** no uniquely perfect answer. Show the consequences after the presentation, and permit later repair.

### Scenario S2 — Lunch for Friends (Cafe)

The group has **10 pretend coins** and would like lunch and a drink. Menu could offer sandwich 5, fruit bowl 3, water 1, cookie 2; prices must be visible and fully internally consistent.
- Choose for one doll, or try to include another friend.
- Compare two combinations; preview remaining coins before confirmation.
- A friend prefers water over juice; ask preference rather than assuming.
- If budget would go negative, show **Need 2 more coins** and allow changing choices—not punitive failure.

**Learning:** counting, addition/subtraction and budget reasoning in a real decision; no pressure to buy treats, no endless purchases. Earn nothing for simply spending.

### Scenario S3 — The Missing Notebook (Park)

A friend notices their small sketch notebook is gone after a picnic. Nearby: bench, fountain and path with visually distinct clues.
- Ask last-seen place; investigate; offer to help search before accusing anyone.
- Check 2 or 3 locations (player can return to any).
- Find evidence that it slid under a bench during the picnic, not that a stranger stole it.
- If the player guessed wrong, they can update the hypothesis and apologize; not lose the notebook permanently.

**Learning:** observation, evidence, revising belief and kind communication. **Replay variation:** seeded location of notebook, weather, and companion, without randomizing logic to incoherence.

### Optional Scenario S4 — Design a Cozy Room (Home)

Choose 3–5 pieces from a no-cost set of furniture and decor. Arrange visually within a bedroom; test fit and choose a color or pattern. A friend visiting may comment based on functionality (e.g. space for two to sit) but never aesthetic correctness. The child may iterate indefinitely without a completion meter.

**Learning:** spatial planning, iteration and personal expression.

## Character and world persistence

Persist per existing shell profile: selected doll type; cosmetic selections; owned default wardrobe patterns; room layout; bag contents; completed/current outing; visited places; friend conversation flags (bounded); scenario variation seed; short scrapbook entries / snapshots or references. **No daily attendance score, streaks, happiness decay, food/sleep neglect, relationships punished by real absence, or social-ranking leaderboard.**

Activities may refresh or vary based on *in-game* story decisions, not require waiting until tomorrow. Friends aren't "sad" because the kid has not opened the app. If session is interrupted by shell rest/daily budget, reopen exact state with no loss or pressure to finish.

Each location needs a compact state machine (enter → discover → choose → respond → reflect/return), with a clear back path. Build small composable verbs: **Talk, Look, Help, Plan, Create, Go**, not a huge collection of unrelated minigames.

## Child-centered anti-compulsion design

- **No** streaks, daily gifts, timed rewards, loot boxes, limited-time offers, variable-ratio rewards, infinite autoplay, negative body/appearance language, coercive notifications, or artificial urgency.
- Use **finite, authored activities** with a clear completion point; after 1–2 outings show a pleasant **All done for now / Keep exploring** landing screen with equally easy Stop and Continue paths. This is guidance, **not** a second punitive time limit; shell parental rules own actual time caps.
- No endless economy grind. Cosmetics should be largely free and instantly accessible; any unlocks are transparent and tied to creative accomplishments, not random compulsion.
- No dark patterns: purchase/completion buttons visually neutral; avoid enticing animated "BUY" prompts. Optional choice to do nothing or go home is valid.
- Respect identity diversity; no gender-locked clothes, hairstyles, jobs or locations. Dolls' material affects look and flavor, not intelligence or social worth.
- Respect Porthole's "nothing punishes absence", offline/no personal data rules and shell caps. Never bypass existing family controls.
- Keep age-appropriate choices with agency, readable English and humor. Avoid formal scoring of friendship/empathy.

## UX and art: actual round hardware

**Target:** 480×480 circular ST7701S IPS, touch only. Large child finger targets **≥72 physical px** and fully inside circular bezel; visual text is not clipped at top/bottom chords. Use existing native RGB565 font and image mechanisms. No game-specific sound. No required gyro or tilt.

**Style:** cheerful anime slice-of-life with delicate hand-illustrated streets, pastel sky, cherry blossoms, distinctive doll types; not generic glossy plastic 3D. Tiny backgrounds and character expressions may use pre-rendered art, but prioritize legibility and touch response over board-level complexity. Each game must own sprites/colors/fonts; do not borrow Biscuit or Kingdom's art. UI is crisp, not watercolor-smeared or relying on unreadable baked-in copy.

**Screens:** Title/Continue; Doll Picker (one large selected doll and horizontally scrollable alternatives); Wardrobe tabs (Hair, Outfit, Shoes, Accessory, optional Makeup); Home/Street Hub; Plan My Day (destination/companions/bag); Scenario with short dialogue (may paginate); Decision (2–3 big cards); Cafe budget calculator; Park clue scene; Decorator; End-of-Story scrapbook; gentle stopping point. A child must be able to return, change dolls/outfits and resume without restarting campaign. Distinguish **displayed character** from **shell profile**: never create a new Porthole user profile for every in-world doll.

**Density caution:** The generated boards are moodboards at 1536px wide, not actual hardware snapshots. Do not cram their multi-panel UI or tiny captions onto 480px glass. Each native screen should present at most one primary decision, a short prose block and 2–3 large actions. Show actual emulator screenshots at real-device scale in implementation PR.

## Scope and acceptance

V0.1 should prove these end-to-end paths through the *visible emulator*:
1. Select doll and customize two attributes; save/reboot/reopen with appearance intact.
2. Pack a bag, choose School, listen to friend, commit a support plan, witness consequences and note in scrapbook.
3. Budget Cafe meal; see calculations, handle over-budget gracefully and keep remaining balance internally consistent.
4. Park investigation: inspect clues, make/revise guess, return a notebook; optional replay changes the notebook location without contradictory evidence.
5. Optional free room decoration, if within schedule. Never let a decorative feature consume the time needed for 3 complete scenario loops.
6. Finish one outing, return to hub; exit through shell, reenter same profile; no session penalties.
7. Interruption mid-scenario (shell rest): scene and pending options preserved.
8. Native 480×480 screenshots, tap region audit, no sound emitted, scripted regression tests and final-build manual black-box emulator tour.

**Success criterion:** a real eight-year-old voluntarily explains at least one choice, completes an outing without grown-up handholding, and remains willing to stop without feeling something has been lost. Evaluate these through observations, not invented user studies.

## Review: pressure-test before calling it ready

**Design review from 4 perspectives:**
- **Game designer:** does the chosen plan visibly matter, do we have an intrinsically fun loop, can the scenario be enjoyed without an "educational" score?
- **Development/education:** does each scenario induce age-appropriate planning, counting or perspective-taking, or is the challenge trivial/fake? No claims of proven learning outcomes.
- **Child autonomy/ethics:** does the game avoid appearance/social approval pressure, manipulative timers, unfair fail states, and reinforcement schedules?
- **Porthole QA/accessibility:** can a child read/operate the scene at a *physical* 2.1-inch circle, with time caps, back, repeat and save?

**Design risks and mitigations:**
- Too much dressing → the main game becomes a UI catalog. Mitigate by making complete story outings the primary path and making cosmetics a quick optional play area.
- Too many lessons → preachy, boring story. Mitigate with character motivations, humor, and evidence that choices change the situation.
- Too many buttons → cramped circle. Mitigate with progressive disclosure; no permanent bottom tab bar; 2–3 large choices.
- Infinite shopping → compulsion. Mitigate with short authored visit goals, finite budgets and free expression, no stock scarcity.
- Pretend social grades → unrealistic morality judgments. Mitigate with relationship memory and differing responses, never "kindness points."
- Rewards + absence → return anxiety. Mitigate with no real-world clock-based changes and transparent, non-random progress.
- Graphics overload → firmware and touch latency. Mitigate with trimmed original art, palette discipline, texture reuse, and measurable performance.

## Repository handoff and boundaries

Read `AGENTS.md`, `apps/porthole/AGENTS.md`, `.claude/agents/game-designer.md`, `.claude/skills/feature/SKILL.md`, `apps/porthole/docs/playtesting.md` before coding. Implement within `apps/porthole/games/toy-town-life/` and register in the launcher carefully; launcher redesign is separate work owned by user. Document versioned/append-only per-profile save schema with CRC. Author tests in `apps/porthole/host/`, emulator playtests in `apps/porthole/tests/playtests/`, screenshot evidence in `apps/porthole/docs/toy-town-life/`. Run make check, CI, firmware compile and ordinary-input black-box playthrough; do not claim physical board testing if unavailable.

The referenced concept art is visual inspiration; do not ship a 1536-wide storyboard collage as a game screen. If raster images are included as references, preserve them in `assets/reference/` and author true runtime assets separately. No code changes are part of this design-only PR.
