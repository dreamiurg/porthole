"""Grip v4, built in Blender from Meshy's yoke and the build123d snap cradle.

Run inside Blender (e.g. through the Blender MCP: exec(open(path).read(), {"__file__": path})).
Inputs, next to this file:
  source/meshy_yoke_v2.glb   Meshy multi-image-to-3D of the "Yoke Pro" concept with a simplified centre
  stl/snap_test.stl          the snap cradle from grip_v1.py (fingers, key rib, ledge, root ring)
Output: stl/grip_v4.stl.

Frame as in puck_v1.py: origin = puck centre, +Z out of the screen, +Y = top of the screen, mm.
Meshy supplies only the handles and the bottom bar; the centre (cradle, two spokes, a pad to the bar)
is replaced by exact geometry so the puck seats without the generated ring, claws or lips.
The puck is charged out of the grip, so there is no USB access.

v4 after the first print: the handles were too thick for a kid's hand and too far from the screen for a
thumb, and their rounded backs hung in the air (spaghetti). Every surface is pulled in by INSET (thinner
tubes, same shape), the back is cut flat FLATTEN above its lowest point and set on the bed, and each half
moves PULL_IN towards the puck, the bottom bar shortened on both sides of its badge to match.
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
INSET = 2.0  # mm off every surface: handles 31 -> 27 mm across
FLATTEN = 2.5  # flat back this far above the lowest point, so the rounded handle backs sit on the bed
PULL_IN = 20.0  # each half moves this far towards the puck: the thumb is 2 cm closer to the screen
BADGE_X = 17.0  # the bottom bar's centre (badge) stays; the bar is shortened outside it
VOXEL = 0.25  # distance-field resolution for the inset
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


def sdf_offset(m, distance):
    """Move every surface of m by distance (negative = inwards) through a signed distance field: an exact
    offset that never folds, unlike pushing vertices along their normals."""
    ng = bpy.data.node_groups.new("sdf_offset", "GeometryNodeTree")
    ng.interface.new_socket("Geometry", in_out="INPUT", socket_type="NodeSocketGeometry")
    ng.interface.new_socket("Geometry", in_out="OUTPUT", socket_type="NodeSocketGeometry")
    gin, gout = ng.nodes.new("NodeGroupInput"), ng.nodes.new("NodeGroupOutput")
    sdf = ng.nodes.new("GeometryNodeMeshToSDFGrid")
    sdf.inputs["Voxel Size"].default_value = VOXEL
    sdf.inputs["Band Width"].default_value = int(abs(distance) / VOXEL) + 4  # the band must reach the new surface
    off = ng.nodes.new("GeometryNodeSDFGridOffset")
    off.inputs["Distance"].default_value = distance
    mesh = ng.nodes.new("GeometryNodeGridToMesh")
    ng.links.new(gin.outputs[0], sdf.inputs["Mesh"])
    ng.links.new(sdf.outputs["SDF Grid"], off.inputs["Grid"])
    ng.links.new(off.outputs["Grid"], mesh.inputs["Grid"])
    ng.links.new(mesh.outputs["Mesh"], gout.inputs[0])
    select_only(m)
    mod = m.modifiers.new("sdf", "NODES")
    mod.node_group = ng
    bpy.ops.object.modifier_apply(modifier="sdf")
    bpy.data.node_groups.remove(ng)


def inset_and_flatten(m):
    """Pull every surface in by INSET, then cut the back flat and set it on the bed."""
    sdf_offset(m, -INSET)
    zcut = min(v.co.z for v in m.data.vertices) + FLATTEN
    boolean(m, extrude("flat", [(-400, -400), (400, -400), (400, 400), (-400, 400)], zcut - 100, zcut), "DIFFERENCE")
    m.data.transform(Matrix.Translation((0, 0, (BACK_Z - 0.3) - zcut)))  # 0.3 below the bed; the floor cut trims it


def keep(m, x0, x1, name):
    """A copy of m clipped to x0 < x < x1."""
    c = m.copy()
    c.data = m.data.copy()
    c.name = name
    SC.collection.objects.link(c)
    boolean(c, extrude(f"{name}_box", [(x0, -400), (x1, -400), (x1, 400), (x0, 400)], -400, 400), "INTERSECT")
    return c


def pull_in(m):
    """Move each half PULL_IN towards the puck; cut the bar between the badge and the moved halves."""
    left = keep(m, -400, -(BADGE_X + PULL_IN - 0.5), "left")  # 0.5 overlap with the badge: no coincident faces
    right = keep(m, BADGE_X + PULL_IN - 0.5, 400, "right")
    centre = keep(m, -BADGE_X, BADGE_X, "centre")
    left.data.transform(Matrix.Translation((PULL_IN, 0, 0)))
    right.data.transform(Matrix.Translation((-PULL_IN, 0, 0)))
    bpy.data.objects.remove(m, do_unlink=True)
    boolean(centre, left, "UNION")
    boolean(centre, right, "UNION")
    return centre


def build_grip():
    m = import_meshy(os.path.join(HERE, "source", "meshy_yoke_v2.glb"))
    inset_and_flatten(m)
    # Meshy's centre out: its ring (r 56, r 53 above the bottom bar), the ring's top arc, its spokes
    centre = [polar(53.0 if -150 <= a <= -30 else 56.0, a) for a in range(-180, 180)]
    cuts = [centre, sector(51.5, 68, 35, 145), sector(51.5, 64, 25, 155)]
    cuts += both_sides([(46, 3), (69.5, 3), (69.5, 27), (63, 31.5), (55, 33.5), (46, 36)])
    for i, pts in enumerate(cuts):
        boolean(m, extrude(f"cut{i}", pts, -60, 60), "DIFFERENCE")
    m = pull_in(m)
    m.name = "grip_v4"
    # new centre: two rounded spokes into the handles and a pad to the bottom bar
    for pts in both_sides([(40, 12), (73 - PULL_IN, 12), (73 - PULL_IN, 29), (40, 29)]):
        boolean(m, extrude("spoke", pts, BACK_Z - 0.5, -11.0, bevel=3.0), "UNION")
    boolean(m, extrude("pad", [(-17, -58), (17, -58), (17, -44), (-17, -44)], BACK_Z - 0.5, CUP_TOP - 0.5, bevel=3.0), "UNION")  # top below the rim: no coincident faces
    # the snap cradle: clear its space (radii offset from its own faces so no faces coincide), then merge it
    boolean(m, cylinder(46.5, ROOT_Z - 0.5, 40), "DIFFERENCE")
    boolean(m, cylinder(34.5, BACK_Z - 5, ROOT_Z), "DIFFERENCE")
    bpy.ops.wm.stl_import(filepath=os.path.join(HERE, "stl", "snap_test.stl"))
    boolean(m, bpy.context.selected_objects[0], "UNION")
    floor = extrude("floor", [(-400, -400), (400, -400), (400, 400), (-400, 400)], BACK_Z - 100, BACK_Z + 0.01)
    boolean(m, floor, "DIFFERENCE")
    return m


def check(ob):
    bm = bmesh.new()
    bm.from_mesh(ob.data)
    bad = sum(1 for e in bm.edges if not e.is_manifold)
    vol = bm.calc_volume() / 1000
    bm.free()
    assert bad == 0, f"{ob.name}: {bad} non-manifold edges"
    return round(vol, 1)


def export(ob, name, decimate=1.0):
    """Write ob as an STL, decimated on a copy and cleaned so the file stays watertight."""
    c = ob.copy()
    c.data = ob.data.copy()
    SC.collection.objects.link(c)
    if decimate < 1.0:
        select_only(c)
        d = c.modifiers.new("dec", "DECIMATE")
        d.ratio = decimate
        bpy.ops.object.modifier_apply(modifier="dec")
        bm = bmesh.new()  # decimation leaves a few zero-length edges that break the manifold
        bm.from_mesh(c.data)
        bmesh.ops.dissolve_degenerate(bm, dist=1e-3, edges=bm.edges[:])
        bmesh.ops.remove_doubles(bm, verts=bm.verts, dist=1e-3)
        bm.to_mesh(c.data)
        bm.free()
    check(c)
    select_only(c)
    bpy.ops.wm.stl_export(filepath=os.path.join(HERE, "stl", name), export_selected_objects=True)
    bpy.data.objects.remove(c, do_unlink=True)


grip = build_grip()
volume = check(grip)
export(grip, "grip_v4.stl", decimate=0.25)  # the distance-field mesh is dense; keeps the STL under 12 MB
result = {"grip_v4_cm3": volume, "size_mm": [round(d, 1) for d in grip.dimensions]}
