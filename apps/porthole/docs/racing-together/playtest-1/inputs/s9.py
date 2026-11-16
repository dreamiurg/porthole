# mypy: ignore-errors
# ruff: noqa: F403, F405, E402
from duo import *
import sys

mode = sys.argv[1]
a, b = start()
to_lobby(a, b)
if mode == "guest":
    for x, y in [(35, 76), (125, 76), (80, 76), (106, 124), (110, 128)]:
        b.tap(x, y)
        both(a, b, 500)
    a.snap("13_guest_taps")
    b.snap("13_guest_taps")
    a.tap(125, 76)
    both(a, b, 600)
    a.snap("13_host_next")
    b.snap("13_host_next")
    a.tap(128, 80)
    both(a, b, 600)
    a.tap(122, 70)
    both(a, b, 600)  # double-ish
    a.snap("13_host_next3")
    b.snap("13_host_next3")
elif mode == "guestback":
    b.tap(54, 124)
    both(a, b, 1500)
    a.snap("14_guest_back")
    b.snap("14_guest_back")
    both(a, b, 5000)
    a.snap("14_guest_back_5s")
    b.snap("14_guest_back_5s")
elif mode == "hostback":
    a.tap(54, 124)
    both(a, b, 1500)
    a.snap("15_host_back")
    b.snap("15_host_back")
    both(a, b, 5000)
    a.snap("15_host_back_5s")
    b.snap("15_host_back_5s")
a.close()
b.close()
