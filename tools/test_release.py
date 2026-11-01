#!/usr/bin/env python3
"""Self-check for tools/release.py against a throwaway git repo. Run: python3 tools/test_release.py"""

import importlib.util
import os
import subprocess
import tempfile
from datetime import UTC, datetime, timedelta
from pathlib import Path

# Run from a git hook, GIT_DIR / GIT_INDEX_FILE point at the real repository; left in place, the throwaway
# repo's `git init`, `config` and `add` would write to the real index and config. Drop them all.
for key in [k for k in os.environ if k.startswith("GIT_")]:
    del os.environ[key]

spec = importlib.util.spec_from_file_location("release", Path(__file__).with_name("release.py"))
assert spec and spec.loader
release = importlib.util.module_from_spec(spec)
spec.loader.exec_module(release)


def run(repo: Path, *args: str) -> None:
    subprocess.run(args, cwd=repo, check=True, capture_output=True)


def commit(repo: Path, path: str, msg: str) -> None:
    f = repo / path
    f.parent.mkdir(parents=True, exist_ok=True)
    f.write_text(f.read_text() + "x" if f.exists() else "x")
    run(repo, "git", "add", "-A")
    run(repo, "git", "commit", "-q", "-m", msg)


def versions(repo: Path) -> dict[str, str]:
    setattr(release, "ROOT", repo)  # noqa: B010  (module loaded by path, mypy cannot see ROOT)
    return {p["app"]: p["version"] for p in release.plan("HEAD")}


with tempfile.TemporaryDirectory() as tmp:
    repo = Path(tmp)
    run(repo, "git", "init", "-q", "-b", "main")
    run(repo, "git", "config", "user.email", "t@example.com")
    run(repo, "git", "config", "user.name", "t")
    for app in ("alpha", "beta"):
        commit(repo, f"apps/{app}/Makefile", f"chore: scaffold {app}")
    assert versions(repo) == {}, "chore-only history must not release"

    commit(repo, "apps/alpha/main.cpp", "feat(alpha): first light")
    assert versions(repo) == {"alpha": "0.1.0"}, versions(repo)
    run(repo, "git", "tag", "alpha-v0.1.0")
    assert versions(repo) == {}, "nothing new since the tag"

    commit(repo, "apps/alpha/main.cpp", "fix: off by one")
    commit(repo, "apps/beta/README.md", "docs: typo")
    assert versions(repo) == {"alpha": "0.1.1"}, versions(repo)

    commit(repo, "platform/board.ini", "fix: shared flash setting")
    assert versions(repo) == {"alpha": "0.1.1", "beta": "0.1.0"}, "platform/ changes release every app"

    commit(repo, "apps/alpha/main.cpp", "feat: a new screen")
    assert versions(repo)["alpha"] == "0.1.1", "every beta is a patch, features too"
    commit(repo, "apps/alpha/main.cpp", "refactor: tidy\n\nBREAKING CHANGE: renamed a setting")
    assert versions(repo)["alpha"] == "0.1.1", "breaking changes too"

    notes = release.plan("HEAD")[0]["notes"]
    assert all(h in notes for h in ("### Breaking changes", "### Features", "### Fixes")), notes
    assert "alpha-0.1.1-factory.bin" in notes, notes

    commit(repo, "apps/alpha/main.cpp", "fix: wording\n\nThis is not a BREAKING CHANGE: just prose.")
    assert release.classify("fix: wording", "This is not a BREAKING CHANGE: just prose.")[0] == "fix", "mid-line is prose"
    assert release.classify("fix: x", "BREAKING-CHANGE: hyphenated")[0] == "breaking", "the hyphenated footer counts"
    assert release.classify("feat!: x", "")[0] == "breaking"

    # Promotion. Betas 0.1.1 .. 0.1.3, stable 0.1.0; "now" is 2026-11-10 12:00 UTC.
    run(repo, "git", "tag", "alpha-v0.1.1")
    for n in (2, 3):
        commit(repo, "apps/alpha/main.cpp", f"fix: patch {n}")
        run(repo, "git", "tag", f"alpha-v0.1.{n}")
    now = datetime(2026, 11, 10, 12, tzinfo=UTC)
    three_days = timedelta(hours=72)

    def rel(tag: str, days_old: float, pre: bool = True, name: str = "") -> dict[str, object]:
        created = (now - timedelta(days=days_old)).isoformat().replace("+00:00", "Z")
        return {"tagName": tag, "name": name or tag, "isPrerelease": pre, "createdAt": created}

    def promote(releases: list[dict[str, object]], tag: str | None = None, min_age: timedelta = three_days) -> dict[str, str] | None:
        return release.promotion("alpha", releases, tag, min_age, now)

    releases = [rel("alpha-v0.1.0", 9, pre=False), rel("alpha-v0.1.1", 5), rel("alpha-v0.1.2", 3.5), rel("alpha-v0.1.3", 1), rel("alpha-beta", 1, name="alpha beta channel")]
    p = promote(releases)
    assert p and p["source"] == "alpha-v0.1.2" and p["tag"] == "alpha-v0.2.0", "newest beta at least three days old -> next minor"
    assert "patch 2" in p["notes"] and "patch 3" not in p["notes"] and "the previous stable, alpha-v0.1.0" in p["notes"], p["notes"]
    assert "alpha-0.2.0-factory.bin" in p["notes"], p["notes"]

    releases[2] = rel("alpha-v0.1.2", 3.5, name="alpha 0.1.2 (yanked)")
    assert (p := promote(releases)) and p["source"] == "alpha-v0.1.1", "a yanked beta is skipped"
    assert (p := promote(releases, min_age=timedelta(0))) and p["source"] == "alpha-v0.1.3", "promote now: the newest beta"
    assert (p := promote(releases, tag="alpha-v0.1.2")) and p["source"] == "alpha-v0.1.2", "an explicit tag wins, yanked or young"
    assert promote(releases, tag="other-v1.0.0") is None, "another app's tag"

    releases += [rel("alpha-v0.2.0", 2, pre=False)]  # 0.1.2 went stable as 0.2.0; the next beta is 0.2.1
    assert promote(releases) is None, "every beta is older than stable"
    run(repo, "git", "tag", "alpha-v0.2.0", "alpha-v0.1.2")
    commit(repo, "apps/alpha/main.cpp", "fix: after stable")
    assert versions(repo)["alpha"] == "0.2.1", versions(repo)

    p = promote(releases, tag="alpha-v0.1.1")
    assert p and p["tag"] == "alpha-v0.3.0" and "rolls stable back" in p["notes"], "a rollback is a higher version"
    assert release.promotion("alpha", [rel("alpha-v0.1.3", 1)], None, three_days, now) is None, "nothing old enough"

print("release.py: all checks passed")
