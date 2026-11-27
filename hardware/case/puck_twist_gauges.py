"""Quick, full-scale fit gauges cut from the R2 bayonet solids (mm).

Run: cad run hardware/case/puck_twist_gauges.py. Print both on their flat bases
with the same settings as the final parts. In the assembled frame the lid
seats at SEATED_Z. The hub's cut base is flat and does not need a wheel.
"""

import build123d as bd
from cad import model
from puck_twist import LUGS, TURN, twist_lid, wheel_hub
from puck_v1 import R_IN, cyl, sector


@model
def lid_gauge():
    # A low continuous ring carries all three pads throughout the turn.
    # Keep the full curved shell around each funnel, track, stop and mark.
    keep = cyl(43, -1, 2.4)
    for angle in LUGS:
        keep += sector(R_IN, 43, angle - TURN - 21, angle + 12, 2.3, 10)
    part = (twist_lid().shape & keep) - cyl(35.5, -2, 11)
    assert part.is_valid and len(part.solids()) == 1
    part.label, part.color = "R2 lid gauge", "#d4dedb"
    return part


@model
def hub_gauge():
    # Preserve the closed hoop so all three noses still load the same ring.
    # The three short shelves below the pads replace the wide glue flange.
    bottom = -2.25
    keep = cyl(46, bottom, 10) - cyl(40, bottom - 1, 11)
    for angle in LUGS:
        keep += sector(35, 42, angle - 9, angle + 9, bottom, 0)
    part = wheel_hub().shape & keep
    assert part.is_valid and len(part.solids()) == 1
    part.label, part.color = "R2 hub gauge", "#dd8655"
    return part


@model
def single_catch():
    # Use the track with the detent. Arrange the two independent pieces on
    # one print bed, maintaining the original layer direction in both.
    angle = LUGS[0]
    lid = lid_gauge().shape & sector(33, 47, angle - TURN - 21, angle + 12, -3, 12)
    hub = hub_gauge().shape & sector(33, 47, angle - 20, angle + 20, -3, 12)
    pieces = []
    row = 0
    for part, name, color in ((lid, "Track with detent", "#d4dedb"), (hub, "Lug and bearing pad", "#dd8655")):
        assert part.is_valid and len(part.solids()) == 1
        bounds = part.bounding_box()
        piece = bd.Pos(-bounds.min.X, row - bounds.min.Y, -bounds.min.Z) * part
        piece.label, piece.color = name, color
        pieces.append(piece)
        row += bounds.size.Y + 5
    return bd.Compound(children=pieces, label="R2 single catch")


if __name__ == "__main__":
    lid_gauge()
    hub_gauge()
    single_catch()
