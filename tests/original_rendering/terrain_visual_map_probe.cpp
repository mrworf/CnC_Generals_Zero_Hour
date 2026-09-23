#include "PreRTS.h"

#include "Common/GlobalData.h"
#include "W3DDevice/GameClient/HeightMap.h"
#include "W3DDevice/GameClient/W3DDisplay.h"
#include "W3DDevice/GameClient/W3DScene.h"
#include "W3DDevice/GameClient/W3DTerrainVisual.h"
#include "WW3D2/scene.h"
#include "WW3D2/camera.h"
#include "WW3D2/rinfo.h"
#include "w3d_shader_manager_cpu_types.h"
#include "W3DDevice/GameClient/W3DShaderManager.h"
#include "original_gpu_edge.h"
#include "zh/renderer/recording_device.h"

#include <cstdlib>
#include <cstdio>
#include <memory>
#include <stdexcept>
#include <string>

namespace {
void require(bool value, const char *message)
{
	if (!value) throw std::runtime_error(message);
}

unsigned draw_count(const std::string &trace)
{
	unsigned count = 0;
	for (std::size_t at = trace.find("draw pipeline="); at != std::string::npos;
		at = trace.find("draw pipeline=", at + 1)) ++count;
	return count;
}

void require_rolled_back(W3DTerrainVisual *visual, const char *message)
{
	require(!visual->getLogicHeightMap() && TheHeightMap && !TheHeightMap->getMap() &&
		!TheHeightMap->Peek_Scene(), message);
}
}

extern "C" void zh_probe_terrain_visual_map()
{
	const char *map_path = std::getenv("ZH_M22_TERRAIN_VISUAL_MAP");
	const char *bad_map_path = std::getenv("ZH_M22_TERRAIN_VISUAL_BAD_MAP");
	require(map_path && bad_map_path, "original terrain visual map fixture missing");
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
		require(TheHeightMap && !TheHeightMap->getMap() && !TheHeightMap->Peek_Scene(),
			"original terrain visual init published a map");

		require(!visual->load(AsciiString("missing-generated-terrain.map")),
			"original terrain visual missing map succeeded");
		require_rolled_back(visual.get(), "original terrain visual missing map retained owner");

		bool malformed_rejected = false;
		try { visual->load(AsciiString(bad_map_path)); }
		catch (const std::runtime_error &) { malformed_rejected = true; }
		require(malformed_rejected, "original terrain visual malformed map accepted");
		require_rolled_back(visual.get(), "original terrain visual malformed map retained owner");

		device.fail_next_buffer_create();
		bool buffer_rejected = false;
		try { visual->load(AsciiString(map_path)); }
		catch (const std::runtime_error &) { buffer_rejected = true; }
		require(buffer_rejected, "original terrain visual buffer failure accepted");
		require_rolled_back(visual.get(), "original terrain visual buffer failure retained owner");
		require(device.resource_counts().total() == 0,
			"original terrain visual failed transaction retained Recording resource");

		require(visual->load(AsciiString(map_path)) && visual->getLogicHeightMap() &&
			TheHeightMap && TheHeightMap->getMap() == visual->getLogicHeightMap() &&
			TheHeightMap->Peek_Scene() == W3DDisplay::m_3DScene,
			"original terrain visual map transaction did not attach primary terrain");

		ShaderClass baseline;
		baseline.Set_Texturing(ShaderClass::TEXTURING_ENABLE);
		DX8Wrapper::Set_Shader(baseline);
		DX8Wrapper::Set_Material(NULL);
		DX8Wrapper::Set_Transform(D3DTS_WORLD, Matrix4x4(true));
		DX8Wrapper::Set_Transform(D3DTS_VIEW, Matrix4x4(true));
		DX8Wrapper::Set_Transform(D3DTS_PROJECTION, Matrix4x4(true));
		DX8Wrapper::Apply_Render_State_Changes();
		W3DShaderManager::init();
		CameraClass camera;
		RenderInfoClass render_info(camera);
		const std::string before_no_frame = device.snapshot();
		bool no_frame_rejected = false;
		try { TheHeightMap->Render(render_info); }
		catch (const std::runtime_error &) { no_frame_rejected = true; }
		require(no_frame_rejected && draw_count(device.snapshot().substr(before_no_frame.size())) == 0,
			"original terrain visual accepted a draw without source frame");

		zh::renderer::TextureDesc target;
		target.width = target.height = 32;
		target.render_target = true;
		target.sampled = false;
		target.format = zh::renderer::TextureFormat::bgra8;
		const auto color = device.create_texture(target, "original terrain visual color");
		target.format = zh::renderer::TextureFormat::depth24_stencil8;
		const auto depth = device.create_texture(target, "original terrain visual depth");
		const std::string before_draw = device.snapshot();
		edge.bind_frame_targets(color, depth, 32, 32);
		edge.begin_source_frame(true, true, 0, 0, 0, 1);
		try {
			TheHeightMap->Render(render_info);
			edge.end_source_frame(false);
		} catch (...) {
			edge.abort_source_frame();
			throw;
		}
		const std::string draws = device.snapshot().substr(before_draw.size());
		const std::string range = "count=6144 point_size=0.000000 index_bits=16 first_index=0 base_vertex=0";
		require(draw_count(draws) == 2 && draws.find(range) != std::string::npos &&
			draws.find(range, draws.find(range) + 1) != std::string::npos,
			"original terrain visual changed two-pass terrain submission");
		W3DShaderManager::shutdown();
		DX8Wrapper::Set_Vertex_Buffer(NULL);
		DX8Wrapper::Set_Index_Buffer(NULL, 0);
		device.destroy(depth);
		device.destroy(color);

		visual.reset();
		require(!TheTerrainVisual && !TheHeightMap && !TheTerrainRenderObject,
			"original terrain visual unload retained map owner");
		TheTerrainVisual = saved_visual;
		edge.release_source_buffers();
		display.reset();
	}
	require(device.resource_counts().total() == 0,
		"original terrain visual re-entry retained Recording resource");
	TheWritableGlobalData->m_partitionCellSize = saved_partition;
	std::puts("original terrain visual map: attach=1 rollback=3 draws=2 generations=2 resources=0");
}
