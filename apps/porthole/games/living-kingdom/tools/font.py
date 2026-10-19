#!/usr/bin/env python3
"""Convert Atkinson reading text and Willowbook headings to native 4bpp faces.

Pillow/FreeType rasterizes glyph coverage. fontTools reads original advances and
GPOS kerning, compressed losslessly to the runtime's class-kerning interface.
No installed system font, font download, shaping library or external CLI is used.
"""

import argparse
import hashlib
from dataclasses import dataclass
from pathlib import Path

from fontTools import subset
from fontTools.ttLib import TTFont
from PIL import Image, ImageDraw, ImageFont

ROOT = Path(__file__).resolve().parents[1]
FONT_DIR = ROOT / "art/fonts"
PREVIEWS = ROOT.parents[1] / "build/art/living-kingdom-font"
EXTRAS = tuple(map(ord, "\u00b7\u00d7\u2013\u2014\u2018\u2019\u201c\u201d\u2022\u2026"))
CODEPOINTS = (*range(32, 127), *EXTRAS)
SOURCES = {
    "AtkinsonHyperlegible-Regular": "7fb917c89019896d0b52ee84b7cbb3304c18cb90b19a62f5e32712bd23e97669",
    "WillowbookSerif-Regular": "b62bfe85037dc26b8599836554c23c111ade744360aaaf34635a2ba734c44deb",
    "WillowbookSerif-Bold": "8b36590fef8100d04db1bfda349b3bd89e4957d8060bf5bf017ef09ff13fab9a",
}
ORIGINALS = {
    "Regular": "9d7583b7dc9e812afd32a14280c5cac3160012efe50c8d08938f4fea266ff67f",
    "Bold": "0af0ff2be8f84910fb21ec5fe1b6b7395e3073250502a334baf6ca2f860c88fe",
}


@dataclass(frozen=True)
class Face:
    name: str
    typeface: str
    size: int
    height: int
    baseline: int


FACES = (
    Face("BODY", "AtkinsonHyperlegible-Regular", 24, 30, 7),
    Face("SMALL", "AtkinsonHyperlegible-Regular", 22, 26, 6),
    Face("DETAIL", "AtkinsonHyperlegible-Regular", 20, 24, 5),
    Face("HEADING", "WillowbookSerif-Bold", 24, 28, 7),
    Face("TITLE", "WillowbookSerif-Bold", 32, 36, 9),
    Face("LABEL", "WillowbookSerif-Bold", 22, 26, 6),
)


def source(typeface):
    path = FONT_DIR / f"{typeface}.ttf"
    if hashlib.sha256(path.read_bytes()).hexdigest() != SOURCES[typeface]:
        raise ValueError(f"original font checksum changed: {path.name}")
    return path


def subset_face(directory, weight):
    original = directory / f"NotoSerif-{weight}.ttf"
    if hashlib.sha256(original.read_bytes()).hexdigest() != ORIGINALS[weight]:
        raise ValueError(f"unexpected upstream original: {original}")
    font = TTFont(original, recalcTimestamp=False)
    options = subset.Options()
    options.layout_features = ["kern"]
    options.name_IDs, options.name_languages = ["*"], ["*"]
    options.name_legacy, options.recalc_timestamp, options.canonical_order = True, False, True
    converter = subset.Subsetter(options=options)
    converter.populate(unicodes=CODEPOINTS)
    converter.subset(font)
    names = {
        1: "Willowbook Serif",
        2: weight,
        3: f"Willowbook Serif {weight} 1.0; Noto Serif 2.004 subset",
        4: f"Willowbook Serif {weight}",
        6: f"WillowbookSerif-{weight}",
        16: "Willowbook Serif",
        17: weight,
    }
    for record in font["name"].names:
        if record.nameID in names:
            record.string = names[record.nameID].encode(record.getEncoding())
    font.save(FONT_DIR / f"WillowbookSerif-{weight}.ttf")
    font.close()


def scale(units, face, upm):
    magnitude = (abs(units) * face.size * 16 + upm // 2) // upm
    return -magnitude if units < 0 else magnitude


def raster(face, char):
    font = ImageFont.truetype(str(source(face.typeface)), face.size, layout_engine=ImageFont.Layout.BASIC)
    mask, offset = font.getmask2(char, mode="L", anchor="ls")
    image = Image.frombytes("L", mask.size, bytes(mask)).point(lambda value: (value * 15 + 127) // 255)
    bounds = image.getbbox()
    if not bounds:
        return [], (0, 0, 0, 0)
    image = image.crop(bounds)
    left, top = offset[0] + bounds[0], offset[1] + bounds[1]
    width, height = image.size
    if top + face.height - face.baseline < 0 or top + height + face.height - face.baseline > face.height:
        raise ValueError(f"{face.name}: {char!r} exceeds its line")
    pixels = list(image.tobytes())
    if len(pixels) % 2:
        pixels.append(0)
    packed = [pixels[index] * 16 + pixels[index + 1] for index in range(0, len(pixels), 2)]
    return packed, (width, height, left, -top - height)


def pair_units(subtable, left, right):
    if left not in subtable.Coverage.glyphs:
        return None
    if subtable.Format == 1:
        pairs = subtable.PairSet[subtable.Coverage.glyphs.index(left)].PairValueRecord
        value = next((pair.Value1 for pair in pairs if pair.SecondGlyph == right), None)
        if value is None:
            return None
    elif subtable.Format == 2:
        first = subtable.ClassDef1.classDefs.get(left, 0)
        second = subtable.ClassDef2.classDefs.get(right, 0)
        value = subtable.Class1Record[first].Class2Record[second].Value1
    else:
        raise ValueError("unsupported GPOS pair format")
    return getattr(value, "XAdvance", 0)


def lookup_units(subtables, left, right):
    for table in subtables:
        value = pair_units(table, left, right)
        if value is not None:
            return value
    return 0


def kerning_matrix(font, face, names):
    features = font["GPOS"].table.FeatureList.FeatureRecord
    lookups = {index for record in features if record.FeatureTag == "kern" for index in record.Feature.LookupListIndex}
    tables = []
    for index in sorted(lookups):
        lookup = font["GPOS"].table.LookupList.Lookup[index]
        if lookup.LookupType != 2:
            raise ValueError("unsupported kerning lookup")
        tables.append(lookup.SubTable)
    upm = font["head"].unitsPerEm
    matrix = [[scale(sum(lookup_units(table, left, right) for table in tables), face, upm) for right in names] for left in names]
    if any(not -128 <= value <= 127 for row in matrix for value in row):
        raise ValueError("kerning exceeds signed 4-bit fractional pixel range")
    return matrix


def classes(vectors):
    unique, indices = [], [0]
    for vector in map(tuple, vectors):
        if not any(vector):
            indices.append(0)
        else:
            if vector not in unique:
                unique.append(vector)
            indices.append(unique.index(vector) + 1)
    return unique, indices


def kerning(font, face, names):
    matrix = kerning_matrix(font, face, names)
    rows, left = classes(matrix)
    columns, right = classes(zip(*rows, strict=True))
    values = [column[row] for row in range(len(rows)) for column in columns]
    for first, row in enumerate(matrix, start=1):
        for second, expected in enumerate(row, start=1):
            actual = values[(left[first] - 1) * len(columns) + right[second] - 1] if left[first] and right[second] else 0
            if actual != expected:
                raise ValueError("class compression changed a kerning pair")
    return left, right, values or [0], len(columns)


def array(kind, name, values):
    lines = [f"static const {kind} {name}[] = {{"]
    lines.extend("  " + ",".join(map(str, values[index : index + 32])) + "," for index in range(0, len(values), 32))
    return "\n".join([*lines, "};"])


def face_output(face):
    font = TTFont(source(face.typeface))
    cmap, upm = font.getBestCmap(), font["head"].unitsPerEm
    names = [cmap[code] for code in CODEPOINTS]
    data, glyphs = [], [(0, 0, 0, 0, 0, 0)]
    for code, name in zip(CODEPOINTS, names, strict=True):
        pixels, bounds = raster(face, chr(code))
        advance = scale(font["hmtx"].metrics[name][0], face, upm)
        glyphs.append((len(data), advance, *bounds))
        data.extend(pixels)
    left, right, values, count = kerning(font, face, names)
    sections = [array("uint8_t", f"{face.name}_BITMAP", data), f"static const font::Glyph {face.name}_GLYPHS[] = {{"]
    sections.extend("  {" + ",".join(map(str, metrics)) + "}," for metrics in glyphs)
    sections.extend(["};", array("uint8_t", f"{face.name}_LEFT", left), array("uint8_t", f"{face.name}_RIGHT", right)])
    sections.append(array("int8_t", f"{face.name}_KERN", values))
    sections.append(f"const font::Font {face.name} = {{{face.name}_BITMAP, {face.name}_GLYPHS, EXTRAS, {len(EXTRAS)},")
    sections.append(f"  {face.name}_LEFT, {face.name}_RIGHT, {face.name}_KERN, {count}, {face.height}, {face.baseline}}};")
    font.close()
    return "\n".join(sections)


def output():
    heading = "// Generated by living-kingdom/tools/font.py. Atkinson Hyperlegible + Willowbook Serif, SIL OFL-1.1; see art/fonts/.\n"
    heading += '#pragma once\n#include "font.h"\nnamespace kingdom { namespace art {\n'
    return heading + array("uint16_t", "EXTRAS", EXTRAS) + "\n" + "\n".join(map(face_output, FACES)) + "\n}}  // namespace kingdom::art\n"


def preview():
    PREVIEWS.mkdir(parents=True, exist_ok=True)
    image = Image.new("RGB", (1050, 650), "#fff0cf")
    draw = ImageDraw.Draw(image)
    for face, y in zip(FACES, (20, 90, 160, 230, 300, 370), strict=True):
        font = ImageFont.truetype(str(source(face.typeface)), face.size, layout_engine=ImageFont.Layout.BASIC)
        draw.text((24, y), f"{face.name} {face.size}px / line {face.height}:  Willowbrook kept its promises.", font=font, fill="#30263b")
    small = ImageFont.truetype(str(source("AtkinsonHyperlegible-Regular")), 22, layout_engine=ImageFont.Layout.BASIC)
    for y, text in (
        (440, "ABCDEFGHIJKLMNOPQRSTUVWXYZ  0123456789"),
        (490, "abcdefghijklmnopqrstuvwxyz  I l 1  O 0  rn m"),
        (540, "! ? . , : ; ( ) [ ] { } + - * / = # & @   \u201cA promise\u201d \u00b7 \u2026"),
    ):
        draw.text((24, y), text, font=small, fill="#30263b")
    image.save(PREVIEWS / "preview.png")


def main():
    parser = argparse.ArgumentParser(description=__doc__)
    parser.add_argument("--check", action="store_true")
    parser.add_argument("--subset-source", type=Path, metavar="DIRECTORY", help="recreate bundled derivatives from verified upstream originals")
    args = parser.parse_args()
    if args.subset_source:
        for weight in ORIGINALS:
            subset_face(args.subset_source, weight)
    path, data = ROOT / "generated/fonts.h", output()
    if args.check:
        if not path.exists() or path.read_text() != data:
            raise SystemExit("Kingdom fonts differ; run games/living-kingdom/tools/font.py")
        print(f"Kingdom reading/heading fonts match: {len(FACES)} faces, 95 ASCII + 10 punctuation glyphs, 4bpp coverage, class kerning.")
    else:
        path.write_text(data)
        preview()


if __name__ == "__main__":
    main()
