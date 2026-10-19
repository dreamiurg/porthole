# Selected board08 artwork

The authoritative, untouched original is
`docs/superpowers/specs/living-kingdom/concept-art/08-chapter-one-polished-ux.png`
at repository root. This is the user's selected rich pixel-illustrated board,
not the previously rejected flat SVG interpretation.

`manifest.json` records SHA-256 hashes for that original and every generated
source atlas, plus exact pixel crop rectangles and hashes for every extracted
PNG. The generator verifies every crop pixel against its source. No image is
painted, traced, or reconstructed with programmatically drawn shapes.

## Direct original crops

The title retains its original wordmark and party, ending above the baked CTA.
Village, forest, ruins and sunset are cropped between baked UI elements.
Princess, Lucas and Dribble come directly from the party screen; bread and map
come directly from the inventory screen. The launcher is a resized Princess
crop converted to the shell's mandatory indexed palette.

## GPT reference edits and missing assets

GPT Image Gen was given the selected complete board and its extracted village,
forest or map images as visual references. Untouched results are retained:

- `atlas-base.png`: broken bridge, repaired bridge, mill, marsh and bridge
  channel work. The sixth panoramic map study is retained in the source but
  intentionally unused in the game.
- `atlas-water.png`: mill-priority and marsh-priority consequences for both
  the mill and dragon marsh.
- `atlas-work.png`: timed sharing and completed shared-channel consequences
  for both locations. Timed gates have hourglasses; the completed channel
  visibly divides the river.
- `world-map.png`: GPT edit of the original map crop to remove labels, pins,
  arrow and circular frame, completing terrain beneath the obscured areas.
- `atlas-items.png`: missing rope, timber, thyme and flour icons, plus blank
  textured parchment, gold, lavender and navy panel skins.

The original atlases remain untouched. Individual runtime sources are exact
rectangular crops. Prompt intent is documented above; generated anatomy,
architecture, textures and geometry are used directly, never code-drawn.

## Documented runtime transforms

All sources are converted directly to RGB565 without palette reduction.
Scene panoramas use Pillow `ImageOps.fit` with Lanczos to 240×120; map is
240×240 and title 240×175. These are displayed by exact nearest-neighbor 2×
blits, yielding physical 480×240, 480×480 and 480×350 rectangles. This preserves
rich colors while matching the original board's approximately 300px artwork
and fitting firmware flash. No invisible-circle masking is applied.

Portraits preserve aspect within 80×112, bottom-aligned on navy; items preserve
aspect within 48×48, also on navy. Blank panel crops are resized to 48×48 and
sampled with six-pixel nine-slice borders and tiled centers. The code samples
existing pixels only; it does not draw new illustrative geometry. The title,
map and scene viewport positions are controlled by the screen renderer.

## State selection

Village, woods, ruins and map remain unchanged for all outcomes. Mill and marsh
select the corresponding consequence image only after the delayed event.
An open bridge always selects the repaired image, regardless of water outcome.
A closed bridge uses channel work only for outcome four; other outcomes keep
its base broken-bridge image. Sunset is the original rest/camp panorama.

## Reproduction

From `apps/porthole`:

```sh
python3 -W error games/living-kingdom/tools/kingdom_art.py --check
# Optional explicit crop regeneration from hash-verified source PNGs:
python3 -W error games/living-kingdom/tools/kingdom_art.py --extract-source
# Only when native transforms/source crops change, with zopfli==0.4.3:
python3 -W error games/living-kingdom/tools/kingdom_art.py --recompress
```

Normal generation/checking requires Pillow and Python's standard zlib, without
network access or image-generation services. Compression provenance and bounded
native decoding are described in [../encoded/README.md](../encoded/README.md).
