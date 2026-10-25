# Toy Town Life — Design handoff index

This design-only branch prepares an **educational, non-compulsive** anime slice-of-life doll-neighborhood game for the Porthole device, with true playable scenario mechanics and child-centered progression.

- [CODEX-TASK.md](CODEX-TASK.md) — implementation instructions and expected proof
- [gameplay-brief.md](gameplay-brief.md) — core loop, School/Cafe/Park/Home scenarios, learning goals, persistence, style and v0.1 acceptance
- [design-review.md](design-review.md) — concrete adversarial educational, wellbeing and hardware pressure tests
- [assets/README.md](assets/README.md) — original PNG concept artwork and its import status

**Design principle:** a child designs her own tiny world, then makes plans and learns from small, believable consequences. Dolls/wardrobe are creative agency, not a reward treadmill; School, Cafe and Park introduce reasoning through actual play. No timers, streaks, in-app purchases, appearance rankings or pressure to return daily.

## Selected visual style

**Anime slice-of-life** from the earlier *Toy Town Life* 6-style board: bright hand-drawn-looking characters, pastel streets, cozy cardboard/plastic dollhouses, friendly UI. Do not recreate glossy doll-plastic 3D. All screens must be adapted for native 480×480 round display; the art boards below are moodboards, not shipped runtime assets.

## Educational review

The brief incorporates the actual repository principles from the game-designer agent, `feature` skill and black-box playtesting guide; consult them. Tests ask whether a child can reason within the game's situation, not whether arbitrary points indicate "learning".

## Next

A Codex implementation branch should use this design branch as its base, build a real first playable slice, run three complete emulator scenario playthroughs, add native screenshots and open a PR without merging. Preserve per-child progress and a natural stop.
