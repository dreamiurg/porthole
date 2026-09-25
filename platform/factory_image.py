# PlatformIO post-script shared by every app.
#
#   pio run -e firmware -t factory
#
# writes two things into .pio/build/<env>/, using the same images and offsets as `-t upload`:
#   factory.bin  bootloader, partition table, boot_app0 and app merged into one image
#                that flashes at 0x0 (fresh installs; it overwrites the save partition).
#   web/         the same images as separate files plus parts.json ([{path, offset}]),
#                for the web installer: writing parts one by one leaves saves alone.
# ruff: noqa: F821
# mypy: disable-error-code="name-defined"
import json
import os
import shutil
from os.path import basename, join

Import("env")
platform = env.PioPlatform()


def build_factory(source, target, env):
    esptool = join(platform.get_package_dir("tool-esptoolpy") or "", "esptool.py")
    out = env.subst("$BUILD_DIR/factory.bin")
    images = [(offset, env.subst(path)) for offset, path in env.get("FLASH_EXTRA_IMAGES", [])]
    images.append((env.subst("$ESP32_APP_OFFSET"), env.subst("$BUILD_DIR/${PROGNAME}.bin")))
    cmd = [
        '"$PYTHONEXE"',
        f'"{esptool}"',
        "--chip",
        env.BoardConfig().get("build.mcu"),
        "merge-bin",
        "-o",
        f'"{out}"',
        "--flash-mode",
        "${__get_board_flash_mode(__env__)}",
        "--flash-freq",
        "${__get_board_f_image(__env__)}",
        "--flash-size",
        env.BoardConfig().get("upload.flash_size", "16MB"),
    ]
    for offset, path in images:
        cmd += [offset, f'"{path}"']
    web = env.subst("$BUILD_DIR/web")
    shutil.rmtree(web, ignore_errors=True)
    os.makedirs(web)
    parts = []
    for offset, path in images:
        shutil.copy(path, join(web, basename(path)))
        parts.append({"path": basename(path), "offset": int(offset, 0)})
    with open(join(web, "parts.json"), "w") as f:
        json.dump(parts, f, indent=2)
    return env.Execute(env.VerboseAction(" ".join(cmd), f"Merging {out}"))


env.AddCustomTarget(
    name="factory",
    dependencies="$BUILD_DIR/${PROGNAME}.bin",
    actions=build_factory,
    title="Factory image",
    description="factory.bin (flash at 0x0) plus web/ parts for the web installer",
)
