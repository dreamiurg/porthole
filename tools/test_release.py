#!/usr/bin/env python3
"""Self-check for tools/release.py against a throwaway git repo. Run: python3 tools/test_release.py"""

import importlib.util
import subprocess
import tempfile
from pathlib import Path

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

    commit(repo, "apps/alpha/main.cpp", "feat!: new save format")
    assert versions(repo)["alpha"] == "0.2.0", "breaking below 1.0 bumps minor"
    run(repo, "git", "tag", "alpha-v1.4.2")
    commit(repo, "apps/alpha/main.cpp", "refactor: tidy\n\nBREAKING CHANGE: renamed a setting")
    assert versions(repo)["alpha"] == "2.0.0", "breaking footer at or above 1.0 bumps major"

    notes = release.plan("HEAD")[0]["notes"]
    assert "### Breaking changes" in notes and "alpha-1.4.2" not in notes and "alpha-2.0.0-factory.bin" in notes, notes

print("release.py: all checks passed")
