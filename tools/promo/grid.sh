#!/usr/bin/env bash
# Usage: STL=path/to/hardware/case/stl tools/promo/grid.sh   (edit tools/promo/grid.txt: id|table|#case|roughness|label)
# 16 table/case pairs in one home light, the same orbit frame, then a labelled 4x4 sheet.
set -euo pipefail
cd "$(dirname "$0")/../.."
S=${STL:?set STL to the puck case STL folder}
FR=apps/porthole/build/host/film/home
O=build/promo/out/grid; mkdir -p "$O"
while IFS='|' read -r id table case rough _; do
  [ -f "$O/$id.png" ] || blender -b -P tools/promo/scene.py -- --stl "$S" --frames "$FR" --look home --table "$table" --case "$case" --rough "$rough" \
    --shot orbit --frame 100 --res 720 --samples 64 --out "$O/$id.png" 2>&1 | grep -E "Error|Traceback" || true
done < tools/promo/grid.txt
python3 - <<'PY'
from PIL import Image, ImageDraw, ImageFont
rows = [l.rstrip("\n").split("|") for l in open("tools/promo/grid.txt") if l.strip()]
t, band = 440, 44
f = ImageFont.truetype("/System/Library/Fonts/Supplemental/Arial Rounded Bold.ttf", 19)
s = Image.new("RGB", (t * 4, (t + band) * 4), (24, 24, 26)); d = ImageDraw.Draw(s)
for i, (id_, table, case, rough, label) in enumerate(rows):
    x, y = (i % 4) * t, (i // 4) * (t + band)
    s.paste(Image.open(f"build/promo/out/grid/{id_}.png").resize((t, t)), (x, y))
    d.rectangle((x + 10, y + t + 12, x + 32, y + t + 34), fill=case)
    d.text((x + 42, y + t + 23), f"{id_}  {label}", font=f, fill=(235, 235, 235), anchor="lm")
s.save("build/promo/out/grid/sheet.jpg", quality=90); print("sheet ok")
PY
