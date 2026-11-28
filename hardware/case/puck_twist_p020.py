"""P020 handheld lid. Run with text-to-cad 0.11.18: cad run hardware/case/puck_twist_p020.py"""

import math
from pathlib import Path
from tempfile import TemporaryDirectory

import build123d as bd
from cad import model
from export_twist import remove_collapsed_facets
from puck_twist import LUGS, ROOT_R
from puck_twist_p019 import TURN, production_lid, shelf_air

OUTPUT = Path(__file__).resolve().parent / "stl/twist-lock/r5-comfort/P020-lid.stl"
FLOOR_RADIUS = 1.25
HEEL_RADIUS = 0.65
NOSE_RADIUS = 0.50


def mouth_edges(lid, *, heels):
    """Look up the exposed P004 edges by their positions, independently of BREP indexing."""
    selected = []
    for edge in lid.edges():
        center = edge @ 0.5
        radius = math.hypot(center.X, center.Y)
        angle = math.degrees(math.atan2(center.Y, center.X))
        # Signed offset from the nearest entry center, symmetric across the three mouths.
        offset = (angle - (LUGS[0] - TURN) + 60) % 120 - 60
        if heels:
            # Transverse edge where the trailing 45-degree lead-in becomes vertical.
            if edge.geom_type == bd.GeomType.LINE and abs(center.Z - 2.8) < 1e-5 and 40.6 < radius < 41.2 and -10.1 < offset < -9.5:
                selected.append(edge)
        elif edge.geom_type == bd.GeomType.BSPLINE:
            outer_cheek = 1.0 < center.Z < 1.35 and 40.9 < radius < 41.7 and 11 < abs(offset) < 13
            shelf_nose = 2.6 < center.Z < 2.8 and 40.6 < radius < 41.3 and 9.6 < offset < 9.9
            if outer_cheek or shelf_nose:
                selected.append(edge)
    expected = len(LUGS) * (1 if heels else 3)
    if len(selected) != expected:
        raise ValueError(f"Expected {expected} P004 mouth edges, found {len(selected)}")
    return selected


def floor_round_tool():
    """Round the root into the floor, blending both corners into the mouth cheeks."""
    profiles = []
    # Six-degree steps limit the 1.25 mm meridian's chord error to 0.002 mm.
    # The first station extends below the bed for a clean through-cut.
    for angle in [-1, *range(0, 91, 6)]:
        phi = math.radians(max(angle, 0))
        z = FLOOR_RADIUS * (1 - math.cos(phi)) if angle >= 0 else -0.15
        root = ROOT_R - FLOOR_RADIUS + FLOOR_RADIUS * math.sin(phi)
        # Use the P004 45-degree mouth line. A 1.8 mm flare at floor level
        # recedes with zero slope at the top, where the original guide takes over.
        outer_y = 9.7 - z + 1.8 * (1 - max(z, 0) / FLOOR_RADIUS) ** 2 - 0.05
        blend = 1.55
        y = outer_y - blend
        x = math.sqrt(root * root - y * y)
        tangent = y / x * blend * 0.56
        with bd.BuildLine() as profile:
            bd.ThreePointArc((x, -y), (root, 0), (x, y))
            bd.Bezier((x, y), (x - tangent, y + blend * 0.56), (x + 0.4, outer_y), (x + blend, outer_y))
            bd.Polyline((x + blend, outer_y), (45, outer_y), (45, -outer_y), (x + blend, -outer_y))
            bd.Bezier((x + blend, -outer_y), (x + 0.4, -outer_y), (x - tangent, -y - blend * 0.56), (x, -y))
        profiles.append(bd.Pos(0, 0, z) * profile.wire())
    return bd.Solid.make_loft(profiles, ruled=True)


def shelf_round_tool():
    """Ease the first few millimetres of the shelf's outside edge, following its actual height."""

    def ease(fraction):
        t = min(1, max(0, fraction))
        return t * t * (3 - 2 * t)

    shelf = shelf_air()
    profiles = []
    for i in range(18):
        angle = 9 + i / 2
        radius = 0.35 * min(ease((angle - 9) / 1.5), ease((17.5 - angle) / 3)) + 0.01
        phi = math.radians(angle)
        axis = bd.Axis((42 * math.cos(phi), 42 * math.sin(phi), -2), (0, 0, 1))
        z = min(point.Z for point, _ in shelf.find_intersection_points(axis))
        r = 42 - radius
        mid = radius / math.sqrt(2)
        with bd.BuildLine() as profile:
            bd.ThreePointArc((r, z), (r + mid, z - radius + mid), (42, z - radius))
            bd.Polyline((42, z - radius), (43, z - radius), (43, z + 0.16), (r, z + 0.16), (r, z))
        profiles.append(bd.Rot(0, 0, angle) * (bd.Plane.XZ * profile.wire()))
    return bd.Solid.make_loft(profiles, ruled=True)


def comfort_lid():
    lid = production_lid("P020")
    lid = bd.fillet(mouth_edges(lid, heels=True), HEEL_RADIUS)
    lid = bd.fillet(mouth_edges(lid, heels=False), NOSE_RADIUS)
    for tool in (floor_round_tool(), shelf_round_tool()):
        for stop in LUGS:
            lid -= bd.Rot(0, 0, stop - TURN) * tool
    assert lid.is_valid and len(lid.solids()) == 1
    lid.label = "P020 – handheld comfort lid"
    return lid


@model
def p020():
    return comfort_lid()


if __name__ == "__main__":
    OUTPUT.parent.mkdir(parents=True, exist_ok=True)
    with TemporaryDirectory(dir=OUTPUT.parent) as staging:
        stl = Path(staging) / OUTPUT.name
        bd.export_stl(comfort_lid(), stl, tolerance=0.015)
        remove_collapsed_facets(stl)
        stl.replace(OUTPUT)
