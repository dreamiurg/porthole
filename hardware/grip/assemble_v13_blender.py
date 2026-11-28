"""Open the editable V13 kid wheel: blender -P hardware/grip/assemble_v13_blender.py."""

from math import radians
from pathlib import Path

import bpy
from mathutils import Euler, Matrix

KIT = Path(__file__).resolve().parent / "stl/v13-kid"
PIN_CENTRES = [(-68, -50), (-62, 46), (62, -50), (62, 46)]


def material(name, rgb):
    mat = bpy.data.materials.new(name)
    srgb = [c / 255 for c in rgb]
    mat.diffuse_color = [c / 12.92 if c < 0.04045 else ((c + 0.055) / 1.055) ** 2.4 for c in srgb] + [1]
    return mat


def assemble():
    bpy.ops.wm.read_factory_settings(use_empty=True)
    scene = bpy.context.scene
    scene.name = "V13 kid wheel"
    scene.unit_settings.system = "METRIC"
    scene.unit_settings.scale_length = 0.001
    scene.unit_settings.length_unit = "MILLIMETERS"
    main = material("Deep Midnight PLA", (9, 24, 44))
    accent = material("Hot Orange PLA", (255, 121, 0))
    for path in sorted((KIT / "components").glob("*.stl")):
        bpy.ops.wm.stl_import(filepath=str(path))
        obj = bpy.context.object
        obj.name = path.stem
        obj.data.materials.append(main if path.stem.endswith("body") else accent)
        if path.stem.startswith("P016"):
            obj.location.z = -14.6  # Rear bed plane to wheel split plane.
        for face in obj.data.polygons:
            face.use_smooth = True
        obj.data.set_sharp_from_angle(angle=radians(38))

    bpy.ops.wm.stl_import(filepath=str(KIT / "pins-4.stl"))
    bpy.context.object.data.materials.append(main)
    bpy.ops.object.mode_set(mode="EDIT")
    bpy.ops.mesh.select_all(action="SELECT")
    bpy.ops.mesh.separate(type="LOOSE")
    bpy.ops.object.mode_set(mode="OBJECT")
    pins = sorted(bpy.context.selected_objects, key=lambda ob: ob.bound_box[0][1])
    for i, (pin, (x, y)) in enumerate(zip(pins, PIN_CENTRES, strict=True)):
        pin.name = f"Pin {i + 1}"
        centre = Matrix.Translation((0, -8 * i, -1.9))
        pin.matrix_world = Matrix.Translation((x, y, 0)) @ Matrix.Rotation(radians(90), 4, "Y") @ centre

    bpy.ops.object.select_all(action="DESELECT")
    bpy.context.view_layer.objects.active = None
    for area in bpy.context.screen.areas:
        if area.type == "VIEW_3D":
            view = area.spaces.active
            view.region_3d.view_perspective = "ORTHO"
            view.region_3d.view_rotation = Euler((radians(28), 0, radians(-10))).to_quaternion()
            view.region_3d.view_distance = 234
            view.shading.color_type = "MATERIAL"
            view.overlay.show_floor = False


if __name__ == "__main__":
    assemble()
