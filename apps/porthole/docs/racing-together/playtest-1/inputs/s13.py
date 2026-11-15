# ruff: noqa: F403, F405, E402
from duo import *

a, b = start()
to_race(a, b)
for t in range(0, 10):
    both(a, b, 1000)
    if t in (0, 2, 4, 6, 9):
        a.snap(f"20_count_{t}")
        b.snap(f"20_count_{t}")
print("A", a.do("ui"))
print(a.do("debug"))
a.close()
b.close()
