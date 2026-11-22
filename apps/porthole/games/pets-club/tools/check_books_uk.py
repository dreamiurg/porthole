#!/usr/bin/env python3
"""Ukrainian books gate (content_books_uk.h): letters, words per level and the fit in Andika on the round screen.

The reader draws a Ukrainian book in Andika Bold (os/uk_fonts.h) at native size: levels 1-2 in the 48 px font, 3-5 in
the 24 px one (books.h PAGE_FONT). Widths here come from the font's own glyph advances and wrap at spaces only, so they
are never narrower than the device's wrap (which may also break after punctuation); host/test_pets_books.cpp measures
the same strings with os/font.cpp itself. Also: Ukrainian letters only (no ы э ъ ё, no common Russian words), level 1
without я ю є ї and the apostrophe, 3-11 pages, 2-3 answers, unique titles.
"""

import os
import re
import sys
from pathlib import Path

os.chdir(os.path.dirname(os.path.abspath(__file__)) + "/../../..")
FILE = Path("games/pets-club/content_books_uk.h")
# Words per book, by level (books.h); a level-1 or -2 page in the 48 px font, the rest in 24 px.
WORDS = {1: (15, 40), 2: (40, 80), 3: (80, 150), 4: (140, 220), 5: (200, 300)}
BIG = {1, 2}
# Boxes (physical px, lines): the page under a one-line title, the title on the reader and on the cover picker, the
# question and an answer button. game.cpp/books.cpp draw them; host/test_pets_books.cpp holds them to these.
PAGE_W, PAGE_LINES = {48: 366, 24: 366}, {48: 4, 24: 8}
TITLE = (324, 1)
COVER_TITLE = (330, 2)
QUESTION = (348, 3)
ANSWER = (306, 2)
UK = set("абвгґдеєжзиіїйклмнопрстуфхцчшщьюяАБВГҐДЕЄЖЗИІЇЙКЛМНОПРСТУФХЦЧШЩЬЮЯʼ’")
ASCII = {chr(c) for c in range(32, 127)}
NOT_LEVEL1 = set("яюєїЯЮЄЇʼ’'")
RUSSIAN = set("ыэъёЫЭЪЁ")
# Russian words and surzhyk that look Ukrainian enough to slip through (lowercase, whole words).
NOT_UKRAINIAN = {"тоже", "ладно", "сейчас", "вообще", "кот", "конечно", "щас", "пожалуста", "спасибо", "смотри",
                 "всьо", "канєшно", "получилось", "звонить", "кушать", "нету", "єслі", "шо", "чьо", "ніхто-ніхто"}


def font_widths(size):
    src = Path("os/uk_fonts.h").read_text(encoding="utf-8")
    extras = [int(x, 16) for x in re.findall(r"0x([0-9A-F]+)", re.search(r"EXTRAS\[\] = \{([^}]*)\}", src).group(1))]
    body = re.search(r"FONT%d_GLYPHS\[\] = \{(.*?)\};" % size, src, re.S).group(1)
    glyphs = re.findall(r"\{\s*(\d+),\s*(\d+),", body)
    adv = [int(a) for _, a in glyphs]
    cps = [None] + list(range(32, 127)) + extras
    return {chr(cp): (adv[i] + 8) >> 4 for i, cp in enumerate(cps) if cp is not None and i < len(adv)}


W = {24: font_widths(24), 48: font_widths(48)}


def width(t, size):
    return sum(W[size].get(c, W[size]["?"]) for c in t)


def lines(t, size, maxw):
    n, cur, widest = 0, "", 0
    for word in t.split():
        widest = max(widest, width(word, size))
        trial = f"{cur} {word}" if cur else word
        if width(trial, size) <= maxw or not cur:
            cur = trial
        else:
            n, cur = n + 1, word
    return n + (1 if cur else 0), widest


def fits(t, size, box):
    maxw, maxlines = box
    n, widest = lines(t, size, maxw)
    if widest > maxw:
        return f"a word is {widest}px, the box {maxw}px"
    if n > maxlines:
        return f"{n} lines, the box has {maxlines} ({size} px font, {maxw}px)"
    return None


def strings_in(block):
    return [m.encode("utf-8").decode("unicode_escape").encode("latin-1").decode("utf-8")
            for m in re.findall(r'"((?:[^"\\]|\\.)*)"', block)]


def words_of(t):
    return re.findall(r"[^\W\d_]+(?:[ʼ’'][^\W\d_]+)*", t)


def main():
    errors, titles = [], {}
    text = FILE.read_text(encoding="utf-8")
    book_re = (r'\{"((?:[^"\\]|\\.)*)",\s*C_\w+,\s*(\d+),\s*(\d+),\s*\{(.*?)nullptr\},'
               r'\s*"((?:[^"\\]|\\.)*)",\s*\{(.*?)\},\s*(\d+)\}')
    for m in re.finditer(book_re, text, re.S):
        title, icon, level, pages, question, answers, correct = m.groups()
        level = int(level)
        line = text.count("\n", 0, m.start()) + 1
        where = f"{FILE}:{line} [{title}]"
        if title in titles:
            errors.append(f"{where}: duplicate title (also {titles[title]})")
        titles[title] = where
        pg, ans = strings_in(pages), strings_in(answers)
        everything = [title, question] + pg + ans
        for s in everything:
            bad = {c for c in s if c not in UK and c not in ASCII}
            if bad:
                errors.append(f"{where}: characters the font lacks: {''.join(sorted(bad))}")
            if RUSSIAN & set(s):
                errors.append(f"{where}: Russian letters in: {s}")
            if re.search(r"[A-Za-z]", s):
                errors.append(f"{where}: Latin letters in: {s}")
            if "'" in s:
                errors.append(f"{where}: use the apostrophe ʼ (U+02BC), not ': {s}")
            for w in words_of(s.lower()):
                if w in NOT_UKRAINIAN:
                    errors.append(f"{where}: not Ukrainian: {w}")
            if level == 1 and NOT_LEVEL1 & set(s):
                errors.append(f"{where}: level 1 has no я ю є ї or apostrophe: {s}")
        if level not in WORDS:
            errors.append(f"{where}: level {level} out of range (1..5)")
            continue
        if not (0 <= int(icon) <= 7):
            errors.append(f"{where}: icon {icon} out of range")
        nw = sum(len(words_of(p)) for p in pg)
        lo, hi = WORDS[level]
        if not (lo <= nw <= hi):
            errors.append(f"{where}: {nw} words, level {level} wants {lo}..{hi}")
        if not (3 <= len(pg) <= 11):
            errors.append(f"{where}: {len(pg)} pages (want 3..11)")
        size = 48 if level in BIG else 24
        for n, p in enumerate(pg, 1):
            if why := fits(p, size, (PAGE_W[size], PAGE_LINES[size])):
                errors.append(f"{where}: page {n}: {why}")
        for what, t, box in (("title", title, TITLE), ("cover title", title, COVER_TITLE), ("question", question, QUESTION)):
            if why := fits(t, 24, box):
                errors.append(f"{where}: {what}: {why}")
        if not (2 <= len(ans) <= 3):
            errors.append(f"{where}: {len(ans)} answers (want 2..3)")
        for a in ans:
            if why := fits(a, 24, ANSWER):
                errors.append(f"{where}: answer '{a}': {why}")
        if not (0 <= int(correct) < len(ans)):
            errors.append(f"{where}: correct index {correct} out of range")
    print(f"ukrainian books: {len(titles)} stories, {len(errors)} problems")
    for e in errors:
        print("  " + e)
    return 1 if errors else 0


if __name__ == "__main__":
    sys.exit(main())
