"""Regenerate the five R2 print meshes with text-to-cad 0.11.18.

From the repository root: cad run hardware/case/export_twist.py
"""

import struct
from pathlib import Path
from tempfile import TemporaryDirectory

from puck_twist import twist_lid, wheel_hub
from puck_twist_gauges import hub_gauge, lid_gauge, single_catch

OUTPUT = Path(__file__).resolve().parent / "stl/twist-lock/r2"
PARTS = (
    (twist_lid, "side-groove-lid-r2.stl"),
    (wheel_hub, "wheel-hub-r2.stl"),
    (lid_gauge, "fit-test/lid-gauge.stl"),
    (hub_gauge, "fit-test/hub-gauge.stl"),
    (single_catch, "fit-test/single-catch-r2.stl"),
)


def remove_collapsed_facets(stl):
    """OCCT sometimes emits triangles whose vertices coincide in float32 STL."""
    data = stl.read_bytes()
    count = struct.unpack_from("<I", data, 80)[0]
    if len(data) != 84 + count * 50:
        raise ValueError(f"Invalid binary STL length in {stl}")
    facets = []
    for offset in range(84, len(data), 50):
        facet = data[offset : offset + 50]
        xyz = struct.unpack_from("<9f", facet, 12)
        if len({xyz[:3], xyz[3:6], xyz[6:]}) == 3:
            facets.append(facet)
    if len(facets) != count:
        stl.write_bytes(data[:80] + struct.pack("<I", len(facets)) + b"".join(facets))


def export_parts(output):
    for part, name in PARTS:
        path = output / name
        path.parent.mkdir(parents=True, exist_ok=True)
        # Publish only a completed, cleaned mesh. A failed build leaves its previous STL intact.
        with TemporaryDirectory(dir=path.parent) as staging:
            stl = Path(staging) / path.name
            part().export(stl, tolerance=0.025)
            remove_collapsed_facets(stl)
            stl.replace(path)


if __name__ == "__main__":
    export_parts(OUTPUT)
