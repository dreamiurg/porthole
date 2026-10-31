# Original artwork and provenance

## Delivered binaries

| File | Source | Format | Size |
| --- | --- | --- | --- |
| [reference/generated-image.png](reference/generated-image.png) | Attachment returned by the reader for ChatGPT conversation `6acfa02b-4738-83ea-a3f5-9733fac03df8` | PNG, RGB, 1536×1024 | 2,390,407 bytes |
| [reference/level-showcase-upload.png](reference/level-showcase-upload.png) | User-uploaded `codex-clipboard-41d343db-21e7-4077-84c5-e189ec575b13.png` in this continuation | PNG, RGB, 1536×1024 | 2,490,306 bytes |

SHA-256 of `generated-image.png`: `77602edf392de8c7946d65cf79c9483de437deb50069ad237165382ff779c360`.

SHA-256 of `level-showcase-upload.png`: `c6cde5eebcc5bbd8a79325be40f0e370c199898757858283edfa0b500c63d85e`.

Both files were copied as bytes. No crop, re-encoding, upscaling, regeneration, SVG substitute or screenshot replacement was made. They are ordinary Git blobs, not Git LFS pointers. The new upload has a descriptive repository filename; its bytes are unchanged. [SHA256SUMS](SHA256SUMS) provides a portable integrity check for both files.

Visual inspection of `generated-image.png` confirms an eight-panel board. The **bottom row**, labeled **Style 2 — Rugged Action Arcade**, contains the user-selected four environments:

| Panel | Track | Read from the original |
| --- | --- | --- |
| 2A | Desert Canyon | Orange canyon, dusty road, compact rear-view car |
| 2B | Industrial Ruins | Rusted towers, amber light, industrial road |
| 2C | Snow Track | Snowy mountains/pines, dark road, cold blue palette |
| 2D | Night Run | Neon city, wet-looking street, indigo and magenta |

Top-row panels 1A–1D are earlier alternatives. The bottom-row style selection is supported by the user's preference for those four scenes. The image still contains a mini-map and button charge rings; the later written requirements remove them. It does not prove the final HUD geometry is safe on the real circular glass.

## User-supplied level showcase

`level-showcase-upload.png` contains:

- Top row: Desert Canyon, Industrial Ruins, Snow Track and Night Run, in that order. Each shows the rear-view car, lap/place and separate Rocket/Boost counts.
- Middle row: HUD A Minimal, B Curved edge, C Compact top badges, D Very minimal. A is labeled “default” in the image; this is image content, not an additional user approval. D lacks a lap counter, so it cannot be copied as the final HUD. Any arcs must not imply charge/health meters.
- Bottom row: Boost, Rocket, Shield, Jump ramp, Magnet and Mine examples. **Only Boost, Rocket and Jump ramp are in scope.** The explicit no-shield/no-health rule remains; Magnet and Mine add unrequested mechanics and are not part of this implementation.

The board supplies the missing visual content for a level/HUD showcase. It does not establish the historical `/mnt/data` filename or byte identity, and does not override the written target geometry, small-car/clear-road rules, or pause/exit requirements.

## Missing originals

| Requested original path | Verified status |
| --- | --- |
| `/mnt/data/a_clean_concept_art_game_ui_level_showcase_poster.png` | Historical file not accessible; the user supplied the level/HUD board as a clipboard PNG, preserved as `reference/level-showcase-upload.png`. Byte identity to the historical path is unverified |
| `/mnt/data/a_clean_concept_art_game_ui_moodboard_poster_in.png` | Not returned by the conversation reader; no local file available |
| `/mnt/data/generated-image.png` | Original path unavailable; a same-named conversation attachment was recovered and visually inspected as described above |

`/mnt/data` belongs to the former chat environment and is not mounted here. A filename in conversation text is not downloadable file content. No claim is made that the recovered attachment is byte-identical to an inaccessible `/mnt/data` file; its own provenance and checksum are recorded.

To complete recovery, supply the remaining moodboard PNG or a working direct download URL. To establish exact historical showcase identity, supply that original or its independently recorded checksum. Decode/inspect any added PNG, record its byte size and SHA-256 hash, preserve the bytes under `reference/`, and update this inventory. Do not duplicate an available file under a missing name, generate approximations, or claim historical identity from visual similarity alone.

If Git cannot accept recovered originals, put **the real bytes** plus checksums in a ZIP at a location the implementation session can read, then download/extract it from that session and verify hashes. An archive of missing files or an untested URL is not a fallback. Both delivered PNGs fit in ordinary Git, so they do not require external hosting or LFS.

## Read and verify in a fresh checkout

From the repository root:

```sh
git fetch origin design/porthole-racing
git worktree add ../porthole-racing-read --detach origin/design/porthole-racing
cd ../porthole-racing-read/docs/superpowers/specs/porthole-racing/assets
shasum -a 256 -c SHA256SUMS
file reference/*.png
```

Use a free worktree path if that example path exists. On Linux, `sha256sum -c SHA256SUMS` is equivalent. Open both actual PNGs with the session's image viewer before authoring visuals. Verify 1536×1024 decoding in addition to the hashes. GitHub's rendered preview alone does not establish the downloaded bytes' integrity.
