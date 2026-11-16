# mypy: ignore-errors
# ruff: noqa: F403, F405, E402
from duo import *

a = Sim("A", ["Mia"])
b = Sim("B", ["Leo"])
both(a, b, 1500)
a.tap(124, 104)
both(a, b, 800)
a.snap("02_more")
print(a.do("ui"))
a.close()
b.close()
