# PlatformIO post-script: fail the build when static internal RAM (.dram0.data + .dram0.bss) passes the env's
# custom_dram_static_max. Every byte there is gone from the heap for good, and the heap is what a wireless update
# needs (its screen, wifi, TLS). Porthole 0.26.1 and 0.27.0 shipped with too little left and could never update
# again; this catches that growth in the pull request instead of on a board.
# ruff: noqa: F821
# mypy: disable-error-code="name-defined"
import os
import subprocess

Import("env")


def check(source, target, env):
    limit = int(env.GetProjectOption("custom_dram_static_max", "0"), 0)
    if not limit:
        return None
    size = env.subst("$SIZETOOL")
    out = subprocess.run([size, "-A", str(target[0])], capture_output=True, text=True, check=True).stdout
    used = sum(int(f[1]) for f in (line.split() for line in out.splitlines()) if f and f[0] in (".dram0.data", ".dram0.bss"))
    print(f"Static internal RAM: {used} of {limit} bytes")
    if used > limit:
        print("Error: static internal RAM over custom_dram_static_max. Put big buffers on the heap (PSRAM on the board).")
        os.remove(str(target[0]))  # or the next incremental build skips this check and passes
        return 1
    return None


env.AddPostAction("$BUILD_DIR/${PROGNAME}.elf", check)
