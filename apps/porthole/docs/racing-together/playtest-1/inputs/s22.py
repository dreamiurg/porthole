# mypy: ignore-errors
# ruff: noqa: F403, F405, E402
from duo import *
import sys

mode = sys.argv[1]
if mode == "solo":
    a, b = start()
    b.tap(53, 52)
    both(a, b, 3000)
    print("B after card", b.scr())
    b.tap(106, 124)
    both(a, b, 9000)
    print("B", b.scr())
    a.tap(80, 21)
    both(a, b, 3000)
    a.snap("36_invite_while_b_solo")
    b.snap("36_invite_while_b_solo")
    b.do("hold 42 40")
    both(a, b, 800)
    b.snap("36_b_pauses_solo")
else:
    a = Sim("A", ["Mia"])
    b = Sim("B", ["Kai", "Leo"])
    both(a, b, 1000)
    to_rally(a, b)
    a.tap(80, 21)
    both(a, b, 3000)
    b.snap("37_b_on_launcher")
    to_rally(b, a)
    both(a, b, 1000)
    a.snap("37_b_opens_rally_later")
    b.snap("37_b_opens_rally_later")
a.close()
b.close()
