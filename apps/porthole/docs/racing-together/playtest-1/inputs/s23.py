# mypy: ignore-errors
# ruff: noqa: F403, F405, E402
from duo import *

for x, y in [(70, 33), (90, 34), (80, 9)]:
    a = Sim("A", ["Mia"])
    both(a, None, 1000)
    to_rally(a)
    a.tap(x, y)
    both(a, None, 900)
    print((x, y), a.scr())
    if (x, y) == (70, 33):
        a.snap("38_tap_just_below_together")
    a.close()
