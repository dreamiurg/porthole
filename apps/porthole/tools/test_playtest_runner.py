"""Keep the playtest runner strict while separating compilation and result checks."""

import importlib.util
import subprocess
import tempfile
import unittest
from pathlib import Path
from unittest.mock import patch

spec = importlib.util.spec_from_file_location("playtest_runner", Path(__file__).with_name("playtest.py"))
assert spec and spec.loader
runner = importlib.util.module_from_spec(spec)
spec.loader.exec_module(runner)


class RunnerChecks(unittest.TestCase):
    def test_scope_long_deadline(self):
        self.assertEqual(runner.scenario_limit(Path("29_family_controls.txt")), 120)
        self.assertEqual(runner.scenario_limit(Path("20_monkey_home.txt")), 120)
        self.assertEqual(runner.scenario_limit(Path("28_clock.txt")), 30)

    def test_failed_expectations_remain_failures(self):
        result = {"fails": []}
        runner.check_result(result, "expect", ("today", ">=", "60"), "daily cap", ["today=59"])
        runner.check_result(result, "expect", ("today", "=", "0"), "missing value", ["screen=picker"])
        runner.check_result(result, "expect-screen", "rest", "cutoff", ["screen=launcher"])
        self.assertEqual(len(result["fails"]), 3)

    def test_missing_output_is_not_a_pass(self):
        result = {"fails": [], "checks": 0}
        runner.collect_results(Path("case.txt"), Path("."), result, "", [("0", "cap", "expect", ("today", "=", "60"))])
        self.assertEqual(len(result["fails"]), 1)
        self.assertEqual(result["checks"], 0)

    def test_runtime_errors_remain_fatal(self):
        cases = ((0, "ERROR: AddressSanitizer", "sanitizer error"), (0, "unknown command: bad", "unknown sim command"), (2, "", "exit code 2"))
        with tempfile.TemporaryDirectory() as directory:
            for status, error, message in cases:
                with self.subTest(message=message), patch.object(runner.subprocess, "run", return_value=subprocess.CompletedProcess([], status, "", error)):
                    with self.assertRaisesRegex(runner.Fail, message):
                        runner.run_script(Path("snap"), Path(directory), [], 100, 30)


if __name__ == "__main__":
    unittest.main()
