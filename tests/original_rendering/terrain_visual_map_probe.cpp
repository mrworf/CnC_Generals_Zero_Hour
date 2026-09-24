#include "PreRTS.h"

#include "Common/GlobalData.h"
#include "Common/ThingFactory.h"
#include "Common/ThingTemplate.h"
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
#include <limits>
#include <stdexcept>
#include <string>

namespace {
void require(bool value, const char *message)
{
	if (!value) throw std::runtime_error(message);
}

template <typename Operation>
bool rejected(Operation operation)
{
	try { operation(); }
	catch (const std::runtime_error &) { return true; }
	return false;
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
	const ThingTemplate *plain_prop = TheThingFactory->findTemplate(AsciiString("FixtureProp"), FALSE);
	const ThingTemplate *modeled_prop = TheThingFactory->findTemplate(AsciiString("ModeledProp"), FALSE);
	require(plain_prop && modeled_prop && plain_prop->getDrawModuleInfo().getCount() == 0 &&
		modeled_prop->getDrawModuleInfo().getCount() > 0,
		"original terrain prop generated templates unavailable");
	Coord3D prop_pos;
	prop_pos.set(20.0f, 20.0f, 0.0f);
	zh::renderer::RecordingGpuDevice device;
	for (Int generation = 0; generation != 2; ++generation) {
		zh::original_runtime::OriginalGpuEdge edge(device);
		auto display = std::make_unique<W3DDisplay>();
		display->init();
		Display *saved_display = TheDisplay;
		TheDisplay = display.get();
		TerrainVisual *saved_visual = TheTerrainVisual;
		auto visual = std::make_unique<W3DTerrainVisual>();
		TheTerrainVisual = visual.get();
		visual->init();
		require(TheHeightMap && !TheHeightMap->getMap() && !TheHeightMap->Peek_Scene(),
			"original terrain visual init published a map");
		require(rejected([&]() { visual->addProp(plain_prop, &prop_pos, 0.0f); }),
			"original terrain prop accepted a missing map");
		const auto owner_resources = device.resource_counts().total();

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
		require(device.resource_counts().total() == owner_resources,
			"original terrain visual failed transaction retained bounded smudge resource");

		require(visual->load(AsciiString(map_path)) && visual->getLogicHeightMap() &&
			TheHeightMap && TheHeightMap->getMap() == visual->getLogicHeightMap() &&
			TheHeightMap->Peek_Scene() == W3DDisplay::m_3DScene,
			"original terrain visual map transaction did not attach primary terrain");
		const auto prop_resources = device.resource_counts();
		const auto prop_refs = TheHeightMap->Num_Refs();
		visual->addProp(plain_prop, &prop_pos, 0.0f);
		visual->addProp(plain_prop, &prop_pos, 1.0f);
		require(TheHeightMap->Num_Refs() == prop_refs && device.resource_counts() == prop_resources,
			"original no-model terrain prop changed source owners or Recording resources");
		require(rejected([&]() { visual->addProp(modeled_prop, &prop_pos, 0.0f); }),
			"original modeled terrain prop bypassed pending producer");
		Coord3D malformed_pos = prop_pos;
		malformed_pos.x = std::numeric_limits<Real>::quiet_NaN();
		require(rejected([&]() { visual->addProp(NULL, &prop_pos, 0.0f); }) &&
			rejected([&]() { visual->addProp(plain_prop, NULL, 0.0f); }) &&
			rejected([&]() { visual->addProp(plain_prop, &malformed_pos, 0.0f); }) &&
			rejected([&]() { visual->addProp(plain_prop, &prop_pos,
				std::numeric_limits<Real>::infinity()); }),
			"original terrain prop accepted malformed input");
		TheDisplay = saved_display;
		const bool missing_display = rejected([&]() { visual->addProp(plain_prop, &prop_pos, 0.0f); });
		TheDisplay = display.get();
		TheTerrainVisual = saved_visual;
		const bool missing_visual = rejected([&]() { visual->addProp(plain_prop, &prop_pos, 0.0f); });
		TheTerrainVisual = visual.get();
		BaseHeightMapRenderObjClass *saved_terrain = TheTerrainRenderObject;
		TheTerrainRenderObject = NULL;
		const bool missing_terrain = rejected([&]() { visual->addProp(plain_prop, &prop_pos, 0.0f); });
		TheTerrainRenderObject = saved_terrain;
		HeightMapRenderObjClass *saved_height = TheHeightMap;
		TheHeightMap = NULL;
		const bool missing_height = rejected([&]() { visual->addProp(plain_prop, &prop_pos, 0.0f); });
		TheHeightMap = saved_height;
		W3DAssetManager *saved_assets = W3DDisplay::m_assetManager;
		W3DDisplay::m_assetManager = NULL;
		const bool missing_assets = rejected([&]() { visual->addProp(plain_prop, &prop_pos, 0.0f); });
		W3DDisplay::m_assetManager = saved_assets;
		GlobalData *saved_global_data = TheWritableGlobalData;
		TheWritableGlobalData = NULL;
		const bool missing_global_data = rejected([&]() { visual->addProp(plain_prop, &prop_pos, 0.0f); });
		TheWritableGlobalData = saved_global_data;
		W3DDisplay::m_3DScene->Remove_Render_Object(TheHeightMap);
		const bool missing_scene = rejected([&]() { visual->addProp(plain_prop, &prop_pos, 0.0f); });
		W3DDisplay::m_3DScene->Add_Render_Object(TheHeightMap);
		require(missing_display && missing_visual && missing_terrain && missing_height &&
			missing_assets && missing_global_data && missing_scene &&
			device.resource_counts() == prop_resources &&
			TheHeightMap->Num_Refs() == prop_refs,
			"original terrain prop accepted removed provider or changed owner");
		visual->addProp(plain_prop, &prop_pos, 0.0f);
		require(device.resource_counts() == prop_resources,
			"original terrain prop provider retry created Recording resources");

		ShaderClass baseline;
		baseline.Set_Texturing(ShaderClass::TEXTURING_ENABLE);
		DX8Wrapper::Set_Shader(baseline);
		DX8Wrapper::Set_Material(NULL);
		DX8Wrapper::Set_Transform(D3DTS_WORLD, Matrix4x4(true));
		DX8Wrapper::Set_Transform(D3DTS_VIEW, Matrix4x4(true));
		DX8Wrapper::Set_Transform(D3DTS_PROJECTION, Matrix4x4(true));
		DX8Wrapper::Apply_Render_State_Changes();
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
		DX8Wrapper::Set_Vertex_Buffer(NULL);
		DX8Wrapper::Set_Index_Buffer(NULL, 0);
		device.destroy(depth);
		device.destroy(color);

		visual.reset();
		require(!TheTerrainVisual && !TheHeightMap && !TheTerrainRenderObject,
			"original terrain visual unload retained map owner");
		TheTerrainVisual = saved_visual;
		edge.release_source_buffers();
		TheDisplay = saved_display;
		display.reset();
	}
	require(device.resource_counts().total() == 0,
		"original terrain visual re-entry retained Recording resource");
	TheWritableGlobalData->m_partitionCellSize = saved_partition;
	std::puts("original terrain visual map: attach=1 rollback=3 draws=2 props=2 generations=2 resources=0");
}
