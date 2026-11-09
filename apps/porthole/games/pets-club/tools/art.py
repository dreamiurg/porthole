#!/usr/bin/env python3
"""Pixel-art generator for Pets Club.

Dogs are drawn by a tiny parametric rig (ellipses + rects + inner outline) so every pose exists at
three sizes (puppy / dog / grown) with one consistent style. Icons are hand-drawn ASCII.
Emits games/pets-club/sprites.h and a preview sheet at build/art/sheet.png (needs Pillow for the preview).
"""

import os
import sys
from pathlib import Path

SPRITES = "games/pets-club/sprites.h"
T = 255
# palette indices (see palette.h)
K, NAVY, PLUM, DKGREEN, BROWN, DKGRAY, LTGRAY, WHITE, RED, ORANGE, YELLOW, GREEN, BLUE, LAV, PINK, PEACH = range(16)
TAN, DKBROWN, CREAM, SKY, NIGHT, WOOD, DKWOOD, WALL, FLOOR, MINT, LEAF, GOLD, MUD, WATER, ROSE, SLATE = range(16, 32)
LEGEND = {
    ".": T,
    "k": K,
    "n": NAVY,
    "m": PLUM,
    "G": DKGREEN,
    "b": BROWN,
    "d": DKGRAY,
    "l": LTGRAY,
    "w": WHITE,
    "r": RED,
    "o": ORANGE,
    "y": YELLOW,
    "g": GREEN,
    "B": BLUE,
    "v": LAV,
    "p": PINK,
    "P": PEACH,
    "t": TAN,
    "D": DKBROWN,
    "c": CREAM,
    "s": SKY,
    "N": NIGHT,
    "W": WOOD,
    "X": DKWOOD,
    "a": WALL,
    "f": FLOOR,
    "M": MINT,
    "L": LEAF,
    "O": GOLD,
    "u": MUD,
    "q": WATER,
    "R": ROSE,
    "S": SLATE,
}


class Img:
    def __init__(self, w, h):
        self.w, self.h, self.px = w, h, [[T] * w for _ in range(h)]

    def set(self, x, y, c):
        x, y = int(round(x)), int(round(y))
        if 0 <= x < self.w and 0 <= y < self.h:
            self.px[y][x] = c

    def get(self, x, y):
        return self.px[y][x] if 0 <= x < self.w and 0 <= y < self.h else T

    def ellipse(self, cx, cy, rx, ry, c):
        for y in range(self.h):
            for x in range(self.w):
                if ((x + 0.5 - cx) / rx) ** 2 + ((y + 0.5 - cy) / ry) ** 2 <= 1.0:
                    self.px[y][x] = c

    def rect(self, x, y, w, h, c):
        x, y, w, h = int(round(x)), int(round(y)), int(round(w)), int(round(h))
        for yy in range(y, y + h):
            for xx in range(x, x + w):
                self.set(xx, yy, c)

    def line(self, x0, y0, x1, y1, c, thick=1):
        x0, y0, x1, y1 = int(round(x0)), int(round(y0)), int(round(x1)), int(round(y1))
        n = max(abs(x1 - x0), abs(y1 - y0), 1)
        for i in range(n + 1):
            x = round(x0 + (x1 - x0) * i / n)
            y = round(y0 + (y1 - y0) * i / n)
            for dx in range(thick):
                for dy in range(thick):
                    self.set(x + dx, y + dy, c)

    def outline(self, col=DKBROWN, skip=()):
        """Inner outline: filled pixels touching transparency (or the edge) become col."""
        mark = []
        for y in range(self.h):
            for x in range(self.w):
                c = self.px[y][x]
                if c == T or c in skip:
                    continue
                if any(self.get(x + dx, y + dy) == T for dx, dy in ((1, 0), (-1, 0), (0, 1), (0, -1))):
                    mark.append((x, y))
        for x, y in mark:
            self.px[y][x] = col

    def paste(self, other, x, y):
        for yy in range(other.h):
            for xx in range(other.w):
                c = other.px[yy][xx]
                if c != T:
                    self.set(x + xx, y + yy, c)

    def tint(self, ell, c, only, mask=None):
        """Recolor the `only`-colored pixels inside ell=(cx, cy, rx, ry) where mask(x, y) holds: markings that follow the silhouette."""
        cx, cy, rx, ry = ell
        for y in range(self.h):
            for x in range(self.w):
                if self.px[y][x] == only and ((x + 0.5 - cx) / rx) ** 2 + ((y + 0.5 - cy) / ry) ** 2 <= 1.0 and (mask is None or mask(x, y)):
                    self.px[y][x] = c

    def tri(self, pts, c):
        """Fill the triangle pts=[(x, y)] * 3, by pixel centers."""
        (ax, ay), (bx, by), (qx, qy) = pts
        for y in range(self.h):
            for x in range(self.w):
                px, py = x + 0.5, y + 0.5
                d = ((bx - ax) * (py - ay) - (by - ay) * (px - ax), (qx - bx) * (py - by) - (qy - by) * (px - bx), (ax - qx) * (py - qy) - (ay - qy) * (px - qx))
                if min(d) >= 0 or max(d) <= 0:
                    self.px[y][x] = c


def ascii_img(rows):
    rows = [r for r in rows.strip("\n").split("\n")]
    w = max(len(r) for r in rows)
    im = Img(w, len(rows))
    for y, r in enumerate(rows):
        for x, ch in enumerate(r.ljust(w, ".")):
            im.px[y][x] = LEGEND[ch]
    return im


# ---------------------------------------------------------------- dog rig
# Proportions per size. All coordinates in sprite pixels; dog faces right.
SIZES = {
    "pup": dict(
        W=24,
        H=18,
        body=(10, 10.5, 6, 4),
        head=(16, 7, 5),
        muzzle=(20.5, 9, 2.5, 2),
        ear=(12.5, 7.5, 2, 3.2),
        legY=13,
        legH=4,
        legW=2,
        backX=(6, 9),
        frontX=(12, 15),
        tail=(5, 9),
        collar=None,
        eye=(17, 5),
        nose=(22, 8),
    ),
    "dog": dict(
        W=32,
        H=24,
        body=(14, 14, 9, 5.5),
        head=(23, 9, 6),
        muzzle=(28.5, 11, 3, 2.5),
        ear=(18.5, 9.5, 2.2, 4.2),
        legY=17,
        legH=6,
        legW=2,
        backX=(6, 10),
        frontX=(17, 21),
        tail=(6, 12),
        collar=(19, 14),
        eye=(25, 7),
        nose=(30, 10),
    ),
    "big": dict(
        W=36,
        H=27,
        body=(16, 16, 10, 6),
        head=(26, 10, 7),
        muzzle=(32.5, 12.5, 3.5, 3),
        ear=(21, 10.5, 2.5, 4.8),
        legY=20,
        legH=6,
        legW=3,
        backX=(7, 12),
        frontX=(20, 25),
        tail=(7, 14),
        collar=(22, 16),
        eye=(28.5, 8),
        nose=(34, 11.5),
    ),
}
POSES = ["IDLE0", "IDLE1", "WALK0", "WALK1", "SIT", "SIT_PAW", "SLEEP0", "SLEEP1", "EAT0", "EAT1", "JUMP", "SAD", "BACK", "DEAD", "BEG", "BARK", "LISTEN"]
SIZE_ORDER = ("pup", "dog", "big")

# ---------------------------------------------------------------- breeds
# A breed is colors plus a few shape knobs on the one rig; breed 0 (the Golden pup) is the original dog, pixel for pixel.
# ears: floppy (the original), long (hangs past the jaw), pointy / upright (triangles above the head).
# short: legs cut per size, with everything above them lowered so the feet stay on the ground line.
GOLDEN = dict(coat=TAN, legs=TAN, belly=CREAM, muzzle=CREAM, ear=BROWN, ears="floppy", line=DKBROWN, ink=DKBROWN, paw=CREAM, pupil=K)
GOLDEN.update(saddle=None, cap=None, tip=None, spots=None, curls=None, short=None)
BREEDS = {  # order = enum Breed = save_.breed
    "GOLDEN": GOLDEN,
    "DALMATIAN": {**GOLDEN, "coat": WHITE, "legs": WHITE, "belly": WHITE, "muzzle": WHITE, "ear": K, "paw": WHITE, "spots": K},
    "HUSKY": {
        **GOLDEN,
        "coat": LTGRAY,
        "legs": WHITE,
        "belly": WHITE,
        "muzzle": WHITE,
        "ear": DKGRAY,
        "ears": "pointy",
        "paw": WHITE,
        "pupil": BLUE,
        "saddle": DKGRAY,
        "cap": DKGRAY,
    },
    "BEAGLE": {**GOLDEN, "coat": WOOD, "belly": WHITE, "legs": WOOD, "muzzle": WHITE, "ear": DKWOOD, "ears": "long", "paw": WHITE, "saddle": K, "tip": WHITE},
    "POODLE": {**GOLDEN, "coat": CREAM, "legs": CREAM, "belly": CREAM, "muzzle": CREAM, "ear": PEACH, "ears": "long", "curls": TAN},
    "CORGI": {
        **GOLDEN,
        "coat": ORANGE,
        "legs": ORANGE,
        "belly": WHITE,
        "muzzle": WHITE,
        "ear": ORANGE,
        "ears": "upright",
        "paw": WHITE,
        "short": {"pup": 1, "dog": 2, "big": 2},
    },
    "LAB": {**GOLDEN, "coat": K, "legs": K, "belly": K, "muzzle": SLATE, "ear": SLATE, "line": DKGRAY, "ink": LTGRAY, "paw": K},
}
SPOTS = [(-0.6, -0.3), (0.1, -0.55), (0.5, 0.15), (-0.25, 0.35), (0.75, -0.35)]  # Dalmatian, in body radii from its center


def rig(size, b):
    p = dict(SIZES[size])
    k = (b["short"] or {}).get(size, 0)
    if k:
        p["legY"] += k
        p["legH"] -= k
        for key in ("body", "head", "muzzle", "ear", "tail", "collar", "eye", "nose"):
            if p[key]:
                p[key] = (p[key][0], p[key][1] + k, *p[key][2:])
    return p


# ---- pose families: each fills the pose state (head offset, legs, tail, body, belly, haunch, extra limbs, face)
def stand_legs(p, d=0):
    bk, fr, legY, legH = p["backX"], p["frontX"], p["legY"], p["legH"]
    return [(bk[0] - d, legY - 1, legH + 1), (bk[1] + d, legY, legH), (fr[0] - d, legY - 1, legH + 1), (fr[1] + d, legY, legH)]


def pose_stand(s, p, pose, size):
    bx, by, brx, bry = p["body"]
    hr, (tx, ty), legY, legH = p["head"][2], p["tail"], p["legY"], p["legH"]
    s["legs"] = stand_legs(p, {"WALK0": 1, "WALK1": -1}.get(pose, 0))
    s["tail"] = (tx, ty, tx - 4, ty - 5) if pose != "IDLE1" else (tx, ty, tx - 5, ty - 2)
    if pose in ("EAT0", "EAT1"):
        s.update(hdx=1, hdy=int(hr * 0.7), mouth="open" if pose == "EAT1" else "closed", tail=(tx, ty, tx - 5, ty - 3))
    if pose == "JUMP":
        bk, fr = p["backX"], p["frontX"]
        s.update(hdy=-3, body=(bx, by - 3, brx, bry), belly=(bx + 1, by - 3 + bry * 0.45, brx * 0.6, bry * 0.55), eyes="happy", mouth="open")
        s["legs"] = [(bk[0] - 1, legY - 3, legH - 1), (bk[1] + 1, legY - 2, legH - 2), (fr[0] - 1, legY - 3, legH - 1), (fr[1] + 1, legY - 2, legH - 2)]
        s["tail"] = (tx, ty - 3, tx - 4, ty - 8)
    if pose == "SAD":
        s.update(hdy=2, tail=(tx, ty, tx - 2, ty + 4), eyes="sad", mouth="sad")
    if pose == "BARK":
        s.update(hdy=-1, mouth="open", tail=(tx, ty, tx - 4, ty - 6))


def pose_sit(s, p, pose, size):
    bx, by, brx, bry = p["body"]
    fr, legY, legW, ground = p["frontX"], p["legY"], p["legW"], p["legY"] + p["legH"]
    s.update(haunch=(bx - 1, legY + 1, bry + 1.2, bry + 1.2), body=(bx + 2, by - 1.5, brx - 2.5, bry + 1.5), belly=(bx + 3, by + 0.5, brx * 0.45, bry * 0.8), hdy=-1)
    s["legs"] = [(fr[0], int(by - 1), ground - int(by - 1)), (fr[1], int(by - 1), ground - int(by - 1))]
    s["tail"] = (int(bx - bry - 1), ground - 1, int(bx - bry - 5), ground - 3)
    if pose == "SIT_PAW":
        s["legs"] = s["legs"][:1]
        s["extra"].append(lambda im, c: im.line(fr[1], int(by), fr[1] + 4, int(by) - 4, c, legW))
    if pose == "LISTEN":
        s.update(hdx=1, hdy=-2, eyes="big")


def pose_sleep(s, p, pose, size):
    bx, by, brx, bry = p["body"]
    hy, hr, tx, ground = p["head"][1], p["head"][2], p["tail"][0], p["legY"] + p["legH"]
    lift = 1 if pose == "SLEEP1" else 0
    s.update(body=(bx + 1, ground - bry + 0.5 - lift, brx + 1, bry - 0.5 + lift * 0.5), belly=None, hdx=-2, hdy=ground - hr - hy - 1, eyes="closed")
    s["extra"].append(lambda im, c: im.rect(p["frontX"][0] + 1, ground - 2, 5, 2, c))
    s["tail"] = (tx, ground - 2, tx - 4, ground - 1)


def pose_back(s, p, pose, size):
    bx, by, brx, bry = p["body"]
    bk, fr, legH, ground = p["backX"], p["frontX"], p["legH"], p["legY"] + p["legH"]
    hy, hr, tx = p["head"][1], p["head"][2], p["tail"][0]
    s.update(body=(bx + 1, ground - bry - 1, brx, bry), belly=(bx + 1, ground - bry - 1, brx * 0.7, bry * 0.6), hdx=0, hdy=ground - hr - hy - 2)
    top = ground - 2 * bry
    s["legs"] = [(bk[0], int(top - 3), legH - 1), (bk[1] + 1, int(top - 2), legH - 2), (fr[0] - 1, int(top - 3), legH - 1), (fr[1], int(top - 2), legH - 2)]
    s.update(eyes="x" if pose == "DEAD" else "open", mouth="tongue" if pose == "DEAD" else "closed", tail=(tx, ground - 2, tx - 4, ground - 1), belly_up=True)


def pose_beg(s, p, pose, size):
    bx, by, brx, bry = p["body"]
    legY, legW, ground = p["legY"], p["legW"], p["legY"] + p["legH"]
    paw = 3 if size != "big" else 4
    s.update(haunch=(bx - 1, legY + 1, bry + 1.2, bry + 1.2), body=(bx + 2, by - 3, bry + 0.5, brx - 2.5), belly=(bx + 3, by - 1.5, bry * 0.5, brx * 0.45), hdx=-3, hdy=-4)
    s["extra"].append(lambda im, c: im.rect(int(bx + bry + 1), int(by - 3), paw, legW, c))
    s["extra"].append(lambda im, c: im.rect(int(bx + bry + 1), int(by), paw, legW, c))
    s["tail"] = (int(bx - bry - 1), ground - 1, int(bx - bry - 5), ground - 3)


POSE_FAMILY = {"SIT": pose_sit, "SIT_PAW": pose_sit, "LISTEN": pose_sit, "SLEEP0": pose_sleep, "SLEEP1": pose_sleep, "BACK": pose_back, "DEAD": pose_back, "BEG": pose_beg}


def pose_state(p, pose, size):
    bx, by, brx, bry = p["body"]
    s = dict(
        eyes="open", mouth="closed", hdx=0, hdy=0, legs=[], tail=None, body=(bx, by, brx, bry), belly=(bx + 1, by + bry * 0.45, brx * 0.6, bry * 0.55), haunch=None, extra=[]
    )
    s["belly_up"] = False
    POSE_FAMILY.get(pose, pose_stand)(s, p, pose, size)
    return s


# ---- painting
def paint_ear(im, p, s, b):
    ex, ey, erx, ery = p["ear"]
    hx, hy, hr = p["head"]
    HX, HY = hx + s["hdx"], hy + s["hdy"]
    if b["ears"] == "floppy":
        im.ellipse(ex + s["hdx"], ey + s["hdy"], erx, ery, b["ear"])
    elif b["ears"] == "long":
        im.ellipse(ex + s["hdx"], ey + s["hdy"] + ery * 0.5, erx * 1.1, ery * 1.5, b["ear"])
    else:
        ax, ay = HX - hr * 0.4, HY - hr * (1.45 if b["ears"] == "pointy" else 1.8)
        if ay < 0:  # a jumping or begging head would push the tip off the sprite: the ear folds back instead
            ax, ay = ax + ay, 0
        im.tri([(HX - hr * 0.9, HY - hr * 0.2), (ax, ay), (HX + hr * 0.25, HY - hr * 0.8)], b["ear"])


def puffs(p, s):
    """Poodle pompoms: topknot, tail tip, one per standing paw."""
    hx, hy, hr = p["head"]
    legW, ground = p["legW"], p["legY"] + p["legH"]
    out = [(hx + s["hdx"] - hr * 0.2, max(hy + s["hdy"] - hr * 0.9, hr * 0.6), hr * 0.8, hr * 0.6)]
    if s["tail"]:
        out.append((s["tail"][2] + 1, s["tail"][3] + 1, hr * 0.35, hr * 0.35))
    out += [(lx + legW / 2, ground - 1.2, legW / 2 + 0.8, 1.3) for lx, ly, lh in s["legs"] if ly + lh >= ground]
    return out


def paint_markings(im, p, s, b, size):
    """Post-outline: Dalmatian spots and Poodle curls, only on interior coat pixels so the outline stays whole."""
    if b["spots"] is not None:
        bx, by, brx, bry = s["body"]
        hx, hy, hr = p["head"]
        r, off = (0.8, 0.5) if size == "pup" else (1.2, 0)
        pts = [(bx + fx * brx, by + fy * bry) for fx, fy in SPOTS] + [(hx + s["hdx"] - hr * 0.35, hy + s["hdy"] - hr * 0.5)]
        for x, y in pts:
            im.tint((int(x) + off, int(y) + off, r, r), b["spots"], b["coat"])
    if b["curls"] is not None:
        for ell in puffs(p, s):
            im.tint(ell, b["curls"], b["coat"], lambda x, y: y % 2 == 0 and (x + y // 2) % 2 == 0)


def paint_silhouette(im, p, s, b):
    """Draw order: tail, haunch, body, belly, legs, extra, head, muzzle, ear; then the outline."""
    coat, sy = b["coat"], 1 if s["belly_up"] else -1
    if s["tail"]:
        im.line(*s["tail"], coat, 2)
        if b["tip"] is not None:
            im.rect(s["tail"][2], s["tail"][3], 2, 2, b["tip"])
    if s["haunch"]:
        im.ellipse(*s["haunch"], coat)
    bx, by, brx, bry = s["body"]
    im.ellipse(bx, by, brx, bry, coat)
    if b["saddle"] is not None:
        im.tint((bx - brx * 0.1, by + sy * bry * 0.55, brx * 0.8, bry * 0.75), b["saddle"], coat)
    if s["belly"]:
        im.ellipse(*s["belly"], b["belly"])
    for lx, ly, lh in s["legs"]:
        im.rect(lx, ly, p["legW"], lh, b["legs"])
    for fn in s["extra"]:
        fn(im, b["legs"])
    hx, hy, hr = p["head"]
    HX, HY = hx + s["hdx"], hy + s["hdy"]
    im.ellipse(HX, HY, hr, hr, coat)
    if b["cap"] is not None:
        im.tint((HX - hr * 0.2, HY - hr * 0.75, hr * 0.95, hr * 0.7), b["cap"], coat)
    mx, my, mrx, mry = p["muzzle"]
    im.ellipse(mx + s["hdx"], my + s["hdy"], mrx, mry, b["muzzle"])
    paint_ear(im, p, s, b)
    if b["curls"] is not None:
        for ell in puffs(p, s):
            im.ellipse(*ell, coat)
    im.outline(b["line"])


def paint_eyes(im, EX, EY, eyes, b):
    ink, pupil = b["ink"], b["pupil"]
    if eyes in ("open", "sad"):
        im.rect(EX, EY, 2, 2, WHITE)
        im.set(EX + 1, EY + 1, pupil)
        if eyes == "sad":
            im.set(EX - 1, EY - 1, ink)
            im.set(EX, EY - 1, ink)
    elif eyes == "big":
        im.rect(EX - 1, EY, 3, 3, WHITE)
        im.set(EX + 1, EY + 1, pupil)
        im.set(EX + 1, EY + 2, pupil)
    elif eyes == "closed":
        im.rect(EX, EY + 1, 3, 1, ink)
    elif eyes == "happy":
        for dx, dy in ((0, 1), (1, 0), (2, 1)):
            im.set(EX + dx, EY + dy, ink)
    elif eyes == "x":
        for dx, dy in ((0, 0), (2, 0), (1, 1), (0, 2), (2, 2)):
            im.set(EX + dx, EY + dy, ink)


def paint_mouth(im, NX, NY, mouth, ink):
    im.rect(NX - 1, NY, 2, 2, K)
    if mouth == "open":
        im.rect(NX - 3, NY + 3, 3, 2, PINK)
        im.rect(NX - 4, NY + 2, 1, 1, ink)
        im.rect(NX - 3, NY + 2, 3, 1, ink)
    elif mouth == "tongue":
        im.rect(NX - 2, NY + 3, 2, 3, PINK)
        im.set(NX - 2, NY + 2, ink)
    elif mouth == "sad":
        im.set(NX - 3, NY + 2, ink)
        im.set(NX - 2, NY + 3, ink)
    else:
        im.set(NX - 2, NY + 2, ink)


def paint_collar(im, p, s, pose):
    if not p["collar"] or pose in ("BACK", "DEAD", "SLEEP0", "SLEEP1"):  # not on the puppy, nor lying down
        return
    cx, cy = p["collar"][0] + s["hdx"], p["collar"][1] + s["hdy"]
    if pose in ("SIT", "SIT_PAW", "LISTEN"):
        cx += 1
    if pose == "BEG":
        cx, cy = cx - 2, cy - 3
    im.rect(cx - 1, cy, 4, 1, RED)
    im.set(cx, cy + 1, GOLD)


def draw_dog(size, pose, breed="GOLDEN"):
    b = BREEDS[breed]
    p = rig(size, b)
    s = pose_state(p, pose, size)
    im = Img(p["W"], p["H"])
    paint_silhouette(im, p, s, b)
    ground = p["legY"] + p["legH"]
    for lx, ly, lh in s["legs"]:  # paws (toes) at leg bottoms
        if ly + lh >= ground:
            im.rect(lx, ground - 1, p["legW"], 1, b["paw"])
    paint_markings(im, p, s, b, size)
    EX, EY = p["eye"][0] + s["hdx"], p["eye"][1] + s["hdy"]
    paint_eyes(im, EX, EY, s["eyes"], b)
    paint_mouth(im, p["nose"][0] + s["hdx"], p["nose"][1] + s["hdy"], s["mouth"], b["ink"])
    paint_collar(im, p, s, pose)
    # anchors for hats/zones: head top-center, eye, body center, tail
    hx, hy, hr = p["head"]
    body, tail = s["body"], s["tail"]
    parts = dict(headX=int(round(hx + s["hdx"])), headY=int(round(hy + s["hdy"])), headR=int(hr))
    parts.update(bodyX=int(round(body[0])), bodyY=int(round(body[1])), bodyRX=int(round(body[2])), bodyRY=int(round(body[3])))
    parts.update(tailX=int(tail[2]) if tail else 0, tailY=int(tail[3]) if tail else 0, eyeX=int(EX), eyeY=int(EY))
    return im, parts


# ---------------------------------------------------------------- icons
ICONS = {
    "HEART": """
.rr.rr.
rwrrrrr
rrrrrrr
.rrrrr.
..rrr..
...r...
""",
    "HEART_EMPTY": """
.dd.dd.
d..d..d
d.....d
.d...d.
..d.d..
...d...
""",
    "BONE": """
.dd....dd.
dwwd..dwwd
dwwwwwwwwd
dwwwwwwwwd
dwwd..dwwd
.dd....dd.
""",
    "BOOK": """
kkkkk.kkkkk
kwwwwkwwwwk
kwlwwkwwlwk
kwwwwkwwwwk
kwlwwkwwlwk
kwwwwkwwwwk
kkkkkkkkkkk
""",
    "PAW": """
..k..k..
.kk..kk.
k..kk..k
kk.kk.kk
..kkkk..
.kkkkkk.
.kkkkkk.
..kkkk..
""",
    "BALL": """
..kkkk..
.krrrrk.
krwrrrrk
krrrrrrk
kwwwwwwk
kBBBBBBk
.kBBBBk.
..kkkk..
""",
    "BOWL": """
....bDbb....
...bbDbbb...
.BBBBBBBBBB.
BBBBBBBBBBBB
.BBBBBBBBBB.
..BBBBBBBB..
..nnnnnnnn..
""",
    "POOP": """
....D....
...DbD...
..DbbbD..
.DbbbbbD.
.DbbbbbD.
DbwbbbwbD
DbbbbbbbD
.DDDDDDD.
""",
    "TUB": """
..........ww
........ww..
wwwwwwwwwwww
wqqqqqqqqqqw
.wwwwwwwwww.
..wwwwwwww..
..w......w..
..w......w..
""",
    "BUBBLE": """
.qqq.
qwqqq
qqqqq
qqqqq
.qqq.
""",
    "LAMP_ON": """
...yyy...
..yyyyy..
.yyyyyyy.
yyyyyyyyy
yyyyyyyyy
....d....
....d....
....d....
....d....
...ddd...
..ddddd..
""",
    "LAMP_OFF": """
...vvv...
..vvvvv..
.vvvvvvv.
vvvvvvvvv
vvvvvvvvv
....d....
....d....
....d....
....d....
...ddd...
..ddddd..
""",
    "STAR": """
...y...
..yyy..
yyyyyyy
.yyyyy.
..yyy..
.yy.yy.
.y...y.
""",
    "PARCEL": """
......rr......
.....r..r.....
WWWWWWrrWWWWWW
WWWWWWrrWWWWWW
WWWWWWrrWWWWWW
rrrrrrrrrrrrrr
WWWWWWrrWWWWWW
WWWWWWrrWWWWWW
WWWWWWrrWWWWWW
WWWWWWrrWWWWWW
WWWWWWrrWWWWWW
XXXXXXXXXXXXXX
""",
    "COOKIE": """
..tttt..
.tttttt.
ttDttttt
tttttDtt
tDtttttt
ttttDttt
.tttttt.
..tttt..
""",
    "ARROW": """
k.....
kk....
kkk...
kkkk..
kkkkk.
kkkk..
kkk...
kk....
k.....
""",
    "CHECK": """
......k.
.....kk.
....kk..
k..kk...
kkkk....
.kk.....
""",
    "CROSS": """
k.....k
kk...kk
.kk.kk.
..kkk..
.kk.kk.
kk...kk
k.....k
""",
    "BUTTERFLY0": """
.pp...pp.
pppp.pppp
ppppkpppp
pppppkppp
.pppkppp.
..ppkpp..
...p.p...
""",
    "BUTTERFLY1": """
..p..
.pp..
.pkp.
.pkp.
.pkp.
..p..
.....
""",
    "SQUIRREL": """
.......bbbb.
......bbbbbb
.bb..bbbkbbb
bbbb.bbbbbbb
bbbbbbbbb...
bbbbbbbbb...
.bbbbbbbb...
..bbbbbbb...
....bb.bb...
...bb..bb...
""",
    "MUD": """
.uuuu.
uuuuuu
uuuuuu
.uuuu.
""",
    "DROP": """
.q.
.q.
qqq
qqq
.q.
""",
    "NOTE": """
...kk.
...k.k
...k..
...k..
...k..
.kkk..
kkkk..
.kk...
""",
    "SPARKLE": """
..y..
..y..
yywyy
..y..
..y..
""",
    "DUST": """
.ll.ll.
lllllll
.lllll.
..l.l..
""",
    "BEE": """
.ww.ww.
.wwww..
kykyky.
kykykyk
.kykyk.
...k...
""",
    "CAKE": """
.....y......
.....o......
.....k......
..pppppppp..
.pppppppppp.
.pwpwpwpwpp.
.pppppppppp.
.RRRRRRRRRR.
.RRRRRRRRRR.
.RRRRRRRRRR.
""",
    "SUN": """
....y....
.y..y..y.
..yyyyy..
.yyyyyyy.
yyyyyyyyy
.yyyyyyy.
..yyyyy..
.y..y..y.
....y....
""",
    "MOON": """
..ccc...
.cc.....
cc......
cc......
cc......
cc......
.cc.....
..ccc...
""",
    "CLOUD": """
....wwww....
..wwwwwwww..
.wwwwwwwwww.
wwwwwwwwwwww
wwwwwwwwwwww
.wwwwwwwwww.
""",
    "HAT_BOW": """
rr...rr
rrr.rrr
rrrrrrr
rrr.rrr
rr...rr
""",
    "HAT_PARTY": """
....y....
....y....
...BBB...
...ByB...
..BBBBB..
..ByyyB..
.BBBBBBB.
.ByyyyyB.
BBBBBBBBB
""",
    "HAT_GLASSES": """
kkkk.k
k..kkk
k..k..
k..k..
kkkk..
""",
    "HAT_BANDANA": """
rrrrrrrrr
.rrrrrrr.
..rwrrr..
...rrr...
....r....
""",
    "HAT_CROWN": """
O...O...O
OO.OOO.OO
OOOOOOOOO
OrOOOBOOO
OOOOOOOOO
""",
    "HAT_WIZARD": """
.....v.....
.....v.....
....vvv....
....vyv....
...vvvvv...
...vvvvv...
..vvvyvvv..
..vvvvvvv..
.vvvvvvvvv.
vvvvvvvvvvv
.OOOOOOOOO.
""",
    "HAT_FLOWER": """
.pp.pp.
pppyppp
.pyyyp.
pppyppp
.pp.pp.
...g...
...g...
""",
    "TROPHY": """
kOOOOOOOk
kOOOOOOOk
.kOOOOOk.
..kOOOk..
...kOk...
....O....
...kkk...
..kkkkk..
""",
    "MAIL": """
............
.BBBBBBBBBB.
BwwwwwwwwwwB
BwBwwwwwwBwB
BwwBwwwwBwwB
BwwwBwwBwwwB
BwwwwBBwwwwB
BwwwwwwwwwwB
.BBBBBBBBBB.
""",
    # Porthole launcher icon (about 32x32, drawn full-color with gfx::blit).
    "APP_ICON": """
................................
..........DDDDDDDDDDDD..........
........DDttttttttttttDD........
......DDttttttttttttttttDD......
.....DttttttttttttttttttttD.....
..DDDttttttttttttttttttttttDDD..
.DbbbbttttttttttttttttttttbbbbD.
DbbbbbbttttttttttttttttttbbbbbbD
DbbbbbbttttttttttttttttttbbbbbbD
DbbbbbbttttttttttttttttttbbbbbbD
DbbbbbbtwkkttttttttttkkwtbbbbbbD
DbbbbbbtkkkttttttttttkkktbbbbbbD
DbbbbbbtkkkttttttttttkkktbbbbbbD
DbbbbbbttttttttttttttttttbbbbbbD
DbbbbbbttttttttttttttttttbbbbbbD
DbbbbbbtttcccccccccccctttbbbbbbD
.DbbbbbttcccckkkkkkccccttbbbbbD.
.DbbbbbtpcccckkkkkkccccptbbbbbD.
..DbbbbtpccccckkkkcccccptbbbbD..
..DbbbbttcccccckkccccccttbbbbD..
...DbbbttcckkkkkkkkkkccttbbbD...
...DbbbttcckRRRRRRRRkccttbbbD...
....DbbttccckppppppkcccttbbD....
....DbbttcccckppppkccccttbbD....
.....DbttccccckkkkcccccttbD.....
......DttccccccccccccccttD......
.......DrrrrrrrrrrrrrrrrD.......
.......DrrrrrrrrrrrrrrrrD.......
.......DDDDDDDOOOODDDDDDD.......
..............DOOD..............
..............DDDD..............
................................
""",
}
HATS = ["HAT_BOW", "HAT_PARTY", "HAT_GLASSES", "HAT_BANDANA", "HAT_CROWN", "HAT_WIZARD", "HAT_FLOWER"]


# ---------------------------------------------------------------- emit
PART_KEYS = ("headX", "headY", "headR", "bodyX", "bodyY", "bodyRX", "bodyRY", "tailX", "tailY", "eyeX", "eyeY")


def px_data(im):
    return ",".join(str(c) for row in im.px for c in row)


# The memory wall's pictures, cut from the sitting pup: (name, crop) with crop (x, y, w, h) or, for the room's small
# frames, a 10x10 box around the head. A crop always stays on the art.
PORTRAITS = (("DOG_PORTRAIT_SMALL", None), ("DOG_PORTRAIT", (1, 0, 22, 14)))


def crop(im, x, y, w, h):
    x, y = max(0, min(x, im.w - w)), max(0, min(y, im.h - h))
    out = Img(w, h)
    out.px = [row[x : x + w] for row in im.px[y : y + h]]
    return out


def emit_portraits(w, frames):
    for name, box in PORTRAITS:
        for br in BREEDS:
            im, parts = frames[(br, "pup", "SIT")]
            pic = crop(im, *(box or (parts["headX"] - 5, parts["headY"] - 5, 10, 10)))
            w(f"static const uint8_t {name.lower()}_{br.lower()}_px[] = {{{px_data(pic)}}};")
        w(f"static const gfx::Sprite {name}[NUM_BREEDS_ART] = {{" + ", ".join(f"{{{pic.w},{pic.h},{name.lower()}_{br.lower()}_px}}" for br in BREEDS) + "};")


def emit_dogs(w, frames):
    w("enum Pose { " + ", ".join("P_" + p for p in POSES) + ", NUM_POSES };")
    w("enum DogSize { SZ_PUP = 0, SZ_DOG = 1, SZ_BIG = 2 };")
    w("enum Breed { " + ", ".join("BR_" + br for br in BREEDS) + ", NUM_BREEDS_ART };")
    w("struct DogParts { int8_t headX, headY, headR, bodyX, bodyY, bodyRX, bodyRY, tailX, tailY, eyeX, eyeY; };")
    for (br, sz, pose), (im, _) in frames.items():
        w(f"static const uint8_t dog_{br.lower()}_{sz}_{pose.lower()}_px[] = {{{px_data(im)}}};")
    w("static const gfx::Sprite DOG_FRAMES[NUM_BREEDS_ART][3][NUM_POSES] = {")
    for br in BREEDS:
        rows = ["{" + ", ".join(f"{{{frames[(br, sz, p)][0].w},{frames[(br, sz, p)][0].h},dog_{br.lower()}_{sz}_{p.lower()}_px}}" for p in POSES) + "}" for sz in SIZE_ORDER]
        w("  {" + ", ".join(rows) + "},")
    w("};")
    w("static const DogParts DOG_PARTS[NUM_BREEDS_ART][3][NUM_POSES] = {")
    for br in BREEDS:
        rows = ["{" + ", ".join("{" + ",".join(str(frames[(br, sz, p)][1][k]) for k in PART_KEYS) + "}" for p in POSES) + "}" for sz in SIZE_ORDER]
        w("  {" + ", ".join(rows) + "},")
    w("};")


def emit_icons(w):
    icons = {}
    for name, art in ICONS.items():
        im = icons[name] = ascii_img(art)
        w(f"static const uint8_t spr_{name.lower()}_px[] = {{{px_data(im)}}};")
        w(f"static const gfx::Sprite SPR_{name} = {{{im.w},{im.h},spr_{name.lower()}_px}};")
    w("static const gfx::Sprite SPR_HATS[8] = {{0,0,nullptr}, " + ", ".join("SPR_" + h for h in HATS) + "};")
    return icons


def emit(check=False):
    out = []
    w = out.append
    w("// AUTO-GENERATED by tools/art.py. Edit the generator, not this file.")
    w("#pragma once")
    w('#include "gfx.h"')
    frames = {(br, sz, pose): draw_dog(sz, pose, br) for br in BREEDS for sz in SIZE_ORDER for pose in POSES}
    emit_dogs(w, frames)
    emit_portraits(w, frames)
    icons = emit_icons(w)
    text = "\n".join(out) + "\n"
    if check:
        current = Path(SPRITES).read_text(encoding="utf-8") if Path(SPRITES).exists() else ""
        if current != text:
            print("sprites.h is out of date: run `make art`")
            sys.exit(1)
        print("sprites.h up to date")
        return
    with open(SPRITES, "w") as f:
        f.write(text)
    print(f"wrote {SPRITES} ({len(frames)} dog frames, {len(icons)} icons)")
    preview(frames, icons)


# ---------------------------------------------------------------- preview sheet
PAL = [0x000000, 0x1D2B53, 0x7E2553, 0x008751, 0xAB5236, 0x5F574F, 0xC2C3C7, 0xFFF1E8, 0xFF004D, 0xFFA300, 0xFFEC27, 0x00E436, 0x29ADFF, 0x83769C, 0xFF77A8, 0xFFCCAA]
PAL += [0xE6B27A, 0x5C3A1E, 0xFFF7D6, 0xA8E0FF, 0x16213E, 0xC47A3A, 0x8A4B22, 0xF2D9B5, 0xD9A066, 0x6EE7B7, 0x3BAA3B, 0xF5C400, 0x6B4423, 0x5FB8F5, 0xE0587A, 0x3A3F58]
SHEET_SCALE, CELL_W, CELL_H, HAT_CELL_H = 4, 40, 30, 42
HAT_CHECKS = [("IDLE0", "HAT_PARTY"), ("SIT", "HAT_WIZARD"), ("IDLE0", "HAT_GLASSES"), ("BEG", "HAT_BANDANA")]


def hat_xy(name, h, parts):
    """Game::drawHat's placement at scale 1, facing right."""
    if name == "HAT_GLASSES":
        return parts["eyeX"] - 1, parts["eyeY"] - 1
    if name == "HAT_BANDANA":
        return parts["headX"] - h.w // 2 + 3, parts["headY"] + parts["headR"] - 2
    return parts["headX"] - h.w // 2 + 1, parts["headY"] - parts["headR"] - h.h + 2


def sheet_put(px, im, ox, oy):
    S = SHEET_SCALE
    for y in range(im.h):
        for x in range(im.w):
            if im.px[y][x] == T:
                continue
            rgb = PAL[im.px[y][x]]
            for dy in range(S):
                for dx in range(S):
                    px[(ox + x) * S + dx, (oy + y) * S + dy] = ((rgb >> 16) & 255, (rgb >> 8) & 255, rgb & 255)


def preview(frames, icons):
    """Rows: every breed x size x pose, then a hat on each breed (hats at Game::drawHat's anchors), then the icons."""
    try:
        from PIL import Image
    except ImportError:
        print("Pillow missing; skipping preview")
        return
    rows = len(BREEDS) * len(SIZE_ORDER)
    W, hat_top = len(POSES) * CELL_W, rows * CELL_H
    img = Image.new("RGB", (W * SHEET_SCALE, (hat_top + len(BREEDS) * HAT_CELL_H + 40) * SHEET_SCALE), (0x60, 0x80, 0x60))
    px = img.load()
    for (br, sz, pose), (im, parts) in frames.items():
        bi, si = list(BREEDS).index(br), SIZE_ORDER.index(sz)
        put_y = (bi * len(SIZE_ORDER) + si) * CELL_H + CELL_H - im.h - 2
        sheet_put(px, im, POSES.index(pose) * CELL_W + 2, put_y)
        for ci, (hat_pose, hat) in enumerate(HAT_CHECKS):
            if hat_pose == pose:
                ox, oy = (si * len(HAT_CHECKS) + ci) * CELL_W + 2, hat_top + (bi + 1) * HAT_CELL_H - im.h - 2
                sheet_put(px, im, ox, oy)
                hx, hy = hat_xy(hat, icons[hat], parts)
                sheet_put(px, icons[hat], ox + hx, oy + hy)
    x = 1
    for im in icons.values():
        sheet_put(px, im, x, hat_top + len(BREEDS) * HAT_CELL_H + 4)
        x += im.w + 3
        if x > W - 20:
            break
    os.makedirs("build/art", exist_ok=True)
    img.save("build/art/sheet.png")
    print("wrote build/art/sheet.png")


if __name__ == "__main__":
    os.chdir(os.path.dirname(os.path.abspath(__file__)) + "/../../..")  # the Porthole app root, wherever this runs from
    emit(check="--check" in sys.argv)
