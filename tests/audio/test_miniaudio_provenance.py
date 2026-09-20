from __future__ import annotations

import hashlib
from pathlib import Path
import tempfile
import unittest


ROOT = Path(__file__).resolve().parents[2]
EXPECTED = {
    "miniaudio.h": "ac7af4de748b7e26b777f37e01cee313a308a7296a3eb080e2906b320cc55c89",
    "LICENSE": "457f1b500e0adf6bc059edddfa78a2f62012e7c3bb43476c20e0bd23b25ba0eb",
}


def verify_hash(path: Path, expected: str) -> None:
    actual = hashlib.sha256(path.read_bytes()).hexdigest()
    if actual != expected:
        raise ValueError(f"miniaudio provenance hash mismatch for {path.name}: {actual}")


class MiniaudioProvenanceTest(unittest.TestCase):
    def test_vendored_files_match_reviewed_release(self) -> None:
        for name, digest in EXPECTED.items():
            verify_hash(ROOT / "third_party" / "miniaudio" / name, digest)

    def test_modified_dependency_is_rejected(self) -> None:
        with tempfile.TemporaryDirectory() as directory:
            changed = Path(directory) / "miniaudio.h"
            changed.write_bytes(b"not the reviewed dependency")
            with self.assertRaisesRegex(ValueError, "provenance hash mismatch"):
                verify_hash(changed, EXPECTED["miniaudio.h"])

    def test_build_has_no_dependency_download(self) -> None:
        cmake = (ROOT / "CMakeLists.txt").read_text(encoding="utf-8").lower()
        forbidden = ("fetchcontent", "externalproject", "file(download", "url_hash")
        for token in forbidden:
            self.assertNotIn(token, cmake, f"configure-time download primitive found: {token}")


if __name__ == "__main__":
    unittest.main()
