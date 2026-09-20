import unittest
from pathlib import Path


class RendererPublicHeaderTest(unittest.TestCase):
    def test_headers_are_backend_neutral(self):
        forbidden = ("d3d", "direct3d", "sdl", "vulkan")
        for path in Path("include/zh/renderer").glob("*.h"):
            text = path.read_text(encoding="utf-8").lower()
            for token in forbidden:
                self.assertNotIn(token, text, f"{path}: public renderer header contains {token}")


if __name__ == "__main__":
    unittest.main()
