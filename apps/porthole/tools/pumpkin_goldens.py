#!/usr/bin/env python3
"""Check pixel-exact Pumpkin storyboards; --update accepts reviewed art changes.

Run after `make snap`. Reuses deterministic win/loss tours and stores full 480px
PNGs with no resampling. Interior 3x3 blocks stay crisp; the emulator clips the physical circular rim.
"""

import argparse
import subprocess
import tempfile
from pathlib import Path

from PIL import Image

ROOT = Path(__file__).resolve().parents[1]
DEST = ROOT / "games/pumpkin-hide-seek/docs"
SHOTS = (
    "pumpkin_intro",
    "pumpkin_calibrate",
    "pumpkin_hidden_search",
    "pumpkin_won",
    "pumpkin_catch",
    "pumpkin_icing_top",
    "pumpkin_icing_middle",
    "pumpkin_lost",
)


def render(directory):
    (directory / "build/host").mkdir(parents=True)
    for name in ("70_pumpkin_win", "71_pumpkin_loss"):
        lines = (ROOT / f"tests/playtests/{name}.txt").read_text().splitlines()
        raw = [line for line in lines if not line.startswith(("expect", "ui-check", "sheet"))]
        script = directory / "tour.txt"
        script.write_text("\n".join(raw) + "\n")
        subprocess.run([str(ROOT / "build/host/snap"), "--script", str(script)], cwd=directory, check=True, stdout=subprocess.DEVNULL)


def check_image(directory, name, update):
    with Image.open(directory / f"build/host/{name}.bmp") as source:
        image = source.convert("RGB")
    interior = image.crop((72, 72, 408, 408))
    reduced = interior.resize((112, 112), Image.Resampling.NEAREST)
    assert interior.tobytes() == reduced.resize((336, 336), Image.Resampling.NEAREST).tobytes(), name
    target = DEST / f"{name}.png"
    if update:
        image.save(target)
        return True
    if not target.exists():
        print(f"Missing golden: {target}")
        return False
    with Image.open(target) as golden:
        same = golden.size == image.size and golden.convert("RGB").tobytes() == image.tobytes()
    if not same:
        print(f"Golden differs: {target}; review the tour before using --update")
    return same


def main():
    parser = argparse.ArgumentParser(description=__doc__)
    parser.add_argument("--update", action="store_true")
    args = parser.parse_args()
    with tempfile.TemporaryDirectory() as tmp:
        directory = Path(tmp)
        render(directory)
        results = [check_image(directory, name, args.update) for name in SHOTS]
    if not all(results):
        raise SystemExit(1)
    print(f"Pumpkin: {len(SHOTS)} pixel-exact golden screenshots {'updated' if args.update else 'match'}")


if __name__ == "__main__":
    main()
