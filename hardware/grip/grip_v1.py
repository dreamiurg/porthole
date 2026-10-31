"""Snap-on tilt grip for the Porthole puck, v1 (build123d / cadgen).

Run: `cadgen +0.8.4 run hardware/grip/grip_v1.py` builds the grip, the seated assembly with the
v1.2 puck, and a small snap test ring. Same frame as the puck: origin = puck center, +Z out of the
screen, +Y = top of the screen. Prints back-down (z = BACK_Z on the bed) in one colour, PETG.

The puck presses in from the front: six fingers flex out and their bumps click into the lid's V
groove; a rib in the lid's key notch fixes the rotation so tilt axes stay square to the grips.
Push it out through the thumb hole in the back. Shape: Yoke Pro horns + Racing U bottom bar.
"""

import sys
from pathlib import Path

import build123d as bd
from cadgen import model
from cadgen.geometry import overlap_volume

sys.path.insert(0, str(Path(__file__).resolve().parents[1] / "case"))
import puck_v1 as pk  # noqa: E402

# ---- cradle (mm) ----------------------------------------------------------------------------
FIT = 0.2  # radial clearance around the lid
R_CUP = pk.R_OUT + FIT  # 42.2, inner face of the fingers
FINGER_T = 1.4
R_FINGER = R_CUP + FINGER_T  # 43.6
RELIEF = 2.0  # gap behind the fingers so they can flex out
R_HUB_IN = R_FINGER + RELIEF  # 45.6
R_HUB = R_HUB_IN + 5.0  # 50.6, rigid ring the arms join
SEAT_Z = pk.BOT_Z  # lid bottom rests on the ledge here
LEDGE_T = 2.0
ROOT_Z = SEAT_Z - LEDGE_T  # fingers grow from the root ring at z -22.2
BACK_Z = ROOT_Z - 4.0  # back face, on the print bed
CUP_TOP = pk.SPLIT_Z - 0.6  # fingers stop below the bezel/lid seam: ports and pinholes stay open
LEDGE_R = R_CUP - 0.6  # ledge stops short of the fingers so it doesn't stiffen their roots
THUMB_R = 34.0  # push-out hole; also clears the buzzer vent (r 30 to 33)
FINGERS = 6
FINGER_SLOT = 3.0  # gap between fingers
BUMP = 0.8  # radial engagement: how far a bump reaches into the lid groove
KEY_FIT = 0.3  # side clearance of the key rib in the lid notch

# ---- frame (mm) -----------------------------------------------------------------------------
GRIP_X = 78.0  # grip centre; a thumb reaches the glass edge (r 37.5) from here
GRIP_LEN, GRIP_W = 72.0, 28.0  # centre-to-centre length and width of each grip
GRIP_Y, GRIP_TILT = 6.0, 8.0  # grips sit a little high and lean out at the top, like the yoke horns
GRIP_FRONT_Z = 0.0  # grips are 26 mm deep
ARM_Y, ARM_W = -6.0, 18.0
BAR_Y, BAR_W = -64.0, 14.0  # bottom bar runs under the puck
FRAME_FRONT_Z = CUP_TOP - 1.0  # arms and bar, below the cup rim
BED = 250.0  # X1 Carbon bed is 256; keep room for a brim


def ring(r0, r1, z0, z1):
    return pk.cyl(r1, z0, z1) - pk.cyl(r0, z0 - 1, z1 + 1)


def slot(p1, p2, w, z0, z1, front_fillet):
    """A stadium from p1 to p2 extruded z0..z1, its front edges rounded."""
    (x1, y1), (x2, y2) = p1, p2
    length = ((x2 - x1) ** 2 + (y2 - y1) ** 2) ** 0.5
    ang = bd.Vector(x2 - x1, y2 - y1, 0).get_angle(bd.Vector(1, 0, 0)) * (1 if y2 >= y1 else -1)
    face = bd.Pos((x1 + x2) / 2, (y1 + y2) / 2) * bd.Rot(0, 0, ang) * bd.SlotCenterToCenter(length, w)
    solid = bd.Pos(0, 0, z0) * bd.extrude(face, z1 - z0)
    return solid.fillet(front_fillet, solid.edges().group_by(bd.Axis.Z)[-1])


def frame():
    f = pk.cyl(R_HUB, BACK_Z, CUP_TOP)
    grips = []
    for s in (-1, 1):
        lean = s * GRIP_TILT
        top = bd.Vector(0, GRIP_LEN / 2).rotate(bd.Axis.Z, -lean)
        a = (s * GRIP_X - top.X, GRIP_Y - top.Y)
        b = (s * GRIP_X + top.X, GRIP_Y + top.Y)
        grips.append(slot(a, b, GRIP_W, BACK_Z, GRIP_FRONT_Z, 9.0))
        f += slot((s * (R_HUB - 5), ARM_Y), (s * GRIP_X, ARM_Y), ARM_W, BACK_Z, FRAME_FRONT_Z, 4.0)
        f += slot((s * GRIP_X, a[1] + 6), (s * 40, BAR_Y), BAR_W, BACK_Z, FRAME_FRONT_Z, 4.0)
    f += slot((-40, BAR_Y), (40, BAR_Y), BAR_W, BACK_Z, FRAME_FRONT_Z, 4.0)
    for g in grips:
        f += g
    return f


def cradle_cut():
    """Everything the cradle hollows out of the frame: puck space, finger relief, thumb hole."""
    return pk.cyl(R_HUB_IN, ROOT_Z, 40) + pk.cyl(THUMB_R, BACK_Z - 1, ROOT_Z + 1)


def fingers():
    f = ring(R_CUP, R_FINGER, ROOT_Z, CUP_TOP)
    # bump: the lid groove's V profile, BUMP deep, 0.2 mm short of the groove floor
    tip, h = pk.R_OUT - BUMP, pk.SNAP_FLAT / 2
    f += pk.revolved((tip, pk.SNAP_Z - h), (R_CUP + 0.5, pk.SNAP_Z - h - (R_CUP + 0.5 - tip)), (R_CUP + 0.5, pk.SNAP_Z + h + (R_CUP + 0.5 - tip)), (tip, pk.SNAP_Z + h))
    for i in range(FINGERS):  # slots between fingers; one is centred on the key so the rib sits in a gap
        deg = pk.KEY_DEG + i * 360 / FINGERS
        w = pk.KEY_W + 4.0 if i == 0 else FINGER_SLOT
        f -= pk.radial_box(R_CUP, deg, (ROOT_Z + CUP_TOP) / 2 + 1, 6.0, w, CUP_TOP - ROOT_Z + 2)
    return f


def key_rib():
    r_tip = pk.R_OUT - pk.KEY_D + FIT
    return pk.radial_box((r_tip + R_HUB_IN) / 2 + 0.5, pk.KEY_DEG, (ROOT_Z + CUP_TOP) / 2, R_HUB_IN - r_tip + 1, pk.KEY_W - 2 * KEY_FIT, CUP_TOP - ROOT_Z)


def cradle_parts():
    ledge = ring(THUMB_R, LEDGE_R, ROOT_Z, SEAT_Z)
    return ledge + fingers() + key_rib()


def grip_body():
    g = frame() - cradle_cut() + cradle_parts()
    g.label, g.color = "grip", "#3d7bd9"
    return g


def finger_strain():
    """Peak bending strain of a finger pushed out by BUMP: 1.5 * t * d / L^2 (cantilever)."""
    length = pk.SNAP_Z - ROOT_Z  # free length from the root ring to the bump (the ledge doesn't touch the fingers)
    return 1.5 * FINGER_T * BUMP / length**2


@model
def grip():
    g = grip_body()
    bb = g.bounding_box().size
    assert bb.X < BED and bb.Y < BED, f"too big for the bed: {bb}"
    assert finger_strain() < 0.03, f"finger strain {finger_strain():.3f}: PETG wants under 3% for repeated snaps"
    return g


@model
def grip_with_puck():
    g = grip().shape
    puck = [pk.bezel_body(), pk.plate_body(), pk.lid_body(), pk.board_ref()]
    for p in puck:  # seated, nothing interferes: bumps sit in the groove, the rib in the notch
        v = overlap_volume(g, p)
        assert v < 1e-3, f"grip x {p.label} overlap {v:.3f} mm3"
    lid = puck[2]
    assert lid.bounding_box().min.Z >= SEAT_Z - 1e-6, "lid sits below the ledge"
    return bd.Compound(children=[g, *puck], label="grip_with_puck")


@model
def snap_test():
    """Just the cradle (hub ring, ledge, fingers, rib): a 30-minute print to check the click."""
    t = ring(THUMB_R, R_HUB, BACK_Z, CUP_TOP) - cradle_cut() + cradle_parts()
    t.label, t.color = "snap_test", "#3d7bd9"
    return t


if __name__ == "__main__":
    grip_with_puck()
    snap_test()
