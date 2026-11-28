"""P022 simple-entry lid. Regenerate with cad run hardware/case/puck_twist_p022.py."""

import math
from pathlib import Path
from tempfile import TemporaryDirectory

import build123d as bd
from cad import model
from export_twist import remove_collapsed_facets
from puck_twist import LUGS, ROOT_R
from puck_twist_p019 import TURN, production_lid, track_with_mouth

OUTPUT = Path(__file__).resolve().parent / "stl/twist-lock/r7-simple-entry/P022-lid.stl"
GUIDE_AT_FLOOR = -9.6
GUIDE_SLOPE = 0.35  # Tangential advance per mm of height on the annotated trailing cheek.
RETAINING_WALL = 9.4
EDGE_RADIUS = 0.30


def exposed_entry_edges(lid):
    """Find the two outer cheeks, the retaining lip's nose, and the floor edge at each entry."""
    edges = []
    for edge in lid.edges():
        midpoint = edge @ 0.5
        radius = math.hypot(midpoint.X, midpoint.Y)
        offset = (math.degrees(math.atan2(midpoint.Y, midpoint.X)) - (LUGS[0] - TURN) + 60) % 120 - 60
        y = radius * math.sin(math.radians(offset))
        z = midpoint.Z
        on_cheek = min(abs(y - RETAINING_WALL), abs(y - (GUIDE_AT_FLOOR + GUIDE_SLOPE * z))) < 1e-5
        floor_edge = edge.geom_type == bd.GeomType.CIRCLE and abs(radius - ROOT_R) < 1e-5 and z < 0.25
        cheek_edge = edge.geom_type == bd.GeomType.BSPLINE and on_cheek and radius > ROOT_R and 0 < z < 3.5
        if floor_edge or cheek_edge:
            edges.append(edge)
    if len(edges) != 4 * len(LUGS):
        raise ValueError(f"Expected four exposed edges per entry, found {len(edges)}")
    return edges


def simple_entry_lid():
    bottom, top = -1.0, 9.0
    opening = bd.Plane.YZ * bd.Polygon(
        (GUIDE_AT_FLOOR + GUIDE_SLOPE * bottom, bottom),
        (RETAINING_WALL, bottom),
        (RETAINING_WALL, top),
        (GUIDE_AT_FLOOR + GUIDE_SLOPE * top, top),
        align=None,
    )
    mouth = bd.Pos(33, 0, 0) * bd.extrude(opening, amount=12, dir=(1, 0, 0))
    lid = production_lid("P022", track_with_mouth(mouth))
    lid = bd.fillet(exposed_entry_edges(lid), EDGE_RADIUS)
    assert lid.is_valid and len(lid.solids()) == 1
    lid.label = "P022 simple-entry lid"
    return lid


@model
def p022():
    return simple_entry_lid()


if __name__ == "__main__":
    OUTPUT.parent.mkdir(parents=True, exist_ok=True)
    with TemporaryDirectory(dir=OUTPUT.parent) as staging:
        stl = Path(staging) / OUTPUT.name
        bd.export_stl(simple_entry_lid(), stl, tolerance=0.015)
        remove_collapsed_facets(stl)
        stl.replace(OUTPUT)
