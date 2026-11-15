# ruff: noqa: F403, F405, E402
from duo import *

a, b = start()
a.tap(68, 28)
both(a, b, 300)
a.snap("05_looking_early")
b.snap("05_invite_early")
both(a, b, 1500)
a.snap("05_looking")
b.snap("05_invite")
print("A", a.do("ui"))
print("B", b.do("ui"))
both(a, b, 8000)
a.snap("05_looking_10s")
b.snap("05_invite_10s")
a.close()
b.close()
