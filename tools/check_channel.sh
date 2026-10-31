#!/usr/bin/env bash
# Resolve an update channel the way a board does, and fail unless it leads to a real update image:
#
#   tools/check_channel.sh <app> stable|beta [expected-version]
#
# stable: releases/latest/download/<app>-version.txt. beta: the rolling <app>-beta pre-release's copy. Then the image
# the board would download, releases/download/<app>-v<ver>/<app>-<ver>-ota.bin, must exist and not be empty. The
# release and promote workflows run it after they publish; run it by hand to check what boards see right now.
set -euo pipefail

app=$1 channel=$2 expect=${3:-}
base=https://github.com/${GITHUB_REPOSITORY:-dreamiurg/porthole}/releases
case $channel in
  stable) version_url=$base/latest/download/$app-version.txt ;;
  beta) version_url=$base/download/$app-beta/$app-version.txt ;;
  *) echo "usage: $0 <app> stable|beta [expected-version]" >&2; exit 2 ;;
esac

# GitHub's release CDN can lag a little behind an edit: retry before calling it broken.
for try in 1 2 3 4 5 6; do
  version=$(curl -fsSL "$version_url" 2>/dev/null || true)
  if [[ $version =~ ^[0-9]+\.[0-9]+\.[0-9]+$ ]] && [[ -z $expect || $version == "$expect" ]]; then break; fi
  [[ $try == 6 ]] && { echo "::error::$channel: $version_url gave '$version' (want ${expect:-X.Y.Z})"; exit 1; }
  sleep 10
done

image=$base/download/$app-v$version/$app-$version-ota.bin
size=$(curl -fsSIL "$image" | tr -d '\r' | awk 'tolower($1) == "content-length:" { n = $2 } END { print n + 0 }')
if (( size == 0 )); then echo "::error::$channel: $image is missing or empty"; exit 1; fi
echo "$channel -> $version, $image ($size bytes)"
