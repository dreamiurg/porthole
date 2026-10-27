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

## Scene interface production layers (2026-10-27)

The approved `../../../docs/toy-town-life/scene-interface/approved-school-reference.png`
was inspected before creating these original Image Gen sources. They carry its
warm outlined anime classroom language into independently rendered game layers;
the reference itself is never embedded in runtime art. Original shipped concept
PNGs and customization doll/material data remain intact.

- `scene-world-atlas.png`: four empty places in a 2×2 atlas: cozy doll bedroom,
  school art room with unfinished cork display, indoor cafe with serving counter
  and empty tray, park with distinct bench/fountain/path. The converter replaces
  background slots 1–4 and retains slot 0's original street.
- `scene-sprite-atlas.png`: transparent full-body Mina, Theo, Nori and server;
  framed paintings, cue cards, paper tray, ribbon notebook, ribbon, sandwich,
  fruit bowl, water, cookie, bag, pencil and camera. Actual row boundaries are
  explicitly cropped because generated sheets are not perfectly even grids.
- `scene-pose-atlas.png`: Mina waiting and rehearsing, Theo hanging a rocket
  painting and planning. These are separate illustrated poses, not palette swaps.
- `scene-objects-atlas.png`: empty serving tray, schoolbook and finished pinned
  art display, all transparent and independently placed at runtime.

`scene-bag-atlas.png` adds the actual checked picnic cloth and magnifying glass
used by packing controls, avoiding misleading bag/camera substitutions.
`placePicture(scene, box)` samples the exact production scene into map cards
without allocating an intermediate image.

All six new source images were generated with Image Gen for this game. The
brief specified warm painterly anime, clean dark brown outlines, generous clear
floor for layered characters, recognizable place landmarks, no words or UI, and
independently isolated sprites. Character briefs: Mina has long purple hair,
navy pinafore and purple bow; Theo has curly brown hair, green cardigan and
shorts; Nori wears a teal beret and yellow cardigan. Sprites use actual source
alpha with a 128 cutoff, **never green extraction** (Theo's clothing is green).
Tiny detached neighbor-cell alpha islands below 2% of the largest connected
component are removed before cropping; the original source PNGs are preserved.
At conversion, bounded thumbnails use 63 local colors plus transparent index 63;
all source images and fonts participate in cache SHA-256 verification.

`art::npc` fits a sprite into a box. `art::npcPortrait` samples its upper 55%
into picture controls so facial expressions and Mina's bow remain recognizable. Mina pose 0 holds a card, 1 waits, 2 or higher
rehearses; Theo 0 holds a picture, 1 plans, 2 or higher hangs a picture. Other NPCs
have one pose. `art::prop` fits an illustrated object. `HUNG_PICTURES` is the
completed display. Runtime drawing unpacks individual indexed pixels directly
into the existing RGB565 surface, without heap allocation or frame buffers.
`walkingDoll` preserves every cosmetic layer and alternates foot lift with an
elapsed-millisecond phase; phase 0 uses the existing exact production detail.

Open Sans Bold `ACTION` is independently converted at 18px with a 21px line;
BODY/TITLE retain the shipped 24/30px faces. Combined estimated art runtime cost
is recorded by the generated manifest (approximately 570 KB, including the
original assets). Conversion also writes `world-sprites.png` beside `assets.png`
for inspection. `--check` uses the committed source-verified cache and remains
independent of the installed Pillow conversion version.

`scene-pictures-atlas.png` adds three separately illustrated front-facing framed
paintings for hands-on School picture hanging: flowers, a rocket, and a rabbit.
`FLOWER_PICTURE`, `ROCKET_PICTURE`, and `RABBIT_PICTURE` are contiguous prop IDs,
with individually converted transparent sprites recognizable at 60×70px. The
original stack of pictures remains a separate prop.
