# Toy Town Life — design review and stress test

This review revises the design *before implementation*. It is grounded in Porthole's checked-in rules:
- `.claude/agents/game-designer.md`: kid-first clarity and immediate response, exact pacing, no punishment, intrinsic fun.
- `.claude/skills/feature/SKILL.md`: entry/play/completion/replay/exit contract; deterministic rewards and save migration.
- `apps/porthole/AGENTS.md`: round-screen/touch boundaries, progress preservation, different visual identity, no injury/death/absence penalty.
- `apps/porthole/docs/playtesting.md`: real emulator black-box tour, mis-taps, recovery and usability at native screen size.
- `apps/porthole/games/living-kingdom/AGENTS.md`: reusable data-defined narrative consequences, page and choice stability (inspiration, not shared assets/code).

## Four independent review lenses

| Lens | Initial risk | Design correction / gate |
|---|---|---|
| Game design | Dress-up dominates and visits are empty scene galleries | Require **Plan → Situation → meaningful decision → visible outcome → return**. Prototype has 3 complete outings, not nine decorative screens. |
| Educational validity | Labeling buying, shopping or putting clothes on dolls as 'learning' | Every mission must require an authentic planning, arithmetic, observation or perspective-taking choice *and* offer feedback and a retry. Define measurable in-game demonstration, not unsupported evidence of real-world skill growth. |
| Child wellbeing | Consumptive rewards, compulsive collecting, return-pressure | No loot boxes, streaks, countdowns, in-app purchases, social status ranks, appearance/body judgments or daily attendance bonuses. Narrative closure after each outing. |
| Hardware/QA | High-resolution moodboards promise too much UI and animation in 480x480 circular touch area | Limit to one primary choice per scene and 2–3 large controls; use RGB565 + pre-rendered/cropped sprites; tests include image/clipping/latency in actual host simulation and 2.1-inch viewing. |

## Scenario-by-scenario adversarial tests

### S1 School friendship / art presentation
- **Failure:** "Encourage her" is always correct and friendship becomes an invisible virtue score.
- **Revision:** Child must decide between two obligations. Listening first reveals the friend's actual concern; if they help one child directly, another may need to improvise. Both outcomes can be repaired later with an honest conversation.
- **Observation:** Tester can explain at least two plausible motivations, and player sees dialogue adapt. No +10 friendship notification.

### S2 Cafe budget
- **Failure:** Coins are either meaningless (infinite supply) or become a repetitive shopping grind.
- **Revision:** One transparent *pretend* budget per story (e.g., 10 coins) and 4 clearly priced food choices. Every price and total visible; adjust cart freely. Player can bring a friend and negotiate preferences.
- **Observation:** User can compare costs, reduce an over-budget cart, and correctly understand remaining coins. Replay is about a new social choice, not shopping for cosmetics. No lost coins carried permanently.

### S3 Park missing notebook
- **Failure:** Correct answer is guessed by tapping three random areas, or NPC falsely accused without a way to repair.
- **Revision:** Child gets a last-seen clue, looks at 2–3 sensible locations, draws a provisional inference, can change view after new evidence. Scenario seed changes notebook placement AND the matching clue, never only location.
- **Observation:** Story and clue agree for many seeds; wrong hypothesis is revisable, and apology is possible.

### S4 Home decoration
- **Failure:** endless tapping / low educational value / approval through appearance.
- **Revision:** Provide a simple space constraint—leave a path between bed and door while making two friends comfortable—and make all visual themes acceptable. Afterward the child can modify design freely.
- **Observation:** Placement matters spatially; no aesthetic grade. Reward is the beautiful scene itself.

## Counterfactual pressure tests

1. **Player never opens wardrobe.** Can they enjoy all three learning-rich outings? **Must: yes.** Avatar defaults are valid.
2. **Player spends 15 min only dressing dolls.** Is that okay? **Yes:** creative play is itself legitimate; no shaming or manipulation to funnel them into lessons.
3. **Player chooses unfashionable, unusual, or gender-nonconforming styling.** NPC reaction should be neutral/positive about *personality or practical function*, never body/beauty judgment.
4. **Player chooses "wrong" in school or park.** No permanent penalty. Show realistic small costs and a path to revisit/repair.
5. **Child forgets bag item.** No softlock, no restart; offer borrow, improvise or return.
6. **Child exits halfway through dialogue or hits shell rest cap.** Resume the same spot with the same options; no reroll.
7. **Child returns after two weeks.** All dolls, rooms and stories are there, no sadness or lost points.
8. **Child double-taps confirmation.** Exact-once commit. No duplicate credits, item loss or scene skipping.
9. **Child is not a fluent reader.** Short dialogue, readable text, character expressions and clear pictograms; provide reread/back controls.
10. **All visual assets are unavailable or heavy.** The engine remains playable, and resource budgets are verified before art is accepted; don't claim a polished game based on a collage.

## Session pacing (targets, not timers)

- Pick doll and choose a destination: 1–3 min.
- Plan/bag and brief setup: 2–4 min.
- Situational story with 2–3 meaningful actions: 5–8 min.
- Consequence, optional creative extension, scrapbook: 2–5 min.
- **12–20 min** total with a natural stopping point and optional second outing. No in-game timer required; existing shell family controls remain authoritative.

## Learning signal and child-playtest plan

Game-level evidence: choices are understood, explanations refer to scene facts, math interactions are consistent, and revised plans lead to different outcomes. Do not claim that the child has measurably gained empathy or transferable arithmetic skills without further research.

Run real child playtests with brief debrief prompts, outside the game: "Why did you pick that?", "What would you do differently next time?", "Was any screen confusing?" Do not gather children's personal data or analytics. Observe whether player can voluntarily stop after an outing. Fix confusing interactions rather than adding tutorial walls.

## V0.1 priority ranking

1. **Must:** one doll picker, basic wardrobe, street/home hub, plan/bag, School + Cafe + Park complete, scrapbook, persisted state, proper navigation.
2. **Should:** Home decoration and seeded, coherent scenario variants; return NPC dialogue reflects previous action.
3. **Later:** Mall with shopping comparisons beyond one episode, more dolls/material-specific motion, home furnishing breadth, more social dilemmas and creative stories.
4. **Explicitly not now:** streaks, shop economy meta-loop, ranking friends/outfits, collecting for its own sake, always-on need meters, roleplay AI chatbot.

## Implementation handoff checklist

Before calling a prototype playable: one complete, natural emulator run of each of three scenarios, screenshots from final build, mis-taps and cancellations, exact-page save/resume, content coherence, static/firmware tests, and a child-facing critique. Follow `apps/porthole/docs/playtesting.md` literally rather than relying on scripted assertions as proof.
