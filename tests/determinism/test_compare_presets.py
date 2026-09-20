from __future__ import annotations

import subprocess
import sys
import tempfile
import unittest
from pathlib import Path


SCRIPT = Path(__file__).with_name("compare_presets.py")


def report(crc: int) -> str:
    return (
        "format=zh-determinism-v1\n"
        "scenario=asset-free-skirmish\n"
        "config=seed=42;players=2;rate=30\n"
        f"checkpoint=2 crc={crc}\n"
    )


class ComparePresetsTest(unittest.TestCase):
    def test_match_and_diagnosed_divergence(self) -> None:
        with tempfile.TemporaryDirectory() as directory:
            root = Path(directory)
            expected = root / "linux-gcc-debug.txt"
            actual = root / "linux-clang-release.txt"
            expected.write_text(report(100), encoding="utf-8")
            actual.write_text(report(100), encoding="utf-8")
            matching = subprocess.run(
                [sys.executable, str(SCRIPT), str(expected), str(actual)],
                text=True,
                capture_output=True,
                check=False,
            )
            self.assertEqual(matching.returncode, 0, matching.stderr)
            self.assertIn("reports match", matching.stdout)

            actual.write_text(report(101), encoding="utf-8")
            divergent = subprocess.run(
                [sys.executable, str(SCRIPT), str(expected), str(actual)],
                text=True,
                capture_output=True,
                check=False,
            )
            self.assertNotEqual(divergent.returncode, 0)
            diagnostic = divergent.stdout + divergent.stderr
            self.assertIn(f"source={actual}", diagnostic)
            self.assertIn("scenario=asset-free-skirmish", diagnostic)
            self.assertIn("config=seed=42;players=2;rate=30", diagnostic)
            self.assertIn("checkpoint=2", diagnostic)
            self.assertIn("expected=100 actual=101", diagnostic)


if __name__ == "__main__":
    unittest.main()
