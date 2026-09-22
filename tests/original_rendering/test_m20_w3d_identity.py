#!/usr/bin/env python3
"""Positive and negative DSO-name controls for the production W3D identity gate."""

import sys
import unittest
from pathlib import Path

sys.path.insert(0, str(Path(__file__).resolve().parents[2] / "tools"))
from check_m20_w3d_identity import has_direct3d_dependency


class W3dDependencyIdentityTest(unittest.TestCase):
    def test_aslr_addresses_are_not_dependencies(self) -> None:
        self.assertFalse(has_direct3d_dependency(
            "libfreetype.so.6 => /usr/lib/libfreetype.so.6 (0x00007f960d3d5000)\n"
            "/lib64/ld-linux-x86-64.so.2 (0x00007f960d3d0000)\n"))

    def test_direct3d_sonames_are_rejected(self) -> None:
        for name in ("libd3d8.so", "libvkd3d.so", "libDirect3D.so"):
            with self.subTest(name=name):
                self.assertTrue(has_direct3d_dependency(
                    f"{name} => /usr/lib/{name} (0x00007f9609315000)\n"))


if __name__ == "__main__":
    unittest.main()
