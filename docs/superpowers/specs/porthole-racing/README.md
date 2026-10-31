# Porthole Racing — design handoff

Design-only package for a short arcade racer on the round 480×480 Porthole. Turn the device like a steering wheel, collect Boost and Rocket charges, jump over trouble, and race for position. Cars never die. This branch adds no game implementation.

| Read | Purpose |
| --- | --- |
| [Gameplay brief](gameplay-brief.md) | Required mechanics, proposed tuning, steering, circular HUD, race flow |
| [Four tracks](tracks.md) | Desert Canyon, Industrial Ruins, Snow Track, Night Run |
| [Platform and acceptance](implementation-contract.md) | Verified repository constraints, integration dependencies, completion checks |
| [Original artwork and provenance](assets/README.md) | The actual PNG, checksum, selected panels, missing-source inventory |
| [Codex task](CODEX-TASK.md) | Self-contained implementation handoff |

## Visual direction

The selected direction is **Style 2 — Rugged Action Arcade**, the bottom row of this original board. Panels **2A–2D** correspond to the four requested tracks. Preserve the small rear-view car, open road, strong silhouettes, warm canyon/industrial scenery, cold snow scenery, and vivid neon night. The top row is historical exploration, not another set of levels to build.

![Original style board; use bottom-row panels 2A–2D](assets/reference/generated-image.png)

The artwork is a style reference. The written brief overrides obsolete elements: remove charge rings, shields/health, and the mini-map; simplify lap/place; put complete touch targets inside the glass. Do not reproduce oversized traffic or control clipping from a concept panel.

## What is confirmed, proposed, and unavailable

- **Confirmed by the user:** four named tracks, round 480×480 display, steering by device rotation, Boost and Rocket, road pickups capped at three, jumps, collisions that cost speed/rank, no death/lives/health/shield, simple HUD, restrained traffic and small car.
- **Proposed design defaults:** three laps, four racers, 90–120 second clean runs, separate inventories of three per type, exact coordinates, tuning, AI and pause behavior below. These have not been playtested or approved individually.
- **Verified repository baseline:** `fedf62ce0c804dcb20ef3e7e7ee11ba9545cecb8`; the integration contract distinguishes current capabilities from racing work still needed.
- **Available original:** `generated-image.png`, 1536×1024, 2,390,407 bytes, copied unchanged from the referenced ChatGPT conversation attachment.
- **Unavailable originals:** `a_clean_concept_art_game_ui_level_showcase_poster.png` and `a_clean_concept_art_game_ui_moodboard_poster_in.png`. The chat reader returned neither binary; `/mnt/data` is not mounted on this host. Their appearance and relationship to the available image are unverified. They are not silently replaced, renamed, regenerated, or claimed as delivered.

Source: ChatGPT conversation `6acfa02b-4738-83ea-a3f5-9733fac03df8`, “Porhole Games”, and the user's explicit continuation request. This package contains the relevant decisions, so implementation does not depend on reading that chat.

The available selected-style board supports implementation, but **the requested three-file artwork transfer is incomplete** until the two missing originals are supplied. See [asset recovery](assets/README.md#missing-originals).
