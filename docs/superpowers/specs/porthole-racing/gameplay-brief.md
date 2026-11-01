# Gameplay brief

## Product contract

An offline, approachable arcade race for children approximately 5–11. The player holds the round screen upright in both hands and turns it like a steering wheel. Automatic acceleration leaves attention on the road, passing, ramps and two thumb actions. A mistake briefly costs speed and position; the player always gets to finish.

The user-required rules are firm. Every numerical value below, other than verified hardware limits, is an **initial design default requiring playtest**. Do not report tuning or performance as validated by this document.

### Scope

- One rear-chase, pseudo-3D road renderer; one car handling model; four authored closed-loop tracks.
- One player plus three computer rivals, three laps, all tracks selectable immediately.
- Two gameplay buttons: **Boost** and **Rocket**. No accelerator, brake, touch steering, shield, damage bar, lives or kill count.
- At most three Boost charges and three Rocket charges independently; no passive recharge or charge percentage.
- Jumps, road hazards, off-road slowing, car contact, rocket hits and a finish order.
- No online play, garage economy, upgrades, unlock grind, championship, destructive crashes or persistent records in the first version. A run is self-contained; exiting/rebooting starts fresh without deducting rewards or imposing a penalty.

### Rendering choice

Use segmented pseudo-3D road projection, billboard cars/props and pre-rendered background layers. It matches the chosen chase-view art and suits a small device. A top-down racer would change the selected view; a general 3D engine adds unnecessary rendering and asset cost. Neither is in scope. The implementation may choose projection details after profiling; it must preserve the visible direction.

## Player flow

1. **Launcher → title/track select.** Four named cards with a small scene preview. Short launcher name proposal: `Rally`; internal folder proposal: `racing`. Validate label fit before registration.
2. **How to play.** Three short illustrated instructions: “Turn to steer”, “Pick up boosts and rockets”, “Finish all 3 laps”. Show that holding the lap chip pauses. A visible Back is always available outside the race.
3. **Calibrate.** “Hold like a wheel” → hold steady → preview left/right movement. Calibrate the comfortable neutral grip, never the current turn during a race. Start only after a valid upright grip and a fresh touch.
4. **Countdown.** `3`, `2`, `1`, `GO`; racers do not move or spend inventory before GO. Start with one Boost and one Rocket so both actions can be learned immediately.
5. **Race.** Auto acceleration, passing, hazards, pickups and jumps. A brief `Lap 2`/`Final lap` callout does not cover the road. Lap chip stays `1/3`, `2/3`, `3/3`; position is `1st`–`4th`.
6. **Pause.** Hold the lap chip for 600 ms. Freeze every racer, projectile, pickup timer, boost timer and race clock. Menu offers Resume, Recenter, Restart, Tracks, Home; separate into pages if needed for full-size targets. Restart/leave needs a clear confirmation because it discards this run. Resume uses a short 1-second cue, then continues exact simulation state. Recenter returns through calibration with the race frozen. System rest/time limits retain priority over the game.
7. **Finish.** Crossing the line after three complete laps locks the player's place and elapsed active time. Show `Finished!`, place, time, Replay and Tracks, plus visible Home. No failure/game-over state even for fourth. Replay resets all per-run state, inventory, timers and rivals.

There is no race timeout or elimination when the leader finishes. Finished rivals retain their finish order and stop blocking the track. The player keeps driving until they finish or choose to leave. Remaining AI positions can be shown as “Racing”; do not invent their finish times.

## Steering and camera

- Use **screen-plane wheel rotation relative to calibrated gravity**, not tabletop tilt or horizontal yaw. Clockwise turns right, counterclockwise turns left. Keep the HUD anchored to the screen; never rotate text or the entire framebuffer in software.
- Initial steering response: 3° dead zone, full steer at ±20°, about 100 ms smoothing. These are existing Starfighter precedents, not proven racing values. Calibration needs at least 350 ms of steady, usable screen-plane gravity. Validate comfortable grips on the real device.
- Clamp steering smoothly at full lock. Do not require turning the device upside down. Keep car movement continuous, not lane-snapped; lane markings are visual guidance.
- On a curve, neutral input follows a straight heading and drifts toward the outside of the road; steering into the bend maintains the racing line. Curvature, speed and lateral response must use the same simulation step. Curves must remain controllable without Boost.
- For a measured invalid/near-flat pose lasting 300 ms, pause all simulation and prompt “Hold like a wheel”. Require valid calibration/resume input to continue. Current firmware retains the last good sensor sample after a read failure; this is **not** a complete sensor-disconnect detector. Do not claim it is.
- Use a steady chase camera. Suggest horizon near y=175–190, player center near (240,320), body approximately 54–60×60–70 physical pixels. Keep the car below the central road sightline and separate from button targets. No violent camera shake, screen roll, head-bob or motion blur.

## Race model and initial tuning

Use normalized speed, where `1.0` is normal maximum on dry road. This is game tuning, not a claim about real vehicle speed. Do not show km/h.

| Parameter | Initial default | Player-facing reason |
| --- | --- | --- |
| Race length | 3 laps; 30–40 s clean lap | A short complete run with recovery opportunities |
| Racers | 4 total; player starts 4th, staggered grid | A readable pack and someone to pass immediately |
| Acceleration | 0 → 1.0 in 2.5 s; automatic | No extra button; quick recovery |
| Boost | 1.35 speed cap for 1.5 s; acceleration doubled | A clear burst after a pickup or mistake |
| Contact loss | multiply current speed by 0.65 | Visible setback without stopping |
| Solid obstacle loss | multiply by 0.55 | Obstacles matter more than a light bump |
| Rocket loss | multiply by 0.60 | Passing opportunity, never destruction |
| Impact recovery floor | 0.25, never above pre-hit speed | Repeated hits cannot stop a moving car or accelerate a slow one |
| Mud / oil / slush cap | 0.65 while overlapping | A predictable slow patch |
| Off-road cap | 0.55 until back on the road | Gentle boundaries, no falling |
| Hit protection | 0.8 s after contact/obstacle/rocket | One collision cannot drain speed every frame |
| Jump | 0.7 s airborne | A readable hop without another control |
| Rocket firing cooldown | 0.45 s | Distinct taps; no accidental salvo |

Apply instantaneous losses once on contact entry, not every update. Ground contact requires separation before the same pair can hit again. While inside an obstacle, steer/slide the car into the nearest free road space; never trap it in a wall or repeatedly respawn it. Protection suppresses only new instantaneous speed penalties; it does not turn off terrain caps or need a shield icon.

Terrain caps take precedence over Boost. While on mud/off-road, use the lower terrain cap even during a boost; its timer continues. Apply only the strongest simultaneous terrain cap, never multiply caps together. A collision during Boost still causes its speed loss, then the car accelerates again toward its currently permitted cap. Boost is no immunity.

At GO speed starts from zero and follows ordinary acceleration. Apply the recovery floor only inside an accepted instantaneous impact: `afterHit = min(beforeHit, max(0.25, beforeHit * factor))`. Thus a hit at speed 0.10 leaves it at 0.10, never accelerates it to 0.25; a hit at normal speed uses its listed multiplier. This is not a per-frame minimum. Ordinary acceleration and terrain caps remain separate, and pause/finish never apply the impact floor. Use swept collision/segment traversal so high speed or a slow frame cannot skip a hazard, pickup, ramp or finish line.

### Progress and place

Maintain unwrapped forward distance and a lap/checkpoint sequence. A lap completes only after traversing the full ordered circuit and crossing the finish line forward. Starting on the grid is not a lap. Off-road movement follows the course boundary and cannot bypass progress checks. Projected screen position is not a ranking input.

Finished racers sort by their interpolated finish-crossing time. Active racers sort by total validated progress. For equal progress/finish times, use stable grid order to avoid flicker. Once the player crosses the third finish line, record the place exactly once and ignore later collisions. A rival finishing earlier remains ahead even when removed from collision/render lists.

## Boost, Rocket and road pickups

**Inventory:** `boostCount` and `rocketCount`, each 0–3. A successful pickup adds one to its type, clamped to three. A full inventory leaves that pickup available for a later lap; it does not silently swap types or create hidden reserves. Start/replay resets each to one.

**Boost button:** a fresh press with count >0 and no active boost spends one and begins the burst. While boosting, ignore another press without spending or queueing. Holding never auto-repeats. The button shows icon, `Boost` and count; it may brighten during the burst but has no filling ring, recharge meter or countdown.

**Rocket button:** a fresh press with count >0 and cooldown clear spends one. Launch forward along the local track tangent from the player's lateral position. Initial defaults: 2.5× normal car speed, lifetime 1.2 s, one hit then despawn. No homing, lock-on UI, area damage or rear firing. It can miss. Road/guardrail contact ends it; decorations beyond the road have no collision. The target's speed falls, with a brief spark/puff and wobble, then automatic recovery. The car is never destroyed, removed, reset or awarded as a kill.

Cooldown-blocked presses spend nothing and are not queued. A held touch, a touch dragged into a button, or a carried touch from a menu cannot fire. Use the existing single-pointer/fresh-touch conventions. Sequential Boost/Rocket use is supported; simultaneous multitouch is not required.

**Pickups on the road:** a cyan lightning symbol for Boost and orange rocket symbol for Rocket, distinguished by shape as well as color. Prefer explicit contents over question-mark roulette. Initial road density: 4–6 pickup gates per lap, no more than two visible gates at once. Choose one safe gate early in every lap; optional outside-line pickups create modest risk/reward. Consumed pickups return on the player's next lap, once per authored gate per lap; they do not respawn under the car. Rivals do not consume pickups or fire weapons. From adaptive level 1 up (below), rivals may hold Boost charges of their own, never more than three, spent only while chasing.

Pickups at road level can be collected only while grounded. An authored raised pickup can be collected only during a jump; show its height and shadow clearly. One gate grants at most one charge per pass. At a full inventory, show a brief `Full` beside the button without obscuring the road. Pickups never create speed loss.

## Ramps, hazards and rivals

**Ramps:** crossing an aligned ramp automatically starts a jump. The road is continuous beneath it: no bottomless gaps, falls, missed-landing death, recovery lives or jump button. Animate a vertical arc and separated shadow; forward speed and track progress continue normally. Allow half the normal lateral response while airborne. Ignore ground hazards, off-road caps and car contact until landing. Rockets can hit airborne cars using track distance/lateral alignment; their speed effect never changes landing time. Boost works in the air under its normal rules. Airborne cars cannot trigger another ramp; the ramp requires exit/re-entry. Landing itself has no speed penalty; ordinary ground rules resume at the landing location. Provide a wide, hazard-free landing zone with at least 1 s of normal-speed travel before the next obstacle or tight turn.

**Hazards:** static debris, mud/oil/slush, shallow water, guardrails and traffic. All are visibly distinguishable from decoration, telegraphed at least 1.5 s ahead at normal speed. No random obstacle appears in the near foreground. Do not combine a blind turn, pickup decision, ramp landing and blocked road at one point.

**Rivals:** use the same course/progress system and physical speed penalties. Dry-road pace targets at level 0: 0.92, 0.97 and 1.00 times normal player maximum, with seeded occasional line errors and recovery. No teleporting or sudden hidden catch-up speed. Do not change difficulty because the player pauses. Rivals may avoid hazards imperfectly; they are racers, not traffic walls. Never deliberately form more than two abreast in the near field. Author road width and lateral placement to leave a usable corridor of at least one player car width plus 24 physical pixels at the player's depth. Keep passing possible on every track.

*Amendment, 2026-10-31 (adaptive rivals):* as a kid gets better, races must stay close (rivals visible ahead, overtakes, lead changes) without punishing anyone. Each kid has a hidden level per track. A won race adds one, two when the kid led for 3/5 of it. A loss at 0 or below takes two away; from 1 to 3 it drops to 0; above that, while the level's ladder rung (below) is 9 or less, straight to -2, because there the rivals only boost to pass a kid who leads, and a kid stuck behind would otherwise get the same race again; from a higher rung it takes three. So a losing streak from the top goes 14, 11, 8, 5, 2, 0 on Desert Canyon, 15, 12, 9, 6, 3, 0 on Industrial Ruins, 19, 16, 13, 10, 7, -2 on Snow Track and 16, 13, 10, 7, 4, -2 on Night Run. A competent driver settles at about 60% wins (57-68% by track). Leaving a race never counts. A never-raced track starts one below the kid's best level elsewhere, never below 0 or above 3. Below 0, on every track, the rivals are a little slower, gently at first and faster deeper down (at -16 about 0.87 of their pace on Desert Canyon, whose straights let a kid who never steers finish close, and 0.80 elsewhere), and from -7 a rival with the kid just behind it moves out of the kid's lane. Separately each level is a rung of a ladder: rivals earn Boost charges, one rival one charge a rung, at most three each; then the slower two close a little on 1.00; then a rival takes its three charges back at each lap line, as the kid does from road pickups; the last three rungs, each track's top three levels (Pro, a user request of 2026-10-31: tracks get harder as a kid masters them; 12-14 on Desert Canyon, 13-15 on Industrial Ruins, 17-19 on Snow Track, 14-16 on Night Run), leave the rivals alone and can each switch on one more hazard drawn in the track's own art, clear of every pickup and ramp and kept only where simulation showed the race no easier for it: two rocks on Desert Canyon and a barrier on Night Run, none yet on Industrial Ruins or Snow Track. A loss at the top drops below them. Every solid hazard, and every other car, hits only where it is drawn. Narrower roads and sharper bends were tried in simulation and rejected: a narrower road made races easier (rivals jam) and sharper bends changed nothing. Each track's level sits some rungs up that ladder, one rung a level all the way down: its base strength, so level 0 is a race a competent driver wins 50-75% of on every track (7 rungs on Desert Canyon, 6 on Industrial Ruins, 2 on Snow Track, 5 on Night Run). A rival fires a charge only while chasing (the kid 3-40 m ahead of it), never within 2.5 s of another rival's boost, with the player's own cap (1.35) and duration (1.5 s), drawn with cyan exhaust flames. This is not hidden catch-up: a rival boost is seen as it catches the kid, it is the kid's own tool under the kid's own cap, it is earned by the kid's skill level rather than by race position alone, and no rival's dry-road top exceeds the kid's (the slower two stay below 1.00). In simulation, rival boosts fired at fixed points of the lap, whatever the kid was doing, did not keep races close; boosts fired only while chasing did. The chip on the track cards says Auto for this adaptive setting, the default; a kid may instead pick a fixed setting there (a user request of 2026-10-31): Easy (slower, courteous rivals), Normal (level 0 on every track) or Hard (by track, a level a competent driver wins 30-43% of, and harder than Normal for a less precise driver too); a fixed setting never moves the adaptive levels.

## Circular HUD and visual language

Draw within the real circle centered at (240,240), radius 240. Proposed HUD targets stay within radius 228 for a 12 px safety margin. Check **every corner** of the full interactive rectangle and every rendered glyph, not just its center. Below are physical coordinates; runtime touch coordinates are divided by three.

| Element | Physical rectangle x, y, w, h | Behavior |
| --- | --- | --- |
| Lap chip, upper left | 84, 84, 84, 72 | `1/3` and small pause symbol + `Hold`; hold 600 ms opens pause |
| Place, upper right | 312, 96, 84, 48 | `3rd`; informational, no action |
| Rocket, lower left | 108, 336, 84, 84 | Rocket icon, label and 0–3 count |
| Boost, lower right | 288, 336, 84, 84 | Lightning icon, label and 0–3 count |

This preserves **two drawn gameplay buttons**. The visibly labeled hold action on the lap chip supplies the required pause/exit path without a third action button over the road. Explain it before the first race; it must be discoverable in-race. Holding the chip must cancel if the finger leaves it; opening pause must not activate a menu item with the same touch.

Align the upper labels to the circle's shoulders, not a full-width rectangular status bar. Do not draw the artwork's mini-map, charge arcs, speedometer, health, shield or extra counters. Inventory is a large digit `0`–`3` on each action; exhausted buttons remain visible and recognizably inactive. Target readable 24 px labels and 30–36 px major numbers, then measure the actual bundled font. Never shrink text simply to force a label to fit; adjust layout inside the safe region.

Use the board's bottom-row atmosphere with simpler baked details. Keep the player silhouette, lane edges, pickups and obstacles more legible than scenery. Preserve dry-road contrast in Snow Track and Night Run. No runtime bloom, transparent fog stack or full-screen damage flash. Spark, dust, tire trail and boost streak effects are small and bounded. First version is silent, consistent with the harsh-buzzer constraint; feedback must work without audio.
