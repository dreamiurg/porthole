#!/usr/bin/env python3
"""Rally's raster art: Codex images in art/*.png -> pictures.h and skyline<N>.h (RGB565, CLEAR = transparent).

One header per skyline keeps every generated file under the repository's 500 KB large-file hook.

Every PNG came from Codex's built-in image generator with both approved reference boards attached
(docs/superpowers/specs/porthole-racing/assets/reference/: Style 2 panels 2A-2D and the four-track
showcase), asked to match that style exactly, on a flat #00FF00 (or #FF00FF) key. PROMPTS below keeps
each asset's description; the shared preamble is in docs/racing/art.md.

Each output cell takes the most common 4-bit-per-channel color bucket among its source pixels and
the integer mean of the pixels in that bucket; a cell that is mostly key is transparent. Integer math
only, so --check is byte-identical on every machine. Rivals are the player's car with the body hue
turned. Needs Pillow.
  python3 games/racing/tools/pictures.py           regenerate the headers
  python3 games/racing/tools/pictures.py --check   fail if any header is stale
"""

import colorsys
import sys
from collections import Counter
from pathlib import Path

from PIL import Image

GAME = Path(__file__).resolve().parents[1]
CLEAR = 0xF81F  # pure magenta never survives quantization of the art (the key is removed first)
GREEN, MAGENTA = "g", "m"
# (name, source, max w, max h, key): sizes in art pixels; the game draws them 2 panel px per art px at the player's depth.
SPRITES = (
    ("CAR0", "car.png", 34, 30, GREEN),
    ("BOOST", "boost.png", 20, 20, MAGENTA),
    ("ROCKET", "rocket.png", 20, 20, GREEN),
    ("MISSILE", "missile.png", 10, 14, GREEN),
    ("PUFF", "puff.png", 18, 14, GREEN),
    ("ROCK", "rock.png", 28, 22, GREEN),
    ("CRATES", "crates.png", 28, 24, GREEN),
    ("SNOWBLOCK", "snowblock.png", 30, 20, GREEN),
    ("BARRIER", "barrier.png", 30, 20, GREEN),
    ("CACTUS", "cactus.png", 22, 46, MAGENTA),
    ("SPIRE", "spire.png", 24, 56, GREEN),
    ("PYLON", "pylon.png", 26, 64, GREEN),
    ("DRUMS", "drums.png", 30, 26, GREEN),
    ("PINE", "pine.png", 30, 56, MAGENTA),
    ("CHEVRON", "chevron.png", 28, 32, GREEN),
    ("LAMP", "lamp.png", 22, 64, GREEN),
    ("NEON", "neon.png", 28, 46, GREEN),
    ("CLOUDS", "clouds.png", 150, 50, MAGENTA),
)
BACKDROPS = (("BG0", "bg_desert.png"), ("BG1", "bg_industrial.png"), ("BG2", "bg_snow.png"), ("BG3", "bg_night.png"))  # skyline<N>.h
BACKDROP_W = 320  # 640 panel px at 2x; the game mirrors it to tile
RIVAL_HUES = (0.60, 0.13, 0.33)  # blue, gold, green


def keyed(p, key):
    r, g, b = p
    return g > r + 60 and g > b + 60 if key == GREEN else r > g + 60 and b > g + 60 and abs(r - b) < 90


def rgb565(r, g, b):
    c = (r >> 3) << 11 | (g >> 2) << 5 | b >> 3
    return 0xF81E if c == CLEAR else c


def bbox(img, key):
    px = img.load()
    xs, ys = [], []
    for y in range(0, img.height, 2):
        for x in range(0, img.width, 2):
            if not keyed(px[x, y], key):
                xs.append(x)
                ys.append(y)
    return min(xs), min(ys), max(xs) + 2, max(ys) + 2


def cell(px, xs, ys, key):
    pixels = [px[x, y] for y in ys for x in xs]
    solid = [p for p in pixels if not keyed(p, key)]
    if len(solid) * 2 < len(pixels):
        return None
    bucket = Counter((r >> 4, g >> 4, b >> 4) for r, g, b in solid).most_common(1)[0][0]
    chosen = [p for p in solid if (p[0] >> 4, p[1] >> 4, p[2] >> 4) == bucket]
    n = len(chosen)
    return tuple(sum(p[i] for p in chosen) // n for i in range(3))


def grid(img, box, w, h, key):
    x0, y0, x1, y1 = box
    px = img.load()
    out = []
    for gy in range(h):
        ys = range(y0 + gy * (y1 - y0) // h, max(y0 + gy * (y1 - y0) // h + 1, y0 + (gy + 1) * (y1 - y0) // h))
        row = []
        for gx in range(w):
            xs = range(x0 + gx * (x1 - x0) // w, max(x0 + gx * (x1 - x0) // w + 1, x0 + (gx + 1) * (x1 - x0) // w))
            row.append(cell(px, xs, ys, key))
        out.append(row)
    return out


def fit(box, mw, mh):
    bw, bh = box[2] - box[0], box[3] - box[1]
    if bw * mh >= bh * mw:
        return mw, max(1, (bh * mw + bw // 2) // bw)
    return max(1, (bw * mh + bh // 2) // bh), mh


def sprite(source, mw, mh, key):
    img = Image.open(GAME / "art" / source).convert("RGB")
    box = bbox(img, key)
    w, h = fit(box, mw, mh)
    return grid(img, box, w, h, key)


def backdrop(source):
    img = Image.open(GAME / "art" / source).convert("RGB")
    h = (img.height * BACKDROP_W + img.width // 2) // img.width
    rows = grid(img, (0, 0, img.width, img.height), BACKDROP_W, h, GREEN)
    while rows and all(c is None for c in rows[0]):  # empty sky above the tallest peak
        rows.pop(0)
    return rows


def body(p):  # the car's red paint, not its tail lights, glass or tires
    r, g, b = (v / 255 for v in p)
    hue, light, sat = colorsys.rgb_to_hls(r, g, b)
    return (hue < 0.04 or hue > 0.94) and sat > 0.35 and 0.18 < light < 0.62


def recolor(rows, hue):
    def turn(p):
        if p is None or not body(p):
            return p
        _, light, sat = colorsys.rgb_to_hls(*(v / 255 for v in p))
        return tuple(round(v * 255) for v in colorsys.hls_to_rgb(hue, light, sat))

    return [[turn(p) for p in row] for row in rows]


def emit(name, rows):
    data = ",".join(f"0x{CLEAR if p is None else rgb565(*p):04X}" for row in rows for p in row)
    return [
        f"inline constexpr uint16_t {name}_PX[] = {{{data}}};",
        f"inline constexpr Pic {name}{{{len(rows[0])},{len(rows)},{name}_PX}};",
    ]


PREAMBLE = "// AUTO-GENERATED by tools/pictures.py from art/*.png. Edit the PNGs or the generator, never this file."


def skyline(i, source):
    return "\n".join(
        [
            PREAMBLE,
            "#pragma once",
            '#include "pic.h"',
            "namespace racing::pic {",
            *emit(f"BG{i}", backdrop(source)),
            "}  // namespace racing::pic",
            "",
        ]
    )


def header():
    lines = [PREAMBLE, "#pragma once", '#include "pic.h"']
    lines += [f'#include "skyline{i}.h"' for i in range(len(BACKDROPS))]
    lines += ["namespace racing::pic {"]
    car = None
    for name, source, mw, mh, key in SPRITES:
        rows = sprite(source, mw, mh, key)
        car = rows if name == "CAR0" else car
        lines += emit(name, rows)
    for i, hue in enumerate(RIVAL_HUES):
        lines += emit(f"CAR{i + 1}", recolor(car, hue))
    lines += [
        "inline constexpr const Pic* CAR[4] = {&CAR0, &CAR1, &CAR2, &CAR3};",
        "inline constexpr const Pic* PICKUP[2] = {&BOOST, &ROCKET};",
        "inline constexpr const Pic* OBSTACLE[4] = {&ROCK, &CRATES, &SNOWBLOCK, &BARRIER};",
        "inline constexpr const Pic* PROP[4][2] = {{&CACTUS, &SPIRE}, {&PYLON, &DRUMS}, {&PINE, &CHEVRON}, {&LAMP, &NEON}};",
        "inline constexpr const Pic* BACKDROP[4] = {&BG0, &BG1, &BG2, &BG3};",
        "}  // namespace racing::pic",
        "",
    ]
    return "\n".join(lines)


def outputs():
    yield GAME / "pictures.h", header()
    for i, (_, source) in enumerate(BACKDROPS):
        yield GAME / f"skyline{i}.h", skyline(i, source)


def main():
    check = "--check" in sys.argv[1:]
    for path, text in outputs():
        if not check:
            path.write_text(text, encoding="utf-8")
            print(f"wrote {path.relative_to(GAME.parents[1])}")
        elif not path.exists() or path.read_text(encoding="utf-8") != text:
            sys.exit(f"{path.name} out of date: run games/racing/tools/pictures.py")
    if check:
        print("racing pictures up to date")


if __name__ == "__main__":
    main()
