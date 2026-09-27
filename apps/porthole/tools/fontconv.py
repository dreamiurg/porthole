#!/usr/bin/env python3
"""Convert lv_font_conv 1.5.3 C output into a game's generated/fonts.h (os/font.h tables, no LVGL).

    python3 tools/fontconv.py [--game biscuit|tiltfc] fontN.c ...

Run from apps/porthole. Each game's font, its output and its inputs (GAMES below):

Biscuit (the default): the fonts of Biscuit's LVGL firmware (tag biscuit-v0.1.0).

    for n in 16 20 24 28; do git show biscuit-v0.1.0:apps/biscuit/firmware/generated/font$n.c > build/font$n.c; done
    python3 tools/fontconv.py build/font{16,20,24,28}.c

To add a glyph, first regenerate them from Montserrat-Regular.ttf (same tag, apps/biscuit/assets/fonts/) with the same
converter and flags (that tag's apps/biscuit/firmware/tools/export-fonts.mjs):

    npx lv_font_conv@1.5.3 --font Montserrat-Regular.ttf --range 0x20-0x7e,0xb7,0xe9,0xf6 --size N --bpp 4
        --format lvgl --no-compress --lv-include lvgl.h --lv-font-name biscuit_font_N --output fontN.c

Tilt FC: Rubik Mono One (games/tilt-fc/fonts/, from github.com/google/fonts ofl/rubikmonoone), only the glyphs it
draws (KEEP): every other ASCII glyph keeps its advance but has no bitmap.

    for n in 18 36 64; do npx lv_font_conv@1.5.3 --font games/tilt-fc/fonts/RubikMonoOne-Regular.ttf --range 0x20-0x7e
        --size $n --bpp 4 --format lvgl --no-compress --lv-include lvgl.h --lv-font-name tiltfc_font_$n
        --output build/fc$n.c; done
    python3 tools/fontconv.py --game tiltfc build/fc{18,36,64}.c

The converter's layout is checked, not assumed: 4 bpp plain bitmaps, class kerning at kern_scale 16 (or none, for a
monospace font), ASCII 32..126 as glyphs 1..95 and, if any, a sparse list of extra code points after it. Anything
else stops the conversion.
"""

import re
import sys
from pathlib import Path
from typing import NamedTuple


class Game(NamedTuple):
    out: str
    ns: str
    name: str
    credit: str
    license: str
    keep: str | None  # the only glyphs with bitmaps, or None for all


GAMES = {
    "biscuit": Game(
        "games/biscuit/generated/fonts.h",
        "biscuit",
        "Montserrat Regular",
        "Copyright 2024 The Montserrat.Git Project Authors (https://github.com/JulietaUla/Montserrat.git).",
        "see OFL.txt next to this file",
        None,
    ),
    "tiltfc": Game(
        "games/tilt-fc/generated/fonts.h",
        "fc",
        "Rubik Mono One",
        "Copyright 2015 The Rubik Project Authors (mail@hubertfischer.com).",
        "see games/tilt-fc/fonts/OFL.txt",
        " !0123456789AGLO",  # GOAL!, the scores, the countdown and the shirt numbers
    ),
}


def array(src: str, name: str) -> list[int]:
    m = re.search(rf"\b{name}\[\]\s*=\s*\{{(.*?)\}};", src, re.S)
    if not m:
        sys.exit(f"fontconv: no {name}[] in the input")
    return [int(v, 0) for v in re.findall(r"-?(?:0x[0-9a-fA-F]+|\d+)", m[1])]


def field(src: str, name: str) -> int:
    m = re.search(rf"\.{name}\s*=\s*(-?\d+)", src)
    if not m:
        sys.exit(f"fontconv: no .{name} in the input")
    return int(m[1])


def expect(ok: bool, what: str) -> None:
    if not ok:
        sys.exit(f"fontconv: unexpected converter output: {what}")


def rows(vals: list[int], per: int = 24) -> str:
    return ",\n".join("  " + ", ".join(str(v) for v in vals[i : i + per]) for i in range(0, len(vals), per))


def kerning(src: str, n_glyphs: int) -> tuple[list[int], list[int], list[int], int]:
    """Class kerning tables, or all-zero classes (no pair kerns) for a font without kerning."""
    if field(src, "kern_classes") == 0:
        expect(re.search(r"\.kern_dsc\s*=\s*NULL", src) is not None and field(src, "kern_scale") == 0, "kern_classes 0 with kerning data")
        return [0] * n_glyphs, [0] * n_glyphs, [0], 1
    expect(field(src, "kern_classes") == 1 and field(src, "kern_scale") == 16, "kern_classes != 1 or kern_scale != 16")
    left, right, values = array(src, "kern_left_class_mapping"), array(src, "kern_right_class_mapping"), array(src, "kern_class_values")
    lcnt, rcnt = field(src, "left_class_cnt"), field(src, "right_class_cnt")
    expect(len(left) == len(right) == n_glyphs and len(values) == lcnt * rcnt, "kerning table sizes")
    expect(max(left) <= lcnt and max(right) <= rcnt and all(-128 <= v < 128 for v in values), "kerning classes or values out of range")
    return left, right, values, rcnt


def extra_cmap(src: str) -> list[int]:
    """The extra code points after ASCII (the second cmap), or none."""
    cmaps = re.findall(r"\.range_start = (\d+), \.range_length = (\d+), \.glyph_id_start = (\d+),.*?\.type = (\w+)", src, re.S)
    expect(len(cmaps) == field(src, "cmap_num") and len(cmaps) in (1, 2), "one or two cmaps")
    expect(cmaps[0] == ("32", "95", "1", "LV_FONT_FMT_TXT_CMAP_FORMAT0_TINY"), "first cmap is not ASCII 32..126 as glyphs 1..95")
    if len(cmaps) == 1:
        return []
    expect(cmaps[1][2] == "96" and cmaps[1][3] == "LV_FONT_FMT_TXT_CMAP_SPARSE_TINY", "second cmap is not a sparse list from glyph 96")
    return [int(cmaps[1][0]) + o for o in array(src, "unicode_list_1")]


def keep_only(glyphs: list[tuple], bitmap: list[int], keep: str | None) -> tuple[list[tuple], list[int]]:
    """Every glyph not in `keep` loses its bitmap (box 0 x 0, advance kept); the kept bitmaps are packed again."""
    if keep is None:
        return glyphs, bitmap
    starts = [int(g[0]) for g in glyphs] + [len(bitmap)]
    out: list[tuple] = [glyphs[0]]
    packed: list[int] = []
    for i, g in enumerate(glyphs[1:], 1):
        cp = 31 + i if i <= 95 else None
        if cp is not None and chr(cp) in keep:
            out.append((str(len(packed)), *g[1:]))
            packed += bitmap[starts[i] : starts[i + 1]]
        else:
            out.append(("0", g[1], "0", "0", "0", "0"))
    return out, packed


def convert(path: Path, game: Game) -> tuple[int, str, list[int], bool]:
    raw = path.read_text(encoding="utf-8")
    src = re.sub(r"/\*.*?\*/", "", raw, flags=re.S)
    m = re.search(r"Size: (\d+) ?px", raw)  # the converter's header comment, or export-fonts.mjs's version of it
    expect(m is not None, "no size in the header comment")
    size = int(m[1]) if m else 0
    for name, want in (("bpp", 4), ("bitmap_format", 0)):
        expect(field(src, name) == want, f"{name} != {want}")
    extras = extra_cmap(src)
    glyphs = re.findall(r"\{\.bitmap_index = (\d+), \.adv_w = (\d+), \.box_w = (\d+), \.box_h = (\d+), \.ofs_x = (-?\d+), \.ofs_y = (-?\d+)\}", src)
    expect(len(glyphs) == 96 + len(extras), f"{len(glyphs)} glyph descriptions for {95 + len(extras)} glyphs")
    left, right, values, rcnt = kerning(src, len(glyphs))
    glyphs, bitmap = keep_only(glyphs, array(src, "glyph_bitmap"), game.keep)
    n = f"FONT{size}"
    glyph_rows = ",\n".join("  {" + ", ".join(g) + "}" for g in glyphs)
    extras_ref = "EXTRAS" if extras else "nullptr"
    body = f"""
// {game.name} {size} px: line height {field(src, "line_height")}, baseline {field(src, "base_line")} px from the bottom.
inline constexpr uint8_t {n}_BITMAP[] = {{
{rows([int(v) for v in bitmap], 32)}
}};
inline constexpr font::Glyph {n}_GLYPHS[] = {{
{glyph_rows}
}};
inline constexpr uint8_t {n}_KERN_LEFT[] = {{
{rows(left)}
}};
inline constexpr uint8_t {n}_KERN_RIGHT[] = {{
{rows(right)}
}};
inline constexpr int8_t {n}_KERN[] = {{
{rows(values)}
}};
inline constexpr font::Font {n} = {{{n}_BITMAP, {n}_GLYPHS, {extras_ref}, {len(extras)}, {n}_KERN_LEFT, {n}_KERN_RIGHT, {n}_KERN, {rcnt}, \
{field(src, "line_height")}, {field(src, "base_line")}}};
"""
    return size, body, extras, rcnt > 1 or any(values)


def head(game: Game, fonts: list, extras: list[int]) -> str:
    sizes = "/".join(str(f[0]) for f in fonts)
    kerns = "class kerning" if any(f[3] for f in fonts) else "no kerning"
    covers = " ".join(f"U+{c:04X}" for c in extras)
    what = f"ASCII 32..126 plus {covers}" if extras else "ASCII 32..126"
    lines = [
        "// AUTO-GENERATED by tools/fontconv.py from lv_font_conv 1.5.3 output. Do not edit.",
        f"// {game.name} {sizes} px, 4 bpp, {kerns}: {what}.",
    ]
    if game.keep is not None:
        lines.append(f"// Only these glyphs have bitmaps (GLYPHS); every other one draws nothing: {game.keep.strip()}")
    lines += [f"// {game.credit}", f"// SIL Open Font License 1.1: {game.license}.", "#pragma once", "#include <stdint.h>", '#include "font.h"', ""]
    lines.append(f"namespace {game.ns} {{")
    if extras:
        lines.append(f"inline constexpr uint16_t EXTRAS[] = {{{', '.join(f'0x{c:04X}' for c in extras)}}};")
    if game.keep is not None:
        lines.append(f'inline constexpr char GLYPHS[] = "{game.keep}";')
    return "\n".join(lines) + "\n"


def main() -> None:
    args = sys.argv[1:]
    name = "biscuit"
    if args[:1] == ["--game"]:
        if len(args) < 2:
            sys.exit(__doc__)
        name, args = args[1], args[2:]
    if not args or name not in GAMES:
        sys.exit(__doc__)
    game = GAMES[name]
    fonts = sorted(convert(Path(p), game) for p in args)
    extras = fonts[0][2]
    if any(f[2] != extras for f in fonts):
        sys.exit("fontconv: the fonts cover different code points")
    out = Path(game.out)
    out.parent.mkdir(parents=True, exist_ok=True)
    out.write_text(head(game, fonts, extras) + "".join(f[1] for f in fonts) + f"}}  // namespace {game.ns}\n", encoding="utf-8")
    print(f"wrote {out}: {', '.join(f'{f[0]} px' for f in fonts)}")


if __name__ == "__main__":
    main()
