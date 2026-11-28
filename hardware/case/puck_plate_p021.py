"""P021 battery connector access plate, in the original plate's assembly coordinates (mm).

Run with text-to-cad 0.11.18: cad run hardware/case/puck_plate_p021.py
"""

import struct
from pathlib import Path
from tempfile import TemporaryDirectory

import build123d as bd
from cad import model
from OCP.BRepBuilderAPI import BRepBuilderAPI_Sewing
from OCP.StlAPI import StlAPI_Reader
from OCP.TopoDS import TopoDS_Shape

from export_twist import remove_collapsed_facets
from puck_v1 import PLATE_BOT

HERE = Path(__file__).resolve().parent
STOCK = HERE / "stl/twist-lock/r5-comfort/puck_plate.stl"
OUTPUT = HERE / "stl/twist-lock/r6-battery-access/P021-plate.stl"
FONT = HERE.parents[1] / "apps/porthole/games/toy-town-life/art/fonts/OpenSans-Bold.ttf"
# Use the printed STL's float32 planes so the enlarged opening has uninterrupted side walls.
LEFT, RIGHT, START_Y = struct.unpack("<3f", struct.pack("<3f", 15.2, 27.2, 1.1))
TIP_X, TIP_Y = 19.8, -7.6


def stock_plate():
    """Reconstruct the shipped plate's planar STL facets, preserving the printed screw seats and posts."""
    facets = TopoDS_Shape()
    if not StlAPI_Reader().Read(facets, str(STOCK)):
        raise ValueError(f"Could not read stock plate {STOCK}")
    sew = BRepBuilderAPI_Sewing(1e-6)
    sew.Add(facets)
    sew.Perform()
    if sew.NbFreeEdges():
        raise ValueError("The stock plate has open mesh edges")
    return bd.Solid(bd.Shell(sew.SewedShape())).clean()


def access_tool():
    # Two tangent quarter ellipses follow the marked U. The slightly offset tip
    # leaves a straighter lead-out on the inner side, away from the cell pouch.
    left_y = -3.8
    with bd.BuildLine() as profile:
        bd.Polyline((LEFT, left_y), (LEFT, START_Y + 1), (RIGHT, START_Y + 1), (RIGHT, START_Y))
        bd.EllipticalCenterArc((TIP_X, START_Y), RIGHT - TIP_X, START_Y - TIP_Y, start_angle=0, arc_size=-90)
        bd.EllipticalCenterArc((TIP_X, left_y), TIP_X - LEFT, left_y - TIP_Y, start_angle=-90, arc_size=-90)
    return bd.Pos(0, 0, PLATE_BOT - 1) * bd.extrude(bd.Face(profile.wire()), amount=4, dir=(0, 0, 1))


def access_plate(part_id="P021"):
    plate = stock_plate() - access_tool()
    # Read from the battery side, outside the pouch and screw-contact areas.
    face = bd.Plane(origin=(-26, 1, PLATE_BOT - 0.05), x_dir=(0, 1, 0), z_dir=(0, 0, -1))
    label = bd.Text(part_id, font_size=4.5, font_path=str(FONT))
    plate -= face * bd.extrude(label, amount=-0.37)
    assert plate.is_valid and len(plate.solids()) == 1
    plate.label = f"{part_id} battery access plate"
    return plate


@model
def p021():
    return access_plate()


if __name__ == "__main__":
    OUTPUT.parent.mkdir(parents=True, exist_ok=True)
    with TemporaryDirectory(dir=OUTPUT.parent) as staging:
        stl = Path(staging) / OUTPUT.name
        bd.export_stl(access_plate(), stl, tolerance=0.015)
        remove_collapsed_facets(stl)
        stl.replace(OUTPUT)
