# PlatformIO post-script shared by every app.
#
#   pio run -e firmware -t factory
#
# writes .pio/build/<env>/factory.bin: bootloader, partition table, boot_app0 and
# the app merged into one image that flashes at offset 0x0. Releases attach it so
# people can flash a board from a browser (https://espressif.github.io/esptool-js/)
# without installing PlatformIO. It mirrors the images and flags `-t upload` uses.
# ruff: noqa: F821
# mypy: disable-error-code="name-defined"
from os.path import join

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
    return env.Execute(env.VerboseAction(" ".join(cmd), f"Merging {out}"))


env.AddCustomTarget(
    name="factory",
    dependencies="$BUILD_DIR/${PROGNAME}.bin",
    actions=build_factory,
    title="Factory image",
    description="Merge bootloader, partitions and app into factory.bin (flash at 0x0)",
)
