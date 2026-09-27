#!/usr/bin/env python3
"""Burn timed captions into rendered frames and encode an MP4 (25 fps).

Usage: tools/promo/caption.py FRAMES_DIR CAPTIONS.txt OUT.mp4
CAPTIONS.txt: one "first<TAB>last<TAB>text" line per caption, frame numbers as Blender wrote them (from 1),
as scene.py --shot assembly writes it next to the frames.
"""

import glob
import os
import subprocess
import sys
import tempfile

from PIL import Image, ImageDraw, ImageFont

FADE = 6  # frames
FONT = "/System/Library/Fonts/Supplemental/Arial Rounded Bold.ttf"


def caption_at(caps, n):
    """The caption showing on frame n and its opacity (0-1), fading in and out over FADE frames."""
    for first, last, text in caps:
        if first <= n <= last:
            return text, min(1.0, (n - first + 1) / FADE, (last - n + 1) / FADE)
    return "", 0.0


def burn(img, text, alpha):
    w, h = img.size
    font = ImageFont.truetype(FONT, round(h * 0.036))
    layer = Image.new("RGBA", img.size)
    d = ImageDraw.Draw(layer)
    x0, y0, x1, y1 = d.textbbox((w / 2, h * 0.91), text, font=font, anchor="mm")
    pad = h * 0.018
    d.rounded_rectangle((x0 - 2 * pad, y0 - pad, x1 + 2 * pad, y1 + pad), radius=pad * 1.6, fill=(18, 14, 12, round(170 * alpha)))
    d.text((w / 2, h * 0.91), text, font=font, fill=(250, 244, 234, round(255 * alpha)), anchor="mm")
    return Image.alpha_composite(img.convert("RGBA"), layer).convert("RGB")


def main():
    frames_dir, cap_file, out = sys.argv[1:4]
    caps = [(int(a), int(b), t.rstrip("\n")) for a, b, t in (ln.split("\t", 2) for ln in open(cap_file) if ln.strip())]
    frames = sorted(glob.glob(os.path.join(frames_dir, "f*.png")))
    with tempfile.TemporaryDirectory() as tmp:
        for i, path in enumerate(frames):
            n = int(os.path.basename(path)[1:-4])
            text, alpha = caption_at(caps, n)
            img = Image.open(path)
            (burn(img, text, alpha) if alpha > 0 else img.convert("RGB")).save(os.path.join(tmp, f"{i:05d}.png"))
        subprocess.run(
            [
                "ffmpeg",
                "-y",
                "-loglevel",
                "error",
                "-framerate",
                "25",
                "-i",
                os.path.join(tmp, "%05d.png"),
                "-c:v",
                "libx264",
                "-pix_fmt",
                "yuv420p",
                "-crf",
                "18",
                "-movflags",
                "+faststart",
                out,
            ],
            check=True,
        )
    print(out, len(frames), "frames")


if __name__ == "__main__":
    main()
