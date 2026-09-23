#include "PreRTS.h"

#include "Common/GlobalData.h"
#include "W3DDevice/GameClient/HeightMap.h"
#include "W3DDevice/GameClient/W3DDisplay.h"
#include "w3d_shader_manager_cpu_types.h"
#include "W3DDevice/GameClient/W3DShaderManager.h"
#include "W3DDevice/GameClient/W3DShroud.h"
#include "W3DDevice/GameClient/W3DTerrainVisual.h"
#include "WW3D2/camera.h"
#include "original_gpu_edge.h"
#include "zh/renderer/recording_device.h"

#include <cstdlib>
#include <cstdio>
#include <memory>
#include <stdexcept>

namespace {
void require(bool value, const char *message)
{
	if (!value) throw std::runtime_error(message);
}

template <typename Operation>
bool rejected(Operation operation)
{
	try {
		operation();
	} catch (const std::runtime_error &) {
		return true;
	}
	return false;
}
}

extern "C" void zh_probe_terrain_shroud_projection()
{
	const char *map_path = std::getenv("ZH_M22_TERRAIN_SHROUD_PROJECTION_MAP");
	require(map_path, "original terrain shroud projection map missing");
	const Real saved_partition = TheWritableGlobalData->m_partitionCellSize;
	TheWritableGlobalData->m_partitionCellSize = MAP_XY_FACTOR;
	zh::renderer::RecordingGpuDevice device;
	for (Int generation = 0; generation != 2; ++generation) {
		zh::original_runtime::OriginalGpuEdge edge(device);
		auto display = std::make_unique<W3DDisplay>();
		display->init();
		TerrainVisual *saved_visual = TheTerrainVisual;
		auto visual = std::make_unique<W3DTerrainVisual>();
		TheTerrainVisual = visual.get();
		visual->init();
		require(visual->load(AsciiString(map_path)) && TheTerrainRenderObject &&
			TheTerrainRenderObject->getShroud(),
			"original terrain shroud projection map load failed");
		W3DShroud *shroud = TheTerrainRenderObject->getShroud();
		CameraClass camera;
		const zh::renderer::ResourceCounts baseline = device.resource_counts();
		require(!shroud->getShroudTexture(),
			"original terrain shroud projection existed before render");

		require(rejected([&]() { shroud->render(NULL); }) && !shroud->getShroudTexture(),
			"original terrain shroud null camera published a texture");
		device.fail_next_texture_create();
		require(rejected([&]() { shroud->render(&camera); }) && !shroud->getShroudTexture() &&
			device.resource_counts() == baseline,
			"original terrain shroud allocation failure retained projection resource");
		W3DShroudMaterialPassClass material;
		require(rejected([&]() { material.Install_Materials(); }),
			"original terrain shroud material accepted no projection texture");

		shroud->render(&camera);
		TextureClass *first_texture = shroud->getShroudTexture();
		require(first_texture && device.resource_counts().textures == baseline.textures + 1,
			"original terrain shroud did not publish one projection texture");
		shroud->setShroudFilter(FALSE);
		require(rejected([&]() { shroud->render(&camera); }) &&
			shroud->getShroudTexture() == first_texture,
			"original terrain shroud filter rejection changed projection owner");
		shroud->setShroudFilter(TRUE);
		material.Install_Materials();
		material.UnInstall_Materials();

		shroud->ReleaseResources();
		require(!shroud->getShroudTexture() && device.resource_counts() == baseline,
			"original terrain shroud release retained projection resource");
		require(rejected([&]() { material.Install_Materials(); }),
			"original terrain shroud material accepted released resource");
		require(shroud->ReAcquireResources() && shroud->getShroudTexture(),
			"original terrain shroud re-acquire failed");
		shroud->render(&camera);
		material.Install_Materials();
		material.UnInstall_Materials();

		visual->reset();
		visual.reset();
		require(!TheTerrainVisual && !TheHeightMap && !TheTerrainRenderObject,
			"original terrain shroud visual teardown retained owner");
		TheTerrainVisual = saved_visual;
		edge.release_source_buffers();
		require(device.resource_counts().total() == 0,
			"original terrain shroud teardown retained Recording resource");
		display.reset();
	}
	require(device.resource_counts().total() == 0,
		"original terrain shroud projection re-entry retained Recording resource");
	TheWritableGlobalData->m_partitionCellSize = saved_partition;
	std::puts("original terrain shroud projection: material=1 rollback=2 generations=2 resources=0 draws=0");
}
