"""Porthole puck case, v1 - Fusion 360 script (run via the Fusion MCP or Scripts & Add-Ins).

Rebuilds the whole design in the active (empty) document: Bezel, Plate, Lid, plus reference
bodies for the board and the cell. All numbers are millimetres in the board's STEP frame:
origin = board center, +Z = out of the screen, glass top at z = +3.4, standoff feet at z = -6.1.
Board positions come from Waveshare's ESP32-S3-Touch-LCD-2.1 STEP (see docs/superpowers/specs/
2026-09-24-puck-case-design.md). Fusion's API works in centimetres; `m()` converts.

Parts:
  Bezel  front lip over the glass edge + outer wall; USB-C x2, switch slot, BOOT/RESET pinholes,
         LED windows, 3 radial M2 screws into the plate posts.
  Plate  screws to the board's 4 M2 standoffs (countersunk from below); 3 posts carry the bezel's
         radial screws and the lid's axial screws; cutouts for the cell lead, buzzer and USB plugs.
  Lid    back cover with the cell pocket fence and a buzzer vent; 3 M2 screws into the plate posts.
"""

import math

import adsk.core
import adsk.fusion

# ---- parameters (mm) ------------------------------------------------------------------------
GLASS_R = 37.5  # board / touch glass radius
GLASS_TOP = 3.4
STANDOFF_Z = -6.1  # bottom face of the board's M2 standoffs
FIT = 0.3  # radial gap glass -> bezel wall
WALL = 2.0
R_IN = GLASS_R + FIT  # 37.8
R_OUT = R_IN + WALL  # 39.8
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
RADIAL_SCREW_Z = -9.6
STANDOFFS = [(29.5, 13.6), (-29.5, 13.6), (18.0, -27.05), (-18.0, -27.05)]
USB = {"USB (native)": -43.45, "UART": -136.4}  # receptacle axis angles, mouth at r = 36.8
USB_Z = -3.27
USB_OPEN = (13.0, 7.5)  # tangential x height, fits a standard USB-C overmold
SWITCH = (-11.3, -2.75, 9.0, 3.6)  # angle, z, tangential slot length, height
BUTTONS = {"BOOT": 13.5, "RESET": 0.9}  # angles; plungers at r = 34, z = -2.85
LEDS = {"CHG": -23.3, "PWR": -30.3}
BUZZER = (-27.5, -12.3)  # from Waveshare's labelled photo; not in the STEP by name - verify
J1 = (21.2, 8.6)  # battery socket center


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
    obb = adsk.core.OrientedBoundingBox3D.create(P(cx, cy, cz), adsk.core.Vector3D.create(1, 0, 0), adsk.core.Vector3D.create(0, 1, 0), m(lx), m(ly), m(lz))
    return TBM.createBox(obb)


def radial_box(r_center, deg, z, radial, tangential, height):
    a = math.radians(deg)
    obb = adsk.core.OrientedBoundingBox3D.create(
        polar(r_center, deg, z),
        adsk.core.Vector3D.create(math.cos(a), math.sin(a), 0),
        adsk.core.Vector3D.create(-math.sin(a), math.cos(a), 0),
        m(radial),
        m(tangential),
        m(height),
    )
    return TBM.createBox(obb)


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


# ---- parts ----------------------------------------------------------------------------------
def make_bezel():
    b = cyl(R_OUT, SPLIT_Z, TOP_Z)
    cut(b, cyl(R_IN, SPLIT_Z - 1, GLASS_TOP), cyl(GLASS_R - LIP_W, GLASS_TOP - 1, TOP_Z + 1))
    for deg in USB.values():
        cut(b, radial_box(R_OUT, deg, USB_Z, 8.0, *USB_OPEN))
    ang, z, length, height = SWITCH
    cut(b, radial_box(R_OUT, ang, z, 8.0, length, height))
    for deg in BUTTONS.values():
        cut(b, radial_hole(deg, -2.85, 1.6, R_IN - 1, R_OUT + 1))
    for deg in LEDS.values():
        cut(b, radial_hole(deg, -4.4, 2.0, R_IN - 1, R_OUT + 1))
    for deg in POST_ANGLES:  # M2 clearance + counterbore for a pan head
        cut(b, radial_hole(deg, RADIAL_SCREW_Z, 2.3, R_IN - 1, R_OUT + 1))
        cut(b, radial_hole(deg, RADIAL_SCREW_Z, 4.2, R_OUT - 0.8, R_OUT + 1))
    return b


def make_plate():
    p = cyl(PLATE_R, PLATE_BOT, STANDOFF_Z)
    for deg in POST_ANGLES:
        add(p, radial_box((POST_R0 + PLATE_R) / 2, deg, (FLOOR_TOP + PLATE_BOT) / 2 + 0.05, PLATE_R - POST_R0, POST_W, PLATE_BOT - FLOOR_TOP + 0.1))
    TBM.booleanOperation(p, cyl(PLATE_R, FLOOR_TOP, STANDOFF_Z), INTER)  # trim posts to the round wall
    for x, y in STANDOFFS:  # M2 countersunk from below
        cut(p, cyl(1.15, PLATE_BOT - 1, STANDOFF_Z + 1, x, y), cone(2.1, PLATE_BOT - 0.01, 1.0, PLATE_BOT + 1.1, x, y))
    for deg in POST_ANGLES:
        a = math.radians(deg)
        rc = (POST_R0 + PLATE_R) / 2
        cut(p, cyl(0.8, FLOOR_TOP - 1, PLATE_BOT - 1.0, rc * math.cos(a), rc * math.sin(a)))  # lid screw pilot
        cut(p, radial_hole(deg, RADIAL_SCREW_Z, 1.6, PLATE_R - 5.0, PLATE_R + 1))  # bezel screw pilot
    cut(p, box(J1[0], J1[1] - 2.0, STANDOFF_Z - 1, 12.0, 11.0, 4.0))  # cell lead to the battery socket
    for deg in USB.values():  # room for the USB-C overmold below the port
        cut(p, radial_box(PLATE_R, deg, USB_Z, 3.0, USB_OPEN[0], USB_OPEN[1]))
    bx, by = BUZZER
    for dx, dy in [(0, 0), (2.2, 0), (-2.2, 0), (0, 2.2), (0, -2.2)]:
        cut(p, cyl(0.75, PLATE_BOT - 1, STANDOFF_Z + 1, bx + dx, by + dy))
    return p


def make_lid():
    lid = cyl(R_OUT, BOT_Z, SPLIT_Z)
    cut(lid, cyl(R_IN, FLOOR_TOP, SPLIT_Z + 1))
    cx, cy = 0.0, CELL_Y0
    fence = box(cx, cy, FLOOR_TOP + 2.0, CELL[0] + 2.4, CELL[1] + 2.4, 4.0)
    cut(fence, box(cx, cy, FLOOR_TOP + 2.0, CELL[0], CELL[1], 5.0))
    for sx in (-1, 1):  # finger gaps in the fence so the cell lifts out
        cut(fence, box(sx * (CELL[0] / 2 + 1.2), cy, FLOOR_TOP + 3.0, 3.0, 14.0, 4.1))
    add(lid, fence)
    for deg in POST_ANGLES:  # M2 clearance + counterbore from below
        a = math.radians(deg)
        rc = (POST_R0 + PLATE_R) / 2
        x, y = rc * math.cos(a), rc * math.sin(a)
        cut(lid, cyl(1.15, BOT_Z - 1, FLOOR_TOP + 1, x, y), cyl(2.1, BOT_Z - 1, BOT_Z + 1.4, x, y))
    bx, by = BUZZER
    for dx, dy in [(0, 0), (2.2, 0), (-2.2, 0), (0, 2.2), (0, -2.2)]:
        cut(lid, cyl(0.75, BOT_Z - 1, FLOOR_TOP + 1, bx + dx, by + dy))
    return lid


def make_board_ref():
    g = cyl(GLASS_R, 1.2, GLASS_TOP)
    add(g, cyl(35.5, -2.1, 1.2))  # PCB + LCD module envelope
    for x, y in STANDOFFS:
        add(g, cyl(1.75, STANDOFF_Z, -2.1, x, y))
    for deg in USB.values():
        add(g, radial_box(33.0, deg, USB_Z, 7.6, 8.94, 3.26))
    add(g, radial_box(31.0, SWITCH[0], -2.75, 7.5, 9.3, 2.7))
    add(g, box(J1[0], J1[1], -3.7, 7.6, 5.2, 3.4))
    return g


def make_cell_ref():
    return box(0.0, CELL_Y0, CELL_TOP - 10.3 / 2 - 0.15, 34.5, 56.0, 10.3)


# ---- Fusion plumbing ------------------------------------------------------------------------
def place(root, name, body):
    # Part Design documents hold one component, so each part is a named body in the root.
    bf = root.features.baseFeatures.add()
    bf.startEdit()
    root.bRepBodies.add(body, bf)
    bf.finishEdit()
    bf.name = name
    b = bf.bodies.item(0)
    b.name = name
    return None, root, b


def fillet_circle(comp, body, radius_mm, r_edge, z_edge, fr):
    edges = adsk.core.ObjectCollection.create()
    for e in body.edges:
        g = e.geometry
        if isinstance(g, adsk.core.Circle3D) and abs(g.radius - m(r_edge)) < 1e-3 and abs(g.center.z - m(z_edge)) < 1e-3:
            edges.add(e)
    if edges.count == 0:
        return 0
    fi = comp.features.filletFeatures.createInput()
    fi.edgeSetInputs.addConstantRadiusEdgeSet(edges, adsk.core.ValueInput.createByString(f"{fr} mm"), True)
    comp.features.filletFeatures.add(fi)
    return edges.count


def overlap_mm3(a, b):
    t = TBM.copy(a)
    TBM.booleanOperation(t, TBM.copy(b), INTER)
    return t.volume * 1000.0  # cm3 -> mm3


def run(context):
    app = adsk.core.Application.get()
    des = adsk.fusion.Design.cast(app.activeProduct)
    root = des.rootComponent
    bezel, plate, lid = make_bezel(), make_plate(), make_lid()
    board, cell = make_board_ref(), make_cell_ref()

    checks = {
        "bezel x board": overlap_mm3(bezel, board),
        "plate x board": overlap_mm3(plate, board),
        "lid x board": overlap_mm3(lid, board),
        "bezel x plate": overlap_mm3(bezel, plate),
        "bezel x lid": overlap_mm3(bezel, lid),
        "plate x lid": overlap_mm3(plate, lid),
        "cell x plate": overlap_mm3(cell, plate),
        "cell x lid": overlap_mm3(cell, lid),
        "cell x bezel": overlap_mm3(cell, bezel),
    }
    for k, v in checks.items():
        print(f"interference {k}: {v:.3f} mm3")

    for name, body in (("Board (reference)", board), ("Cell (reference)", cell)):
        place(root, name, body)
    _, c, b = place(root, "Bezel", bezel)
    print("bezel top fillet edges", fillet_circle(c, b, 0, R_OUT, TOP_Z, EDGE_FILLET_TOP))
    place(root, "Plate", plate)
    _, c, b = place(root, "Lid", lid)
    print("lid bottom fillet edges", fillet_circle(c, b, 0, R_OUT, BOT_Z, EDGE_FILLET_BOT))
    print(f"overall: dia {2 * R_OUT:.1f} mm, thickness {TOP_Z - BOT_Z:.1f} mm")
