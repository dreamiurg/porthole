#!/usr/bin/env python3
"""Encode direct board08 crops and GPT reference edits as lossless RGB565.

Source PNGs are never painted or rewritten. --extract-source repeats exact crops;
--recompress rebuilds cached Zopfli streams. Normal --check needs only Pillow/zlib.
"""

import argparse
import hashlib
import json
import re
import sys
import zlib
from array import array
from concurrent.futures import ProcessPoolExecutor
from functools import cache
from pathlib import Path

from PIL import Image, ImageDraw, ImageOps

ROOT = Path(__file__).resolve().parents[1]
SOURCE = ROOT.parents[3] / "docs/superpowers/specs/living-kingdom/concept-art/08-chapter-one-polished-ux.png"
BOARD = ROOT / "art/board08"
ENCODED = ROOT / "art/encoded"
NAVY = "#0d0d1e"
PARCHMENT = "#f8d7a8"
LOCATIONS = ["village", "bridge", "mill", "woods", "marsh", "tower"]
ITEM_NAMES = ["bread", "rope", "wood", "thyme", "flour", "map"]
STATES = [0, 1, 2, 3, 4, 128, 129, 130, 131, 132]
SKIN_NAMES = ["paper", "gold", "violet", "dark"]


def digest(data):
    return hashlib.sha256(data).hexdigest()


def source_path(name):
    return SOURCE if name == "source.png" else BOARD / name


@cache  # several crops share one atlas: decode each once
def decoded(path):
    return Image.open(path).convert("RGB")


def validate_sources(extract=False):
    manifest = json.loads((BOARD / "manifest.json").read_text())
    for name, expected in manifest["sources"].items():
        if digest(source_path(name).read_bytes()) != expected:
            raise SystemExit(f"Source image hash changed: {name}")
    for name, entry in manifest["assets"].items():
        cropped = decoded(source_path(entry["source"])).crop(entry["crop"])
        path = BOARD / f"{name}.png"
        if extract and path != source_path(entry["source"]):
            cropped.save(path)
        current = Image.open(path).convert("RGB")
        if cropped.size != current.size or cropped.tobytes() != current.tobytes():
            raise SystemExit(f"Artwork is not its documented exact source crop: {name}")
        if not extract and digest(path.read_bytes()) != entry["sha256"]:
            raise SystemExit(f"Crop file hash changed: {name}")
        entry["sha256"] = digest(path.read_bytes())
    if extract:
        (BOARD / "manifest.json").write_text(json.dumps(manifest, indent=2) + "\n")


def fitted(image, size):
    return ImageOps.fit(image, size, Image.Resampling.LANCZOS)


def contained(image, size):
    ratio = min(size[0] / image.width, size[1] / image.height)
    resized = image.resize((round(image.width * ratio), round(image.height * ratio)), Image.Resampling.LANCZOS)
    output = Image.new("RGB", size, NAVY)
    output.paste(resized, ((size[0] - resized.width) // 2, size[1] - resized.height))
    return output


@cache
def artwork(name):
    image = Image.open(BOARD / f"{name}.png").convert("RGB")
    if name in ("princess", "lucas", "dribble"):
        return contained(image, (80, 112))
    if name.startswith("item-"):
        return contained(image, (48, 48))
    if name.startswith("skin-"):
        return image.resize((48, 48), Image.Resampling.LANCZOS)
    if name == "world-map":
        return fitted(image, (240, 240))
    if name == "title":
        return fitted(image, (240, 175))
    return fitted(image, (240, 120))


def scene_name(name, changes):
    outcome = changes & 15
    if name == "bridge":
        if changes & 128:
            return "bridge-repaired"
        return "bridge-state4" if outcome == 4 else "bridge"
    if name in ("mill", "marsh") and outcome:
        return f"{name}-state{outcome}"
    return name


def all_assets():
    assets = {name: artwork(name) for name in ["title", "world-map", "village", "woods", "tower", "sunset", "princess", "lucas", "dribble"]}
    for name in LOCATIONS[1:]:
        for state in STATES:
            key = scene_name(name, state)
            assets[key] = artwork(key)
    assets.update({"item-" + name: artwork("item-" + name) for name in ITEM_NAMES})
    return assets


def image_pixels(image):
    return [image.getpixel((x, y)) for y in range(image.height) for x in range(image.width)]


def rgb565(rgb):
    r, g, b = rgb
    return ((r >> 3) << 11) | ((g >> 2) << 5) | (b >> 3)


def native_bytes(image):
    rgb = image.convert("RGB").tobytes()
    words = array("H", (((rgb[i] >> 3) << 11) | ((rgb[i + 1] >> 2) << 5) | (rgb[i + 2] >> 3) for i in range(0, len(rgb), 3)))
    if sys.byteorder != "little":
        words.byteswap()
    return words.tobytes()


def compress_one(pair):
    import zopfli
    import zopfli.zlib

    if zopfli.__version__ != "0.4.3":
        raise SystemExit("Explicit recompression requires zopfli==0.4.3")
    name, image = pair
    raw = native_bytes(image)
    data = zopfli.zlib.compress(raw, numiterations=15)
    if zlib.decompress(data) != raw:
        raise SystemExit(f"Compression verification failed: {name}")
    (ENCODED / f"{name}.zlib").write_bytes(data)
    return name, {"size": list(image.size), "rgb565_sha256": digest(raw), "stream_sha256": digest(data), "bytes": len(data)}


def recompress(assets):
    ENCODED.mkdir(parents=True, exist_ok=True)
    with ProcessPoolExecutor(max_workers=4) as pool:
        records = dict(pool.map(compress_one, sorted(assets.items())))
    manifest = {"format": "zlib-wrapped DEFLATE of little-endian RGB565", "compressor": "zopfli==0.4.3, numiterations=15", "assets": records}
    (ENCODED / "manifest.json").write_text(json.dumps(manifest, indent=2) + "\n")


def packed_assets(assets):
    manifest = json.loads((ENCODED / "manifest.json").read_text())["assets"]
    if set(manifest) != set(assets):
        raise SystemExit("Compressed asset inventory changed; run --recompress")
    packed = {}
    for name, image in assets.items():
        raw = native_bytes(image)
        data = (ENCODED / f"{name}.zlib").read_bytes()
        record = manifest[name]
        if record["rgb565_sha256"] != digest(raw) or record["stream_sha256"] != digest(data):
            raise SystemExit(f"Compressed source/hash changed: {name}; run --recompress")
        if record["size"] != list(image.size) or record["bytes"] != len(data):
            raise SystemExit(f"Compressed dimensions/length changed: {name}")
        decoder = zlib.decompressobj()
        decoded = decoder.decompress(data, len(raw) + 1)
        if decoded != raw or not decoder.eof or decoder.unused_data or decoder.unconsumed_tail:
            raise SystemExit(f"Compressed pixels differ from source RGB565: {name}")
        packed[name] = data
    return packed


def encode(name, image, data):
    lines = [f"static const uint8_t {name}_DATA[] = {{"]
    lines += ["  " + ",".join(map(str, data[i : i + 32])) + "," for i in range(0, len(data), 32)]
    lines += ["};", f"static const PackedImage {name} = {{{image.width}, {image.height}, {name}_DATA, {len(data)}}};\n"]
    return "\n".join(lines)


def art_output(assets, packed):
    lines = [
        "// Generated LOSSLESS board08 crops and GPT reference edits. Do not edit.",
        "// See art/{board08,encoded}/ and tools/kingdom_art.py.",
        "namespace kingdom { namespace art {",
    ]
    chunks, definitions = {}, {}
    for i, (name, image) in enumerate(assets.items()):
        symbol = f"IMAGE_{i}"
        filename = f"art_image_{i:02d}.inc"
        chunks[filename] = "// Generated lossless RGB565 source pixels. Do not edit.\n" + encode(symbol, image, packed[name])
        lines.append(f'#include "{filename}"')
        definitions[name] = "&" + symbol
    scenes = ["{" + ",".join(definitions[scene_name(name, state)] for state in STATES) + "}" for name in [*LOCATIONS, "world-map"]]
    faces = [definitions[name] for name in ["princess", "lucas", "dribble"]]
    items = [definitions["item-" + name] for name in ITEM_NAMES]
    lines += [
        "static const PackedImage* const LANDSCAPE = " + definitions["title"] + ";",
        "static const PackedImage* const CAMP = " + definitions["sunset"] + ";",
        "static const PackedImage* const SCENES[][10] = {" + ",".join(scenes) + "};",
        "static const PackedImage* const PORTRAITS[] = {" + ",".join(faces) + "};",
        "static const PackedImage* const ITEMS[] = {" + ",".join(items) + "};",
        "}}  // namespace kingdom::art\n",
    ]
    chunks["art_data.inc"] = "\n".join(lines)
    return chunks, sum(len(data) for data in packed.values())


def icon_output():
    source = (ROOT.parents[1] / "os/palette.h").read_text()
    values = re.search(r"PALETTE_RGB\[C_COUNT\] = \{(.*?)\};", source, re.S).group(1)
    palette = [int(value, 16) for value in re.findall(r"0x([0-9A-Fa-f]{6})", values)]
    colors = [((c >> 16) & 255, (c >> 8) & 255, c & 255) for c in palette]
    pixels = [
        min(range(32), key=lambda i: sum((p[j] - colors[i][j]) ** 2 for j in range(3))) for p in image_pixels(artwork("princess").resize((36, 36), Image.Resampling.LANCZOS))
    ]
    lines = ["// Generated from ORIGINAL board08 Princess crop. Do not edit.\nnamespace kingdom { namespace art {", "static const uint8_t ICON_PIXELS[] = {"]
    lines += ["  " + ",".join(map(str, pixels[i : i + 36])) + "," for i in range(0, len(pixels), 36)]
    return "\n".join(lines + ["};", "static const gfx::Sprite ICON = {36, 36, ICON_PIXELS};", "}}  // namespace kingdom::art\n"])


def skin_output():
    lines = ["// Generated direct GPT blank-panel pixels. Do not edit.", "namespace kingdom { namespace art {"]
    for i, name in enumerate(SKIN_NAMES):
        pixels = [rgb565(p) for p in image_pixels(artwork("skin-" + name))]
        lines.append(f"static const uint16_t SKIN_{i}[] = {{")
        lines += ["  " + ",".join(map(str, pixels[j : j + 24])) + "," for j in range(0, len(pixels), 24)]
        lines.append("};")
    lines += ["static const uint16_t* const SKINS[] = {SKIN_0,SKIN_1,SKIN_2,SKIN_3};", "}}"]
    return "\n".join(lines) + "\n"


def preview(path):
    sheet = Image.new("RGB", (1440, 1120), NAVY)
    draw = ImageDraw.Draw(sheet)
    names = ["title", "world-map", *LOCATIONS, "sunset"]
    for i, name in enumerate(names):
        x, y = (i % 3) * 480, (i // 3) * 300 + 24
        image = artwork(name)
        image = image.resize((480, min(270, image.height * 2)), Image.Resampling.NEAREST)
        sheet.paste(image, (x, y))
        draw.text((x + 8, y - 18), name, fill=PARCHMENT)
    for i, name in enumerate(["princess", "lucas", "dribble"]):
        sheet.paste(artwork(name), (50 + i * 120, 950))
    for i, name in enumerate(ITEM_NAMES):
        sheet.paste(artwork("item-" + name), (500 + i * 70, 950))
    sheet.save(path)


def main():
    parser = argparse.ArgumentParser(description=__doc__)
    parser.add_argument("--check", action="store_true")
    parser.add_argument("--extract-source", action="store_true")
    parser.add_argument("--recompress", action="store_true")
    parser.add_argument("--preview", type=Path)
    args = parser.parse_args()
    validate_sources(args.extract_source)
    assets = all_assets()
    if args.recompress:
        recompress(assets)
    outputs, size = art_output(assets, packed_assets(assets))
    outputs["icon.h"] = icon_output()
    outputs["art_skins.inc"] = skin_output()
    for name, text in outputs.items():
        if len(text.encode()) >= 500_000:
            raise SystemExit(f"Generated {name} exceeds the 500KB source-file limit")
        path = ROOT / "generated" / name
        if args.check:
            if not path.exists() or path.read_text() != text:
                raise SystemExit(f"Kingdom {name} differs; rerun kingdom_art.py")
        else:
            path.write_text(text)
    if args.preview:
        preview(args.preview)
    print(f"Kingdom art {'matches' if args.check else 'generated'}: {size:,} compressed art bytes plus 18,432 skin and 1,296 indexed icon bytes.")


if __name__ == "__main__":
    main()
