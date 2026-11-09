#!/usr/bin/env python3
"""Replay a touch trace against a fresh fixture copy of snap --serve; save named frames as PNG.
Trace grammar: tap X Y | hold X Y MS | down X Y | move X Y | up | tick MS | shot NAME | restart | # comment"""

import glob
import os
import shutil
import subprocess
import sys
from PIL import Image

APP = "/home/user/porthole/apps/porthole"
FIX = "/home/user/porthole/.superpowers/sdd/2026-11-08-pets-club-shelter/fixture"
trace, out = sys.argv[1], sys.argv[2]
os.makedirs(out, exist_ok=True)
WD = os.environ.get("WD", APP)  # isolated workdir for parallel runs (saves go to WD/build/host)
os.makedirs(WD + "/build/host", exist_ok=True)
for f in glob.glob(WD + "/build/host/*.sav"):
    os.remove(f)
for f in glob.glob(FIX + "/*.sav"):
    shutil.copy(f, WD + "/build/host/")


def start():
    return subprocess.Popen([APP + "/build/host/snap", "--serve"], cwd=WD, stdin=subprocess.PIPE, stdout=subprocess.PIPE)


p = start()


def cmd(l):
    p.stdin.write((l + "\n").encode())
    p.stdin.flush()
    return p.stdout.readline()


def frame(name):
    p.stdin.write(b"frame\n")
    p.stdin.flush()
    n = int(p.stdout.readline().split()[1])
    buf = b""
    while len(buf) < n:
        buf += p.stdout.read(n - len(buf))
    Image.frombytes("RGB", (480, 480), buf).save(os.path.join(out, name + ".png"))


for raw in open(trace):
    l = raw.split("#")[0].strip()
    if not l:
        continue
    w = l.split()
    if w[0] == "tap":
        cmd(f"down {w[1]} {w[2]}")
        cmd("tick 66")
        cmd("up")
    elif w[0] == "hold":
        cmd(f"down {w[1]} {w[2]}")
        cmd(f"tick {w[3]}")
        cmd("up")
    elif w[0] == "shot":
        frame(w[1])
    elif w[0] == "restart":
        p.stdin.close()
        p.wait()
        p = start()
    elif w[0] in ("down", "move", "up", "tick"):
        cmd(l)
    else:
        sys.exit("bad line: " + l)
frame("last")
p.stdin.close()
p.wait()
