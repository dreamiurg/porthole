"""Porthole puck case v1.4 as a build123d model (text-to-cad / cadgen). Port of puck_case_v1_fusion.py.

Run: `cadgen +0.8.4 run hardware/case/puck_v1.py` builds Bezel, Plate, Lid, the screw test and the
assembly with reference bodies for the board, the cell and the magnet ring, and opens them in the
CAD viewer. Millimetres in the board's STEP frame: origin = board center, +Z out of the screen,
glass top at z = +3.4.

v1.1 after the first print: M2 pilots 1.6 -> 1.8 (screws drove too tight), wall 2.0 -> 4.2 for a
fatter puck (dia 79.6 -> 84). The radial screw counterbore deepens with the wall so the same
screws still reach the posts; side cutouts span the thicker wall.

v1.2: the lid gets a V groove and a key notch so it clicks into the snap-on grip v3 (its model,
grip_v1.py, was removed with v1.3) in one orientation only, keeping the tilt axes square to the grips.
Bezel and plate are unchanged from v1.1.

v1.3 after the v1.2 print: the lid screws pulled through, because the counterbore left 0.2 mm of
floor under the head (and a tapered head wedged it apart). Every case screw is now one M2x6
hex-socket cap head self-tapper (flat head ~3.5 x 1.8): it sits in a flat seat with SEAT_UNDER of
solid plastic under it, on a boss in the lid and behind a deeper seat in the bezel wall. The lid
floor also carries a MagSafe magnet ring (RING_OD x RING_ID x RING_T) in a groove under the cell,
so the puck sticks to the steel ring in grip v7; the cell holds it down. The buzzer holes in the
lid move outward to clear the ring. The v1.2 snap groove and key notch are gone: the grip now
holds the puck by the magnet.

v1.4: the board's PCB has a flat at x = 33 on the switch side (from the STEP), so the power slider
becomes a straight block that rides on that flat inside and on a flat-bottomed groove in the bezel
wall outside, with a ledge under the switch body and a fin 3 mm proud. The groove passed over the
350 degree screw seat, so that plate post moves to 335.
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
RING_OD, RING_ID, RING_T = 56.0, 46.0, 1.5  # N52 MagSafe magnet ring as sold (56 x 46 x 1.5 mm)
RING_FIT = 0.2  # radial clearance each side of the ring groove
RING_FLOOR = 0.8  # lid floor under the ring: 4 layers between the magnet and the grip
FLOOR_T = RING_FLOOR + RING_T + 0.1  # 2.4, was 1.6; the ring top sits flush with the floor
FLOOR_TOP = CELL_TOP - CELL[2]  # -18.6
BOT_Z = FLOOR_TOP - FLOOR_T  # -21.0
SPLIT_Z = -12.0  # bezel / lid seam
EDGE_FILLET_TOP = 2.5
EDGE_FILLET_BOT = 3.0
POST_ANGLES = (90.0, 190.0, 335.0)  # plate posts: bezel radial screws + lid axial screws; 350 sat on the slider groove
POST_W = 6.0
POST_R0 = PLATE_R - 6.0
POST_RC = (POST_R0 + PLATE_R) / 2  # lid screw axis radius
RADIAL_SCREW_Z = -9.6
PILOT_D = 1.8  # M2 self-tapping pilot; 1.6 was too tight in the first print
CLEAR_D = 2.3  # M2 clearance
SEAT_D, SEAT_H = 4.0, 2.0  # flat seat for the M2 cap head (3.5 x 1.8)
SEAT_UNDER = 2.0  # solid plastic under every head; v1.2 had 0.2 under the lid screws
BOSS_TOP = BOT_Z + SEAT_H + SEAT_UNDER  # -17.0: lid boss top = plate post bottom
CUT_DEPTH = 2 * (WALL + 2.0)  # radial length of side cutouts centered on R_OUT: reach r = R_IN - 2
HEAD_R = R_IN + SEAT_UNDER  # radial screw head seat: SEAT_UNDER of wall under the head
STANDOFFS = [(29.5, 13.6), (-29.5, 13.6), (18.0, -27.05), (-18.0, -27.05)]
USB = {"USB (native)": -43.45, "UART": -136.4}  # receptacle axis angles, mouth at r = 36.8
USB_Z = -3.27
USB_OPEN = (13.0, 7.5)  # tangential x height, fits a standard USB-C overmold
# Power switch SW1 (from the v2 Fusion script's reading of the STEP): body and lever in board xy.
SW_BODY = (29.4, 32.5, 9.0, -4.1, -1.4)  # x from, x to, length along y, z from, z to
LEVER = (34.8, 1.5, 0.7, -3.1)  # tip x, width (y), height (z), centre z; it slides along y
LEVER_YC = -7.3  # lever travel centre = switch body centre
SW_ANG = math.degrees(math.atan2(LEVER_YC, LEVER[0]))  # -11.85
# The PCB has a flat at |x| = 33 (switch side and opposite) and y = 34, per the STEP; the acrylic
# spacer between PCB and glass sits ~0.5 inside the PCB edge (photos of the real board).
PCB_FLAT_X = 33.0
PCB_Y_MAX = 34.0
SPACER_INSET = 0.5
# Power slider: a straight block between the PCB's flat and a flat-bottomed groove in the bezel wall,
# held by the glass above and the plate below. Its inner face rides on the PCB's flat edge, its outer
# face on the groove bottom (parallel to the flat); the groove is open down to the seam, because the
# slider goes in from the back after the board. The lever, which
# sticks out past the flat, sits in a notch open to the top (the slider goes in from the back after
# the board). A fin through a slot in the wall is the knob. The slot allows SLIDER_SLOT_TRAVEL each
# way; each test slider's fin width sets its own travel, because the switch's real travel is not in the STEP.
SLIDER_X0 = PCB_FLAT_X + 0.2  # inner face: 0.2 off the PCB flat
SLIDER_Z = (STANDOFF_Z + 0.5, 1.0)  # 0.5 over the plate, 0.2 under the glass
SLIDER_LEN = 8.0  # along y
POCKET_X = 38.5  # groove bottom, parallel to the PCB flat; leaves >= 1.4 of wall at the groove ends
POCKET_FIT = 0.2  # slider outer face off the groove bottom
OVERTRAVEL = 0.2  # the groove runs this past the travel too, so only the fin stops the slider
NOTCH_BACK = 0.6  # notch depth past the lever tip; more than the 0.2 off the flat, so the lever never bottoms
FIN_R0 = 36.0  # the fin starts inside the block
LEDGE_X0 = SW_BODY[0] + 0.5  # the slider's bottom ledge reaches in under the switch body...
LEDGE_TOP = SW_BODY[3] - 0.2  # ...and rides 0.2 under its back face
FIN_Z = (SLIDER_Z[0], -1.6)  # the knob fin runs from the slider bottom, so it prints on the bed
FIN_PROUD = 3.0  # how far the fin stands out of the wall: enough to grip (1.2 was too short)
SLIDER_SLOT_TRAVEL = 1.5
SLIDER_SLOT_W = 6.4  # slot width at the wall surface; the fin is narrower by the travel each way
SLOT_FIT = 0.2  # slot clearance above and below the fin
SLIDER_GAPS = (0.25, 0.45)  # lever clearance each side, per test slider
SLIDER_TRAVELS = (1.0, 1.5)  # travel each way, per test slider
BUTTONS = {"BOOT": 13.5, "RESET": 0.9}  # angles; plungers at r = 34, z = -2.85
LEDS = {"CHG": -23.3, "PWR": -30.3}
LED_Z = -2.85  # window center; LEDs sit on the back of the PCB (z -2.1 down), -4.4 in v1 missed them
BUZZER = (-27.5, -12.3)  # from Waveshare's labelled photo; not in the STEP by name - verify
BUZZER_HOLES = [(0, 0), (2.2, 0), (-2.2, 0), (0, 2.2), (0, -2.2)]
LID_BUZZER_R = 32.5  # the lid's set of buzzer holes sits further out, past the ring groove
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


def ring_groove():
    """Annular groove for the magnet ring, open to the top of the lid floor and up through the fence."""
    return cyl(RING_OD / 2 + RING_FIT, FLOOR_TOP - RING_T - 0.1, FLOOR_TOP + 5) - cyl(RING_ID / 2 - RING_FIT, FLOOR_TOP - 6, FLOOR_TOP + 6)


def lid_buzzer_holes():
    deg = math.degrees(math.atan2(BUZZER[1], BUZZER[0]))
    cx, cy = polar(LID_BUZZER_R, deg)
    holes = [(cx + dx, cy + dy) for dx, dy in BUZZER_HOLES]
    for x, y in holes:  # 0.8 mm of floor between every hole and the ring groove
        assert math.hypot(x, y) - 0.75 >= RING_OD / 2 + RING_FIT + 0.8, (x, y)
    return holes


def sector(r0, r1, a0, a1, z0, z1):
    """Annular sector between radii r0..r1 and angles a0..a1 (degrees, a1 - a0 < 180)."""
    big = r1 * 2
    pts = [(0, 0)] + [polar(big, a0 + (a1 - a0) * i / 8) for i in range(9)]
    wedge = bd.Pos(0, 0, z0) * bd.extrude(bd.Polygon(*pts, align=None), z1 - z0)
    return (cyl(r1, z0, z1) - cyl(r0, z0 - 1, z1 + 1)) & wedge


def arc_deg(length, r):
    return math.degrees(length / r)


def slider_slot():
    """Straight along y, like the slider's travel."""
    z0, z1 = FIN_Z[0] - SLOT_FIT, FIN_Z[1] + SLOT_FIT
    return box((R_IN - 2 + R_OUT + 2) / 2, LEVER_YC, (z0 + z1) / 2, R_OUT - R_IN + 4, SLIDER_SLOT_W, z1 - z0)


def switch_ref(lever_dy=0.0):
    x0, x1, length, z0, z1 = SW_BODY
    tip, w, h, zc = LEVER
    s = box((x0 + x1) / 2, LEVER_YC, (z0 + z1) / 2, x1 - x0, length, z1 - z0)
    return s + box((x1 + tip) / 2, LEVER_YC + lever_dy, zc, tip - x1, w, h)


def slider_body(gap, travel, marks=0):
    """Power slider at the middle of its travel, lever in the notch."""
    z0, z1 = SLIDER_Z
    x1 = POCKET_X - POCKET_FIT
    s = box((SLIDER_X0 + x1) / 2, LEVER_YC, (z0 + z1) / 2, x1 - SLIDER_X0, SLIDER_LEN, z1 - z0)
    s += box((LEDGE_X0 + SLIDER_X0 + 0.5) / 2, LEVER_YC, (z0 + LEDGE_TOP) / 2, SLIDER_X0 + 0.5 - LEDGE_X0, SLIDER_LEN, LEDGE_TOP - z0)
    fz0, fz1 = FIN_Z
    fin = box((FIN_R0 + R_OUT + FIN_PROUD) / 2, LEVER_YC, (fz0 + fz1) / 2, R_OUT + FIN_PROUD - FIN_R0, SLIDER_SLOT_W - 2 * travel, fz1 - fz0)
    s += fin & cyl(R_OUT + FIN_PROUD, fz0 - 1, fz1 + 1)
    tip, w, h, zc = LEVER
    nz0 = zc - h / 2 - 0.3
    s -= box((SLIDER_X0 - 1 + tip + NOTCH_BACK) / 2, LEVER_YC, (nz0 + z1 + 1) / 2, tip + NOTCH_BACK - SLIDER_X0 + 1, w + 2 * gap, z1 + 1 - nz0)
    for i in range(marks):  # dimples on the top face tell the variants apart, either side of the notch
        side = 1 if i % 2 else -1
        s -= cyl(0.4, z1 - 0.4, z1 + 1, SLIDER_X0 + 0.8, LEVER_YC + side * (SLIDER_LEN / 2 - 1.0 - (i // 2) * 1.2))
    return s


def slider_pocket():
    """Flat-bottomed groove for the slider, the length of its longest travel each way; open down to the seam."""
    length = SLIDER_LEN + 2 * (max(SLIDER_TRAVELS) + OVERTRAVEL + POCKET_FIT)
    z0, z1 = SPLIT_Z - 1, SLIDER_Z[1] + SLOT_FIT
    return box((SLIDER_X0 + POCKET_X) / 2, LEVER_YC, (z0 + z1) / 2, POCKET_X - SLIDER_X0, length, z1 - z0)


def at_travel(shape, off):
    """A slider moved `off` mm along its travel (straight along y, like the lever)."""
    return bd.Pos(0, off, 0) * shape


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
    b -= slider_slot()
    b -= slider_pocket()
    for deg in BUTTONS.values():
        b -= radial_hole(deg, -2.85, 1.6, R_IN - 1, R_OUT + 1)
    for deg in LEDS.values():
        b -= radial_hole(deg, LED_Z, 2.0, R_IN - 1, R_OUT + 1)
    for deg in POST_ANGLES:  # M2 clearance + flat seat for the cap head
        b -= radial_hole(deg, RADIAL_SCREW_Z, CLEAR_D, R_IN - 1, R_OUT + 1)
        b -= radial_hole(deg, RADIAL_SCREW_Z, SEAT_D, HEAD_R, R_OUT + 1)
    b = fillet_rim(b, R_OUT, TOP_Z, EDGE_FILLET_TOP)
    b.label, b.color = "bezel", "#e8e4dc"
    return b


def plate_body():
    p = cyl(PLATE_R, PLATE_BOT, STANDOFF_Z)
    for deg in POST_ANGLES:
        p += radial_box(POST_RC, deg, (BOSS_TOP + PLATE_BOT) / 2 + 0.05, PLATE_R - POST_R0, POST_W, PLATE_BOT - BOSS_TOP + 0.1)
    p &= cyl(PLATE_R, BOSS_TOP, STANDOFF_Z)  # trim posts to the round wall; they stand on the lid bosses
    for x, y in STANDOFFS:  # M2 countersunk from below
        p -= cyl(1.15, PLATE_BOT - 1, STANDOFF_Z + 1, x, y)
        p -= cone(2.1, PLATE_BOT - 0.01, 1.0, PLATE_BOT + 1.1, x, y)
    for deg in POST_ANGLES:
        p -= cyl(PILOT_D / 2, BOSS_TOP - 1, PLATE_BOT - 1.0, *polar(POST_RC, deg))  # lid screw pilot
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
    for deg in POST_ANGLES:  # bosses under the plate posts carry the screw heads
        lid += radial_box(POST_RC, deg, (FLOOR_TOP + BOSS_TOP) / 2, PLATE_R - POST_R0 + 0.4, POST_W, BOSS_TOP - FLOOR_TOP + 0.1) & cyl(R_IN + 0.1, FLOOR_TOP - 0.1, BOSS_TOP)
    lid -= ring_groove()  # also gaps the cell fence where the ring passes under it, so the ring drops in
    for deg in POST_ANGLES:  # M2 clearance + flat seat from below
        x, y = polar(POST_RC, deg)
        lid -= cyl(CLEAR_D / 2, BOT_Z - 1, BOSS_TOP + 1, x, y)
        lid -= cyl(SEAT_D / 2, BOT_Z - 1, BOT_Z + SEAT_H, x, y)
    for x, y in lid_buzzer_holes():
        lid -= cyl(0.75, BOT_Z - 1, FLOOR_TOP + 1, x, y)
    lid = fillet_rim(lid, R_OUT, BOT_Z, EDGE_FILLET_BOT)
    lid.label, lid.color = "lid", "#e8e4dc"
    return lid


def board_ref():
    flat = box(0, (PCB_Y_MAX - 40) / 2, 0, 2 * PCB_FLAT_X, PCB_Y_MAX + 40, 20)
    spacer = box(0, (PCB_Y_MAX - 40) / 2, 0, 2 * (PCB_FLAT_X - SPACER_INSET), PCB_Y_MAX + 40, 20)
    g = cyl(GLASS_R, 1.2, GLASS_TOP) + (cyl(35.5, -2.1, -0.5) & flat) + (cyl(35.5, -0.5, 1.2) & spacer)  # glass, PCB, spacer
    for x, y in STANDOFFS:
        g += cyl(1.75, STANDOFF_Z, -2.1, x, y)
    for deg in USB.values():
        g += radial_box(33.0, deg, USB_Z, 7.6, 8.94, 3.26)
    g += box(J1[0], J1[1], -3.7, 7.6, 5.2, 3.4)
    g.label, g.color = "board_ref", "#2b2b2b"
    return g  # the switch is added per check by switch_ref(), so its lever can move


def ring_ref():
    r = cyl(RING_OD / 2, FLOOR_TOP - RING_T, FLOOR_TOP) - cyl(RING_ID / 2, FLOOR_TOP - RING_T - 1, FLOOR_TOP + 1)
    r.label, r.color = "ring_ref", "#9aa0a6"
    return r


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


TEST_PILOTS = (1.6, 1.7, 1.8)  # pilot sizes on the screw test block, left to right


def screw_test_body():
    """A post block with three pilots and a strip with three lid seats: one M2x6 per pair, before the case."""
    pitch, w = 10.0, POST_W
    xs = [(i - 1) * pitch for i in range(len(TEST_PILOTS))]
    post = box(0, 0, 4.0, 3 * pitch, w, 8.0) - box(-1.5 * pitch, w / 2, 8.0, 2.0, 2.0, 2.0)  # notch marks the 1.6 end
    strip = box(0, w + 4.0, (SEAT_H + SEAT_UNDER) / 2, 3 * pitch, w, SEAT_H + SEAT_UNDER)
    for x, d in zip(xs, TEST_PILOTS, strict=True):
        post -= cyl(d / 2, 1.0, 8.1, x, 0)
        strip -= cyl(CLEAR_D / 2, -1, SEAT_H + SEAT_UNDER + 1, x, w + 4.0)
        strip -= cyl(SEAT_D / 2, -1, SEAT_H, x, w + 4.0)  # seat on the bed side, as on the lid
    post.label, strip.label = "test_post", "test_strip"
    return bd.Compound(children=[post, strip], label="screw_test")


@model
def screw_test():
    return screw_test_body()


def slider_checks(bez, pl, board):
    """Every test slider clears the case and the board at both ends of its travel, and pushes the
    lever there (the lever, moved with it, still sits in the notch)."""
    for gap in SLIDER_GAPS:
        for travel in SLIDER_TRAVELS:
            for off in (-travel, 0.0, travel):
                sl = at_travel(slider_body(gap, travel), off)
                for other, name in ((bez, "bezel"), (pl, "plate"), (board, "board"), (switch_ref(off), "switch")):
                    v = overlap_volume(sl, other)
                    assert v < 1e-3, f"slider gap {gap} travel {travel} at {off:+}: hits {name} ({v:.3f} mm3)"
            # past the travel the fin hits the slot end: the slider cannot go further
            assert overlap_volume(at_travel(slider_body(gap, travel), travel + 0.15), bez) > 1e-3, (gap, travel)


@model
def slider():
    s = slider_body(SLIDER_GAPS[0], SLIDER_TRAVELS[0])
    s.label, s.color = "slider", "#d9534f"
    return s


@model
def slider_test():
    """Four test sliders, gap x travel, marked by 1-4 dimples on top, laid out to print standing."""
    cx, cy = (SLIDER_X0 + R_IN) / 2, LEVER_YC
    parts = []
    for i, (gap, travel) in enumerate((g, t) for g in SLIDER_GAPS for t in SLIDER_TRAVELS):
        sl = bd.Pos(-cx, -cy + 12.0 * i, -SLIDER_Z[0]) * slider_body(gap, travel, marks=i + 1)
        sl.label = f"slider_gap{gap}_travel{travel}"
        parts.append(sl)
    return bd.Compound(children=parts, label="slider_test")


@model
def switch_section():
    """The bezel around the switch, to try the test sliders on the real board; prints lip down."""
    piece = bezel_body() & sector(0, R_OUT + 2, SW_ANG - 40, SW_ANG + 40, SPLIT_Z - 1, TOP_Z + 1)
    piece = bd.Pos(0, 0, TOP_Z) * bd.Rot(180, 0, 0) * piece
    piece.label = "switch_section"
    return piece


@model
def slider_view():
    """The slider in place with the bezel, plate and board cut to 80° around the switch, for review."""
    cut = sector(0, R_OUT + 2, SW_ANG - 40, SW_ANG + 40, -50, 50)
    board = board_ref() + switch_ref()
    parts = []
    for shape, label, color in (
        (bezel_body(), "bezel", "#e8e4dc"),
        (plate_body(), "plate", "#5b8fd9"),
        (board, "board_ref", "#2b2b2b"),
    ):
        piece = shape & cut
        piece.label, piece.color = label, color
        parts.append(piece)
    parts.append(slider().shape)
    return bd.Compound(children=parts, label="slider_view")


@model
def puck():
    bez, pl = bezel().shape, plate().shape
    slider_checks(bez, pl, board_ref())
    board = board_ref() + switch_ref()
    board.label, board.color = "board_ref", "#2b2b2b"
    parts = [bez, pl, lid().shape, board, cell_ref(), ring_ref(), slider().shape]
    for i, a in enumerate(parts):  # nothing interferes: the same check the Fusion script printed
        for b in parts[i + 1 :]:
            v = overlap_volume(a, b)
            assert v < 1e-3, f"{a.label} x {b.label} overlap {v:.3f} mm3"
    bb = parts[0].bounding_box().size
    assert abs(bb.X - 2 * R_OUT) < 0.01, bb  # outer diameter is what the user asked for
    return bd.Compound(children=parts, label="puck")


if __name__ == "__main__":
    puck()
    screw_test()
    slider_test()
    switch_section()
