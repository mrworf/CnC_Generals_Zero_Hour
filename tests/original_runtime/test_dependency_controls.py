import csv
import pathlib
import tempfile
import unittest

from tools.check_original_dependency_ledger import COLUMNS, validate
from tools.check_m26_original_identity import archive_member_included, target_object


ROOT = pathlib.Path(__file__).resolve().parents[2]
LEDGER = ROOT / "docs/original-runtime-dependency-ledger.tsv"


class DependencyControlTests(unittest.TestCase):
    def test_basename_only_compile_match_is_rejected(self):
        source = ROOT / "GeneralsMD/Code/GameEngine/Source/Common/System/FPUControl.cpp"
        commands = [{"file": str(source), "output": "/tmp/CMakeFiles/proxy.dir/FPUControl.cpp.o"}]
        self.assertIsNone(target_object(commands, source, "zh_original_process"))

    def test_discarded_only_link_member_is_rejected(self):
        link_map = "Discarded input sections\n libzh_original_process.a(FPUControl.cpp.o)\n"
        self.assertFalse(archive_member_included(
            link_map, "zh_original_process", "FPUControl.cpp.o"))

    def test_source_drift_is_rejected(self):
        with tempfile.TemporaryDirectory() as directory:
            root = pathlib.Path(directory)
            source = root / "provider.cpp"
            source.write_text("changed", encoding="utf-8")
            ledger = root / "ledger.tsv"
            row = {column: "none" for column in COLUMNS}
            row.update(source="provider.cpp", sha256="0" * 64, symbol="provider",
                       consumer="test", lifecycle_or_config="init", provider_target="target",
                       owner="M27", evidence_grade="inspected", tests="later")
            with ledger.open("w", newline="", encoding="utf-8") as stream:
                writer = csv.DictWriter(stream, fieldnames=COLUMNS, delimiter="\t")
                writer.writeheader(); writer.writerow(row)
            self.assertTrue(any("source drift" in error for error in validate(root, ledger)))

    def test_ownerless_edge_is_rejected(self):
        with tempfile.TemporaryDirectory() as directory:
            ledger = pathlib.Path(directory) / "ledger.tsv"
            with LEDGER.open(encoding="utf-8") as source:
                rows = list(csv.DictReader(source, delimiter="\t"))
            rows[0]["owner"] = "none"
            with ledger.open("w", newline="", encoding="utf-8") as stream:
                writer = csv.DictWriter(stream, fieldnames=COLUMNS, delimiter="\t")
                writer.writeheader(); writer.writerows(rows)
            errors = validate(ROOT, ledger)
            self.assertTrue(any("ownerless/unknown" in error for error in errors))


if __name__ == "__main__":
    unittest.main()
