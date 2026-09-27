"""Porthole puck case, v3 - Fusion 360 script (run via the Fusion MCP or Scripts & Add-Ins).

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
import os

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
LED_Z = -4.4  # window height in the wall
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
LEVER_YC = -7.3  # switch body centre = lever travel centre
SL_ANG = math.degrees(math.atan2(LEVER_YC, LEVER[0]))  # -11.85
SL_LEN, SL_Z = 8.0, (-5.8, SEAM_Z - 0.3)
CH_R = (38.4, 40.2)  # channel radial band; the slider body is 1.4 thick inside it
KNOB = (5.0, 1.5)  # width, proud of the wall; full slider height
NECK_W = 3.0  # web from the slider body through the inner wall to the fork
FORK_HALF, FORK_TOP = 2.8, -2.4  # fork half-width; top stays under the PCB (z -2.1)
FORK_X = 33.1  # fork inner face at travel centre, parallel to the switch body face (x = 32.5)
NOTCH_GAP = 0.4  # lever clearance each side, along y (the notch is aligned with the lever)
NOTCH_BACK = 0.9  # notch depth past the lever tip; deeper than the fork-to-body gap, so a push
# on the knob lands on the switch body, not the lever
PROTO_GAPS = (0.25, 0.4, 0.55)  # slider prototypes: NOTCH_GAP variants
PROTO_TRAVELS = (1.0, 1.5)  # wall test pieces: LEVER_TRAVEL variants (travel not in the STEP)
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


def lever_y(off):
    """Lever y with the slider at `off` along its travel (0 = switch body centre)."""
    return LEVER_YC + off * math.cos(math.radians(SL_ANG))


STEP_OFF = (LEVER[1] - LEVER_YC) / math.cos(math.radians(SL_ANG))  # slider offset for the STEP lever


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


def make_cup(travel=LEVER_TRAVEL, channel=True):
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
        cut(c, radial_hole(deg, LED_Z, 2.0, R_IN - 1, R_OUT + 1))
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
    # inner slot for the neck that carries the fork
    if not channel:
        return c
    t_trav = travel + 0.3
    z0 = SL_Z[0] - 0.2
    cut(c, rt_box(SL_ANG, CH_R[0], CH_R[1], -SL_LEN / 2 - t_trav - 0.2, SL_LEN / 2 + t_trav + 0.2, z0, SEAM_Z + 1))
    cut(c, rt_box(SL_ANG, CH_R[1] - 0.1, R_OUT + 1, -KNOB[0] / 2 - t_trav, KNOB[0] / 2 + t_trav, z0, SEAM_Z + 1))
    cut(c, rt_box(SL_ANG, R_IN - 1, CH_R[0] + 0.1, -NECK_W / 2 - t_trav, NECK_W / 2 + t_trav, z0, SEAM_Z + 1))
    return c


def id_marks(n, r, t0, z_top):
    """n small dimples in a row along t, to tell prototype variants apart."""
    return [cyl(0.4, z_top - 0.8, z_top + 1, *rt_xy(SL_ANG, r, t0 + 1.4 * i)) for i in range(n)]


def rt_xy(deg, r, t):
    a = math.radians(deg)
    return r * math.cos(a) - t * math.sin(a), r * math.sin(a) + t * math.cos(a)


def make_test_piece(travel, marks=0):
    """Cup wall section around the slider channel, to try the slider on the real switch."""
    piece = make_cup(travel)
    TBM.booleanOperation(piece, rt_box(SL_ANG, 30.0, R_OUT + 5, -9.0, 7.0, PLATE_BOT - 1.3, SEAM_Z + 1), INTER)
    return cut(piece, *id_marks(marks, R_IN - LEDGE_W / 2, -8.0, PLATE_BOT))  # on the ledge


def make_slider(offset=None, gap=NOTCH_GAP, marks=0):
    """Slider at `offset` along its travel (default: where the lever is in the STEP).

    One solid part that prints upright with no supports: body in the wall channel, thumb knob
    outside, and a fork block inside with a notch the lever drops into from above (the Cup goes
    on from below), so the notch walls are tied to the block behind and below them.
    """
    off = STEP_OFF if offset is None else offset
    dx = -off * math.sin(math.radians(SL_ANG))  # board-x shift of the slider at this offset
    s = rt_box(SL_ANG, CH_R[0] + 0.2, CH_R[1] - 0.2, off - SL_LEN / 2, off + SL_LEN / 2, *SL_Z)
    add(s, rt_box(SL_ANG, CH_R[1] - 0.3, R_OUT + KNOB[1], off - KNOB[0] / 2, off + KNOB[0] / 2, *SL_Z))
    for k in (-1, 0, 1):  # grip ridges on the knob
        add(s, rt_box(SL_ANG, R_OUT + KNOB[1] - 0.1, R_OUT + KNOB[1] + 0.4, off + k * 1.5 - 0.3, off + k * 1.5 + 0.3, *SL_Z))
    add(s, rt_box(SL_ANG, R_IN - 1.0, CH_R[0] + 0.4, off - NECK_W / 2, off + NECK_W / 2, SL_Z[0], FORK_TOP))
    fork = rt_box(SL_ANG, 30.0, R_IN - 0.8, off - FORK_HALF, off + FORK_HALF, SL_Z[0], FORK_TOP)
    cut(fork, box(FORK_X + dx - 10.0, LEVER_YC, 0.0, 20.0, 40.0, 40.0))
    x1 = LEVER[0] + NOTCH_BACK + dx
    z0 = LEVER[2] - LEVER[4] / 2 - 0.5
    cut(fork, box((25.0 + x1) / 2, lever_y(off), (z0 + 5.0) / 2, x1 - 25.0, LEVER[3] + 2 * gap, 5.0 - z0))
    add(s, fork)
    return cut(s, *id_marks(marks, (CH_R[0] + CH_R[1]) / 2, off - 1.4 * (marks - 1) / 2, SL_Z[1]))


def plate_cutters(z0, z1, fork=True):
    """Plate notches, as prisms over [z0, z1] so the same tools build the assembly keep-out."""
    tools = []
    for x, y in STANDOFFS:
        tools.append(cyl(1.15, z0 - 1, z1 + 1, x, y))
    tools.append(box(J1[0], J1[1] - 2.0, (z0 + z1) / 2, 12.0, 11.0, z1 - z0 + 2))
    for deg in USB.values():
        tools.append(rt_box(deg, PLATE_R - 2.5, PLATE_R + 1, -USB_OPEN[0] / 2, USB_OPEN[0] / 2, z0 - 1, z1 + 1))
    for y in BUTTONS.values():  # nubs pass the plate edge when the Cup goes on
        tools.append(box(36.5, y, (z0 + z1) / 2, 5.0, NUB[0] + 1.0, z1 - z0 + 2))
    if fork:
        half = FORK_HALF + LEVER_TRAVEL + 0.5  # the slider's fork passes the plate edge
        tools.append(rt_box(SL_ANG, 30.0, PLATE_R + 1, -half, half, z0 - 1, z1 + 1))
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


def make_board_ref(lever_dy=0.0):
    g = cyl(GLASS_R, 1.2, GLASS_TOP)
    add(g, cyl(33.0, -2.1, 1.2))  # PCB + LCD module (the PCB corners reach r = 35.5 only away from the controls)
    for x, y in STANDOFFS:
        add(g, cyl(1.75, STANDOFF_Z, -2.1, x, y))
    for deg in USB.values():
        add(g, rt_box(deg, 29.2, 36.8, -4.47, 4.47, -4.9, -1.64))
    add(g, box((29.4 + SW_BODY_X) / 2, -7.3, -2.75, SW_BODY_X - 29.4, 9.0, 2.7))  # switch body
    add(g, box((SW_BODY_X + LEVER[0]) / 2, LEVER[1] + lever_dy, LEVER[2], LEVER[0] - SW_BODY_X, LEVER[3], LEVER[4]))
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


def slider_checks(slider_at, cup, keepout, travel, tag, bad):
    """The slider at both travel ends: clear of the cup and of the board with the lever moved to
    match, and able to pass the plate when the Cup goes on."""
    for off in (-travel, travel):
        s = slider_at(off)
        board = make_board_ref(lever_y(off) - LEVER[1])
        for k, other in (("board", board), ("cup", cup), ("plate path", keepout)):
            v = overlap_mm3(s, other)
            if v > 1e-3:
                bad.append(f"{tag}@{off:+.1f} x {k}: {v:.3f} mm3")


def export_stl(des, body, path):
    opts = des.exportManager.createSTLExportOptions(body, path)
    opts.meshRefinement = adsk.fusion.MeshRefinementSettings.MeshRefinementHigh
    des.exportManager.execute(opts)


def run(context):
    app = adsk.core.Application.get()
    des = adsk.fusion.Design.cast(app.activeProduct)
    root = des.rootComponent
    clear(des)
    ring_, plate, cup, slider = make_ring(), make_plate(), make_cup(), make_slider()
    board, cell = make_board_ref(), make_cell_ref()

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
    # assembly path: everything that goes on with the Cup must pass the plate's outline going up
    keepout = cyl(PLATE_R, PLATE_BOT, SEAM_Z)
    cut(keepout, *plate_cutters(PLATE_BOT, SEAM_Z))
    cup_above = TBM.copy(cup)
    TBM.booleanOperation(cup_above, cyl(R_IN, PLATE_BOT + 0.01, SEAM_Z), INTER)
    v = overlap_mm3(cup_above, keepout)
    if v > 1e-3:
        bad.append(f"cup blocks plate on assembly: {v:.3f} mm3")
    slider_checks(lambda off: make_slider(off), cup, keepout, LEVER_TRAVEL, "slider", bad)
    # prototypes: every notch variant against every travel variant's wall piece
    protos = {}
    for travel in PROTO_TRAVELS:
        piece = make_test_piece(travel)
        protos[f"proto_wall_travel{travel:.1f}"] = make_test_piece(travel, PROTO_TRAVELS.index(travel) + 1)
        for gap in PROTO_GAPS:
            slider_checks(lambda off, g=gap: make_slider(off, g), piece, keepout, travel, f"gap{gap} travel{travel}", bad)
    for i, gap in enumerate(PROTO_GAPS):
        protos[f"proto_slider_gap{gap:.2f}"] = make_slider(0.0, gap, i + 1)
    print("interference:", "none" if not bad else bad)

    for name, body in (("Board (reference)", board), ("Cell (reference)", cell)):
        place(root, name, body)
    placed = {n: place(root, n.capitalize(), b) for n, b in parts.items()}
    out = os.path.join(os.path.dirname(__file__), "stl")
    for n, b in placed.items():
        export_stl(des, b, os.path.join(out, f"puck_{n}.stl"))
    os.makedirs(os.path.join(out, "proto"), exist_ok=True)
    for n, body in protos.items():
        b = place(root, n, body)
        export_stl(des, b, os.path.join(out, "proto", f"{n}.stl"))
        b.isLightBulbOn = False
    print(f"overall: dia {2 * R_OUT:.1f} mm, thickness {TOP_Z - BOT_Z:.1f} mm; STLs in {out}")
