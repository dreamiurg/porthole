# ruff: noqa: F403, F405, E402
from duo import *
import sys

mode = sys.argv[1]
a, b = start()
a.tap(80, 21)
both(a, b, 1500)
if mode == "notnow":
    b.tap(84, 124)
    both(a, b, 600)
    a.snap("06_after_notnow")
    b.snap("06_after_notnow")
    both(a, b, 5000)
    a.snap("06_after_notnow_5s")
    b.snap("06_after_notnow_5s")
    # B changes mind: taps Together itself
    b.tap(80, 21)
    both(a, b, 1500)
    a.snap("06_b_together_after_notnow")
    b.snap("06_b_together_after_notnow")
elif mode == "together_on_invite":
    b.tap(80, 21)
    both(a, b, 1500)
    a.snap("07_b_taps_together_on_invite")
    b.snap("07_b_taps_together_on_invite")
elif mode == "behind":
    b.tap(30, 60)
    both(a, b, 700)  # left edge of panel, over Desert card
    b.snap("08_b_tap_panel_edge")
    b.tap(40, 140)
    both(a, b, 700)  # below panel where Home peeks
    b.snap("08_b_tap_below_panel")
a.close()
b.close()
