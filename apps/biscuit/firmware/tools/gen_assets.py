"""PlatformIO pre-build hook: generate firmware/generated/assets.{h,cpp} from the browser game's sources."""

# mypy: disable-error-code="name-defined"
import shutil
import subprocess
import sys
from pathlib import Path

Import("env")  # noqa: F821  (injected by PlatformIO/SCons)

node = shutil.which("node")
if not node:
    sys.exit("Node.js 22+ is required to build the firmware: it generates assets from the browser game (tools/export-assets.mjs).")
subprocess.run([node, str(Path(env.subst("$PROJECT_DIR")) / "tools" / "export-assets.mjs")], check=True)  # noqa: F821
