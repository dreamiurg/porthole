"""P023 middle plate for M3 × 3 mm heat-set inserts and the P024 lower lid.

The measured insert has a 4.50 mm knurl and a 3.90 mm pilot. The straight
4.10 mm nominal bore clears the pilot and leaves 1.0 mm below the brass.
Run with text-to-cad 0.11.18: cad run hardware/case/puck_plate_p023.py
"""

from pathlib import Path
from tempfile import TemporaryDirectory

import build123d as bd
from cad import model
from export_twist import remove_collapsed_facets
from puck_plate_p021 import access_plate
from puck_v1 import BOSS_TOP, PILOT_D, PLATE_BOT, PLATE_R, POST_ANGLES, RADIAL_SCREW_Z, cone, cyl, polar, radial_hole

OUTPUT = Path(__file__).resolve().parent / "stl/twist-lock/r8-insert-plate/P023-plate.stl"
INSERT_AXIS_R = 33.70  # Shared with the P024 lid.
POST_FACE_Z = BOSS_TOP + 1.0  # P024 raises the seat for its M3 screw.
POST_DIAMETER = 7.70
BORE_DIAMETER = 4.10
POCKET_DEPTH = 4.00
LEAD_IN = 0.20  # 45-degree edge break, opening to 4.50 mm at the lid-side face.


def insert_plate(bore_diameter=BORE_DIAMETER, pocket_depth=POCKET_DEPTH):
    plate = access_plate("P023").split(bd.Plane.XY.offset(POST_FACE_Z), keep=bd.Keep.TOP)
    for angle in POST_ANGLES:
        x, y = polar(INSERT_AXIS_R, angle)
        # Join the pad to the full back of the plate so it rises from the flat
        # face when printed. Its solid center fills the old offset axial pilot.
        pad = cyl(POST_DIAMETER / 2, POST_FACE_Z, PLATE_BOT + 0.1, x, y)
        # The printed plate's side pilots are faceted. Clear them on the added
        # material first so their original surfaces and screw axes are preserved.
        pad -= radial_hole(angle, RADIAL_SCREW_Z, PILOT_D + 0.10, PLATE_R - 5.05, PLATE_R + 1)
        plate += pad
        bore = cyl(bore_diameter / 2, POST_FACE_Z - 0.1, POST_FACE_Z + pocket_depth, x, y)
        lead = cone(bore_diameter / 2 + LEAD_IN + 0.1, POST_FACE_Z - 0.1, bore_diameter / 2, POST_FACE_Z + LEAD_IN, x, y)
        plate -= bore + lead
    assert plate.is_valid and len(plate.solids()) == 1
    plate.label = "P023 M3 heat-set insert plate"
    return plate


@model
def p023():
    return insert_plate()


if __name__ == "__main__":
    OUTPUT.parent.mkdir(parents=True, exist_ok=True)
    with TemporaryDirectory(dir=OUTPUT.parent) as staging:
        stl = Path(staging) / OUTPUT.name
        bd.export_stl(insert_plate(), stl, tolerance=0.015)
        remove_collapsed_facets(stl)
        stl.replace(OUTPUT)
