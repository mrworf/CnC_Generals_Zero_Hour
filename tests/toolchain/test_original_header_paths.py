"""Case-sensitive repository include contract for original game-owned callers."""
from pathlib import Path
import re
import unittest
from collections import defaultdict
import hashlib
import json

REPO = Path(__file__).resolve().parents[2]
CODE = REPO / "GeneralsMD/Code"
ROOTS = (CODE / "GameEngine/Include", CODE / "GameEngine/Include/Precompiled",
         CODE / "Libraries/Include", CODE / "Libraries/Source/WWVegas",
         CODE / "Libraries/Source/WWVegas/WWLib", CODE / "Libraries/Source/GameSpy",
         CODE / "Libraries/Source/Compression")
INCLUDE = re.compile(r'^\s*#\s*include\s*"([^"\n]+)"', re.MULTILINE)


def mismatches():
    headers = defaultdict(set)
    for root in ROOTS:
        for header in root.rglob("*"):
            if header.is_file():
                relative = header.relative_to(root).as_posix()
                headers[relative.lower()].add(relative)
    failures = []
    for root in (CODE / "GameEngine/Include", CODE / "GameEngine/Source"):
        for source in sorted(root.rglob("*")):
            if source.suffix.lower() not in (".h", ".hpp", ".cpp", ".c"):
                continue
            for match in INCLUDE.finditer(source.read_text(errors="replace")):
                requested = match[1]
                if (source.parent / requested).is_file():
                    continue
                candidates = headers.get(requested.replace('\\', '/').lower(), set())
                if len(candidates) == 1 and requested not in candidates:
                    failures.append((source.relative_to(REPO).as_posix(), requested,
                                     next(iter(candidates))))
    return failures


class OriginalHeaderPaths(unittest.TestCase):
    def test_shared_startup_data_has_one_actual_provider(self):
        sources = sorted((CODE / "GameEngine/Source").rglob("*.cpp"))
        publications = {
            "TheNetwork": "NetworkInterface", "TheLAN": "LANAPI",
            "TheIMEManager": "IMEManagerInterface", "TheDisconnectMenu": "DisconnectMenu",
            "TheChallengeGameInfo": "SkirmishGameInfo", "TheSkirmishGameInfo": "SkirmishGameInfo",
        }
        text = {p: re.sub(r'/\*.*?\*/|//[^\n]*', '', p.read_text(), flags=re.S) for p in sources}
        provider = CODE / "GameEngine/Source/Common/System/NativeStartupPublications.cpp"
        for symbol, kind in publications.items():
            pattern = re.compile(r'^\s*' + kind + r'\s*\*\s*' + symbol + r'\s*=', re.M)
            found = [p for p, body in text.items() for _ in pattern.finditer(body)]
            self.assertEqual(found, [provider])
        fixup = re.compile(r'\bvoid\s+FixupScoreScreenMovieWindow\s*\([^;]*?\)\s*\{')
        found = [p for p, body in text.items() for _ in fixup.finditer(body)]
        self.assertEqual(found, [CODE / "GameEngine/Source/GameClient/GUI/NativeScoreScreenState.cpp"])
        # Source-locked to the original shared enum/defaults at 174a2946.
        pairs = [
            (CODE / "GameEngine/Include/GameClient/OnlineChatColors.h",
             r'enum GameSpyColors\s*\{(.*?)\};',
             "2183c4c9502658ab08c06edcc4ac45a73fb8593ca4b32921ef1d5280bf420f2b"),
            (CODE / "GameEngine/Source/Common/INI/INIOnlineChatColors.cpp",
             r'Color GameSpyColor\[GSCOLOR_MAX\]\s*=\s*\{(.*?)\};',
             "b8574db363321ec1bed874ce51d3797c4f912c06fe16ceea1a82de403bd3c44b"),
            (CODE / "GameEngine/Source/Common/INI/INIOnlineChatColors.cpp",
             r'static const FieldParse GameSpyColorFieldParse\[\]\s*=\s*\{(.*?)\n\};',
             "b8c0303450282983d0f4dd1f3577518a908dbf0d5f5a326de28272d63573b889"),
        ]
        for path, pattern, digest in pairs:
            match = re.search(pattern, path.read_text(), re.S)
            self.assertIsNotNone(match)
            body = re.sub(r'/\*.*?\*/|//[^\n]*', '', match[1], flags=re.S)
            self.assertEqual(hashlib.sha256(re.sub(r'\s+', '', body).encode()).hexdigest(), digest)

    def test_native_runtime_retained_source_graph(self):
        cmake = (REPO / "cmake/OriginalRuntime.cmake").read_text()
        project = (CODE / "GameEngine/GameEngine.dsp").read_text()
        for variable, area in (("ZH_ORIGINAL_GAMEPLAY_COMMON", "Common"),
                               ("ZH_ORIGINAL_LOGICAL_CLIENT", "GameClient")):
            block = cmake[cmake.index('set(' + variable):
                          cmake.index('list(TRANSFORM ' + variable)]
            selected = re.findall(r'[A-Za-z0-9_/]+\.cpp', block)
            providers = {source.replace('\\', '/').removeprefix('./Source/' + area + '/')
                         for source in re.findall(r'^SOURCE=(.*)$', project, re.M)}
            self.assertTrue(selected)
            self.assertEqual(len(selected), len(set(selected)))
            for source in selected:
                self.assertIn(source, providers)
                self.assertTrue((CODE / 'GameEngine/Source' / area / source).is_file())

    def test_excluded_bootstrap_owners_are_not_retained(self):
        source = (CODE / 'GameEngine/Source/Common/GameEngine.cpp').read_text()
        body = re.sub(r'/\*.*?\*/|//[^\n]*', '', source, flags=re.S)
        self.assertNotRegex(body, r'\b(TheCDManager|CreateCDManager|TheGameResultsQueue|GameResultsInterface)\b')
        self.assertNotIn('getArchiveFilenameForFile', body)
        self.assertIn('TheAudio->isMusicAlreadyLoaded()', body)
        self.assertIn('TheGameLogic->UPDATE()', body)
        self.assertIn('TheNetwork->UPDATE()', body)
        self.assertNotRegex(body, r'\b(TheLocalFileSystem|TheArchiveFileSystem|updateTGAtoDDS|CONVERT_EXEC1)\b')
        self.assertNotRegex(body, r'\bsystem\s*\(')
        self.assertIn('TheFileSystem->init()', body)
        self.assertIn('supplied assets are read-only', body)

    def test_release_lookup_has_no_development_test_art(self):
        header = (CODE / "GameEngine/Include/Common/FileSystem.h").read_text()
        body = re.sub(r'/\*.*?\*/|//[^\n]*', '', header, flags=re.S)
        self.assertNotRegex(body, r'#\s*define\s+(LOAD_TEST_ASSETS|LOOK_FOR_TEST_ART|TEST_STRING|TEST_W3D_DIR_PATH|TEST_TGA_DIR_PATH)\b')
        for source in (REPO / "CMakeLists.txt", *sorted((REPO / "cmake").glob("*.cmake"))):
            text = re.sub(r'#[^\n]*', '', source.read_text())
            self.assertNotRegex(text, r'\b(LOAD_TEST_ASSETS|LOOK_FOR_TEST_ART)\b')

    def test_native_command_line_preserves_source_table(self):
        source = (CODE / 'GameEngine/Source/Common/CommandLine.cpp').read_text()
        table = source[source.index('static CommandLineParam params[]'):source.index('// parseCommandLine')]
        self.assertEqual(hashlib.sha256(table.encode()).hexdigest(),
                         '5c4bb5b78b547b75c4e2340693b7a4a685d1dd84cb3e681c6d4a4905d397f6cc')
        body = re.sub(r'/\*.*?\*/|//[^\n]*', '', source, flags=re.S)
        self.assertNotRegex(body, r'\b(atoi|fopen|TheArchiveFileSystem|TheLocalFileSystem|DX8Wrapper_PreserveFPU)\b')
        self.assertNotIn('DEBUG_LOG', body)
        self.assertIn('mountReadOnlyMods', body)
        self.assertIn('ModPublicationGuard', body)
        self.assertIn('std::from_chars', body)

    def test_native_key_protocol(self):
        manifest = json.loads((REPO / "tests/original/key_protocol.json").read_text())
        text = (CODE / "GameEngine/Include/GameClient/KeyDefs.h").read_text()
        key_body = text[text.index('enum KeyDefType'):text.index('// state for keyboard IO')]
        keys = {key: int(value,16) for key,value in
                re.findall(r'\b(KEY_\w+)\s*=\s*(0x[0-9A-Fa-f]+)',key_body)}
        self.assertEqual(keys,manifest["keys"])
        self.assertNotIn("DIK_",text)
        self.assertNotIn("dinput.h",text)
        self.assertIn("enum KeyDefType : UnsignedInt",text)
        meta = (CODE / "GameEngine/Include/GameClient/MetaEvent.h").read_text()
        subset = re.findall(r'\b(MK_\w+)\s*=\s*(KEY_\w+)',meta)
        self.assertTrue(subset)
        for _,key in subset:
            self.assertIn(key,keys)
        state = text[text.index('// state for keyboard IO'):text.index('// INLINING')]
        body = re.sub(r'/\*.*?\*/|//[^\n]*', '', state, flags=re.S)
        self.assertEqual(hashlib.sha256(re.sub(r'\s+', '', body).encode()).hexdigest(),
                         manifest["modifier_body_sha256"])

    def test_shared_viewport_protocol(self):
        header = CODE / "GameEngine/Include/GameClient/ViewFilters.h"
        text = header.read_text()
        expected = {
            "FilterTypes": "f4f22ad47fb6a7f68a2add989d0f7f32f5938456122362f779b4b76c205124af",
            "FilterModes": "06458e9f6acd9adab3137ed315c2beaff081a0a1c9079368c627a4cae3b003e3",
        }
        # Source-locked to the preceding original CommandXlat bodies at 9e5c0e4f.
        for name, digest in expected.items():
            match = re.search(r'enum ' + name + r'\s*\{(.*?)\};', text, re.S)
            self.assertIsNotNone(match)
            body = re.sub(r'/\*.*?\*/|//[^\n]*', '', match[1], flags=re.S)
            self.assertEqual(hashlib.sha256(re.sub(r'\s+', '', body).encode()).hexdigest(), digest)
        for caller in ("View.h", "CommandXlat.h"):
            source = (header.parent / caller).read_text()
            self.assertIn('#include "GameClient/ViewFilters.h"', source)
            self.assertNotRegex(source, r'enum (FilterTypes|FilterModes)\s*\{')

    def test_canonical_casing(self):
        self.assertEqual(mismatches(), [])

    def test_dynamic_id_declarations(self):
        ids = ("ObjectID", "DrawableID", "FormationID", "ParticleSystemID",
               "ProductionID", "WaypointID")
        declarations = re.compile(r'\benum\s+(' + '|'.join(ids) +
                                  r')\s*(?::\s*([^;{\n]+))?\s*[;{]')
        definitions = {name: 0 for name in ids}
        for root in (CODE / "GameEngine/Include", CODE / "GameEngine/Source"):
            for source in root.rglob("*"):
                if source.suffix.lower() not in (".h", ".hpp", ".cpp", ".c"):
                    continue
                for match in declarations.finditer(source.read_text(errors="replace")):
                    self.assertEqual((match[2] or "").strip(), "UnsignedInt",
                                     source.relative_to(REPO).as_posix())
                    if match[0].rstrip().endswith("{"):
                        definitions[match[1]] += 1
                        self.assertEqual(source, CODE / "GameEngine/Include/Common/EngineIDs.h")
        self.assertEqual(definitions, {name: 1 for name in ids})


if __name__ == "__main__":
    unittest.main()
