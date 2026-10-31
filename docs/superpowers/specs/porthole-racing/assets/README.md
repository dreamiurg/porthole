# Original artwork and provenance

## Delivered binary

| File | Source | Format | Size |
| --- | --- | --- | --- |
| [reference/generated-image.png](reference/generated-image.png) | Attachment returned by the reader for ChatGPT conversation `6acfa02b-4738-83ea-a3f5-9733fac03df8` | PNG, RGB, 1536×1024 | 2,390,407 bytes |

SHA-256: `77602edf392de8c7946d65cf79c9483de437deb50069ad237165382ff779c360`.

The file was copied as bytes. No crop, re-encoding, upscaling, regeneration, SVG substitute or screenshot replacement was made. It is a Git blob, not a Git LFS pointer. [SHA256SUMS](SHA256SUMS) provides a portable integrity check.

Visual inspection confirms an eight-panel board. The **bottom row**, labeled **Style 2 — Rugged Action Arcade**, contains the user-selected four environments:

| Panel | Track | Read from the original |
| --- | --- | --- |
| 2A | Desert Canyon | Orange canyon, dusty road, compact rear-view car |
| 2B | Industrial Ruins | Rusted towers, amber light, industrial road |
| 2C | Snow Track | Snowy mountains/pines, dark road, cold blue palette |
| 2D | Night Run | Neon city, wet-looking street, indigo and magenta |

Top-row panels 1A–1D are earlier alternatives. The bottom-row style selection is supported by the user's preference for those four scenes. The image still contains a mini-map and button charge rings; the later written requirements remove them. It does not prove the final HUD geometry is safe on the real circular glass.

## Missing originals

| Requested original path | Verified status |
| --- | --- |
| `/mnt/data/a_clean_concept_art_game_ui_level_showcase_poster.png` | Not returned by the conversation reader; no local file available |
| `/mnt/data/a_clean_concept_art_game_ui_moodboard_poster_in.png` | Not returned by the conversation reader; no local file available |
| `/mnt/data/generated-image.png` | Original path unavailable; a same-named conversation attachment was recovered and visually inspected as described above |

`/mnt/data` belongs to the former chat environment and is not mounted here. A filename in conversation text is not downloadable file content. No claim is made that the recovered attachment is byte-identical to an inaccessible `/mnt/data` file; its own provenance and checksum are recorded.

To complete recovery, supply the two actual original PNGs or working direct download URLs. Decode/inspect them, record their byte sizes and SHA-256 hashes, add them under `reference/` without modifying bytes, and update this inventory. Do not copy the available file under the missing names, generate approximations, or mark the transfer complete from a text link alone.

If Git cannot accept recovered originals, put **the real bytes** plus checksums in a ZIP at a location the implementation session can read, then download/extract it from that session and verify hashes. An archive of missing files or an untested URL is not a fallback. The currently delivered PNG fits in ordinary Git, so it does not require external hosting or LFS.

## Read and verify in a fresh checkout

From the repository root:

```sh
git fetch origin design/porthole-racing
git worktree add ../porthole-racing-read --detach origin/design/porthole-racing
cd ../porthole-racing-read/docs/superpowers/specs/porthole-racing/assets
shasum -a 256 -c SHA256SUMS
file reference/generated-image.png
```

Use a free worktree path if that example path exists. On Linux, `sha256sum -c SHA256SUMS` is equivalent. Open the actual PNG with the session's image viewer before authoring visuals. Verify 1536×1024 decoding in addition to the hash. GitHub's rendered preview alone does not establish the downloaded bytes' integrity.
