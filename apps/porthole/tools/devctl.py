#!/usr/bin/env python3
"""Drive the real board over serial: tap it, read what it shows, run playtest-style scripts.

    python3 tools/devctl.py shot build/device/now.png
    python3 tools/devctl.py tap 80 120            # logical px, same as the sim scripts
    python3 tools/devctl.py hold 80 120
    python3 tools/devctl.py stats                 # the firmware's "S" output
    python3 tools/devctl.py run tests/device/smoke.txt

Script commands (a subset of the sim's): tap X Y | hold X Y | wait MS | snap NAME | screen | echo TEXT
| raw CMD (send a firmware serial command as-is, e.g. "raw T1790300000"). Snaps go to build/device/NAME.png at
480x480 with the round mask, like the sim's snapshots: an indexed frame upscaled 3x, an RGB565 frame as is.
Firmware side: "X<x>,<y>,<ms>" and "F" in firmware/main.cpp. The port defaults to the first /dev/cu.usbmodem*; override with PORT=...
Opening the port does not reset the board (DTR/RTS are left alone).
"""

import glob
import os
import struct
import sys
import time
import zlib
from pathlib import Path

import serial  # pyserial: ships with PlatformIO's Python; `pip install pyserial` otherwise

OUT = Path("build/device")


def open_port() -> serial.Serial:
    port = os.environ.get("PORT") or next(iter(sorted(glob.glob("/dev/cu.usbmodem*") + glob.glob("/dev/ttyACM*"))), None)
    if not port:
        sys.exit("no board found; set PORT=...")
    # Leave DTR/RTS at the OS default (both asserted = neutral for the auto-reset transistors). Setting them
    # one at a time passes through DTR=0/RTS=1, which pulls EN low and reboots the board, or strands it in
    # download mode (blank, unresponsive screen) if IO0 is caught low.
    p = serial.Serial(port, 115200, timeout=3)
    time.sleep(0.05)
    p.reset_input_buffer()
    return p


def press(p: serial.Serial, x: int, y: int, ms: int) -> None:
    p.write(f"X{x},{y},{ms}\n".encode())
    time.sleep(ms / 1000 + 0.12)  # the press, then a few frames for the release to register


def rgb565(c: int) -> tuple[int, int, int]:
    r, g, b = (c >> 11) & 31, (c >> 5) & 63, c & 31
    return (r << 3 | r >> 2, g << 2 | g >> 4, b << 3 | b >> 2)  # bit replication, as the sim's snapshots


def frame(p: serial.Serial) -> tuple[int, int, list[tuple[int, int, int]]]:
    """The frame on the glass as (w, h, w*h RGB pixels). Header "FB w h [idx|565 runs]"; no token = idx (older firmware)."""
    p.reset_input_buffer()
    p.write(b"F\n")
    while True:  # skip any log lines printed before the header
        line = p.readline()
        if not line:
            sys.exit("no frame from the board (is the Porthole firmware with the F command flashed?)")
        if line.startswith(b"FB "):
            break
    head = line.split()
    w, h, kind = int(head[1]), int(head[2]), head[3].decode() if len(head) > 3 else "idx"
    if kind == "565":  # (count, color) runs; 115200 baud moves about 11 KB/s, so the timeout follows the size
        runs, timeout = int(head[4]), p.timeout
        p.timeout = 5 + runs * 4 / 10_000
        data = p.read(runs * 4)
        p.timeout = timeout
        if len(data) != runs * 4:
            sys.exit(f"short frame: {len(data)} of {runs * 4} run bytes")
        out = [px for n, c in struct.iter_unpack("<HH", data) for px in [rgb565(c)] * n]
        if len(out) != w * h:
            sys.exit(f"short frame: {len(out)} of {w * h} px")
        return w, h, out
    px = p.read(w * h)
    pal565 = p.read(64)
    if len(px) != w * h or len(pal565) != 64:
        sys.exit(f"short frame: {len(px)} px, {len(pal565)} palette bytes")
    pal = [rgb565(c) for (c,) in struct.iter_unpack("<H", pal565)]
    return w, h, [pal[i] if i < len(pal) else (32, 32, 32) for i in px]


def write_png(path: Path, w: int, h: int, px: list[tuple[int, int, int]]) -> None:
    scale = 480 // w  # the panel is 480 px: an indexed frame is upscaled 3x
    W, H, r = w * scale, h * scale, w * scale / 2
    rows = bytearray()
    for y in range(H):
        rows.append(0)
        for x in range(W):
            inside = (x - r + 0.5) ** 2 + (y - r + 0.5) ** 2 <= r * r
            rows += bytes(px[(y // scale) * w + x // scale] if inside else (32, 32, 32))

    def chunk(kind: bytes, data: bytes) -> bytes:
        return struct.pack(">I", len(data)) + kind + data + struct.pack(">I", zlib.crc32(kind + data))

    path.parent.mkdir(parents=True, exist_ok=True)
    path.write_bytes(b"\x89PNG\r\n\x1a\n" + chunk(b"IHDR", struct.pack(">IIBBBBB", W, H, 8, 2, 0, 0, 0)) + chunk(b"IDAT", zlib.compress(bytes(rows), 6)) + chunk(b"IEND", b""))


def shot(p: serial.Serial, path: Path) -> None:
    write_png(path, *frame(p))
    print(f"snap {path}")


def stats(p: serial.Serial) -> str:
    p.reset_input_buffer()
    p.write(b"S\n")
    time.sleep(0.4)
    return p.read(p.in_waiting).decode(errors="replace")


def run(p: serial.Serial, script: Path) -> None:
    for n, raw in enumerate(script.read_text().splitlines(), 1):
        words = raw.split("#", 1)[0].split()
        if not words:
            continue
        cmd, args = words[0], words[1:]
        if cmd in ("tap", "hold"):
            press(p, int(args[0]), int(args[1]), 80 if cmd == "tap" else 1000)
        elif cmd == "wait":
            time.sleep(int(args[0]) / 1000)
        elif cmd == "snap":
            shot(p, OUT / f"{args[0]}.png")
        elif cmd == "screen":
            line = next((ln for ln in stats(p).splitlines() if ln.startswith("screen=")), "screen=?")
            print(line)
        elif cmd == "echo":
            print(" ".join(args))
        elif cmd == "raw":
            p.write((" ".join(args) + "\n").encode())
            time.sleep(0.3)
        else:
            print(f"{script}:{n}: '{cmd}' has no device equivalent, skipped")


def main() -> None:
    if len(sys.argv) < 2:
        sys.exit(__doc__)
    cmd, args = sys.argv[1], sys.argv[2:]
    p = open_port()
    if cmd == "shot":
        shot(p, Path(args[0]) if args else OUT / "now.png")
    elif cmd in ("tap", "hold"):
        press(p, int(args[0]), int(args[1]), 80 if cmd == "tap" else 1000)
    elif cmd == "stats":
        print(stats(p))
    elif cmd == "run":
        run(p, Path(args[0]))
    else:
        sys.exit(__doc__)


if __name__ == "__main__":
    main()
