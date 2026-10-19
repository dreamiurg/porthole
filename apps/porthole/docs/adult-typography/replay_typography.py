"""Reproduce the recorded visible-input typography tour in an isolated simulator."""

import hashlib
import json
import subprocess
import tempfile
from pathlib import Path

from PIL import Image

HERE = Path(__file__).resolve().parent
SNAP = HERE.parents[1] / "build" / "host" / "snap"


def replay(work):
    images = work / "build" / "host"
    images.mkdir(parents=True)
    subprocess.run(
        [str(SNAP), "--fresh", "--script", str(HERE / "final-input-trace.txt")],
        cwd=work,
        check=True,
        capture_output=True,
    )
    frames = json.loads((HERE / "frame-hashes.json").read_text())
    for frame in frames:
        name, expected = frame["frame"], frame["sha256_rgb"]
        with Image.open(images / (name + ".bmp")) as actual:
            digest = hashlib.sha256(actual.convert("RGB").tobytes()).hexdigest()
            if digest != expected:
                raise AssertionError(f"Rendered frame changed: {name}")
    print(f"PASS: {len(frames)} final native frames reproduced in a fresh process")


if __name__ == "__main__":
    with tempfile.TemporaryDirectory(prefix="adult-typography-") as directory:
        replay(Path(directory))
