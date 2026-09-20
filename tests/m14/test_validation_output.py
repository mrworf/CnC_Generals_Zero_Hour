import unittest

from tools.run_validation_clean import has_validation_error, result_code


class ValidationOutputTest(unittest.TestCase):
    def test_clean_success_is_accepted(self):
        self.assertFalse(has_validation_error("scene=water status=pass\n"))
        self.assertEqual(result_code(0, "scene=water status=pass\n"), 0)

    def test_validation_error_fails_a_successful_process(self):
        output = "Validation Error: [ VUID-vkCmdDraw-viewType-07752 ]\n"
        self.assertTrue(has_validation_error(output))
        self.assertEqual(result_code(0, output), 3)

    def test_process_failure_is_preserved(self):
        self.assertEqual(result_code(7, "ordinary failure\n"), 7)


if __name__ == "__main__":
    unittest.main()
