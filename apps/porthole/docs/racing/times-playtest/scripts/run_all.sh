#!/bin/sh
# Replays every tm_ script against a snap built from a pinned checkout.
# usage: run_all.sh <pinned apps/porthole dir> <evidence dir>
set -e
WA=$1; D=$2
cd "$WA"
rm -f build/host/tm_*.bmp
for f in "$D"/scripts/s*.txt; do
  ./build/host/snap --script "$f" > "$D/scripts/$(basename "$f" .txt).out" 2>&1
done
