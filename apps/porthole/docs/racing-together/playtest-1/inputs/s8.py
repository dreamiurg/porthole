# mypy: ignore-errors
# ruff: noqa: F403, F405, E402
from duo import *

a, b = start()
to_lobby(a, b)
a.snap("12_lobby")
b.snap("12_lobby")
print("A", a.do("ui"))
print("B", b.do("ui"))
a.close()
b.close()
