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


def draw_dog(size, pose):
    p = SIZES[size]
    im = Img(p["W"], p["H"])
    bx, by, brx, bry = p["body"]
    hx, hy, hr = p["head"]
    mx, my, mrx, mry = p["muzzle"]
    ex, ey, erx, ery = p["ear"]
    legY, legH, legW = p["legY"], p["legH"], p["legW"]
    tx, ty = p["tail"]
    eyex, eyey = p["eye"]
    nx, ny = p["nose"]
    ground = legY + legH
    parts = {}
    eyes, mouth = "open", "closed"
    hdx = hdy = 0
    legs = []  # (x, ytop, h)
    tail = None  # (x0,y0,x1,y1)
    body = (bx, by, brx, bry)
    belly = (bx + 1, by + bry * 0.45, brx * 0.6, bry * 0.55)
    haunch = None
    extra = []  # callables drawn after body, before outline

    if pose in ("IDLE0", "IDLE1", "WALK0", "WALK1", "EAT0", "EAT1", "JUMP", "SAD", "BARK"):
        legs = [(p["backX"][0], legY - 1, legH + 1), (p["backX"][1], legY, legH), (p["frontX"][0], legY - 1, legH + 1), (p["frontX"][1], legY, legH)]
        tail = (tx, ty, tx - 4, ty - 5) if pose != "IDLE1" else (tx, ty, tx - 5, ty - 2)
        if pose == "WALK0":
            legs = [(p["backX"][0] - 1, legY - 1, legH + 1), (p["backX"][1] + 1, legY, legH), (p["frontX"][0] - 1, legY - 1, legH + 1), (p["frontX"][1] + 1, legY, legH)]
        if pose == "WALK1":
            legs = [(p["backX"][0] + 1, legY - 1, legH + 1), (p["backX"][1] - 1, legY, legH), (p["frontX"][0] + 1, legY - 1, legH + 1), (p["frontX"][1] - 1, legY, legH)]
        if pose in ("EAT0", "EAT1"):
            hdx, hdy = 1, int(hr * 0.7)
            mouth = "open" if pose == "EAT1" else "closed"
            tail = (tx, ty, tx - 5, ty - 3)
        if pose == "JUMP":
            hdy = -3
            body = (bx, by - 3, brx, bry)
            belly = (bx + 1, by - 3 + bry * 0.45, brx * 0.6, bry * 0.55)
            legs = [
                (p["backX"][0] - 1, legY - 3, legH - 1),
                (p["backX"][1] + 1, legY - 2, legH - 2),
                (p["frontX"][0] - 1, legY - 3, legH - 1),
                (p["frontX"][1] + 1, legY - 2, legH - 2),
            ]
            tail = (tx, ty - 3, tx - 4, ty - 8)
            eyes = "happy"
            mouth = "open"
        if pose == "SAD":
            hdy = 2
            tail = (tx, ty, tx - 2, ty + 4)
            eyes = "sad"
            mouth = "sad"
        if pose == "BARK":
            hdy = -1
            mouth = "open"
            tail = (tx, ty, tx - 4, ty - 6)
    elif pose in ("SIT", "SIT_PAW", "LISTEN"):
        haunch = (bx - 1, legY + 1, bry + 1.2, bry + 1.2)
        body = (bx + 2, by - 1.5, brx - 2.5, bry + 1.5)
        belly = (bx + 3, by + 0.5, brx * 0.45, bry * 0.8)
        legs = [(p["frontX"][0], int(by - 1), ground - int(by - 1)), (p["frontX"][1], int(by - 1), ground - int(by - 1))]
        hdy = -1
        tail = (int(bx - bry - 1), ground - 1, int(bx - bry - 5), ground - 3)
        if pose == "SIT_PAW":
            legs = [(p["frontX"][0], int(by - 1), ground - int(by - 1))]
            extra.append(lambda im: im.line(p["frontX"][1], int(by), p["frontX"][1] + 4, int(by) - 4, TAN, legW))
        if pose == "LISTEN":
            hdx, hdy = 1, -2
            eyes = "big"
    elif pose in ("SLEEP0", "SLEEP1"):
        lift = 1 if pose == "SLEEP1" else 0
        body = (bx + 1, ground - bry + 0.5 - lift, brx + 1, bry - 0.5 + lift * 0.5)
        belly = None
        hdx, hdy = -2, ground - hr - hy - 1
        eyes = "closed"
        legs = []
        extra.append(lambda im: im.rect(p["frontX"][0] + 1, ground - 2, 5, 2, TAN))
        tail = (tx, ground - 2, tx - 4, ground - 1)
    elif pose in ("BACK", "DEAD"):
        body = (bx + 1, ground - bry - 1, brx, bry)
        belly = (bx + 1, ground - bry - 1, brx * 0.7, bry * 0.6)
        legs = [
            (p["backX"][0], int(ground - 2 * bry - 3), legH - 1),
            (p["backX"][1] + 1, int(ground - 2 * bry - 2), legH - 2),
            (p["frontX"][0] - 1, int(ground - 2 * bry - 3), legH - 1),
            (p["frontX"][1], int(ground - 2 * bry - 2), legH - 2),
        ]
        hdx, hdy = 0, ground - hr - hy - 2
        eyes = "x" if pose == "DEAD" else "open"
        mouth = "tongue" if pose == "DEAD" else "closed"
        tail = (tx, ground - 2, tx - 4, ground - 1)
    elif pose == "BEG":
        haunch = (bx - 1, legY + 1, bry + 1.2, bry + 1.2)
        body = (bx + 2, by - 3, bry + 0.5, brx - 2.5)
        belly = (bx + 3, by - 1.5, bry * 0.5, brx * 0.45)
        hdx, hdy = -3, -4
        legs = []
        extra.append(lambda im: im.rect(int(bx + bry + 1), int(by - 3), 3 if size != "big" else 4, legW, TAN))
        extra.append(lambda im: im.rect(int(bx + bry + 1), int(by), 3 if size != "big" else 4, legW, TAN))
        tail = (int(bx - bry - 1), ground - 1, int(bx - bry - 5), ground - 3)

    # ---- draw order: tail, haunch, body, belly, legs, extra, head, muzzle, ear; then outline; then features
    if tail:
        im.line(*tail, TAN, 2)
    if haunch:
        im.ellipse(*haunch, TAN)
    im.ellipse(*body, TAN)
    if belly:
        im.ellipse(*belly, CREAM)
    for lx, ly, lh in legs:
        im.rect(lx, ly, legW, lh, TAN)
    for fn in extra:
        fn(im)
    HX, HY = hx + hdx, hy + hdy
    im.ellipse(HX, HY, hr, hr, TAN)
    im.ellipse(mx + hdx, my + hdy, mrx, mry, CREAM)
    im.ellipse(ex + hdx, ey + hdy, erx, ery, BROWN)
    im.outline(DKBROWN)
    # paws (cream toes) at leg bottoms
    for lx, ly, lh in legs:
        if ly + lh >= ground:
            im.rect(lx, ground - 1, legW, 1, CREAM) if legW > 1 else None
    # eyes
    EX, EY = eyex + hdx, eyey + hdy
    if eyes == "open":
        im.rect(EX, EY, 2, 2, WHITE)
        im.set(EX + 1, EY + 1, K)
    elif eyes == "big":
        im.rect(EX - 1, EY, 3, 3, WHITE)
        im.set(EX + 1, EY + 1, K)
        im.set(EX + 1, EY + 2, K)
    elif eyes == "closed":
        im.rect(EX, EY + 1, 3, 1, DKBROWN)
    elif eyes == "happy":
        im.set(EX, EY + 1, DKBROWN)
        im.set(EX + 1, EY, DKBROWN)
        im.set(EX + 2, EY + 1, DKBROWN)
    elif eyes == "sad":
        im.rect(EX, EY, 2, 2, WHITE)
        im.set(EX + 1, EY + 1, K)
        im.set(EX - 1, EY - 1, DKBROWN)
        im.set(EX, EY - 1, DKBROWN)
    elif eyes == "x":
        im.set(EX, EY, DKBROWN)
        im.set(EX + 2, EY, DKBROWN)
        im.set(EX + 1, EY + 1, DKBROWN)
        im.set(EX, EY + 2, DKBROWN)
        im.set(EX + 2, EY + 2, DKBROWN)
    # nose + mouth
    NX, NY = nx + hdx, ny + hdy
    im.rect(NX - 1, NY, 2, 2, K)
    if mouth == "open":
        im.rect(NX - 3, NY + 3, 3, 2, PINK)
        im.rect(NX - 4, NY + 2, 1, 1, DKBROWN)
        im.rect(NX - 3, NY + 2, 3, 1, DKBROWN)
    elif mouth == "tongue":
        im.rect(NX - 2, NY + 3, 2, 3, PINK)
        im.set(NX - 2, NY + 2, DKBROWN)
    elif mouth == "sad":
        im.set(NX - 3, NY + 2, DKBROWN)
        im.set(NX - 2, NY + 3, DKBROWN)
    else:
        im.set(NX - 2, NY + 2, DKBROWN)
    # collar (not on puppy)
    if p["collar"] and pose not in ("BACK", "DEAD", "SLEEP0", "SLEEP1"):
        cx, cy = p["collar"]
        cx += hdx
        cy += hdy
        if pose in ("SIT", "SIT_PAW", "LISTEN"):
            cx += 1
        if pose == "BEG":
            cx -= 2
            cy -= 3
        im.rect(cx - 1, cy, 4, 1, RED)
        im.set(cx, cy + 1, GOLD)
    # anchors for hats/zones: head top-center, eye, body center, tail
    parts = dict(
        headX=int(round(HX)),
        headY=int(round(HY)),
        headR=int(hr),
        bodyX=int(round(body[0])),
        bodyY=int(round(body[1])),
        bodyRX=int(round(body[2])),
        bodyRY=int(round(body[3])),
        tailX=int(tail[2]) if tail else 0,
        tailY=int(tail[3]) if tail else 0,
        eyeX=int(EX),
        eyeY=int(EY),
    )
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
def emit(check=False):
    out = []
    w = out.append
    w("// AUTO-GENERATED by tools/art.py. Edit the generator, not this file.")
    w("#pragma once")
    w('#include "gfx.h"')
    w("enum Pose { " + ", ".join("P_" + p for p in POSES) + ", NUM_POSES };")
    w("enum DogSize { SZ_PUP = 0, SZ_DOG = 1, SZ_BIG = 2 };")
    w("struct DogParts { int8_t headX, headY, headR, bodyX, bodyY, bodyRX, bodyRY, tailX, tailY, eyeX, eyeY; };")
    frames = {}
    for sz in ("pup", "dog", "big"):
        for pose in POSES:
            im, parts = draw_dog(sz, pose)
            frames[(sz, pose)] = (im, parts)
            name = f"dog_{sz}_{pose.lower()}_px"
            data = ",".join(str(c) for row in im.px for c in row)
            w(f"static const uint8_t {name}[] = {{{data}}};")
    w("static const gfx::Sprite DOG_FRAMES[3][NUM_POSES] = {")
    for sz in ("pup", "dog", "big"):
        w("  {" + ", ".join(f"{{{frames[(sz, p)][0].w},{frames[(sz, p)][0].h},dog_{sz}_{p.lower()}_px}}" for p in POSES) + "},")
    w("};")
    w("static const DogParts DOG_PARTS[3][NUM_POSES] = {")
    for sz in ("pup", "dog", "big"):
        row = []
        for p in POSES:
            pr = frames[(sz, p)][1]
            row.append("{" + ",".join(str(pr[k]) for k in ("headX", "headY", "headR", "bodyX", "bodyY", "bodyRX", "bodyRY", "tailX", "tailY", "eyeX", "eyeY")) + "}")
        w("  {" + ", ".join(row) + "},")
    w("};")
    icons = {}
    for name, art in ICONS.items():
        im = ascii_img(art)
        icons[name] = im
        data = ",".join(str(c) for row in im.px for c in row)
        w(f"static const uint8_t spr_{name.lower()}_px[] = {{{data}}};")
        w(f"static const gfx::Sprite SPR_{name} = {{{im.w},{im.h},spr_{name.lower()}_px}};")
    w("static const gfx::Sprite SPR_HATS[8] = {{0,0,nullptr}, " + ", ".join("SPR_" + h for h in HATS) + "};")
    text = "\n".join(out) + "\n"
    if check:
        try:
            current = Path(SPRITES).read_text(encoding="utf-8")
        except FileNotFoundError:
            current = ""
        if current != text:
            print("sprites.h is out of date: run `make art`")
            sys.exit(1)
        print("sprites.h up to date")
        return
    with open(SPRITES, "w") as f:
        f.write(text)
    print(f"wrote {SPRITES} ({len(frames)} dog frames, {len(icons)} icons)")
    preview(frames, icons)


def preview(frames, icons):
    try:
        from PIL import Image
    except ImportError:
        print("Pillow missing; skipping preview")
        return
    PAL = [
        0x000000,
        0x1D2B53,
        0x7E2553,
        0x008751,
        0xAB5236,
        0x5F574F,
        0xC2C3C7,
        0xFFF1E8,
        0xFF004D,
        0xFFA300,
        0xFFEC27,
        0x00E436,
        0x29ADFF,
        0x83769C,
        0xFF77A8,
        0xFFCCAA,
        0xE6B27A,
        0x5C3A1E,
        0xFFF7D6,
        0xA8E0FF,
        0x16213E,
        0xC47A3A,
        0x8A4B22,
        0xF2D9B5,
        0xD9A066,
        0x6EE7B7,
        0x3BAA3B,
        0xF5C400,
        0x6B4423,
        0x5FB8F5,
        0xE0587A,
        0x3A3F58,
    ]
    S = 4
    cols = len(POSES)
    cellw, cellh = 40, 30
    W = cols * cellw
    H = 3 * cellh + 40
    img = Image.new("RGB", (W * S, H * S), (0x60, 0x80, 0x60))
    px = img.load()

    def put(im, ox, oy):
        for y in range(im.h):
            for x in range(im.w):
                c = im.px[y][x]
                if c == T:
                    continue
                rgb = PAL[c]
                col = ((rgb >> 16) & 255, (rgb >> 8) & 255, rgb & 255)
                for dy in range(S):
                    for dx in range(S):
                        px[(ox + x) * S + dx, (oy + y) * S + dy] = col

    for si, sz in enumerate(("pup", "dog", "big")):
        for pi, pose in enumerate(POSES):
            im = frames[(sz, pose)][0]
            put(im, pi * cellw + 2, si * cellh + (cellh - im.h - 2))
    x = 1
    for im in icons.values():
        put(im, x, 3 * cellh + 4)
        x += im.w + 3
        if x > W - 20:
            break
    os.makedirs("build/art", exist_ok=True)
    img.save("build/art/sheet.png")
    print("wrote build/art/sheet.png")


if __name__ == "__main__":
    os.chdir(os.path.dirname(os.path.abspath(__file__)) + "/../../..")  # the Porthole app root, wherever this runs from
    emit(check="--check" in sys.argv)
