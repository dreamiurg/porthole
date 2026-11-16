#!/usr/bin/env python3
"""Content gate for Pets Club's puzzles and little reads (content_puzzles.h, content_reads.h).

Checks the glyphs (ASCII 32-126, the 8x8 font), the {name} and {pet} tokens, the spelling words' letters, and each
little read: its fields, its word count for its level, what levels 1 and 2 may spell, and how the right answers spread.
Whether each string fits its box is measured in pixels in host/test_pets_learn.cpp (make test).
"""

import os
import re
import sys
from pathlib import Path

os.chdir(os.path.dirname(os.path.abspath(__file__)) + "/../../..")  # paths below are relative to apps/porthole
GAME = Path("games/pets-club")
GLYPHS = {chr(c) for c in range(32, 127)}
TOKEN = re.compile(r"\{(\w*)\}")
PERSONALIZED = {"clue", "read", "question", "answer", "why"}  # learn.cpp fills {name} and {pet} in these

LEX = re.compile(r'"((?:[^"\\]|\\.)*)"|([{},])|([\w.]+)|(\s+|//[^\n]*)', re.S)


def decode(s):
    return re.sub(r"\\(u[0-9a-fA-F]{4}|.)", lambda m: chr(int(m[1][1:], 16)) if len(m[1]) == 5 else m[1], s)


def parse(src, pos):
    """The brace initializer starting at src[pos] == '{' as nested lists of str (literals) and bare words."""
    stack, joined = [], False  # joined: the last token was a literal, so the next one continues it (C joins them)
    for m in LEX.finditer(src, pos):
        if m[4] is not None:
            continue
        if m[1] is not None and joined:
            stack[-1][-1] += decode(m[1])
        elif m[1] is not None:
            stack[-1].append(decode(m[1]))
        joined = m[1] is not None
        if m[1] is not None:
            continue
        if m[2] == "{":
            stack.append([])
        elif m[2] == "}":
            done = stack.pop()
            if not stack:
                return done
            stack[-1].append(done)
        elif m[3] is not None:
            stack[-1].append(m[3])
    return None


def table(path, name):
    src = (GAME / path).read_text(encoding="utf-8")
    m = re.search(rf"\b{name}\[\]\s*=\s*\{{", src)
    return parse(src, m.end() - 1)


class Gate:
    def __init__(self):
        self.errors = []

    def text(self, where, kind, s):
        if not s:
            self.errors.append(f"{where}: empty {kind}")
            return
        bad = sorted({c for c in s if c not in GLYPHS})
        if bad:
            self.errors.append(f"{where}: {kind} has glyphs outside the font: {bad}")
        for token in TOKEN.findall(s):
            if kind not in PERSONALIZED:
                self.errors.append(f"{where}: {kind} is drawn as written, so {{{token}}} would show literally")
            elif token not in ("name", "pet"):
                self.errors.append(f"{where}: unknown token {{{token}}} (only {{name}} and {{pet}})")

    def count(self, what, items, n):
        if len(items) != n:
            self.errors.append(f"{what}: {len(items)} entries, expected {n}")

    def unique(self, what, ids):
        dupes = sorted({i for i in ids if ids.count(i) > 1})
        if dupes:
            self.errors.append(f"{what}: duplicate ids {dupes}")


def spelling(g):  # puzzles.h's spell(): lowercase letters for vowel(), and "w o r d" with its gap fits Puzzle::ask[24]
    for name in ("SHORT_WORDS", "LONG_WORDS"):
        rows = table("content_puzzles.h", name)
        g.unique(name, [r[0] for r in rows])
        for word, clue in rows:
            if not re.fullmatch(r"[a-z]{2,11}", word):
                g.errors.append(f"{name} {word!r}: 2 to 11 lowercase letters only")
            g.text(word, "clue", clue)


# Little reads (content_reads.h): what levels 1 and 2 may spell. A word is a sight word, or one syllable of single
# consonants around one short vowel (level 1; -ll -ss -ff and a plural or possessive s too), or of digraphs and
# blends (level 2). {name} and {pet} count as known words.
SIGHT_1 = set(
    "a i the to is said you of was he she we me be my go no so do are for they look see here come what where put "
    "one two like have play down all too saw there with this".split()
)
SIGHT_2 = SIGHT_1 | set(
    "some from were give live again could would does push pull full any many your want new good ball tall fall "
    "call small walk talk out about our now how who why when water little come down day away".split()
)
C = "bcdfghjklmnpqrstvwxyz"
CODA = "bcdfghjklmnpqrstvxz"  # w and y after a vowel make a vowel team (now, day), not a short vowel
WORD_1 = re.compile(rf"[{C}]?[aeiou]([{CODA}]|ll|ss|ff)?('?s)?")
ONSET_2 = rf"(sch|scr|shr|spl|spr|str|thr|squ|qu|sh|ch|th|wh|bl|br|cl|cr|dr|fl|fr|gl|gr|pl|pr|sc|sk|sl|sm|sn|sp|st|sw|tr|tw|[{C}])?"
CODA_2 = rf"(tch|nch|lch|ck|sh|ch|th|ng|nk|ll|ss|ff|zz|st|sk|sp|nd|nt|mp|lt|lk|lp|ld|lf|ft|pt|ct|xt|[{CODA}])?"
WORD_2 = re.compile(rf"{ONSET_2}[aeiou]{CODA_2}(e?s|'s)?")
READ_WORDS = {1: (12, 30), 2: (25, 45), 3: (40, 65), 4: (60, 90), 5: (75, 120)}


def decodable(level, text):
    """The words of a level 1 or 2 text that are neither sight words nor of its letter patterns."""
    sight, pattern = (SIGHT_1, WORD_1) if level == 1 else (SIGHT_2, WORD_2)
    words = re.findall(r"[a-z']+", TOKEN.sub("Pip", text).lower())  # a name reads as a known word, {pet}'s too
    return sorted({w for w in words if w.strip("'") not in sight and not pattern.fullmatch(w.strip("'"))})


def read_entry(g, row):
    rid, level, title, text, question, answers, right, why = row
    level = int(level)
    if not re.fullmatch(r"[a-z0-9]+(-[a-z0-9]+)*", rid):
        g.errors.append(f"{rid}: id is not lowercase-with-hyphens")
    for kind, s in (("read_title", title), ("read", text), ("question", question), ("why", why)):
        g.text(rid, kind, s)
    g.count(f"{rid} answers", answers, 3)
    for a in answers:
        g.text(rid, "answer", a)
    if len(set(answers)) != len(answers):
        g.errors.append(f"{rid}: two answers are the same")
    if right not in ("0", "1", "2"):
        g.errors.append(f"{rid}: right {right} is not 0, 1 or 2")
    if not question.endswith("?"):
        g.errors.append(f"{rid}: the question does not end with ?")
    n = len(TOKEN.sub("Pip", text).split())
    low, high = READ_WORDS[level]
    if not low <= n <= high:
        g.errors.append(f"{rid}: {n} words, level {level} takes {low} to {high}")
    off = decodable(level, " ".join([title, text, question, *answers])) if level <= 2 else []
    if off:
        g.errors.append(f"{rid}: level {level} words outside its patterns and sight words: {off}")


def reads(g):
    rows = table("content_reads.h", "READS")
    g.unique("READS", [r[0] for r in rows])
    by_level = {n: [] for n in READ_WORDS}
    for rid, level, *rest in rows:
        if not level.isdigit() or int(level) not in READ_WORDS:
            g.errors.append(f"{rid}: level {level} outside 1..5")
            continue
        by_level[int(level)].append(rest[-2])
        read_entry(g, [rid, level, *rest])
    for level, rights in by_level.items():
        if len(rights) < 8:
            g.errors.append(f"READS level {level}: {len(rights)} reads, expected at least 8")
        for i in "012":
            if rights.count(i) < 2:
                g.errors.append(f"READS level {level}: the right answer is {i} only {rights.count(i)} times, vary it")


def main():
    g = Gate()
    for name in ("content_puzzles.h", "content_reads.h"):
        for n, line in enumerate((GAME / name).read_text(encoding="utf-8").splitlines(), 1):
            if any(ord(c) > 126 for c in line):
                g.errors.append(f"{name}:{n}: non-ASCII byte in the source")
    spelling(g)
    reads(g)
    for e in g.errors:
        print(f"check_learn: {e}")
    if g.errors:
        sys.exit(1)
    print("check_learn: pets club puzzles and little reads OK")


if __name__ == "__main__":
    main()
