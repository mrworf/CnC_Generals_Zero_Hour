import pathlib
import shutil
import tempfile
import unittest

from tools.check_m27_original_provenance import EXTRACTIONS, validate


ROOT = pathlib.Path(__file__).resolve().parents[2]


class ProvenanceControlTests(unittest.TestCase):
    def fixture(self, target: pathlib.Path) -> None:
        paths = {pathlib.Path("CMakeLists.txt"), pathlib.Path("include/zh/original_data.h"),
                 pathlib.Path("GeneralsMD/Code/GameEngine/Source/Common/System/FPUControl.cpp")}
        for extraction, (authority, _) in EXTRACTIONS.items():
            paths.add(pathlib.Path(extraction)); paths.add(pathlib.Path(authority))
        for relative in paths:
            destination = target / relative
            destination.parent.mkdir(parents=True, exist_ok=True)
            shutil.copy2(ROOT / relative, destination)

    def test_registry_drift_is_rejected(self):
        with tempfile.TemporaryDirectory(prefix="m27-provenance-") as directory:
            root = pathlib.Path(directory); self.fixture(root)
            path = root / "GeneralsMD/Code/GameEngine/Source/Common/INI/INI.cpp"
            path.write_text(path.read_text(encoding="utf-8").replace('"MapCache"', '"RemovedMapCache"'), encoding="utf-8")
            self.assertTrue(any("MapCache" in error for error in validate(root)))

    def test_extraction_marker_drift_is_rejected(self):
        with tempfile.TemporaryDirectory(prefix="m27-provenance-") as directory:
            root = pathlib.Path(directory); self.fixture(root)
            path = root / "GeneralsMD/Code/GameEngine/Source/Common/System/OriginalXfer.cpp"
            path.write_text(path.read_text(encoding="utf-8").replace("Xfer::xferBool", "removed"), encoding="utf-8")
            self.assertTrue(any("xferBool" in error for error in validate(root)))

    def test_public_declaration_drift_is_rejected(self):
        with tempfile.TemporaryDirectory(prefix="m27-provenance-") as directory:
            root = pathlib.Path(directory); self.fixture(root)
            path = root / "include/zh/original_data.h"
            path.write_text(path.read_text(encoding="utf-8").replace("load_map_catalog", "removed_map_catalog"), encoding="utf-8")
            self.assertTrue(any("load_map_catalog" in error for error in validate(root)))


if __name__ == "__main__":
    unittest.main()
