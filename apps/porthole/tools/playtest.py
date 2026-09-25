#!/usr/bin/env python3
"""Porthole playtest runner (the shell and Pets Club): scripted playthroughs with assertions, a UI audit, and sanitizer monkey runs.

  python3 tools/playtest.py            every scenario in tests/playtests/ (make playtest builds the sims first)
  python3 tools/playtest.py 06 monkey  only scenarios whose file name contains one of the words

PLAYTEST_SNAP and PLAYTEST_OUT override the non-monkey sim binary and the report directory (make coverage uses both).

A scenario is a sim script (host/sim.cpp) plus runner directives; docs/playtesting.md has the syntax and the rules.
Directives that need live state (follow-glow, solve-word, answer-book) run the script so far, read the answer and
append the taps: the sim is deterministic, so every run replays the same game. Report: build/playtest/report.md.
"""

import math
import os
import re
import subprocess
import sys
import tempfile
import time
from concurrent.futures import ThreadPoolExecutor
from pathlib import Path

ROOT = Path(__file__).resolve().parent.parent
OUT = Path(os.environ.get("PLAYTEST_OUT") or ROOT / "build" / "playtest").resolve()
SNAP = Path(os.environ.get("PLAYTEST_SNAP") or ROOT / "build" / "host" / "snap").resolve()
_pal = (ROOT / "os/palette.h").read_text()


def _block(pattern: str, text: str) -> str:
    """First capture group of a required source pattern; fail loudly if the source moved."""
    m = re.search(pattern, text, re.S)
    if not m:
        raise SystemExit(f"playtest: pattern not found in game source: {pattern}")
    return m[1]


PALETTE = [int(h, 16) for h in re.findall(r"0x([0-9A-Fa-f]{6})", _block(r"PALETTE_RGB\[C_COUNT\] = \{(.*?)\};", _pal))]
COLOR = [n.lower() for n in re.findall(r"\bC_([A-Z]+)\b", _block(r"enum Col : uint8_t \{(.*?)\};", _pal))][: len(PALETTE)]
SCREENS = {  # the shell's screens and every game's (each lists its names in a `static const char* N[]`)
    name
    for src in ("shell/shell.cpp", "games/pets-club/game.cpp")
    for name in re.findall(r'"(\w+)"', _block(r"static const char\* N\[\] = \{(.*?)\};", (ROOT / src).read_text()))
}
TAP_ANYWHERE = {"splash", "celebrate", "intro", "gift"}  # screens where the whole glass is the button
SANITIZER = re.compile(r"ERROR: (Address|Leak)Sanitizer|runtime error:")
OPS = {
    "=": lambda a, b: a == b,
    "==": lambda a, b: a == b,
    "!=": lambda a, b: a != b,
    "<": lambda a, b: a < b,
    "<=": lambda a, b: a <= b,
    ">": lambda a, b: a > b,
    ">=": lambda a, b: a >= b,
}


class Fail(Exception):
    def __init__(self, msg, context=()):
        super().__init__(msg)
        self.context = list(context)


def contrast(a, b):  # WCAG 2 contrast ratio of two palette indices
    def lum(c):
        r, g, b = (v / 255 for v in ((c >> 16) & 255, (c >> 8) & 255, c & 255))
        r, g, b = (v / 12.92 if v <= 0.03928 else ((v + 0.055) / 1.055) ** 2.4 for v in (r, g, b))
        return 0.2126 * r + 0.7152 * g + 0.0722 * b

    hi, lo = sorted((lum(PALETTE[a]), lum(PALETTE[b])), reverse=True)
    return (hi + 0.05) / (lo + 0.05)


def audit(tag, screen, regions, texts, full):
    """UI rules in logical px (1 px = 0.33 mm). Returns [(severity, rule, tag, detail)]."""
    out = []

    def add(sev, rule, detail):
        out.append((sev, rule, tag, detail))

    def name(r):
        return f"region {r[0]},{r[1]} {r[2]}x{r[3]}"

    if full:
        add("WARN", "audit", "registry full: some regions or texts were not recorded")
    if not regions and screen not in TAP_ANYWHERE:
        add("WARN", "dead screen", "nothing tappable")
    for r in regions:
        x, y, w, h = r
        if min(w, h) < 18:
            add("FAIL", "target size", f"{name(r)}: {min(w, h)} px < 18 px (6 mm)")
        elif w < 24 or h < 22:
            add("WARN", "target size", f"{name(r)}: under 24x22 px (8 mm)")
        inside = sum((px + 0.5 - 80) ** 2 + (py + 0.5 - 80) ** 2 <= 6400 for px in range(x, x + w) for py in range(y, y + h)) / max(1, w * h)
        dist = math.hypot(x + w / 2 - 80, y + h / 2 - 80)
        if inside < 0.85 or dist > 70:
            add("FAIL", "round edge", f"{name(r)}: {inside:.0%} inside the glass, center {dist:.0f} px from the middle")
    for i, a in enumerate(regions):
        for b in regions[i + 1 :]:
            ox = min(a[0] + a[2], b[0] + b[2]) - max(a[0], b[0])
            oy = min(a[1] + a[3], b[1] + b[3]) - max(a[1], b[1])
            if ox > 0 and oy > 0:  # intersecting: up to 4 px of hit padding is fine
                if ox > 4 and oy > 4:

                    def holds(p, q):
                        return p[0] <= q[0] and p[1] <= q[1] and p[0] + p[2] >= q[0] + q[2] and p[1] + p[3] >= q[1] + q[3]

                    nested = holds(a, b) or holds(b, a)
                    add("WARN" if nested else "FAIL", "overlap", f"{name(a)} and {name(b)} share {ox}x{oy} px" + (" (nested)" if nested else ""))
            elif math.hypot(max(0, -ox), max(0, -oy)) < 2:
                add("WARN", "spacing", f"{name(a)} and {name(b)} are {math.hypot(max(0, -ox), max(0, -oy)):.0f} px apart")
    for i, (x, y, w, h, c, bg) in enumerate(texts):
        if i + 1 < len(texts) and texts[i + 1][:4] == (x - h // 8, y - h // 8, w, h):
            continue  # a textShadow shadow (drawn first, offset by the scale): judged with its letters below
        t = f"text {x},{y} {w}x{h} {COLOR[c]}"
        sh = texts[i - 1] if i and texts[i - 1][:4] == (x + h // 8, y + h // 8, w, h) else None
        t += f" (with a {COLOR[sh[4]]} shadow)" if sh else ""
        far = max(math.hypot(px - 80, py - 80) for px in (x, x + w) for py in (y, y + h))
        if far > 79:
            add("FAIL", "clipped text", f"{t}: a corner is {far:.1f} px from the middle (> 79)")
        if c != bg:  # equal means the sampler saw the text itself: unknown
            k = contrast(c, bg)
            if k < 4:
                add("FAIL" if k < 2.5 else "WARN", "contrast", f"{t} on {COLOR[bg]}: {k:.2f}:1 (< {'2.5' if k < 2.5 else '4.0'})")
    return out


def segments(out):  # output between `echo @@<id>` and `echo @@`, per id
    seg, cur = {}, None
    for line in out.splitlines():
        if line.startswith("@@"):
            cur = line[2:] or None
            if cur:
                seg[cur] = []
        elif cur:
            seg[cur].append(line)
    return seg


def pairs(lines):  # latest key=value pairs of the debug output
    d = {}
    for line in lines:
        d.update(re.findall(r'(\w+)=("[^"]*"|[^\s\]]+)', line))
    return {k: v.strip('"') for k, v in d.items()}


def plan(d, seg, where):
    """Taps for the interactive directives, computed from the live state."""
    if d == "follow-glow":
        glows = [ln.split()[1:3] for ln in seg if ln.startswith("glow ")]
        if not glows:
            raise Fail(f"{where}: no glow in 6 s", seg)
        return [c for x, y in glows for c in (f"tap {x} {y}", "wait 250")]
    kv = pairs(seg)
    if d == "answer-book":
        if "correct" not in kv:
            raise Fail(f"{where}: no book open", seg)
        return [f"tap 80 {74 + int(kv['correct']) * 22 + 9}"]
    if "word" not in kv:
        raise Fail(f"{where}: no word on screen", seg)
    word, cols, tiles = kv["word"], int(kv["cols"]), kv["tiles"]
    x0, y0 = (41, 66) if cols == 3 else (28, 60)
    used, taps = set(), []
    for ch in word[int(kv["typed"]) :]:  # meant for the start of a word: tiles already typed are not in the debug output
        i = next((i for i, t in enumerate(tiles) if t == ch and i not in used), None)
        if i is None:
            raise Fail(f"{where}: letter {ch} of {word} is not among the tiles {tiles}", seg)
        used.add(i)
        taps.append(f"tap {x0 + (i % cols) * 26 + 12} {y0 + (i // cols) * 26 + 12}")
    return taps


def compare(key, got, op, want):
    if key in ("books", "profile") and "/" not in want:
        got = got.split("/")[0]
    if key == "play":
        got, want = got.rstrip("s"), want.rstrip("s")
    try:
        got, want = float(got), float(want)
    except ValueError:
        if op not in ("=", "==", "!="):
            raise Fail(f"cannot order {got!r} {op} {want!r}") from None
    return OPS[op](got, want)


def unroll(text):  # (line number, line) with `repeat N` ... `end` blocks unrolled; blocks do not nest
    out, block = [], None
    for no, line in enumerate(text.splitlines(), 1):
        w = line.split()
        if w[:1] == ["repeat"]:
            if block is not None or len(w) != 2 or not w[1].isdigit():
                raise Fail(f"line {no}: expected `repeat N` outside another repeat")
            block, times = [], int(w[1])
        elif w == ["end"] and block is not None:
            out, block = out + block * times, None
        else:
            (out if block is None else block).append((no, line))
    if block is not None:
        raise Fail("`repeat` without `end`")
    return out


def make_sheet(snaps, dest):
    try:
        from PIL import Image, ImageDraw
    except ImportError:  # sheets are optional
        return False
    shots = [(p.stem, Image.open(p).convert("RGB").resize((160, 160), Image.NEAREST).resize((320, 320), Image.NEAREST)) for p in snaps if p.exists()]
    if not shots:
        return False
    cols = min(4, len(shots))
    sheet = Image.new("RGB", (cols * 328 + 8, (len(shots) + cols - 1) // cols * 344 + 8), (24, 24, 24))
    draw = ImageDraw.Draw(sheet)
    for i, (label, im) in enumerate(shots):
        x, y = 8 + i % cols * 328, 8 + i // cols * 344
        sheet.paste(im, (x, y))
        draw.text((x, y + 324), label, fill=(235, 235, 235))
    sheet.save(dest)
    return True


def run_scenario(path):
    res = {"name": path.stem, "fails": [], "ui": [], "sheets": [], "checks": 0, "sec": 0.0}
    start = time.monotonic()
    for old in OUT.glob(f"{path.stem}-*.png"):  # no stale sheet survives a renamed or removed `sheet`
        old.unlink()
    with tempfile.TemporaryDirectory() as tmp:  # own cwd: the sim writes saves and snaps under build/host/
        (Path(tmp) / "build" / "host").mkdir(parents=True)
        try:
            play(path, Path(tmp), res, start)
        except Fail as e:
            res["fails"].append((str(e), e.context))
        except Exception as e:  # a harness bug must show up as a failure, not kill the other scenarios
            res["fails"].append((f"harness error: {e!r}", []))
    res["sec"] = time.monotonic() - start
    return res


def play(path, cwd, res, start):
    binary, limit = sim_for(path), 120 if "monkey" in path.stem else 30
    lines, checks = [], []  # expanded script; (id, where, kind, data) per query

    def query(cmd, where, kind, data=None):  # output lands between `@@<id>` and `@@`
        qid = str(len(checks))
        checks.append((qid, where, kind, data))
        lines.extend([f"echo @@{qid}", *([cmd] if cmd else []), "echo @@"])
        return qid

    def run():
        (cwd / "script.txt").write_text("\n".join(lines) + "\n")
        try:
            p = subprocess.run(
                [str(binary), "--fresh", "--script", "script.txt"],
                cwd=cwd,
                capture_output=True,
                text=True,
                errors="replace",
                timeout=max(1.0, start + limit - time.monotonic()),
                env=dict(os.environ, UBSAN_OPTIONS="print_stacktrace=1"),
            )
        except subprocess.TimeoutExpired as e:
            raise Fail(f"timeout: over {limit} s", (e.stdout or b"").decode(errors="replace").splitlines()[-12:]) from None
        bad = SANITIZER.search(p.stdout + p.stderr)
        if bad or p.returncode or "unknown command" in p.stderr:
            what = "sanitizer error" if bad else f"exit code {p.returncode}" if p.returncode else "unknown sim command"
            raise Fail(f"{what} ({binary.name})", p.stdout.splitlines()[-8:] + ["--- stderr"] + p.stderr.splitlines()[:80])
        return p.stdout

    for no, raw in unroll(path.read_text()):
        w = raw.split()
        if not w or w[0].startswith("#"):
            continue
        d, arg, where = w[0], " ".join(w[1:]), f"line {no} `{raw.strip()}`"
        if d == "expect":
            m = re.fullmatch(r"(\w+)\s*(==|!=|<=|>=|=|<|>)\s*(.+)", arg)
            if not m:
                raise Fail(f"{where}: expected `expect <key><op><value>`")
            query("debug", where, d, m.groups())
        elif d in ("expect-screen", "ui-check", "sheet"):
            query({"expect-screen": "screen", "ui-check": "ui", "sheet": None}[d], where, d, arg)
        elif d in ("follow-glow", "solve-word", "answer-book"):
            qid = query("watch 6000" if d == "follow-glow" else "debug", where, "info")
            lines.extend(plan(d, segments(run()).get(qid, []), where))
        else:
            lines.append(raw.strip())
    out = run()

    seg, snaps, sheet_at = segments(out), [], {}
    sheet_ids = {c[0] for c in checks if c[2] == "sheet"}
    for line in out.splitlines():  # a sheet takes the snaps written since the previous sheet
        if line.startswith("wrote "):
            snaps.append(cwd / line[6:])
        elif line.startswith("@@") and line[2:] in sheet_ids:
            sheet_at[line[2:]], snaps = snaps, []
    for qid, where, kind, data in checks:
        s = seg.get(qid)
        if s is None:
            res["fails"].append((f"{where}: no output (the sim stopped early)", out.splitlines()[-12:]))
            continue
        res["checks"] += 1
        if kind == "expect":
            key, op, want = data
            got = pairs(s).get(key)
            try:
                ok = got is not None and compare(key, got, op, want)
            except Fail as e:
                ok, got = False, f"{got} ({e})"
            if not ok:
                res["fails"].append((f"{where}: got {key}={got}" if got is not None else f"{where}: no {key}= in the debug output", s))
        elif kind == "expect-screen":
            got = pairs(s).get("screen", "?")
            if not (got in SCREENS if data == "*" else got in data.split("|")):
                res["fails"].append((f"{where}: screen is {got}", s))
        elif kind == "ui-check":
            head = pairs(s[:1])
            regions = [tuple(map(int, ln.split()[1:5])) for ln in s if ln.startswith("region ")]
            texts = [tuple(map(int, ln.split()[1:5])) + tuple(int(v) for v in re.findall(r"=(\d+)", ln)) for ln in s if ln.startswith("text ")]
            full = int(head.get("regions", 0)) >= 96 or int(head.get("texts", 0)) >= 64
            screen = head.get("screen", "?")
            tag = data if data.startswith(screen) else f"{data} ({screen})" if data else screen  # a tag never hides the real screen
            res["ui"] += [(f, where) for f in audit(tag, screen, regions, texts, full)]
        elif kind == "sheet":
            dest = OUT / f"{path.stem}-{data or 'sheet'}.png"
            if make_sheet(sheet_at.get(qid, []), dest):
                res["sheets"].append(dest)


def ui_count(r, sev):
    return len({f for f, _ in r["ui"] if f[0] == sev})


def failed(r):
    return bool(r["fails"]) or ui_count(r, "FAIL") > 0 or ui_count(r, "WARN") > 0  # warnings are errors


def report(results):
    try:
        rev = subprocess.run(["git", "describe", "--always", "--dirty"], cwd=ROOT, capture_output=True, text=True).stdout.strip()
    except OSError:
        rev = ""
    md = [
        "# Pets Club playtest report",
        "",
        f"`python3 tools/playtest.py` at {rev or 'unknown revision'}, {time.strftime('%Y-%m-%d %H:%M %Z')}: "
        f"{sum(not failed(r) for r in results)} of {len(results)} scenarios pass. A scenario fails on a failed directive, a crash, "
        "a sanitizer report or any UI finding: FAIL and WARN both fail a run (warnings are errors).",
        "",
        "| Scenario | Result | Checks | Failures | UI FAIL | UI WARN | Time |",
        "| --- | --- | --- | --- | --- | --- | --- |",
    ]
    for r in results:
        md.append(
            f"| {r['name']} | {'FAIL' if failed(r) else 'PASS'} | {r['checks']} | {len(r['fails'])} | {ui_count(r, 'FAIL')} | {ui_count(r, 'WARN')} | {r['sec']:.1f} s |"
        )
    if any(r["fails"] for r in results):
        md += ["", "## Failures"]
        for r in results:
            for msg, ctx in r["fails"]:
                md += ["", f"**{r['name']}**: {msg}"] + (["", "```", *ctx, "```"] if ctx else [])
    seen = {}  # finding -> scenarios; the same screen audited twice reports once
    for r in results:
        for f, _ in r["ui"]:
            seen.setdefault(f, set()).add(r["name"])
    groups = {}
    for f in sorted(seen, key=lambda f: (f[0] != "FAIL", f[1], f[2], [int(n) for n in re.findall(r"\d+", f[3])])):  # FAIL first, then top-left first
        groups.setdefault(f[:3], []).append(f[3])
    md += [
        "",
        "## UI findings",
        "",
        f"{sum(f[0] == 'FAIL' for f in seen)} FAIL (hard rule) and {sum(f[0] == 'WARN' for f in seen)} WARN (soft rule) "
        "findings. Logical px: 1 px = 0.33 mm, the glass is the circle of radius 80 around (80,80).",
        "",
        "| Severity | Rule | Screen | Count | Findings | Seen in |",
        "| --- | --- | --- | --- | --- | --- |",
    ]
    for (sev, rule, tag), details in groups.items():
        names = sorted(set().union(*(seen[(sev, rule, tag, d)] for d in details)))
        shown = "<br>".join(details[:4]) + (f"<br>... {len(details) - 4} more below" if len(details) > 4 else "")
        md.append(f"| {sev} | {rule} | {tag} | {len(details)} | {shown} | {', '.join(names)} |")
    long = [(k, d) for k, d in groups.items() if len(d) > 4]
    if long:
        md += ["", "### Complete lists", ""]
        for (sev, rule, tag), details in long:
            md += [f"<details><summary>{sev} {rule} on {tag}: {len(details)}</summary>", "", *[f"- {d}" for d in details], "", "</details>", ""]
    sheets = [s for r in results for s in r["sheets"]]
    if sheets:
        md += ["", "## Contact sheets", ""] + [f"- {os.path.relpath(s, ROOT)}" for s in sheets]
    (OUT / "report.md").write_text("\n".join(md) + "\n")


def sim_for(path):  # monkeys run under the sanitizers
    return ROOT / "build" / "host" / "snap-asan" if "monkey" in path.stem else SNAP


def main():
    want = sys.argv[1:]
    paths = [p for p in sorted((ROOT / "tests" / "playtests").glob("*.txt")) if not want or any(w in p.stem for w in want)]
    missing = sorted({os.path.relpath(sim_for(p), ROOT) for p in paths if not sim_for(p).exists()})
    if missing or not paths:
        sys.exit(f"playtest: {'missing ' + ', '.join(missing) + ' (run make playtest)' if missing else 'no scenario matches'}")
    OUT.mkdir(parents=True, exist_ok=True)
    results = []
    with ThreadPoolExecutor(os.cpu_count()) as pool:  # every scenario has its own cwd, so they run side by side
        for r in pool.map(run_scenario, paths):
            results.append(r)
            print(f"{'FAIL' if failed(r) else 'PASS'} {r['name']} ({r['sec']:.1f} s, {r['checks']} checks, UI {ui_count(r, 'FAIL')} FAIL / {ui_count(r, 'WARN')} WARN)")
            for msg, ctx in r["fails"]:
                print(f"     {msg}")
                for line in ctx[:40]:
                    print(f"     | {line}")
            for (_, rule, tag, detail), where in dict.fromkeys((f, w) for f, w in r["ui"]):
                print(f"     {where}: {rule} on {tag}: {detail}")
    report(results)
    bad = sum(failed(r) for r in results)
    if not any(r["sheets"] for r in results) and any("\nsheet" in p.read_text() for p in paths):
        print("note: no contact sheets written (pip install pillow to get them)")
    print(f"{len(results) - bad} of {len(results)} scenarios pass; report: {os.path.relpath(OUT / 'report.md', ROOT)}")
    sys.exit(1 if bad else 0)


if __name__ == "__main__":
    main()
