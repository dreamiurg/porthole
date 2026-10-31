#!/usr/bin/env python3
"""Focused fixtures for flash accounting, attribution and incomplete tool output."""

from pathlib import Path
import tempfile
import unittest
from unittest.mock import patch

from flash_report import find_tool, make_report, parse_sections, parse_symbols

SIZE = """.pio/build/firmware/firmware.elf  :
section size addr
.iram0.vectors 10 1077362688
.iram0.text 20 1077362700
.dram0.data 30 1070170112
.dram0.bss 40 1070170142
.flash.text 100 1107296256
.flash.rodata 200 1006632960
.flash_rodata_dummy 123456 1006632860
.debug_info 99999 0
Total 223355
"""


class FlashReportTests(unittest.TestCase):
    def test_default_platformio_install_without_env_or_path(self):
        with tempfile.TemporaryDirectory() as directory:
            home = Path(directory)
            tool = home / ".platformio/packages/toolchain-xtensa-esp-elf/bin/xtensa-esp-elf-size"
            tool.parent.mkdir(parents=True)
            tool.write_text("#!/bin/sh\n", encoding="utf-8")
            tool.chmod(0o755)
            with patch("flash_report.Path.home", return_value=home), patch.dict("os.environ", {}, clear=True), patch("flash_report.shutil.which", return_value=None):
                self.assertEqual(find_tool("size", None), str(tool))

    def test_budget_excludes_dummy_debug_and_bss_flash(self):
        with tempfile.TemporaryDirectory() as directory:
            elf = Path(directory) / "fixture.elf"
            elf.write_bytes(b"fixture")
            report = make_report(elf, 400, parse_sections(SIZE), [])
        self.assertEqual((report["flash_used"], report["flash_free"], report["flash_percent"]), (360, 40, 90))
        self.assertEqual(report["ram_used"], 70)
        self.assertEqual(report["elf_sha256"], "f16d05ec6b29248d2c61adb1e9263f78e4f7bace1b955014a2d17872cfe4064d")

    def test_flash_data_aliases_and_range_boundaries(self):
        nm = """3c000000 00000020 d game::ART
3c000000 00000020 r game::ART_ALIAS
42000000 00000010 T game::draw()
3fc90000 00001000 B game::RAM
3c0000c0 00000008 d game::EDGE
3c0000c7 00000002 d game::OUTSIDE
3c0000c8 00000001 d game::AFTER
42000064 00000001 T game::AFTER_CODE
42000010 00000000 T game::ZERO
00000010 a .callsz
42000020 T game::UNSIZED
"""
        sections = parse_sections(SIZE)
        symbols = parse_symbols(nm, sections)
        self.assertEqual([s["name"] for s in symbols], ["game::ART", "game::draw()", "game::EDGE"])
        self.assertEqual(sum(s["size"] for s in symbols), 56)
        self.assertEqual(symbols, parse_symbols("\n".join(reversed(nm.splitlines())), sections))

    def test_missing_and_malformed_sections_fail(self):
        for output in (SIZE.replace(".dram0.bss 40 1070170142\n", ""), SIZE.replace(".flash.text 100", ".flash.text bad"), SIZE + ".flash.text 1 2\n"):
            with self.subTest(output=output), self.assertRaises(ValueError):
                parse_sections(output)

    def test_attribution_is_separate_from_exact_flash_budget(self):
        sections = parse_sections(SIZE)
        symbols = parse_symbols("3c000000 20 d game::ART\n42000000 10 T game::draw()\n42000010 08 T helper", sections)
        with tempfile.TemporaryDirectory() as directory:
            elf = Path(directory) / "fixture.elf"
            elf.write_bytes(b"fixture")
            report = make_report(elf, 400, sections, symbols)
        self.assertEqual(report["named_flash_bytes_estimate"], 56)
        self.assertEqual(report["flash_used"], 360)
        self.assertEqual(report["largest_namespaces"], [{"namespace": "game", "size": 48}, {"namespace": "<global>", "size": 8}])

    def test_malformed_symbol_fails(self):
        with self.assertRaisesRegex(ValueError, "Malformed nm"):
            parse_symbols("3c000000 size d bad", parse_sections(SIZE))

    def test_over_budget_is_negative_headroom(self):
        with tempfile.TemporaryDirectory() as directory:
            elf = Path(directory) / "fixture.elf"
            elf.write_bytes(b"fixture")
            report = make_report(elf, 300, parse_sections(SIZE), [])
            self.assertEqual((report["flash_free"], report["flash_percent"]), (-60, 120))
            with self.assertRaises(ValueError):
                make_report(elf, 0, parse_sections(SIZE), [])


if __name__ == "__main__":
    unittest.main()
