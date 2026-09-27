#!/usr/bin/env python3
"""Film a sim script frame by frame (25 fps) and render a captioned 1080x1080 MP4 of the round screen in a bezel.

Usage: tools/promo/film.py SHOT.txt [OUT.mp4] [hires]   (after `make -C apps/porthole snap`)
Frames land in apps/porthole/build/host/film/<shot>/ (raw panel BMPs, what tools/promo/scene.py puts on the
screen) and <shot>-comp/ (the flat 2D composite). Without OUT.mp4 only the raw frames are made.
Script = the sim's raw grammar plus: `rec on|off` (capture or not), `caption TEXT...` (from this frame on),
`repeat N` ... `end`. Playtest-only directives (expect, ui-check, snap, sheet) are dropped.
"""

import os
import shutil
import subprocess
import sys

from PIL import Image, ImageDraw, ImageFont

W = 1080
SCREEN = 800  # 480 px panel -> 800 px on the canvas
CX, CY = W // 2, 600  # screen centre: leaves the top band for captions
BG, BEZEL = (246, 236, 220), (22, 22, 26)
FONT = ImageFont.truetype("/System/Library/Fonts/Supplemental/Arial Rounded Bold.ttf", 58)
HIRES = "hires" in sys.argv[3:]
DROP = ("expect", "expect-screen", "ui-check", "snap", "sheet", "ui", "debug", "screen")


def unroll(lines):
    out, i = [], 0
    while i < len(lines):
        w = lines[i].split()
        if w and w[0] == "repeat":
            j = lines.index("end", i)
            out += lines[i + 1 : j] * int(w[1])
            i = j + 1
        else:
            out.append(lines[i])
            i += 1
    return out


def expand(src, name):
    """Sim script with a snap after every 40 ms step, plus per-frame (finger, caption) metadata."""
    lines = unroll([ln.strip() for ln in open(src) if ln.strip() and not ln.startswith("#")])
    out, meta = [], []
    rec, finger, cap = True, (False, 0, 0), ""

    def shot(cmd, f):
        nonlocal finger
        out.append(cmd)
        finger = f
        if rec:
            out.append(f"snap film/{name}/f{len(meta):05d}")
            meta.append((finger, cap))

    for ln in lines:
        w = ln.split()
        c = w[0]
        if c in DROP:
            continue
        if c == "rec":
            rec = w[1] == "on"
        elif c == "caption":
            cap = ln[len("caption") :].strip()
        elif c == "tap":
            x, y = int(w[1]), int(w[2])
            shot(f"down {x} {y}", (True, x, y))
            shot("wait 40", (True, x, y))
            shot("up", (False, x, y))
        elif c == "hold":
            x, y = int(w[1]), int(w[2])
            shot(f"down {x} {y}", (True, x, y))
            for _ in range(24):
                shot("wait 40", (True, x, y))
            shot("up", (False, x, y))
        elif c == "down":
            shot(ln, (True, int(w[1]), int(w[2])))
        elif c == "move":
            shot(ln, (finger[0], int(w[1]), int(w[2])))
        elif c == "up":
            shot(ln, (False, finger[1], finger[2]))
        elif c == "wait":
            for _ in range(max(1, -(-int(w[1]) // 40))):  # the sim steps ceil(ms / 40) frames
                shot("wait 40", finger)
        elif c == "skip":
            shot(ln, finger)
        else:  # newgame, profile, app, reset, dbg: instant
            out.append(ln)
    return out, meta


def compose(bmp, finger, cap, mask):
    canvas = Image.new("RGB", (W, W), BG)
    d = ImageDraw.Draw(canvas)
    r = SCREEN // 2
    d.ellipse((CX - r - 34, CY - r - 34 + 10, CX + r + 34, CY + r + 34 + 10), fill=(215, 203, 185))  # soft shadow
    d.ellipse((CX - r - 34, CY - r - 34, CX + r + 34, CY + r + 34), fill=BEZEL)
    screen = Image.open(bmp).convert("RGB")
    if HIRES:  # Biscuit: native 480 px art, smooth scaling
        screen = screen.resize((SCREEN, SCREEN), Image.LANCZOS)
    else:  # Pets Club: 160 px art tripled; back to 160, then an exact 5x nearest upscale keeps pixels square
        screen = screen.resize((160, 160), Image.NEAREST).resize((SCREEN, SCREEN), Image.NEAREST)
    canvas.paste(screen, (CX - r, CY - r), mask)
    down, x, y = finger
    if down:
        px, py = CX - r + x * 3 * SCREEN / 480, CY - r + y * 3 * SCREEN / 480
        ov = Image.new("RGBA", (W, W))
        od = ImageDraw.Draw(ov)
        od.ellipse((px - 34, py - 34, px + 34, py + 34), fill=(255, 255, 255, 110), outline=(255, 255, 255, 230), width=5)
        canvas = Image.alpha_composite(canvas.convert("RGBA"), ov).convert("RGB")
        d = ImageDraw.Draw(canvas)
    if cap:
        d.text((CX, 95), cap, font=FONT, fill=(40, 34, 30), anchor="mm")
    return canvas


def main():
    src = os.path.abspath(sys.argv[1])
    dest = os.path.abspath(sys.argv[2]) if len(sys.argv) > 2 and sys.argv[2] != "hires" else None
    os.chdir(os.path.join(os.path.dirname(os.path.abspath(__file__)), "..", "..", "apps", "porthole"))
    name = os.path.splitext(os.path.basename(src))[0]
    raw, frames = f"build/host/film/{name}", f"build/host/film/{name}-comp"
    for p in (raw, frames):
        shutil.rmtree(p, ignore_errors=True)
        os.makedirs(p)
    script, meta = expand(src, name)
    open(f"build/host/film/{name}.sim", "w").write("\n".join(script) + "\n")
    subprocess.run(["./build/host/snap", "--script", f"build/host/film/{name}.sim"], check=True, stdout=subprocess.DEVNULL)
    print(f"{raw}: {len(meta)} frames, {len(meta) / 25:.1f} s")
    if not dest:
        return
    mask = Image.new("L", (SCREEN, SCREEN))
    ImageDraw.Draw(mask).ellipse((0, 0, SCREEN - 1, SCREEN - 1), fill=255)
    for i, (finger, cap) in enumerate(meta):
        compose(f"{raw}/f{i:05d}.bmp", finger, cap, mask).save(f"{frames}/f{i:05d}.png")
    subprocess.run(
        [
            "ffmpeg",
            "-y",
            "-loglevel",
            "error",
            "-framerate",
            "25",
            "-i",
            f"{frames}/f%05d.png",
            "-c:v",
            "libx264",
            "-pix_fmt",
            "yuv420p",
            "-crf",
            "18",
            "-movflags",
            "+faststart",
            dest,
        ],
        check=True,
    )
    print(dest)


if __name__ == "__main__":
    main()
