# mypy: ignore-errors
# ruff: noqa: F403, F405, E402
from duo import *
import sys

mode = sys.argv[1]
a, b = start()
if mode == "simul":
    a.tap(80, 21)
    b.tap(80, 21)
    both(a, b, 2000)
    a.snap("09_both_together_2s")
    b.snap("09_both_together_2s")
    both(a, b, 30000)
    a.snap("09_both_together_32s")
    b.snap("09_both_together_32s")
elif mode == "notnow_long":
    a.tap(80, 21)
    both(a, b, 1500)
    b.tap(84, 124)
    both(a, b, 30000)
    a.snap("10_notnow_30s")
    b.snap("10_notnow_30s")
elif mode == "notnow_then_both":
    a.tap(80, 21)
    both(a, b, 1500)
    b.tap(84, 124)
    both(a, b, 1000)
    b.tap(80, 21)
    both(a, b, 30000)
    a.snap("11_deadlock_30s")
    b.snap("11_deadlock_30s")
    # recover: A taps back then together again
    a.tap(80, 132)
    both(a, b, 1000)
    a.snap("11_a_back")
    b.snap("11_a_back")
    a.tap(80, 21)
    both(a, b, 2000)
    a.snap("11_a_again")
    b.snap("11_a_again")
a.close()
b.close()
