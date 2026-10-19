# Living Kingdom artwork and typography

The selected artwork is the original rich pixel-illustrated concept board08,
reused directly through exact crops. Missing and UI-obscured artwork comes from
GPT Image Gen using that board and its crops as references. See
[board08/README.md](board08/README.md) for untouched sources, crop provenance,
state selection and the exact runtime resizing transforms. The prior SVG and
flat generated interpretation is no longer used by the production pipeline.

`../tools/kingdom_art.py` verifies all source/crop hashes and pixels, then emits
losslessly compressed RGB565 assets and the indexed launcher icon. Scenes are
stored at 240×120 and displayed at 480×240 by exact 2× enlargement; colors are
never reduced to a small palette. The bounded ROM-miniz/host-zlib decoder and
cache format are described in [encoded/README.md](encoded/README.md).

## Typography

Message text uses the exact **Atkinson Hyperlegible Regular** font from the
adult UI in PR #80, commit `ec62db400f9d4681a49b4927733ffbf1ba5a3abf`.
The bundled 54,348-byte TTF retains its original family name and data. Body,
small UI text and choice descriptions prioritize readable letter shapes;
decorative headings and bold choice captions use **Willowbook Serif**, a
renamed subset of Noto Serif 2.004.

| Runtime face | Typeface | Native size | Line height |
| --- | --- | ---: | ---: |
| `BODY` | Atkinson Hyperlegible Regular | 24 px | 30 px |
| `SMALL` | Atkinson Hyperlegible Regular | 22 px | 26 px |
| `DETAIL` | Atkinson Hyperlegible Regular | 20 px | 24 px |
| `HEADING` | Willowbook Serif Bold | 24 px | 28 px |
| `TITLE` | Willowbook Serif Bold | 32 px | 36 px |
| `LABEL` | Willowbook Serif Bold | 22 px | 26 px |

All six faces include printable ASCII 32..126 and ten punctuation glyphs:
middle dot, multiplication sign, en/em dashes, curly single/double quotes,
bullet and ellipsis. The native RGB565 renderer uses proportional glyphs,
16-level antialiased coverage and original advances with real class kerning.
Glyphs are rendered at their target size without horizontal condensation or
pixel-grid enlargement.

Atkinson Hyperlegible is copyright 2020 Braille Institute of America, Inc.;
its SIL Open Font License 1.1 is included in
[fonts/Atkinson-OFL.txt](fonts/Atkinson-OFL.txt). Noto Serif is copyright 2015
Google LLC, with its SIL Open Font License 1.1 in
[fonts/OFL.txt](fonts/OFL.txt). Willowbook's subset names identify its derivative
status; the original Noto outlines, metrics and applicable kerning remain.
[fonts/README.md](fonts/README.md) records source checksums, provenance,
baseline metrics and the optional reproducible subsetting command.

`../tools/font.py` reads only bundled, checksum-verified TTFs and emits
`../generated/fonts.h`. Byte-identical font output has been verified with
Python 3.12.14, fontTools 4.61.1, and both Pillow 12.3.0 and 13.0.0 using
FreeType 2.14.3. The converter checks line-box fit and lossless kerning-class
compression; `--check` compares the complete generated header byte for byte.
Normal conversion does not download fonts or select installed system fonts.

## Regeneration and checks

From the application directory:

```sh
python3 -W error games/living-kingdom/tools/kingdom_art.py
python3 -W error games/living-kingdom/tools/kingdom_art.py --check
python3 -W error games/living-kingdom/tools/font.py
python3 -W error games/living-kingdom/tools/font.py --check
```

Generated headers must not be edited directly. Font generation writes its
current specimen to `build/art/living-kingdom-font/preview.png`. Native C++
renderer samples and 480px/201px body-font comparisons are documented in
[fonts/README.md](fonts/README.md). Final screenshots and ordinary-input
emulator playthroughs verify the complete screens, circular fit and reading
experience.
