#!/usr/bin/env python3
"""Content gate for Biscuit: stories, discoveries, adventures, tricks and stickers.

Checks every string in games/biscuit/content_*.h: the glyph set of the bundled font (ASCII 32-126 plus e-acute,
o-umlaut and the middle dot), only {name} and {pet} tokens and only in copy the game personalizes, every required
field present, the fixed counts, unique ids, the discovery order equal to DiscoveryId in generated/discovery_art.h
(pictures and save bits are indexed by it), and a character-count ceiling per field.

TODO(font gate): the character ceilings are a cheap stand-in. Replace them with the pixel-accurate check once
games/biscuit/generated/fonts.h exists: measure every string with its font's advances and kerning, apply the
font::pageBreaks fill rule (352x176 box, font24, 4 px line spacing) to story and discovery pages, and the circle rule
to every box the game draws a string in. Measure tokens at their widest fill (an 11-letter name).
"""

import os
import re
import sys
from pathlib import Path

os.chdir(os.path.dirname(os.path.abspath(__file__)) + "/../../..")  # paths below are relative to apps/porthole
GAME = Path("games/biscuit")
GLYPHS = {chr(c) for c in range(32, 127)} | {"é", "ö", "·"}
TOKEN = re.compile(r"\{(\w*)\}")
CUES, ACTIONS = set("LURDP"), {"feed", "play", "pet", "read", "train", "rest"}
# Longest string of each kind, in characters (TODO(font gate) above).
LIMITS = dict(
    story_title=24,
    subtitle=32,
    page=200,
    prompt=80,
    label=30,
    topic=12,
    fact_title=52,
    fact_page=180,
    wonder=140,
    source=48,
    url=160,
    adv_title=28,
    description=110,
    word=14,
    meaning=125,
    trick=14,
    sticker=16,
)
PERSONALIZED = {"page", "prompt", "fact_page", "wonder", "description", "meaning"}

LEX = re.compile(r'"((?:[^"\\]|\\.)*)"|([{},])|([\w.]+)|(\s+|//[^\n]*)', re.S)


def decode(s):
    return re.sub(r"\\(u[0-9a-fA-F]{4}|.)", lambda m: chr(int(m[1][1:], 16)) if len(m[1]) == 5 else m[1], s)


def parse(src, pos):
    """The brace initializer starting at src[pos] == '{' as nested lists of str (literals) and bare words."""
    stack = []
    for m in LEX.finditer(src, pos):
        if m[1] is not None:
            stack[-1].append(decode(m[1]))
        elif m[2] == "{":
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
        if len(s) > LIMITS[kind]:
            self.errors.append(f"{where}: {kind} is {len(s)} characters, limit {LIMITS[kind]}")

    def count(self, what, items, n):
        if len(items) != n:
            self.errors.append(f"{what}: {len(items)} entries, expected {n}")

    def unique(self, what, ids):
        dupes = sorted({i for i in ids if ids.count(i) > 1})
        if dupes:
            self.errors.append(f"{what}: duplicate ids {dupes}")


def stories(g):
    rows = table("content_stories.h", "STORIES")
    g.count("STORIES", rows, 7)
    g.unique("STORIES", [r[0] for r in rows])
    for r in rows:
        sid, title, subtitle, day, pages, prompt, choices = r
        g.text(sid, "story_title", title)
        g.text(sid, "subtitle", subtitle)
        g.count(f"{sid} pages", pages, 10)
        g.text(sid, "prompt", prompt)
        g.count(f"{sid} choices", choices, 2)
        if not day.isdigit() or not 1 <= int(day) <= 30:
            g.errors.append(f"{sid}: unlockDay {day} outside 1..30")
        for i, p in enumerate(pages):
            g.text(f"{sid} page {i + 1}", "page", p)
        for label, ending in choices:
            g.text(sid, "label", label)
            g.count(f"{sid} ending '{label}'", ending, 4)
            for i, p in enumerate(ending):
                g.text(f"{sid} '{label}' page {i + 1}", "page", p)


def discoveries(g):
    topics = table("content_discoveries.h", "TOPICS")
    g.count("TOPICS", topics, 12)
    for tid, name in topics:
        g.text(tid, "topic", name)
    rows = table("content_discoveries.h", "DISCOVERIES")
    g.count("DISCOVERIES", rows, 96)
    ids = [r[0] for r in rows]
    g.unique("DISCOVERIES", ids)
    for fid, topic, title, pages, wonder, source, url in rows:
        tid = topics[int(topic)][0] if topic.isdigit() and int(topic) < len(topics) else None
        if tid is None or not fid.startswith(f"{tid}-"):
            g.errors.append(f"{fid}: topic {topic} does not match the id")
        g.text(fid, "fact_title", title)
        g.count(f"{fid} pages", pages, 2)
        for p in pages:
            g.text(fid, "fact_page", p)
        g.text(fid, "wonder", wonder)
        g.text(fid, "source", source)
        g.text(fid, "url", url)
        if not url.startswith("https://"):
            g.errors.append(f"{fid}: source url is not https")
    for tid, _ in topics:
        n = sum(r[0].startswith(f"{tid}-") for r in rows)
        if n != 8:
            g.errors.append(f"topic {tid}: {n} discoveries, expected 8")
    art = (GAME / "generated/discovery_art.h").read_text(encoding="utf-8")
    enum = re.findall(r"^\s*DISC_(\w+),", art, re.M)
    if [e.lower().replace("_", "-") for e in enum] != ids:
        g.errors.append("DISCOVERIES order differs from DiscoveryId in generated/discovery_art.h (make art)")


def daily(g):
    adventures = table("content_daily.h", "ADVENTURES")
    g.count("ADVENTURES", adventures, 7)
    g.unique("ADVENTURES", [a[0] for a in adventures])
    for title, description, actions, word, meaning in adventures:
        g.text(title, "adv_title", title)
        g.text(title, "description", description)
        g.text(title, "word", word)
        g.text(title, "meaning", meaning)
        if len(set(actions)) != 3 or not set(actions) <= ACTIONS:
            g.errors.append(f"{title}: actions {actions} must be three different of {sorted(ACTIONS)}")
    tricks = table("content_daily.h", "TRICKS")
    g.count("TRICKS", tricks, 6)
    g.unique("TRICKS", [t[0] for t in tricks])
    for tid, name, _day, lessons in tricks:
        g.text(tid, "trick", name)
        g.count(f"{tid} lessons", lessons, 3)
        for cues in lessons:
            if not 3 <= len(cues) <= 6 or not set(cues) <= CUES:
                g.errors.append(f"{tid}: lesson {cues!r} must be 3-6 of L U R D P")
    stickers = table("content_daily.h", "STICKERS")
    g.count("STICKERS", stickers, 12)
    for s in stickers:
        g.text(s, "sticker", s)


def main():
    g = Gate()
    for path in sorted(GAME.glob("content_*.h")):
        for n, line in enumerate(path.read_text(encoding="utf-8").splitlines(), 1):
            if any(ord(c) > 126 for c in line):
                g.errors.append(f"{path}:{n}: non-ASCII byte in the source (write \\u00e9 and friends)")
    stories(g)
    discoveries(g)
    daily(g)
    for e in g.errors:
        print(f"check_content: {e}")
    if g.errors:
        sys.exit(1)
    print("check_content: biscuit content OK (7 stories, 96 discoveries, 7 adventures, 6 tricks, 12 stickers)")


if __name__ == "__main__":
    main()
