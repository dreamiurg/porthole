#!/usr/bin/env python3
"""Replay an observed touch trace through the emulator's unchanged 33ms serve protocol.

Unlike script mode, this keeps the exact timing of the recorded live session.
The only setup fixture is the documented parental allowance; no campaign is staged.
"""

import argparse
import hashlib
import json
from pathlib import Path
import struct
import subprocess

from PIL import Image


class Replay:
    def __init__(self, binary, output):
        self.binary, self.output = binary, output
        self.proc = None
        self.matches, self.differences = [], []

    def start(self):
        self.proc = subprocess.Popen([str(self.binary), "--serve"], cwd=self.output, stdin=subprocess.PIPE, stdout=subprocess.PIPE)

    def stop(self, forced=False):
        if self.proc:
            if forced:
                self.proc.kill()
            self.proc.stdin.close()
            status = self.proc.wait()
            self.proc.stdout.close()
            self.proc = None
            if status and not forced:
                raise RuntimeError("emulator exited unsuccessfully")

    def command(self, text):
        self.proc.stdin.write((text + "\n").encode())
        self.proc.stdin.flush()
        return self.proc.stdout.readline().decode().strip()

    def snapshot(self, name, expected):
        header = self.command("frame")
        if not header.startswith("PPM "):
            raise RuntimeError(header)
        count = int(header.split()[1])
        payload = self.proc.stdout.read(count)
        image = Image.frombytes("RGB", (480, 480), payload)
        name = Path(name).name + ".png"
        image.save(self.output / name)
        if (expected / name).exists():
            with Image.open(expected / name) as original:
                same = original.convert("RGB").tobytes() == payload
            (self.matches if same else self.differences).append(name)

    def line(self, raw, expected):
        words = raw.strip().split(maxsplit=1)
        if not words or words[0].startswith("#"):
            return
        operation, argument = words[0], words[1] if len(words) > 1 else ""
        if operation == "snap":
            self.snapshot(argument, expected)
        elif operation == "reboot":
            self.stop()
            self.start()
        elif operation == "crash":
            self.stop(forced=True)
            self.start()
        elif operation in ("down", "move", "up", "wait", "tick"):
            command = "tick " + argument if operation == "wait" else raw.strip()
            if self.command(command) != "ok":
                raise RuntimeError("unexpected emulator response")
        else:
            raise ValueError(f"non-player command in observed trace: {operation}")


def main():
    parser = argparse.ArgumentParser(description=__doc__)
    parser.add_argument("trace", type=Path)
    parser.add_argument("output", type=Path)
    parser.add_argument("--binary", type=Path, default=Path("build/host/snap"))
    parser.add_argument("--expected", type=Path)
    args = parser.parse_args()
    output = args.output.resolve()
    if output.exists() and any(output.iterdir()):
        raise SystemExit("Use a fresh output directory to avoid reusing campaign state.")
    (output / "build/host").mkdir(parents=True, exist_ok=True)
    (output / "build/host/sys-limits.sav").write_bytes(struct.pack("<HH", 20, 120))
    run = Replay(args.binary.resolve(), output)
    run.start()
    try:
        for line in args.trace.read_text().splitlines():
            run.line(line, args.expected or args.trace.parent)
    finally:
        run.stop()
    report = {
        "binary_sha256": hashlib.sha256(args.binary.read_bytes()).hexdigest(),
        "matching_snapshots": len(run.matches),
        "changed_snapshots": run.differences,
        "setup": "Fresh campaign; shell session20/daily120 minutes only.",
    }
    (output / "replay.json").write_text(json.dumps(report, indent=2) + "\n")
    print(json.dumps(report))


if __name__ == "__main__":
    main()
