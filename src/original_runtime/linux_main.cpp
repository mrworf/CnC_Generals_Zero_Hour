#include "PreRTS.h"

#include "Common/GameEngine.h"
#include "Common/GlobalData.h"
#include "Common/Errors.h"
#include "Common/FileSystem.h"
#include "Common/INI.h"
#include "Common/INIException.h"
#include "Common/GameMemory.h"
#include "Common/SubsystemInterface.h"
#include "zh/original_process.h"
#include "zh/foundation/platform.h"
#if defined(ZH_M22_FULL_DRAW_TEST)
#define ZH_WW3D_CPU_ONLY 1
#include "GameClient/GameClient.h"
#include "GameClient/Display.h"
#include "GameClient/TerrainVisual.h"
#include "GameClient/View.h"
#include "W3DDevice/GameClient/W3DDisplay.h"
#include "original_gpu_edge.h"
#include "zh/platform/bgfx_device.h"
#undef ZH_WW3D_CPU_ONLY
#endif

#include <cstdio>
#include <cstdlib>
#include <cstring>
#include <exception>
#include <filesystem>
#include <optional>
#include <memory>
#include <stdexcept>
#include <string>

extern "C" Bool zh_linux_lifecycle_report(UnsignedInt *values, std::size_t count);
extern "C" Bool zh_linux_scenario_setup_report(UnsignedInt *values, std::size_t count);
extern "C" Bool zh_linux_simulation_report(Int *values, std::size_t count);
extern "C" Bool zh_linux_reentry_report(Int *values, std::size_t count);
extern "C" Int zh_linux_benchmark_timer();
extern "C" UnsignedInt zh_linux_device_acquisition_attempts();
#if defined(ZH_M22_FULL_DRAW_TEST)
extern "C" Bool zh_linux_original_factory_counts(UnsignedInt *values, std::size_t count);
#endif

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
		try
		{
			load_ini(overrides, loaded ? INI_LOAD_CREATE_OVERRIDES : INI_LOAD_OVERWRITE, xfer);
		}
		catch (...)
		{
			// A failed installed GameData parse can already have linked an override.
			// The subsystem-list failure path deletes the root; unwind its layers first.
			if (std::strcmp(overrides, "Data\\INI\\GameData.ini") == 0 && TheWritableGlobalData)
				TheWritableGlobalData->reset();
			throw;
		}
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
			ini.loadDirectory(AsciiString(directory), TRUE, INI_LOAD_MULTIFILE, xfer);
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
#if defined(ZH_M22_FULL_DRAW_TEST)
	const bool originalFactoryProfile = std::getenv("ZH_M22_ORIGINAL_FACTORY_PROFILE") != nullptr;
	const bool graphicsProfile = originalFactoryProfile ||
		std::getenv("ZH_M22_GPU_DEVICE_ONLY_PROFILE") != nullptr;
	std::unique_ptr<zh::renderer::BgfxGpuDevice> originalDisplayDevice;
	std::unique_ptr<zh::original_runtime::OriginalGpuEdge> originalDisplayEdge;
	zh::renderer::TextureHandle originalDisplayColor;
	zh::renderer::TextureHandle originalDisplayDepth;
	std::size_t graphicsReadyAllocations = 0;
	std::size_t graphicsEngineAllocations = 0;
	std::size_t graphicsResidualAllocations = 0;
	UnsignedInt originalFactoryCounts[3]{};
	bool originalEmptyPixels = false;
	std::size_t originalChangedPixels = 0;
#endif
	try
	{
#if defined(ZH_M22_FULL_DRAW_TEST)
		if (graphicsProfile)
		{
			if (cacheBuild || (originalFactoryProfile &&
				std::getenv("ZH_M22_GPU_DEVICE_ONLY_PROFILE")))
				throw std::runtime_error("original graphics profile requires one normal startup mode");
			if (std::getenv("ZH_M22_FACTORY_FAIL_DEVICE"))
				throw std::runtime_error("forced original graphics device failure");
			zh::renderer::BgfxOptions options;
			options.shader_root = ZH_BGFX_SHADER_DIR;
			originalDisplayDevice = std::make_unique<zh::renderer::BgfxGpuDevice>(options);
			originalDisplayEdge = std::make_unique<zh::original_runtime::OriginalGpuEdge>(*originalDisplayDevice);
			zh::renderer::TextureDesc target;
			target.width = 800;
			target.height = 600;
			target.render_target = true;
			target.format = zh::renderer::TextureFormat::bgra8;
			originalDisplayColor = originalDisplayDevice->create_texture(target, "original factory color");
			target.format = zh::renderer::TextureFormat::depth24_stencil8;
			originalDisplayDepth = originalDisplayDevice->create_texture(target, "original factory depth");
			if (!originalDisplayColor || !originalDisplayDepth)
				throw std::runtime_error("original factory render targets unavailable");
			originalDisplayEdge->bind_frame_targets(originalDisplayColor, originalDisplayDepth, 800, 600);
			graphicsReadyAllocations = zh::original_process::live_pool_allocations();
		}
#endif
		GameMain(argc, argv);
	}
	catch (const std::exception& error)
	{
		std::fprintf(stderr, "original startup failed: %s\n", error.what());
		result = 3;
	}
	catch (ErrorCode error)
	{
		std::fprintf(stderr, "original startup failed: original error 0x%x\n", static_cast<unsigned>(error));
		result = 3;
	}
	catch (...)
	{
		std::fprintf(stderr, "original startup failed: unknown original runtime exception\n");
		result = 3;
	}
#if defined(ZH_M22_FULL_DRAW_TEST)
	if (graphicsProfile && originalDisplayDevice)
	{
		graphicsEngineAllocations = zh::original_process::live_pool_allocations();
		// GameMain has returned after deleting GameEngine, whose subsystem list
		// deletes GameClient in reverse order. Display::deleteViews then deletes
		// its tactical view. Only retire their non-owning aliases after all
		// owning slots and the engine/subsystem list are proven gone.
		if (TheGameEngine || TheSubsystemList || TheDisplay || TheTerrainVisual ||
			W3DDisplay::m_3DScene || W3DDisplay::m_2DScene ||
			W3DDisplay::m_3DInterfaceScene || W3DDisplay::m_assetManager)
		{
			std::fprintf(stderr, "original graphics factory left published owners after GameMain\n");
			result = 4;
		}
		else
		{
			TheGameClient = nullptr;
			TheTacticalView = nullptr;
		}
		if (!zh_linux_original_factory_counts(originalFactoryCounts, 3))
			result = 4;
		if (originalFactoryProfile && result == 0)
		{
			const auto pixels = originalDisplayDevice->readback_rgba(originalDisplayColor);
			const std::size_t center = (300U * 800U + 400U) * 4U;
			for (std::size_t i = 0; i < pixels.size(); i += 4)
				originalChangedPixels += pixels[i] != 0 || pixels[i + 1] != 0 || pixels[i + 2] != 0;
			originalEmptyPixels = pixels.size() == 800U * 600U * 4U &&
				pixels[center] == 0 && pixels[center + 1] == 0 &&
				pixels[center + 2] == 0 && pixels[center + 3] == 255;
			const bool rigidProfile = std::getenv("ZH_M22_FACTORY_RIGID_ASSET") != nullptr;
			if ((rigidProfile ? originalChangedPixels == 0 : !originalEmptyPixels) ||
				originalFactoryCounts[0] != 1 ||
				originalFactoryCounts[1] != 1 || originalFactoryCounts[2] != 1)
			{
				std::fprintf(stderr, "original graphics factory empty frame or owner identity failed\n");
				result = 4;
			}
		}
	}
	if (originalDisplayDevice)
	{
		if (originalDisplayDepth) originalDisplayDevice->destroy(originalDisplayDepth);
		if (originalDisplayColor) originalDisplayDevice->destroy(originalDisplayColor);
	}
	originalDisplayEdge.reset();
	originalDisplayDevice.reset();
	if (graphicsProfile)
		graphicsResidualAllocations = zh::original_process::live_pool_allocations();
	if (graphicsProfile && result != 0)
		std::fprintf(stderr, "original graphics rollback: residual=%zu baseline=%zu owners=%u\n",
			graphicsResidualAllocations, allocationBaseline,
			(TheGameClient || TheDisplay || TheTacticalView || TheTerrainVisual || W3DDisplay::m_3DScene ||
			W3DDisplay::m_2DScene || W3DDisplay::m_3DInterfaceScene ||
			W3DDisplay::m_assetManager) ? 1U : 0U);
#endif
	installSubsystemINIDataLoader(nullptr);
	UnsignedInt lifecycle[8]{};
	UnsignedInt scenario[8]{};
	Int simulation[14]{};
	Int reentry[11]{};
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
		else if (
#if defined(ZH_M22_FULL_DRAW_TEST)
			!graphicsProfile &&
#endif
			zh::original_process::live_pool_allocations() != allocationBaseline)
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
	if (result == 0 && std::getenv("ZH_M21_SCENARIO"))
	{
		if (!zh_linux_scenario_setup_report(scenario, 8))
		{
			std::fprintf(stderr, "original scenario failed: setup did not complete\n");
			result = 4;
		}
		else if (scenario[1] == 0 || scenario[2] == 0 || scenario[3] == 0 ||
			(scenario[4] == 0 && !std::getenv("ZH_M21_ALLOW_EMPTY_PROPS")) ||
			scenario[6] == 0 || scenario[7] == 0)
		{
			std::fprintf(stderr, "original scenario failed: source-owned setup witness is incomplete (%u,%u,%u,%u,%u,%u,%u,%u)\n",
				scenario[0], scenario[1], scenario[2], scenario[3], scenario[4], scenario[5], scenario[6], scenario[7]);
			result = 4;
		}
		else if (zh_linux_device_acquisition_attempts() != 0)
		{
			std::fprintf(stderr, "original scenario failed: physical device acquisition attempted\n");
			result = 4;
		}
	}
	if (result == 0 && std::getenv("ZH_M21_SIMULATION") &&
		!std::getenv("ZH_M24_SAVE_AT_START"))
	{
		if (!zh_linux_simulation_report(simulation, 14))
		{
			std::fprintf(stderr, "original simulation failed: checkpoint did not complete\n");
			result = 4;
		}
		else if (std::getenv("ZH_M24_RECORD_REPLAY") ?
			(simulation[1] <= simulation[0] || simulation[2] == 0 || simulation[3] == 0 ||
			 simulation[6] == 0 || simulation[7] == 0 || !simulation[9] ||
			 simulation[13] >= simulation[12]) :
			(simulation[1] <= simulation[0] || simulation[2] == 0 || simulation[3] == 0 ||
			 !simulation[8] || !simulation[9] || !simulation[10] || !simulation[11]))
		{
			std::fprintf(stderr, "original simulation failed: source-owned checkpoint is incomplete (%d,%d,%d,%d,%d,%d,%d,%d,%d,%d,%d,%d,%d,%d)\n",
				simulation[0], simulation[1], simulation[2], simulation[3], simulation[4], simulation[5],
				simulation[6], simulation[7], simulation[8], simulation[9], simulation[10], simulation[11], simulation[12], simulation[13]);
			result = 4;
		}
	}
	if (result == 0 && std::getenv("ZH_M21_REENTRY"))
	{
		if (!zh_linux_reentry_report(reentry, 11))
		{
			std::fprintf(stderr, "original re-entry failed: lifecycle checkpoint did not complete\n");
			result = 4;
		}
		else if (reentry[0] != 0 || reentry[1] != 0 || reentry[2] != 0 ||
			reentry[3] != 0 || reentry[4] != 0 || reentry[5] != 0 ||
			reentry[6] == reentry[7] || reentry[8] == 0 || reentry[9] == 0 || reentry[10] == 0)
		{
			std::fprintf(stderr, "original re-entry failed: reset/fresh-state witness is incomplete (%d,%d,%d,%d,%d,%d,%d,%d,%d,%d,%d)\n",
				reentry[0], reentry[1], reentry[2], reentry[3], reentry[4], reentry[5],
				reentry[6], reentry[7], reentry[8], reentry[9], reentry[10]);
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
#if defined(ZH_M22_FULL_DRAW_TEST)
		if (graphicsProfile)
			std::printf("original graphics bootstrap: mode=%s display=%u view=%u terrain=%u empty=%u changed=%zu aliases=%u ready=%zu engine=%zu residual=%zu baseline=%zu\n",
				originalFactoryProfile ? "original" : "device-only",
				originalFactoryCounts[0], originalFactoryCounts[1], originalFactoryCounts[2],
				originalEmptyPixels ? 1U : 0U, originalChangedPixels,
				(TheGameClient || TheTacticalView) ? 1U : 0U, graphicsReadyAllocations,
				graphicsEngineAllocations, graphicsResidualAllocations, allocationBaseline);
		else
#endif
		if (cacheBuild)
			std::printf("original production early-exit: buildmapcache allocations=0 workers=0 devices=0\n");
		else
			std::printf("original production lifecycle: logic=%u>%u>reset:%u>%u client=%u>%u>reset:%u>%u benchmark=%d allocations=0 workers=0 devices=0\n",
				lifecycle[0], lifecycle[1], lifecycle[4], lifecycle[6],
				lifecycle[2], lifecycle[3], lifecycle[5], lifecycle[7], zh_linux_benchmark_timer());
	}
	if (result == 0 && std::getenv("ZH_M21_SCENARIO"))
		std::printf("original scenario setup: mode=%u players=%u teams=%u objects=%u props=%u model-preloads=%u texture-preloads=%u recorder-controls=%u devices=0\n",
			scenario[0], scenario[1], scenario[2], scenario[3], scenario[4], scenario[5], scenario[6], scenario[7]);
	if (result == 0 && std::getenv("ZH_M21_SIMULATION"))
		std::printf("original simulation checkpoint: frames=%d>%d ai=%d scripts=%d position=%d>%d actor=%d target=%d moved=%d attacked=%d invalid-rejected=%d terminal=%d target-health=%d>%d\n",
			simulation[0], simulation[1], simulation[2], simulation[3], simulation[4], simulation[5],
			simulation[6], simulation[7], simulation[8], simulation[9], simulation[10], simulation[11], simulation[12], simulation[13]);
	if (result == 0 && std::getenv("ZH_M21_REENTRY"))
		std::printf("original re-entry checkpoint: reset-objects=%d reset-map-objects=%d reset-props=%d reset-model-preloads=%d reset-texture-preloads=%d reset-recorder-controls=%d positions=%d>%d second-objects=%d second-props=%d second-recorder-controls=%d devices=0\n",
			reentry[0], reentry[1], reentry[2], reentry[3], reentry[4], reentry[5],
			reentry[6], reentry[7], reentry[8], reentry[9], reentry[10]);
	return result;
}
