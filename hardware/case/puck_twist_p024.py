"""P024 lower lid for P023's M3 inserts and 90° countersunk M3 × 8 screws.

All heights below are measured from the flat exterior of the lid in print coordinates.
Run with text-to-cad 0.11.18: cad run hardware/case/puck_twist_p024.py
"""

from pathlib import Path
from tempfile import TemporaryDirectory

import build123d as bd
from cad import model
from export_twist import remove_collapsed_facets
from puck_plate_p023 import INSERT_AXIS_R, POST_DIAMETER, POST_FACE_Z
from puck_twist_p022 import simple_entry_lid
from puck_v1 import BOT_Z, POST_ANGLES, cone, cyl, polar

OUTPUT = Path(__file__).resolve().parent / "stl/twist-lock/r8-insert-plate/P024-lid.stl"
SEAT_HEIGHT = POST_FACE_Z - BOT_Z  # P023 land at assembly Z -16.00, 5 mm above the bed.
HEAD_MOUTH = 7.00
SCREW_CLEARANCE = 3.40
PAD_CLEARANCE = 0.20


def insert_lid():
    lid = simple_entry_lid("P024")
    # A 90° included angle gives 1 mm axial depth per mm change in radius.
    cone_depth = (HEAD_MOUTH - SCREW_CLEARANCE) / 2
    for angle in POST_ANGLES:
        x, y = polar(INSERT_AXIS_R, angle)
        # The full pad fills the previous M2 recess and supports the new head along its entire seat.
        lid += cyl(POST_DIAMETER / 2, 0, SEAT_HEIGHT, x, y)
        if angle == 90:
            # This foot passes the short upper end of the battery fence.
            lid -= cyl(POST_DIAMETER / 2 + PAD_CLEARANCE, SEAT_HEIGHT, 8, x, y)
        clearance = cyl(SCREW_CLEARANCE / 2, -0.1, SEAT_HEIGHT + 0.1, x, y)
        seat = cone(HEAD_MOUTH / 2 + 0.1, -0.1, SCREW_CLEARANCE / 2, cone_depth, x, y)
        lid -= clearance + seat
    assert lid.is_valid and len(lid.solids()) == 1
    lid.label = "P024 M3 countersunk lid"
    return lid


@model
def p024():
    return insert_lid()


if __name__ == "__main__":
    OUTPUT.parent.mkdir(parents=True, exist_ok=True)
    with TemporaryDirectory(dir=OUTPUT.parent) as staging:
        stl = Path(staging) / OUTPUT.name
        bd.export_stl(insert_lid(), stl, tolerance=0.015)
        remove_collapsed_facets(stl)
        stl.replace(OUTPUT)
