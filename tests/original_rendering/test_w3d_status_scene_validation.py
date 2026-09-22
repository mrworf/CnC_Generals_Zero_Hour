#!/usr/bin/env python3
"""Validation output gate for a successful original 2D status-scene child."""

import unittest

from test_w3d_status_scene import has_validation_diagnostic


class StatusSceneValidationOutputTest(unittest.TestCase):
    def test_ordinary_success_is_accepted(self) -> None:
        self.assertFalse(has_validation_diagnostic(
            "original 2D status scene: physical generations=4 resources=0\n"))

    def test_validation_error_is_rejected(self) -> None:
        self.assertTrue(has_validation_diagnostic(
            "original 2D status scene: physical generations=4 resources=0\n"
            "Validation Error: invalid framebuffer state\n"))

    def test_vuid_is_rejected(self) -> None:
        self.assertTrue(has_validation_diagnostic(
            "original 2D status scene: physical generations=4 resources=0\n"
            "VUID-vkCmdDraw-None-00001\n"))


if __name__ == "__main__":
    unittest.main()
