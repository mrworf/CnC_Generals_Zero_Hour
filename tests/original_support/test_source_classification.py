import pathlib
import unittest

from tools.original_source_classification import Record, classify, validate


ROOT = pathlib.Path(__file__).resolve().parents[2]


class SourceClassificationTest(unittest.TestCase):
    def test_canonical_inventory_has_complete_unique_coverage(self):
        records = classify((ROOT / "cmake/LegacySourceInventory.cmake").read_text(encoding="utf-8"))
        validate(records)
        self.assertGreater(len(records), 3000)
        self.assertEqual(len(records), len({record.path for record in records}))
        self.assertTrue(all(record.provider and record.rationale and record.origins for record in records))

    def test_missing_policy_is_rejected(self):
        with self.assertRaisesRegex(ValueError, "missing classification policy"):
            classify('set(ZH_LEGACY_UNKNOWN_SOURCES\n  "source.cpp"\n)\n')

    def test_bootstrap_and_toy_production_providers_are_rejected(self):
        for provider in ("src/bootstrap/component.cpp", "src/simulation/toy.cpp", "fixture.cpp"):
            with self.subTest(provider=provider), self.assertRaisesRegex(ValueError, "not an original translation unit"):
                validate([Record("legacy.cpp", "production-compiled", provider, "test", ("inventory",))])

    def test_original_translation_unit_provider_is_accepted(self):
        validate([Record("legacy.cpp", "production-compiled", "GeneralsMD/Code/legacy.cpp", "compiled", ("inventory",))])


if __name__ == "__main__":
    unittest.main()
