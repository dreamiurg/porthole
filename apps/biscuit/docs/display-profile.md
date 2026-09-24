# Display constraints

The preview preserves the preferred screen layout while using a conservative hardware profile. It is not a CPU emulator or proof of achieved device performance.

The [Waveshare documentation](https://docs.waveshare.com/ESP32-S3-Touch-LCD-2.1) specifies 480×480 pixels, a 2.1-inch display, 8MB PSRAM, and 16MB flash. Its [FAQ](https://docs.waveshare.com/ESP32-S3-Touch-LCD-2.1/FAQ) specifies RGB565 and single touch. The advertised 262K-color panel does not mean the documented driver uses that full palette.

- Fixed 480×480 logical screen, with 160×160 original pixel art at exactly 3× scale. Discovery illustrations use 96×48 static pixel canvases, displayed at 1× in lists and 3× in the reader. One RGB565 illustration requires 9KiB if stored as a bitmap; the browser draws them from small local routines.
- RGB565 channel values in the sprite and flat UI palette; no gradients or decorative shadows inside the display.
- Bundled Montserrat Regular/SemiBold, also practical to convert to bitmap fonts for LVGL. Story and discovery prose is 24 logical pixels and paginated by measured font width.
- One touch at a time. All game navigation stays on the touchscreen; the case has no buttons.
- Pixel animation updates at most 10fps and pauses behind menus and in hidden tabs. This cap is a design budget, not a measured ESP32 frame rate.
- An optional physical-size view defaults to 202 browser pixels. The slider lets the user match the visible screen diameter to 53.3mm with a ruler. Browser zoom and monitor density make an uncalibrated CSS size unreliable.

One complete RGB565 frame buffer requires 460,800 bytes (450KiB); two require 900KiB, before fonts, game state, and working memory. [Espressif's RGB LCD documentation](https://docs.espressif.com/projects/esp-idf/en/stable/esp32s3/api-reference/peripherals/lcd/rgb_lcd.html) explains the PSRAM/DMA bandwidth constraints that need device testing. The current browser bundles full TTF files; the firmware port should include only required bitmap glyphs at the actual used sizes. [LVGL supports antialiased bitmap fonts](https://docs.lvgl.io/8.3/overview/font.html).

Unverified: exact board revision/variant, brightness, contrast, viewing angles, text rasterization, physical touch accuracy, achievable frame rate, and firmware memory use. Browser antialiasing can introduce intermediate colors; the preview is not a bit-exact LCD framebuffer. Actual-device readability and timing must be checked before claiming hardware parity.
