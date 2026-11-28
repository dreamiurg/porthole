"""Build the P014 two-color bezel from the shipped R2 mesh.

Run Blender -b -P hardware/case/pinstripe_blender.py. The two STL components
already share the same lip-down print pose. All dimensions are millimetres.
"""

import math
import shutil
from pathlib import Path

import bmesh
import bpy
from mathutils import Matrix

HERE = Path(__file__).resolve().parent
R2 = HERE / "stl/twist-lock/r2"
KIT = HERE / "stl/twist-lock/r3-pinstripe"
STRIPE_IN = 39.35
STRIPE_OUT = 40.70
DEPTH = 0.8
FACE_Z = 4.6


def activate(obj):
    for selected in bpy.context.selected_objects:
        selected.select_set(False)
    obj.select_set(True)
    bpy.context.view_layer.objects.active = obj


def cut(obj, tool, operation):
    activate(obj)
    modifier = obj.modifiers.new("Material boundary", "BOOLEAN")
    modifier.solver = "MANIFOLD"
    modifier.operation = operation
    modifier.object = tool
    bpy.ops.object.modifier_apply(modifier=modifier.name)


def stripe_tool():
    # Lower surface follows the original 2.5 mm round on the 42 mm outer
    # radius. The visible surface itself is cut directly from the stock STL.
    profile = [(STRIPE_IN, FACE_Z + 1), (STRIPE_OUT, FACE_Z + 1)]
    for i in range(9):
        radius = STRIPE_OUT + (STRIPE_IN - STRIPE_OUT) * i / 8
        height = FACE_Z - 2.5 + math.sqrt(2.5**2 - max(0, radius - 39.5) ** 2)
        profile.append((radius, height - DEPTH))
    n = 288
    pts = [(r * math.cos(math.tau * j / n), r * math.sin(math.tau * j / n), z) for r, z in profile for j in range(n)]
    faces = []
    for row in range(len(profile)):
        lo, hi = row * n, ((row + 1) % len(profile)) * n
        for j in range(n):
            k = (j + 1) % n
            faces.append((lo + j, hi + j, hi + k, lo + k))
    mesh = bpy.data.meshes.new("Annulus")
    mesh.from_pydata(pts, [], faces)
    mesh.update()
    tool = bpy.data.objects.new("Stripe tool", mesh)
    bpy.context.scene.collection.objects.link(tool)
    return tool


def engrave_revision(body):
    # Readable from inside an empty case. The mark is a recess in the upper
    # inner wall opposite the slider, well above the radial screw seats.
    bpy.ops.object.text_add()
    text = bpy.context.object
    text.data.body = "P014"
    text.data.size = 4.0
    text.data.align_x = text.data.align_y = "CENTER"
    text.data.extrude = 0.25
    bpy.ops.object.convert(target="MESH")
    text = bpy.context.object
    bm = bmesh.new()
    bm.from_mesh(text.data)
    bmesh.ops.remove_doubles(bm, verts=bm.verts[:], dist=0.0001)
    bm.to_mesh(text.data)
    bm.free()
    for v in text.data.vertices:
        x, y, z = v.co
        v.co = (x, math.sqrt(37.8**2 - x * x) + 0.11 - z, -3.8 + y)
    text.data.update()
    cut(body, text, "DIFFERENCE")
    bpy.data.objects.remove(text, do_unlink=True)


def export(obj, path):
    # Rotate about X to put the broad flat LCD lip on the plate.
    obj.data.transform(Matrix.Translation((0, 0, FACE_Z)) @ Matrix.Diagonal((1, -1, -1, 1)))
    bm = bmesh.new()
    bm.from_mesh(obj.data)
    bmesh.ops.triangulate(bm, faces=bm.faces[:])
    bmesh.ops.dissolve_degenerate(bm, edges=bm.edges[:], dist=0.0001)
    bmesh.ops.remove_doubles(bm, verts=bm.verts[:], dist=0.0001)
    assert bm.calc_volume(signed=True) > 0 and all(e.is_manifold and e.is_contiguous for e in bm.edges), path
    bm.to_mesh(obj.data)
    bm.free()
    activate(obj)
    bpy.ops.wm.stl_export(filepath=str(path), export_selected_objects=True)


def build():
    bpy.ops.wm.read_factory_settings(use_empty=True)
    bpy.ops.wm.stl_import(filepath=str(R2 / "puck_bezel.stl"))
    body = bpy.context.object
    accent = body.copy()
    accent.data = body.data.copy()
    bpy.context.scene.collection.objects.link(accent)
    tool = stripe_tool()
    cut(accent, tool, "INTERSECT")
    cut(body, tool, "DIFFERENCE")
    engrave_revision(body)
    out = KIT / "components"
    out.mkdir(parents=True, exist_ok=True)
    export(body, out / "P014-bezel-body.stl")
    export(accent, out / "P014-bezel-pinstripe.stl")
    for name in ("puck_plate.stl", "puck_slider.stl", "side-groove-lid-r2.stl"):
        shutil.copyfile(R2 / name, KIT / name)


if __name__ == "__main__":
    build()
