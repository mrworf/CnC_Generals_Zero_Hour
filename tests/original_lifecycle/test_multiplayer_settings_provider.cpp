#include "PreRTS.h"

#include "Common/FileSystem.h"
#include "Common/INI.h"
#include "Common/MultiplayerSettings.h"
#include "Common/NameKeyGenerator.h"
#include "Common/PlayerTemplate.h"
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
	char temporary[] = "/tmp/zh-e1c0a2-multiplayer-XXXXXX";
	char *created = mkdtemp(temporary);
	if (!created) return 1;
	const std::filesystem::path root(created);
	const auto valid = root / "generated-valid.ini";
	const auto update = root / "generated-update.ini";
	const auto malformed = root / "generated-malformed.ini";
	write_generated(valid,
		"PlayerTemplate GeneratedFaction\n"
		" Side = GeneratedSide\n"
		" BaseSide = GeneratedBase\n"
		" PlayableSide = Yes\n"
		" PreferredColor = R:17 G:34 B:51\n"
		" StartingBuilding = GeneratedBuilding\n"
		"END\n"
		"MultiplayerSettings\n"
		" StartCountdownTimer = 7\n"
		" MaxBeaconsPerPlayer = 5\n"
		" UseShroud = No\n"
		" ShowRandomPlayerTemplate = No\n"
		" ShowRandomStartPos = No\n"
		" ShowRandomColor = No\n"
		"END\n"
		"MultiplayerColor GeneratedColor\n"
		" TooltipName = GeneratedColor\n"
		" RGBColor = R:17 G:34 B:51\n"
		" RGBNightColor = R:3 G:2 B:1\n"
		"END\n"
		"MultiplayerStartingMoneyChoice\n"
		" Value = 500\n"
		" Default = Yes\n"
		"END\n");
	write_generated(update,
		"PlayerTemplate GeneratedFaction\n"
		" Side = UpdatedSide\n"
		"END\n"
		"MultiplayerColor GeneratedColor\n"
		" TooltipName = GeneratedColor\n"
		" RGBColor = R:51 G:34 B:17\n"
		" RGBNightColor = R:1 G:2 B:3\n"
		"END\n");
	write_generated(malformed,
		"PlayerTemplate BrokenFaction\n"
		" UnsupportedGeneratedField = reject\n"
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
		check(ThePlayerTemplateStore == NULL && TheMultiplayerSettings == NULL,
			"stale multiplayer providers survived generation boundary");
		PlayerTemplateStore templates;
		templates.init();
		ThePlayerTemplateStore = &templates;
		check(load_generated(valid), "generated multiplayer provider bundle did not parse");
		const PlayerTemplate *templateValue = templates.findPlayerTemplate(NAMEKEY("GeneratedFaction"));
		check(templateValue && templateValue->getSide() == "GeneratedSide" &&
			templateValue->getStartingBuilding() == "GeneratedBuilding" &&
			templateValue->getMoney()->countMoney() == 0,
			"generated PlayerTemplate source fields changed");
		check(templates.getPlayerTemplateCount() == 1 && templates.getNthPlayerTemplate(-1) == NULL &&
			templates.findPlayerTemplate(NAMEKEY("MissingGeneratedFaction")) == NULL,
			"source PlayerTemplate missing/out-of-range controls changed");
		check(TheMultiplayerSettings && TheMultiplayerSettings->getNumColors() == 1 &&
			TheMultiplayerSettings->getColor(0) && TheMultiplayerSettings->getColor(1) == NULL &&
			TheMultiplayerSettings->getColor(0)->getTooltipName() == "GeneratedColor" &&
			TheMultiplayerSettings->getMaxBeaconsPerPlayer() == 5 &&
			TheMultiplayerSettings->getDefaultStartingMoney().countMoney() == 500,
			"generated MultiplayerSettings/color/default-money values changed");
		check(!TheMultiplayerSettings->showRandomPlayerTemplate() && !TheMultiplayerSettings->showRandomStartPos() &&
			!TheMultiplayerSettings->showRandomColor() && !TheMultiplayerSettings->isShroudInMultiplayer(),
			"generated MultiplayerSettings boolean values changed");
		check(load_generated(update) && templates.getPlayerTemplateCount() == 1 &&
			templates.getNthPlayerTemplate(0)->getSide() == "UpdatedSide" &&
			TheMultiplayerSettings->getNumColors() == 1 && TheMultiplayerSettings->getColor(0)->getRGBValue().red == (51.0f / 255.0f),
			"same-name source provider update lost identity or duplicated state");
		check(!load_generated(malformed) && templates.findPlayerTemplate(NAMEKEY("BrokenFaction")) == NULL,
			"malformed PlayerTemplate input was accepted or retained ownership");
		delete TheMultiplayerSettings;
		TheMultiplayerSettings = NULL;
		ThePlayerTemplateStore = NULL;
	}
	TheFileSystem = NULL;
	TheLocalFileSystem = NULL;
	TheNameKeyGenerator = NULL;
	std::filesystem::remove_all(root);
	zh::original_process::shutdown_services();
	shutdownMemoryManager();
	if (!failures) std::puts("original generated multiplayer settings: generations=2 templates=0 colors=0");
	return failures ? 1 : 0;
}
