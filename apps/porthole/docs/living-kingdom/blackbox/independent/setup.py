"""Prepare only documented shell time limits for a fresh black-box run.

No profile or story progress is supplied. Create the profile and enter Kingdom
through the visible interface after running this script.
"""

from pathlib import Path
import struct
import sys

run_directory = Path(sys.argv[1]).resolve()
save_directory = run_directory / "build" / "host"
save_directory.mkdir(parents=True, exist_ok=True)
(save_directory / "sys-limits.sav").write_bytes(struct.pack("<HH", 20, 120))
