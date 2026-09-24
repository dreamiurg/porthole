#!/usr/bin/env bash
set -euo pipefail

firmware_dir="$(cd "$(dirname "$0")/.." && pwd)"
lvgl_dir="$firmware_dir/.pio/libdeps/firmware/lvgl"
if [[ ! -f "$lvgl_dir/src/misc/lv_txt.c" ]]; then
  echo 'LVGL is missing: build the firmware once first (make firmware) to install PlatformIO dependencies.' >&2
  exit 1
fi
node "$firmware_dir/tools/export-assets.mjs"
build_dir="$(mktemp -d "${TMPDIR:-/tmp}/biscuit-layout.XXXXXX")"
trap 'rm -rf "$build_dir"' EXIT

# Test the actual firmware paginator, so a change to its font or page budget is
# exercised without maintaining a second implementation in the test.
python3 - "$firmware_dir/src/main.cpp" "$build_dir/paginate.inc" <<'PY'
import pathlib, re, sys
source = pathlib.Path(sys.argv[1]).read_text()
match = re.search(r'static std::vector<std::string> paginate\(const char\* text\) \{.*?\n\}', source, re.S)
if not match:
    raise SystemExit('Could not find the firmware paginator; update the layout harness.')
pathlib.Path(sys.argv[2]).write_text(match.group() + '\n')
PY

includes=(-DLV_CONF_INCLUDE_SIMPLE -I"$firmware_dir/include" -I"$firmware_dir/generated" -I"$lvgl_dir" -I"$build_dir")
objects=()
for relative in misc/lv_txt.c misc/lv_mem.c misc/lv_gc.c misc/lv_utils.c misc/lv_printf.c font/lv_font.c font/lv_font_fmt_txt.c; do
  object="$build_dir/$(basename "$relative" .c).o"
  "${CC:-clang}" -std=c11 -O1 "${includes[@]}" -c "$lvgl_dir/src/$relative" -o "$object"
  objects+=("$object")
done
for size in 16 20 24 28; do
  object="$build_dir/font$size.o"
  "${CC:-clang}" -std=c11 -O1 "${includes[@]}" -c "$firmware_dir/generated/font$size.c" -o "$object"
  objects+=("$object")
done
"${CXX:-clang++}" -std=c++17 -O1 -Wall -Wextra -Werror "${includes[@]}" \
  "$firmware_dir/tests/layout_test.cpp" "$firmware_dir/generated/assets.cpp" \
  "${objects[@]}" -o "$build_dir/layout-test"
"$build_dir/layout-test"
