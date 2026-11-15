# ruff: noqa: F403, F405, E402
from duo import *
import sys

mode = sys.argv[1]
a, b = start()
to_race(a, b)
both(a, b, 12000)
a.do("dbg lastlap")


def until(s, scr, ms=40000):
    t = 0
    while s.scr() != scr and t < ms:
        both(a, b, 660)
        t += 660


both(a, b, 1000)
until(a, "race_results")
both(a, b, 800)
if mode == "again_early":
    a.tap(52, 112)
    both(a, b, 1500)
    a.snap("31_a_again_while_b_races")
    b.snap("31_a_again_while_b_races")
    b.do("dbg lastlap")
    until(b, "race_results")
    both(a, b, 1500)
    a.snap("31_after_b_finish")
    b.snap("31_after_b_finish")
    b.tap(52, 112)
    both(a, b, 1500)
    a.snap("31_b_again")
    b.snap("31_b_again")
elif mode == "again_both":
    b.do("dbg lastlap")
    until(b, "race_results")
    both(a, b, 1000)
    b.tap(52, 112)
    both(a, b, 1500)
    a.snap("32_b_again_first")
    b.snap("32_b_again_first")
    both(a, b, 5000)
    a.snap("32_b_again_5s")
    b.snap("32_b_again_5s")
    a.tap(52, 112)
    both(a, b, 1500)
    a.snap("32_a_again")
    b.snap("32_a_again")
elif mode == "home":
    b.do("dbg lastlap")
    until(b, "race_results")
    both(a, b, 1000)
    a.tap(52, 112)
    both(a, b, 1000)
    b.tap(80, 140)
    both(a, b, 2000)
    a.snap("33_b_home_after_a_again")
    b.snap("33_b_home_after_a_again")
    both(a, b, 5000)
    a.snap("33_5s")
    b.snap("33_5s")
elif mode == "tracks":
    b.do("dbg lastlap")
    until(b, "race_results")
    both(a, b, 1000)
    b.tap(108, 112)
    both(a, b, 1500)
    a.snap("34_b_tracks")
    b.snap("34_b_tracks")
    a.tap(52, 112)
    both(a, b, 1500)
    a.snap("34_a_again_after_b_tracks")
    b.snap("34_a_again_after_b_tracks")
a.close()
b.close()
