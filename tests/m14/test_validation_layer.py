import unittest

from tools.check_vulkan_validation import parse


class ValidationLayerProbeTest(unittest.TestCase):
    def test_exact_khronos_layer_is_required(self):
        self.assertTrue(parse("Instance Layers:\nVK_LAYER_KHRONOS_validation Khronos Validation Layer\n"))
        self.assertFalse(parse("VK_LAYER_NV_optimus NVIDIA Optimus layer\n"))
        self.assertFalse(parse("prefix VK_LAYER_KHRONOS_validation\n"))


if __name__ == "__main__":
    unittest.main()
