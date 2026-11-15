# ruff: noqa: F403, F405, E402
from duo import *

a, b = start()
to_race(a, b)
both(a, b, 9000)
r = random.Random(5)
for t in range(1, 61):
    for _ in range(15):
        tiltdeg(b, r.uniform(-15, 15))
        tiltdeg(a, r.uniform(-4, 4))
        a.cmd("tick 66")
        b.cmd("tick 66")
    if t % 6 == 0:
        a.snap(f"21_race_{t:02d}")
        b.snap(f"21_race_{t:02d}")
    if a.scr() != "race_play" or b.scr() != "race_play":
        print(t, a.screen, b.screen)
a.close()
b.close()
