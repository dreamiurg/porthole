# Pinned ESP-IDF RGB panel driver

These sources are from Espressif ESP-IDF commit
[`858a988d6eb90f54661abe282c523879c6ad0116`](https://github.com/espressif/esp-idf/tree/858a988d6eb90f54661abe282c523879c6ad0116),
the IDF revision in Porthole's pinned pioarduino 54.03.21 framework. The original
SPDX/copyright headers and Apache-2.0 license are retained. The two headers are
unchanged; they are private companion headers absent from the precompiled SDK.

| Local file | Original source | Original SHA-256 |
| --- | --- | --- |
| `esp_lcd_panel_rgb.c` | [components/esp_lcd/rgb/esp_lcd_panel_rgb.c](https://github.com/espressif/esp-idf/blob/858a988d6eb90f54661abe282c523879c6ad0116/components/esp_lcd/rgb/esp_lcd_panel_rgb.c) | `4775dcbbe07af12a4f3c6e55532b98d36f583ccc91b5944526c857bf90c73c17` |
| `esp_lcd_common.h` | [components/esp_lcd/priv_include/esp_lcd_common.h](https://github.com/espressif/esp-idf/blob/858a988d6eb90f54661abe282c523879c6ad0116/components/esp_lcd/priv_include/esp_lcd_common.h) | `bbff1d2b72f1f6b255112557a45fef28ce3be5d652541f1203fdc403225b085c` |
| `rgb_lcd_rotation_sw.h` | [components/esp_lcd/rgb/rgb_lcd_rotation_sw.h](https://github.com/espressif/esp-idf/blob/858a988d6eb90f54661abe282c523879c6ad0116/components/esp_lcd/rgb/rgb_lcd_rotation_sw.h) | `c623989c33f0d43015cd215f79490b95850f02212dece6e0941a23a591bcdf9b` |

Only two local changes are applied to the C source:

1. Immediately after `sdkconfig.h`, override `CONFIG_LCD_RGB_RESTART_IN_VSYNC`
   to zero for this translation unit. The driver's existing recovery branch
   restarts DMA on an explicit request or when fewer bounce EOFs than expected
   arrive in a frame. Healthy frames no longer suffer a forced DMA reset that
   can shift the image when its VSYNC interrupt arrives late. The recovery
   branch already resets `bb_eof_count` under the driver's spinlock each VSYNC.
2. Reset `bb_eof_count` alongside `bounce_pos_px` when starting transmission,
   backporting that startup hunk from upstream
   [`c0fb05b37e19fafab249d15e98dd161d9a70a593`](https://github.com/espressif/esp-idf/commit/c0fb05b37e19fafab249d15e98dd161d9a70a593).
   A restarted transfer begins with bounce buffer zero, so the software EOF
   index must start there too. The upstream commit's other hunk moves the
   VSYNC counter reset outside the forced-restart conditional; the local
   override already selects the branch containing that reset.

The existing PlatformIO `+<firmware/>` source filter compiles this C file.
Its public RGB-driver symbols resolve before the SDK's `libesp_lcd.a`, so the
SDK RGB object is not linked. All other LCD services, HAL code and public
headers still come from the pinned SDK; no public API or private struct layout
is copied into application code. Recheck that symbol selection and the pin
when updating the framework.

Normal presentation uses internal bounce buffers and acknowledges framebuffer
reuse at the copy-complete callback. Explicit OTA progress temporarily takes
over the same DMA channel with its independent internal-SRAM picture; the
normal driver resumes through `esp_lcd_panel_init` afterward.

`make check` verifies the original source hashes after reversing these two exact patches, the companion-header hashes, and the framework pin (`tools/check_rgb_driver.py`). This also guards the narrow vendor exception below.

The root complexity command excludes this exact vendor directory because its
functions retain upstream structure. Application display code remains subject
to the normal complexity limits. A firmware build verifies SDK compatibility;
display stability still needs a physical board check.
