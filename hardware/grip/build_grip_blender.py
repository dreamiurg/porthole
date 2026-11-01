"""Grip v6, built in Blender from Meshy's yoke and the build123d snap cradle, printed as two glued pieces.

Run inside Blender (e.g. through the Blender MCP: exec(open(path).read(), {"__file__": path})), or headless:
  Blender -b --factory-startup --python-expr "p='hardware/grip/build_grip_blender.py'; exec(open(p).read(), {'__file__': p})"
Inputs, next to this file:
  source/meshy_yoke_v2.glb   Meshy multi-image-to-3D of the "Yoke Pro" concept with a simplified centre
  stl/snap_test.stl          the snap cradle from grip_v1.py (fingers, key rib, ledge, root ring)
Outputs in stl/, each lying in its print orientation (cut face on the bed):
  grip_v6_front.stl, grip_v6_back.stl            the grip
  grip_v6_test_front.stl, grip_v6_test_back.stl  the right horn above the spoke, cut the same way

Frame as in puck_v1.py: origin = puck centre, +Z out of the screen, +Y = top of the screen, mm.
Meshy supplies only the handles and the bottom bar; the centre (cradle, two spokes, a pad to the bar)
is replaced by exact geometry so the puck seats without the generated ring, claws or lips.
The puck is charged out of the grip, so there is no USB access.

v4 after the first print: the handles were too thick for a kid's hand and too far from the screen for a
thumb, and their rounded backs hung in the air (spaghetti). Every surface is pulled in by INSET (thinner
tubes, same shape), the back is cut flat FLATTEN above its lowest point and set on the bed, and each half
moves PULL_IN towards the puck, the bottom bar shortened on both sides of its badge to match.

v5 after the v4 print: the flat back left sharp edges where it met the sides, so the back is fully round
again and prints on supports. The handles are thinner (INSET 3.5) with shallower grooves (SMOOTH), and PULL_IN drops
to 12 so fingers fit between the handles and the hub.

v6 after the v5 print: supports under the round back left a rough surface where fingers wrap. The v5 shape is
cut at SPLIT_Z into a front piece and a back skin, and each prints with its cut face on the bed, so no surface
needs support. The cut sits under the cradle's root ring, so the snap fingers stay one piece with their roots
and the front piece prints like v1-v5. The skin is up to 2 mm thick (thinner under the handles, whose backs
curve). Pins cut from 1.75 mm filament go into blind holes where the skin is full thickness, to line the pieces
up for gluing.
"""

import math
import os

import bmesh
import bpy
from mathutils import Matrix
from mathutils.bvhtree import BVHTree

HERE = os.path.dirname(os.path.abspath(__file__))
BACK_Z = -26.2  # print bed plane (grip_v1.BACK_Z)
ROOT_Z = -22.2  # cradle finger roots (grip_v1.ROOT_Z)
CUP_TOP = -12.6  # cradle rim (grip_v1.CUP_TOP)
WIDTH = 215.0  # overall grip width
DEPTH_SCALE = 1.45  # Meshy's model is shallow; with INSET this gives ~20 mm deep handles
INSET = 3.5  # mm off every surface: handles 31 -> 24 mm across
SMOOTH = 1.0  # closing radius: fills the grip grooves part way, so they stay but get shallow
PULL_IN = 12.0  # each half moves this far towards the puck: thumb closer to the screen, room left for fingers
BADGE_X = 17.0  # the bottom bar's centre (badge) stays; the bar is shortened outside it
VOXEL = 0.25  # distance-field resolution for the inset
SPLIT_Z = ROOT_Z - 2.0  # the cut: the front piece keeps 2 mm of root ring under the fingers
PIN_D = 2.0  # holes for 1.75 mm filament pins; printed holes come out ~0.1 mm small, so the pins sit snug
PIN_FRONT, PIN_BACK = 6.0, 1.3  # hole depths: cut the pins 7 mm long; the skin keeps a 0.45 mm roof or more
PINS = [(57.0, 21.0), (66.0, 52.0), (-57.0, 21.0), (-57.0, 47.0)]  # spokes and horn tops, where the skin is thickest (Meshy is not symmetric)
TEST_MIN = (51.0, 0.0)  # test piece: the right horn above the spoke, x > 51 and y > 0
TRIS = 220_000  # triangles in the whole grip before the cut, so each STL stays under 12 MB
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


def cylinder(r, z0, z1, x=0.0, y=0.0, vertices=256):
    bpy.ops.mesh.primitive_cylinder_add(radius=r, depth=z1 - z0, vertices=vertices, location=(x, y, (z0 + z1) / 2))
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


def sdf_offset(m, *distances):
    """Move every surface of m by each distance in turn (negative = inwards) through a signed distance
    field, re-meshing between steps so a dilate followed by an erode closes grooves (and the reverse
    rounds edges). An exact offset that never folds, unlike pushing vertices along their normals."""
    ng = bpy.data.node_groups.new("sdf_offset", "GeometryNodeTree")
    ng.interface.new_socket("Geometry", in_out="INPUT", socket_type="NodeSocketGeometry")
    ng.interface.new_socket("Geometry", in_out="OUTPUT", socket_type="NodeSocketGeometry")
    gin, gout = ng.nodes.new("NodeGroupInput"), ng.nodes.new("NodeGroupOutput")
    out = gin.outputs[0]
    for d in distances:
        sdf = ng.nodes.new("GeometryNodeMeshToSDFGrid")
        sdf.inputs["Voxel Size"].default_value = VOXEL
        sdf.inputs["Band Width"].default_value = int(abs(d) / VOXEL) + 4  # the band must reach the new surface
        off = ng.nodes.new("GeometryNodeSDFGridOffset")
        off.inputs["Distance"].default_value = d
        mesh = ng.nodes.new("GeometryNodeGridToMesh")
        ng.links.new(out, sdf.inputs["Mesh"])
        ng.links.new(sdf.outputs["SDF Grid"], off.inputs["Grid"])
        ng.links.new(off.outputs["Grid"], mesh.inputs["Grid"])
        out = mesh.outputs["Mesh"]
    ng.links.new(out, gout.inputs[0])
    select_only(m)
    mod = m.modifiers.new("sdf", "NODES")
    mod.node_group = ng
    bpy.ops.object.modifier_apply(modifier="sdf")
    bpy.data.node_groups.remove(ng)


def inset(m):
    """Make the grooves shallow, pull every surface in by INSET and set the lowest point on the bed. The
    handle backs stay fully round (ergonomics first), so the print needs supports under them."""
    sdf_offset(m, SMOOTH, -(SMOOTH + INSET))
    zmin = min(v.co.z for v in m.data.vertices)
    m.data.transform(Matrix.Translation((0, 0, BACK_Z - 0.3 - zmin)))  # the floor cut leaves a thin strip on the bed


def keep(m, name, x=(-400, 400), y=(-400, 400), z=(-400, 400)):
    """A copy of m clipped to the box x[0] < x < x[1], y[0] < y < y[1], z[0] < z < z[1]."""
    c = m.copy()
    c.data = m.data.copy()
    c.name = name
    SC.collection.objects.link(c)
    (x0, x1), (y0, y1) = x, y
    boolean(c, extrude(f"{name}_box", [(x0, y0), (x1, y0), (x1, y1), (x0, y1)], *z), "INTERSECT")
    return c


def pull_in(m):
    """Move each half PULL_IN towards the puck; cut the bar between the badge and the moved halves."""
    left = keep(m, "left", x=(-400, -(BADGE_X + PULL_IN - 0.5)))  # 0.5 overlap with the badge: no coincident faces
    right = keep(m, "right", x=(BADGE_X + PULL_IN - 0.5, 400))
    centre = keep(m, "centre", x=(-BADGE_X, BADGE_X))
    left.data.transform(Matrix.Translation((PULL_IN, 0, 0)))
    right.data.transform(Matrix.Translation((-PULL_IN, 0, 0)))
    bpy.data.objects.remove(m, do_unlink=True)
    boolean(centre, left, "UNION")
    boolean(centre, right, "UNION")
    return centre


def build_grip():
    m = import_meshy(os.path.join(HERE, "source", "meshy_yoke_v2.glb"))
    inset(m)
    # Meshy's centre out: its ring (r 56, r 53 above the bottom bar), the ring's top arc, its spokes
    centre = [polar(53.0 if -150 <= a <= -30 else 56.0, a) for a in range(-180, 180)]
    cuts = [centre, sector(51.5, 68, 35, 145), sector(51.5, 64, 25, 155)]
    cuts += both_sides([(46, 3), (69.5, 3), (69.5, 27), (63, 31.5), (55, 33.5), (46, 36)])
    for i, pts in enumerate(cuts):
        boolean(m, extrude(f"cut{i}", pts, -60, 60), "DIFFERENCE")
    m = pull_in(m)
    m.name = "grip_v6"
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
    clean(m)
    return m


def clean(ob):
    """Booleans and decimation leave a few zero-length edges that break the manifold."""
    bm = bmesh.new()
    bm.from_mesh(ob.data)
    bmesh.ops.dissolve_degenerate(bm, dist=1e-3, edges=bm.edges[:])
    bmesh.ops.remove_doubles(bm, verts=bm.verts, dist=1e-3)
    bm.to_mesh(ob.data)
    bm.free()


def cm3(ob):
    bm = bmesh.new()
    bm.from_mesh(ob.data)
    vol = bm.calc_volume() / 1000
    bm.free()
    return vol


def check(ob):
    bm = bmesh.new()
    bm.from_mesh(ob.data)
    bad = sum(1 for e in bm.edges if not e.is_manifold)
    bm.free()
    assert bad == 0, f"{ob.name}: {bad} non-manifold edges"
    return round(cm3(ob), 1)


def decimate(ob, tris):
    """Thin the dense distance-field mesh to about tris triangles, cleaned so it stays watertight."""
    ratio = tris / sum(len(p.vertices) - 2 for p in ob.data.polygons)
    if ratio < 1.0:
        select_only(ob)
        d = ob.modifiers.new("dec", "DECIMATE")
        d.ratio = ratio
        bpy.ops.object.modifier_apply(modifier="dec")
        clean(ob)


def pin_room(tree, x, y):
    """Skin under the cut and front piece over it, at a pin and 2 mm around it, by vertical rays."""
    skin, front = [], []
    for dx, dy in [(0.0, 0.0)] + [polar(2.0, a) for a in range(0, 360, 45)]:
        down = tree.ray_cast((x + dx, y + dy, SPLIT_Z), (0, 0, -1))[0]
        up = tree.ray_cast((x + dx, y + dy, SPLIT_Z), (0, 0, 1))[0]
        assert down and up, f"pin at {x}, {y} is off the grip"
        skin.append(SPLIT_Z - down.z)
        front.append(up.z - SPLIT_Z)
    return round(min(skin), 2), round(min(front), 1)


def split(m, pins):
    """Front piece and back skin, cut at SPLIT_Z, with blind pin holes facing each other."""
    front = keep(m, "front", z=(SPLIT_Z, 400))
    back = keep(m, "back", z=(-400, SPLIT_Z))
    for x, y in pins:
        boolean(front, cylinder(PIN_D / 2, SPLIT_Z - 1, SPLIT_Z + PIN_FRONT, x, y, 32), "DIFFERENCE")
        boolean(back, cylinder(PIN_D / 2, SPLIT_Z - PIN_BACK, SPLIT_Z + 1, x, y, 32), "DIFFERENCE")
    return front, back


def drop_specks(ob, min_cm3=0.05):
    """Where the back curves up through the cut, the skin thins to nothing and can leave loose specks."""
    select_only(ob)
    bpy.ops.object.mode_set(mode="EDIT")
    bpy.ops.mesh.select_all(action="SELECT")
    bpy.ops.mesh.separate(type="LOOSE")
    bpy.ops.object.mode_set(mode="OBJECT")
    parts = sorted(bpy.context.selected_objects, key=cm3, reverse=True)
    specks = [p for p in parts if cm3(p) < min_cm3]
    dropped = sum(cm3(p) for p in specks)
    for p in specks:
        bpy.data.objects.remove(p, do_unlink=True)
    keepers = parts[: len(parts) - len(specks)]
    select_only(keepers[0])
    for p in keepers[1:]:
        p.select_set(True)
    bpy.ops.object.join()
    return keepers[0], len(keepers), len(specks), round(dropped, 3)


def lay_flat(ob, flip):
    """Cut face down at z = 0 for printing; flip turns the skin over so its round back is on top."""
    if flip:
        ob.data.transform(Matrix.Rotation(math.pi, 4, "X"))
    ob.data.transform(Matrix.Translation((0, 0, -min(v.co.z for v in ob.data.vertices))))


grip = build_grip()
decimate(grip, TRIS)  # the distance-field mesh is dense; keeps each STL under 12 MB
volume = check(grip)
tree = BVHTree.FromObject(grip, bpy.context.evaluated_depsgraph_get())
room = {f"{x:g},{y:g}": pin_room(tree, x, y) for x, y in PINS}
for skin, over in room.values():
    assert skin >= PIN_BACK + 0.45 and over >= PIN_FRONT + 1.0, f"a pin has no room (skin, front): {room}"
front, back = split(grip, PINS)
test = {f"test_{p.name}": keep(p, f"test_{p.name}", x=(TEST_MIN[0], 400), y=(TEST_MIN[1], 400)) for p in (front, back)}
result = {"grip_v6_cm3": volume, "size_mm": [round(d, 1) for d in grip.dimensions], "pins_skin_front_mm": room}
for name, ob in {"front": front, "back": back, **test}.items():
    ob, parts, specks, dropped = drop_specks(ob)
    lay_flat(ob, flip=name.endswith("back"))
    result[name] = {"cm3": check(ob), "size_mm": [round(d, 1) for d in ob.dimensions], "parts": parts, "specks": (specks, dropped)}
    select_only(ob)
    bpy.ops.wm.stl_export(filepath=os.path.join(HERE, "stl", f"grip_v6_{name}.stl"), export_selected_objects=True)
print("RESULT", result)
