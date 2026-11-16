# mypy: ignore-errors
# ruff: noqa: F403, F405, E402
from duo2 import *

for x, y in [(70, 33), (80, 31), (80, 32)]:
    a = Sim("A", ["Mia"])
    both(a, None, 1000)
    to_rally(a)
    print([l for l in a.do("ui") if l.startswith("region 54")])
    a.tap(x, y)
    both(a, None, 900)
    print((x, y), a.scr())
    a.close()
