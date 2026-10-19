# Adult menu font

Atkinson Hyperlegible Regular is designed to distinguish similar characters and
keep counters open. The adult system menu uses native-size, anti-aliased glyphs:
24 px for body text and controls, 28 px for headings, and 32 px for PIN digits and
keyboard previews. Game and child-facing shell fonts are independent.

The bundled TTF comes from
[Google Fonts](https://github.com/google/fonts/tree/main/ofl/atkinsonhyperlegible).
The committed TTF has SHA-256
`7fb917c89019896d0b52ee84b7cbb3304c18cb90b19a62f5e32712bd23e97669`.

Copyright 2020 Braille Institute of America, Inc.; licensed under the
[SIL Open Font License 1.1](OFL.txt). Firmware uses the generated tables directly;
it does not load a font file or download anything at runtime.

To regenerate `shell/adult_fonts.h` from the committed TTF, run from
`apps/porthole/` with Node and Python installed:

```sh
mkdir -p build
for n in 24 28 32; do
  npx --yes lv_font_conv@1.5.3 \
    --font shell/fonts/AtkinsonHyperlegible-Regular.ttf \
    --range 0x20-0x7e --size "$n" --bpp 4 --format lvgl --no-compress \
    --force-fast-kern-format --lv-include lvgl.h \
    --lv-font-name "adult_font_$n" --output "build/adult$n.c"
done
python3 tools/fontconv.py --game adult build/adult24.c build/adult28.c build/adult32.c
```

The tables cover printable ASCII, matching the system menu's text-entry contract.
