"""Replay recorded visible inputs against snap --serve in a fresh isolated directory."""

import json
import subprocess
import tempfile
from pathlib import Path

from PIL import Image

HERE = Path(__file__).resolve().parent
SNAP = HERE.parents[1] / "build" / "host" / "snap"


def start(work):
    return subprocess.Popen([str(SNAP), "--serve"], cwd=work, stdin=subprocess.PIPE, stdout=subprocess.PIPE)


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
    subprocess.run(
        [str(SNAP), "--fresh", "--script", str(HERE / "fixture.txt")],
        cwd=work,
        check=True,
        capture_output=True,
    )
    proc = start(work)
    frames = {}
    try:
        for line in (HERE / "input-trace.jsonl").read_text().splitlines():
            step = json.loads(line)
            if step.get("power_cycle"):
                proc.terminate()
                proc.wait()
                proc = start(work)
            for command in step["commands"]:
                proc.stdin.write((command + "\n").encode())
                proc.stdin.flush()
                if proc.stdout.readline().strip() != b"ok":
                    raise RuntimeError(f"Emulator rejected {command!r}")
            proc.stdin.write(b"frame\n")
            proc.stdin.flush()
            count = int(proc.stdout.readline().split()[1])
            rgb = read_exact(proc.stdout, count)
            frames[step["frame"]] = Image.frombytes("RGB", (480, 480), rgb)
    finally:
        proc.terminate()
        proc.wait()
    for name, frame in frames.items():
        with Image.open(HERE / (name + ".png")) as expected:
            if frame.tobytes() != expected.convert("RGB").tobytes():
                raise AssertionError(f"Rendered frame changed: {name}")
    print(f"PASS: {len(frames)} recorded native frames reproduced in a fresh process")


if __name__ == "__main__":
    with tempfile.TemporaryDirectory(prefix="admin-playtest-") as directory:
        replay(Path(directory))
