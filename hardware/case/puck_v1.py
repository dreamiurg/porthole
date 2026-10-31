"""Porthole puck case v1.1 as a build123d model (text-to-cad / cadgen). Port of puck_case_v1_fusion.py.

Run: `cadgen +0.8.4 run hardware/case/puck_v1.py` builds Bezel, Plate, Lid and the assembly with
reference bodies for the board and the cell, and opens them in the CAD viewer. Millimetres in the
board's STEP frame: origin = board center, +Z out of the screen, glass top at z = +3.4.

v1.1 after the first print: M2 pilots 1.6 -> 1.8 (screws drove too tight), wall 2.0 -> 4.2 for a
fatter puck (dia 79.6 -> 84). The radial screw counterbore deepens with the wall so the same
screws still reach the posts; side cutouts span the thicker wall.
"""

import math

import build123d as bd
from cadgen import model
from cadgen.geometry import overlap_volume

# ---- parameters (mm) ------------------------------------------------------------------------
GLASS_R = 37.5  # board / touch glass radius
GLASS_TOP = 3.4
STANDOFF_Z = -6.1  # bottom face of the board's M2 standoffs
FIT = 0.3  # radial gap glass -> bezel wall
WALL = 4.2
R_IN = GLASS_R + FIT  # 37.8
R_OUT = R_IN + WALL  # 42.0
LIP_T = 1.2  # lip thickness over the glass
LIP_W = 2.0  # lip overlap onto the glass (active area r = 26.7, black mask beyond)
TOP_Z = GLASS_TOP + LIP_T  # 4.6
PLATE_T = 1.6
PLATE_R = R_IN - 0.2
PLATE_BOT = STANDOFF_Z - PLATE_T  # -7.7
CELL = (36.2, 57.0, 10.6)  # pocket for EEMB LP103454 (34.5 x 55-56 x 10.3) + tolerance
CELL_Y0 = 1.2  # pocket center y (clears the top post and the bottom standoff screws)
CELL_TOP = PLATE_BOT - 0.3
FLOOR_T = 1.6
FLOOR_TOP = CELL_TOP - CELL[2]  # -18.6
BOT_Z = FLOOR_TOP - FLOOR_T  # -20.2
SPLIT_Z = -12.0  # bezel / lid seam
EDGE_FILLET_TOP = 2.5
EDGE_FILLET_BOT = 3.0
POST_ANGLES = (90.0, 190.0, 350.0)  # plate posts: bezel radial screws + lid axial screws
POST_W = 6.0
POST_R0 = PLATE_R - 6.0
POST_RC = (POST_R0 + PLATE_R) / 2  # lid screw axis radius
RADIAL_SCREW_Z = -9.6
PILOT_D = 1.8  # M2 self-tapping pilot; 1.6 was too tight in the first print
CUT_DEPTH = 2 * (WALL + 2.0)  # radial length of side cutouts centered on R_OUT: reach r = R_IN - 2
HEAD_R = R_IN + 1.2  # radial screw head seat, same as v1 so the same screws reach the posts
STANDOFFS = [(29.5, 13.6), (-29.5, 13.6), (18.0, -27.05), (-18.0, -27.05)]
USB = {"USB (native)": -43.45, "UART": -136.4}  # receptacle axis angles, mouth at r = 36.8
USB_Z = -3.27
USB_OPEN = (13.0, 7.5)  # tangential x height, fits a standard USB-C overmold
SWITCH = (-11.3, -2.75, 9.0, 3.6)  # angle, z, tangential slot length, height
BUTTONS = {"BOOT": 13.5, "RESET": 0.9}  # angles; plungers at r = 34, z = -2.85
LEDS = {"CHG": -23.3, "PWR": -30.3}
LED_Z = -2.85  # window center; LEDs sit on the back of the PCB (z -2.1 down), -4.4 in v1 missed them
BUZZER = (-27.5, -12.3)  # from Waveshare's labelled photo; not in the STEP by name - verify
BUZZER_HOLES = [(0, 0), (2.2, 0), (-2.2, 0), (0, 2.2), (0, -2.2)]
J1 = (21.2, 8.6)  # battery socket center


# ---- primitives (same signatures as the Fusion script) --------------------------------------
def cyl(r, z0, z1, x=0.0, y=0.0):
    return bd.Pos(x, y, z0) * bd.Cylinder(r, z1 - z0, align=(bd.Align.CENTER, bd.Align.CENTER, bd.Align.MIN))


def cone(r0, z0, r1, z1, x=0.0, y=0.0):
    return bd.Pos(x, y, z0) * bd.Cone(r0, r1, z1 - z0, align=(bd.Align.CENTER, bd.Align.CENTER, bd.Align.MIN))


def box(cx, cy, cz, lx, ly, lz):
    return bd.Pos(cx, cy, cz) * bd.Box(lx, ly, lz)


def radial_box(r_center, deg, z, radial, tangential, height):
    return bd.Rot(0, 0, deg) * bd.Pos(r_center, 0, z) * bd.Box(radial, tangential, height)


def radial_hole(deg, z, d, r0, r1):
    return bd.Rot(0, 0, deg) * bd.Pos(r0, 0, z) * bd.Rot(0, 90, 0) * cyl(d / 2, 0, r1 - r0)


def polar(r, deg):
    a = math.radians(deg)
    return r * math.cos(a), r * math.sin(a)


def fillet_rim(body, r_edge, z_edge, radius):
    edges = [
        e
        for e in body.edges().filter_by(bd.GeomType.CIRCLE)
        if abs(e.radius - r_edge) < 1e-3 and abs(e.arc_center.Z - z_edge) < 1e-3 and abs(e.arc_center.X) < 1e-3 and abs(e.arc_center.Y) < 1e-3
    ]
    assert edges, f"no rim edge at r={r_edge} z={z_edge}"
    return body.fillet(radius, edges)


# ---- parts ----------------------------------------------------------------------------------
def bezel_body():
    b = cyl(R_OUT, SPLIT_Z, TOP_Z) - cyl(R_IN, SPLIT_Z - 1, GLASS_TOP) - cyl(GLASS_R - LIP_W, GLASS_TOP - 1, TOP_Z + 1)
    for deg in USB.values():
        b -= radial_box(R_OUT, deg, USB_Z, CUT_DEPTH, *USB_OPEN)
    ang, z, length, height = SWITCH
    b -= radial_box(R_OUT, ang, z, CUT_DEPTH, length, height)
    for deg in BUTTONS.values():
        b -= radial_hole(deg, -2.85, 1.6, R_IN - 1, R_OUT + 1)
    for deg in LEDS.values():
        b -= radial_hole(deg, LED_Z, 2.0, R_IN - 1, R_OUT + 1)
    for deg in POST_ANGLES:  # M2 clearance + counterbore for a pan head
        b -= radial_hole(deg, RADIAL_SCREW_Z, 2.3, R_IN - 1, R_OUT + 1)
        b -= radial_hole(deg, RADIAL_SCREW_Z, 4.2, HEAD_R, R_OUT + 1)
    b = fillet_rim(b, R_OUT, TOP_Z, EDGE_FILLET_TOP)
    b.label, b.color = "bezel", "#e8e4dc"
    return b


def plate_body():
    p = cyl(PLATE_R, PLATE_BOT, STANDOFF_Z)
    for deg in POST_ANGLES:
        p += radial_box((POST_R0 + PLATE_R) / 2, deg, (FLOOR_TOP + PLATE_BOT) / 2 + 0.05, PLATE_R - POST_R0, POST_W, PLATE_BOT - FLOOR_TOP + 0.1)
    p &= cyl(PLATE_R, FLOOR_TOP, STANDOFF_Z)  # trim posts to the round wall
    for x, y in STANDOFFS:  # M2 countersunk from below
        p -= cyl(1.15, PLATE_BOT - 1, STANDOFF_Z + 1, x, y)
        p -= cone(2.1, PLATE_BOT - 0.01, 1.0, PLATE_BOT + 1.1, x, y)
    for deg in POST_ANGLES:
        p -= cyl(PILOT_D / 2, FLOOR_TOP - 1, PLATE_BOT - 1.0, *polar(POST_RC, deg))  # lid screw pilot
        p -= radial_hole(deg, RADIAL_SCREW_Z, PILOT_D, PLATE_R - 5.0, PLATE_R + 1)  # bezel screw pilot
    p -= box(J1[0], J1[1] - 2.0, STANDOFF_Z - 1, 12.0, 11.0, 4.0)  # cell lead to the battery socket
    for deg in USB.values():  # room for the USB-C overmold below the port
        p -= radial_box(PLATE_R, deg, USB_Z, 3.0, USB_OPEN[0], USB_OPEN[1])
    for dx, dy in BUZZER_HOLES:
        p -= cyl(0.75, PLATE_BOT - 1, STANDOFF_Z + 1, BUZZER[0] + dx, BUZZER[1] + dy)
    p.label, p.color = "plate", "#5a7fa8"
    return p


def lid_body():
    lid = cyl(R_OUT, BOT_Z, SPLIT_Z) - cyl(R_IN, FLOOR_TOP, SPLIT_Z + 1)
    fence = box(0.0, CELL_Y0, FLOOR_TOP + 2.0, CELL[0] + 2.4, CELL[1] + 2.4, 4.0) - box(0.0, CELL_Y0, FLOOR_TOP + 2.0, CELL[0], CELL[1], 5.0)
    for sx in (-1, 1):  # finger gaps in the fence so the cell lifts out
        fence -= box(sx * (CELL[0] / 2 + 1.2), CELL_Y0, FLOOR_TOP + 3.0, 3.0, 14.0, 4.1)
    lid += fence
    for deg in POST_ANGLES:  # M2 clearance + counterbore from below
        x, y = polar(POST_RC, deg)
        lid -= cyl(1.15, BOT_Z - 1, FLOOR_TOP + 1, x, y)
        lid -= cyl(2.1, BOT_Z - 1, BOT_Z + 1.4, x, y)
    for dx, dy in BUZZER_HOLES:
        lid -= cyl(0.75, BOT_Z - 1, FLOOR_TOP + 1, BUZZER[0] + dx, BUZZER[1] + dy)
    lid = fillet_rim(lid, R_OUT, BOT_Z, EDGE_FILLET_BOT)
    lid.label, lid.color = "lid", "#e8e4dc"
    return lid


def board_ref():
    g = cyl(GLASS_R, 1.2, GLASS_TOP) + cyl(35.5, -2.1, 1.2)  # glass + PCB/LCD module envelope
    for x, y in STANDOFFS:
        g += cyl(1.75, STANDOFF_Z, -2.1, x, y)
    for deg in USB.values():
        g += radial_box(33.0, deg, USB_Z, 7.6, 8.94, 3.26)
    g += radial_box(31.0, SWITCH[0], -2.75, 7.5, 9.3, 2.7)
    g += box(J1[0], J1[1], -3.7, 7.6, 5.2, 3.4)
    g.label, g.color = "board_ref", "#2b2b2b"
    return g


def cell_ref():
    c = box(0.0, CELL_Y0, CELL_TOP - 10.3 / 2 - 0.15, 34.5, 56.0, 10.3)
    c.label, c.color = "cell_ref", "#c9a227"
    return c


@model
def bezel():
    return bezel_body()


@model
def plate():
    return plate_body()


@model
def lid():
    return lid_body()


@model
def puck():
    parts = [bezel().shape, plate().shape, lid().shape, board_ref(), cell_ref()]
    for i, a in enumerate(parts):  # nothing interferes: the same check the Fusion script printed
        for b in parts[i + 1 :]:
            v = overlap_volume(a, b)
            assert v < 1e-3, f"{a.label} x {b.label} overlap {v:.3f} mm3"
    bb = parts[0].bounding_box().size
    assert abs(bb.X - 2 * R_OUT) < 0.01, bb  # outer diameter is what the user asked for
    return bd.Compound(children=parts, label="puck")


if __name__ == "__main__":
    puck()
