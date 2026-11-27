"""Revision 2 side-groove bayonet for the stock v1.4 puck and a printed v8 wheel.

cad run hardware/case/puck_twist.py (mm). +Y is up on the wheel. Remove the
old badge/cover and steel ring. Register the hub pilot in the empty recess,
point the hub index at wheel-up and dry-fit before bonding the flat surfaces.
Align any lid entry mark with any hub mark. Lower the puck, turn 30 degrees
clockwise to the stop, and turn back to remove it. For an upright display,
start 30 degrees counterclockwise from upright.

Regenerate production and gauge STLs with cad run hardware/case/export_twist.py.
"""

import build123d as bd
from cad import model
from cad.build123d import configuration_sequence, mechanism_state
from puck_v1 import BOT_Z, R_OUT, cyl, lid_body, polar, sector

TURN = 30
LUGS = (90, 210, 330)
ROOT_R, TIP_R = 39.8, 40.5  # 0.70 mm radial running clearance
BORE_R = R_OUT + 0.65
HALF_TRACK = 6.1  # degrees, clears a rounded 8 mm lug at its inner radius
PILOT_R, PILOT_H = 27.95, 1.0  # full circular 56.4 x 1.3 mm cover recess
SEAT_R, BASE_Z = 41.9, -5.0  # v8 puck seat is 84.6 mm across and 3 mm deep
SEATED_Z = -0.20  # stock ring floor is at -1.05; revised bearing height is 0.85 mm
CAM_ANGLE = LUGS[0] - 8


def swept(profile, start=0, end=360):
    face = bd.Plane.XZ * bd.Polygon(*profile, align=None)
    return bd.Rot(0, 0, start) * bd.revolve(face, axis=bd.Axis.Z, revolution_arc=end - start)


def funnel(angle):
    # One side guides the lug down. The other rises smoothly into the
    # retaining shelf, ending tangent to its horizontal face at z=2.95.
    with bd.BuildLine() as outline:
        bd.Polyline((-15, -1), (8, -1))
        bd.Bezier((8, -1), (11, -1), (12.5, 2.95), (16.5, 2.95))
        bd.Polyline((16.5, 2.95), (16.5, 6.4), (-4.2, 6.4), (-4.2, 4.7), (-13, 0), (-15, -1))
    cut = bd.extrude(bd.Plane.YZ * bd.Face(outline.wire()), amount=13, dir=(1, 0, 0))
    return bd.Rot(0, 0, angle) * bd.Pos(35, 0, 0) * cut


def detent():
    # Only one lug crosses this bump per turn, in any of the three orientations.
    # The first print needed too much force at 0.15 mm nominal interference.
    r = 1.4
    return cyl(r, 4.45, 5.25, *polar(TIP_R + 0.04 - r, CAM_ANGLE))


@model
def twist_lid():
    stock = bd.Pos(0, 0, -BOT_Z) * lid_body(magnet=False, vent_pitch=2.4)
    lid = stock
    for angle in LUGS:
        start = angle - TURN
        track = swept([(34, 2.95), (44, 2.95), (44, 8.05), (42.15, 8.05), (40.7, 6.60), (34, 6.60)], start - HALF_TRACK, angle + HALF_TRACK)
        lid -= (track + funnel(start)) - cyl(ROOT_R, -2, 10)
        # Visible with the bezel screwed on.
        lid -= sector(41.55, 44, start - 1.1, start + 1.1, 7.80, 9.1)
    lid += detent() & stock
    assert lid.is_valid and len(lid.solids()) == 1 and abs(lid.bounding_box().size.Y - 84) < 0.001
    lid.label, lid.color = "Side-groove lid", "#d4dedb"
    return lid


def lug(angle):
    # Short rounded boss, backed across its entire root by the continuous rim.
    length, width, height = 4.05, 8.0, 2.75
    blank = bd.Box(length, width, height)
    rounded = bd.fillet(blank.edges(), 0.8)
    return bd.Rot(0, 0, angle) * bd.Pos(TIP_R + length / 2, 0, 4.775 + SEATED_Z) * rounded


@model
def wheel_hub():
    # One continuous ring. The outside bevel clears the v8 pocket lip and
    # the flared inside lip centers the puck before the lugs meet the funnels.
    outer = 45.35
    profile = [
        (19, BASE_Z - PILOT_H),
        (PILOT_R, BASE_Z - PILOT_H),
        (PILOT_R, BASE_Z),
        (SEAT_R, BASE_Z),
        (SEAT_R, -2),
        (outer, -2 + outer - SEAT_R),
        (outer, 8.65),
        (44.0, 8.65),
        (BORE_R, 7.05),
        (BORE_R, 0.85),
        (40.5, -1.05),
        (19, -1.05),
    ]
    ring = swept(profile)
    for angle in LUGS:
        ring += lug(angle)
        ring += sector(36, 38.8, angle - 6, angle + 6, -1.15, SEATED_Z)
        ring -= sector(43, outer + 1, angle - 1.1, angle + 1.1, 7.9, 9.0)
    assert ring.is_valid and len(ring.solids()) == 1
    ring.label, ring.color = "Glue-on wheel hub", "#dd8655"
    return ring


@model
def twist_lock():
    hub, lid = wheel_hub().shape, twist_lid().shape
    joint = bd.CylindricalJoint("mount", hub, axis=bd.Axis.Z, angular_range=(0, TURN), linear_range=(SEATED_Z, 18))
    bd.RigidJoint("mount", lid, bd.Location())
    joint.connect_to(lid.joints["mount"], angle=TURN, position=18)
    pair = bd.Compound(children=[hub, lid], label="Side bayonet")
    for name, pose in (("Align", (TURN, 18)), ("Insert", (TURN, SEATED_Z)), ("Lock", (0, SEATED_Z))):
        mechanism_state(pair, name, mount=pose)
    return configuration_sequence(pair, "Insert and twist", [(0, "Align"), (1.4, "Insert"), (2.6, "Lock")])


if __name__ == "__main__":
    twist_lid()
    wheel_hub()
    twist_lock()
