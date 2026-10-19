#!/usr/bin/env python3
"""Compose documentation from unchanged board08 and frozen emulator captures."""

import hashlib
import json
from pathlib import Path

from PIL import Image, ImageDraw, ImageFont, ImageOps

HERE = Path(__file__).resolve().parent
SOURCE = HERE.parents[4] / "docs/superpowers/specs/living-kingdom/concept-art/08-chapter-one-polished-ux.png"
FONT = "/usr/share/fonts/truetype/dejavu/DejaVuSans.ttf"
INK, BACKGROUND = "#171323", "#f4f0e9"
PANELS = {
    "title": (8, 0, 327, 310),
    "map": (329, 0, 631, 310),
    "party": (6, 354, 327, 655),
    "inventory": (329, 354, 634, 655),
    "arrival": (630, 0, 935, 310),
    "continuation": (10, 697, 324, 978),
    "choices": (1220, 0, 1536, 311),
    "journal": (933, 697, 1238, 978),
}
PAIRS = [
    ("title", "08-title", "Title: original wordmark and party", "Live Begin and Exit replace baked controls."),
    ("party", "10-party", "Party: direct original character crops", "Original portraits; readable names and skills."),
    ("inventory", "11-items", "Inventory: six slots on navy", "New campaign: empty slots; item art not shown."),
    ("arrival", "20-road-first", "Illustrated first page", "Original village crop; larger Atkinson body text."),
    ("continuation", "21-road-page2", "Continuation reading", "More room for text; illustration omitted on later pages."),
    ("choices", "29-road-choice", "Choice screen", "Larger captions and descriptions; different story choices."),
    ("journal", "13-journal", "Journal", "Actual campaign overview; different content and navigation."),
]


def label(draw, at, text, size=20):
    draw.text(at, text, fill=INK, font=ImageFont.truetype(FONT, size))


def comparison(rows, name):
    sheet = Image.new("RGB", (1040, 110 + len(rows) * 580), BACKGROUND)
    draw = ImageDraw.Draw(sheet)
    label(draw, (24, 18), "Selected board08 vs frozen emulator", 28)
    label(draw, (24, 62), "Original reference (enlarged for comparison)", 18)
    label(draw, (544, 62), "Actual emulator (native 480 x 480)", 18)
    for index, (source, native, title, note) in enumerate(rows):
        y = 110 + index * 580
        image = Image.open(HERE / "source-panels" / f"{source}.png").convert("RGB")
        image = ImageOps.contain(image, (480, 480), Image.Resampling.NEAREST)
        sheet.paste(image, (24 + (480 - image.width) // 2, y + (480 - image.height) // 2))
        sheet.paste(Image.open(HERE / "native" / f"{native}.png"), (544, y))
        label(draw, (24, y + 494), title, 21)
        label(draw, (24, y + 527), note, 18)
    sheet.save(HERE / name)


def reading_sizes():
    sheet = Image.new("RGB", (820, 1210), BACKGROUND)
    draw = ImageDraw.Draw(sheet)
    label(draw, (24, 18), "Final emulator reading: native and size approximation", 24)
    label(draw, (24, 59), "480 px diameter: exact screenshot", 18)
    label(draw, (540, 59), "201 px diameter", 18)
    for index, name in enumerate(["20-road-first", "21-road-page2"]):
        y = 100 + index * 550
        image = Image.open(HERE / "native" / f"{name}.png")
        sheet.paste(image, (24, y))
        small = image.resize((201, 201), Image.Resampling.LANCZOS)
        small.save(HERE / ("reading-first-201px.png" if index == 0 else "reading-continuation-201px.png"))
        sheet.paste(small, (540, y))
        label(draw, (24, y + 492), "First illustrated page" if index == 0 else "Continuation page", 20)
    label(draw, (540, 335), "201 px / 96 ppi = 2.09 in", 16)
    label(draw, (540, 361), "Browser zoom / monitor density", 15)
    label(draw, (540, 385), "change apparent physical size.", 15)
    label(draw, (540, 419), "Not a physical-device test.", 17)
    sheet.save(HERE / "reading-native480-and-201px.png")


def main():
    source = Image.open(SOURCE)
    for name, box in PANELS.items():
        source.crop(box).save(HERE / "source-panels" / f"{name}.png")
    comparison(PAIRS[:3], "board08-vs-final-overview.png")
    comparison(PAIRS[3:6], "board08-vs-final-reading.png")
    comparison(PAIRS[6:], "board08-vs-final-journal.png")
    comparison([("map", "74-ledger-after", "World map", "GPT label-removal edit; six live destination controls.")], "board08-vs-final-map.png")
    reading_sizes()
    files = [SOURCE, *sorted((HERE / "native").glob("*.png"))]
    hashes = {str(path.relative_to(HERE.parents[4])): hashlib.sha256(path.read_bytes()).hexdigest() for path in files}
    manifest = {"emulator_sha256": "6f40297c36fccf7d067c7f37aeeeacf2f29f0fa6416d5cca19832efcf33e87ce", "source_crops": PANELS, "file_sha256": hashes}
    (HERE / "manifest.json").write_text(json.dumps(manifest, indent=2) + "\n")


if __name__ == "__main__":
    main()
