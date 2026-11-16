# mypy: ignore-errors
# ruff: noqa: F403, F405, E402
from duo import *

a, b = start()
to_lobby(a, b)
a.tap(106, 124)
both(a, b, 300)
a.snap("16_grip_0")
b.snap("16_grip_0")
print("A", a.do("ui"))
print("B", b.do("ui"))
both(a, b, 2000)
a.snap("16_grip_2s")
b.snap("16_grip_2s")
both(a, b, 6000)
a.snap("16_grip_8s")
b.snap("16_grip_8s")
print("A", a.do("ui"))
a.close()
b.close()
