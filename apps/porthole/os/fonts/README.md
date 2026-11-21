# Ukrainian text font

Andika Bold is SIL's typeface for children who are learning to read: single-storey a, clear
letter shapes and wide spacing. Every game draws the Ukrainian text of its exercises (words,
syllables, short reads) in Andika through `os/uk_fonts.h`, at 24, 48 and 88 px, while its own
frame, buttons and English labels keep the game's typography.

The bundled TTF comes from [Google Fonts](https://github.com/google/fonts/tree/main/ofl/andika).
The committed TTF has SHA-256
`4361126189a2d0e0da66999352db58aaf1b4ca940faf8a8239d725f8303d890d`.

Copyright (c) 2004-2022 SIL International; see [OFL.txt](OFL.txt). Firmware uses the generated
tables directly; it does not load a font file or download anything at runtime.

The tables cover printable ASCII plus the Ukrainian alphabet (А–Я without Ё Ы Э Ъ, with Ґ Є І Ї)
and both apostrophes (U+02BC, U+2019). To regenerate `os/uk_fonts.h`, run from `apps/porthole/`
with Node and Python installed:

```sh
mkdir -p build
R=0x20-0x7e,0x2bc,0x404,0x406-0x407,0x410-0x429,0x42c,0x42e-0x449,0x44c,0x44e-0x44f,0x454,0x456-0x457,0x490-0x491,0x2019
for n in 24 48 88; do
  npx --yes lv_font_conv@1.5.3 --font os/fonts/Andika-Bold.ttf --range "$R" --size "$n" --bpp 4 \
    --format lvgl --no-compress --force-fast-kern-format --lv-include lvgl.h \
    --lv-font-name "uk_font_$n" --output "build/uk$n.c"
done
python3 tools/fontconv.py --game uk build/uk24.c build/uk48.c build/uk88.c
```
