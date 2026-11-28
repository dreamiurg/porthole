#!/usr/bin/env python3
"""Guard every USB flash of a Porthole board: back up the saves, refuse a build older than the board, then upload.

Run by `make flash` as: safe_flash.py [--port P] -- <upload command...>; the resolved port is appended as
`--upload-port P`. Before the upload it
  1. copies the board's nvs partition (every profile and save) to local/nvs/<mac>/nvs-<time>.bin in the main checkout
     (gitignored), and stops
     if it cannot: no backup, no flash;
  2. reads "sys"/"ran" (the release last booted, firmware/main.cpp) and "sys"/"tried" (the last one downloaded over
     the air) from that copy, and stops unless the newer one's tag is in HEAD. A build from a branch cut before it is older firmware than the board's: in 2026-11 one
     ran over 0.43.9 and hid three of four profiles until a newer build went back on. ALLOW_OLDER=1 skips the check.
Restore a copy with: pio pkg exec -p tool-esptoolpy -- esptool.py -p PORT write_flash 0x9000 <copy>
The copies hold the kids' names and progress: local/ is gitignored, so they stay on this computer and out of git.
"""

import json
import os
import struct
import subprocess
import sys
import time
import zlib
from pathlib import Path

NVS_OFFSET, NVS_SIZE = 0x9000, 0x5000  # apps/porthole/partitions.csv
PAGE, ENTRIES = 4096, 126
BOARD_VIDS = ("VID:PID=1A86:", "VID:PID=303A:")  # the CH343 bridge, the ESP32-S3's own USB
# The main checkout's local/, never a worktree's: removing a worktree would delete the copies with it.
_common = subprocess.run(
    ["git", "rev-parse", "--path-format=absolute", "--git-common-dir"], capture_output=True, text=True, cwd=Path(__file__).parent, check=True
).stdout.strip()
BACKUPS = Path(_common).parent / "local" / "nvs"


def esptool(port: str, *args: str) -> str:
    cmd = ["pio", "pkg", "exec", "-p", "tool-esptoolpy", "--", "esptool.py", "--chip", "esp32s3", "-p", port]
    r = subprocess.run([*cmd, *args], capture_output=True, text=True)
    if r.returncode:
        sys.exit(f"safe_flash: esptool failed, nothing flashed:\n{r.stdout}{r.stderr}")
    return r.stdout


def find_port() -> str:
    out = subprocess.run(["pio", "device", "list", "--json-output"], capture_output=True, text=True, check=True)
    ports = [d["port"] for d in json.loads(out.stdout) if any(v in d["hwid"] for v in BOARD_VIDS)]
    if len(ports) != 1:
        sys.exit(f"safe_flash: found {len(ports)} boards {ports}; pass PORT=...")
    return ports[0]


def nvs_string(nvs: bytes, namespace: str, key: str) -> str | None:
    """A live string or blob value from an ESP-IDF NVS (v2) partition image, or None."""
    entries = []  # (page sequence, slot, ns index, type, key, raw entry, following bytes)
    for pg in range(len(nvs) // PAGE):
        page = nvs[pg * PAGE : (pg + 1) * PAGE]
        seq = struct.unpack_from("<I", page, 4)[0]
        i = 0
        while i < ENTRIES:
            state = (page[32 + i // 4] >> (i % 4 * 2)) & 3  # 3 empty, 2 written, 0 erased
            e = page[64 + i * 32 : 96 + i * 32]
            sound = zlib.crc32(e[0:4] + e[8:32], 0xFFFFFFFF) == struct.unpack_from("<I", e, 4)[0]
            span = e[2] if sound and 0 < e[2] <= ENTRIES - i else 1
            if state == 2 and sound:
                entries.append((seq, i, e[0], e[1], e[8:24].split(b"\0")[0], e, page[96 + i * 32 :]))
            i += span if state != 3 else 1
    ns = {e[4]: e[5][24] for e in entries if e[2] == 0 and e[3] == 0x01}.get(namespace.encode())
    best = None
    for seq, i, nsi, typ, k, e, rest in entries:
        if nsi != ns or k != key.encode() or typ not in (0x21, 0x42):  # a string, or a blob's data chunk
            continue
        size = struct.unpack_from("<H", e, 24)[0]
        if best is None or (seq, i) > best[0]:
            best = ((seq, i), rest[:size].decode(errors="replace"))
    return best[1] if best else None


def version(text: str | None) -> tuple[int, ...] | None:
    """(major, minor, patch) of a release like "0.43.9"; None for "dev", a missing key or anything else."""
    parts = (text or "").split(".")
    return tuple(map(int, parts)) if len(parts) == 3 and all(p.isdigit() for p in parts) else None


def has_release(release: str) -> bool:
    tag = f"porthole-v{release}"
    have = ["git", "rev-parse", "-q", "--verify", f"refs/tags/{tag}"]
    if subprocess.run(have, capture_output=True).returncode:
        subprocess.run(["git", "fetch", "-q", "origin", "tag", tag], capture_output=True)
        if subprocess.run(have, capture_output=True).returncode:
            sys.exit(f"safe_flash: cannot check this checkout against {tag} (no such tag here, and git fetch failed). Nothing flashed.")
    return not subprocess.run(["git", "merge-base", "--is-ancestor", tag, "HEAD"], capture_output=True).returncode


def main() -> None:
    args = sys.argv[1:]
    sep = args.index("--")
    port = args[1] if args[0] == "--port" and sep == 2 and args[1] else find_port()  # make passes "" for none
    upload = args[sep + 1 :]
    BACKUPS.mkdir(parents=True, exist_ok=True)
    read = BACKUPS / "reading.bin"
    # watchdog_reset: the board boots its firmware again, even if the checks below stop here (an RTS reset can leave
    # it in download mode, a black screen).
    out = esptool(port, "--after", "watchdog_reset", "read_flash", hex(NVS_OFFSET), hex(NVS_SIZE), str(read))
    if read.stat().st_size != NVS_SIZE:
        sys.exit("safe_flash: short nvs read, nothing flashed")
    mac = next((ln.split()[-1] for ln in out.splitlines() if ln.startswith("MAC:")), "board")
    folder = BACKUPS / mac.replace(":", "")
    folder.mkdir(exist_ok=True)
    copy = read.rename(folder / time.strftime("nvs-%Y%m%d-%H%M%S.bin"))
    nvs = copy.read_bytes()
    newest = max((v for k in ("ran", "tried") if (v := version(nvs_string(nvs, "sys", k)))), default=None)
    board = ".".join(map(str, newest)) if newest else "no release recorded"
    print(f"safe_flash: saves backed up to {copy}; the board has run or downloaded: {board}", flush=True)
    if newest and not has_release(board) and os.environ.get("ALLOW_OLDER") != "1":
        sys.exit(
            f"safe_flash: the board has run or downloaded {board}, and this checkout does not contain porthole-v{board}.\n"
            "Flashing it would put older firmware over the kids' newer saves. Rebase on origin/main, or flash the\n"
            "release itself; ALLOW_OLDER=1 overrides. Nothing flashed."
        )
    os.execvp(upload[0], [*upload, "--upload-port", port])


if __name__ == "__main__":
    main()
