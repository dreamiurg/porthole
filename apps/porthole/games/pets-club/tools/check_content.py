#!/usr/bin/env python3
"""Content gate: stories and spelling words must fit the round 160px screen and the 7-bit font.

Every string is word-wrapped with the real 8x8 proportional font into the box the game draws it in
(pages: 122px x 7-8 lines, titles 108px x 2, questions 116px x 2, answers 102px x 2, clues 118px x 2),
so nothing can spill past a button or the round bezel. Everything must be ASCII (128-glyph font).
"""

import os
import re
import sys

os.chdir(os.path.dirname(os.path.abspath(__file__)) + "/../../..")  # every path below is relative to the Porthole app, wherever this runs from
FILES = ["games/pets-club/content.h", "games/pets-club/content_level3_books.h", "games/pets-club/content_level3_words.h"]
# Boxes the game actually draws into (logical px, lines). Widths follow games/pets-club/game.cpp.
BOXES = dict(page=(122, 7), title=(108, 2), question=(116, 2), answer=(102, 2), clue=(118, 2))


def load_font():
    src = open("os/font8x8_basic.h", encoding="utf-8").read()
    rows = re.findall(r"\{\s*((?:0x[0-9A-Fa-f]{2},?\s*){8})\}", src)
    glyphs = [[int(v, 16) for v in re.findall(r"0x[0-9A-Fa-f]{2}", r)] for r in rows][:128]
    widths = []
    for g in glyphs:
        left, right = 8, -1
        for row in g:
            for x in range(8):
                if row >> x & 1:
                    left, right = min(left, x), max(right, x)
        widths.append(3 if right < 0 else right - left + 1)
    return widths


WIDTHS = load_font()


def text_width(t):
    return sum(WIDTHS[ord(c) if ord(c) < 128 else 63] + 1 for c in t) - 1 if t else 0


def wrap_lines(t, maxw):
    """Mirror of gfx::wrap: greedy word wrap. Returns (line_count, widest_word_px)."""
    lines, cur, widest = 0, "", 0
    for word in t.split():
        widest = max(widest, text_width(word))
        trial = f"{cur} {word}" if cur else word
        if text_width(trial) <= maxw or not cur:
            cur = trial
        else:
            lines += 1
            cur = word
    return (lines + (1 if cur else 0)), widest


def fits(kind, t):
    maxw, maxlines = BOXES[kind]
    n, widest = wrap_lines(t, maxw)
    if widest > maxw:
        return f"word wider than the {maxw}px {kind} box ({widest}px)"
    if n > maxlines:
        return f"{kind} needs {n} lines, box has {maxlines} (at {maxw}px)"
    return None


def strings_in(block):
    return [bytes(m, "utf-8").decode("unicode_escape") for m in re.findall(r'"((?:[^"\\]|\\.)*)"', block)]


def main():
    errors, titles, words = [], {}, {}
    for path in FILES:
        try:
            text = open(path, encoding="utf-8").read()
        except FileNotFoundError:
            continue
        for i, ch in enumerate(text):
            if ord(ch) > 126 or (ord(ch) < 32 and ch not in "\n\t"):
                line = text.count("\n", 0, i) + 1
                errors.append(f"{path}:{line}: non-ASCII character {ch!r}")
                break
        # books: {"Title", COLOR, icon, level, { pages..., nullptr}, "Question", {answers}, correct},
        book_re = (
            r'\{"((?:[^"\\]|\\.)*)",\s*C_\w+,\s*(\d+),\s*(\d+),\s*\{(.*?)nullptr\},'
            r'\s*"((?:[^"\\]|\\.)*)",\s*\{(.*?)\},\s*(\d+)\}'
        )
        for m in re.finditer(book_re, text, re.S):
            title, icon, level, pages, question, answers, correct = m.groups()
            line = text.count("\n", 0, m.start()) + 1
            where = f"{path}:{line} [{title}]"
            if title in titles:
                errors.append(f"{where}: duplicate title (also {titles[title]})")
            titles[title] = where
            if why := fits("title", title):
                errors.append(f"{where}: {why}")
            if not (0 <= int(icon) <= 7):
                errors.append(f"{where}: icon {icon} out of range")
            if not (1 <= int(level) <= 3):
                errors.append(f"{where}: level {level} out of range")
            pg = strings_in(pages)
            if not (3 <= len(pg) <= 11):
                errors.append(f"{where}: {len(pg)} pages (want 3..11)")
            # the reader gives pages 8 lines under a one-line title, 7 under a two-line title
            BOXES["page"] = (122, 8 if wrap_lines(title, 108)[0] == 1 else 7)
            for n, p in enumerate(pg, 1):
                if why := fits("page", p):
                    errors.append(f"{where}: page {n}: {why}")
            # the question sits between the title and three answer buttons: 3 lines under a one-line title, 2 under two
            BOXES["question"] = (116, 3 if wrap_lines(title, 108)[0] == 1 else 2)
            if why := fits("question", question):
                errors.append(f"{where}: {why}")
            ans = strings_in(answers)
            if not (2 <= len(ans) <= 3):
                errors.append(f"{where}: {len(ans)} answers (want 2..3)")
            for a in ans:
                if why := fits("answer", a):
                    errors.append(f"{where}: answer '{a}': {why}")
            if not (0 <= int(correct) < len(ans)):
                errors.append(f"{where}: correct index {correct} out of range")
        # words: {"WORD", "clue"},
        for m in re.finditer(r'\{"([A-Za-z]+)",\s*"((?:[^"\\]|\\.)*)"\}', text):
            word, clue = m.groups()
            line = text.count("\n", 0, m.start()) + 1
            where = f"{path}:{line} [{word}]"
            if not word.isupper():
                errors.append(f"{where}: word must be uppercase")
            if not (3 <= len(word) <= 9):
                errors.append(f"{where}: word length {len(word)} (want 3..9)")
            if word in words:
                errors.append(f"{where}: duplicate word (also {words[word]})")
            words[word] = where
            if why := fits("clue", clue):
                errors.append(f"{where}: clue: {why}")
    print(f"content: {len(titles)} stories, {len(words)} words, {len(errors)} problems")
    for e in errors:
        print("  " + e)
    return 1 if errors else 0


if __name__ == "__main__":
    sys.exit(main())
