# mypy: ignore-errors
# ruff: noqa: F403, F405, E402
from duo2 import *
import sys

mode = sys.argv[1]
a, b = start()
if mode == "lobby_guest":
    to_lobby(a, b)
    b.tap(54, 124)
    both(a, b, 1500)
    a.snap("09_lobby_guest_back")
    b.snap("09_lobby_guest_back")
elif mode == "lobby_host":
    to_lobby(a, b)
    a.tap(54, 124)
    both(a, b, 1500)
    a.snap("10_lobby_host_back")
    b.snap("10_lobby_host_back")
elif mode == "grip":
    to_lobby(a, b)
    a.tap(106, 124)
    both(a, b, 3000)
    a.tap(106, 124)
    both(a, b, 600)
    b.tap(54, 124)
    both(a, b, 1500)
    a.snap("11_grip_guest_back")
    b.snap("11_grip_guest_back")
elif mode == "midrace":
    to_race(a, b)
    both(a, b, 12000)
    a.do("hold 42 40")
    both(a, b, 600)
    a.tap(52, 108)
    both(a, b, 700)
    a.tap(54, 96)
    both(a, b, 1500)
    a.snap("12_midrace_host_quits")
    b.snap("12_midrace_host_quits")
elif mode == "midrace_guest":
    to_race(a, b)
    both(a, b, 12000)
    b.do("hold 42 40")
    both(a, b, 600)
    b.tap(108, 108)
    both(a, b, 700)
    b.tap(54, 96)
    both(a, b, 1500)
    a.snap("13_midrace_guest_home")
    b.snap("13_midrace_guest_home")
elif mode == "results":
    to_race(a, b)
    both(a, b, 12000)
    a.do("dbg lastlap")
    b.do("dbg lastlap")
    for _ in range(40):
        both(a, b, 660)
        if a.scr() == "race_results" and b.scr() == "race_results":
            break
    both(a, b, 1000)
    b.tap(80, 140)
    both(a, b, 1500)
    a.snap("14_results_guest_home")
    b.snap("14_results_guest_home")
elif mode == "drop":
    to_race(a, b)
    both(a, b, 15000)
    b.kill()
    for _i in range(90):
        a.cmd("tick 66")
    a.snap("15_drop")
a.close() if mode != "drop" else a.close()
if mode != "drop":
    b.close()
