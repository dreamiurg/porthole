#!/usr/bin/env python3
"""Self-check for tools/safe_flash.py's NVS reader on a hand-built partition. Run: python3 tools/test_safe_flash.py"""

import importlib.util
import struct
import zlib
from pathlib import Path

spec = importlib.util.spec_from_file_location("safe_flash", Path(__file__).with_name("safe_flash.py"))
assert spec and spec.loader
sf = importlib.util.module_from_spec(spec)
spec.loader.exec_module(sf)

WRITTEN, ERASED = 2, 0


def entry(ns: int, typ: int, span: int, key: str, data: bytes) -> bytes:
    head, tail = bytes([ns, typ, span, 0]), key.encode().ljust(16, b"\0") + data.ljust(8, b"\xff")
    return head + struct.pack("<I", zlib.crc32(head + tail, 0xFFFFFFFF)) + tail


def page(seq: int, slots: list[tuple[int, bytes]]) -> bytearray:
    """slots: (state, 32-byte entry or a blob's data run); each run fills consecutive slots."""
    p = bytearray(b"\xff" * sf.PAGE)
    struct.pack_into("<II", p, 0, 0xFFFFFFFE, seq)
    i = 0
    for state, raw in slots:
        for k in range(len(raw) // 32 or 1):
            p[32 + (i + k) // 4] &= ~(3 << ((i + k) % 4 * 2)) | (state << ((i + k) % 4 * 2))
        p[64 + i * 32 : 64 + i * 32 + len(raw)] = raw
        i += max(len(raw) // 32, 1)
    return p


def blob(ns: int, key: str, value: bytes) -> bytes:
    body = value.ljust(32, b"\xff")
    return entry(ns, 0x42, 2, key, struct.pack("<H", len(value))) + body


sys_ns = entry(0, 0x01, 1, "sys", b"\x03")
# An older erased "tried" before the live one; a same-named key in another namespace must not be read.
nvs = page(5, [(WRITTEN, sys_ns), (ERASED, blob(3, "tried", b"0.41.2"))])
nvs += page(6, [(WRITTEN, blob(3, "tried", b"0.43.9")), (WRITTEN, blob(4, "tried", b"9.9.9"))])
# "ran" as an NVS string (type 0x21) whose 40 bytes run over two data slots, and a "tried" whose CRC is wrong.
ran = entry(3, 0x21, 3, "ran", struct.pack("<H", 40)) + b"1.2.3".ljust(40, b"x").ljust(64, b"\xff")
damaged = bytearray(blob(3, "tried", b"9.9.9"))
damaged[4] ^= 1
nvs += page(7, [(WRITTEN, ran), (WRITTEN, bytes(damaged))])
nvs += b"\xff" * (sf.NVS_SIZE - len(nvs))

assert sf.nvs_string(bytes(nvs), "sys", "tried") == "0.43.9", sf.nvs_string(bytes(nvs), "sys", "tried")
assert sf.nvs_string(bytes(nvs), "sys", "ran") == "1.2.3".ljust(40, "x")
assert sf.nvs_string(bytes(nvs), "porthole", "tried") is None
assert sf.nvs_string(b"\xff" * sf.NVS_SIZE, "sys", "tried") is None  # an erased partition: nothing to guard
assert sf.version("0.44.10") == (0, 44, 10) and sf.version("0.44.10") > sf.version("0.44.9")  # not text order
assert sf.version("dev") is None and sf.version(None) is None and sf.version("1.2") is None
print("safe_flash self-check passed")
