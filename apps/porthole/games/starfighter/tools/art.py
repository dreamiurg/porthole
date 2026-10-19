#!/usr/bin/env python3
"""Original Starfighter pixel clusters. Reproduce assets.h; --check never writes."""

import argparse
from pathlib import Path

ROOT = Path(__file__).resolve().parents[1]
COLORS = [
    0x050B18,
    0x101F35,
    0x213750,
    0x395675,
    0x6687A4,
    0x9BB4C9,
    0xCFDDE2,
    0xF2F4DB,
    0x0D3971,
    0x14599A,
    0x258ACB,
    0x62D6EF,
    0xA42E3C,
    0xEC4E45,
    0xFF9660,
    0xF9C54E,
    0xFFF1B0,
    0x275949,
    0x4BCF72,
    0xBAF8AD,
    0x694A4F,
    0xB77761,
    0xEDB38B,
    0x483E66,
]


class Pixels:
    def __init__(self, w, h):
        self.w, self.h = w, h
        self.px = [255] * (w * h)

    def pixel(self, x, y, c):
        if 0 <= x < self.w and 0 <= y < self.h:
            self.px[y * self.w + x] = c

    def rect(self, pos, size, c):
        x, y = pos
        w, h = size
        for yy in range(y, y + h):
            for xx in range(x, x + w):
                self.pixel(xx, yy, c)

    def poly(self, points, c):
        for y in range(self.h):
            cuts = []
            for a, b in zip(points, points[1:] + points[:1], strict=True):
                if min(a[1], b[1]) <= y < max(a[1], b[1]):
                    cuts.append(a[0] + (y - a[1]) * (b[0] - a[0]) / (b[1] - a[1]))
            cuts.sort()
            for left, right in zip(cuts[::2], cuts[1::2], strict=True):
                for x in range(round(left), round(right) + 1):
                    self.pixel(x, y, c)

    def disc(self, x, y, r, c):
        for yy in range(y - r, y + r + 1):
            for xx in range(x - r, x + r + 1):
                if (xx - x) ** 2 + (yy - y) ** 2 <= r * r:
                    self.pixel(xx, yy, c)


def fighter():
    p = Pixels(61, 57)
    # Split strike foils: each engine has its own wing and laser cannon.
    for side in (-1, 1):

        def wing(coords, color, side=side):
            p.poly([(30 + side * x, y) for x, y in coords], color)

        wing([(3, 24), (25, 16), (27, 18), (25, 33), (6, 38)], 1)
        wing([(5, 25), (23, 19), (23, 26), (6, 32)], 6)
        wing([(5, 33), (24, 30), (27, 33), (25, 41), (5, 39)], 3)
        wing([(7, 34), (23, 33), (23, 37), (6, 37)], 5)
        wing([(8, 24), (21, 20), (20, 23), (8, 28)], 7)
        wing([(12, 25), (15, 24), (15, 28), (12, 29)], 12)
        wing([(12, 34), (16, 34), (16, 37), (12, 37)], 13)
        x = 30 + side * 26
        p.rect((x - 1, 11), (3, 26), 1)
        p.rect((x, 11), (1, 22), 6)
        p.rect((x - 1, 16), (3, 2), 7)
        p.rect((x - 1, 33), (3, 5), 4)
        # Four independent engine nacelles, not two generic exhaust blocks.
        for xx, yy in ((30 + side * 14, 24), (30 + side * 20, 34)):
            p.rect((xx - 4, yy - 3), (9, 14), 1)
            p.rect((xx - 3, yy - 4), (7, 13), 4)
            p.rect((xx - 2, yy - 4), (3, 11), 6)
            p.rect((xx + 2, yy - 2), (1, 9), 3)
            p.rect((xx - 3, yy + 5), (7, 2), 2)
            p.rect((xx - 3, yy + 9), (7, 4), 12)
            p.rect((xx - 2, yy + 10), (5, 6), 13)
            p.rect((xx - 1, yy + 10), (3, 4), 16)
    p.poly([(30, 0), (33, 5), (35, 19), (38, 30), (37, 43), (34, 48), (26, 48), (23, 43), (22, 30), (25, 19), (27, 5)], 1)
    p.poly([(30, 2), (32, 8), (33, 21), (36, 31), (35, 43), (33, 45), (27, 45), (25, 42), (25, 30), (27, 20), (28, 8)], 6)
    p.poly([(30, 3), (31, 10), (31, 23), (28, 25), (28, 10)], 7)
    p.rect((29, 13), (3, 10), 12)
    p.poly([(28, 23), (32, 23), (34, 30), (32, 35), (27, 35), (26, 30)], 2)
    p.poly([(28, 24), (31, 24), (32, 29), (28, 31), (27, 29)], 11)
    p.rect((28, 24), (2, 3), 7)
    p.rect((29, 34), (3, 8), 4)
    p.rect((26, 39), (2, 5), 3)
    p.rect((33, 39), (2, 5), 3)
    p.rect((29, 43), (3, 4), 7)
    return p


def banked(source, direction):
    p = Pixels(source.w, source.h)
    for y in range(source.h):
        for x in range(source.w):
            yy = y + round((x - 30) * direction * 0.22)
            xx = 30 + round((x - 30) * 0.89) + round((28 - y) * direction * 0.10)
            c = source.px[y * source.w + x]
            if c != 255:
                p.pixel(xx, yy, c)
    return p


def enemy(heavy=False):
    p = Pixels(33 if heavy else 25, 27 if heavy else 23)
    cx, cy = p.w // 2, p.h // 2
    for s in (-1, 1):
        x = cx + s * (12 if heavy else 9)
        p.poly([(x - 3, 1), (x + 2, 0), (x + 4, 5), (x + 4, p.h - 5), (x + 1, p.h - 1), (x - 3, p.h - 2), (x - 4, p.h - 6), (x - 4, 5)], 1)
        p.rect((x - 2, 3), (4, p.h - 6), 4 if heavy else 3)
        p.rect((x - 1, 4), (1, p.h - 8), 2)
        p.rect((x + 1, 4), (1, p.h - 8), 5)
        for y in (5, cy, p.h - 6):
            p.rect((x - 2, y), (4, 1), 6 if heavy else 4)
        p.rect((min(cx, x), cy - 1), (abs(cx - x), 3), 5)
        p.rect((min(cx, x), cy + 2), (abs(cx - x), 1), 1)
    p.disc(cx, cy, 6 if heavy else 5, 1)
    p.disc(cx, cy, 4, 5)
    p.disc(cx, cy, 3, 2)
    p.rect((cx - 2, cy - 1), (5, 3), 12)
    p.rect((cx - 1, cy - 1), (2, 2), 13)
    p.rect((cx + 1, cy - 3), (1, 2), 7)
    if heavy:
        p.rect((cx - 4, cy + 4), (2, 4), 15)
        p.rect((cx + 3, cy + 4), (2, 4), 15)
        p.rect((cx - 4, cy - 6), (9, 2), 4)
    return p


def explosion(frame):
    p = Pixels(35, 35)
    clusters = [(-8, -6, 4), (6, -8, 5), (-10, 5, 4), (7, 6, 5), (0, 0, 7), (1, 11, 3), (11, -1, 3)]
    for x, y, r in clusters:
        mul = (3 + frame) / 5
        xx, yy = 17 + round(x * mul), 17 + round(y * mul)
        p.disc(xx, yy, max(1, r - frame // 2), 12 if frame > 2 else 13)
        p.disc(xx, yy, max(1, r - 2 - frame // 2), 14)
    if frame < 3:
        p.disc(17, 17, 8 - frame, 15)
        p.disc(16, 16, 5 - frame, 16)
    for x, y in ((3, 3), (29, 6), (5, 28), (28, 29)):
        p.rect((x, y), (2, 2), 15 if frame < 3 else 13)
    return p


def pilot():
    p = Pixels(29, 31)
    p.poly([(6, 30), (5, 23), (9, 19), (20, 19), (24, 24), (23, 30)], 1)
    p.rect((6, 24), (17, 7), 13)
    p.rect((10, 24), (3, 7), 6)
    p.rect((17, 24), (3, 7), 6)
    p.rect((12, 25), (6, 5), 2)
    p.rect((14, 26), (2, 2), 11)
    p.disc(14, 12, 11, 1)
    p.disc(14, 11, 10, 6)
    p.rect((12, 2), (5, 8), 13)
    p.rect((13, 2), (2, 5), 7)
    p.rect((6, 11), (17, 8), 20)
    p.rect((8, 12), (13, 8), 22)
    p.rect((9, 10), (11, 5), 2)
    p.rect((10, 10), (9, 2), 15)
    p.rect((8, 11), (2, 3), 14)
    p.rect((14, 17), (3, 1), 20)
    p.rect((12, 20), (6, 2), 21)
    p.rect((5, 13), (3, 6), 4)
    p.rect((21, 13), (3, 6), 4)
    p.rect((22, 18), (1, 4), 2)
    p.rect((18, 21), (5, 1), 2)
    return p


def assets():
    ship = fighter()
    return {
        "FIGHTER": ship,
        "BANK_LEFT": banked(ship, -1),
        "BANK_RIGHT": banked(ship, 1),
        "ENEMY": enemy(),
        "HEAVY": enemy(True),
        "PILOT": pilot(),
        **{f"BURST_{i}": explosion(i) for i in range(5)},
    }


def header(sprites):
    lines = [
        "// AUTO-GENERATED by tools/art.py. Edit the generator, never this file.",
        "#pragma once",
        "#include <stdint.h>",
        "namespace starfighter::art {",
        "struct Sprite { uint8_t w, h; const uint8_t* px; };",
        "inline constexpr uint32_t COLORS[] = {" + ",".join(f"0x{c:06X}" for c in COLORS) + "};",
    ]
    for name, p in sprites.items():
        lines.append(f"inline constexpr uint8_t {name}_PX[] = {{")
        for y in range(p.h):
            lines.append("  " + ",".join(str(c) for c in p.px[y * p.w : (y + 1) * p.w]) + ",")
        lines.extend(["};", f"inline constexpr Sprite {name}{{{p.w},{p.h},{name}_PX}};"])
    lines.append("}  // namespace starfighter::art\n")
    return "\n".join(lines)


def preview(sprites):
    try:
        from PIL import Image
    except ImportError:
        print("Pillow missing; skipping preview.")
        return
    sheet = Image.new("RGB", (320, 180), "#050b18")
    for i, p in enumerate(sprites.values()):
        im = Image.new("RGBA", (p.w, p.h))
        im.putdata([((COLORS[c] >> 16) & 255, (COLORS[c] >> 8) & 255, COLORS[c] & 255, 255) if c != 255 else (0, 0, 0, 0) for c in p.px])
        sheet.paste(im, ((i % 4) * 80 + 8, (i // 4) * 60 + 2), im)
    out = ROOT.parents[1] / "build/art/starfighter-sheet.png"
    out.parent.mkdir(parents=True, exist_ok=True)
    sheet.resize((960, 540), Image.Resampling.NEAREST).save(out)


def main():
    parser = argparse.ArgumentParser(description=__doc__)
    parser.add_argument("--check", action="store_true")
    args = parser.parse_args()
    sprites = assets()
    out = ROOT / "assets.h"
    data = header(sprites)
    if args.check:
        if not out.exists() or out.read_text() != data:
            raise SystemExit("Starfighter assets stale: run games/starfighter/tools/art.py")
        print("Starfighter art matches generator.")
        return
    out.write_text(data)
    preview(sprites)
    print(f"Wrote {out}")


if __name__ == "__main__":
    main()
