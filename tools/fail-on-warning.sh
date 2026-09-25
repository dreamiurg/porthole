#!/usr/bin/env bash
# Warnings are errors. For tools that print warnings but exit 0 and have no switch to fail on them
# (gcovr, PlatformIO's own "Warning!" lines): run the command, show its output, and fail if the
# command fails or any output line matches PATTERN (extended regex).
#   usage: tools/fail-on-warning.sh PATTERN command [args...]
set -uo pipefail
pattern=$1
shift
log=$(mktemp)
trap 'rm -f "$log"' EXIT
"$@" 2>&1 | tee "$log"
status=${PIPESTATUS[0]}
[ "$status" -eq 0 ] || exit "$status"
if grep -Eq -- "$pattern" "$log"; then
  echo "fail-on-warning: output matched /$pattern/; warnings are errors, fix the cause" >&2
  exit 1
fi
