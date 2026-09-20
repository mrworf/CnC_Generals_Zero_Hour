import csv
import tempfile
import unittest
from pathlib import Path

from tools.renderer_inventory import InventoryError, Rule, classify, load_rules


class RendererInventoryTest(unittest.TestCase):
    def test_unknown_identifier_is_rejected(self):
        rules = load_rules(Path("docs/renderer/legacy-api-mapping.tsv"))
        with self.assertRaisesRegex(InventoryError, "D3D_FUTURE_UNMAPPED: unmapped"):
            classify({"D3D_FUTURE_UNMAPPED"}, rules)

    def test_ambiguous_rules_are_rejected(self):
        rules = [
            Rule("one", r"^D3DFMT_.*$", __import__("re").compile(r"^D3DFMT_.*$"), "formats", "public-api", "one", "one"),
            Rule("two", r"^D3D.*$", __import__("re").compile(r"^D3D.*$"), "formats", "public-api", "two", "two"),
        ]
        with self.assertRaisesRegex(InventoryError, "ambiguous: one, two"):
            classify({"D3DFMT_DXT1"}, rules)

    def test_empty_mapping_field_is_rejected(self):
        with tempfile.TemporaryDirectory() as directory:
            path = Path(directory) / "rules.tsv"
            with path.open("w", encoding="utf-8", newline="") as stream:
                writer = csv.writer(stream, delimiter="\t")
                writer.writerow(("rule_id", "pattern", "category", "disposition", "target", "rationale"))
                writer.writerow(("broken", "^D3D$", "legacy", "unsupported", "", "reason"))
            with self.assertRaisesRegex(InventoryError, "empty fields: target"):
                load_rules(path)


if __name__ == "__main__":
    unittest.main()
