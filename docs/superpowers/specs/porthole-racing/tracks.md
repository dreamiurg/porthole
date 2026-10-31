# Four authored tracks

All four use the same handling, inventories and finish rules from the [gameplay brief](gameplay-brief.md). Select any track from the start. These are proposed course designs, not levels already implemented or playtested. Each is a closed circuit repeated three times; do not require four unrelated engines.

Percentages describe normalized distance around one lap. Connect the final straight smoothly to the start. Use continuous curvature and road width, with advance scenery/chevron cues. Every ramp has continuous road beneath it, a clear bypass and a safe landing zone. Place pickups outside unavoidable collision lines. Decorative scenery never blocks the driver without an explicit obstacle silhouette.

## Desert Canyon — learn the rhythm

Reference: **2A** in the original PNG. Terracotta walls, dusty ochre road shoulders, blue sky, distant mesas, sparse scrub, dark compact rally cars. Palette direction: burnt orange, sand, deep brown, sky blue; cyan pickups remain distinct. Clear afternoon visibility and broad curves make this the recommended first track.

| Lap segment | Shape and activity |
| --- | --- |
| 0–20% | Broad start straight; first safe Boost pickup near 8%; staggered rivals and room to pass |
| 20–40% | Long right sweep around a mesa; outer sand slows, arrow boards warn early |
| 40–55% | Shallow dry wash with one visible mud patch; Rocket pickup on the clear inside path |
| 55–72% | Optional dirt ramp on the left, flat bypass on the right; raised Boost above the jump |
| 72–90% | Gentle left/right bends with isolated rock debris outside the natural racing line |
| 90–100% | Wide finish straight and a safe pickup gate; no obstacle at the lap line |

Target clean lap: 30 s. Suggested 4 pickup gates, 1 ramp, 2 isolated solid obstacles per lap. Difficulty comes from deciding when to Boost and steering through broad bends. Dust must not hide the road. No falling from cliffs; guardrails and forgiving shoulders bound the playable corridor.

## Industrial Ruins — choose a clear line

Reference: **2B**. Rusted towers, broken overhead gantries, warm amber sky, steel road edges and orange-lit structural silhouettes. The road stays readable among dark machinery. Static scenery supplies the industrial scale; moving machinery traps are out of scope.

| Lap segment | Shape and activity |
| --- | --- |
| 0–18% | Factory approach straight; safe Rocket gate and passing room |
| 18–38% | Medium left bend between support columns; columns outside the road, marked crates inside one shoulder |
| 38–54% | Broad service yard; two separated oil patches leave a clear central route; Boost gate after them |
| 54–70% | Optional loading-dock ramp over a shallow rough patch, side bypass; clear landing lane |
| 70–88% | Alternating medium bends through a ruined gantry; advance chevrons, no falling debris |
| 88–100% | Open return straight under the start structure; safe pickup and finish |

Target clean lap: 35 s. Suggested 5 gates, 1 ramp, 3 solid obstacles. Oil uses the standard 0.65 terrain cap, not an additional random spin. A good line conserves speed; the risky pickup is avoidable. Never build an impassable lane of crates plus rivals.

## Snow Track — smooth steering

Reference: **2C**. Blue-white mountain backdrop, snow-heavy pines, deep navy shadows, dark asphalt and bright red/white chevrons. Keep white pickups off white snow; the two pickup types retain saturated colors and dark outlines. Snowflakes are optional sparse decoration, not a screen filter.

| Lap segment | Shape and activity |
| --- | --- |
| 0–20% | Dark, grippy start straight and safe Boost; teach the normal road line |
| 20–40% | Broad right sweep; one pale, clearly edged ice patch and an asphalt bypass |
| 40–58% | Alpine S-bend; slush only on the outside, Rocket pickup on dry road |
| 58–74% | Gentle optional snow-bank ramp with dark takeoff markers and a wide dry landing area |
| 74–90% | Long left return with another isolated ice patch; no hazard immediately after landing |
| 90–100% | Dry finish approach and safe pickup gate |

Target clean lap: 38 s. Suggested 4 gates, 1 ramp, 2 ice patches and 2 slush patches. Proposed ice behavior: only on the marked patch, lateral steering response is 75% and lateral motion settles over 250 ms; no random slide or forced spin. Slush uses the standard speed cap. Dry asphalt keeps normal handling. Do not make the whole track slippery; the child must see why the car behaves differently.

## Night Run — read the road at speed

Reference: **2D**. Indigo skyline, magenta/cyan signs, warm windows, lit guardrails and baked colored reflections on wet asphalt. The road, rival silhouettes and upcoming arrows stay visible without a real-time headlight cone. Night is the level's artwork, unrelated to device clock or bedtime.

| Lap segment | Shape and activity |
| --- | --- |
| 0–18% | Neon avenue straight; safe Boost gate; establish strong lane edge contrast |
| 18–38% | Lit right bend under an overhead walkway; repeated arrow signs reveal the curve |
| 38–54% | Broad plaza with shallow puddles on alternating sides and a safe central Rocket gate |
| 54–72% | Optional overpass ramp and flat service-lane bypass; city view opens on the jump |
| 72–90% | Left/right return through warehouses; isolated marked barrier near the outer shoulder |
| 90–100% | Wide illuminated finish avenue, safe pickup; finish arch visible in advance |

Target clean lap: 35 s. Suggested 5 gates, 1 ramp, 2 puddle patches, 2 solid obstacles. Wet reflections are visual only; puddles use the 0.65 terrain cap. No blackout, flashing hazard strobe or invisible debris. Difficulty comes from linked bends and deliberate Boost timing, not reduced visibility.

## Authoring and fairness checks

- Total authored segment proportions equal 100%; start/end join without curvature or width discontinuity.
- Hazard warning distance is calculated at normal speed; also playtest approach under Boost, especially before turns.
- Ground and raised pickup heights are visually distinct. Passing a ramp via its bypass never prevents finishing.
- Validate one clear corridor through each obstacle/pickup gate even when two rivals occupy the near field. Adjust rival line selection if a road segment cannot support the required corridor.
- No contact event repeatedly fires while objects remain overlapped. No jump teleports distance or bypasses lap validation.
- Three laps repeat a learnable layout; only bounded seeded rival behavior varies. Do not randomly rearrange obstacles between laps.
- Each track has a distinct skyline, road-edge treatment, hazard silhouette and color family at native 480×480. Distinction must survive simplified rendering.
