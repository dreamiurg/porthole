"""Replay the fixture and visible-input trace against snap --serve in a fresh directory; compare the saved frames."""

import json
import subprocess
import sys
import tempfile
from pathlib import Path

from PIL import Image

HERE = Path(__file__).resolve().parent
SNAP = Path(sys.argv[1]) if len(sys.argv) > 1 else HERE.parents[1] / "build" / "host" / "snap"


def read_exact(stream, count):
    data = bytearray()
    while len(data) < count:
        chunk = stream.read(count - len(data))
        if not chunk:
            raise EOFError("Emulator ended before completing its frame")
        data.extend(chunk)
    return bytes(data)


def replay(work):
    (work / "build" / "host").mkdir(parents=True)
    subprocess.run([str(SNAP), "--fresh", "--script", str(HERE / "fixture.txt")], cwd=work, check=True, capture_output=True)
    proc = subprocess.Popen([str(SNAP), "--serve"], cwd=work, stdin=subprocess.PIPE, stdout=subprocess.PIPE)
    frames = {}
    try:
        for line in (HERE / "trace.jsonl").read_text().splitlines():
            step = json.loads(line)
            for command in step["commands"]:
                proc.stdin.write((command + "\n").encode())
                proc.stdin.flush()
                if proc.stdout.readline().strip() != b"ok":
                    raise RuntimeError(f"Emulator rejected {command!r}")
            proc.stdin.write(b"frame\n")
            proc.stdin.flush()
            count = int(proc.stdout.readline().split()[1])
            frames[step["frame"]] = Image.frombytes("RGB", (480, 480), read_exact(proc.stdout, count))
    finally:
        proc.terminate()
        proc.wait()
    recorded = sorted(HERE.glob("*.png"))
    for path in recorded:
        with Image.open(path) as expected:
            if frames[path.stem].tobytes() != expected.convert("RGB").tobytes():
                raise AssertionError(f"Rendered frame changed: {path.stem}")
    print(f"PASS: {len(recorded)} recorded native frames reproduced in a fresh process")


if __name__ == "__main__":
    with tempfile.TemporaryDirectory(prefix="admin-profiles-playtest-") as directory:
        replay(Path(directory))
