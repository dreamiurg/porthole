#!/usr/bin/env python3
"""Black-box driver (pro-playtest copy of ../adaptive-playtest/drv.py; adds a hit/slide log, a fine-frame window,
lane offset and an obstacle-seeing dodge, see README.md): replays session log into snap --serve, then runs new actions.
Usage: drv.py SESSION ACTION...   actions:
  tap X Y | hold X Y MS | tick MS | tilt X Y Z | frame NAME | shot NAME (frame saved as png)
  drive SEC LABEL [every_ms] [style]  -- pixel-only steering helper, frames every_ms
  dbg CMD... (supplementary only)
New actions' raw commands are appended to SESSION. Everything it writes (the sim's working directory and the frames)
goes under apps/porthole/build/adaptive-playtest/, or $RALLY_DRV_DIR. It runs that directory's `snapf` (a frozen copy
of the sim) when there is one, else apps/porthole/build/host/snap.
"""

import json
import math
import os
import random
import subprocess
import sys
from PIL import Image

HERE = os.path.join(os.path.dirname(os.path.abspath(__file__)), "..", "adaptive-playtest")
APP = os.path.normpath(os.path.join(HERE, "..", "..", ".."))
WORK = os.environ.get("RALLY_DRV_DIR") or os.path.join(APP, "build", "adaptive-playtest")
OUT = os.path.join(WORK, "frames")
KID_BOOST_S = 8  # "kid" style taps Boost about this often


def snap():
    frozen = os.path.join(WORK, "snapf")
    return frozen if os.path.exists(frozen) else os.path.join(APP, "build", "host", "snap")


class Sim:
    def __init__(self, wd):
        os.makedirs(os.path.join(wd, "build/host"), exist_ok=True)
        self.p = subprocess.Popen([snap(), "--serve"], cwd=wd, stdin=subprocess.PIPE, stdout=subprocess.PIPE)
        self.log = []
        self.t = 0
        self.tilt = (0, 1000, 0)

    def cmd(self, c, record=True):
        if record:
            self.log.append(c)
        self.p.stdin.write((c + "\n").encode())
        self.p.stdin.flush()
        if c.startswith("frame"):
            hdr = self.p.stdout.readline().decode()
            n = int(hdr.split()[1])
            data = b""
            while len(data) < n:
                data += self.p.stdout.read(n - len(data))
            return Image.frombytes("RGB", (480, 480), data)
        out = self.p.stdout.readline().decode().strip()
        if c.startswith("tick"):
            self.t += int(c.split()[1])
        return out

    def frame(self):
        return self.cmd("frame", record=False)


def tap(s, x, y, ms=66):
    s.cmd(f"down {x} {y}")
    s.cmd(f"tick {ms}")
    s.cmd("up")


# ---------- pixel steering helper (reads only rendered pixels) ----------
REF = [None]


def is_road(px):
    r = REF[0]
    return r is not None and sum(abs(a - b) for a, b in zip(px, r, strict=True)) < 45


def learn_ref(img):
    from collections import Counter

    c = Counter(img.getpixel((x, 320)) for x in range(170, 310, 2))
    REF[0] = c.most_common(1)[0][0]


def road_center(img, y):
    xs = [x for x in range(40, 440, 3) if is_road(img.getpixel((x, y)))]
    if len(xs) < 5:
        return None
    # take the longest contiguous run
    runs, start, prev = [], xs[0], xs[0]
    for x in xs[1:]:
        if x - prev > 36:
            runs.append((start, prev))
            start = x
        prev = x
    runs.append((start, prev))
    a, b = max(runs, key=lambda r: r[1] - r[0])
    return (a + b) / 2, b - a


def player_x(img):
    # the kid's red car near the bottom centre
    xs = []
    for y in range(330, 420, 3):
        for x in range(120, 360, 3):
            r, g, b = img.getpixel((x, y))
            if r > 170 and g < 70 and b < 70:
                xs.append(x)
    return sum(xs) / len(xs) if xs else None


DODGE = [False]
RING = []
HIT = [False, 0]
LASTX = [None]
HITLOG = open(os.path.join(WORK, "hits.log"), "a")


def blocker(img, pc):
    xs = []
    for y in range(270, 350, 3):
        for x in range(max(40, int(pc) - 70), min(440, int(pc) + 70), 3):
            r, g, b = img.getpixel((x, y))
            if (g > 150 and r < 110 and b < 110) or (b > 160 and r < 100) or (r > 200 and g > 160 and b < 70):
                xs.append(x)
    return sum(xs) / len(xs) if len(xs) > 40 else None


SEE = [False]
SEEW = int(os.environ.get("SEEW", "45"))


def blocker_see(img, pc):
    # anything on the road ahead that is not road colour or a lane marking
    r = REF[0]
    xs = []
    for y in range(int(os.environ.get("SEEY", "255")), 340, 3):
        rc = road_center(img, y)
        if rc is None:
            continue
        lo, hi = rc[0] - rc[1] / 2 + 8, rc[0] + rc[1] / 2 - 8
        for x in range(max(40, int(pc) - SEEW, int(lo)), min(440, int(pc) + SEEW, int(hi)), 3):
            p = img.getpixel((x, y))
            if max(p) - min(p) < 55 or sum(abs(a - b) for a, b in zip(p, r, strict=True)) < 70:
                continue
            if min(p) > 190 or (p[0] > 200 and p[1] > 170 and p[2] < 110):
                continue
            xs.append(x)
    return sum(xs) / len(xs) if len(xs) > int(os.environ.get("SEEN", "12")) else None


def rival_ahead(img):
    n = 0
    for y in range(250, 330, 4):
        for x in range(190, 290, 4):
            r, g, b = img.getpixel((x, y))
            if (g > 150 and r < 110 and b < 110) or (b > 160 and r < 100):
                n += 1
    return n > 4


def steer_target(img, gain, maxang):
    pc = player_x(img)
    rc = road_center(img, 300) or road_center(img, 330)
    rf = road_center(img, 250)
    if pc is None or rc is None:
        return 0.0
    err = rc[0] + int(os.environ.get("OFF", "0")) - pc
    ahead = (rf[0] - 240) if rf else 0
    if DODGE[0]:
        bx = blocker_see(img, pc) if SEE[0] else blocker(img, pc)
        if bx is not None:
            if SEE[0]:
                tgt = bx - 80 if pc <= bx else bx + 80
                err = tgt - pc
            else:
                err += -80 if bx >= pc else 80
    return max(-maxang, min(maxang, gain * (0.25 * err + 0.12 * ahead)))


def kid_presses(s, img, i, last_rocket):
    if i % (KID_BOOST_S * 10) == 5:
        tap(s, 142, 82)
    if rival_ahead(img) and i - last_rocket[0] > 30:
        last_rocket[0] = i
        tap(s, 18, 82)


def red_pixels(img):
    return sum(1 for x in range(195, 275, 2) for y in range(35, 80, 2) if img.getpixel((x, y))[0] > 200 and img.getpixel((x, y))[1] < 120 and img.getpixel((x, y))[2] < 100)


def log_hits(s, img, label):
    red = red_pixels(img)
    RING.append((s.t, img))
    del RING[:-16]
    if red > 150 and not HIT[0]:
        HITLOG.write(f"{label} hit t={s.t}\n")
        HITLOG.flush()
        HIT[1] = 3
        for t0, im in RING:
            im.save(os.path.join(OUT, f"{label}-hit-{t0:07d}.png"))
    elif HIT[1] > 0:
        HIT[1] -= 1
        img.save(os.path.join(OUT, f"{label}-hit-{s.t:07d}.png"))
    HIT[0] = red > 150


def log_slides(s, img, label):
    px = player_x(img)
    if px is not None and LASTX[0] is not None and abs(px - LASTX[0]) > 18:
        HITLOG.write(f"{label} slide t={s.t} dx={px - LASTX[0]:.0f}\n")
        HITLOG.flush()
        for t0, im in RING[-8:]:
            im.save(os.path.join(OUT, f"{label}-hit-{t0:07d}.png"))
    LASTX[0] = px


def save_frame(s, img, label, i, every):
    fine = os.environ.get("FINE")
    fa, fb = map(int, fine.split("-")) if fine else (1, 0)
    if (every and (i * 99) % every < 99) or fa <= s.t <= fb:
        img.save(os.path.join(OUT, f"{label}-{s.t:07d}.png"))


def drive(s, sec, label, every, opt):
    """opt: style, gain, noise, maxang, kid."""
    last_rocket = [-99]
    rnd = random.Random(7)
    ang = 0.0
    for i in range(int(sec * 1000 / 99)):
        img = s.frame()
        log_hits(s, img, label)
        log_slides(s, img, label)
        if REF[0] is None:
            learn_ref(img)
        save_frame(s, img, label, i, every)
        target = steer_target(img, opt.get("gain", 1.0), opt.get("maxang", 60))
        if opt.get("noise"):
            target += rnd.gauss(0, opt["noise"])
        if opt.get("kid"):
            kid_presses(s, img, i, last_rocket)
        ang += (target - ang) * (0.6 if opt.get("style", "smooth") == "smooth" else 0.9)
        th = math.radians(ang)
        s.cmd(f"tilt {int(1000 * math.sin(th))} {int(1000 * math.cos(th))} 0")
        s.cmd("tick 99")
    return s.frame()


def drive_action(s, a):
    every = int(a[3]) if len(a) > 3 else 2000
    style = a[4] if len(a) > 4 else "smooth"
    gain = float(a[5]) if len(a) > 5 else 1.0
    noise = float(a[6]) if len(a) > 6 else 0.0
    kid = len(a) > 7 and a[7].startswith("kid")
    DODGE[0] = len(a) > 7 and a[7] in ("kiddodge", "kidsee")
    SEE[0] = len(a) > 7 and a[7] == "kidsee"
    drive(s, float(a[1]), a[2], every, {"style": style, "gain": gain, "noise": noise, "kid": kid})


def run_action(s, act):
    a = act.split()
    k = a[0]
    if k == "tap":
        tap(s, a[1], a[2])
    elif k == "hold":
        s.cmd(f"down {a[1]} {a[2]}")
        s.cmd(f"tick {a[3]}")
        s.cmd("up")
    elif k in ("tick", "tilt", "dbg", "down", "move", "up", "skip"):
        s.cmd(act)
    elif k == "shot":
        s.frame().save(os.path.join(OUT, a[1] + ".png"))
        print("shot", a[1], "t=", s.t)
    elif k == "drive":
        drive_action(s, a)
    else:
        print("unknown", act)


def main():
    session = sys.argv[1]
    os.makedirs(OUT, exist_ok=True)
    wd = os.path.join(WORK, "wd-" + os.path.basename(session).split(".")[0])
    subprocess.run(["rm", "-rf", wd])
    os.makedirs(os.path.join(wd, "build/host"))
    subprocess.run([snap(), "--fresh", "--script", os.path.join(HERE, "fixture.txt")], cwd=wd, stdout=subprocess.DEVNULL, check=True)
    s = Sim(wd)
    old = []
    if os.path.exists(session):
        with open(session) as f:
            old = json.load(f)
    for c in old:
        s.cmd(c)
    for act in sys.argv[2:]:
        run_action(s, act)
    with open(session, "w") as f:
        json.dump(s.log, f)
    s.p.stdin.close()
    s.p.wait()


if __name__ == "__main__":
    main()
