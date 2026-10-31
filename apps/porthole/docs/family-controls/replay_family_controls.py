"""Replay family-control evidence privately, checking a list of native RGB hashes."""

import argparse
import hashlib
import json
import subprocess
import tempfile
from pathlib import Path

from PIL import Image

HERE = Path(__file__).resolve().parent
ROOT = HERE.parents[1]
PHASES = (
    ("visible", "fixture.txt", "visible-input-trace.jsonl"),
    ("clock", "clock-fixture.txt", "clock-input-trace.jsonl"),
)


def start(snap, work):
    return subprocess.Popen([str(snap), "--serve"], cwd=work, stdin=subprocess.PIPE, stdout=subprocess.PIPE)


def read_exact(stream, count):
    data = bytearray()
    while len(data) < count:
        chunk = stream.read(count - len(data))
        if not chunk:
            raise EOFError("Emulator stopped before completing its frame")
        data.extend(chunk)
    return bytes(data)


def command(proc, line):
    proc.stdin.write((line + "\n").encode())
    proc.stdin.flush()
    if proc.stdout.readline().strip() != b"ok":
        raise RuntimeError(f"Emulator rejected {line!r}")


def replay_phase(snap, capture_root, phase, fixture, trace):
    entries = []
    build_hash = hashlib.sha256(snap.read_bytes()).hexdigest()
    with tempfile.TemporaryDirectory(prefix=f"family-{phase}-") as temporary:
        work = Path(temporary)
        (work / "build" / "host").mkdir(parents=True)
        subprocess.run(
            [str(snap), "--fresh", "--script", str(HERE / fixture)],
            cwd=work,
            capture_output=True,
            check=True,
        )
        proc = start(snap, work)
        try:
            for line in (HERE / trace).read_text().splitlines():
                step = json.loads(line)
                if step.get("power_cycle"):
                    proc.terminate()
                    proc.wait()
                    proc = start(snap, work)
                for action in step.get("commands", []):
                    command(proc, action)
                proc.stdin.write(b"frame\n")
                proc.stdin.flush()
                count = int(proc.stdout.readline().split()[1])
                rgb = read_exact(proc.stdout, count)
                name = step["frame"]
                Image.frombytes("RGB", (480, 480), rgb).save(capture_root / (name + ".png"))
                entries.append(
                    {
                        "phase": phase,
                        "frame": name,
                        "width": 480,
                        "height": 480,
                        "rgb_sha256": hashlib.sha256(rgb).hexdigest(),
                        "build_sha256": build_hash,
                    }
                )
        finally:
            proc.terminate()
            proc.wait()
    print(f"Replayed {phase}: {len(entries)} native frames", flush=True)
    return entries


def main():
    parser = argparse.ArgumentParser(description=__doc__)
    frozen = ROOT / "build" / "family-controls" / "final-snap"
    parser.add_argument("--snap", type=Path, default=frozen if frozen.exists() else ROOT / "build/host/snap")
    parser.add_argument("--capture-root", type=Path, default=ROOT / "build/family-controls/replay")
    parser.add_argument("--record", action="store_true", help="Record reviewed final hashes; PNGs stay private")
    args = parser.parse_args()
    args.capture_root.mkdir(parents=True, exist_ok=True)
    manifest_path = HERE / "frame-manifest.json"
    expected = json.loads(manifest_path.read_text())
    if not isinstance(expected, list):
        raise TypeError("Frame manifest must be a list")
    actual = []
    for phase, fixture, trace in PHASES:
        actual.extend(replay_phase(args.snap.resolve(), args.capture_root, phase, fixture, trace))
    names = [entry["frame"] for entry in actual]
    if len(names) != len(set(names)):
        raise ValueError("Frame names must be unique across phases")
    hashes = {entry["frame"]: entry["rgb_sha256"] for entry in expected}
    changed = [entry["frame"] for entry in actual if hashes.get(entry["frame"]) != entry["rgb_sha256"]]
    if args.record:
        manifest_path.write_text(json.dumps(actual, indent=2) + "\n")
        print(f"Recorded {len(actual)} final RGB hashes; changed references: {changed}", flush=True)
    elif changed:
        raise AssertionError(f"Rendered frames changed: {changed}")
    else:
        print(f"PASS: {len(actual)} final native RGB hashes match", flush=True)


if __name__ == "__main__":
    main()
