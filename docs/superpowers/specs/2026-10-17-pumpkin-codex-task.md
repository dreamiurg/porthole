# Codex implementation task — Pumpkin Hide & Seek

Start from branch `design/pumpkin-hide-and-seek` in `dreamiurg/porthole`. Read `docs/superpowers/specs/2026-10-17-pumpkin-hide-and-seek-design.md`, root `CLAUDE.md`, `apps/porthole/CLAUDE.md`, and sibling game instructions.

Implement a **first playable vertical slice** within the existing Porthole firmware as `apps/porthole/games/pumpkin-hide-seek/` and wire it into the launcher. Do not create a separate app. The target is the 480×480 round Waveshare ESP32-S3 touchscreen using tilt and touch.

Use distinctive crisp retro pixel art, ideally the existing 160×160 indexed framebuffer scaled 3×. Pumpkin rolls with tilt, taps to hide in 2–3 spots, Grandma searches with a flashlight cone, surviving a short forgiving round wins. Getting caught triggers a humorous game-over: orange icing theatrically drips downward **from the upper circular edge**, covering the scene, revealing pumpkin pie with a large legible **YOU LOST** label and **TRY AGAIN** tap target. No gore, jumpscares, lives or loss of progress.

Visual storyboard beats: (1) smiling pumpkin on rug, moonlit cozy room; (2) pumpkin rolls behind a chair while Grandma enters with flashlight; (3) pumpkin peeks from under furniture as the light passes; (4) orange icing drips, then pie and YOU LOST. The longer design document preserves the direction of the images created in ChatGPT; **the generated raster storyboards themselves are not attached to this branch**.

Prioritize animation, character, legibility, circular clipping, and simple inputs over complexity. Follow project conventions and shared runtime, emulator, back navigation, mute, automated tests, screenshot checks and firmware build. Validate using `make -C apps/porthole check` and `make -C apps/porthole ci` plus relevant firmware checks. Report what passes and what cannot be tested. Open a PR for review; do not merge.
