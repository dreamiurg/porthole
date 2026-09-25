#!/usr/bin/env python3
"""Decide which apps need a release, from git history alone.

    python3 tools/release.py plan [--ref HEAD]

prints a JSON list, one entry per app that has releasable commits since its last
tag: {"app", "version", "tag", "notes"}. The release workflow turns each entry into
a tag, a GitHub release and a factory image; nothing is committed back to main.

Rules
- Apps are the directories under apps/ that have a Makefile. Tags are <app>-v<semver>.
- A commit belongs to an app when it touches apps/<app>/ or the shared platform/.
- Conventional commit titles pick the bump: feat -> minor, fix/perf -> patch,
  `type!:` or a "BREAKING CHANGE" footer -> major (minor while the app is below 1.0).
  Other types (docs, chore, ci, test, refactor, style, build) never cause a release
  on their own, but are listed in the notes of a release that happens anyway.
- An app's first release is 0.1.0.
"""

import argparse
import json
import re
import subprocess
from pathlib import Path

ROOT = Path(__file__).resolve().parent.parent
TITLE = re.compile(r"^(?P<type>\w+)(?:\((?P<scope>[^)]*)\))?(?P<bang>!)?:\s*(?P<desc>.+)$")
SECTIONS = [("breaking", "Breaking changes"), ("feat", "Features"), ("fix", "Fixes"), ("other", "Other changes")]
FIRST = (0, 1, 0)


def git(*args: str) -> str:
    return subprocess.run(["git", *args], cwd=ROOT, check=True, capture_output=True, text=True).stdout


def apps() -> list[str]:
    return sorted(p.parent.name for p in (ROOT / "apps").glob("*/Makefile"))


def last_tag(app: str, ref: str) -> tuple[str, tuple[int, int, int]] | None:
    best = None
    for tag in git("tag", "--merged", ref, "--list", f"{app}-v*").split():
        m = re.fullmatch(rf"{re.escape(app)}-v(\d+)\.(\d+)\.(\d+)", tag)
        if m:
            v = (int(m[1]), int(m[2]), int(m[3]))
            if best is None or v > best[1]:
                best = (tag, v)
    return best


def commits(app: str, since: str | None, ref: str) -> list[tuple[str, str, str]]:
    rng = f"{since}..{ref}" if since else ref
    out = git("log", "--no-merges", "--format=%H%x1f%s%x1f%b%x1e", rng, "--", f"apps/{app}/", "platform/")
    return [tuple(c.strip("\n").split("\x1f")) for c in out.split("\x1e") if c.strip()]  # type: ignore[misc]


def classify(subject: str, body: str) -> tuple[str, str]:
    """(kind, text) where kind is breaking | feat | fix | other."""
    m = TITLE.match(subject)
    if not m:
        return "other", subject
    if m["bang"] or "BREAKING CHANGE" in body:
        return "breaking", subject
    if m["type"] == "feat":
        return "feat", subject
    if m["type"] in ("fix", "perf"):
        return "fix", subject
    return "other", subject


def bump(v: tuple[int, int, int], kinds: set[str]) -> tuple[int, int, int] | None:
    major, minor, patch = v
    if "breaking" in kinds:
        return (major, minor + 1, 0) if major == 0 else (major + 1, 0, 0)
    if "feat" in kinds:
        return (major, minor + 1, 0)
    if "fix" in kinds:
        return (major, minor, patch + 1)
    return None


def plan(ref: str) -> list[dict[str, str]]:
    out = []
    for app in apps():
        prev = last_tag(app, ref)
        items = [classify(s, b) for _, s, b in commits(app, prev[0] if prev else None, ref)]
        kinds = {k for k, _ in items}
        new = FIRST if prev is None and kinds & {"breaking", "feat", "fix"} else bump(prev[1], kinds) if prev else None
        if new is None:
            continue
        version = ".".join(map(str, new))
        tag = f"{app}-v{version}"
        lines = [f"Changes to `apps/{app}` since {prev[0] if prev else 'the start'}."]
        for kind, heading in SECTIONS:
            picked = [text for k, text in items if k == kind]
            if picked:
                lines += ["", f"### {heading}", "", *[f"- {t}" for t in picked]]
        lines += [
            "",
            "### Install",
            "",
            f"Flash `{app}-{version}-factory.bin` at address `0x0` with "
            "[esptool-js](https://espressif.github.io/esptool-js/) (Chrome or Edge). "
            "A factory image is a fresh install and clears saved progress; to keep saves, "
            f"build from source and run `make flash APP={app}`.",
        ]
        out.append({"app": app, "version": version, "tag": tag, "notes": "\n".join(lines)})
    return out


def main() -> None:
    ap = argparse.ArgumentParser(description=__doc__, formatter_class=argparse.RawDescriptionHelpFormatter)
    ap.add_argument("command", choices=["plan"])
    ap.add_argument("--ref", default="HEAD")
    print(json.dumps(plan(ap.parse_args().ref)))


if __name__ == "__main__":
    main()
