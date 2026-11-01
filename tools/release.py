#!/usr/bin/env python3
"""Decide which apps need a release, and what a promotion to stable becomes, from git history alone.

    python3 tools/release.py plan [--ref HEAD]
    gh release list --limit 200 --json tagName,name,isPrerelease,createdAt \\
      | python3 tools/release.py promote [--app porthole] [--tag TAG] [--min-age-hours 72]

`plan` prints a JSON list, one entry per app that has releasable commits since its last
tag: {"app", "version", "tag", "notes"}. The release workflow turns each entry into
a tag and a GitHub pre-release (beta) with a factory image, web-installer parts, the bare
OTA image and a version.txt; nothing is committed back to main.

`promote` reads the app's GitHub releases on stdin and prints the stable release to cut, or
nothing: `source=`, `version=` and `tag=` lines for $GITHUB_OUTPUT, with the notes in --notes.
promote.yml rebuilds the source tag's commit under the new version and publishes it as GitHub's Latest.

Rules
- Apps are the directories under apps/ that have a Makefile. Tags are <app>-v<semver>.
- A commit belongs to an app when it touches apps/<app>/ or the shared platform/.
- Every beta is a patch: any feat, fix, perf or breaking commit since the app's last tag bumps
  the patch. Other types (docs, chore, ci, test, refactor, style, build) never cause a release
  on their own, but are listed in the notes of a release that happens anyway. Notes still group
  breaking changes, features and fixes from the conventional commit titles.
- A stable release is the next minor above every tag the app has: beta 0.38.7 becomes 0.39.0,
  and the next beta is 0.39.1. A board never installs an older version, so a rollback is a
  promotion too: the old code goes out under a new, higher version.
- Unattended promotion picks the newest beta that is newer than the current stable, is at least
  --min-age-hours old, and is not yanked (its release title says "(yanked)"). With --tag, that
  published release is promoted whatever its age; with --min-age-hours 0, the newest beta is.
- An app's first release is 0.1.0.
"""

import argparse
import json
import re
import subprocess
import sys
from datetime import UTC, datetime, timedelta
from pathlib import Path
from typing import Any

ROOT = Path(__file__).resolve().parent.parent
TITLE = re.compile(r"^(?P<type>\w+)(?:\((?P<scope>[^)]*)\))?(?P<bang>!)?:\s*(?P<desc>.+)$")
BREAKING = re.compile(r"^BREAKING[ -]CHANGE:", re.M)  # a footer starts its line; a mention in prose is not one
SECTIONS = [("breaking", "Breaking changes"), ("feat", "Features"), ("fix", "Fixes"), ("other", "Other changes")]
RELEASABLE = {"breaking", "feat", "fix"}
FIRST = (0, 1, 0)
YANK_MARK = "(yanked)"  # yank.yml appends it to a beta's release title
Version = tuple[int, int, int]


def git(*args: str) -> str:
    return subprocess.run(["git", *args], cwd=ROOT, check=True, capture_output=True, text=True).stdout


def apps() -> list[str]:
    return sorted(p.parent.name for p in (ROOT / "apps").glob("*/Makefile"))


def parse(app: str, tag: str) -> Version | None:
    m = re.fullmatch(rf"{re.escape(app)}-v(\d+)\.(\d+)\.(\d+)", tag)
    return (int(m[1]), int(m[2]), int(m[3])) if m else None


def newest(app: str, tags: list[str]) -> tuple[str, Version] | None:
    found = [(tag, v) for tag in tags if (v := parse(app, tag))]
    return max(found, key=lambda tv: tv[1], default=None)


def last_tag(app: str, ref: str) -> tuple[str, Version] | None:
    return newest(app, git("tag", "--merged", ref, "--list", f"{app}-v*").split())


def commits(app: str, since: str | None, ref: str) -> list[tuple[str, str, str]]:
    rng = f"{since}..{ref}" if since else ref
    out = git("log", "--no-merges", "--format=%H%x1f%s%x1f%b%x1e", rng, "--", f"apps/{app}/", "platform/")
    return [tuple(c.strip("\n").split("\x1f")) for c in out.split("\x1e") if c.strip()]  # type: ignore[misc]


def classify(subject: str, body: str) -> tuple[str, str]:
    """(kind, text) where kind is breaking | feat | fix | other."""
    m = TITLE.match(subject)
    if not m:
        return "other", subject
    if m["bang"] or BREAKING.search(body):
        return "breaking", subject
    if m["type"] == "feat":
        return "feat", subject
    if m["type"] in ("fix", "perf"):
        return "fix", subject
    return "other", subject


def notes(app: str, version: str, intro: str, items: list[tuple[str, str]]) -> str:
    lines = [intro]
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
    return "\n".join(lines)


def plan(ref: str) -> list[dict[str, str]]:
    out = []
    for app in apps():
        prev = last_tag(app, ref)
        items = [classify(s, b) for _, s, b in commits(app, prev[0] if prev else None, ref)]
        if not {k for k, _ in items} & RELEASABLE:
            continue
        new = (prev[1][0], prev[1][1], prev[1][2] + 1) if prev else FIRST
        version = ".".join(map(str, new))
        intro = f"Changes to `apps/{app}` since {prev[0] if prev else 'the start'}."
        out.append({"app": app, "version": version, "tag": f"{app}-v{version}", "notes": notes(app, version, intro, items)})
    return out


def commit_of(tag: str) -> str:
    return git("rev-parse", f"{tag}^{{commit}}").strip()


def is_yanked(release: dict[str, Any]) -> bool:
    return YANK_MARK in (release.get("name") or "").lower()


def pick_source(app: str, releases: list[dict[str, Any]], stable: tuple[str, Version] | None, min_age: timedelta, now: datetime) -> str | None:
    def eligible(r: dict[str, Any]) -> bool:
        v = parse(app, r["tagName"])  # None for the rolling <app>-beta pointer
        return r["isPrerelease"] and v is not None and (stable is None or v > stable[1]) and not is_yanked(r) and now - datetime.fromisoformat(r["createdAt"]) >= min_age

    best = newest(app, [r["tagName"] for r in releases if eligible(r)])
    return best[0] if best else None


def promotion(app: str, releases: list[dict[str, Any]], tag: str | None, min_age: timedelta, now: datetime) -> dict[str, str] | None:
    """The stable release to cut, or None when no beta qualifies or stable already runs that code. Raises ValueError for a --tag that is not a release."""
    if tag and not (parse(app, tag) and any(r["tagName"] == tag for r in releases)):
        raise ValueError(f"{tag} is not a published {app}-vX.Y.Z release")
    stable = newest(app, [r["tagName"] for r in releases if not r["isPrerelease"]])
    source = tag or pick_source(app, releases, stable, min_age, now)
    if source is None or (stable and commit_of(stable[0]) == commit_of(source)):
        return None  # nothing qualifies, or stable already is this code (say, a re-run after publishing)
    top = newest(app, git("tag", "--list", f"{app}-v*").split())
    assert top, "a source tag exists, so the app has tags"
    version = f"{top[1][0]}.{top[1][1] + 1}.0"
    items = [classify(s, b) for _, s, b in commits(app, stable[0] if stable else None, source)]
    intro = (
        f"Stable release of {source}, rebuilt from the same commit under a new version. "
        f"Changes to `apps/{app}` since {f'the previous stable, {stable[0]}' if stable else 'the start'}."
    )
    if not items:
        intro += " None: this rolls stable back to older code."
    return {"app": app, "source": source, "version": version, "tag": f"{app}-v{version}", "notes": notes(app, version, intro, items)}


def main() -> None:
    ap = argparse.ArgumentParser(description=__doc__, formatter_class=argparse.RawDescriptionHelpFormatter)
    ap.add_argument("command", choices=["plan", "promote"])
    ap.add_argument("--ref", default="HEAD")
    ap.add_argument("--app", default="porthole")
    ap.add_argument("--tag", default="", help="promote: this tag, whatever its age (empty: pick a beta)")
    ap.add_argument("--min-age-hours", type=float, default=72)
    ap.add_argument("--notes", default="notes.md", help="promote: where to write the release notes")
    args = ap.parse_args()
    if args.command == "plan":
        print(json.dumps(plan(args.ref)))
        return
    try:
        p = promotion(args.app, json.load(sys.stdin), args.tag or None, timedelta(hours=args.min_age_hours), datetime.now(UTC))
    except ValueError as e:
        sys.exit(f"error: {e}")
    if p:  # GITHUB_OUTPUT lines; nothing at all when there is nothing to promote
        Path(args.notes).write_text(p["notes"] + "\n")
        print(f"source={p['source']}\nversion={p['version']}\ntag={p['tag']}")


if __name__ == "__main__":
    main()
