"""P019 production lower lid with the P004 gauge's 40° bayonet (millimetres).

Run with text-to-cad 0.11.18: cad run hardware/case/puck_twist_p019.py
The STL is floor-down in print coordinates. +Y is up at the locked stop.
"""

import math
from itertools import pairwise
from pathlib import Path
from tempfile import TemporaryDirectory

import build123d as bd
from cad import model
from export_twist import remove_collapsed_facets
from puck_twist import LUGS, ROOT_R, detent, swept
from puck_v1 import BOT_Z, FLOOR_T, cyl, lid_body, sector

TURN = 40.0
MOUTH_RADIUS = 41.0
STOP_ALLOWANCE = 5.6  # Center of the 8 mm lug to its leading corner at the stop.
FONT = Path(__file__).resolve().parents[2] / "apps/porthole/games/toy-town-life/art/fonts/OpenSans-Bold.ttf"
OUTPUT = Path(__file__).resolve().parent / "stl/twist-lock/r4-p004/P019-lid.stl"


def shelf_air():
    """Air above the P004 retaining shelf, with horizontal tangents at its stations."""

    def tangent(angle):
        return MOUTH_RADIUS * math.sin(math.radians(angle - TURN / 2))

    stations = ((8, 2.55), (14, 3.425), (30, 3.465), (37, 3.425))
    start_y = tangent(0)
    with bd.BuildLine() as outline:
        first, last = stations[0], stations[-1]
        bd.Polyline((start_y, first[1]), (tangent(first[0]), first[1]))
        for (a0, z0), (a1, z1) in pairwise(stations):
            y0, y1 = tangent(a0), tangent(a1)
            third = (y1 - y0) / 3
            bd.Bezier((y0, z0), (y0 + third, z0), (y1 - third, z1), (y1, z1))
        bd.Polyline((tangent(last[0]), last[1]), (32, last[1]), (32, 11), (start_y, 11), (start_y, first[1]))
    face = bd.Plane.YZ * bd.Face(outline.wire())
    return bd.Rot(0, 0, TURN / 2) * bd.Pos(28, 0, 0) * bd.extrude(face, amount=22, dir=(1, 0, 0))


def p004_track():
    mouth_profile = bd.Plane.YZ * bd.Polygon((-10.7, -1), (10.7, -1), (6.9, 2.8), (6.9, 9), (-6.9, 9), (-6.9, 2.8), align=None)
    mouth = bd.Pos(33, 0, 0) * bd.extrude(mouth_profile, amount=12, dir=(1, 0, 0))
    roof = swept([(34, -1), (44, -1), (44, 8.05), (42.15, 8.05), (40.7, 6.60), (34, 6.60)], -17, TURN + STOP_ALLOWANCE)
    return (roof & (shelf_air() + mouth)) - cyl(ROOT_R, -2, 12)


def production_lid(part_id="P019"):
    stock = bd.Pos(0, 0, -BOT_Z) * lid_body(magnet=False, vent_pitch=2.4)
    lid = stock
    track = p004_track()
    for stop in LUGS:
        entry = stop - TURN
        lid -= bd.Rot(0, 0, entry) * track
        lid -= sector(41.55, 44, entry - 1.1, entry + 1.1, 7.80, 9.1)
    lid += detent() & stock
    # Recess the ID in the interior floor beside the battery fence, clear of the lock and the five buzzer holes.
    label = bd.Text(part_id, font_size=4.5, font_path=str(FONT), align=(bd.Align.CENTER, bd.Align.CENTER))
    lid -= bd.Pos(25.5, 8, FLOOR_T - 0.4) * bd.Rot(0, 0, 90) * bd.extrude(label, amount=0.5)
    assert lid.is_valid and len(lid.solids()) == 1
    lid.label = f"{part_id} – P004 production lid"
    return lid


@model
def p019():
    return production_lid()


if __name__ == "__main__":
    OUTPUT.parent.mkdir(parents=True, exist_ok=True)
    with TemporaryDirectory(dir=OUTPUT.parent) as staging:
        stl = Path(staging) / OUTPUT.name
        bd.export_stl(production_lid(), stl, tolerance=0.015)
        remove_collapsed_facets(stl)
        stl.replace(OUTPUT)
