"""Keep the locally compiled RGB driver pinned to its reviewed upstream source and two patches."""

import hashlib
from pathlib import Path

APP = Path(__file__).resolve().parents[1]
VENDOR = APP / "firmware/vendor/rgb"
HASHES = {
    "esp_lcd_panel_rgb.c": "4775dcbbe07af12a4f3c6e55532b98d36f583ccc91b5944526c857bf90c73c17",
    "esp_lcd_common.h": "bbff1d2b72f1f6b255112557a45fef28ce3be5d652541f1203fdc403225b085c",
    "rgb_lcd_rotation_sw.h": "c623989c33f0d43015cd215f79490b95850f02212dece6e0941a23a591bcdf9b",
}
PATCHES = (
    (
        b"// Porthole: restart only on request or a missed EOF, rather than resetting healthy frames.\n"
        b"#undef CONFIG_LCD_RGB_RESTART_IN_VSYNC\n#define CONFIG_LCD_RGB_RESTART_IN_VSYNC 0\n",
        b"",
    ),
    (
        b"        // Upstream c0fb05b37e: a new DMA transaction starts with bounce buffer 0.\n        rgb_panel->bb_eof_count = 0;\n",
        b"",
    ),
)


def main() -> None:
    for filename, expected in HASHES.items():
        original = (VENDOR / filename).read_bytes()
        if filename.endswith(".c"):
            for patched, upstream in PATCHES:
                if original.count(patched) != 1:
                    raise SystemExit(f"RGB driver patch missing or duplicated: {filename}")
                original = original.replace(patched, upstream)
        if hashlib.sha256(original).hexdigest() != expected:
            raise SystemExit(f"Unreviewed RGB vendor changes: {filename}; revalidate the upstream pin and patches")
    platform = (APP.parents[1] / "platform/waveshare-round.ini").read_text()
    pin = "https://github.com/pioarduino/platform-espressif32/releases/download/54.03.21/platform-espressif32.zip"
    if f"platform = {pin}\n" not in platform:
        raise SystemExit("Framework pin changed: revalidate the local RGB driver before updating the pin")
    print("RGB driver: upstream pin, companion headers and both recovery patches verified")


if __name__ == "__main__":
    main()
