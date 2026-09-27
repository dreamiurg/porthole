"""Slider v4 mechanism prototype: a bare wall block with the rail, the slider, and a lid bar
standing in for the Ring. No board, plate or other features - just the sliding fit.

Runs on top of puck_case_fusion.py (same frame: the block is the real wall around the switch).
Slider: curved body in a curved pocket (long tail toward the USB side), narrow stem through the
outer wall, wide cap outside hugging the wall, neck through the inner wall to the fork.
"""

import importlib.util
import math
import os

import adsk.core
import adsk.fusion

SRC = os.path.join(os.path.dirname(__file__), "puck_case_fusion.py")
_spec = importlib.util.spec_from_file_location("puck_case", SRC)  # re-read on every Fusion run
assert _spec and _spec.loader
pc = importlib.util.module_from_spec(_spec)
_spec.loader.exec_module(pc)
_NAMES = (
    "FORK_HALF",
    "FORK_TOP",
    "FORK_X",
    "INTER",
    "LEVER",
    "LEVER_TRAVEL",
    "LEVER_YC",
    "NECK_W",
    "NOTCH_BACK",
    "NOTCH_GAP",
    "R_IN",
    "R_OUT",
    "SEAM_GAP",
    "SEAM_Z",
    "SL_ANG",
    "SL_Z",
    "TBM",
    "add",
    "box",
    "clear",
    "cut",
    "export_stl",
    "m",
    "overlap_mm3",
    "place",
    "ring",
    "rt_box",
)
(
    FORK_HALF,
    FORK_TOP,
    FORK_X,
    INTER,
    LEVER,
    LEVER_TRAVEL,
    LEVER_YC,
    NECK_W,
    NOTCH_BACK,
    NOTCH_GAP,
    R_IN,
    R_OUT,
    SEAM_GAP,
    SEAM_Z,
    SL_ANG,
    SL_Z,
    TBM,
    add,
    box,
    clear,
    cut,
    export_stl,
    m,
    overlap_mm3,
    place,
    ring,
    rt_box,
) = (getattr(pc, k) for k in _NAMES)

OUT = os.path.join(os.path.dirname(__file__), "stl", "proto")
R_LEVER = math.hypot(LEVER[0], LEVER_YC)  # arc radius the fork follows

BAND = (38.6, 40.3)  # pocket radial band: 0.8 inner wall, 1.3 outer wall
BODY_T = 1.4  # slider body thickness inside the 1.7 band (0.3 radial play)
TAIL, HEAD = 9.0, 3.8  # body length from the stem toward -t (USB side) / +t (RESET side)
STEM_W = 2.4
CAP = (8.0, 1.5)  # width, proud of the wall
BLOCK_T = (-17.0, 11.0)  # test block extent along the slider axis
BLOCK_Z = (-9.0, SEAM_Z)
LID_Z = (SEAM_Z + SEAM_GAP, 1.5)


def rot(body, deg):
    mtx = adsk.core.Matrix3D.create()
    mtx.setToRotation(math.radians(deg), adsk.core.Vector3D.create(0, 0, 1), adsk.core.Point3D.create(0, 0, 0))
    TBM.transform(body, mtx)
    return body


def sector(r0, r1, t0, t1, z0, z1):
    """Curved band r0..r1 between two cuts at t0..t1 along the slider axis."""
    s = ring(r0, r1, z0, z1)
    TBM.booleanOperation(s, rt_box(SL_ANG, 0.0, r1 + 1, t0, t1, z0 - 1, z1 + 1), INTER)
    return s


def fork(gap=NOTCH_GAP):
    f = rt_box(SL_ANG, 30.0, R_IN - 0.8, -FORK_HALF, FORK_HALF, SL_Z[0], FORK_TOP)
    cut(f, box(FORK_X - 10.0, LEVER_YC, 0.0, 20.0, 40.0, 40.0))
    x1 = LEVER[0] + NOTCH_BACK
    z0 = LEVER[2] - LEVER[4] / 2 - 0.5
    cut(f, box((25.0 + x1) / 2, LEVER_YC, (z0 + 5.0) / 2, x1 - 25.0, LEVER[3] + 2 * gap, 5.0 - z0))
    return f


def make_block(travel=LEVER_TRAVEL):
    b = sector(R_IN, R_OUT, *BLOCK_T, *BLOCK_Z)
    tr = travel + 0.3
    z0 = SL_Z[0] - 0.2
    cut(b, sector(BAND[0], BAND[1], -TAIL - tr - 0.2, HEAD + tr + 0.2, z0, SEAM_Z + 1))
    cut(b, rt_box(SL_ANG, BAND[1] - 0.1, R_OUT + 1, -STEM_W / 2 - tr, STEM_W / 2 + tr, z0, SEAM_Z + 1))
    cut(b, rt_box(SL_ANG, R_IN - 1, BAND[0] + 0.1, -NECK_W / 2 - tr, NECK_W / 2 + tr, z0, SEAM_Z + 1))
    return b


def make_slider(off=0.0, gap=NOTCH_GAP, play=(BAND[1] - BAND[0] - BODY_T) / 2, head=HEAD):
    s = sector(BAND[0] + play, BAND[1] - play, -TAIL, head, *SL_Z)
    add(s, rt_box(SL_ANG, BAND[1] - 0.4, R_OUT + 0.3, -STEM_W / 2, STEM_W / 2, *SL_Z))
    add(s, sector(R_OUT + 0.15, R_OUT + 0.15 + CAP[1], -CAP[0] / 2, CAP[0] / 2, *SL_Z))
    for k in (-1, 0, 1):  # grip ridges
        add(s, sector(R_OUT + CAP[1], R_OUT + CAP[1] + 0.5, k * 2.2 - 0.3, k * 2.2 + 0.3, *SL_Z))
    add(s, rt_box(SL_ANG, R_IN - 1.0, BAND[0] + 0.4, -NECK_W / 2, NECK_W / 2, SL_Z[0], FORK_TOP))
    add(s, fork(gap))
    return rot(s, math.degrees(off / R_LEVER))  # slides along the wall's arc


def make_lid():
    return sector(R_IN, R_OUT, *BLOCK_T, *LID_Z)


def run(context):
    app = adsk.core.Application.get()
    des = adsk.fusion.Design.cast(app.activeProduct)
    root = des.rootComponent
    clear(des)
    block, lid = make_block(), make_lid()
    bad = []
    for off in (-LEVER_TRAVEL, 0.0, LEVER_TRAVEL):
        for k, other in (("block", block), ("lid", lid)):
            v = overlap_mm3(make_slider(off), other)
            if v > 1e-3:
                bad.append(f"slider@{off:+.1f} x {k}: {v:.3f} mm3")
    print("interference:", "none" if not bad else bad)
    os.makedirs(OUT, exist_ok=True)
    for name, body in (("Block", block), ("Slider", make_slider()), ("Slider_sym", make_slider(head=TAIL)), ("Lid", lid)):
        b = place(root, name, body)
        export_stl(des, b, os.path.join(OUT, f"v4_{name.lower()}.stl"))
        b.isLightBulbOn = name != "Slider_sym"

    os.makedirs(OUT, exist_ok=True)
    vp = app.activeViewport

    def snap(name, eye, hide=()):
        for b in root.bRepBodies:
            b.isLightBulbOn = b.name not in hide and b.name != "Slider_sym"
        cam = vp.camera
        cam.isFitView = False
        cam.cameraType = adsk.core.CameraTypes.PerspectiveCameraType
        t = (38.5, -8.0, -4.0)
        cam.target = adsk.core.Point3D.create(*[m(v) for v in t])
        n = math.sqrt(sum(v * v for v in eye))
        cam.eye = adsk.core.Point3D.create(*[m(a + 75.0 * v / n) for a, v in zip(t, eye, strict=True)])
        cam.upVector = adsk.core.Vector3D.create(0, 0, 1)
        vp.camera = cam
        vp.refresh()
        adsk.doEvents()
        vp.saveAsImageFile(os.path.join(OUT, name), 1400, 900)

    snap("v4_outside.png", (30, -8, 10))
    snap("v4_inside.png", (-30, 4, 8), hide=("Lid",))
    snap("v4_top.png", (-4, 2, 30), hide=("Lid",))
    for b in root.bRepBodies:
        b.isLightBulbOn = b.name != "Slider_sym"
    print("images in", OUT)
