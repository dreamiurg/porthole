# Platform contract and acceptance

## Verified baseline

Inspected repository commit: `fedf62ce0c804dcb20ef3e7e7ee11ba9545cecb8`. Re-check these source files when implementation starts; later main may differ. Paths below are relative to the repository root.

| Fact | Source |
| --- | --- |
| ESP32-S3, dual-core 240 MHz, 8 MB PSRAM, 16 MB flash; 480×480 round RGB565 display | `docs/hardware.md` |
| Game is an `App` under `apps/porthole/games/`; games depend on `os/`, not other games or shell | `apps/porthole/AGENTS.md`, `apps/porthole/os/app.h` |
| Native RGB565 drawing, logical 160×160 touch coordinates; full target and glyph bounds inside the circle | `apps/porthole/AGENTS.md`, `apps/porthole/os/input.h` |
| Nine games already registered; `MAX_APPS=9`; launcher preserves 3+3+2+1 layout | `apps/porthole/firmware/main.cpp`, `apps/porthole/host/sim.cpp`, `apps/porthole/shell/shell.h`, `apps/porthole/shell/launcher.h` |
| Upright wheel input precedent, steady calibration, 3° dead zone, 20° full input | `apps/porthole/games/starfighter/controls.h` |
| Accelerometer currently supplies gravity; flat-table yaw is not exposed; failed reads retain last sample | `apps/porthole/firmware/board.cpp`, `apps/porthole/firmware/main.cpp` |
| Static internal RAM ceiling 131,072 bytes; OTA slot ceiling 8,323,072 bytes | `apps/porthole/platformio.ini`, `platform/dram_budget.py` |
| OTA boot checks update-display allocation plus 48 KiB internal-DMA headroom | `apps/porthole/firmware/main.cpp` |
| Lossless packed RGB565 decoder available, with bounded scratch; this is not a PNG loader | `apps/porthole/os/packed_image.h`, `apps/porthole/os/packed_image.cpp` |
| Fresh-touch gates, visible exit patterns and single-pointer input | `apps/porthole/os/ui.h`, `apps/porthole/os/input.h` |

The prior local checkout had only eight games; this package uses the fetched nine-game baseline. Avoid carrying that stale capacity assumption into implementation. Also, `docs/hardware.md` has an outdated “Not used yet” network note; use the live firmware/app brief for present network behavior.

## Integration decisions

1. Build one native game in `apps/porthole/games/racing/` with its own `AGENTS.md` and the required `CLAUDE.md` symlink. No new app/firmware stack or replacement of existing games.
2. Use `App::surface()` with `SURFACE_RGB565`, existing input, time and lifecycle contracts. Prefix screen names, for example `race_title`, `race_tracks`, `race_calibrate`, `race_countdown`, `race_play`, `race_pause`, `race_results`.
3. Register on firmware and host. Racing is the **tenth game**: extend capacity and pagination narrowly, preserve all existing tile positions, verify label wrapping, initialization and per-game store enumeration. Do not only append an entry past `MAX_APPS`. No save-layout change is justified solely by a new stateless racing game.
4. Follow the wheel-control math as a precedent without including another game's private header. Keep racing-specific input local. Extract a shared runtime helper only if real current consumers require it and behavior is preserved.
5. Race state is RAM-only for the first version. No best-time, currency, unlock or championship store. Respect shell profile/time-limit/mute rules; leaving/re-entering begins a new run. Pause is game-owned; `App` has lifecycle/exit hooks, not a general pause system.
6. Keep simulation independent of rendering and use bounded timestep processing. Track progress, lap order, collisions, projectiles and pickups must agree at low and high rendering rates. Bound object counts and eliminate per-frame allocation.
7. Source PNGs stay in design docs unchanged. They are not sprite sheets or firmware-ready assets. Author/convert game-owned backgrounds, car poses, hazards, pickups, buttons and effects into repository-supported formats; preserve source, deterministic conversion and freshness checks. Use packed image/RLE paths where appropriate after profiling. A whole poster is not a playable background.

## Performance contract

No racing frame rate, spare flash budget or free-memory margin has been measured. The following are proposed acceptance targets, not verified performance:

- Aim for stable **30 FPS** (roughly 33 ms per frame) at native output, with steering response visible within about 100 ms. Measure update, draw, present and worst-frame timing on the device with serial `M`; retain evidence and report actual results.
- Prefer flat road spans, precomputed projection information, opaque/baked imagery and a few scenery layers. Do not decode a large compressed backdrop every frame without measuring its cost. Keep particles/projectiles in fixed pools; tune scene density before reducing input reliability or legibility.
- Use existing framebuffer ownership and redraw requirements. Both RGB565 buffers must contain correct full frames; indexed-shell transitions invalidate assumptions about their contents. Verify round-trip transitions and effects without stale pixels.
- Allocate large renderer scratch only after PSRAM initialization, using the repository's allocation patterns; do not introduce global image-sized internal-RAM arrays. Never raise the 128 KiB static-DRAM ceiling, OTA slot limit or weaken the OTA headroom check to make the build pass.
- Four track themes should share mechanics and projection code. Reuse structural rendering primitives, not other games' visible art/palettes/fonts. Quantify total asset flash and peak memory before committing the complete set.
- If hardware misses the target, reduce decoration, particle counts and repeated decoding first; report the measured trade-off. Host/emulator speed is not proof of device speed.

## Acceptance matrix for implementation

| Area | Required proof |
| --- | --- |
| Entry and shell | Existing nine games still launch and retain their positions; racing is reachable and fits; every screen exits; system time limits still work |
| Wheel | Comfortable upright neutral calibrates; clockwise/right and counterclockwise/left; no unwanted drift in dead zone; flat/shaking input cannot start; resume/recenter does not advance time |
| Touch | Full HUD rectangles/glyphs inside circle; physical/logical scaling correct; one press spends one charge; hold/drag/carried touches cannot repeat; no simultaneous-touch dependency |
| Inventory | Separate counts stay 0–3; empty, full, cooldown and active-boost presses have the specified effect; one pickup gate per pass/lap; replay resets state |
| Speed | Every hit is once per contact with protection; strongest terrain cap wins; Boost does not bypass terrain; speed floor cannot override countdown/pause/results |
| Rocket | Hit, miss, cooldown, road edge, airborne hit, protected target and despawn; no kill, health or car removal |
| Jumps | Ramp and bypass work; ground hazards skipped only in flight; raised/ground pickups differ; fixed landing time even after hit; no falling or duplicate ramp trigger |
| Laps/place | Starting line not a lap; ordered course traversal; skipped-segment/high-speed crossing; stable ties; early AI finish; player can finish fourth; results lock once |
| Tracks | Complete all three laps of all four tracks using normal visible controls; no blocked corridor or unreadable hazards; each theme recognizable at 480×480 |
| Pause/replay/exit | All simulation timers freeze; resume exact state; menu touches do not leak; replay is clean; returning from shell produces correct frames |
| Resources | Firmware fits the existing flash/RAM guards; device evidence for frame time and memory, or explicit physical-device verification gap |

Test business outcomes and boundaries, not exhaustive permutations or library behavior. Place host tests in `apps/porthole/host/test_racing*.cpp` and wire their source list in the app Makefile. Use the existing playtest and content/art checks. Preserve all existing save formats and warning/error rules.

Before calling implementation complete: run `make check`, `make ci`, the firmware build and the repository review gate; fix failures. Perform the final-build black-box emulator procedure in `apps/porthole/docs/playtesting.md`, using ordinary visible touch/tilt inputs through full races, results, replay and exit. Save screenshots and input traces; inspect visual quality and replay after fixes. State-aware bots, debug stage jumps, synthetic screenshots and unit tests do not replace that playthrough. Device steering comfort and hardware performance remain separate checks if no board is connected.

This **design-only** PR requires document/asset verification, review and repository checks. It does not claim implemented gameplay or a completed gameplay playthrough.
