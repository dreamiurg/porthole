# Starfighter — Porthole game design brief

**Status:** Design exploration / visual reference; implementation deliberately out of scope.
**Date:** October 17, 2026
**Story:** A child pilots a small X-wing-inspired rebel starfighter, shoots waves of enemy twin-wing fighters, then completes a cinematic trench run and final precision shot.
**Preferred direction:** Hybrid of the *Squadron Battle* and *Trench Run* concepts, in bright, readable retro digital / 16-bit pixel art. Strong but economical parallax is a defining visual feature.

## Design north star

**The puck is the flight controller.** A six-year-old holds the printed round case in both hands like a small steering wheel, banks it left/right to fly, and taps the glass to fire. It should feel physical and theatrical, not like a tiny touchscreen with a digital joystick. Most of the action should be playable without moving a thumb over the ship or hiding the enemy formation. Three lives; forgiving, replayable runs; a clear beginning, climax, win and loss.

Keep the project a **Porthole game** on the existing device, not a separate app. The reference board in [starfighter-storyboard.svg](starfighter-storyboard.svg) is a clean 12-screen art-direction guide (vector schematic built for repo viewing). Our ChatGPT conversation also produced a richer generated 4×3 raster storyboard; that external generated PNG **is not embedded in this branch**, so treat the SVG and this description as the version-controlled references.

## Actual display and motion constraints

Verified against `docs/hardware.md`, `apps/porthole/CLAUDE.md` and `apps/porthole/firmware/board.cpp`:

- **Waveshare ESP32-S3-Touch-LCD-2.1**, round **480×480** RGB565 IPS display, no hardware buttons, ESP32-S3 240 MHz, 8 MB PSRAM. A 160×160 logical pixel-art presentation scaled 3× is already supported in Porthole; its native 480×480 rendering exists too. Illustrations should *look achievable* with limited colors, discrete sprites, short loops, and simple layer shifts, not fluid 3D polygon rendering.
- Capacitive touch is available; large targets are required (about **72 physical pixels / 8 mm**). Normal play is tap anywhere to shoot, with a separate unobtrusive press-and-hold back affordance.
- QMI8658 has a **6-axis accelerometer/gyroscope**, but **the current Porthole firmware reads the accelerometer only**. Gravity-based steering-wheel banking works with an upright device; reliable **flat tabletop yaw/spinning** does not work from acceleration alone. Gyroscope-powered spin gestures are future explorations, *not a requirement for this design*.
- The existing tilt games calibrate a child's comfortable grip. Starfighter should also start with a brief **hold like a steering wheel → calibrate neutral** step, since resting grip varies.
- Rotating the physical puck rotates its display too. Keep bank angles modest (~15–25° comfortable, tune with a child); keep HUD and mission text away from the bezel and favor very short labels. In the trench, allow environmental lean/parallax to sell banking rather than excessively rotating the entire scene.

## Screen language and retro art direction

**Visual inspiration:** compact 1990s space arcade cabinet: dark near-black/navy backgrounds, cobalt/azure planet limb, pale-grey player starfighter with four red engine glows, enemy dark-grey twin-panel silhouettes with red cockpit, **red player lasers**, **green enemy fire**, yellow-orange explosions. Chunky pixel clusters; strong readable silhouettes; 2–4 frame sprite poses; no tiny texture that turns to noise on the 2.1-inch round screen.

**Game objects should be deliberately oversized** for legibility. Enemy ships should present unmistakable wing/cockpit silhouettes. Reserve dark space behind projectiles, and reduce star count during laser-heavy frames.

**Parallax:** in space, distant stars drift slowly, a planet/Death Star-like moon drifts more slowly than ships, mid-distance debris and laser bolts move faster, nearest enemies move fastest. In the trench, wall ribs/panels sweep past at different apparent speeds by depth, a vanishing point stays stable, and the lower foreground walls appear to bank with steering. Prefer a handful of distinct depth bands to ornate 3D.

**HUD:** three tiny ship glyphs at the **upper-left safe interior** of the circle; modest score at the upper-right interior. Keep both comfortably below the very top where the circular chord is narrow. No permanent bottom bar. Only reveal reticles, target markers, or text during relevant moments. Preserve an optional hold-to-exit sign that does not interfere with shoot-anywhere.

## Controls and player experience

### Primary: one consistent hybrid control scheme
- **Puck upright, rotate left/right:** during squadron stages, the X-Wing **banks visually and translates sideways**, stopping before the circular boundary. In the trench stage, it primarily **banks/steers**, while wall and floor parallax shift in response. The physical gesture is the same; the world response changes to express flight.
- **Tap:** shoot a pair of bright red laser bolts. One tap = one volley. Avoid precision tap hotspots during combat.
- **Hands in natural grip:** calibrate at the beginning, give small bank dead zone to prevent drift. If a child tilts too far, cap steering instead of causing uncontrollable flight.
- **Forward/backward pitch:** optional second-layer dodge/elevation mechanic *only after testing that it is comfortable*. Do not require it for first playable or the final shot.
- **Evasive roll:** appealing optional trick, but do not assume that a sharp physical spin is recognizable without gyro integration. Could instead trigger from a sufficiently sharp gravity-bank movement and be tuned after playtesting.

### Player goals
- **Three lives**, represented by three X-Wing silhouettes. A hit removes one with a clear one-second invulnerable blink; never take multiple lives in one laser burst.
- **Short run:** roughly 2–4 minutes; the fun comes from changing spectacle, not high difficulty.
- During space combat, clear several simple enemy waves. Then transition to the trench for a faster, more cinematic final segment.
- In the trench, dodge turrets and green bolts, hold a roughly safe path, and at the end align a generous reticle and **tap for the proton torpedo**.
- After losing all three lives, show **GAME OVER**, final score, and a large **TRY AGAIN** target. No lost collectibles or punishing penalty.
- Victory ends with a gigantic orange pixel explosion behind the retreating fighter and **VICTORY!** / **PLAY AGAIN**.

## Twelve-screen storyboard and interactive beats

| Frame | Screen/state | Key visual and why it exists |
|---|---|---|
| 1 | **Title** | Red/gold STARFIGHTER logotype, hero ship in lower center, blue planet limb and a giant moon. TAP TO START. |
| 2 | **Hold & steer** | Two hands around puck, left/right arrows and banking fighter; CALIBRATE / TAP TO CONTINUE. Teach the physical grip immediately. |
| 3 | **Mission brief** | Tiny pixel pilot portrait; one objective: “CLEAR THE FIGHTERS. THEN FLY THE TRENCH.” Three ship lives. |
| 4 | **Squadron entry** | Hero ship at lower quarter, first four enemy fighters approach from upper half, red and green lasers, minimal HUD. |
| 5 | **Bank left** | Hero ship rolls left with a clear lateral displacement; *different-depth* star trails and planet show parallax. No textual HUD besides lives/score. |
| 6 | **Tap to shoot / explosion** | Paired red lasers, one enemy exploding in chunky orange clusters. Primary combat satisfaction. |
| 7 | **Player hit** | Brief orange spark around hero ship, “LIFE LOST” / two remaining ship icons, protective blink; stage continues. |
| 8 | **Wave clear** | Bold centered WAVE CLEAR, stars/planet visible behind it; quick upbeat transition rather than a menu. |
| 9 | **Trench entrance** | Perspective snaps into large grey trench with receding sidewalls, ship bottom center, moon ahead. Noticeably different pace. |
| 10 | **Trench steering** | Closest walls race past with layered parallax, laser turrets along walls, banking moves the ship left/right within the trench. |
| 11 | **Final exhaust port** | Large glowing red ring centered in distant wall. Steer to line up then TAP; keep target large enough for child success. |
| 12 | **Victory** | Ship accelerating away from exploding space station / orange fireburst, VICTORY and PLAY AGAIN. |
| variant | **Game over** | Darkened wreckage, very large GAME OVER, remaining lives zero, clear TRY AGAIN. Must be produced in final implementation though not one of the twelve main storyboard tiles. |

**Narrative pacing:** space combat is approximately the first half to two-thirds; the trench and final shot form a memorable second act. Important: do not fill the board with UI controls. Images in the storyboard show game states, **not interactive overlay screens stacked on gameplay**.

## Potential delight and replayability

- Two memorable enemy formations: V formation and alternating swoop, not ten types of ship.
- Larger enemy that takes two hits and gives a distinct explosion.
- Near misses briefly flash the edge of the circle; no camera shake so strong that the small screen becomes unreadable.
- Tiny exhaust and engine animations that intensify when banking.
- Finale should be forgiving: if the exhaust port is missed, loop or offer another pass without restarting the entire run.
- Sound is optional and sparing: Porthole's piezo is a harsh single-tone buzzer. Visual feedback must work silently and honor mute.

## Acceptance criteria for visual direction (not an implementation mandate)

The team should be able to view the storyboard and answer: where is my ship; where are enemies; what direction am I steering; how many lives remain; what do I shoot; how does space flight look different from the trench; what happens when I win or lose?

Use this design as a *starting proposal*, not a locked product spec. Validate the sensation of steering the **physical printed puck** with an actual six-year-old before setting final banking range, enemy speed, animation budget or pitch controls.

## References in the repo

- [Hardware specification](../../hardware.md)
- [Porthole game/runtime constraints](../../../apps/porthole/CLAUDE.md)
- [Existing tilt football](../../../apps/porthole/games/tilt-fc/CLAUDE.md)
- [Existing tilt labyrinth](../../../apps/porthole/games/marble-kick/CLAUDE.md)

This is a personal fan-game concept using familiar space-opera imagery. Publishing beyond a private hobby prototype may require replacing licensed franchise identifiers/designs with original ships and terminology.
