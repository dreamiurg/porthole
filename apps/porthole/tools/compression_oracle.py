#!/usr/bin/env python3
"""Check packed Biscuit pixels against the immutable pre-optimization SHA-256 oracle.

Captured from commit 5653dfd, before regeneration. The ordered hashes cover 96
pictures followed by all 504 scene references, including repeated frames. They
were independently verified against the original RLE arrays. The original
manifest file's SHA-256 was
6023e07eebdf690ed41f70594ce70b1b6736705c3786f3aa8ac2ce9493ac0d20.

This oracle intentionally does not read generated/manifest.json: changing both
the payload and that manifest must never silently authorize a visual change.
"""

import hashlib
import re
import zlib
from pathlib import Path

PIXEL_ORACLE = "d806344faff31cdba030b2e5f2c589fb9f9bc065aec89134c2096efcfab7dcf6"
GENERATED = Path(__file__).resolve().parents[1] / "games/biscuit/generated"


def check() -> None:
    table = (GENERATED / "art_data.inc").read_text()
    chunks = re.findall(r'#include "(art_data_\d+\.inc)"', table)
    assert chunks and len(chunks) == len(set(chunks)), "missing or repeated payload chunks"
    arrays = {}
    for chunk in chunks:
        for name, values in re.findall(r"const uint8_t ((?:picture|scene)_\d+)\[\] = \{(.*?)\};", (GENERATED / chunk).read_text(), re.S):
            assert name not in arrays, f"repeated payload: {name}"
            arrays[name] = bytes(map(int, re.findall(r"\d+", values)))
    assert len(arrays) == 465, "96 pictures and 369 unique scenes must remain"
    refs = re.findall(r"\{(\d+),(\d+),((?:picture|scene)_\d+),(\d+)\}", table)
    assert len(refs) == 600, "96 picture and 504 scene references must remain"
    oracle = hashlib.sha256()
    used = set()
    for index, (width, height, name, count) in enumerate(refs):
        dimensions = (96, 48) if index < 96 else (160, 160)
        assert (int(width), int(height)) == dimensions, f"changed dimensions: {name}"
        assert name.startswith("picture_" if index < 96 else "scene_"), f"changed reference order: {name}"
        payload = arrays[name]
        assert len(payload) == int(count), f"wrong byte count: {name}"
        decoder = zlib.decompressobj(wbits=15)
        raw = decoder.decompress(payload)
        assert decoder.eof and not decoder.unused_data and not decoder.unconsumed_tail, f"invalid zlib stream: {name}"
        assert len(raw) == dimensions[0] * dimensions[1] * 2, f"wrong pixel count: {name}"
        oracle.update(hashlib.sha256(raw).digest())
        used.add(name)
    assert used == arrays.keys(), "unreferenced packed art"
    assert oracle.hexdigest() == PIXEL_ORACLE, "Biscuit pixels or reference order differ from commit 5653dfd"
    print("Biscuit compression oracle: all 96 pictures and 504 scene references match original SHA-256")


if __name__ == "__main__":
    check()
