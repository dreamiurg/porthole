#!/usr/bin/env bash
# Downloads the render assets into build/promo/assets (git-ignored): Waveshare's board STEP converted to GLB
# (colours kept), and CC0 textures and HDRIs from Poly Haven. Safe to re-run: skips what is already there.
set -euo pipefail
cd "$(dirname "$0")/../.."
A=build/promo/assets
mkdir -p "$A"

if [ ! -f "$A/board.glb" ]; then
  curl -sSL -o "$A/structure.zip" https://files.waveshare.com/wiki/ESP32-S3-Touch-LCD-2.1/ESP32-S3-Touch-LCD-2.1_structure.zip
  unzip -q -o -j "$A/structure.zip" "*.stp" -d "$A"
  uv run -q --with cadquery-ocp python tools/promo/step2glb.py "$A/ESP32-S3-Touch-LCD-2_1_asm.stp" "$A/board.glb"
fi

ph() {  # ph KIND ASSET: Poly Haven 2k files (textures: colour, roughness, GL normal; HDRIs: .hdr)
  curl -sSL "https://api.polyhaven.com/files/$2" | python3 -c "
import json, sys
d = json.load(sys.stdin)
maps = {'Diffuse': 'jpg', 'Rough': 'jpg', 'nor_gl': 'jpg'} if '$1' == 'tex' else {'hdri': 'hdr'}
for m, ext in maps.items():
    print(m, d[m]['2k'][ext]['url'])" | while read -r map url; do
    out="$A/$2_$map.${url##*.}"
    [ -f "$out" ] || curl -sSL -o "$out" "$url"
  done
}
for t in oak_veneer_01 dark_wood concrete_floor_02 concrete_floor_worn_001 marble_01 plywood white_plaster_02 \
  painted_plaster_wall blue_painted_planks red_plaster_weathered leather_white denim_fabric brown_leather wood_table_001 kitchen_wood; do ph tex "$t"; done
for h in studio_small_08 studio_small_03 fireplace empty_play_room; do ph hdri "$h"; done
ls -1 "$A"
