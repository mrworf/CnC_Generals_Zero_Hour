#include "PreRTS.h"

#include "Common/GameEngine.h"
#include "Common/FileSystem.h"
#include "Common/INI.h"
#include "Common/INIException.h"
#include "Common/GameMemory.h"
#include "Common/SubsystemInterface.h"
#include "zh/original_process.h"
#include "zh/foundation/platform.h"

#include <cstdio>
#include <cstdlib>
#include <cstring>
#include <exception>
#include <filesystem>
#include <optional>
#include <stdexcept>
#include <string>

extern "C" Bool zh_linux_lifecycle_report(UnsignedInt *values, std::size_t count);
extern "C" Int zh_linux_benchmark_timer();
extern "C" UnsignedInt zh_linux_device_acquisition_attempts();

namespace {

void prepare_xdg_roots()
{
	const auto lookup = [](std::string_view name) -> std::optional<std::string> {
		const std::string key(name);
		const char *value = std::getenv(key.c_str());
		return value ? std::optional<std::string>(value) : std::nullopt;
	};
	const zh::foundation::XdgPaths paths = zh::foundation::resolve_xdg_paths(lookup);
	for (const std::filesystem::path *path : {&paths.config, &paths.data, &paths.state, &paths.cache})
	{
		std::error_code error;
		std::filesystem::create_directories(*path, error);
		if (error)
			throw std::runtime_error("cannot create XDG output directory '" + path->string() + "': " + error.message());
	}
}

void load_ini(const char *path, INILoadType type, Xfer *xfer)
{
	try
	{
		INI ini;
		ini.load(AsciiString(path), type, xfer);
	}
	catch (const INIException& error)
	{
		throw std::runtime_error(error.mFailureMessage ? error.mFailureMessage :
			(std::string("failed to parse INI: ") + path));
	}
	catch (const std::exception& error)
	{
		throw std::runtime_error(std::string("failed to load INI ") + path + ": " + error.what());
	}
	catch (...)
	{
		throw std::runtime_error(std::string("failed to load INI: ") + path);
	}
}

void load_subsystem_ini(const char *defaults, const char *overrides,
	const char *directory, Xfer *xfer)
{
	Bool loaded = FALSE;
	if (defaults)
	{
		load_ini(defaults, INI_LOAD_OVERWRITE, xfer);
		loaded = TRUE;
	}
	if (overrides && TheFileSystem->doesFileExist(overrides))
	{
		load_ini(overrides, loaded ? INI_LOAD_CREATE_OVERRIDES : INI_LOAD_OVERWRITE, xfer);
		loaded = TRUE;
	}
	else if (overrides && !loaded)
	{
		// A subsystem with no shipped default names its sole required input in
		// the second slot (for example Rank.ini and Weapon.ini).
		throw std::runtime_error(std::string("missing required INI: ") + overrides);
	}
	if (directory)
	{
		try
		{
			INI ini;
			ini.loadDirectory(AsciiString(directory), TRUE,
				loaded ? INI_LOAD_CREATE_OVERRIDES : INI_LOAD_OVERWRITE, xfer);
		}
		catch (const INIException& error)
		{
			throw std::runtime_error(error.mFailureMessage ? error.mFailureMessage :
				(std::string("failed to parse INI directory: ") + directory));
		}
		catch (...)
		{
			throw std::runtime_error(std::string("failed to load INI directory: ") + directory);
		}
	}
}

}

int main(int argc, char **argv)
{
	Bool cacheBuild = FALSE;
	for (int i = 1; i < argc; ++i)
		if (std::strcmp(argv[i], "-buildmapcache") == 0)
			cacheBuild = TRUE;
	char diagnostic[512]{};
	try
	{
		prepare_xdg_roots();
	}
	catch (const std::exception& error)
	{
		std::fprintf(stderr, "original startup failed: %s\n", error.what());
		return 2;
	}
	if (!zh::original_process::initialize_services(0, diagnostic, sizeof(diagnostic)))
	{
		std::fprintf(stderr, "original startup failed: %s\n", diagnostic);
		return 2;
	}
	const std::size_t allocationBaseline = zh::original_process::live_pool_allocations();
	installSubsystemINIDataLoader(load_subsystem_ini);
	int result = 0;
	try
	{
		GameMain(argc, argv);
	}
	catch (const std::exception& error)
	{
		std::fprintf(stderr, "original startup failed: %s\n", error.what());
		result = 3;
	}
	catch (...)
	{
		std::fprintf(stderr, "original startup failed: unknown original runtime exception\n");
		result = 3;
	}
	installSubsystemINIDataLoader(nullptr);
	UnsignedInt lifecycle[8]{};
	if (result == 0 && std::getenv("ZH_M20_HEADLESS_PROFILE"))
	{
		const Bool lifecycleComplete = zh_linux_lifecycle_report(lifecycle, 8);
		if (cacheBuild && lifecycleComplete)
		{
			std::fprintf(stderr, "original startup failed: cache-build entered execute/reset\n");
			result = 4;
		}
		else if (!cacheBuild && !lifecycleComplete)
		{
			std::fprintf(stderr, "original startup failed: bounded lifecycle did not complete\n");
			result = 4;
		}
		else if (zh::original_process::live_pool_allocations() != allocationBaseline)
		{
			std::fprintf(stderr, "original startup failed: live allocations did not return to baseline (%zu != %zu)\n",
				zh::original_process::live_pool_allocations(), allocationBaseline);
			TheMemoryPoolFactory->writeLiveAllocationReport(stderr);
			result = 4;
		}
		else if (zh_linux_device_acquisition_attempts() != 0)
		{
			std::fprintf(stderr, "original startup failed: bounded lifecycle attempted physical device acquisition\n");
			result = 4;
		}
	}
	zh::original_process::shutdown_services();
	const zh::original_process::ServiceCounts services = zh::original_process::service_counts();
	if (result == 0 && (services.synchronization != 0 || services.logging != 0 ||
		services.version != 0 || services.workers != 0))
	{
		std::fprintf(stderr, "original startup failed: process services did not return to baseline\n");
		result = 4;
	}
	shutdownMemoryManager();
	if (result == 0 && std::getenv("ZH_M20_HEADLESS_PROFILE"))
	{
		if (cacheBuild)
			std::printf("original production early-exit: buildmapcache allocations=0 workers=0 devices=0\n");
		else
			std::printf("original production lifecycle: logic=%u>%u>reset:%u>%u client=%u>%u>reset:%u>%u benchmark=%d allocations=0 workers=0 devices=0\n",
				lifecycle[0], lifecycle[1], lifecycle[4], lifecycle[6],
				lifecycle[2], lifecycle[3], lifecycle[5], lifecycle[7], zh_linux_benchmark_timer());
	}
	return result;
}
