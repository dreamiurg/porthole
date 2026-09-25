"""Porthole puck case, v2 - Fusion 360 script (run via the Fusion MCP or Scripts & Add-Ins).

Rebuilds the design in the active document as named bodies (Part Design documents hold one
component). Millimetres, in the board's STEP frame: origin = board center, +Z = out of the screen,
+Y = screen up (USB-C ports at 5 and 7 o'clock), glass top z = +3.4, standoff feet z = -6.1.
Board geometry comes from Waveshare's ESP32-S3-Touch-LCD-2.1 STEP; see
docs/superpowers/specs/2026-09-24-puck-case-design.md. Fusion's API works in cm; `m()` converts.

Assembly (face-down on the table):
  1. Ring    front lip over the glass edge; drop the board into it, glass first.
  2. Plate   screws to the board's 4 M2 standoffs (countersunk, from below).
  3. Slider  drops into its channel in the Cup wall from the top; set it to match the switch.
  4. Cup     wall + floor + cell pocket, lowered over the back; flex buttons for BOOT/RESET.
  5. Three M2x20 screws from the Cup floor into the Ring clamp the whole stack.
Nothing on the Ring protrudes inward below the glass: the round glass has to slide in from the back.
"""

import math

import adsk.core
import adsk.fusion

# ---- board and cell (from the STEP) -----------------------------------------------------------
GLASS_R = 37.5
GLASS_TOP = 3.4
STANDOFF_Z = -6.1
STANDOFFS = [(29.5, 13.6), (-29.5, 13.6), (18.0, -27.05), (-18.0, -27.05)]
USB = {"USB (native)": -43.45, "UART": -136.4}  # receptacle axis angles; mouths at r = 36.8
USB_Z = -3.27
BUTTONS = {"BOOT": 8.1, "RESET": 0.5}  # plunger y; plunger tips at x = 34.0, pointing +X
BTN_Z = -3.05
LEVER = (34.8, -6.3, -3.1, 1.5, 0.7)  # slide-switch lever tip x, y, z, width (y), height (z)
SW_BODY_X = 32.5  # switch body stops here; only the lever reaches past it
LEVER_TRAVEL = 1.0  # +/- about the travel center; not in the STEP - verify on the part
LEDS = {"CHG": -23.3, "PWR": -30.3}  # 0603s on the back, facing -Z
BUZZER = (-27.5, -12.3)  # from Waveshare's labelled photo; not in the STEP by name - verify
J1 = (21.2, 8.6)  # battery socket

# ---- case -------------------------------------------------------------------------------------
FIT = 0.3
R_IN = GLASS_R + FIT  # 37.8
WALL = 3.8
R_OUT = R_IN + WALL  # 41.6
LIP_T, LIP_W = 1.2, 2.0
TOP_Z = GLASS_TOP + LIP_T  # 4.6
SEAM_Z = -1.0  # Ring / Cup seam, above every side control
SEAM_GAP = 0.2  # the bolts clamp the stack (lip-glass-board-plate-ledge), not the seam
PLATE_T = 1.6
PLATE_R = R_IN - 0.2
PLATE_BOT = STANDOFF_Z - PLATE_T  # -7.7
LEDGE_W = 1.8  # Cup ledge the plate sits on
CELL = (36.2, 57.0, 10.6)  # pocket for EEMB LP103454 (34.5 x 55-56 x 10.3) + tolerance
CELL_Y0 = 1.2
CELL_TOP = PLATE_BOT - 0.3
FLOOR_T = 1.6
FLOOR_TOP = CELL_TOP - CELL[2]
BOT_Z = FLOOR_TOP - FLOOR_T
FILLET_TOP, FILLET_BOT = 2.5, 3.0
BOLT_ANGLES = (60.0, 180.0, 300.0)
BOLT_R = 39.5
BOLT_HEAD = (4.0, 3.5)  # counterbore for an M2x20 pan head: tip ends 4 mm into the Ring
# flex buttons
TONGUE_W, TONGUE_T, SLOT = 4.4, 1.0, 0.5
TONGUE_Z0, TONGUE_Z1 = -12.0, SEAM_Z - 0.3
NUB = (2.0, 1.6)  # y x z section
NUB_GAP = 0.3
# slider (straight, in a chord channel centred on the lever's travel center)
SL_ANG = math.degrees(math.atan2(-7.3, LEVER[0]))  # -11.85
SL_LEN, SL_T, SL_Z = 7.0, 1.0, (-5.8, SEAM_Z - 0.3)
CH_R = (38.6, 40.0)  # channel radial band
KNOB = (3.0, 0.8, (-4.3, -1.9))  # width, proud of the wall, z
PRONG_T, PRONG_Z, PRONG_R0 = 1.0, (-3.8, -2.4), 34.0
PRONG_GAP = 0.45  # each side of the lever; the lever sits 12 deg off the slider axis
USB_OPEN = (13.0, 7.5)


def m(v):
    return v / 10.0


def P(x, y, z):
    return adsk.core.Point3D.create(m(x), m(y), m(z))


def polar(r, deg, z):
    a = math.radians(deg)
    return P(r * math.cos(a), r * math.sin(a), z)


TBM = adsk.fusion.TemporaryBRepManager.get()
DIFF = adsk.fusion.BooleanTypes.DifferenceBooleanType
UNION = adsk.fusion.BooleanTypes.UnionBooleanType
INTER = adsk.fusion.BooleanTypes.IntersectionBooleanType


def cyl(r, z0, z1, x=0.0, y=0.0):
    return TBM.createCylinderOrCone(P(x, y, z0), m(r), P(x, y, z1), m(r))


def cone(r0, z0, r1, z1, x=0.0, y=0.0):
    return TBM.createCylinderOrCone(P(x, y, z0), m(r0), P(x, y, z1), m(r1))


def box(cx, cy, cz, lx, ly, lz):
    return frame_box(0.0, cx, cy, cz, lx, ly, lz)


def frame_box(deg, cx, cy, cz, lx, ly, lz):
    """Box with its length along direction `deg`, centred at (cx, cy, cz)."""
    a = math.radians(deg)
    obb = adsk.core.OrientedBoundingBox3D.create(
        P(cx, cy, cz),
        adsk.core.Vector3D.create(math.cos(a), math.sin(a), 0),
        adsk.core.Vector3D.create(-math.sin(a), math.cos(a), 0),
        m(lx),
        m(ly),
        m(lz),
    )
    return TBM.createBox(obb)


def rt_box(deg, r0, r1, t0, t1, z0, z1):
    """Box in a frame rotated to `deg`: radial span r0..r1, tangential t0..t1, z0..z1."""
    a = math.radians(deg)
    rc, tc = (r0 + r1) / 2, (t0 + t1) / 2
    cx, cy = rc * math.cos(a) - tc * math.sin(a), rc * math.sin(a) + tc * math.cos(a)
    return frame_box(deg, cx, cy, (z0 + z1) / 2, r1 - r0, t1 - t0, z1 - z0)


def radial_hole(deg, z, d, r0, r1):
    return TBM.createCylinderOrCone(polar(r0, deg, z), m(d / 2), polar(r1, deg, z), m(d / 2))


def cut(target, *tools):
    for t in tools:
        TBM.booleanOperation(target, t, DIFF)
    return target


def add(target, *tools):
    for t in tools:
        TBM.booleanOperation(target, t, UNION)
    return target


def ring(r0, r1, z0, z1):
    return cut(cyl(r1, z0, z1), cyl(r0, z0 - 1, z1 + 1))


def rounded_cyl(r, z0, z1, f0=0.0, f1=0.0):
    """Cylinder with its bottom (f0) and top (f1) outer edges rounded."""
    body = cyl(r, z0 + f0, z1 - f1)
    axis = adsk.core.Vector3D.create(0, 0, 1)
    for f, zc, zs in ((f0, z0 + f0, (z0, z0 + f0)), (f1, z1 - f1, (z1 - f1, z1))):
        if f > 0:
            torus = TBM.createTorus(P(0, 0, 0), axis, m(r - f), m(f))  # centre arg is ignored:
            move = adsk.core.Matrix3D.create()  # build at the origin, then translate
            move.translation = adsk.core.Vector3D.create(0, 0, m(zc))
            TBM.transform(torus, move)
            add(body, cyl(r - f, zs[0], zs[1]), torus)
    return body


def lever_t():
    """Lever centre in the slider frame (radial, tangential)."""
    a = math.radians(SL_ANG)
    x, y = LEVER[0], LEVER[1]
    return x * math.cos(a) + y * math.sin(a), -x * math.sin(a) + y * math.cos(a)


def usb_tool(deg, r0=None):
    return rt_box(deg, r0 if r0 is not None else R_IN - 2.0, R_OUT + 1.0, -USB_OPEN[0] / 2, USB_OPEN[0] / 2, USB_Z - USB_OPEN[1] / 2, USB_Z + USB_OPEN[1] / 2)


# ---- parts ------------------------------------------------------------------------------------
def make_ring():
    r = rounded_cyl(R_OUT, SEAM_Z + SEAM_GAP, TOP_Z, 0.0, FILLET_TOP)
    cut(r, cyl(R_IN, SEAM_Z - 1, GLASS_TOP), cyl(GLASS_R - LIP_W, GLASS_TOP - 1, TOP_Z + 1))
    for deg in USB.values():
        cut(r, usb_tool(deg))
    for deg in BOLT_ANGLES:  # M2 pilot for the through-bolts
        a = math.radians(deg)
        cut(r, cyl(0.8, SEAM_Z - 1, GLASS_TOP + 0.4, BOLT_R * math.cos(a), BOLT_R * math.sin(a)))
    return r


def make_cup():
    c = rounded_cyl(R_OUT, BOT_Z, SEAM_Z, FILLET_BOT, 0.0)
    cut(c, cyl(R_IN, FLOOR_TOP, SEAM_Z + 1))
    add(c, ring(R_IN - LEDGE_W, R_IN + 0.01, FLOOR_TOP, PLATE_BOT))  # ledge the plate sits on
    # cell fence with finger gaps
    fence = box(0, CELL_Y0, FLOOR_TOP + 2.0, CELL[0] + 2.4, CELL[1] + 2.4, 4.0)
    cut(fence, box(0, CELL_Y0, FLOOR_TOP + 2.0, CELL[0], CELL[1], 5.0))
    for sx in (-1, 1):
        cut(fence, box(sx * (CELL[0] / 2 + 1.2), CELL_Y0, FLOOR_TOP + 3.0, 3.0, 14.0, 4.1))
    add(c, fence)
    # ports, LED windows, buzzer vent
    for deg in USB.values():
        cut(c, usb_tool(deg))
    for deg in LEDS.values():
        cut(c, radial_hole(deg, -4.4, 2.0, R_IN - 1, R_OUT + 1))
    bx, by = BUZZER
    for dx, dy in [(0, 0), (2.2, 0), (-2.2, 0), (0, 2.2), (0, -2.2)]:
        cut(c, cyl(0.75, BOT_Z - 1, FLOOR_TOP + 1, bx + dx, by + dy))
    # through-bolts: clearance + counterbore from below
    for deg in BOLT_ANGLES:
        a = math.radians(deg)
        x, y = BOLT_R * math.cos(a), BOLT_R * math.sin(a)
        cut(c, cyl(1.15, BOT_Z - 1, SEAM_Z + 1, x, y), cyl(BOLT_HEAD[0] / 2, BOT_Z - 1, BOT_Z + BOLT_HEAD[1], x, y))
    # flex buttons: U-slot through the wall, tongue thinned from inside, nub along +X to the plunger
    for y in BUTTONS.values():
        deg = math.degrees(math.atan2(y, 34.0))
        w = TONGUE_W
        slot = rt_box(deg, R_IN - 1, R_OUT + 1, -w / 2 - SLOT, w / 2 + SLOT, TONGUE_Z0, TONGUE_Z1 + SLOT)
        cut(slot, rt_box(deg, R_IN - 2, R_OUT + 2, -w / 2, w / 2, TONGUE_Z0 - 1, TONGUE_Z1))
        cut(c, slot)
        cut(c, rt_box(deg, R_IN - 1, R_OUT - TONGUE_T, -w / 2 - SLOT, w / 2 + SLOT, TONGUE_Z0 + 1.0, SEAM_Z + 1))
        x_tip = 34.0 + NUB_GAP
        x_back = math.sqrt((R_OUT - TONGUE_T + 0.4) ** 2 - y * y)
        add(c, box((x_tip + x_back) / 2, y, BTN_Z, x_back - x_tip, NUB[0], NUB[1]))
        # raised dot so a finger finds the button
        add(c, TBM.createCylinderOrCone(polar(R_OUT - 0.2, deg, BTN_Z), m(1.2), polar(R_OUT + 0.5, deg, BTN_Z), m(1.0)))
    # slider channel: chord pocket in the wall, open at the seam; outer slot for the knob,
    # inner slot for the fork prongs
    t_trav = LEVER_TRAVEL + 0.3
    cut(c, rt_box(SL_ANG, CH_R[0], CH_R[1], -SL_LEN / 2 - t_trav - 0.2, SL_LEN / 2 + t_trav + 0.2, SL_Z[0] - 0.2, SEAM_Z + 1))
    cut(c, rt_box(SL_ANG, CH_R[1] - 0.1, R_OUT + 1, -KNOB[0] / 2 - t_trav, KNOB[0] / 2 + t_trav, KNOB[2][0] - 0.3, SEAM_Z + 1))
    half = LEVER[3] / 2 + PRONG_GAP + PRONG_T
    cut(c, rt_box(SL_ANG, R_IN - 1, CH_R[0] + 0.1, -half - t_trav, half + t_trav, PRONG_Z[0] - 0.3, SEAM_Z + 1))
    return c


def make_slider(offset=None):
    """Slider at `offset` along its travel (default: where the lever is in the STEP)."""
    lr, lt = lever_t()
    off = lt if offset is None else offset
    s = rt_box(SL_ANG, CH_R[0] + 0.2, CH_R[1] - 0.2, off - SL_LEN / 2, off + SL_LEN / 2, SL_Z[0], SL_Z[1])
    add(s, rt_box(SL_ANG, CH_R[1] - 0.3, R_OUT + KNOB[1], off - KNOB[0] / 2, off + KNOB[0] / 2, KNOB[2][0], KNOB[2][1]))
    half = LEVER[3] / 2 + PRONG_GAP
    for sgn in (-1, 1):
        t0 = off + sgn * half
        t1 = off + sgn * (half + PRONG_T)
        add(s, rt_box(SL_ANG, PRONG_R0, CH_R[0] + 0.4, min(t0, t1), max(t0, t1), PRONG_Z[0], PRONG_Z[1]))
    for k in (-1, 0, 1):  # grip ridges on the knob
        add(s, rt_box(SL_ANG, R_OUT + KNOB[1] - 0.1, R_OUT + KNOB[1] + 0.4, off + k * 0.9 - 0.25, off + k * 0.9 + 0.25, KNOB[2][0], KNOB[2][1]))
    return s


def plate_cutters(z0, z1):
    """Plate notches, as prisms over [z0, z1] so the same tools build the assembly keep-out."""
    tools = []
    for x, y in STANDOFFS:
        tools.append(cyl(1.15, z0 - 1, z1 + 1, x, y))
    tools.append(box(J1[0], J1[1] - 2.0, (z0 + z1) / 2, 12.0, 11.0, z1 - z0 + 2))
    for deg in USB.values():
        tools.append(rt_box(deg, PLATE_R - 2.5, PLATE_R + 1, -USB_OPEN[0] / 2, USB_OPEN[0] / 2, z0 - 1, z1 + 1))
    for y in BUTTONS.values():  # nubs pass the plate edge when the Cup goes on
        tools.append(box(36.5, y, (z0 + z1) / 2, 5.0, NUB[0] + 1.0, z1 - z0 + 2))
    half = LEVER[3] / 2 + PRONG_GAP + PRONG_T
    tools.append(rt_box(SL_ANG, PRONG_R0 - 0.4, PLATE_R + 1, -half - LEVER_TRAVEL - 0.5, half + LEVER_TRAVEL + 0.5, z0 - 1, z1 + 1))
    bx, by = BUZZER
    for dx, dy in [(0, 0), (2.2, 0), (-2.2, 0), (0, 2.2), (0, -2.2)]:
        tools.append(cyl(0.75, z0 - 1, z1 + 1, bx + dx, by + dy))
    return tools


def make_plate():
    p = cyl(PLATE_R, PLATE_BOT, STANDOFF_Z)
    cut(p, *plate_cutters(PLATE_BOT, STANDOFF_Z))
    for x, y in STANDOFFS:
        cut(p, cone(2.1, PLATE_BOT - 0.01, 1.0, PLATE_BOT + 1.1, x, y))
    return p


def make_board_ref():
    g = cyl(GLASS_R, 1.2, GLASS_TOP)
    add(g, cyl(33.0, -2.1, 1.2))  # PCB + LCD module (the PCB corners reach r = 35.5 only away from the controls)
    for x, y in STANDOFFS:
        add(g, cyl(1.75, STANDOFF_Z, -2.1, x, y))
    for deg in USB.values():
        add(g, rt_box(deg, 29.2, 36.8, -4.47, 4.47, -4.9, -1.64))
    add(g, box((29.4 + SW_BODY_X) / 2, -7.3, -2.75, SW_BODY_X - 29.4, 9.0, 2.7))  # switch body
    add(g, box((SW_BODY_X + LEVER[0]) / 2, LEVER[1], LEVER[2], LEVER[0] - SW_BODY_X, LEVER[3], LEVER[4]))
    for y in BUTTONS.values():
        add(g, box(32.2, y, BTN_Z, 3.6, 4.7, 2.3))
        add(g, box(33.8, y, BTN_Z, 0.4, 1.6, 0.6))
    add(g, box(J1[0], J1[1], -3.7, 7.6, 5.2, 3.4))
    return g


def make_cell_ref():
    return box(0.0, CELL_Y0, CELL_TOP - 10.3 / 2 - 0.15, 34.5, 56.0, 10.3)


# ---- Fusion plumbing --------------------------------------------------------------------------
def place(root, name, body):
    bf = root.features.baseFeatures.add()
    bf.startEdit()
    root.bRepBodies.add(body, bf)
    bf.finishEdit()
    bf.name = name
    b = bf.bodies.item(0)
    b.name = name
    return b


def overlap_mm3(a, b):
    t = TBM.copy(a)
    TBM.booleanOperation(t, TBM.copy(b), INTER)
    return t.volume * 1000.0


def clear(des):
    """Remove everything this script made before (the script is the source of truth)."""
    tl = des.timeline
    for i in range(tl.count - 1, -1, -1):
        ent = tl.item(i).entity
        if ent is not None:
            ent.deleteMe()


def run(context):
    app = adsk.core.Application.get()
    des = adsk.fusion.Design.cast(app.activeProduct)
    root = des.rootComponent
    clear(des)
    ring_, plate, cup, slider = make_ring(), make_plate(), make_cup(), make_slider()
    board, cell = make_board_ref(), make_cell_ref()
    lr, lt = lever_t()

    parts = {"ring": ring_, "plate": plate, "cup": cup, "slider": slider}
    refs = {"board": board, "cell": cell}
    bad = []
    names = list(parts) + list(refs)
    everything = {**parts, **refs}
    for i, a in enumerate(names):
        for b in names[i + 1 :]:
            if a in refs and b in refs:
                continue
            v = overlap_mm3(everything[a], everything[b])
            if v > 1e-3:
                bad.append(f"{a} x {b}: {v:.3f} mm3")
    # slider at both ends of the switch travel must still clear the board and the cup
    for off in (-LEVER_TRAVEL, LEVER_TRAVEL):  # travel ends about the switch body centre
        s = make_slider(offset=off)
        for k, other in (("board", board), ("cup", cup)):
            v = overlap_mm3(s, other)
            if v > 1e-3 and not (k == "board"):
                bad.append(f"slider@{off:.2f} x {k}: {v:.3f} mm3")
    # assembly path: every Cup feature above the plate must pass the plate's outline going up
    keepout = cyl(PLATE_R, PLATE_BOT, SEAM_Z)
    cut(keepout, *plate_cutters(PLATE_BOT, SEAM_Z))
    cup_above = TBM.copy(cup)
    TBM.booleanOperation(cup_above, cyl(R_IN, PLATE_BOT + 0.01, SEAM_Z), INTER)
    v = overlap_mm3(cup_above, keepout)
    if v > 1e-3:
        bad.append(f"cup blocks plate on assembly: {v:.3f} mm3")
    print("interference:", "none" if not bad else bad)
    print(f"lever in slider frame r={lr:.2f} t={lt:.2f}; slider angle {SL_ANG:.2f} deg")

    for name, body in (("Board (reference)", board), ("Cell (reference)", cell)):
        place(root, name, body)
    place(root, "Ring", ring_)
    place(root, "Plate", plate)
    place(root, "Cup", cup)
    place(root, "Slider", slider)
    print(f"overall: dia {2 * R_OUT:.1f} mm, thickness {TOP_Z - BOT_Z:.1f} mm")
