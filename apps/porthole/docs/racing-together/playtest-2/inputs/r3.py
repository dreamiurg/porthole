# mypy: ignore-errors
# ruff: noqa: F403, F405, E402
from duo2 import *

a, b = start()
a.tap(80, 21)
both(a, b, 1500)
b.tap(84, 124)
both(a, b, 1000)
b.tap(80, 21)
for t in (5, 10, 20, 30, 40):
    both(a, b, 500 if t == 5 else 1000)
    if t in (10, 20, 30, 40):
        a.snap(f"08_t{t}")
        b.snap(f"08_t{t}")
a.close()
b.close()
