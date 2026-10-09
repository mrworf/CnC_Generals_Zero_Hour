"""Source-locked full GUI tables; no callback deletion or placeholder acceptance."""
import hashlib
from pathlib import Path
import unittest

REPO = Path(__file__).resolve().parents[2]
SYSTEM = REPO / "GeneralsMD/Code/GameEngine/Source/Common/System"


class LexiconTables(unittest.TestCase):
    def test_original_payload_unchanged(self):
        source = (SYSTEM / "FunctionLexiconTables.cpp").read_text()
        payload = source[source.index("// Popup Ladder Select"):source.index("FunctionLexicon::FunctionLexicon()")]
        self.assertEqual(hashlib.sha256(payload.encode()).hexdigest(),
                         "c5070a715576d4bd607502f9bbf987efd98bd8c0f81b6a08e683db5af53b4d3d")
        self.assertEqual(source.count("constinit static FunctionLexicon::TableEntry"), 7)

    def test_shared_owner_not_menu_registration(self):
        source = (SYSTEM / "FunctionLexicon.cpp").read_text()
        self.assertNotIn("constinit static FunctionLexicon::TableEntry", source)
        self.assertNotIn("FunctionLexicon *TheFunctionLexicon =", source)
        self.assertIn("m_registry.loadTables", source)
        publications = (SYSTEM / "NativeStartupPublications.cpp").read_text()
        self.assertEqual(publications.count("FunctionLexicon *TheFunctionLexicon = nullptr;"), 1)


if __name__ == "__main__":
    unittest.main()
