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
NOT_UKRAINIAN = {
    "тоже",
    "ладно",
    "сейчас",
    "вообще",
    "кот",
    "конечно",
    "щас",
    "пожалуста",
    "спасибо",
    "смотри",
    "всьо",
    "канєшно",
    "получилось",
    "звонить",
    "кушать",
    "нету",
    "єслі",
    "шо",
    "чьо",
    "ніхто-ніхто",
}


def font_widths(size):
    src = Path("os/uk_fonts.h").read_text(encoding="utf-8")
    extras = [int(x, 16) for x in re.findall(r"0x([0-9A-F]+)", re.search(r"EXTRAS\[\] = \{([^}]*)\}", src).group(1))]
    body = re.search(rf"FONT{size}_GLYPHS\[\] = \{{(.*?)\}};", src, re.S).group(1)
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
    return [m.encode("utf-8").decode("unicode_escape").encode("latin-1").decode("utf-8") for m in re.findall(r'"((?:[^"\\]|\\.)*)"', block)]


def words_of(t):
    return re.findall(r"[^\W\d_]+(?:[ʼ’'][^\W\d_]+)*", t)


def string_problems(s, level):
    """What is wrong with one string of a book: letters the font or the level lacks, Russian or Latin, the apostrophe."""
    out = []
    bad = {c for c in s if c not in UK and c not in ASCII}
    if bad:
        out.append(f"characters the font lacks: {''.join(sorted(bad))}")
    if RUSSIAN & set(s):
        out.append(f"Russian letters in: {s}")
    if re.search(r"[A-Za-z]", s):
        out.append(f"Latin letters in: {s}")
    if "'" in s:
        out.append(f"use the apostrophe ʼ (U+02BC), not ': {s}")
    out += [f"not Ukrainian: {w}" for w in words_of(s.lower()) if w in NOT_UKRAINIAN]
    if level == 1 and NOT_LEVEL1 & set(s):
        out.append(f"level 1 has no я ю є ї or apostrophe: {s}")
    return out


def size_problems(title, level, pg, question, ans):
    """Words for the level, pages and answers counted, and every string in its box."""
    out = []
    nw = sum(len(words_of(p)) for p in pg)
    lo, hi = WORDS[level]
    if not (lo <= nw <= hi):
        out.append(f"{nw} words, level {level} wants {lo}..{hi}")
    if not (3 <= len(pg) <= 11):
        out.append(f"{len(pg)} pages (want 3..11)")
    size = 48 if level in BIG else 24
    out += [f"page {n}: {why}" for n, p in enumerate(pg, 1) if (why := fits(p, size, (PAGE_W[size], PAGE_LINES[size])))]
    boxes = (("title", title, TITLE), ("cover title", title, COVER_TITLE), ("question", question, QUESTION))
    out += [f"{what}: {why}" for what, t, box in boxes if (why := fits(t, 24, box))]
    if not (2 <= len(ans) <= 3):
        out.append(f"{len(ans)} answers (want 2..3)")
    out += [f"answer '{a}': {why}" for a in ans if (why := fits(a, 24, ANSWER))]
    return out


def book_problems(groups):
    title, icon, level, pages, question, answers, correct = groups
    level = int(level)
    pg, ans = strings_in(pages), strings_in(answers)
    out = [e for s in [title, question] + pg + ans for e in string_problems(s, level)]
    if level not in WORDS:
        return out + [f"level {level} out of range (1..5)"]
    if not (0 <= int(icon) <= 7):
        out.append(f"icon {icon} out of range")
    out += size_problems(title, level, pg, question, ans)
    if not (0 <= int(correct) < len(ans)):
        out.append(f"correct index {correct} out of range")
    return out


def main():
    errors, titles = [], {}
    text = FILE.read_text(encoding="utf-8")
    book_re = (
        r'\{"((?:[^"\\]|\\.)*)",\s*C_\w+,\s*(\d+),\s*(\d+),\s*\{(.*?)nullptr\},'
        r'\s*"((?:[^"\\]|\\.)*)",\s*\{(.*?)\},\s*(\d+)\}'
    )
    for m in re.finditer(book_re, text, re.S):
        title = m.group(1)
        where = f"{FILE}:{text.count(chr(10), 0, m.start()) + 1} [{title}]"
        if title in titles:
            errors.append(f"{where}: duplicate title (also {titles[title]})")
        titles[title] = where
        errors += [f"{where}: {e}" for e in book_problems(m.groups())]
    print(f"ukrainian books: {len(titles)} stories, {len(errors)} problems")
    for e in errors:
        print("  " + e)
    return 1 if errors else 0


if __name__ == "__main__":
    sys.exit(main())
