import json
import pathlib
import tempfile
import unittest

from tools.check_original_identity import verify


class OriginalIdentityTest(unittest.TestCase):
    def test_missing_original_provider_is_rejected_before_runtime(self):
        with tempfile.TemporaryDirectory() as directory:
            root = pathlib.Path(directory)
            commands = root / "compile_commands.json"
            link_map = root / "link.map"
            commands.write_text(json.dumps([{"file": "/repo/another.cpp"}]), encoding="utf-8")
            link_map.write_text("another.cpp.o", encoding="utf-8")
            with self.assertRaisesRegex(ValueError, "was not compiled"):
                verify(commands, link_map, pathlib.Path("/bin/true"), ["GeneralsMD/Code/required.cpp"])


if __name__ == "__main__":
    unittest.main()
