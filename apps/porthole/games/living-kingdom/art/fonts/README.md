# Kingdom typography

Reading and other message text use the exact **Atkinson Hyperlegible Regular**
font selected for the adult UI in PR #80. Readability takes priority over
matching the reference's body serif. Decorative headings and bold choice
captions retain **Willowbook Serif**, derived from Noto Serif, for the warm
storybook character of `starter-screens.svg`.

| Runtime symbol | Typeface | Native size | Line height | Baseline from bottom |
| --- | --- | ---: | ---: | ---: |
| `BODY` | Atkinson Hyperlegible Regular | 24 px | 30 px | 7 px |
| `SMALL` | Atkinson Hyperlegible Regular | 22 px | 26 px | 6 px |
| `DETAIL` | Atkinson Hyperlegible Regular | 20 px | 24 px | 5 px |
| `HEADING` | Willowbook Serif Bold | 24 px | 28 px | 7 px |
| `TITLE` | Willowbook Serif Bold | 32 px | 36 px | 9 px |
| `LABEL` | Willowbook Serif Bold | 22 px | 26 px | 6 px |

At the selected native raster size, body capitals have 16 px of ink and the
lowercase x-height is 12 px. These are proportional, antialiased glyphs with
their original bearings and advances. No glyph is horizontally condensed or
stretched. The runtime's existing `font::Font` interface is unchanged.

## Reading font source and license

`AtkinsonHyperlegible-Regular.ttf` is the exact 54,348-byte original from
PR #80, commit `ec62db400f9d4681a49b4927733ffbf1ba5a3abf`.
Its original family name and font data are unchanged: Atkinson Hyperlegible
Regular **1.006**, copyright 2020 Braille Institute of America, Inc.
Its SIL Open Font License 1.1 is included in
[Atkinson-OFL.txt](Atkinson-OFL.txt).

```text
AtkinsonHyperlegible-Regular.ttf
7fb917c89019896d0b52ee84b7cbb3304c18cb90b19a62f5e32712bd23e97669
```

## Decorative font source and license

The decorative source is Noto Serif Regular/Bold **2.004**, from Debian's
`fonts-noto-core` package **20201225-2**, originally installed at
`/usr/share/fonts/truetype/noto/NotoSerif-{Regular,Bold}.ttf`.
Upstream project: <https://github.com/notofonts/noto-fonts>.

Original source SHA-256 values:

```text
Regular 9d7583b7dc9e812afd32a14280c5cac3160012efe50c8d08938f4fea266ff67f
Bold    0af0ff2be8f84910fb21ec5fe1b6b7395e3073250502a334baf6ca2f860c88fe
```

The unchanged originals exceed the repository's per-file size limit. The
bundled TTFs are reproducible subsets with renamed family, style, full-name,
unique-ID and PostScript-name records. Glyph outlines, metrics and applicable
kerning are preserved; unused scripts, glyphs and shaping features are removed.
The regular subset is 26,892 bytes and bold is 27,660 bytes. Bold supplies the
three decorative runtime faces; regular remains available for the documented
body-font comparison.

Copyright 2015 Google LLC. All Rights Reserved. Both the bundled derivative
fonts and their generated bitmap/metric data remain under the **SIL Open Font
License 1.1**, included in [OFL.txt](OFL.txt). The name change identifies these
subsets as derivatives and does not claim original authorship of Noto's glyphs.

## Conversion and reproduction

From the repository root:

```sh
python3 apps/porthole/games/living-kingdom/tools/font.py
python3 apps/porthole/games/living-kingdom/tools/font.py --check
```

Generation only reads the bundled, checksum-verified font files. No system
font selection, font download, JavaScript package or external font converter is
required. Python dependencies are Pillow and fontTools. Output has been checked
byte-for-byte under both Pillow **12.3.0** and **13.0.0**, each using FreeType
**2.14.3**, with fontTools **4.61.1** and Python **3.12.14**. Keep the build
dependency versions pinned when updating the rendering toolchain.

To reproduce the small renamed TTFs from the verified originals as well:

```sh
python3 apps/porthole/games/living-kingdom/tools/font.py \
  --subset-source /usr/share/fonts/truetype/noto
```

The script checks both upstream and derivative checksums. Subsetting preserves
the original timestamps and uses canonical table ordering, so it does not
insert the current date into the font files.

Pillow/FreeType rasterizes each glyph at its native target size. Coverage is
rounded to 16 levels and packed high nibble first, exactly as `os/font.cpp`
expects. Original `hmtx` advances and GPOS kerning are converted to 1/16-pixel
units. GPOS uses the first matching subtable per lookup, preserving exception
pairs. Lossless row/column compression produces 52 left classes and 40 right
classes for Atkinson, with a 2,080-byte matrix per size. The serif faces use
34 by 28 classes and 952-byte matrices. The converter verifies
every compressed pair against its uncompressed value and rejects ink outside
the declared line box.

Every face includes ASCII **32..126**, plus middle dot, multiplication sign,
en/em dashes, curly single/double quotes, bullet and ellipsis. These extras are
registered through the existing UTF-8 font interface.

Generation writes a specimen to
`apps/porthole/build/art/living-kingdom-font/preview.png`. During selection, a
real C++ `os/font.cpp` reading sample and specimen were also rendered at 480px:
`runtime-reading.png` and `runtime-specimen.png` in that directory. The actual
renderer comparison against the previous serif body is saved as
`atkinson-comparison-480.png` and `atkinson-comparison-201.png` (201px per
display, an approximate 2.1-inch presentation on a 96dpi monitor). Font fit,
circular safety and final in-game screens still use the ordinary compiled
content/UI tests and emulator playthroughs.
