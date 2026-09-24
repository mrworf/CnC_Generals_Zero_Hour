#include "PreRTS.h"

#include "Common/ArchiveFileSystem.h"
#include "Common/FileSystem.h"
#include "Common/GameState.h"
#include "Common/GlobalData.h"
#include "Common/NameKeyGenerator.h"
#include "GameClient/GameWindow.h"
#include "GameClient/GameWindowManager.h"
#include "GameClient/Image.h"
#include "GameClient/MapUtil.h"
#include "LinuxBIGArchive.h"
#include "PosixDevice/Common/PosixLocalFileSystem.h"
#include "zh/original_process.h"

#include <cstdio>
#include <cstdlib>
#include <filesystem>
#include <fstream>
#include <memory>
#include <unistd.h>

void positionStartSpots(AsciiString, GameWindow *[], GameWindow *);

namespace {
int failures;
void check(bool condition, const char *message)
{
	if (!condition) { std::fprintf(stderr, "FAIL: %s\n", message); ++failures; }
}
void write_bytes(const std::filesystem::path &path, const char *bytes)
{
	std::filesystem::create_directories(path.parent_path());
	std::ofstream file(path, std::ios::binary);
	file << bytes;
}
class Window final : public GameWindow { public: void winDrawBorder() override {} };
class Windows final : public GameWindowManager {
public:
	GameWindow *allocateNewWindow() override { return NULL; }
#define DRAW(name) GameWinDrawFunc name() override { return NULL; }
	DRAW(getPushButtonImageDrawFunc) DRAW(getPushButtonDrawFunc) DRAW(getCheckBoxImageDrawFunc)
	DRAW(getCheckBoxDrawFunc) DRAW(getRadioButtonImageDrawFunc) DRAW(getRadioButtonDrawFunc)
	DRAW(getTabControlImageDrawFunc) DRAW(getTabControlDrawFunc) DRAW(getListBoxImageDrawFunc)
	DRAW(getListBoxDrawFunc) DRAW(getComboBoxImageDrawFunc) DRAW(getComboBoxDrawFunc)
	DRAW(getHorizontalSliderImageDrawFunc) DRAW(getHorizontalSliderDrawFunc)
	DRAW(getVerticalSliderImageDrawFunc) DRAW(getVerticalSliderDrawFunc)
	DRAW(getProgressBarImageDrawFunc) DRAW(getProgressBarDrawFunc) DRAW(getStaticTextImageDrawFunc)
	DRAW(getStaticTextDrawFunc)
#undef DRAW
	GameWinDrawFunc getTextEntryImageDrawFunc() override { return NULL; }
	GameWinDrawFunc getTextEntryDrawFunc() override { return NULL; }
};
MapMetaData generated_map()
{
	MapMetaData result;
	result.m_numPlayers = 2;
	result.m_isMultiplayer = TRUE;
	result.m_extent.lo.x = 0.0f; result.m_extent.lo.y = 0.0f;
	result.m_extent.hi.x = 100.0f; result.m_extent.hi.y = 100.0f;
	Coord3D first; first.x = 10.0f; first.y = 20.0f;
	Coord3D second = first; second.x = 12.0f; second.y = 21.0f;
	result.m_waypoints["Player_1_Start"] = first;
	result.m_waypoints["Player_2_Start"] = second;
	Coord3D supply; supply.x = 25.0f; supply.y = 75.0f;
	Coord3D tech; tech.x = 75.0f; tech.y = 25.0f;
	result.m_supplyPositions.push_back(supply);
	result.m_techPositions.push_back(tech);
	return result;
}
}

int main()
{
	char diagnostic[128]{};
	if (!zh::original_process::initialize_services(0, diagnostic, sizeof(diagnostic)))
		return std::fprintf(stderr, "%s\n", diagnostic), 1;
	initMemoryManager();
	char temporary[] = "/tmp/zh-e1c0b2b2-placement-XXXXXX";
	char *created = mkdtemp(temporary);
	if (!created) return 1;
	const std::filesystem::path root(created);
	const std::filesystem::path xdg = root / "xdg";
	setenv("XDG_DATA_HOME", xdg.c_str(), 1);
	write_bytes(root / "Maps" / "Generated.tga", "generated-placement-preview");

	NameKeyGenerator keys; TheNameKeyGenerator = &keys; keys.init();
	PosixLocalFileSystem local_files(root); FileSystem files;
	std::unique_ptr<ArchiveFileSystem> archive(zh::original_runtime::createLinuxBIGArchiveFileSystem());
	TheLocalFileSystem = &local_files; TheArchiveFileSystem = archive.get(); archive->init(); TheFileSystem = &files;
	for (int generation = 0; generation != 2; ++generation) {
		check(TheWritableGlobalData == NULL && TheGameState == NULL && TheMapCache == NULL &&
			TheMappedImageCollection == NULL && TheWindowManager == NULL,
			"stale placement owner survived generation boundary");
		{
			GlobalData global; TheWritableGlobalData = &global;
			GameState state; TheGameState = &state;
			MapCache cache; TheMapCache = &cache; cache["maps\\generated.map"] = generated_map();
			ImageCollection images; TheMappedImageCollection = &images;
			Windows windows; TheWindowManager = &windows;
			Window map; map.winSetSize(200, 100); map.winSetPosition(11, 13);
			Window buttons[MAX_SLOTS]; GameWindow *markers[MAX_SLOTS]{};
			markers[0] = &buttons[0]; markers[1] = &buttons[1];
			buttons[0].winSetSize(10, 10); buttons[1].winSetSize(10, 10);

			positionStartSpots("Maps\\Generated.map", markers, &map);
			Int first_x, first_y, second_x, second_y;
			buttons[0].winGetScreenPosition(&first_x, &first_y);
			buttons[1].winGetScreenPosition(&second_x, &second_y);
			check(!buttons[0].winIsHidden() && !buttons[1].winIsHidden(),
				"source generated waypoints did not publish marker windows");
			check(first_x != second_x || first_y != second_y,
				"source overlapping waypoint placement did not resolve marker collision");
			check(map.winGetUserData() == cache.findMap("Maps\\Generated.map") &&
				map.winGetEnabledImage(0) == images.findImageByName("maps_maps_generated"),
				"source generated preview did not bind map owner/image");
			check(TheSupplyAndTechImageLocations.m_supplyPosList.size() == 1 &&
				TheSupplyAndTechImageLocations.m_techPosList.size() == 1,
				"source additional-image placement did not publish generated coordinates");

			map.winHide(TRUE); positionStartSpots("Maps\\Generated.map", markers, &map);
			check(TheSupplyAndTechImageLocations.m_supplyPosList.empty() &&
				TheSupplyAndTechImageLocations.m_techPosList.empty(),
				"hidden map window retained additional-image placement");
			map.winHide(FALSE); positionStartSpots("Missing.map", markers, &map);
			check(map.winGetUserData() == NULL && buttons[0].winIsHidden() && buttons[1].winIsHidden() &&
				TheSupplyAndTechImageLocations.m_supplyPosList.empty() && TheSupplyAndTechImageLocations.m_techPosList.empty(),
				"source missing-map placement fallback changed");
			positionStartSpots("Maps\\Generated.map", markers, &map);
			check(!buttons[0].winIsHidden() && !buttons[1].winIsHidden() &&
				TheSupplyAndTechImageLocations.m_supplyPosList.size() == 1 &&
				TheSupplyAndTechImageLocations.m_techPosList.size() == 1,
				"source placement retry after missing map changed");
			TheSupplyAndTechImageLocations.m_supplyPosList.clear();
			TheSupplyAndTechImageLocations.m_techPosList.clear();
		}
		TheWindowManager = NULL; TheMappedImageCollection = NULL; TheMapCache = NULL; TheGameState = NULL; TheWritableGlobalData = NULL;
		check(TheSupplyAndTechImageLocations.m_supplyPosList.empty() && TheSupplyAndTechImageLocations.m_techPosList.empty(),
			"source placement teardown retained generated coordinates");
	}
	TheFileSystem = NULL; TheArchiveFileSystem = NULL; TheLocalFileSystem = NULL; TheNameKeyGenerator = NULL;
	std::filesystem::remove_all(root); unsetenv("XDG_DATA_HOME");
	zh::original_process::shutdown_services(); shutdownMemoryManager();
	if (!failures) std::puts("original generated marker placement: generations=2 coordinates=0 pixels=0");
	return failures ? 1 : 0;
}
