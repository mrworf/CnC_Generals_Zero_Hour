from pathlib import Path
import subprocess
import sys
import tempfile
import unittest


ROOT = Path(__file__).resolve().parents[2]
TOOL = ROOT / "tools/legacy_manifest_inventory.py"


class LegacyManifestInventoryTest(unittest.TestCase):
    def test_checked_inventory_is_current(self):
        result = subprocess.run(
            [sys.executable, str(TOOL), "--check"], cwd=ROOT, text=True, capture_output=True
        )
        self.assertEqual(result.returncode, 0, result.stderr)
        self.assertIn("candidate", result.stdout)

    def test_incomplete_inventory_is_rejected(self):
        with tempfile.TemporaryDirectory() as directory:
            bad_cmake = Path(directory) / "LegacySourceInventory.cmake"
            bad_cmake.write_text("# deliberately incomplete\n", encoding="utf-8")
            result = subprocess.run(
                [
                    sys.executable,
                    str(TOOL),
                    "--check",
                    "--cmake-output",
                    str(bad_cmake),
                ],
                cwd=ROOT,
                text=True,
                capture_output=True,
            )
        self.assertNotEqual(result.returncode, 0)
        self.assertIn("missing or stale", result.stderr)
        self.assertIn("LegacySourceInventory.cmake", result.stderr)


if __name__ == "__main__":
    unittest.main()
