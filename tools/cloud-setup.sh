#!/bin/bash
# Setup script for a Claude Code cloud environment (claude.ai/code > environment > Setup script > Choose file).
# Runs as root on Ubuntu 24.04 before the session starts; the result is snapshotted and reused for ~7 days, so it
# installs everything the hooks and CI need: the session's SessionStart hook (.claude/settings.json) only runs
# `make hooks`. Reads pinned files from main on GitHub, because the environment is shared by every branch.
# Network: Custom, with the default list, plus the PlatformIO registry (*.platformio.org) and *.espressif.com.
set -u
raw=https://raw.githubusercontent.com/dreamiurg/porthole/main
export UV_TOOL_BIN_DIR=/usr/local/bin
tmp=$(mktemp -d)

# System packages: zlib for the host builds, SDL2 for the simulator, shellcheck for actionlint.
apt-get update -qq && apt-get install -y -qq zlib1g-dev libsdl2-dev shellcheck >/dev/null || echo "apt install failed"

# Python modules `make -C apps/porthole check/ci` imports directly (same set as CI's porthole job).
python3 -m pip install -q --break-system-packages pillow fonttools==4.61.1 'gcovr>=8.3' lizard==1.21.0 || echo "pip install failed"

# pre-commit, plus every hook environment, so the first commit in a session doesn't build them.
(uv tool install -q pre-commit \
  && git init -q "$tmp/hooks" && curl -fsSL "$raw/.pre-commit-config.yaml" -o "$tmp/hooks/.pre-commit-config.yaml" \
  && cd "$tmp/hooks" && pre-commit install-hooks) || echo "pre-commit setup failed" &

# PlatformIO in its own venv (Core 6.1 needs click<8.2), plus the ESP32 platform and toolchains for the
# pre-push firmware build: install the project's packages from its two config files, no sources needed.
(curl -fsSL "$raw/platform/requirements.txt" -o "$tmp/pio-req.txt" \
  && uv tool install -q "$(grep -E '^platformio==' "$tmp/pio-req.txt")" --with-requirements "$tmp/pio-req.txt" \
  && mkdir -p "$tmp/pio/apps/porthole" "$tmp/pio/platform" \
  && curl -fsSL "$raw/apps/porthole/platformio.ini" -o "$tmp/pio/apps/porthole/platformio.ini" \
  && curl -fsSL "$raw/platform/waveshare-round.ini" -o "$tmp/pio/platform/waveshare-round.ini" \
  && pio pkg install -d "$tmp/pio/apps/porthole" >/dev/null) || echo "PlatformIO setup failed" &

wait
rm -rf "$tmp"
exit 0
