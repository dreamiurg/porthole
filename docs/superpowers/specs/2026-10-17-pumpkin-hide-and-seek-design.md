# Pumpkin Hide & Seek — concept / Codex handoff (2026-10-17)

Status: **Concept and prototype brief, not implemented.** The attached SVG storyboard is a schematic local art reference; earlier generated concept boards in the ChatGPT design discussion are more detailed visual references and are **not** present as raster images in this commit.

## Idea and tone

A playful, slightly spooky Halloween hide-and-seek game for children in grades 1–4 (roughly 6–10). The hero is a small expressive pumpkin who tries to avoid being caught by Grandma in her warm, shadowy house. Grandma is affectionate, funny, and relentlessly determined to bake pumpkin pie. If she finds the hero, a theatrical, horror-film-inspired **orange frosting drip** rolls down from the upper rim and reveals a pie labelled **YOU LOST** in bold icing. No gore, harm, or frightening jumpscares: the absurd pie is the joke. A tap immediately allows a retry.

This is primarily a **visual and animation experience**, rather than complex gameplay. It should feel delightful and readable at a glance.

## Confirmed hardware and platform

- Porthole firmware on Waveshare ESP32-S3-Touch-LCD-2.1, **round 480×480** display, capacitive touch and QMI8658 tilt sensor.
- Implement as a **game within** `apps/porthole/games/pumpkin-hide-seek/`, sharing Porthole shell / OS / board abstractions. **Not** as a standalone `apps/<app>`.
- Consult root `CLAUDE.md`, `apps/porthole/CLAUDE.md`, `docs/hardware.md`, and existing games before coding.
- Prefer the existing **160×160 indexed framebuffer scaled 3×** for chunky pixel art, if practical for the game, and adopt an intentionally distinct palette and visual vocabulary from Pets Club. Use native 480×480 selectively if necessary for readable large text/effects.
- All important action and typography must remain inside the circle; no control at clipped corners.

## Agreed style: retro pixel art

Think cohesive 16-bit Halloween arcade with expressive pixel characters and layered interiors, not painterly 3D. Chunky black outlines, dithering only where useful, limited palette: pumpkin orange, cream, dusty purple, midnight blue, wood brown, warm lantern yellow, sage green. Grandma has grey bun, round spectacles, purple cardigan, patterned apron and a hand-held flashlight. The pumpkin has an oversized readable face, small green stem and bouncy squash/stretch poses. Cozy house interiors: fireplace, couch, dining table, pantry, plant, closet, attic toy chest. Lighting creates playful shadow and silhouette moments.

The attached `pumpkin-storyboard.svg` conveys four frame compositions; it is **not** a final sprite sheet. The previously shown ChatGPT concept images explored title/intro, Grandma searching with flashlight, hiding behind chair/curtains, and pie loss screen, along with room variations. Recreate these visually in native pixel art rather than embedding oversized concept boards into firmware.

## First-playable loop (proposed)

1. **Start:** pumpkin on rug in living room, `TAP TO START`, moon in window; single concise instruction `TILT TO ROLL • TAP TO HIDE`.
2. **Explore:** neutral-grip calibration as already implemented in sibling tilt games; tilt left/right to roll pumpkin across one room. Start with a single horizontal lane, 2–3 distinct hiding spots (behind chair, under table, by large plant). Acceleration/friction tuned to be forgiving.
3. **Hide:** when close to a highlighted hiding spot, tap to duck into it. A hiding pose clearly indicates safety; tap again to emerge, or tilt out depending on tested UX. Grandma enters periodically, searching with a broad flashlight cone.
4. **Search:** sight-based danger is strongly telegraphed (footsteps, yellow light, Grandma silhouette and 'Hmm?'). Staying fully concealed defeats a pass. If the player is exposed in the cone long enough, Grandma notices and a brief comedic catch sequence plays.
5. **Win:** survive a short easy round (target 30–60 seconds, tune for kids). Pumpkin grins, Grandma says she must have misplaced the pumpkin, `YOU DID IT!` and `PLAY AGAIN`.
6. **Lose:** freeze; Grandma leans into frame, smiles, an orange icing drip descends from **top circular rim**, gradually covering the scene like classic horror-film blood drips; transition to giant pumpkin pie with legible `YOU LOST` icing (orange letters on cream frosting, or cream letters on orange frosting—maximize contrast), one big `TRY AGAIN` target. Comedic, not threatening.

No lives, no score, no persistence requirement, no waiting after failure, no irreversible consequences. A short retry should bring player back immediately.

## Art storyboard beats

- Frame A: pixel pumpkin center foreground, cozy room background, start prompt.
- Frame B: pumpkin rolls behind chair while Grandma searches from right with flashlight; clear yellow cone.
- Frame C: close call: face peeks below curtains/table as cone passes, no cluttered HUD.
- Frame D: pie covers most circular viewport; orange frosting drips from top, large readable **YOU LOST**, tappable retry.

See `pumpkin-storyboard.svg` in this folder for a compact 4-scene visual map. Existing rendered concept artwork in the original conversation is additional inspiration, not a required external dependency.

## Scope guidance to Codex

First build a **single-room playable vertical slice**, not five rooms or a giant game. Prioritize memorable pixel art, squash/stretch animation, a convincingly sweeping flashlight cone, and the dripping frosting end sequence. Avoid complex pathfinding. Extend the same room with a few randomized hiding and search patterns only after the core loop is fun.

Potential expansion later: kitchen/table, living room/sofa, attic/toy chest, hallway/closet, and pumpkin patch/other pumpkins; Grandma's slippers sound cue, disguise as a basketball, pumpkin among lanterns, a playful ghost friend.

## Acceptance criteria

- Available in the existing Porthole launcher as a distinct game; shell back navigation works.
- Runs in host simulator and browser emulator, with a plausible tilt substitute in emulator.
- Correct no-corners screen design: no clipped text or buttons; tap targets are child-friendly.
- Playable by tilt and touch on hardware; neither requires impossible gestures.
- Pixel art crisp and distinct from existing game palettes/assets.
- Lose sequence shows **animated frosting entering from top of the circular screen**, then pie with **YOU LOST** readable and simple **TRY AGAIN**.
- Win and retry paths work. No startling audio; honor global mute.
- Run app `make check`, `make ci`, relevant firmware compile and scripted playtests; report actual outcomes and any gaps.
- Follow existing project process: document UX states, create/update golden screenshots with pixel-preserving scaling, and open a PR for review. Do not merge automatically.

## Gameplay update: phase 1 (2026-10-17)

Playtesting the slice showed one dominant strategy: hide at once and wait out the timer.
Phase 1 gives the pumpkin a reason to move and Grandma some surprises, still in one room:

- **Goal:** collect three candy corns on the floor, then roll out the door at the left wall.
  The 40-second timer is gone; hiding only stalls.
- **Grandma's moves vary every round:** full sweep, fast sweep, a sweep that turns back,
  and a spot check ("THE TABLE?") that finds a pumpkin hiding there, so the player has to
  pick a different spot. Every move is announced first; checks get the longest warning.
- **Near misses:** the hidden pumpkin shivers while the beam passes over it, then PHEW!.

Later phases (not built yet): more rooms with one new idea each, and optional stars and hats.

## Gameplay update: phase 2 (2026-10-17)

- **Hold still while hidden:** a hard, sustained wiggle rustles the hiding spot (SHH!);
  Grandma heard it, and her next move checks that spot, with the usual long warning.
- **Ghost friend:** once a round, tap the ghost at the window to knock something over;
  Grandma stops what she is doing and goes to look, buying a few safe seconds.
- **Endings that change:** catch lines, pie captions and escape lines vary per round.
Exact numbers live in `apps/porthole/games/pumpkin-hide-seek/CLAUDE.md`.
