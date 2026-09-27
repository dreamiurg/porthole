"""The Porthole puck, rendered: the real board (Waveshare's STEP) inside the printed case, its screen playing
frames from the simulator. Runs inside Blender, headless:

  blender -b -P tools/promo/scene.py -- --stl DIR --frames DIR [--look NAME] [--shot NAME]
                                        [--frame N | --anim [--from N]] --out PATH [--samples N] [--smooth]

Every model is in the board's STEP frame, millimetres: origin = board centre, +Z out of the screen, +Y = screen
up, USB-C at 5 and 7 o'clock, the power slider at 3 o'clock. The case STLs import at 0.001 as object scale (object
space stays in mm); the scene is in metres. Assets come from tools/promo/fetch.sh.
"""

import argparse
import glob
import math
import os
import sys

import bmesh
import bpy
from mathutils import Vector

ROOT = os.path.dirname(os.path.dirname(os.path.dirname(os.path.abspath(__file__))))
ASSETS = os.path.join(ROOT, "build", "promo", "assets")
MM = 0.001
GLASS_R, GLASS_TOP = 37.5 * MM, 3.4 * MM
ACTIVE_D = 53.34 * MM  # 2.1-inch round panel, active area
R_OUT = 41.6 * MM

# A look = the table, the case plastic, the light. Colours are linear albedo.
LOOKS: dict[str, dict] = {  # values are mixed: numbers, colours, names
    "oak-day": dict(
        table="oak_veneer_01", tile=6, hdri="studio_small_08", sky=0.5, case=(0.70, 0.67, 0.62), rough=0.5, key=(30, (1.0, 0.94, 0.86)), fill=6, rim=10, screen=1.0
    ),
    "walnut-night": dict(
        table="dark_wood", tile=4, hdri="fireplace", sky=0.12, case=(0.035, 0.035, 0.04), rough=0.45, key=(6, (1.0, 0.72, 0.45)), fill=0.6, rim=5, screen=1.6
    ),
    "concrete-sage": dict(
        table="concrete_floor_02", tile=3, hdri="studio_small_08", sky=0.35, case=(0.26, 0.34, 0.26), rough=0.62, key=(22, (1.0, 0.97, 0.92)), fill=4, rim=8, screen=1.1
    ),
    "clear-slate": dict(
        table="concrete_floor_worn_001",
        tile=3,
        hdri="studio_small_03",
        sky=0.4,
        case=(0.9, 0.93, 0.95),
        rough=0.15,
        clear=True,
        key=(20, (1.0, 0.97, 0.94)),
        fill=4,
        rim=14,
        screen=1.2,
    ),
}

a = argparse.ArgumentParser()
a.add_argument("--stl", required=True)
a.add_argument("--frames", required=True)
a.add_argument("--look", default="oak-day", choices=sorted(LOOKS))
a.add_argument("--shot", default="orbit", choices=("hero", "macro", "orbit", "exploded"))
a.add_argument("--frame", type=int, default=1)
a.add_argument("--anim", action="store_true")
a.add_argument("--from", dest="start", type=int, default=1)  # resume an interrupted --anim
a.add_argument("--out", required=True)
a.add_argument("--samples", type=int, default=96)
a.add_argument("--res", type=int, default=1080)
a.add_argument("--smooth", action="store_true")  # Biscuit: native 480 art, linear filtering
args = a.parse_args(sys.argv[sys.argv.index("--") + 1 :])
LOOK = LOOKS[args.look]

bpy.ops.wm.read_factory_settings(use_empty=True)
sc = bpy.context.scene


def link(nt, out, inp):
    nt.links.new(out, inp)


def principled(name, **inputs):
    m = bpy.data.materials.new(name)
    b = m.node_tree.nodes["Principled BSDF"]
    for k, v in inputs.items():
        b.inputs[k].default_value = v
    return m, m.node_tree, b


# ---- case: printed PETG with 0.2 mm layer lines (a bump in object space, which is mm) ------------------------
def case_material():
    col = LOOK["case"] + (1.0,)
    if LOOK.get("clear"):  # frosted translucent PETG: the board shows through
        m, nt, b = principled("PETG", **{"Base Color": col, "Roughness": LOOK["rough"], "Transmission Weight": 1.0, "IOR": 1.57})
    else:
        m, nt, b = principled("PETG", **{"Base Color": col, "Roughness": LOOK["rough"], "Subsurface Weight": 0.05})
    wave = nt.nodes.new("ShaderNodeTexWave")
    wave.bands_direction = "Z"
    wave.inputs["Scale"].default_value = 2 * math.pi / (20 * 0.2)  # sin(20 * scale * z_mm): a 0.2 mm period
    bump = nt.nodes.new("ShaderNodeBump")
    bump.inputs["Strength"].default_value = 1.0
    bump.inputs["Distance"].default_value = 0.05
    link(nt, nt.nodes.new("ShaderNodeTexCoord").outputs["Object"], wave.inputs["Vector"])
    link(nt, wave.outputs["Fac"], bump.inputs["Height"])
    link(nt, bump.outputs["Normal"], b.inputs["Normal"])
    return m


petg = case_material()
case = {}
for part in ("ring", "cup", "plate", "slider"):
    bpy.ops.wm.stl_import(filepath=os.path.join(args.stl, f"puck_{part}.stl"), global_scale=MM)
    o = bpy.context.selected_objects[0]
    o.data.materials.append(petg)
    bpy.ops.object.shade_auto_smooth(angle=math.radians(35))
    case[part] = o

# ---- the board: Waveshare's STEP as GLB (glTF is Y-up; -90 deg about X puts it back in the STEP frame) -------
board = bpy.data.objects.new("board", None)
sc.collection.objects.link(board)
bpy.ops.import_scene.gltf(filepath=os.path.join(ASSETS, "board.glb"))
for o in bpy.context.selected_objects:
    if o.parent is None:
        o.parent = board
board.rotation_euler.x = math.radians(-90)
PMMA, PCB = "mat_35", "mat_15"  # the STEP's acrylic spacer (drawn magenta) and the soldermask (navy)
for m in bpy.data.materials:  # the STEP gives flat colours only: turn them into plausible surfaces
    b = m.node_tree.nodes.get("Principled BSDF") if m.node_tree else None
    if not b or not m.name.startswith("mat_"):
        continue
    r, g, bl = b.inputs["Base Color"].default_value[:3]
    b.inputs["Roughness"].default_value = 0.45
    if m.name == PMMA:
        b.inputs["Base Color"].default_value = (0.95, 0.97, 1.0, 1)
        b.inputs["Transmission Weight"].default_value, b.inputs["Roughness"].default_value = 1.0, 0.05
    elif m.name == PCB:
        b.inputs["Roughness"].default_value, b.inputs["Coat Weight"].default_value = 0.3, 0.4
    elif max(r, g, bl) - min(r, g, bl) < 0.03 and 0.5 <= r <= 0.9:  # neutral greys: tin, shields, pins
        b.inputs["Metallic"].default_value, b.inputs["Roughness"].default_value = 1.0, 0.3

# ---- screen: emission under a glossy coat, just above the panel's own glass ----------------------------------
bm = bmesh.new()
bmesh.ops.create_circle(bm, cap_ends=True, radius=GLASS_R - 0.2 * MM, segments=256)
uv = bm.loops.layers.uv.new()
for face in bm.faces:
    for loop in face.loops:
        loop[uv].uv = (loop.vert.co.x / ACTIVE_D + 0.5, loop.vert.co.y / ACTIVE_D + 0.5)
me = bpy.data.meshes.new("screen")
bm.to_mesh(me)
screen = bpy.data.objects.new("screen", me)
screen.location.z = GLASS_TOP + 0.02 * MM
sc.collection.objects.link(screen)

sm = bpy.data.materials.new("screen")
nt = sm.node_tree
nt.nodes.remove(nt.nodes["Principled BSDF"])
files = sorted(glob.glob(os.path.join(args.frames, "f*.bmp")))
img = bpy.data.images.load(files[0])
img.source = "SEQUENCE" if len(files) > 1 else "FILE"  # a lone still is not a sequence
tex = nt.nodes.new("ShaderNodeTexImage")
tex.image = img
tex.interpolation = "Linear" if args.smooth else "Closest"  # Pets Club pixels stay square
tex.extension = "CLIP"  # black outside the active area
tex.image_user.frame_duration = len(files)
tex.image_user.use_auto_refresh = True
emi = nt.nodes.new("ShaderNodeEmission")
emi.inputs["Strength"].default_value = LOOK["screen"]
coat = nt.nodes.new("ShaderNodeBsdfPrincipled")
coat.inputs["Base Color"].default_value = (0.004, 0.004, 0.005, 1)
coat.inputs["Roughness"].default_value = 0.06
add = nt.nodes.new("ShaderNodeAddShader")
link(nt, tex.outputs["Color"], emi.inputs["Color"])
link(nt, emi.outputs[0], add.inputs[0])
link(nt, coat.outputs[0], add.inputs[1])
link(nt, add.outputs[0], nt.nodes["Material Output"].inputs["Surface"])
screen.data.materials.append(sm)
bpy.context.view_layer.update()
screen.parent, screen.matrix_parent_inverse = board, board.matrix_world.inverted()  # moves with the board

# ---- exploded: the stack pulled apart along Z, in assembly order ---------------------------------------------
if args.shot == "exploded":
    for obj, dz in ((case["ring"], 40), (board, 20), (case["plate"], 4), (case["slider"], -12), (case["cup"], -12)):
        obj.location.z += dz * MM
bpy.context.view_layer.update()
bottom = min((o.matrix_world @ v.co).z for o in case.values() for v in o.data.vertices)

# ---- table ---------------------------------------------------------------------------------------------------
bpy.ops.mesh.primitive_plane_add(size=3, location=(0, 0, bottom))
table = bpy.context.object
tm, nt, b = principled("table")
mp = nt.nodes.new("ShaderNodeMapping")
mp.inputs["Scale"].default_value = (LOOK["tile"],) * 3
mp.inputs["Rotation"].default_value = (0, 0, math.radians(90))
link(nt, nt.nodes.new("ShaderNodeTexCoord").outputs["UV"], mp.inputs["Vector"])


def table_map(kind, colorspace):
    t = nt.nodes.new("ShaderNodeTexImage")
    t.image = bpy.data.images.load(os.path.join(ASSETS, f"{LOOK['table']}_{kind}.jpg"))
    t.image.colorspace_settings.name = colorspace
    link(nt, mp.outputs["Vector"], t.inputs["Vector"])
    return t.outputs["Color"]


link(nt, table_map("Diffuse", "sRGB"), b.inputs["Base Color"])
link(nt, table_map("Rough", "Non-Color"), b.inputs["Roughness"])
nmap = nt.nodes.new("ShaderNodeNormalMap")
link(nt, table_map("nor_gl", "Non-Color"), nmap.inputs["Color"])
link(nt, nmap.outputs["Normal"], b.inputs["Normal"])
table.data.materials.append(tm)

# ---- light: the look's HDRI, a soft key upper left, a fill right, a rim behind --------------------------------
world = bpy.data.worlds.new("w")
sc.world = world
env = world.node_tree.nodes.new("ShaderNodeTexEnvironment")
env.image = bpy.data.images.load(os.path.join(ASSETS, f"{LOOK['hdri']}_hdri.hdr"))
world.node_tree.nodes["Background"].inputs["Strength"].default_value = LOOK["sky"]
link(world.node_tree, env.outputs["Color"], world.node_tree.nodes["Background"].inputs["Color"])
aim_at = bpy.data.objects.new("aim", None)
sc.collection.objects.link(aim_at)


def area(name, loc, size, watts, color):
    bpy.ops.object.light_add(type="AREA", location=loc)
    lt = bpy.context.object
    lt.name, lt.data.size, lt.data.energy, lt.data.color = name, size, watts, color
    lt.constraints.new("TRACK_TO").target = aim_at


area("key", (-0.35, -0.2, 0.45), 0.6, *LOOK["key"])
area("fill", (0.4, -0.1, 0.2), 0.5, LOOK["fill"], (0.86, 0.92, 1.0))
area("rim", (0.1, 0.45, 0.25), 0.3, LOOK["rim"], (1.0, 0.95, 0.9))

# ---- camera per shot -----------------------------------------------------------------------------------------
target = bpy.data.objects.new("target", None)
sc.collection.objects.link(target)
rig = bpy.data.objects.new("rig", None)
sc.collection.objects.link(rig)
bpy.ops.object.camera_add()
cam = bpy.context.object
cam.parent, sc.camera = rig, cam
cam.constraints.new("TRACK_TO").target = target
cam.data.dof.use_dof, cam.data.dof.focus_object = True, target


def polar(az_deg, r, z):
    return Vector((r * math.cos(math.radians(az_deg)), r * math.sin(math.radians(az_deg)), z))


SHOTS = {  # target, camera, lens, f-stop
    "hero": ((0, 0, GLASS_TOP), polar(-100, 0.09, 0.30), 85, 8),  # high, nearly top-down, the screen readable
    "macro": (polar(-28, R_OUT, -5 * MM), polar(-28, 0.20, 0.012), 100, 16),  # the slider, a USB-C port, layer lines
    "orbit": ((0, 0, GLASS_TOP), Vector((0.06, -0.17, 0.24)), 85, 11),  # three-quarter, slow orbit and push-in
    "exploded": ((0, 0, 12 * MM), polar(-60, 0.30, 0.09), 70, 11),  # side-on three-quarter, the stack apart
}
target.location, cam.location, cam.data.lens, cam.data.dof.aperture_fstop = SHOTS[args.shot]
if args.shot == "orbit":
    last = len(files)
    for frame, turn, scale in ((1, -12, 1.08), (last, 10, 0.92)):
        rig.rotation_euler.z, rig.scale = math.radians(turn), (scale,) * 3
        rig.keyframe_insert("rotation_euler", frame=frame)
        rig.keyframe_insert("scale", frame=frame)

# ---- render --------------------------------------------------------------------------------------------------
sc.render.engine = "CYCLES"
prefs = bpy.context.preferences.addons["cycles"].preferences
prefs.compute_device_type = "METAL"
prefs.get_devices()
for d in prefs.devices:
    d.use = True
sc.cycles.device = "GPU"
sc.cycles.samples = args.samples
sc.cycles.use_denoising = True
sc.render.resolution_x = sc.render.resolution_y = args.res
sc.render.fps = 25
sc.view_settings.view_transform = "Khronos PBR Neutral"  # true base colours, soft highlight roll-off
sc.frame_start, sc.frame_end = 1, len(files)
sc.render.filepath = args.out
if args.anim:
    sc.frame_start = args.start
    sc.render.use_persistent_data = True
    sc.render.image_settings.file_format = "PNG"
    bpy.ops.render.render(animation=True)
else:
    sc.frame_set(args.frame)
    bpy.ops.render.render(write_still=True)
