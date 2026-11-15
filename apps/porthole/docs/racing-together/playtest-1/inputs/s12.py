# ruff: noqa: F403, F405, E402
from duo import *
import sys

mode = sys.argv[1]
a, b = start()
to_lobby(a, b)
a.tap(106, 124)
both(a, b, 600)
if mode == "wobble":
    for amp in (25, 12, 6):
        wobble(a, b, b, 4000, amp, seed=amp)
        b.snap(f"17_wobble{amp}")
elif mode == "waiting":
    both(a, b, 3000)
    a.tap(106, 124)
    both(a, b, 800)
    a.snap("18_a_waiting")
    b.snap("18_b_not_ready")
    print("A", a.do("ui"))
    b.tap(54, 124)
    both(a, b, 1500)
    a.snap("18_b_back_while_a_waits")
    b.snap("18_b_back_while_a_waits")
    both(a, b, 5000)
    a.snap("18_b_back_5s")
    b.snap("18_b_back_5s")
elif mode == "aback":
    both(a, b, 3000)
    a.tap(106, 124)
    both(a, b, 800)
    a.tap(54, 124)
    both(a, b, 1500)
    a.snap("19_a_back_while_waiting")
    b.snap("19_a_back_while_waiting")
a.close()
b.close()
