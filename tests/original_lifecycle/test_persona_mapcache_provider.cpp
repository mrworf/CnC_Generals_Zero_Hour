#include "PreRTS.h"

#include "Common/FileSystem.h"
#include "Common/INI.h"
#include "Common/NameKeyGenerator.h"
#include "GameClient/ChallengeGenerals.h"
#include "GameClient/MapUtil.h"
#include "PosixDevice/Common/PosixLocalFileSystem.h"
#include "zh/original_process.h"

#include <cstdio>
#include <filesystem>
#include <fstream>
#include <unistd.h>

namespace {
int failures;

void check(bool condition, const char *message)
{
	if (!condition) { std::fprintf(stderr, "FAIL: %s\n", message); ++failures; }
}

bool load_generated(const std::filesystem::path &path)
{
	try { INI ini; ini.load(AsciiString(path.string().c_str()), INI_LOAD_OVERWRITE, NULL); return true; }
	catch (...) { return false; }
}

void write_generated(const std::filesystem::path &path, const char *contents)
{
	std::ofstream file(path);
	file << contents;
}
}

int main()
{
	char diagnostic[128]{};
	if (!zh::original_process::initialize_services(0, diagnostic, sizeof(diagnostic)))
		return std::fprintf(stderr, "%s\n", diagnostic), 1;
	initMemoryManager();
	char temporary[] = "/tmp/zh-e1c0b1-persona-mapcache-XXXXXX";
	char *created = mkdtemp(temporary);
	if (!created) return 1;
	const std::filesystem::path root(created);
	const auto valid = root / "generated-valid.ini";
	const auto update = root / "generated-update.ini";
	const auto malformed = root / "generated-malformed.ini";
	write_generated(valid,
		"ChallengeGenerals\n"
		" GeneralPersona0\n"
		"  StartsEnabled = Yes\n"
		"  BioNameString = GeneratedGeneral\n"
		"  Campaign = GeneratedCampaign\n"
		"  PlayerTemplate = GeneratedFaction\n"
		" END\n"
		"END\n"
		"MapCache Maps\\Generated\\Generated.map\n"
		" isOfficial = No\n"
		" isMultiplayer = Yes\n"
		" extentMin = X:0 Y:0 Z:0\n"
		" extentMax = X:100 Y:200 Z:0\n"
		" numPlayers = 2\n"
		" fileSize = 77\n"
		" fileCRC = 99\n"
		" timestampLo = 11\n"
		" timestampHi = 12\n"
		" Player_1_Start = X:10 Y:20 Z:0\n"
		" Player_2_Start = X:30 Y:40 Z:0\n"
		" InitialCameraPosition = X:50 Y:60 Z:0\n"
		"END\n");
	write_generated(update,
		"ChallengeGenerals\n"
		" GeneralPersona0\n"
		"  StartsEnabled = No\n"
		"  BioNameString = GeneratedGeneral\n"
		"  Campaign = GeneratedCampaign\n"
		"  PlayerTemplate = GeneratedFaction\n"
		" END\n"
		"END\n"
		"MapCache Maps\\Generated\\Generated.map\n"
		" isOfficial = Yes\n"
		" isMultiplayer = Yes\n"
		" extentMin = X:0 Y:0 Z:0\n"
		" extentMax = X:300 Y:400 Z:0\n"
		" numPlayers = 3\n"
		" fileSize = 88\n"
		" fileCRC = 100\n"
		" timestampLo = 13\n"
		" timestampHi = 14\n"
		" Player_1_Start = X:10 Y:20 Z:0\n"
		" Player_2_Start = X:30 Y:40 Z:0\n"
		" Player_3_Start = X:70 Y:80 Z:0\n"
		" InitialCameraPosition = X:90 Y:100 Z:0\n"
		"END\n");
	write_generated(malformed,
		"MapCache Maps\\Generated\\Broken.map\n"
		" numPlayers = definitely-not-a-number\n"
		"END\n");

	NameKeyGenerator nameKeys;
	TheNameKeyGenerator = &nameKeys;
	nameKeys.init();
	PosixLocalFileSystem localFiles(root);
	FileSystem files;
	TheLocalFileSystem = &localFiles;
	TheFileSystem = &files;
	for (int generation = 0; generation != 2; ++generation)
	{
		check(TheChallengeGenerals == NULL && TheMapCache == NULL,
			"stale persona or map cache provider survived generation boundary");
		ChallengeGenerals challenges;
		MapCache cache;
		TheChallengeGenerals = &challenges;
		TheMapCache = &cache;
		check(load_generated(valid), "generated persona/map metadata bundle did not parse");
		const GeneralPersona *persona = challenges.getGeneralByTemplateName("GeneratedFaction");
		check(persona && persona->isStartingEnabled() && persona->getBioName() == "GeneratedGeneral" &&
			persona->getCampaign() == "GeneratedCampaign" && challenges.getPlayerGeneralByCampaignName("GeneratedCampaign") == persona &&
			challenges.getGeneralByGeneralName("GeneratedGeneral") == persona,
			"generated ChallengeGenerals source fields or lookups changed");
		check(challenges.getGeneralByTemplateName("MissingGeneratedFaction") == NULL &&
			challenges.getGeneralByGeneralName("MissingGeneratedGeneral") == NULL,
			"ChallengeGenerals missing lookup was accepted");
		const MapMetaData *map = cache.findMap("MAPS\\GENERATED\\GENERATED.MAP");
		check(map && cache.size() == 1 && map->m_fileName == "maps\\generated\\generated.map" &&
			map->m_numPlayers == 2 && map->m_isMultiplayer && !map->m_isOfficial && map->m_filesize == 77 &&
			map->m_CRC == 99 && map->m_waypoints.size() == 3 &&
			map->m_waypoints.find("Player_1_Start") != map->m_waypoints.end() &&
			map->m_waypoints.find("Player_2_Start") != map->m_waypoints.end(),
			"generated MapCache metadata/source waypoint fields changed");
		check(cache.findMap("Maps\\Generated\\Missing.map") == NULL,
			"MapCache missing lookup was accepted");
		check(load_generated(update), "same-name generated persona/map update did not parse");
		persona = challenges.getGeneralByTemplateName("GeneratedFaction");
		map = cache.findMap("maps\\generated\\generated.map");
		check(persona && !persona->isStartingEnabled() && map && cache.size() == 1 && map->m_numPlayers == 3 &&
			map->m_isOfficial && map->m_waypoints.size() == 4 && map->m_extent.hi.x == 300.0f,
			"same-name persona/map update duplicated or lost source state");
		check(!load_generated(malformed) && cache.findMap("maps\\generated\\broken.map") == NULL,
			"malformed MapCache input was accepted or retained ownership");
		TheMapCache = NULL;
		TheChallengeGenerals = NULL;
	}
	TheFileSystem = NULL;
	TheLocalFileSystem = NULL;
	TheNameKeyGenerator = NULL;
	std::filesystem::remove_all(root);
	zh::original_process::shutdown_services();
	shutdownMemoryManager();
	if (!failures) std::puts("original generated persona/mapcache: generations=2 cache=0");
	return failures ? 1 : 0;
}
