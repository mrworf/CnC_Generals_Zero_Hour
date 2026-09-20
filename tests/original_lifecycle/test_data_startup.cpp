#include "PreRTS.h"
#include "Common/FileSystem.h"
#include "Common/GameState.h"
#include "Common/GameEngine.h"
#include "Common/GlobalData.h"
#include "Common/INI.h"
#include "Common/INIException.h"
#include "Common/LocalFileSystem.h"
#include "Common/NameKeyGenerator.h"
#include "Common/Snapshot.h"
#include "Common/XferCRC.h"
#include "Common/ThingFactory.h"
#include "Common/ThingTemplate.h"
#include "GameClient/GameText.h"
#include "GameClient/MapUtil.h"
#include "GameNetwork/GameSpy/GameSpyColors.h"
#include "PosixDevice/Common/PosixLocalFileSystem.h"

#include <cstdlib>
#include <array>
#include <filesystem>
#include <fstream>
#include <iostream>
#include <algorithm>
#include <string_view>
#include <vector>
#include <unistd.h>

const Char *g_strFile = "missing.str";
const Char *g_csfFile = "Data/English/Generals.csf";
GameEngine *TheGameEngine = NULL;

namespace {
int failures = 0;
bool enteredExecuteOrReset = false;

void check(bool condition, std::string_view message)
{
	if (!condition) { std::cerr << "FAIL: " << message << '\n'; ++failures; }
}

void append32(std::vector<unsigned char>& out, UnsignedInt value)
{
	for (unsigned shift = 0; shift != 32; shift += 8)
		out.push_back(static_cast<unsigned char>(value >> shift));
}

constexpr UnsignedInt tag(char a, char b, char c, char d)
{
	return (UnsignedInt(a) << 24) | (UnsignedInt(b) << 16) | (UnsignedInt(c) << 8) | UnsignedInt(d);
}

void appendCSFString(std::vector<unsigned char>& out, std::string_view label, std::u16string_view value)
{
	append32(out, tag('L', 'B', 'L', ' ')); append32(out, 1); append32(out, label.size());
	out.insert(out.end(), label.begin(), label.end());
	append32(out, tag('S', 'T', 'R', ' ')); append32(out, value.size());
	for (char16_t character : value) {
		const auto encoded = static_cast<UnsignedShort>(~character);
		out.push_back(static_cast<unsigned char>(encoded));
		out.push_back(static_cast<unsigned char>(encoded >> 8));
	}
}

std::vector<unsigned char> csfFixture()
{
	std::vector<unsigned char> out;
	append32(out, tag('C', 'S', 'F', ' ')); append32(out, 3); append32(out, 2); append32(out, 2);
	append32(out, 0); append32(out, 0);
	appendCSFString(out, "GUI:Command&ConquerGenerals", u"Zero Hour Fixture");
	appendCSFString(out, "MAP:Fixture", u"Localized Fixture");
	return out;
}

void writeBytes(const std::filesystem::path& path, const std::vector<unsigned char>& bytes)
{
	std::filesystem::create_directories(path.parent_path());
	std::ofstream stream(path, std::ios::binary | std::ios::trunc);
	stream.write(reinterpret_cast<const char *>(bytes.data()), static_cast<std::streamsize>(bytes.size()));
}

void writeText(const std::filesystem::path& path, std::string_view text)
{
	std::filesystem::create_directories(path.parent_path());
	std::ofstream stream(path, std::ios::binary | std::ios::trunc);
	stream.write(text.data(), static_cast<std::streamsize>(text.size()));
}

std::vector<std::pair<std::filesystem::path, std::uintmax_t>> snapshotFiles(const std::filesystem::path& root)
{
	std::vector<std::pair<std::filesystem::path, std::uintmax_t>> result;
	for (const auto& entry : std::filesystem::recursive_directory_iterator(root))
		if (entry.is_regular_file()) result.emplace_back(entry.path().lexically_relative(root), entry.file_size());
	std::sort(result.begin(), result.end());
	return result;
}

void setInputReadOnly(const std::filesystem::path& root, bool readOnly)
{
	const auto filePermissions = readOnly ? std::filesystem::perms::owner_read :
		(std::filesystem::perms::owner_read | std::filesystem::perms::owner_write);
	const auto directoryPermissions = readOnly ?
		(std::filesystem::perms::owner_read | std::filesystem::perms::owner_exec) :
		std::filesystem::perms::owner_all;
	std::error_code ignored;
	for (const auto& entry : std::filesystem::recursive_directory_iterator(root))
		std::filesystem::permissions(entry.path(), entry.is_directory() ? directoryPermissions : filePermissions,
			std::filesystem::perm_options::replace, ignored);
	std::filesystem::permissions(root, directoryPermissions, std::filesystem::perm_options::replace, ignored);
}

class FixtureSnapshot final : public Snapshot
{
public:
	Int value = 0x1020304;
	AsciiString name = "fixture-snapshot";
protected:
	void crc(Xfer *xfer) override { xfer->xferInt(&value); xfer->xferAsciiString(&name); }
	void xfer(Xfer *xfer) override { crc(xfer); }
	void loadPostProcess() override {}
};

bool loadINI(const std::filesystem::path& path)
{
	try {
		INI ini;
		ini.load(AsciiString(path.string().c_str()), INI_LOAD_OVERWRITE, NULL);
		return true;
	} catch (...) { return false; }
}
}

int main()
{
	std::array<char, 64> rootTemplate{};
	const std::string templatePath = (std::filesystem::temp_directory_path() / "zh-m20-data-startup-XXXXXX").string();
	std::copy(templatePath.begin(), templatePath.end(), rootTemplate.begin());
	const Char *createdRoot = ::mkdtemp(rootTemplate.data());
	if (!createdRoot) { std::cerr << "FAIL: cannot create isolated fixture root\n"; return 1; }
	const std::filesystem::path root(createdRoot);
	std::error_code ignored;
	const auto input = root / "readonly-input";
	const auto otherCwd = root / "unrelated-cwd";
	const auto xdgData = root / "xdg-data";
	const auto xdgCache = root / "xdg-cache";
	std::filesystem::create_directories(otherCwd);
	::setenv("HOME", (root / "home").c_str(), 1);
	::setenv("XDG_DATA_HOME", xdgData.c_str(), 1);
	::setenv("XDG_CACHE_HOME", xdgCache.c_str(), 1);
	::setenv("XDG_CONFIG_HOME", (root / "xdg-config").c_str(), 1);
	::setenv("XDG_STATE_HOME", (root / "xdg-state").c_str(), 1);

	writeBytes(input / "Data/English/Generals.csf", csfFixture());
	writeText(input / "Maps/MapCache.ini",
		"MapCache Maps\\Fixture\\Fixture.map\n"
		" fileSize = 1234\n fileCRC = 305419896\n timestampLo = 11\n timestampHi = 22\n"
		" isOfficial = yes\n isMultiplayer = yes\n numPlayers = 2\n"
		" extentMin = X:0 Y:0 Z:0\n extentMax = X:100 Y:80 Z:0\n"
		" nameLookupTag = MAP:Fixture\n Player_1_Start = X:10 Y:20 Z:0\n"
		" Player_2_Start = X:90 Y:60 Z:0\n techPosition = X:50 Y:40 Z:0\n"
		" supplyPosition = X:25 Y:30 Z:0\nEND\n");
	writeText(input / "Maps/Fixture/map.str", "MAP:FixtureLocal\n\"Map String Fixture\"\nEND\n");
	writeText(input / "Data/INI/GameData.ini", "GameData\n Windowed = Yes\n XResolution = 1280\n YResolution = 720\nEND\n");
	writeText(input / "Data/INI/Object.ini", "Object FixtureObject\n DisplayName = MAP:Fixture\n"
		" Side = Civilian\n BuildCost = 42\n VisionRange = 100\nEND\n");
	writeText(input / "Data/INI/UnknownProvider.ini", "Object BadFixtureObject\n"
		" MissingRequiredProvider = yes\nEND\n");
	writeText(input / "Data/Layer/10-root.ini", "OnlineChatColors\n Default = R:5 G:6 B:7 A:8\nEND\n");
	writeText(input / "Data/Layer/Sub/20-child.ini", "OnlineChatColors\n Default = R:9 G:10 B:11 A:12\nEND\n");
	auto malformedCsf = csfFixture();
	malformedCsf.resize(30);
	writeBytes(input / "Data/English/Bad.csf", malformedCsf);
	const auto inputSnapshot = snapshotFiles(input);
	setInputReadOnly(input, true);

	const auto oldCwd = std::filesystem::current_path();
	std::filesystem::current_path(otherCwd);
	initMemoryManager();
	PosixLocalFileSystem localFiles(input);
	FileSystem files;
	TheLocalFileSystem = &localFiles;
	TheFileSystem = &files;

	TheNameKeyGenerator = new NameKeyGenerator;
	TheNameKeyGenerator->init();
	const NameKeyType firstKey = TheNameKeyGenerator->nameToKey("FixtureName");
	check(firstKey != NAMEKEY_INVALID && TheNameKeyGenerator->keyToName(firstKey) == "FixtureName",
		"actual name-key generator round trip");

	check(loadINI(input / "Data/INI/GameData.ini"), "actual GameData callback loads");
	check(TheWritableGlobalData && TheWritableGlobalData->m_windowed &&
		TheWritableGlobalData->m_xResolution == 1280 && TheWritableGlobalData->m_yResolution == 720,
		"source-owned GlobalData values observed");
	check(std::string(TheWritableGlobalData->getPath_UserData().str()).find(xdgData.string()) == 0,
		"GlobalData user path is XDG data");

	TheGameText = CreateGameTextInterface();
	TheGameText->init();
	Bool exists = FALSE;
	const UnicodeString title = TheGameText->fetch("GUI:Command&ConquerGenerals", &exists);
	check(exists && title.getLength() == 17, "actual CSF localization loaded");
	TheGameText->initMapStringFile(AsciiString("Maps/Fixture/map.str"));
	const UnicodeString mapString = TheGameText->fetch("MAP:FixtureLocal", &exists);
	check(exists && mapString.getLength() == 18, "actual map.str localization loaded");
	TheThingFactory = new ThingFactory;
	TheThingFactory->init();
	check(loadINI(input / "Data/INI/Object.ini"), "actual configured object loads");
	const ThingTemplate *thing = TheThingFactory->firstTemplate();
	check(thing != NULL, "configured object exists in actual ThingFactory");
	if (thing) {
		check(thing->getName() == "FixtureObject", "configured object name observed");
		check(thing->friend_getBuildCost() == 42, "configured object build cost observed");
	}
	check(!loadINI(input / "Data/INI/UnknownProvider.ini") && !enteredExecuteOrReset,
		"unknown required object provider fails before execute/reset");
	try {
		INI layered;
		layered.loadDirectory(AsciiString("Data/Layer"), TRUE, INI_LOAD_OVERWRITE, NULL);
		check(GameSpyColor[GSCOLOR_DEFAULT] == GameMakeColor(9, 10, 11, 12),
			"actual two-pass INI directory order loads subdirectories after roots");
	} catch (...) { check(false, "actual two-pass INI directory load succeeds"); }

	TheMapCache = new MapCache;
	check(loadINI(input / "Maps/MapCache.ini"), "actual INIMapCache callback loads");
	const MapMetaData *metadata = TheMapCache->findMap(AsciiString("Maps\\Fixture\\Fixture.map"));
	check(metadata && metadata->m_CRC == 305419896U && metadata->m_numPlayers == 2 &&
		!metadata->m_techPositions.empty() && !metadata->m_supplyPositions.empty(),
		"nonempty source map metadata observed");
	check(TheMapCache->writeCacheINI(FALSE), "cold map cache write succeeds");
	const std::filesystem::path cachePath(TheMapCache->getCacheFilePath(FALSE).str());
	check(cachePath.string().find(xdgCache.string()) == 0 && std::filesystem::is_regular_file(cachePath),
		"generated map cache is XDG confined");
	MapCache warmCache;
	TheMapCache = &warmCache;
	check(loadINI(cachePath), "warm generated map cache reloads");
	const MapMetaData *warm = warmCache.findMap(AsciiString("Maps\\Fixture\\Fixture.map"));
	check(warm && warm->m_CRC == metadata->m_CRC && warm->m_numPlayers == metadata->m_numPlayers,
		"cold and warm map cache values match");
	warmCache[AsciiString("maps\\fixture\\fixture.map")].m_CRC = 0xabcdef01U;
	check(warmCache.writeCacheINI(FALSE), "stale map cache refresh succeeds");
	MapCache refreshedCache;
	TheMapCache = &refreshedCache;
	check(loadINI(cachePath) && refreshedCache.findMap(AsciiString("Maps\\Fixture\\Fixture.map"))->m_CRC == 0xabcdef01U,
		"stale generated cache is replaced by current source metadata");
	std::filesystem::remove(cachePath, ignored);
	check(!std::filesystem::exists(cachePath) && refreshedCache.writeCacheINI(FALSE) &&
		std::filesystem::is_regular_file(cachePath), "absent cache follows cold generation path");
	writeText(cachePath, "MapCache truncated\n fileCRC");
	MapCache malformedCache;
	TheMapCache = &malformedCache;
	check(!loadINI(cachePath) && !enteredExecuteOrReset, "malformed cache fails before execute/reset");
	TheMapCache = &refreshedCache;
	check(refreshedCache.writeCacheINI(FALSE), "valid cache restored after malformed input");

	FixtureSnapshot snapshot;
	XferCRC crc;
	crc.open(AsciiString("fixture"));
	crc.xferSnapshot(&snapshot);
	check(crc.getCRC() != 0, "actual Snapshot/XferCRC traversal produces a CRC");
	GameState gameState;
	TheGameState = &gameState;
	check(gameState.getSaveDirectory().startsWithNoCase(TheWritableGlobalData->getPath_UserData()),
		"actual GameState save path follows XDG GlobalData");

	const Char *savedCsf = g_csfFile;
	g_csfFile = "Data/English/Bad.csf";
	GameTextInterface *badText = CreateGameTextInterface();
	badText->init();
	badText->fetch("MAP:Fixture", &exists);
	check(!exists && !enteredExecuteOrReset, "malformed CSF fails before execute/reset");
	delete badText;
	g_csfFile = "Data/English/Missing.csf";
	GameTextInterface *missingText = CreateGameTextInterface();
	missingText->init();
	missingText->fetch("MAP:Fixture", &exists);
	check(!exists && !enteredExecuteOrReset, "missing CSF fails before execute/reset");
	delete missingText;
	g_csfFile = savedCsf;

	::setenv("XDG_CACHE_HOME", "/proc/zh-m20-denied", 1);
	check(!warmCache.writeCacheINI(FALSE) && !enteredExecuteOrReset,
		"denied cache write fails deterministically before execute/reset");
	check(!loadINI(input / "Data/INI/Missing.ini") && !enteredExecuteOrReset,
		"missing INI fails before execute/reset");
	check(snapshotFiles(input) == inputSnapshot, "read-only input tree remains byte-size identical");

	TheMapCache = NULL;
	TheGameState = NULL;
	delete TheThingFactory;
	TheThingFactory = NULL;
	delete TheGameText;
	TheGameText = NULL;
	delete TheWritableGlobalData;
	delete TheNameKeyGenerator;
	TheNameKeyGenerator = NULL;
	TheFileSystem = NULL;
	TheLocalFileSystem = NULL;
	std::filesystem::current_path(oldCwd);
	setInputReadOnly(input, false);
	std::filesystem::remove_all(root, ignored);
	std::cout << "M20 original data startup: " << (failures ? "failed" : "ok") << '\n';
	return failures == 0 ? 0 : 1;
}
