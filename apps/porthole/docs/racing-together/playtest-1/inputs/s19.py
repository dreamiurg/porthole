# mypy: ignore-errors
# ruff: noqa: F403, F405, E402
from duo import *
import sys

mode = sys.argv[1]
a, b = start()
to_race(a, b)
both(a, b, 9000)
r = random.Random(7)
t = 0
afin = None
bfin = None
a.tap(144, 80)
a.tap(18, 80)  # A fires boost and rocket early
while t < 240000:
    for _ in range(15):
        tiltdeg(b, r.uniform(-22, 22))
        tiltdeg(a, r.uniform(-4, 4))
        a.cmd("tick 66")
        b.cmd("tick 66")
        t += 66
    sa, sb = a.scr(), b.scr()
    if sa != "race_play" and afin is None:
        afin = t
        print("A off play at", t / 1000, sa)
        a.snap("29_a_finish")
        b.snap("29_b_when_a_finishes")
    if afin and t - afin in range(4950, 5050) or (afin and abs(t - afin - 6000) < 500 and not hasattr(a, "s6")):
        a.s6 = 1
        a.snap("29_a_waiting_6s")
        b.snap("29_b_racing_6s")
    if sb != "race_play" and bfin is None:
        bfin = t
        print("B off play at", t / 1000, sb)
        a.snap("29_a_when_b_finishes")
        b.snap("29_b_finish")
        break
    if mode == "drop" and afin is None and t > 40000:
        print("kill B at", t / 1000)
        b.kill()
        for _i in range(20):
            a.cmd("tick 66")
        a.snap("30_drop_0s")
        for _i in range(60):
            a.cmd("tick 66")
        a.snap("30_drop_5s")
        while a.scr() == "race_play":
            for _i in range(30):
                tiltdeg(a, r.uniform(-4, 4))
                a.cmd("tick 66")
        a.snap("30_drop_finish")
        for _i in range(60):
            a.cmd("tick 66")
        a.snap("30_drop_finish_4s")
        print(a.do("ui"))
        sys.exit()
both(a, b, 3000)
a.snap("29_results_3s")
b.snap("29_results_3s")
print("A", a.do("ui"))
print("B", b.do("ui"))
a.close()
b.close()
