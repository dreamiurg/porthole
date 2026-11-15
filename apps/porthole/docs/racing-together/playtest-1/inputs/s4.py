# ruff: noqa: F403, F405, E402
from duo import *

a, b = start()
a.snap("04_rally_open")
b.snap("04_rally_open")
print(a.do("ui"))
a.close()
b.close()
