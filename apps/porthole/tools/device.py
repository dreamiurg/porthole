#!/usr/bin/env python3
"""Drive the real board over serial: tap it, read what it shows, run playtest-style scripts.

    python3 tools/device.py shot build/device/now.png
    python3 tools/device.py tap 80 120            # logical px, same as the sim scripts
    python3 tools/device.py hold 80 120
    python3 tools/device.py stats                 # the firmware's "S" output
    python3 tools/device.py run tests/device/smoke.txt

Script commands (a subset of the sim's): tap X Y | hold X Y | wait MS | snap NAME | screen | echo TEXT
| raw CMD (send a firmware serial command as-is, e.g. "raw T1790300000"). Snaps go to build/device/NAME.png,
upscaled 3x with the round mask, like the sim's snapshots. Firmware side: "X<x>,<y>,<ms>" and "F" in
firmware/main.cpp. The port defaults to the first /dev/cu.usbmodem*; override with PORT=...
Opening the port does not reset the board (DTR/RTS are held low before open).
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
    p = serial.Serial()
    p.port, p.baudrate, p.timeout = port, 115200, 3
    p.dtr = p.rts = False  # a DTR/RTS pulse on open would reset the ESP32 through the CH343 auto-reset circuit
    p.open()
    time.sleep(0.05)
    p.reset_input_buffer()
    return p


def press(p: serial.Serial, x: int, y: int, ms: int) -> None:
    p.write(f"X{x},{y},{ms}\n".encode())
    time.sleep(ms / 1000 + 0.12)  # the press, then a few frames for the release to register


def frame(p: serial.Serial) -> tuple[int, int, bytes, list[tuple[int, int, int]]]:
    p.reset_input_buffer()
    p.write(b"F\n")
    while True:  # skip any log lines printed before the header
        line = p.readline()
        if not line:
            sys.exit("no frame from the board (is the Porthole firmware with the F command flashed?)")
        if line.startswith(b"FB "):
            break
    w, h = (int(v) for v in line.split()[1:3])
    px = p.read(w * h)
    pal565 = p.read(64)
    if len(px) != w * h or len(pal565) != 64:
        sys.exit(f"short frame: {len(px)} px, {len(pal565)} palette bytes")
    pal = []
    for (c,) in struct.iter_unpack("<H", pal565):
        r, g, b = (c >> 11) & 31, (c >> 5) & 63, c & 31
        pal.append((r * 255 // 31, g * 255 // 63, b * 255 // 31))
    return w, h, px, pal


def write_png(path: Path, w: int, h: int, px: bytes, pal: list[tuple[int, int, int]], scale: int = 3) -> None:
    W, H, r = w * scale, h * scale, w * scale / 2
    rows = bytearray()
    for y in range(H):
        rows.append(0)
        for x in range(W):
            inside = (x - r + 0.5) ** 2 + (y - r + 0.5) ** 2 <= r * r
            idx = px[(y // scale) * w + x // scale]
            rows += bytes(pal[idx] if inside and idx < len(pal) else (32, 32, 32))

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
