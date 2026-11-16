# mypy: ignore-errors
# ruff: noqa: F403, F405, E402
from duo2 import *

a, b = start()
a.snap("20_cards")
a.tap(78, 26)
both(a, b, 1500)
a.snap("21_looking")
b.snap("21_invite")
b.tap(86, 100)
both(a, b, 1200)
a.snap("22_lobby")
b.snap("22_lobby")
a.tap(124, 72)
both(a, b, 700)
a.tap(108, 126)
both(a, b, 1000)
a.snap("23_grip")
b.snap("23_grip")
both(a, b, 2500)
a.tap(104, 120)
both(a, b, 600)
a.snap("24_waiting")
b.tap(110, 128)
both(a, b, 600)
b.snap("25_countdown")
both(a, b, 8000)
r = random.Random(11)
t = 0
af = bf = None
b.tap(144, 80)
while t < 240000:
    for _ in range(15):
        tiltdeg(a, r.uniform(-20, 20))
        tiltdeg(b, r.uniform(-5, 5))
        a.cmd("tick 66")
        b.cmd("tick 66")
        t += 66
    if t > 30000 and t < 31100:
        a.snap("26_race")
        b.snap("26_race")
    if t > 50000 and t < 51100:
        a.do("hold 42 40")
        both(a, b, 500)
        a.snap("27_pause")
        b.snap("27_pause")
        a.tap(80, 48)
        both(a, b, 600)
    if af is None and a.scr() != "race_play":
        af = t
        a.snap("28_a_done")
        b.snap("28_b_still")
    if bf is None and b.scr() != "race_play":
        bf = t
        a.snap("28_b_done")
        b.snap("28_b_done_b")
    if af and bf:
        break
print("finish A", af and af / 1000, "B", bf and bf / 1000)
both(a, b, 2000)
a.snap("29_results")
b.snap("29_results")
b.tap(52, 112)
both(a, b, 1500)
a.snap("30_b_again")
b.snap("30_b_again")
a.tap(52, 112)
both(a, b, 1500)
a.snap("31_lobby_again")
b.snap("31_lobby_again")
a.close()
b.close()
