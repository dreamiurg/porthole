# ruff: noqa: F403, F405, E402
from duo import *

a = Sim("A", ["Mia"])
b = Sim("B", ["Leo"])
both(a, b, 1500)
a.snap("01_launcher")
print(a.do("ui"))
a.close()
b.close()
