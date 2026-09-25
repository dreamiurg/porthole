#!/usr/bin/env python3
"""Build the web installer site (GitHub Pages) from each app's latest release.

    python3 tools/build_site.py [--out build/site]

Needs `npm ci` in site/ first (for esp-web-tools) and an authenticated `gh` (to download
release assets). For every apps/<app>/app.json it finds the newest <app>-vX.Y.Z tag,
downloads that release's <app>-<version>-web.zip (the separate flash parts written by
`make factory`), and writes <out>/<app>/manifest.json for ESP Web Tools. Apps listing
"play" files in app.json also get <out>/<app>/play/, extracted from the same tag, so
the browser version matches the firmware. Apps without a release yet are listed as
coming soon.
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

CARD = """    <section class="app" id="{app}">
      <img src="{app}/preview.png" alt="{name} screenshots" width="1340" height="320">
      <h3>{name}</h3>
      <div class="version">{version}</div>
      <p>{tagline}</p>
      <div class="actions">{actions}
      </div>
    </section>"""

INSTALL = """
        <esp-web-install-button manifest="{app}/manifest.json">
          <button slot="activate" class="button">Install {name}</button>
          <span slot="unsupported" class="unsupported">Open this page in Chrome or Edge on a computer to install.</span>
          <span slot="not-allowed" class="unsupported">Open this page with https:// at the start of the address to install.</span>
        </esp-web-install-button>"""

PLAY = """
        <a class="button secondary" href="{app}/play/">Play in your browser</a>"""


def build_app(app: str, out: Path) -> str:
    meta = json.loads((ROOT / "apps" / app / "app.json").read_text())
    name, tagline = meta["name"], meta["tagline"]
    dest = out / app
    dest.mkdir(parents=True)
    shutil.copy(ROOT / "apps" / app / "docs" / "preview.png", dest / "preview.png")
    tag = release.last_tag(app, "HEAD")
    if tag is None:
        return CARD.format(app=app, name=html.escape(name), version="Coming soon", tagline=html.escape(tagline), actions="")
    version = ".".join(map(str, tag[1]))
    with tempfile.TemporaryDirectory() as tmp:
        subprocess.run(["gh", "release", "download", tag[0], "-p", f"{app}-{version}-web.zip", "-D", tmp], cwd=ROOT, check=True)
        with zipfile.ZipFile(Path(tmp) / f"{app}-{version}-web.zip") as z:
            z.extractall(dest)
    parts = json.loads((dest / "parts.json").read_text())
    manifest = {
        "name": name,
        "version": version,
        "new_install_prompt_erase": True,  # updates keep saves unless the user asks to erase
        "new_install_improv_wait_time": 0,  # these firmwares do not speak Improv Wi-Fi
        "builds": [{"chipFamily": "ESP32-S3", "serialType": "uart", "parts": parts}],
    }
    (dest / "manifest.json").write_text(json.dumps(manifest, indent=2))
    actions = INSTALL.format(app=app, name=html.escape(name))
    if meta.get("play"):
        paths = [f"apps/{app}/{p}" for p in meta["play"]]
        archive = subprocess.run(["git", "archive", "--format=tar", tag[0], *paths], cwd=ROOT, check=True, capture_output=True).stdout
        with tarfile.open(fileobj=io.BytesIO(archive)) as t:
            for member in t.getmembers():
                if member.isfile():
                    target = dest / "play" / Path(member.name).relative_to(f"apps/{app}")
                    target.parent.mkdir(parents=True, exist_ok=True)
                    source = t.extractfile(member)
                    assert source is not None
                    target.write_bytes(source.read())
        actions += PLAY.format(app=app)
    return CARD.format(app=app, name=html.escape(name), version=f"Version {version}", tagline=html.escape(tagline), actions=actions)


def main() -> None:
    ap = argparse.ArgumentParser(description=__doc__, formatter_class=argparse.RawDescriptionHelpFormatter)
    ap.add_argument("--out", default="build/site")
    out = (ROOT / ap.parse_args().out).resolve()
    vendor = ROOT / "site" / "node_modules" / "esp-web-tools" / "dist" / "web"
    if not vendor.is_dir():
        raise SystemExit("esp-web-tools missing: run `npm ci` in site/ first")
    shutil.rmtree(out, ignore_errors=True)
    shutil.copytree(vendor, out / "vendor" / "esp-web-tools")
    cards = [build_app(app, out) for app in release.apps() if (ROOT / "apps" / app / "app.json").exists()]
    (out / "index.html").write_text((ROOT / "site" / "index.html").read_text().replace("{{APPS}}", "\n".join(cards)))
    (out / ".nojekyll").write_text("")
    print(f"site: {out} ({len(cards)} apps)")


if __name__ == "__main__":
    main()
