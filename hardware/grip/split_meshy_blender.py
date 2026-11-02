"""Grip v7 hold test: Meshy's yoke as modelled, cut in half through its thickness, to print and glue.

Headless:
  Blender -b --factory-startup --python-expr "p='hardware/grip/split_meshy_blender.py'; exec(open(p).read(), {'__file__': p})"
Input:  source/meshy_yoke_v3.glb (Meshy "Kids Game Yoke Grip CAD Ready", task 2a61e941)
Output: stl/grip_v7_front.stl, stl/grip_v7_back.stl, each with its cut face on the bed,
        stl/grip_v7_spacer.stl, the tube that holds the magnet ring up against the front

The mesh is scaled to WIDTH and otherwise left alone, cut at half its depth so each half prints as a dome
without support. Pins cut from 1.75 mm filament go into blind holes at the ends of both handles to line the
halves up for gluing.

The puck sticks on magnetically: an aftermarket MagSafe magnet ring (RING_OD x RING_ID x RING_T) sits under
the centre disc's flat face, behind COVER of plastic printed with the front half, so nothing shows outside.
The ring goes in from the cut side through an annular trench, its phone side towards the face, and the
spacer tube under it holds it there when the back half is glued on.
"""

import math
import os

import bmesh
import bpy
from mathutils import Matrix
from mathutils.bvhtree import BVHTree

HERE = os.path.dirname(os.path.abspath(__file__))
WIDTH = 215.0  # same overall width as v5/v6, which were held
TRIS = 200_000  # whole grip before the cut, so each STL stays under 12 MB
PIN_D = 2.0  # holes for 1.75 mm filament pins; printed holes come out ~0.1 mm small
PIN_DEPTH = 5.0  # each side: cut the pins 9 mm long
PIN_WALL = 1.5  # material kept past the hole end and around it
RING_OD, RING_ID, RING_T = 56.0, 46.0, 1.5  # MagSafe magnet ring as sold (56 x 46 x 1.5 mm N52)
RING_FIT = 0.2  # clearance on each wall of the trench
COVER = 0.8  # plastic between the ring and the face: thin enough to keep the pull, 4 layers to bridge
SC = bpy.context.scene


def select_only(ob):
    for o in bpy.context.selected_objects:
        o.select_set(False)
    bpy.context.view_layer.objects.active = ob
    ob.select_set(True)


def import_yoke(path):
    bpy.ops.import_scene.gltf(filepath=path)
    meshes = [o for o in SC.objects if o.type == "MESH"]
    select_only(meshes[0])
    for o in meshes:
        o.select_set(True)
    bpy.ops.object.join()
    m = bpy.context.active_object
    m.parent = None
    bpy.ops.object.transform_apply(location=True, rotation=True, scale=True)
    m.data.transform(Matrix.Rotation(math.radians(-90), 4, "X"))  # glTF Y-up -> face of the wheel +Z
    vs = [v.co for v in m.data.vertices]
    lo = [min(c[i] for c in vs) for i in range(3)]
    hi = [max(c[i] for c in vs) for i in range(3)]
    m.data.transform(Matrix.Scale(WIDTH / (hi[0] - lo[0]), 4) @ Matrix.Translation([-(a + b) / 2 for a, b in zip(lo, hi, strict=True)]))
    m.data.update()
    return m


def decimate(ob, tris):
    ratio = tris / sum(len(p.vertices) - 2 for p in ob.data.polygons)
    if ratio < 1.0:
        select_only(ob)
        d = ob.modifiers.new("dec", "DECIMATE")
        d.ratio = ratio
        bpy.ops.object.modifier_apply(modifier="dec")
        clean(ob)


def clean(ob):
    """Decimation and booleans leave a few zero-length edges that break the manifold."""
    bm = bmesh.new()
    bm.from_mesh(ob.data)
    bmesh.ops.dissolve_degenerate(bm, dist=1e-3, edges=bm.edges[:])
    bmesh.ops.remove_doubles(bm, verts=bm.verts, dist=1e-3)
    bm.to_mesh(ob.data)
    bm.free()


def thickness(tree, x, y):
    """Material above and below the cut (z = 0) at x, y and on a ring around the hole, by vertical rays."""
    up, down = [], []
    for a in range(0, 360, 45):
        dx, dy = (PIN_D / 2 + PIN_WALL) * math.cos(math.radians(a)), (PIN_D / 2 + PIN_WALL) * math.sin(math.radians(a))
        for px, py in ((x, y), (x + dx, y + dy)):
            hu, hd = tree.ray_cast((px, py, 0), (0, 0, 1))[0], tree.ray_cast((px, py, 0), (0, 0, -1))[0]
            if not (hu and hd):
                return 0.0
            up.append(hu.z)
            down.append(-hd.z)
    return min(up + down)


def pin_spots(m):
    """Top and bottom end of each handle where both halves have room for the hole."""
    tree = BVHTree.FromObject(m, bpy.context.evaluated_depsgraph_get())
    need = PIN_DEPTH + PIN_WALL
    ok = [(x, y) for x in range(-110, 111, 2) for y in range(-80, 81, 2) if abs(x) > 60 and thickness(tree, x, y) >= need]
    spots = []
    for side in (-1, 1):
        mine = [p for p in ok if p[0] * side > 0]
        assert mine, f"no room for a pin in the {'left' if side < 0 else 'right'} handle"
        spots += [max(mine, key=lambda p: p[1]), min(mine, key=lambda p: p[1])]
    return spots


def subtract(ob, tool):
    select_only(ob)
    mod = ob.modifiers.new("b", "BOOLEAN")
    mod.operation, mod.object, mod.solver = "DIFFERENCE", tool, "MANIFOLD"
    bpy.ops.object.modifier_apply(modifier="b")
    bpy.data.objects.remove(tool, do_unlink=True)


def cylinder(r, z0, z1, x=0.0, y=0.0, vertices=256):
    bpy.ops.mesh.primitive_cylinder_add(radius=r, depth=z1 - z0, vertices=vertices, location=(x, y, (z0 + z1) / 2))
    return bpy.context.active_object


def tube(r_in, r_out, z0, z1, x, y):
    t = cylinder(r_out, z0, z1, x, y)
    subtract(t, cylinder(r_in, z0 - 1, z1 + 1, x, y))
    return t


def ring_seat(m):
    """Centre of the flat centre disc and the height of the trench roof, COVER under its face."""
    tree = BVHTree.FromObject(m, bpy.context.evaluated_depsgraph_get())

    def face(x, y):
        hit = tree.ray_cast((x, y, 100), (0, 0, -1))[0]
        return hit.z if hit else -100

    top = face(0, 0)

    def edge(dx, dy):
        r = 0.0
        while face(r * dx, r * dy) > top - 0.3:
            r += 0.25
        return r

    cx, cy = (edge(1, 0) - edge(-1, 0)) / 2, (edge(0, 1) - edge(0, -1)) / 2
    r_out = RING_OD / 2 + RING_FIT
    assert min(edge(1, 0), edge(-1, 0), edge(0, 1), edge(0, -1)) > r_out + 3, "the centre disc is too small for the ring"
    heights = [face(cx + r * math.cos(math.radians(a)), cy + r * math.sin(math.radians(a))) for r in (RING_ID / 2, r_out) for a in range(0, 360, 15)]
    assert max(heights) - min(heights) < 0.3, f"the disc face under the ring is not flat: {min(heights):.2f}..{max(heights):.2f}"
    roof = min(heights) - COVER
    assert roof > RING_T + 2, "the front half is too thin for the ring"
    return round(cx, 2), round(cy, 2), round(roof, 2)


def half(m, name, keep_top, pins, seat):
    h = m.copy()
    h.data = m.data.copy()
    h.name = name
    SC.collection.objects.link(h)
    bm = bmesh.new()
    bm.from_mesh(h.data)
    cut = bmesh.ops.bisect_plane(bm, geom=bm.verts[:] + bm.edges[:] + bm.faces[:], plane_co=(0, 0, 0), plane_no=(0, 0, 1), clear_inner=keep_top, clear_outer=not keep_top)
    bmesh.ops.holes_fill(bm, edges=[e for e in cut["geom_cut"] if isinstance(e, bmesh.types.BMEdge)])
    bmesh.ops.triangulate(bm, faces=bm.faces[:])
    bm.to_mesh(h.data)
    bm.free()
    for x, y in pins:
        subtract(h, cylinder(PIN_D / 2, -PIN_DEPTH, PIN_DEPTH, x, y, 32))
    if keep_top:
        cx, cy, roof = seat
        subtract(h, tube(RING_ID / 2 - RING_FIT, RING_OD / 2 + RING_FIT, -1, roof, cx, cy))
    clean(h)
    if not keep_top:
        h.data.transform(Matrix.Rotation(math.pi, 4, "X"))  # back half: cut face down, round back up
    h.data.transform(Matrix.Translation((0, 0, -min(v.co.z for v in h.data.vertices))))
    return h


def report(ob):
    bm = bmesh.new()
    bm.from_mesh(ob.data)
    bad = sum(1 for e in bm.edges if not e.is_manifold)
    bodies, seen = 0, set()
    for f in bm.faces:
        if f.index in seen:
            continue
        bodies += 1
        stack = [f]
        while stack:
            g = stack.pop()
            if g.index in seen:
                continue
            seen.add(g.index)
            stack += [n for e in g.edges for n in e.link_faces if n.index not in seen]
    overhang = sum(f.calc_area() for f in bm.faces if f.normal.z < -math.cos(math.radians(45)) and f.calc_center_median().z > 0.2)
    vol = bm.calc_volume() / 1000
    bm.free()
    assert bad == 0 and bodies == 1, f"{ob.name}: {bad} non-manifold edges, {bodies} bodies"
    return {"cm3": round(vol, 1), "size_mm": [round(d, 1) for d in ob.dimensions], "overhang_mm2": round(overhang)}


for o in list(SC.objects):
    bpy.data.objects.remove(o, do_unlink=True)
yoke = import_yoke(os.path.join(HERE, "source", "meshy_yoke_v3.glb"))
decimate(yoke, TRIS)
pins = pin_spots(yoke)
seat = ring_seat(yoke)
result = {"size_mm": [round(d, 1) for d in yoke.dimensions], "pins": pins, "ring_centre_and_roof": seat}
parts = {name: half(yoke, name, top, pins, seat) for name, top in (("front", True), ("back", False))}
parts["spacer"] = tube(RING_ID / 2, RING_OD / 2, 0, seat[2] - RING_T - 0.3, 0, 0)  # 0.3 short so the halves close
for name, ob in parts.items():
    result[name] = report(ob)
    select_only(ob)
    bpy.ops.wm.stl_export(filepath=os.path.join(HERE, "stl", f"grip_v7_{name}.stl"), export_selected_objects=True)
print("RESULT", result)
