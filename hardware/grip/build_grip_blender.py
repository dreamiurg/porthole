"""Grip v3 and its ergonomics gauge, built in Blender from Meshy's yoke and the build123d snap cradle.

Run inside Blender (e.g. through the Blender MCP: exec(open(path).read(), {"__file__": path})).
Inputs, next to this file:
  source/meshy_yoke_v2.glb   Meshy multi-image-to-3D of the "Yoke Pro" concept with a simplified centre
  stl/snap_test.stl          the snap cradle from grip_v1.py (fingers, key rib, ledge, root ring)
Outputs: stl/grip_v3.stl (full grip) and stl/ergo_gauge.stl (real right handle + 2.6 mm outline).

Frame as in puck_v1.py: origin = puck centre, +Z out of the screen, +Y = top of the screen, mm.
Meshy supplies only the handles and the bottom bar; the centre (cradle, two spokes, a pad to the bar)
is replaced by exact geometry so the puck seats without the generated ring, claws or lips.
The puck is charged out of the grip, so there is no USB access.
"""

import math
import os

import bmesh
import bpy
from mathutils import Matrix

HERE = os.path.dirname(os.path.abspath(__file__))
BACK_Z = -26.2  # print bed plane (grip_v1.BACK_Z)
ROOT_Z = -22.2  # cradle finger roots (grip_v1.ROOT_Z)
CUP_TOP = -12.6  # cradle rim (grip_v1.CUP_TOP)
WIDTH = 215.0  # overall grip width
DEPTH_SCALE = 1.25  # Meshy's model is shallow; this gives ~24 mm deep handles
GAUGE_PLATE = 2.6  # gauge outline thickness (must not land on z -22.2, the cradle root face)
GAUGE_HANDLE_X = 64.3  # gauge keeps the full right handle beyond this x
SC = bpy.context.scene


def select_only(ob):
    for o in bpy.context.selected_objects:
        o.select_set(False)
    bpy.context.view_layer.objects.active = ob
    ob.select_set(True)


def boolean(ob, tool, op, remove_tool=True):
    select_only(ob)
    mod = ob.modifiers.new("b", "BOOLEAN")
    mod.operation, mod.object, mod.solver = op, tool, "EXACT"
    bpy.ops.object.modifier_apply(modifier="b")
    if remove_tool:
        bpy.data.objects.remove(tool, do_unlink=True)


def extrude(name, pts, z0, z1, bevel=0.0):
    bm = bmesh.new()
    vb = [bm.verts.new((x, y, z0)) for x, y in pts]
    vt = [bm.verts.new((x, y, z1)) for x, y in pts]
    bm.faces.new(vb[::-1])
    bm.faces.new(vt)
    for i in range(len(pts)):
        j = (i + 1) % len(pts)
        bm.faces.new((vb[i], vb[j], vt[j], vt[i]))
    bmesh.ops.recalc_face_normals(bm, faces=bm.faces)
    me = bpy.data.meshes.new(name)
    bm.to_mesh(me)
    bm.free()
    ob = bpy.data.objects.new(name, me)
    SC.collection.objects.link(ob)
    if bevel:
        b = ob.modifiers.new("bev", "BEVEL")
        b.width, b.segments, b.limit_method = bevel, 5, "ANGLE"
        select_only(ob)
        bpy.ops.object.modifier_apply(modifier="bev")
    return ob


def cylinder(r, z0, z1):
    bpy.ops.mesh.primitive_cylinder_add(radius=r, depth=z1 - z0, vertices=256, location=(0, 0, (z0 + z1) / 2))
    return bpy.context.active_object


def polar(r, a):
    return r * math.cos(math.radians(a)), r * math.sin(math.radians(a))


def sector(r0, r1, a0, a1):
    angs = list(range(a0, a1 + 1))
    return [polar(r1, a) for a in angs] + [polar(r0, a) for a in reversed(angs)]


def both_sides(pts):
    return [pts, [(-x, y) for x, y in reversed(pts)]]


def import_meshy(path):
    before = set(bpy.data.objects.keys())
    bpy.ops.import_scene.gltf(filepath=path)
    new = [bpy.data.objects[n] for n in bpy.data.objects.keys() if n not in before]
    meshes = [o for o in new if o.type == "MESH"]
    for o in bpy.context.selected_objects:
        o.select_set(False)
    for o in meshes:
        o.select_set(True)
    bpy.context.view_layer.objects.active = meshes[0]
    if len(meshes) > 1:
        bpy.ops.object.join()
    m = bpy.context.active_object
    m.parent = None
    for o in new:
        if o.name in bpy.data.objects and o != m:
            bpy.data.objects.remove(o, do_unlink=True)
    bpy.ops.object.transform_apply(location=True, rotation=True, scale=True)
    m.data.transform(Matrix.Rotation(math.radians(-90), 4, "X"))  # glTF Y-up -> screen +Z, top +Y
    xs = [v.co.x for v in m.data.vertices]
    m.data.transform(Matrix.Translation((-(max(xs) + min(xs)) / 2, 0, 0)))
    m.data.transform(Matrix.Scale(WIDTH / (max(xs) - min(xs)), 4))
    zmin = min(v.co.z for v in m.data.vertices)
    m.data.transform(Matrix.Translation((0, 0, BACK_Z - 0.6 - zmin)))
    for v in m.data.vertices:
        v.co.z = BACK_Z - 0.6 + (v.co.z - (BACK_Z - 0.6)) * DEPTH_SCALE
    m.data.update()
    return m


def build_grip():
    m = import_meshy(os.path.join(HERE, "source", "meshy_yoke_v2.glb"))
    m.name = "grip_v3"
    # Meshy's centre out: its ring (r 56, r 53 above the bottom bar), the ring's top arc, its spokes
    centre = [polar(53.0 if -150 <= a <= -30 else 56.0, a) for a in range(-180, 180)]
    cuts = [centre, sector(51.5, 68, 35, 145), sector(51.5, 64, 25, 155)]
    cuts += both_sides([(46, 3), (69.5, 3), (69.5, 27), (63, 31.5), (55, 33.5), (46, 36)])
    for i, pts in enumerate(cuts):
        boolean(m, extrude(f"cut{i}", pts, -60, 60), "DIFFERENCE")
    # new centre: two rounded spokes into the handles and a pad to the bottom bar
    for pts in both_sides([(40, 12), (73, 12), (73, 29), (40, 29)]):
        boolean(m, extrude("spoke", pts, BACK_Z - 0.5, -11.0, bevel=3.0), "UNION")
    boolean(m, extrude("pad", [(-17, -58), (17, -58), (17, -44), (-17, -44)], BACK_Z - 0.5, CUP_TOP, bevel=3.0), "UNION")
    # the snap cradle: clear its space (radii offset from its own faces so no faces coincide), then merge it
    boolean(m, cylinder(46.5, ROOT_Z - 0.5, 40), "DIFFERENCE")
    boolean(m, cylinder(34.5, BACK_Z - 5, ROOT_Z), "DIFFERENCE")
    bpy.ops.wm.stl_import(filepath=os.path.join(HERE, "stl", "snap_test.stl"))
    boolean(m, bpy.context.selected_objects[0], "UNION")
    floor = extrude("floor", [(-400, -400), (400, -400), (400, 400), (-400, 400)], BACK_Z - 100, BACK_Z + 0.01)
    boolean(m, floor, "DIFFERENCE")
    return m


def build_gauge(grip):
    g = grip.copy()
    g.data = grip.data.copy()
    g.name = "ergo_gauge"
    SC.collection.objects.link(g)
    cut = extrude("gauge_cut", [(-200, -200), (GAUGE_HANDLE_X, -200), (GAUGE_HANDLE_X, 200), (-200, 200)], BACK_Z + GAUGE_PLATE, 100)
    boolean(g, cut, "DIFFERENCE")
    ring = cylinder(45.5, BACK_Z + 1.0, -15.0)  # puck locating ring, 0.4 mm clearance
    boolean(ring, cylinder(42.4, -40, 40), "DIFFERENCE")
    boolean(g, ring, "UNION")
    return g


def check(ob):
    bm = bmesh.new()
    bm.from_mesh(ob.data)
    bad = sum(1 for e in bm.edges if not e.is_manifold)
    vol = bm.calc_volume() / 1000
    bm.free()
    assert bad == 0, f"{ob.name}: {bad} non-manifold edges"
    return round(vol, 1)


def export(ob, name, decimate=1.0):
    select_only(ob)
    if decimate < 1.0:
        d = ob.modifiers.new("dec", "DECIMATE")
        d.ratio = decimate
    bpy.ops.wm.stl_export(filepath=os.path.join(HERE, "stl", name), export_selected_objects=True, apply_modifiers=True)
    for mod in list(ob.modifiers):
        ob.modifiers.remove(mod)


grip = build_grip()
gauge = build_gauge(grip)
volumes = {"grip_v3_cm3": check(grip), "ergo_gauge_cm3": check(gauge)}
export(grip, "grip_v3.stl", decimate=0.5)
export(gauge, "ergo_gauge.stl", decimate=0.5)
result = volumes
