"""Renders of the steering-wheel grip with the puck in it, for the README and porthole.dreamiurg.net, plus light
copies of the grip halves for the site's 3D viewer.

Headless, from the repo root, with Blender 4.2+ or the bpy module (`pip install bpy`), Pillow alongside:
  python3 hardware/grip/render_grip_blender.py
Input:  stl/v8-kid/front.stl and stl/v8-kid/back.stl (each printed cut face down; reassembled here),
        the puck case STLs the site shows, and site/images/screen.png on the board's screen
Output: site/images/grip.webp (two views side by side), site/grip/front.stl and site/grip/back.stl (decimated)
"""

import math
import os

import bpy
from mathutils import Matrix, Vector

ROOT = os.path.abspath(os.path.join(os.path.dirname(os.path.abspath(__file__)), "..", ".."))
POCKET = 3.0  # puck pocket depth in the grip's front face (split_meshy_blender.py)
PUCK = ("hardware/case/stl/v1.2/puck_bezel.stl", "hardware/case/stl/v1.2/puck_plate.stl", "hardware/case/stl/v1.2/puck_lid.stl")
WEB_TRIS = 30_000  # per half in the site's viewer: about 1.5 MB each
SC = None  # the scene, set once main() has reset Blender


def material(name, rgb, rough, emit=None):
    m = bpy.data.materials.new(name)
    m.use_nodes = True
    p = m.node_tree.nodes["Principled BSDF"]
    p.inputs["Base Color"].default_value = (*rgb, 1)
    p.inputs["Roughness"].default_value = rough
    if emit:
        tex = m.node_tree.nodes.new("ShaderNodeTexImage")
        tex.image = bpy.data.images.load(emit)
        m.node_tree.links.new(tex.outputs["Color"], p.inputs["Emission Color"])
        p.inputs["Emission Strength"].default_value = 1.0
        p.inputs["Base Color"].default_value = (0, 0, 0, 1)
    return m


def stl(path, mat):
    bpy.ops.wm.stl_import(filepath=os.path.join(ROOT, path))
    o = bpy.context.selected_objects[0]
    o.data.materials.append(mat)
    return o


def disc(r, z0, z1, mat):
    bpy.ops.mesh.primitive_cylinder_add(vertices=128, radius=r, depth=z1 - z0, location=(0, 0, (z0 + z1) / 2))
    o = bpy.context.active_object
    o.data.materials.append(mat)
    return o


def export_web(o, path):
    """A decimated copy of one half, in its assembled place, for the site's three.js viewer."""
    c = o.copy()
    c.data = o.data.copy()
    SC.collection.objects.link(c)
    d = c.modifiers.new("d", "DECIMATE")
    d.ratio = min(1.0, WEB_TRIS / len(c.data.polygons))
    for x in SC.objects:
        x.select_set(x is c)
    bpy.context.view_layer.objects.active = c
    bpy.ops.object.modifier_apply(modifier="d")
    os.makedirs(os.path.dirname(path), exist_ok=True)
    bpy.ops.wm.stl_export(filepath=path, export_selected_objects=True, ascii_format=False)
    bpy.data.objects.remove(c)


def shot(path, loc, look_at, lens):
    cam = SC.camera
    cam.location = Vector(loc)
    cam.rotation_euler = (Vector(look_at) - cam.location).to_track_quat("-Z", "Y").to_euler()
    cam.data.lens = lens
    SC.render.filepath = path
    bpy.ops.render.render(write_still=True)


def main():
    global SC
    bpy.ops.wm.read_factory_settings(use_empty=True)
    SC = bpy.context.scene
    black = material("wheel", (0.018, 0.018, 0.02), 0.45)
    front = stl("hardware/grip/stl/v8-kid/front.stl", black)
    back = stl("hardware/grip/stl/v8-kid/back.stl", black)
    back.data.transform(Matrix.Rotation(math.pi, 4, "X"))  # undo the print flip: halves meet at z = 0
    hub = max(v.co.z for v in front.data.vertices if math.hypot(v.co.x, v.co.y) < 45)
    lift = hub - POCKET + 20.2  # the lid's underside (z -20.2 in the case frame) on the pocket floor

    white = material("puck", (0.86, 0.85, 0.82), 0.5)
    glass = material("glass", (0.02, 0.02, 0.025), 0.15)
    screen = material("screen", (0, 0, 0), 0.3, emit=os.path.join(ROOT, "site/images/screen.png"))
    parts = [stl(p, white) for p in PUCK] + [disc(37.5, 1.2, 3.4, glass)]
    bpy.ops.mesh.primitive_circle_add(vertices=128, radius=26.7, fill_type="NGON", location=(0, 0, 3.42))
    face = bpy.context.active_object
    bpy.ops.object.mode_set(mode="EDIT")
    bpy.ops.uv.cube_project(cube_size=53.4)
    bpy.ops.object.mode_set(mode="OBJECT")
    face.data.materials.append(screen)
    for o in parts + [face]:
        o.location.z += lift

    world = bpy.data.worlds.new("w")
    world.use_nodes = True
    world.node_tree.nodes["Background"].inputs["Color"].default_value = (0.8, 0.8, 0.82, 1)
    world.node_tree.nodes["Background"].inputs["Strength"].default_value = 0.6
    SC.world = world
    for rot, power in (((0.7, 0.1, 0.5), 3.0), ((1.2, -0.3, -2.4), 1.2)):
        sun = bpy.data.objects.new("sun", bpy.data.lights.new("sun", "SUN"))
        sun.data.energy = power
        sun.rotation_euler = rot
        SC.collection.objects.link(sun)
    SC.camera = bpy.data.objects.new("cam", bpy.data.cameras.new("cam"))
    SC.collection.objects.link(SC.camera)
    SC.render.engine = "CYCLES"
    SC.cycles.device = "CPU"
    SC.cycles.samples = 96
    SC.render.film_transparent = True
    SC.render.resolution_x, SC.render.resolution_y = 600, 600
    SC.view_settings.view_transform = "Standard"

    tmp = os.path.join(ROOT, "hardware/grip/.render")
    os.makedirs(tmp, exist_ok=True)
    a, b = os.path.join(tmp, "a.png"), os.path.join(tmp, "b.png")
    shot(a, (120, -210, 180), (0, 0, 6), 50)  # three-quarter view
    shot(b, (0, -70, 290), (0, 0, 6), 50)  # straight on, as the kid holds it

    export_web(front, os.path.join(ROOT, "site/grip/front.stl"))
    export_web(back, os.path.join(ROOT, "site/grip/back.stl"))

    from PIL import Image

    out = Image.new("RGB", (1200, 600), (214, 214, 218))
    for i, p in enumerate((a, b)):
        im = Image.open(p).convert("RGBA")
        out.paste(im, (600 * i, 0), im)
        os.remove(p)
    os.rmdir(tmp)
    out.save(os.path.join(ROOT, "site/images/grip.webp"), quality=86)
    print(f"grip lift under the puck: {lift:.2f} mm")
    print("wrote site/images/grip.webp, site/grip/front.stl, site/grip/back.stl")


if __name__ == "__main__":
    main()
