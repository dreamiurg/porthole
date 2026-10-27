#!/usr/bin/env python3
"""Reproduce Toy Town's original Image Gen art and independently bundled fonts.

Pillow quantizes each 240px scene independently to 64 colors. Six-bit pixels
retain anime environmental detail without consuming an OTA slot. Dolls have
transparent index 63 and a separately packed 3-bit semantic material layer.
No runtime allocation, filesystem, decompressor or external service is needed.
Normal generation reads the source-verified conversion cache and is independent
of Pillow/FreeType changes. --convert-source rebuilds it using Pillow==12.3.0.
"""

import argparse
import colorsys
import hashlib
import json
import re
import zlib
from pathlib import Path

from PIL import Image, ImageDraw, ImageFont, __version__ as PILLOW_VERSION

ROOT = Path(__file__).resolve().parents[1]
PREVIEW = ROOT.parents[1] / "build/art/toy-town-life"
SOURCES = (
    "scene-atlas.png",
    "doll-atlas.png",
    "park.png",
    "scene-world-atlas.png",
    "scene-sprite-atlas.png",
    "scene-pose-atlas.png",
    "scene-objects-atlas.png",
    "scene-bag-atlas.png",
    "scene-pictures-atlas.png",
    "fonts/OpenSans-Semibold.ttf",
    "fonts/OpenSans-Bold.ttf",
)
CACHE = ROOT / "art/converted/assets.json.zlib"
CONVERSION_VERSION = "12.3.0"


def array(kind, name, values):
    lines = [f"static const {kind} {name}[] = {{"]
    lines += ["  " + ",".join(map(str, values[i : i + 32])) + "," for i in range(0, len(values), 32)]
    return "\n".join([*lines, "};"])


def pack(values, bits):
    result, accumulator, held = [], 0, 0
    for value in values:
        accumulator = (accumulator << bits) | value
        held += bits
        while held >= 8:
            held -= 8
            result.append((accumulator >> held) & 255)
    if held:
        result.append((accumulator << (8 - held)) & 255)
    return result


def rgb565(color):
    r, g, b = color
    return (r >> 3) << 11 | (g >> 2) << 5 | b >> 3


def quantized(image, count=64):
    result = image.quantize(colors=count, method=Image.Quantize.MEDIANCUT, dither=Image.Dither.NONE)
    raw = result.getpalette()
    palette = [tuple(raw[i * 3 : i * 3 + 3]) for i in range(count)]
    return list(result.tobytes()), palette


def scenes():
    atlas = Image.open(ROOT / "art/scene-atlas.png").convert("RGB")
    # White divider is at x/y 624..630 in the 1254px original.
    boxes = ((0, 0, 624, 624), (631, 0, 1254, 624), (0, 631, 624, 1254), (631, 631, 1254, 1254))
    images = [atlas.crop(box) for box in boxes]
    images.append(Image.open(ROOT / "art/park.png").convert("RGB"))
    world = Image.open(ROOT / "art/scene-world-atlas.png").convert("RGB")
    images[1:] = [world.crop(box) for box in boxes]
    return [image.resize((240, 240), Image.Resampling.LANCZOS) for image in images]


def clean_sprite_alpha(image):
    # Generated atlas cells can contain tiny detached slivers of a neighboring
    # sprite. Keep substantial connected alpha islands, never color-key clothing.
    width, height = image.size
    alpha = bytearray(image.getchannel("A").tobytes())
    remaining = {i for i, value in enumerate(alpha) if value >= 128}
    islands = []
    while remaining:
        seed = remaining.pop()
        island, pending = [seed], [seed]
        while pending:
            pixel = pending.pop()
            x, y = pixel % width, pixel // width
            neighbors = ((pixel - 1, x > 0), (pixel + 1, x + 1 < width), (pixel - width, y > 0), (pixel + width, y + 1 < height))
            for neighbor, valid in neighbors:
                if valid and neighbor in remaining:
                    remaining.remove(neighbor)
                    island.append(neighbor)
                    pending.append(neighbor)
        islands.append(island)
    minimum = max(map(len, islands)) // 50
    for island in islands:
        if len(island) < minimum:
            for pixel in island:
                alpha[pixel] = 0
    image.putalpha(Image.frombytes("L", image.size, bytes(alpha)))
    return image


def sprite_record(image, size):
    image = clean_sprite_alpha(image.convert("RGBA"))
    bounds = image.getchannel("A").point(lambda a: 255 if a >= 128 else 0).getbbox()
    image = image.crop(bounds)
    image.thumbnail(size, Image.Resampling.LANCZOS)
    pixels, palette = quantized(image.convert("RGB"), 63)
    alpha = image.getchannel("A").tobytes()
    pixels = [p if a >= 128 else 63 for p, a in zip(pixels, alpha, strict=True)]
    return {"pixels": pixels, "palette": [*palette, (0, 0, 0)], "w": image.width, "h": image.height}


def scene_sprites():
    atlas = Image.open(ROOT / "art/scene-sprite-atlas.png").convert("RGBA")
    rows = (0, 440, 715, 975, 1254)
    images = [atlas.crop((c * 1254 // 4, rows[r], (c + 1) * 1254 // 4, rows[r + 1])) for r in range(4) for c in range(4)]
    poses = Image.open(ROOT / "art/scene-pose-atlas.png").convert("RGBA")
    images += [poses.crop((c * poses.width // 4, 0, (c + 1) * poses.width // 4, poses.height)) for c in range(4)]
    objects = Image.open(ROOT / "art/scene-objects-atlas.png").convert("RGBA")
    images += [objects.crop((c * objects.width // 3, 0, (c + 1) * objects.width // 3, objects.height)) for c in range(3)]
    bag = Image.open(ROOT / "art/scene-bag-atlas.png").convert("RGBA")
    images += [bag.crop((c * bag.width // 2, 0, (c + 1) * bag.width // 2, bag.height)) for c in range(2)]
    pictures = Image.open(ROOT / "art/scene-pictures-atlas.png").convert("RGBA")
    images += [pictures.crop((c * pictures.width // 3, 0, (c + 1) * pictures.width // 3, pictures.height)) for c in range(3)]
    return [sprite_record(image, (112, 176) if i < 4 or 16 <= i < 20 else (112, 100)) for i, image in enumerate(images)]


def sprite_output(records):
    lines, size = [], 0
    for i, record in enumerate(records):
        pixels = pack(record["pixels"], 6)
        lines += [array("uint8_t", f"WORLD_PIX_{i}", pixels), array("uint16_t", f"WORLD_PAL_{i}", [rgb565(c) for c in record["palette"]])]
        size += len(pixels) + 128
    lines += ["struct WorldSprite { const uint8_t* pixels; const uint16_t* palette; int w; int h; };", "static const WorldSprite WORLD_SPRITES[] = {"]
    lines += [f"  {{WORLD_PIX_{i},WORLD_PAL_{i},{r['w']},{r['h']}}}," for i, r in enumerate(records)]
    return "\n".join([*lines, "};"]), size


def rgba_pixels(image):
    values = image.tobytes()
    channels = len(image.getbands())
    return zip(*(iter(values),) * channels, strict=True)


def key_mask(image):
    return Image.frombytes("L", image.size, bytes(0 if g > r + 55 and g > b + 45 else 255 for r, g, b in rgba_pixels(image)))


def dolls():
    atlas = Image.open(ROOT / "art/doll-atlas.png").convert("RGB")
    result = []
    for index in range(4):
        tile = atlas.crop((index * 512, 0, (index + 1) * 512, 731))
        mask = key_mask(tile)
        box = mask.getbbox()
        tile, mask = tile.crop(box), mask.crop(box)
        tile.putalpha(mask)
        tile.thumbnail((94, 156), Image.Resampling.LANCZOS)
        canvas = Image.new("RGBA", (96, 160))
        canvas.alpha_composite(tile, ((96 - tile.width) // 2, 158 - tile.height))
        result.append(canvas)
    return result


def garment(index, color, hsv):
    r, g, b, _ = color
    hue, saturation, value = hsv
    if index == 1:
        return 0.42 < hue < 0.59
    if index == 2:
        return (hue < 0.2 or 0.5 < hue < 0.7) and saturation > 0.35
    flesh = r > g > b and hue < 0.12 and value > 0.7
    return saturation > 0.18 and not flesh


def skin(index, y, color, hsv):
    r, g, b, _ = color
    hue, saturation, _ = hsv
    if index == 1:
        return r > b * 0.65 and b > g and saturation < 0.45
    threshold = 130 if index == 2 else 175
    warm = r - b < 105 or (index == 2 and r > 145)
    return r > g > b and hue < 0.12 and warm and y > 43 and r > threshold


def hair(index, y, color, hsv):
    r, g, b, _ = color
    matching = (index == 0 and r > g > b) or (index in (2, 3) and b > r)
    return y < 83 and hsv[1] > 0.23 and matching


def material(index, x, y, color):
    if color[3] < 128:
        return 0
    hsv = colorsys.rgb_to_hsv(*(value / 255 for value in color[:3]))
    if y > 139:
        return 4
    if 80 < y < 131 and garment(index, color, hsv):
        return 3
    if skin(index, y, color, hsv):
        return 2
    return 1 if hair(index, y, color, hsv) else 0


def doll_data(image, index):
    rgb = image.convert("RGB")
    pixels, palette = quantized(rgb, 63)
    alpha = list(image.getchannel("A").tobytes())
    pixels = [pixel if a >= 128 else 63 for pixel, a in zip(pixels, alpha, strict=True)]
    materials = [material(index, x, y, image.getpixel((x, y))) for y in range(160) for x in range(96)]
    palette.append((0, 0, 0))
    return pixels, palette, materials


def image_output(scene_images, doll_images):
    sections, size = [], 0
    for index, image in enumerate(scene_images):
        pixels, palette = image["pixels"], image["palette"]
        data = pack(pixels, 6)
        sections += [array("uint8_t", f"SCENE_{index}", data), array("uint16_t", f"SCENE_PAL_{index}", [rgb565(p) for p in palette])]
        size += len(data) + 128
    for index, image in enumerate(doll_images):
        pixels, palette, materials = image["pixels"], image["palette"], image["materials"]
        data, masks = pack(pixels, 6), pack(materials, 3)
        sections += [
            array("uint8_t", f"DOLL_{index}", data),
            array("uint16_t", f"DOLL_PAL_{index}", [rgb565(p) for p in palette]),
            array("uint8_t", f"MATERIAL_{index}", masks),
        ]
        size += len(data) + len(masks) + 128
    groups = (("SCENES", "SCENE", 5), ("SCENE_PALETTES", "SCENE_PAL", 5), ("DOLLS", "DOLL", 4), ("DOLL_PALETTES", "DOLL_PAL", 4), ("MATERIALS", "MATERIAL", 4))
    for name, prefix, count in groups:
        kind = "uint16_t" if "PALETTES" in name else "uint8_t"
        sections.append(f"static const {kind}* const {name}[] = {{" + ",".join(f"{prefix}_{i}" for i in range(count)) + "};")
    return "\n".join(sections), size


def font_data(name, weight, size, height, baseline):
    face = ImageFont.truetype(str(ROOT / f"art/fonts/OpenSans-{weight}.ttf"), size, layout_engine=ImageFont.Layout.BASIC)
    bitmap, glyphs = [], [(0, 0, 0, 0, 0, 0)]
    for code in range(32, 127):
        char = chr(code)
        mask, offset = face.getmask2(char, mode="L", anchor="ls")
        image = Image.frombytes("L", mask.size, bytes(mask))
        bounds = image.getbbox()
        if not bounds:
            glyphs.append((len(bitmap), round(face.getlength(char) * 16), 0, 0, 0, 0))
            continue
        image = image.crop(bounds)
        left, top = offset[0] + bounds[0], offset[1] + bounds[1]
        glyphs.append((len(bitmap), round(face.getlength(char) * 16), image.width, image.height, left, -top - image.height))
        bitmap.extend(pack([(v * 15 + 127) // 255 for v in image.tobytes()], 4))
    return {"name": name, "bitmap": bitmap, "glyphs": glyphs, "height": height, "baseline": baseline}


def font_output(record):
    name, bitmap, glyphs = record["name"], record["bitmap"], record["glyphs"]
    height, baseline = record["height"], record["baseline"]
    lines = [array("uint8_t", f"{name}_BITMAP", bitmap), f"static const font::Glyph {name}_GLYPHS[] = {{"]
    lines += ["  {" + ",".join(map(str, glyph)) + "}," for glyph in glyphs]
    lines += ["};", f"const font::Font {name} = {{{name}_BITMAP,{name}_GLYPHS,nullptr,0,ZERO_CLASSES,ZERO_CLASSES,ZERO_KERN,0,{height},{baseline}}};"]
    return "\n".join(lines), len(bitmap) + len(glyphs) * 12


def icon_pixels(doll):
    palette_source = (ROOT.parents[1] / "os/palette.h").read_text()
    block = palette_source.split("PALETTE_RGB[C_COUNT] = {")[1].split("};")[0]
    palette = [tuple(bytes.fromhex(value)) for value in re.findall(r"0x([0-9A-Fa-f]{6})", block)]
    image = doll.crop((6, 12, 90, 96)).resize((24, 24), Image.Resampling.LANCZOS)
    pixels = []
    for r, g, b, a in rgba_pixels(image):
        if a < 100:
            pixels.append(255)
        else:
            pixels.append(min(range(len(palette)), key=lambda i: sum((u - v) ** 2 for u, v in zip((r, g, b), palette[i], strict=True))))
    return pixels


def icon_output(pixels):
    return array("uint8_t", "ICON_PIXELS", pixels) + "\nstatic const gfx::Sprite ICON = {24,24,ICON_PIXELS};"


def source_hashes():
    sources = {name: hashlib.sha256((ROOT / "art" / name).read_bytes()).hexdigest() for name in SOURCES}
    # Only the launcher's indexed crop depends on the platform's palette.
    sources["../../os/palette.h"] = hashlib.sha256((ROOT.parents[1] / "os/palette.h").read_bytes()).hexdigest()
    return sources


def convert_source():
    if PILLOW_VERSION != CONVERSION_VERSION:
        raise SystemExit(f"Source conversion requires Pillow=={CONVERSION_VERSION}; normal generation/--check accepts any Pillow version.")
    images, converted_dolls, doll_images = [], [], dolls()
    for image in scenes():
        pixels, palette = quantized(image)
        images.append({"pixels": pixels, "palette": palette})
    for index, image in enumerate(doll_images):
        pixels, palette, materials = doll_data(image, index)
        converted_dolls.append({"pixels": pixels, "palette": palette, "materials": materials})
    result = {
        "sources": source_hashes(),
        "pillow": CONVERSION_VERSION,
        "scenes": images,
        "dolls": converted_dolls,
        "icon": icon_pixels(doll_images[0]),
        "world_sprites": scene_sprites(),
        "fonts": [font_data("BODY", "Semibold", 24, 28, 6), font_data("TITLE", "Bold", 30, 34, 7), font_data("ACTION", "Bold", 18, 21, 4)],
    }
    CACHE.parent.mkdir(parents=True, exist_ok=True)
    CACHE.write_bytes(zlib.compress(json.dumps(result, separators=(",", ":")).encode(), 9))


def converted():
    data = json.loads(zlib.decompress(CACHE.read_bytes()))
    if data["sources"] != source_hashes():
        raise SystemExit("Toy Town conversion cache sources changed; rebuild with --convert-source using Pillow==12.3.0.")
    return data


def preview(scene_images, doll_images, sprites):
    PREVIEW.mkdir(parents=True, exist_ok=True)
    sheet = Image.new("RGB", (1200, 720), "#fff7ee")
    for index, scene in enumerate(scene_images):
        pixels, palette = scene["pixels"], scene["palette"]
        image = Image.new("RGB", (240, 240))
        image.putdata([tuple(palette[pixel]) for pixel in pixels])
        sheet.paste(image, (index * 240, 0))
    for index, doll in enumerate(doll_images):
        pixels, palette = doll["pixels"], doll["palette"]
        image = Image.new("RGBA", (96, 160))
        image.putdata([(*palette[pixel], 255) if pixel != 63 else (0, 0, 0, 0) for pixel in pixels])
        sheet.paste(image.resize((192, 320), Image.Resampling.NEAREST), (index * 250 + 80, 280), image.resize((192, 320), Image.Resampling.NEAREST))
    draw = ImageDraw.Draw(sheet)
    face = ImageFont.truetype(str(ROOT / "art/fonts/OpenSans-Semibold.ttf"), 24)
    draw.text((40, 640), "Toy Town Life  |  Open Sans 24  |  Let's make a day together!", font=face, fill="#302937")
    sheet.save(PREVIEW / "assets.png")
    spritesheet = Image.new("RGBA", (8 * 130, 4 * 190), "#fff7ee")
    for index, sprite in enumerate(sprites):
        image = Image.new("RGBA", (sprite["w"], sprite["h"]))
        image.putdata([(*sprite["palette"][p], 255) if p != 63 else (0, 0, 0, 0) for p in sprite["pixels"]])
        spritesheet.alpha_composite(image, ((index % 8) * 130, (index // 8) * 190))
    spritesheet.save(PREVIEW / "world-sprites.png")


def split_output(output):
    # Split only between complete lines, including inside array initializers.
    # The preprocessor concatenates the parts into the exact original tokens.
    parts, lines, size = {}, [], 0
    for line in output.splitlines(keepends=True):
        if size + len(line) > 380000 and lines:
            parts[f"art_part_{len(parts)}.inc"] = "".join(lines)
            lines, size = [], 0
        lines.append(line)
        size += len(line)
    if lines:
        parts[f"art_part_{len(parts)}.inc"] = "".join(lines)
    return parts


def write_outputs(outputs, check):
    directory = ROOT / "generated"
    extras = {path.name for path in directory.glob("art_part_*.inc")} - outputs.keys()
    if extras and check:
        raise SystemExit("Toy Town has stale generated art parts; regenerate tools/toytown_art.py")
    for name in extras:
        (directory / name).unlink()
    for name, data in outputs.items():
        path = directory / name
        if check:
            if not path.exists() or path.read_text() != data:
                raise SystemExit(f"Toy Town {name} stale; run games/toy-town-life/tools/toytown_art.py")
        else:
            path.write_text(data)


def main():
    parser = argparse.ArgumentParser(description=__doc__)
    parser.add_argument("--check", action="store_true")
    parser.add_argument("--convert-source", action="store_true", help="rebuild conversion cache with pinned Pillow==12.3.0")
    args = parser.parse_args()
    if args.convert_source:
        convert_source()
    data = converted()
    scene_images, doll_images = data["scenes"], data["dolls"]
    images, size = image_output(scene_images, doll_images)
    body, body_size = font_output(data["fonts"][0])
    title, title_size = font_output(data["fonts"][1])
    sprites, sprite_size = sprite_output(data["world_sprites"])
    action, action_size = font_output(data["fonts"][2])
    header = "// Generated by tools/toytown_art.py; DO NOT EDIT. Sources and licenses: ../art/.\n"
    fonts = array("uint8_t", "ZERO_CLASSES", [0] * 96) + "\nstatic const int8_t ZERO_KERN[] = {0};\n" + body + "\n" + title + "\n" + action
    output = header + images + "\n" + sprites + "\n" + fonts + "\n" + icon_output(data["icon"]) + "\n"
    outputs = split_output(output)
    manifest = {
        "sources": data["sources"],
        "conversion_pillow": data["pillow"],
        "conversion_cache_sha256": hashlib.sha256(CACHE.read_bytes()).hexdigest(),
        "runtime_bytes": size + body_size + title_size + action_size + sprite_size + 674,
        "scene_resolution": [240, 240],
        "doll_resolution": [96, 160],
        "palette_colors": 64,
        "runtime_source_sha256": hashlib.sha256(output.encode()).hexdigest(),
        "parts": {name: hashlib.sha256(part.encode()).hexdigest() for name, part in outputs.items()},
    }
    outputs["art_data.inc"] = header + "".join(f'#include "{name}"\n' for name in outputs)
    outputs["manifest.json"] = json.dumps(manifest, indent=2) + "\n"
    write_outputs(outputs, args.check)
    if not args.check:
        preview(scene_images, doll_images, data["world_sprites"])
    print(f"Toy Town art {'matches' if args.check else 'generated'}: {manifest['runtime_bytes']:,} runtime bytes, original 5 scenes / 4 dolls / 28 world sprites / 3 fonts")


if __name__ == "__main__":
    main()
