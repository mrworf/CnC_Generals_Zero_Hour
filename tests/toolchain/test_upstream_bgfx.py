import importlib.util
from pathlib import Path
import tempfile
import unittest
from unittest.mock import patch

MODULE_PATH = Path(__file__).resolve().parents[2] / 'tools/upstream_bgfx.py'
spec = importlib.util.spec_from_file_location('upstream_bgfx', MODULE_PATH)
module = importlib.util.module_from_spec(spec)
spec.loader.exec_module(module)


class ProvenanceTests(unittest.TestCase):
    def test_lock_is_official_and_complete(self):
        self.assertEqual(set(module.read_lock()), {'bgfx', 'bx', 'bimg'})

    def test_missing_checkout_rejected(self):
        with tempfile.TemporaryDirectory() as directory:
            with self.assertRaisesRegex(ValueError, 'standalone'):
                module.verify('bgfx', Path(directory), module.read_lock()['bgfx'])

    def test_pristine_and_each_mismatch(self):
        spec = module.read_lock()['bgfx']
        with tempfile.TemporaryDirectory() as directory:
            path = Path(directory)
            (path / '.git').mkdir()
            (path / 'LICENSE').touch()
            cases = [
                ([spec['revision'], spec['url'], ''], None),
                (['0' * 40], 'revision'),
                ([spec['revision'], 'https://example.invalid/fork.git'], 'origin'),
                ([spec['revision'], spec['url'], ' M include/bgfx/bgfx.h'], 'modified'),
                ([spec['revision'], spec['url'], '?? private_extension.h'], 'untracked'),
            ]
            for values, error in cases:
                with self.subTest(error=error), patch.object(module, 'output', side_effect=values):
                    if error:
                        with self.assertRaisesRegex(ValueError, error):
                            module.verify('bgfx', path, spec)
                    else:
                        self.assertEqual(module.verify('bgfx', path, spec), spec['revision'])


if __name__ == '__main__':
    unittest.main()
