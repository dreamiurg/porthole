# ruff: noqa: F403, F405, E402
from duo import *

a, b = start()
a.tap(80, 21)
both(a, b, 1500)
for x, y in [(32, 100), (128, 100), (36, 140), (124, 128), (110, 60)]:
    b.tap(x, y)
    both(a, b, 900)
    print((x, y), b.scr(), a.scr())
    if b.screen != "race_tracks":
        b.snap(f"35_tap_{x}_{y}")
        break
b.snap("35_after_taps")
a.close()
b.close()
