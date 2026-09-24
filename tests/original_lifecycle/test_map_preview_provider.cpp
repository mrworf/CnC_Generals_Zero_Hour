#include "PreRTS.h"

#include "Common/FileSystem.h"
#include "Common/ArchiveFileSystem.h"
#include "Common/GameState.h"
#include "Common/GlobalData.h"
#include "Common/NameKeyGenerator.h"
#include "GameClient/Image.h"
#include "GameClient/MapUtil.h"
#include "PosixDevice/Common/PosixLocalFileSystem.h"
#include "LinuxBIGArchive.h"
#include "zh/original_process.h"

#include <cstdio>
#include <cstdlib>
#include <filesystem>
#include <fstream>
#include <memory>
#include <unistd.h>

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
}

int main()
{
	char diagnostic[128]{};
	if (!zh::original_process::initialize_services(0, diagnostic, sizeof(diagnostic)))
		return std::fprintf(stderr, "%s\n", diagnostic), 1;
	initMemoryManager();
	char temporary[] = "/tmp/zh-e1c0b2a-preview-XXXXXX";
	char *created = mkdtemp(temporary);
	if (!created) return 1;
	const std::filesystem::path root(created);
	const std::filesystem::path xdg = root / "xdg";
	setenv("XDG_DATA_HOME", xdg.c_str(), 1);
	write_bytes(root / "Maps" / "Generated.tga", "generated-preview");
	write_bytes(root / "Maps" / "Retry.tga", "retry-preview");

	NameKeyGenerator nameKeys;
	TheNameKeyGenerator = &nameKeys;
	nameKeys.init();
	PosixLocalFileSystem localFiles(root);
	FileSystem files;
	std::unique_ptr<ArchiveFileSystem> archive(zh::original_runtime::createLinuxBIGArchiveFileSystem());
	TheLocalFileSystem = &localFiles;
	TheArchiveFileSystem = archive.get();
	archive->init();
	TheFileSystem = &files;
	check(getMapPreviewImage("Maps\\Generated.map") == NULL,
		"map preview accepted an absent GlobalData owner");
	for (int generation = 0; generation != 2; ++generation)
	{
		check(TheWritableGlobalData == NULL && TheGameState == NULL && TheMapCache == NULL &&
			TheMappedImageCollection == NULL, "stale map-preview owner survived generation boundary");
		{
		GlobalData globalData;
		TheWritableGlobalData = &globalData;
		GameState state;
		TheGameState = &state;
		MapCache cache;
		TheMapCache = &cache;
		ImageCollection images;
		TheMappedImageCollection = &images;

		Image *preview = getMapPreviewImage("Maps\\Generated.map");
		check(preview != NULL, "source map preview did not publish a generated descriptor");
		if (preview) {
			check(preview == images.findImageByName("maps_maps_generated"),
				"source map preview did not publish under its portable name");
			check(preview->getFilename() == "maps_maps_generated.tga",
				"source map preview generated filename changed");
			check(preview->getTextureSize()->x == 128 && preview->getTextureSize()->y == 128 &&
				preview->getRawTextureData() == NULL,
				"source map preview descriptor boundary changed");
		}
		check(getMapPreviewImage("maps\\generated.map") == preview && images.Enum(0) == preview &&
			images.Enum(1) == NULL, "source map preview reuse duplicated image ownership");
		check(getMapPreviewImage("Maps\\Missing.map") == NULL,
			"missing generated map preview was accepted");

		check(files.doesFileExist("Maps\\Retry.tga"),
			"generated retry source was unavailable before injected copy failure");
		std::filesystem::remove(root / "Maps" / "Retry.tga");
		check(getMapPreviewImage("Maps\\Retry.map") == NULL &&
			images.findImageByName("maps_maps_retry") == NULL,
			"source map preview copy failure retained image ownership");
		write_bytes(root / "Maps" / "Retry.tga", "retry-preview");
		check(getMapPreviewImage("Maps\\Retry.map") != NULL &&
			images.findImageByName("maps_maps_retry") != NULL,
			"source map preview did not retry after generated copy failure");

		}
		TheMappedImageCollection = NULL;
		TheMapCache = NULL;
		TheGameState = NULL;
		TheWritableGlobalData = NULL;
	}
	TheFileSystem = NULL;
	TheArchiveFileSystem = NULL;
	TheLocalFileSystem = NULL;
	TheNameKeyGenerator = NULL;
	std::filesystem::remove_all(root);
	unsetenv("XDG_DATA_HOME");
	zh::original_process::shutdown_services();
	shutdownMemoryManager();
	if (!failures) std::puts("original generated map preview: generations=2 images=0 pixels=0");
	return failures ? 1 : 0;
}
