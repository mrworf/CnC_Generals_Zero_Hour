#include "PreRTS.h"

#include "Common/FileSystem.h"
#include "Common/INI.h"
#include "Common/NameKeyGenerator.h"
#include "GameClient/Image.h"
#include "PosixDevice/Common/PosixLocalFileSystem.h"
#include "zh/original_process.h"

#include <cstdio>
#include <filesystem>
#include <fstream>
#include <string>
#include <unistd.h>

namespace {
int failures;

void check(bool condition, const char *message)
{
	if (!condition) { std::fprintf(stderr, "FAIL: %s\n", message); ++failures; }
}

bool load_generated(const std::filesystem::path &path)
{
	try
	{
		INI ini;
		ini.load(AsciiString(path.string().c_str()), INI_LOAD_OVERWRITE, NULL);
		return true;
	}
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
	{
		std::fprintf(stderr, "%s\n", diagnostic);
		return 1;
	}
	initMemoryManager();
	char template_path[] = "/tmp/zh-e1b1-mapped-image-XXXXXX";
	char *created_root = mkdtemp(template_path);
	if (!created_root)
		return 1;
	const std::filesystem::path root(created_root);
	const auto valid = root / "generated-valid.ini";
	const auto duplicate = root / "generated-duplicate.ini";
	const auto descriptor_only = root / "generated-descriptor-only.ini";
	const auto malformed = root / "generated-malformed.ini";
	write_generated(valid,
		"MappedImage GeneratedLoadImage\n"
		" Texture = Art/GeneratedLoadImage.tga\n"
		" TextureWidth = 8\n"
		" TextureHeight = 8\n"
		" Coords = Left:0 Top:0 Right:8 Bottom:8\n"
		"END\n");
	write_generated(duplicate,
		"MappedImage GeneratedLoadImage\n"
		" Texture = Art/GeneratedLoadImageSecond.tga\n"
		" TextureWidth = 16\n"
		" TextureHeight = 8\n"
		" Coords = Left:0 Top:0 Right:8 Bottom:8\n"
		"END\n");
	write_generated(descriptor_only,
		"MappedImage GeneratedDescriptorOnly\n"
		" Texture = Art/UnsupportedGeneratedPayload.bin\n"
		" TextureWidth = 0\n"
		" TextureHeight = 0\n"
		" Coords = Left:0 Top:0 Right:0 Bottom:0\n"
		"END\n");
	write_generated(malformed,
		"MappedImage MalformedGeneratedLoadImage\n"
		" UnknownGeneratedField = reject\n"
		"END\n");

	NameKeyGenerator name_keys;
	TheNameKeyGenerator = &name_keys;
	name_keys.init();
	PosixLocalFileSystem local_files(root);
	FileSystem files;
	TheLocalFileSystem = &local_files;
	TheFileSystem = &files;
	for (int generation = 0; generation != 2; ++generation)
	{
		ImageCollection collection;
		check(TheMappedImageCollection == NULL, "stale mapped-image provider survived generation boundary");
		TheMappedImageCollection = &collection;
		check(load_generated(valid), "generated mapped-image descriptor did not parse");
		const Image *image = collection.findImageByName(AsciiString("GeneratedLoadImage"));
		check(image != NULL, "generated mapped-image owner did not publish image");
		if (image)
		{
			check(image->getName() == "GeneratedLoadImage" &&
				image->getFilename() == "Art/GeneratedLoadImage.tga", "generated image identity/descriptor changed");
			check(image->getTextureSize()->x == 8 && image->getTextureSize()->y == 8 &&
				image->getImageWidth() == 8 && image->getImageHeight() == 8,
				"generated image dimensions changed");
			check(image->getRawTextureData() == NULL, "mapped-image descriptor allocated texture payload");
			check(collection.findImageByName(AsciiString("generatedloadimage")) == image &&
				collection.findImageByName(AsciiString("GENERATEDLOADIMAGE")) == image,
				"original mapped-image lookup lost case-insensitive identity");
		}
		check(collection.findImageByName(AsciiString("MissingGeneratedLoadImage")) == NULL,
			"missing generated mapped image was accepted");
		const Image *before_duplicate = image;
		check(load_generated(duplicate), "original duplicate mapped-image definition was not source-compatible");
		const Image *after_duplicate = collection.findImageByName(AsciiString("GeneratedLoadImage"));
		check(after_duplicate == before_duplicate, "duplicate mapped-image definition changed owner identity");
		check(after_duplicate && after_duplicate->getTextureSize()->x == 16 &&
			after_duplicate->getImageWidth() == 8, "duplicate mapped-image definition did not update source fields");
		check(load_generated(descriptor_only), "descriptor-only mapped-image definition was rejected before consumer ownership");
		const Image *descriptor = collection.findImageByName(AsciiString("GeneratedDescriptorOnly"));
		check(descriptor && descriptor->getFilename() == "Art/UnsupportedGeneratedPayload.bin" &&
			descriptor->getTextureSize()->x == 0 && descriptor->getTextureSize()->y == 0 &&
			descriptor->getRawTextureData() == NULL,
			"mapped-image provider changed its descriptor-only format/extent boundary");
		check(!load_generated(malformed), "malformed generated mapped-image definition was accepted");
		check(collection.findImageByName(AsciiString("MalformedGeneratedLoadImage")) != NULL,
			"source parser no longer preserves its pre-field malformed ownership boundary");
		check(!load_generated(root / "missing-generated.ini"), "missing generated mapped-image input was accepted");
		TheMappedImageCollection = NULL;
	}
	check(!load_generated(valid), "absent mapped-image provider accepted a generated definition");
	check(TheMappedImageCollection == NULL, "absent mapped-image provider acquired foreign ownership");
	TheFileSystem = NULL;
	TheLocalFileSystem = NULL;
	TheNameKeyGenerator = NULL;
	std::filesystem::remove_all(root);
	zh::original_process::shutdown_services();
	shutdownMemoryManager();
	if (failures == 0)
		std::puts("original generated mapped-image provider: ini=1 generations=2 descriptors=0 resources=0");
	return failures == 0 ? 0 : 1;
}
