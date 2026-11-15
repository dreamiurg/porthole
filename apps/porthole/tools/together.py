#!/usr/bin/env python3
"""Rally's race together on two sims at once: two `snap --serve` processes on this computer, each in its own
directory (its own saves), talking over UDP on 127.0.0.1 (host/sim.cpp SimLan), driven in step from here.

  python3 tools/together.py OUTDIR      # after `make build/host/snap`; writes OUTDIR/{sam,zoe}_*.bmp

Sam taps Together, Zoe joins, Sam picks the second track, both set their grip and race; Zoe finishes first.
Exits non-zero when a screen is not the one expected."""

import os
import subprocess
import sys
import tempfile

HERE = os.path.dirname(os.path.abspath(__file__))
SNAP = os.path.join(HERE, "..", "build", "host", "snap")
# Rally's targets (games/racing/render.h), at their centers in logical pixels.
TOGETHER, JOIN, LOBBY_NEXT, GO = (80, 20), (80, 96), (125, 76), (106, 124)


class Sim:
    def __init__(self, name, profiles, out):
        self.name, self.out = name, out
        self.dir = tempfile.mkdtemp(prefix=f"porthole-{name}-")
        os.makedirs(os.path.join(self.dir, "build", "host"))
        self.p = subprocess.Popen([SNAP, "--serve", "--fresh"], cwd=self.dir, stdin=subprocess.PIPE, stdout=subprocess.PIPE)
        for kid in profiles:
            self.do(f"profile {kid} 7")
        self.do("app Rally")

    def cmd(self, line):
        self.p.stdin.write((line + "\n").encode())
        self.p.stdin.flush()
        reply = self.p.stdout.readline().decode().strip()
        while reply != "ok":  # a script command's own output comes first
            if reply.startswith("screen="):
                self.screen = reply[7:]
            reply = self.p.stdout.readline().decode().strip()

    def do(self, line):
        self.cmd("do " + line)

    def snap(self, shot):
        self.do(f"snap together_{shot}")
        os.replace(os.path.join(self.dir, "build", "host", f"together_{shot}.bmp"), os.path.join(self.out, f"{self.name}_{shot}.bmp"))

    def expect(self, screen):
        self.do("screen")
        if self.screen != screen:
            sys.exit(f"{self.name}: on {self.screen}, expected {screen}")

    def close(self):
        self.p.stdin.close()
        self.p.wait()


def both(a, b, ms):
    for _ in range(0, ms, 66):
        a.cmd("tick 66")
        b.cmd("tick 66")


def main():
    out = os.path.abspath(sys.argv[1] if len(sys.argv) > 1 else ".")
    os.makedirs(out, exist_ok=True)
    sam, zoe = Sim("sam", ["Sam"], out), Sim("zoe", ["Kai", "Zoe"], out)
    try:
        both(sam, zoe, 1000)
        sam.snap("tracks")
        sam.do(f"tap {TOGETHER[0]} {TOGETHER[1]}")
        both(sam, zoe, 600)
        sam.expect("race_looking")
        zoe.snap("invite")
        zoe.do(f"tap {JOIN[0]} {JOIN[1]}")
        both(sam, zoe, 600)
        sam.do(f"tap {LOBBY_NEXT[0]} {LOBBY_NEXT[1]}")
        both(sam, zoe, 600)
        sam.expect("race_lobby")
        zoe.expect("race_lobby")
        sam.snap("lobby")
        zoe.snap("lobby")
        sam.do(f"tap {GO[0]} {GO[1]}")
        both(sam, zoe, 1200)
        sam.do(f"tap {GO[0]} {GO[1]}")
        both(sam, zoe, 400)
        sam.snap("waiting")
        zoe.do(f"tap {GO[0]} {GO[1]}")
        both(sam, zoe, 600)
        sam.expect("race_countdown")
        zoe.expect("race_countdown")
        both(sam, zoe, 9000)
        sam.expect("race_play")
        zoe.expect("race_play")
        sam.snap("race")
        zoe.snap("race")
        zoe.do("dbg lastlap")
        both(sam, zoe, 7000)
        zoe.expect("race_results")
        zoe.snap("results_waiting")
        sam.do("dbg lastlap")
        both(sam, zoe, 7000)
        sam.expect("race_results")
        sam.snap("results")
        zoe.snap("results")
    finally:
        sam.close()
        zoe.close()
    print("together: ok,", out)


if __name__ == "__main__":
    main()
