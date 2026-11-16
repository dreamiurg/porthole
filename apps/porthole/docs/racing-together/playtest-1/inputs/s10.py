# mypy: ignore-errors
# ruff: noqa: F403, F405, E402
from duo import *

a, b = start()
to_lobby(a, b)
b.tap(54, 124)
both(a, b, 1500)
print([l for l in a.do("ui") if "went" in l or l.startswith("text") and int(l.split()[2]) > 125])
both(a, b, 8000)
a.snap("14_guest_back_9s")
a.close()
b.close()
