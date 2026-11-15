# ruff: noqa: F403, F405, E402
from duo import *
import re

a, b = start()
to_race(a, b)
both(a, b, 7000)


def pl(s):
    d = " ".join(s.do("debug"))
    return re.search(r"place=(\d)", d).group(1), re.search(r" dist=(\d+)", d).group(1)


r = random.Random(5)
same = 0
n = 0
for t in range(120):
    for _ in range(4):
        tiltdeg(b, r.uniform(-15, 15))
        tiltdeg(a, r.uniform(-4, 4))
        a.cmd("tick 66")
        b.cmd("tick 66")
    pa, pb = pl(a), pl(b)
    n += 1
    if pa == pb:
        same += 1
        print(f"t={t * 0.264:.1f}s same place A={pa} B={pb}")
    if pa == pb and same == 3:
        a.snap("22_same_place")
        b.snap("22_same_place")
print("same", same, "of", n)
a.close()
b.close()
