#!/usr/bin/env python3
"""Build the web installer site (GitHub Pages) from each app's latest release.

    python3 tools/build_site.py [--out build/site]

Needs `npm ci` in site/ first (for esp-web-tools) and an authenticated `gh` (to download
release assets). For every apps/<app>/app.json it finds the newest <app>-vX.Y.Z tag,
downloads that release's <app>-<version>-web.zip (the separate flash parts written by
`make factory`), and writes <out>/<app>/manifest.json for ESP Web Tools. Apps with
"play" in app.json also get <out>/<app>/play/ from the release's <app>-<version>-play.zip
(`make play`), or for older releases from the static files the list names, taken from
the same tag. "embed": true marks a play page that shows only the screen, filling its
frame, when opened with ?embed; the first such app fills the round window at the top.
Apps without a release yet are listed as coming soon.
"""

import argparse
import html
import importlib.util
import io
import json
import shutil
import subprocess
import tarfile
import tempfile
import zipfile
from pathlib import Path

ROOT = Path(__file__).resolve().parent.parent
_spec = importlib.util.spec_from_file_location("release", ROOT / "tools" / "release.py")
assert _spec and _spec.loader
release = importlib.util.module_from_spec(_spec)
_spec.loader.exec_module(release)

GAME = """    <article class="app" id="{app}">
      <img src="{app}/preview.png" alt="{name}" width="1340" height="320" loading="lazy">
      <h3>{name}</h3>
      <div class="version">{version}</div>
      <p><span lang="en">{tagline}</span><span lang="uk">{tagline_uk}</span></p>
      <div class="actions">{actions}
      </div>
    </article>"""

INSTALL = """
        <esp-web-install-button manifest="{app}/manifest.json">
          <button slot="activate">Install</button>
          <span slot="unsupported" class="small"><span lang="en">To install, open this page in Chrome or Edge on a computer.</span>
            <span lang="uk">Щоб записати гру, відкрийте сторінку в Chrome або Edge на комп&rsquo;ютері.</span></span>
          <span slot="not-allowed" class="small"><span lang="en">To install, open this page with https:// at the start of the address.</span>
            <span lang="uk">Щоб записати гру, відкрийте сторінку з https:// на початку адреси.</span></span>
        </esp-web-install-button>"""

PLAY = """
        <a href="{app}/play/"><span lang="en">Play in the browser</span><span lang="uk">Грати в браузері</span></a>"""

WINDOW_LIVE = """<div class="window" data-src="{app}/play/?embed">
      <iframe class="window-screen" title="{name}" tabindex="-1"></iframe>
      <button class="window-insert" type="button" style="--shot: url('{app}/preview.png')">
        <span class="window-press"><span lang="en">Tap to play {name}</span><span lang="uk">Натисніть, щоб грати</span></span>
      </button>
    </div>"""

WINDOW_LINK = """<div class="window">
      <a class="window-insert" href="{href}" style="--shot: url('{app}/preview.png')">
        <span class="window-press">{label}</span>
      </a>
    </div>"""


def fetch(tag: str, pattern: str, dest: Path) -> bool:
    """Download one release asset into dest and unzip it there; False when the release lacks it."""
    with tempfile.TemporaryDirectory() as tmp:
        got = subprocess.run(["gh", "release", "download", tag, "-p", pattern, "-D", tmp], cwd=ROOT, capture_output=True)
        files = list(Path(tmp).glob("*.zip"))
        if got.returncode != 0 or not files:
            return False
        with zipfile.ZipFile(files[0]) as z:
            z.extractall(dest)
    return True


def play_from_tag(app: str, tag: str, paths: list[str], dest: Path) -> None:
    """Older releases without a play.zip: take the static browser files straight from the tag."""
    archive = subprocess.run(["git", "archive", "--format=tar", tag, *[f"apps/{app}/{p}" for p in paths]], cwd=ROOT, check=True, capture_output=True).stdout
    with tarfile.open(fileobj=io.BytesIO(archive)) as t:
        for member in t.getmembers():
            if member.isfile():
                target = dest / Path(member.name).relative_to(f"apps/{app}")
                target.parent.mkdir(parents=True, exist_ok=True)
                source = t.extractfile(member)
                assert source is not None
                target.write_bytes(source.read())


def build_app(app: str, out: Path) -> dict:
    meta = json.loads((ROOT / "apps" / app / "app.json").read_text())
    name, tagline = meta["name"], meta["tagline"]
    dest = out / app
    dest.mkdir(parents=True)
    shutil.copy(ROOT / "apps" / app / "docs" / "preview.png", dest / "preview.png")
    info = {"app": app, "name": name, "play": False, "embed": False}
    tag = release.last_tag(app, "HEAD")
    actions = ""
    version = '<span lang="en">Coming soon</span><span lang="uk">Скоро</span>'
    if tag is not None:
        v = ".".join(map(str, tag[1]))
        version = f'<span lang="en">Version {v}</span><span lang="uk">Версія {v}</span>'
        if not fetch(tag[0], f"{app}-{v}-web.zip", dest):
            raise SystemExit(f"{tag[0]} has no {app}-{v}-web.zip")
        parts = json.loads((dest / "parts.json").read_text())
        manifest = {
            "name": name,
            "version": v,
            "new_install_prompt_erase": True,  # updates keep saves unless the user asks to erase
            "new_install_improv_wait_time": 0,  # these firmwares do not speak Improv Wi-Fi
            "builds": [{"chipFamily": "ESP32-S3", "serialType": "uart", "parts": parts}],
        }
        (dest / "manifest.json").write_text(json.dumps(manifest, indent=2))
        if meta.get("play"):
            if not fetch(tag[0], f"{app}-{v}-play.zip", dest / "play") and isinstance(meta["play"], list):
                play_from_tag(app, tag[0], meta["play"], dest / "play")
            if (dest / "play" / "index.html").exists():
                info["play"] = True
                info["embed"] = bool(meta.get("embed"))
                actions += PLAY.format(app=app)
        actions += INSTALL.format(app=app)
    info["html"] = GAME.format(
        app=app,
        name=html.escape(name),
        version=version,
        tagline=html.escape(tagline),
        tagline_uk=html.escape(meta.get("tagline_uk", tagline)),
        actions=actions,
    )
    return info


def hero_window(apps: list[dict]) -> str:
    live = next((a for a in apps if a["embed"]), None)
    if live:
        return WINDOW_LIVE.format(app=live["app"], name=html.escape(live["name"]))
    playable = next((a for a in apps if a["play"]), None)
    if playable:
        label = f'<span lang="en">Play {html.escape(playable["name"])}</span><span lang="uk">Грати</span>'
        return WINDOW_LINK.format(app=playable["app"], href=f"{playable['app']}/play/", label=label)
    return WINDOW_LINK.format(app=apps[0]["app"], href=f"#{apps[0]['app']}", label='<span lang="en">See the games</span><span lang="uk">Ігри</span>')


def main() -> None:
    ap = argparse.ArgumentParser(description=__doc__, formatter_class=argparse.RawDescriptionHelpFormatter)
    ap.add_argument("--out", default="build/site")
    out = (ROOT / ap.parse_args().out).resolve()
    vendor = ROOT / "site" / "node_modules" / "esp-web-tools" / "dist" / "web"
    if not vendor.is_dir():
        raise SystemExit("esp-web-tools missing: run `npm ci` in site/ first")
    shutil.rmtree(out, ignore_errors=True)
    shutil.copytree(vendor, out / "vendor" / "esp-web-tools")
    shutil.copytree(ROOT / "site" / "assets", out / "assets")
    names = [app for app in release.apps() if (ROOT / "apps" / app / "app.json").exists()]
    apps = [build_app(app, out) for app in names]
    page = (ROOT / "site" / "index.html").read_text()
    page = page.replace("{{WINDOW}}", hero_window(apps)).replace("{{GAMES}}", "\n".join(a["html"] for a in apps))
    (out / "index.html").write_text(page)
    (out / ".nojekyll").write_text("")
    print(f"site: {out} ({len(apps)} apps)")


if __name__ == "__main__":
    main()
