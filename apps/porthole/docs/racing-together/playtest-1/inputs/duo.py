# mypy: ignore-errors
# ruff: noqa: F403, F405, E402
"""Two-sim driver for black-box Rally Together play. Each scenario script imports this and runs from a fresh start."""

import os
import subprocess
import tempfile
from PIL import Image

SNAP = "/home/claude/pt/apps/porthole/build/host/snap"
OUT = "/home/claude/pt/apps/porthole/docs/racing-together/playtest"


class Sim:
    def __init__(self, name, profiles, age=7):
        self.name = name
        self.dir = tempfile.mkdtemp(prefix=f"pt-{name}-")
        os.makedirs(os.path.join(self.dir, "build", "host"))
        self.p = subprocess.Popen([SNAP, "--serve", "--fresh"], cwd=self.dir, stdin=subprocess.PIPE, stdout=subprocess.PIPE)
        self.screen = "?"
        self.log = []
        for kid in profiles:
            self.do(f"profile {kid} {age}")

    def cmd(self, line):
        self.p.stdin.write((line + "\n").encode())
        self.p.stdin.flush()
        out = []
        reply = self.p.stdout.readline().decode().strip()
        while reply != "ok":
            if reply == "" and self.p.poll() is not None:
                raise RuntimeError(f"{self.name} died")
            if reply.startswith("screen="):
                self.screen = reply[7:]
            out.append(reply)
            reply = self.p.stdout.readline().decode().strip()
        return out

    def do(self, line):
        return self.cmd("do " + line)

    def scr(self):
        self.do("screen")
        return self.screen

    def snap(self, shot):
        self.do("snap s")
        src = os.path.join(self.dir, "build", "host", "s.bmp")
        dst = os.path.join(OUT, f"{shot}_{self.name}.png")
        Image.open(src).save(dst)
        self.scr()
        print(f"  snap {dst} screen={self.screen}")
        return dst

    def tap(self, x, y):
        self.do(f"tap {x} {y}")

    def kill(self):
        self.p.kill()
        self.p.wait()

    def close(self):
        try:
            self.p.stdin.close()
            self.p.wait(timeout=5)
        except Exception:
            self.p.kill()


def both(a, b, ms, step=66):
    for _ in range(0, ms, step):
        if a:
            a.cmd(f"tick {step}")
        if b:
            b.cmd(f"tick {step}")


def pair(*sims, path=None):
    """Side-by-side PNG of the latest shots."""
    pass


def to_rally(s, other=None):
    s.tap(124, 104)
    both(s, other, 700)
    s.tap(96, 135)
    both(s, other, 700)
    s.tap(36, 104)
    both(s, other, 1500)


def start(names=("Mia", "Leo")):  # B gets a second slot so faces differ
    a = Sim("A", [names[0]])
    b = Sim("B", ["Kai", names[1]])
    both(a, b, 1000)
    to_rally(a, b)
    to_rally(b, a)
    return a, b


def to_lobby(a, b, join=(80, 96)):
    a.tap(80, 21)
    both(a, b, 1500)
    b.tap(*join)
    both(a, b, 1200)


import math
import random


def tiltdeg(s, deg, lean=0.0):
    """Wheel turned by deg (right positive), leaned back by lean (0..1 of flat)."""
    m = 1 - lean
    a = math.radians(deg)
    s.do(f"tilt {int(math.sin(a) * m * 1000)} {int(math.cos(a) * m * 1000)} {int(-math.sqrt(max(0, 1 - m * m)) * 1000)}")


def wobble(a, b, s, ms, amp, seed=1, base=0, step=66):
    r = random.Random(seed)
    for _ in range(0, ms, step):
        tiltdeg(s, base + r.uniform(-amp, amp))
        a.cmd(f"tick {step}")
        b.cmd(f"tick {step}")


def to_race(a, b, track_next=0):
    to_lobby(a, b)
    for _ in range(track_next):
        a.tap(125, 76)
        both(a, b, 600)
    a.tap(106, 124)
    both(a, b, 3000)
    a.tap(106, 124)
    both(a, b, 300)
    b.tap(106, 124)
    both(a, b, 300)
