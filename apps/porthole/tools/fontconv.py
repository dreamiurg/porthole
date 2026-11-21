#!/usr/bin/env python3
"""Convert lv_font_conv 1.5.3 C output into a game's generated/fonts.h (os/font.h tables, no LVGL).

    python3 tools/fontconv.py [--game biscuit|tiltfc|radar|adult|cards|cardsbig|uk] fontN.c ...

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

Adult system menu: Atkinson Hyperlegible Regular (shell/fonts/). Regenerate with:

    for n in 24 28 32; do npx lv_font_conv@1.5.3 --font shell/fonts/AtkinsonHyperlegible-Regular.ttf --range 0x20-0x7e
        --size $n --bpp 4 --format lvgl --no-compress --force-fast-kern-format --lv-include lvgl.h --lv-font-name adult_font_$n
        --output build/adult$n.c; done
    python3 tools/fontconv.py --game adult build/adult{24,28,32}.c

Times Cards: Fredoka SemiBold (games/times-cards/art/fonts/, the wght=600 wdth=100 instance of github.com/google/fonts
ofl/fredoka's variable font, cut with fontTools.varLib.instancer). Text at 28 and 40 px; the cards' numbers at 110 px,
only the glyphs a card draws (KEEP).

    for n in 28 40 110; do npx lv_font_conv@1.5.3 --font games/times-cards/art/fonts/Fredoka-SemiBold.ttf
        --range 0x20-0x7e,0xd7,0xf7 --size $n --bpp 4 --format lvgl --no-compress --force-fast-kern-format
        --lv-include lvgl.h --lv-font-name cards_font_$n --output build/cards$n.c; done
    python3 tools/fontconv.py --game cards build/cards{28,40}.c
    python3 tools/fontconv.py --game cardsbig build/cards110.c

Plane Radar: B612 Mono Bold (games/plane-radar/fonts/, from github.com/google/fonts ofl/b612mono), all of ASCII.

    for n in 18 24 32; do npx lv_font_conv@1.5.3 --font games/plane-radar/fonts/B612Mono-Bold.ttf --range 0x20-0x7e
        --size $n --bpp 4 --format lvgl --no-compress --lv-include lvgl.h --lv-font-name radar_font_$n
        --output build/radar$n.c; done
    python3 tools/fontconv.py --game radar build/radar{18,24,32}.c

Ukrainian text in any game (os/fonts/README.md): Andika Bold, ASCII plus the Ukrainian alphabet and its apostrophes.

    R=0x20-0x7e,0x2bc,0x404,0x406-0x407,0x410-0x429,0x42c,0x42e-0x449,0x44c,0x44e-0x44f,0x454,0x456-0x457,0x490-0x491,0x2019
    for n in 24 48 88; do npx lv_font_conv@1.5.3 --font os/fonts/Andika-Bold.ttf --range $R --size $n --bpp 4
        --format lvgl --no-compress --force-fast-kern-format --lv-include lvgl.h --lv-font-name uk_font_$n
        --output build/uk$n.c; done
    python3 tools/fontconv.py --game uk build/uk{24,48,88}.c

The converter's layout is checked, not assumed: 4 bpp plain bitmaps, class kerning at kern_scale 16 (or none, for a
monospace font), ASCII 32..126 as glyphs 1..95 and, if any, runs or sparse lists of extra code points after it, in order. Anything
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
    "adult": Game(
        "shell/adult_fonts.h",
        "adult",
        "Atkinson Hyperlegible Regular",
        "Copyright 2020 Braille Institute of America, Inc.",
        "see shell/fonts/OFL.txt",
        None,
    ),
    "cards": Game(
        "games/times-cards/generated/fonts.h",
        "cards",
        "Fredoka SemiBold",
        "Copyright 2016 The Fredoka Project Authors (https://github.com/hafontia/Fredoka-One).",
        "see games/times-cards/art/fonts/OFL.txt",
        None,
    ),
    "cardsbig": Game(
        "games/times-cards/generated/big.h",
        "cardsbig",
        "Fredoka SemiBold",
        "Copyright 2016 The Fredoka Project Authors (https://github.com/hafontia/Fredoka-One).",
        "see games/times-cards/art/fonts/OFL.txt",
        " 0123456789=?\u00d7\u00f7",  # a card's numbers, its signs and its answer
    ),
    "radar": Game(
        "games/plane-radar/generated/fonts.h",
        "radar",
        "B612 Mono Bold",
        "Copyright 2012 The B612 Project Authors (https://github.com/polarsys/b612).",
        "see games/plane-radar/fonts/OFL.txt",
        None,  # callsigns and plane types arrive over the air: every ASCII glyph
    ),
    "uk": Game(
        "os/uk_fonts.h",
        "ukfont",
        "Andika Bold",
        "Copyright (c) 2004-2022 SIL International (http://www.sil.org/).",
        "see os/fonts/OFL.txt",
        None,
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
    """The extra code points after ASCII (every cmap after the first, in glyph order), or none."""
    cmaps = re.findall(r"\.range_start = (\d+), \.range_length = (\d+), \.glyph_id_start = (\d+),.*?\.type = (\w+)", src, re.S)
    expect(len(cmaps) == field(src, "cmap_num") and len(cmaps) >= 1, "at least one cmap")
    expect(cmaps[0] == ("32", "95", "1", "LV_FONT_FMT_TXT_CMAP_FORMAT0_TINY"), "first cmap is not ASCII 32..126 as glyphs 1..95")
    extras: list[int] = []
    for i, (start, length, glyph, kind) in enumerate(cmaps[1:], 1):
        expect(int(glyph) == 96 + len(extras), f"cmap {i} does not start at glyph {96 + len(extras)}")
        if kind == "LV_FONT_FMT_TXT_CMAP_FORMAT0_TINY":  # a run of consecutive code points (one alone comes as this)
            extras += range(int(start), int(start) + int(length))
        else:
            expect(kind == "LV_FONT_FMT_TXT_CMAP_SPARSE_TINY", f"cmap {i} is neither a run nor a sparse list")
            extras += [int(start) + o for o in array(src, f"unicode_list_{i}")]
    expect(extras == sorted(set(extras)) and len(extras) <= 255, "extra code points not ascending, or more than 255")
    return extras


def keep_only(glyphs: list[tuple], bitmap: list[int], keep: str | None, extras: list[int]) -> tuple[list[tuple], list[int]]:
    """Every glyph not in `keep` loses its bitmap (box 0 x 0, advance kept); the kept bitmaps are packed again."""
    if keep is None:
        return glyphs, bitmap
    starts = [int(g[0]) for g in glyphs] + [len(bitmap)]
    out: list[tuple] = [glyphs[0]]
    packed: list[int] = []
    for i, g in enumerate(glyphs[1:], 1):
        cp = 31 + i if i <= 95 else extras[i - 96]
        if chr(cp) in keep:
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
    glyphs, bitmap = keep_only(glyphs, array(src, "glyph_bitmap"), game.keep, extras)
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
