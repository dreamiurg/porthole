#!/usr/bin/env python3
"""Compose round-screen screenshots into one transparent PNG for READMEs.

Every app ships two of these (see docs/new-app.md):

    python3 tools/gallery.py --cols 4 apps/<app>/docs/preview.png     shot1 shot2 shot3 shot4
    python3 tools/gallery.py --cols 3 apps/<app>/docs/screenshots.png shot1 ... shot6

Each input is a square capture of the 480x480 panel (or a web capture of it). Tiles are
masked to the round screen, 320 px each, 20 px apart, on a transparent background so
they read on GitHub's light and dark themes. --pixel keeps pixel art crisp: it samples
the capture down to --native px with nearest-neighbour, then scales up by an integer.
A shot written pixel:<path> gets that treatment alone, so one strip can mix pixel art
with full-resolution screens.
--inset trims a rim off web captures before masking.
"""

import argparse

from PIL import Image, ImageDraw

TILE, GAP = 320, 20


def tile(path: str, pixel: bool, native: int, inset: int) -> Image.Image:
    im = Image.open(path).convert("RGB")
    if inset:
        im = im.crop((inset, inset, im.width - inset, im.height - inset))
    if pixel:
        im = im.resize((native, native), Image.Resampling.NEAREST).resize((TILE, TILE), Image.Resampling.NEAREST)
    else:
        im = im.resize((TILE, TILE), Image.Resampling.LANCZOS)
    mask = Image.new("L", (TILE * 4, TILE * 4), 0)  # draw big, shrink: anti-aliased rim
    ImageDraw.Draw(mask).ellipse((0, 0, TILE * 4 - 1, TILE * 4 - 1), fill=255)
    out = Image.new("RGBA", (TILE, TILE), (0, 0, 0, 0))
    out.paste(im, (0, 0), mask.resize((TILE, TILE), Image.Resampling.LANCZOS))
    return out


def main() -> None:
    ap = argparse.ArgumentParser(description=__doc__, formatter_class=argparse.RawDescriptionHelpFormatter)
    ap.add_argument("out")
    ap.add_argument("shots", nargs="+")
    ap.add_argument("--cols", type=int, default=4)
    ap.add_argument("--pixel", action="store_true", help="nearest-neighbour scaling for pixel art")
    ap.add_argument("--native", type=int, default=160, help="logical resolution for --pixel (default 160)")
    ap.add_argument("--inset", type=int, default=0, help="px to trim from each edge before masking")
    a = ap.parse_args()
    tiles = [tile(p.removeprefix("pixel:"), a.pixel or p.startswith("pixel:"), a.native, a.inset) for p in a.shots]
    cols = min(a.cols, len(tiles))
    rows = (len(tiles) + cols - 1) // cols
    sheet = Image.new("RGBA", (cols * TILE + (cols - 1) * GAP, rows * TILE + (rows - 1) * GAP), (0, 0, 0, 0))
    for i, t in enumerate(tiles):
        sheet.paste(t, ((i % cols) * (TILE + GAP), (i // cols) * (TILE + GAP)), t)
    sheet.save(a.out, optimize=True)
    print(f"{a.out}: {sheet.width}x{sheet.height}, {len(tiles)} tiles")


if __name__ == "__main__":
    main()
