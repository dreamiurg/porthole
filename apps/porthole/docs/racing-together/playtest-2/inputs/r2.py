# mypy: ignore-errors
# ruff: noqa: F403, F405, E402
from duo2 import *
import sys

delay = int(sys.argv[1])
join = sys.argv[2] == "join"
a, b = start()
a.tap(80, 21)
both(a, b, 1500)
b.tap(84, 124)
both(a, b, delay)
b.tap(80, 21)
tl = []
for i in range(60):
    both(a, b, 99)
    tl.append((a.scr(), b.scr()))
    if join and i == 8:
        a.tap(80, 96)
seq = []
for s in tl:
    if not seq or seq[-1][0] != s:
        seq.append([s, 1])
    else:
        seq[-1][1] += 1
print(delay, join, [(s, n * 0.1) for s, n in seq])
tag = f"07_d{delay}_{'join' if join else 'wait'}"
a.snap(tag)
b.snap(tag)
a.close()
b.close()
