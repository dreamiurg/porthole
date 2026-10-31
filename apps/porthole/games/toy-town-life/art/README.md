# Toy Town Life artwork

The original reference is the approved **4 — Anime Slice-of-Life** concept in
`docs/superpowers/specs/toy-town-life/assets/reference/toy_town_life_six_worlds.png`.
No other game's sprites, colors or typefaces supply this game's art.

The three committed PNGs are original OpenAI Image Gen production artwork,
generated for this game on 2026-10-25. They are conversion inputs, not screenshots
of another product. The source files retain their original resolution:

- `scene-atlas.png`: four square, empty environments with a small white divider:
  cherry-blossom village street, cozy bedroom, school art room and outdoor cafe.
- `park.png`: cherry-blossom park with a bench, path, flowers and fountain.
- `doll-atlas.png`: four isolated, expressive anime dolls on green: a brown-haired
  dress-up girl, stitched lavender plush rabbit, brown-skinned clay boy and blue
  hair acrylic keychain girl. This supplies the game's material diversity.

`../tools/toytown_art.py --convert-source` crops the divider, extracts green,
resizes, independently quantizes each image and caches the converted pixel,
palette, material and glyph records in `converted/assets.json.zlib`. Normal
generation verifies every source SHA-256 and emits `../generated/art_data.inc`
from those records. Scene pixels have
six bits and 64 local colors at 240×240; runtime renders them at 2×. Doll pixels
have six bits with index 63 transparent at 96×160. Three-bit material maps identify
skin/fur, garment and shoe pixels for customization. Source SHA-256 values and
estimated runtime asset bytes are recorded in `../generated/manifest.json`.

The first skin, color and shoe options preserve the original artwork. Other
options recolor its shaded material pixels. Three hair silhouettes, three garment
patterns, three shoe treatments and two removable accessories provide visible
customization. No animation or saved state is embedded in an image.

`fonts/` independently bundles Open Sans Semibold and Bold from the system's
`fonts-open-sans` package, with the upstream copyright notice and Apache 2.0
license. The converter uses Semibold at 24px (28px line) and Bold at 30px (34px
line), 95 printable ASCII characters and native four-bit glyph coverage. Measuring
and drawing both use the same unkerned advances. No other game's font is used.

Run from the repository root:

```sh
python3 -W error apps/porthole/games/toy-town-life/tools/toytown_art.py
python3 -W error apps/porthole/games/toy-town-life/tools/toytown_art.py --check
```

Pillow is the only conversion dependency. Generation also writes the converted
scene/doll/font contact sheet to `apps/porthole/build/art/toy-town-life/assets.png`.
The launcher uses a unique face crop converted to the shell's required indexed
sprite format; gameplay uses only its own local image palettes.

Pillow 13 changed adaptive quantization/resizing, so full source conversion is
explicitly pinned to **Pillow==12.3.0**. Normal generation and `--check` read the
committed cache and produce identical C++ with Pillow 12 or 13. The cache contains
logical pixel and glyph records rather than generated C++ text; source hashes
prevent silently using it after any original PNG, font or shell palette changes.
The runtime does not link zlib for these assets; zlib only packages the development
cache. To recreate it from original sources in an isolated environment:

```sh
python3 -m venv /tmp/toy-town-conversion
/tmp/toy-town-conversion/bin/pip install Pillow==12.3.0
/tmp/toy-town-conversion/bin/python apps/porthole/games/toy-town-life/tools/toytown_art.py --convert-source
```
