"""Grip v7 hold test: Meshy's yoke as modelled, cut in half through its thickness, to print and glue.

Headless:
  Blender -b --factory-startup --python-expr "p='hardware/grip/split_meshy_blender.py'; exec(open(p).read(), {'__file__': p})"
Input:  source/meshy_yoke_v3.glb (Meshy "Kids Game Yoke Grip CAD Ready", task 2a61e941)
Output: stl/grip_v7_front.stl, stl/grip_v7_back.stl, each with its cut face on the bed,
        stl/grip_v7_cover.stl, the lid over the steel ring, and stl/grip_v7_pin.stl, one alignment pin on its side;
        stl/grip_v8_kid_front.stl and stl/grip_v8_kid_back.stl, the same grip at 80% (ring, cover and pins unscaled)
        with a puck pocket sunk into its centre

The mesh is scaled to each variant's width and otherwise left alone, cut at half its depth so each half prints as a dome
without support. Square printed pins go into blind square holes at the ends of both handles to line the
halves up for gluing; a square pin prints lying on a side, so its faces come out flat and true.

The puck sticks on magnetically: it carries its own MagSafe magnet ring and Qi coil, like an iPhone, and the
grip holds an aftermarket steel MagSafe ring (RING_OD x RING_ID x RING_T, not a magnet, so it has no polarity).
The ring lies in a round recess in the centre disc's flat face, under a printed cover that sits flush with
the face.
"""

import math
import os

import bmesh
import bpy
from mathutils import Matrix
from mathutils.bvhtree import BVHTree

HERE = os.path.dirname(os.path.abspath(__file__))
# name, overall width, puck pocket depth: v7 matches v5/v6 (adult hands); v8 is v7 at 80% for kids, its centre
# sunk POCKET deep over the puck's whole footprint (wider than the scaled hub) so the puck sits lower
VARIANTS = (("grip_v7", 215.0, 0.0), ("grip_v8_kid", 172.0, 3.0))
PUCK_D, PUCK_FIT = 84.0, 0.3  # puck case v1.3 outer diameter; radial clearance in the pocket
TRIS = 200_000  # whole grip before the cut, so each STL stays under 12 MB
PIN_HOLE = 4.0  # square hole side; printed holes come out ~0.1 mm small
PIN_SIDE = 3.8  # square pin side: snug in the printed hole
PIN_DEPTH = 5.0  # hole depth in each half; the pin is 0.6 mm shorter than both holes so the halves close
PIN_WALL = 1.5  # material kept past the hole end and around it
RING_OD, RING_ID, RING_T = 56.0, 46.0, 0.5  # steel MagSafe ring sticker as sold (56 x 46 x 0.5 mm)
RING_FIT = 0.2  # clearance between the recess wall and the ring or the cover
COVER = 0.8  # cover over the ring: 4 layers, thin enough to keep the puck's pull
SC = bpy.context.scene


def select_only(ob):
    for o in bpy.context.selected_objects:
        o.select_set(False)
    bpy.context.view_layer.objects.active = ob
    ob.select_set(True)


def import_yoke(path, width):
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
    m.data.transform(Matrix.Scale(width / (hi[0] - lo[0]), 4) @ Matrix.Translation([-(a + b) / 2 for a, b in zip(lo, hi, strict=True)]))
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
        r = PIN_HOLE / math.sqrt(2) + PIN_WALL
        dx, dy = r * math.cos(math.radians(a)), r * math.sin(math.radians(a))
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


def box(sx, sy, z0, z1, x=0.0, y=0.0):
    bpy.ops.mesh.primitive_cube_add(size=1, location=(x, y, (z0 + z1) / 2), scale=(sx, sy, z1 - z0))
    ob = bpy.context.active_object
    bpy.ops.object.transform_apply(scale=True)
    return ob


def ring_seat(m):
    """Centre of the flat centre disc and the height of its face."""
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
    face = min(heights)
    assert face - RING_T - COVER > 3, "the front half is too thin for the ring recess"
    return round(cx, 2), round(cy, 2), round(face, 2)


def half(m, name, keep_top, pins, seat, pocket):
    h = m.copy()
    h.data = m.data.copy()
    h.name = name
    SC.collection.objects.link(h)
    # a boolean, not bisect + hole fill: filling the cut's loops one by one also skins over the windows
    subtract(h, cylinder(400, -200, 0) if keep_top else cylinder(400, 0, 200))
    for x, y in pins:
        subtract(h, box(PIN_HOLE, PIN_HOLE, -PIN_DEPTH, PIN_DEPTH, x, y))
    if keep_top:
        cx, cy, face = seat
        floor = face - pocket
        if pocket:
            subtract(h, cylinder(PUCK_D / 2 + PUCK_FIT, floor, face + 20, cx, cy))
        subtract(h, cylinder(RING_OD / 2 + RING_FIT, floor - RING_T - COVER, floor + 5, cx, cy))
    clean(h)
    if not keep_top:
        h.data.transform(Matrix.Rotation(math.pi, 4, "X"))  # back half: cut face down, round back up
    h.data.transform(Matrix.Translation((0, 0, -min(v.co.z for v in h.data.vertices))))
    return h


def films(ob):
    """Grid points where a vertical ray crosses the surface an odd number of times: a skin with nothing behind it."""
    tree = BVHTree.FromObject(ob, bpy.context.evaluated_depsgraph_get())
    lo, hi = [min(v.co[i] for v in ob.data.vertices) for i in range(2)], [max(v.co[i] for v in ob.data.vertices) for i in range(2)]
    odd = 0
    for x in range(int(lo[0]), int(hi[0]) + 1, 2):
        for y in range(int(lo[1]), int(hi[1]) + 1, 2):
            z, hits = 1000.0, 0
            while (hit := tree.ray_cast((x + 0.37, y + 0.53, z), (0, 0, -1))[0]) is not None:
                hits, z = hits + 1, hit.z - 1e-3
            odd += hits % 2
    return odd


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
    assert (odd := films(ob)) == 0, f"{ob.name}: {odd} spots with a zero-thickness skin"
    return {"cm3": round(vol, 1), "size_mm": [round(d, 1) for d in ob.dimensions], "overhang_mm2": round(overhang)}


def build(tag, width, pocket, extras):
    for o in list(SC.objects):
        bpy.data.objects.remove(o, do_unlink=True)
    yoke = import_yoke(os.path.join(HERE, "source", "meshy_yoke_v3.glb"), width)
    decimate(yoke, TRIS)
    pins = pin_spots(yoke)
    seat = ring_seat(yoke)  # ring, recess and cover keep their size at every scale
    assert seat[2] - pocket - RING_T - COVER > 3, f"{tag}: the pocket leaves too little under the ring"
    result = {"size_mm": [round(d, 1) for d in yoke.dimensions], "pins": pins, "ring_centre_and_face": seat}
    parts = {name: half(yoke, name, top, pins, seat, pocket) for name, top in (("front", True), ("back", False))}
    if extras:  # cover and pin do not scale: the v7 files serve every variant
        parts["cover"] = cylinder(RING_OD / 2, 0, COVER)
        parts["pin"] = box(2 * PIN_DEPTH - 0.6, PIN_SIDE, 0, PIN_SIDE)  # lying on a side
    for name, ob in parts.items():
        result[name] = report(ob)
        select_only(ob)
        bpy.ops.wm.stl_export(filepath=os.path.join(HERE, "stl", f"{tag}_{name}.stl"), export_selected_objects=True)
    print("RESULT", tag, result)


for i, (tag, width, pocket) in enumerate(VARIANTS):
    build(tag, width, pocket, extras=i == 0)
