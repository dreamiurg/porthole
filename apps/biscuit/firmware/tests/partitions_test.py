#!/usr/bin/env python3
"""Protect saves from PlatformIO's mandatory boot_app0 write at 0xe000."""

import csv
from pathlib import Path

firmware = Path(__file__).resolve().parents[1]
rows = list(csv.reader(line for line in (firmware / "partitions.csv").read_text().splitlines() if line and not line.startswith("#")))
partitions = {row[0].strip(): (int(row[3].strip(), 0), int(row[4].strip(), 0)) for row in rows}
ranges = sorted((start, start + size, name) for name, (start, size) in partitions.items())
for index, (start, end, name) in enumerate(ranges):
    assert start >= 0x9000 and end <= 0x1000000, (name, "outside partition/flash bounds")
    assert start % 0x1000 == 0 and end % 0x1000 == 0, (name, "not sector aligned")
    if index:
        assert start >= ranges[index - 1][1], (name, "overlaps another partition")
assert partitions["otadata"] == (0xE000, 0x2000), "Arduino uploader writes boot_app0 here even with a factory app"
start, size = partitions["nvs"]
assert start + size <= 0xE000, "Uploading firmware must not overwrite saved progress"
start, size = partitions["factory"]
assert start == 0x10000 and start % 0x10000 == 0
image = firmware / ".pio/build/firmware/firmware.bin"
if image.exists():
    assert image.stat().st_size <= size, "Firmware image exceeds the app partition"
print("Partition boundaries pass: save storage is outside every upload range.")
