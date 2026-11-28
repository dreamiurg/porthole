"""Build the three-piece R7 inspection GLB. Run blender -b -P hardware/case/assemble_r7_blender.py."""

from math import pi
from pathlib import Path

import bpy
from mathutils import Matrix

KIT = Path(__file__).resolve().parent / "stl/twist-lock/r7-simple-entry"


def material(name, rgb):
    result = bpy.data.materials.new(name)
    result.diffuse_color = (*rgb, 1)
    result.use_nodes = True
    shader = result.node_tree.nodes["Principled BSDF"]
    shader.inputs["Base Color"].default_value = result.diffuse_color
    shader.inputs["Roughness"].default_value = 0.67
    return result


def part(name, filename, color, z=0, flipped=False):
    bpy.ops.wm.stl_import(filepath=str(KIT / filename))
    obj = bpy.context.selected_objects[0]
    obj.name = name
    obj.data.materials.clear()
    obj.data.materials.append(color)
    obj.data.transform(Matrix.Scale(0.001, 4) @ Matrix.Translation((0, 0, z)) @ Matrix.Rotation(pi if flipped else 0, 4, "X"))
    return obj


def assemble():
    bpy.ops.wm.read_factory_settings(use_empty=True)
    bezel = part("P014 - Front bezel", "components/P014-bezel-body.stl", material("Deep Midnight", (0.026, 0.060, 0.115)), 4.6, True)
    part("Pinstripe", "components/P014-bezel-pinstripe.stl", material("Hot Orange", (1, 0.191, 0)), 4.6, True)
    bezel.select_set(True)
    bpy.context.view_layer.objects.active = bezel
    bpy.ops.object.join()
    part("P021 - Middle plate", "P021-plate.stl", material("Teal", (0.015, 0.35, 0.31)))
    part("P022 - Lower lid", "P022-lid.stl", material("Copper", (0.60, 0.155, 0.038)), -21)
    bpy.ops.object.select_all(action="SELECT")
    bpy.ops.object.origin_set(type="ORIGIN_GEOMETRY", center="BOUNDS")
    dest = KIT / "preview/Puck-case-assembly.glb"
    dest.parent.mkdir(exist_ok=True)
    bpy.ops.export_scene.gltf(filepath=str(dest), export_format="GLB", export_yup=True, export_animations=False, export_cameras=False)


if __name__ == "__main__":
    assemble()
