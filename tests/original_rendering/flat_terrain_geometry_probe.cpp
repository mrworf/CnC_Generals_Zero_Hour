#include "PreRTS.h"

#include "Common/GlobalData.h"
#include "Common/MapReaderWriterInfo.h"
#include "W3DDevice/GameClient/HeightMap.h"
#include "W3DDevice/GameClient/W3DDisplay.h"
#include "w3d_shader_manager_cpu_types.h"
#include "W3DDevice/GameClient/W3DShaderManager.h"
#include "WW3D2/camera.h"
#include "WW3D2/rinfo.h"
#include "original_gpu_edge.h"
#include "zh/renderer/recording_device.h"

#include <cstdlib>
#include <memory>
#include <stdexcept>
#include <string>

namespace {
void require(bool value, const char *message)
{
	if (!value) throw std::runtime_error(message);
}

class TestTerrain final : public HeightMapRenderObjClass
{
public:
	Int tile_count() const { return m_numVertexBufferTiles; }
	DX8IndexBufferClass *index() const { return m_indexBuffer; }
	DX8VertexBufferClass *vertices() const { return m_vertexBufferTiles ? m_vertexBufferTiles[0] : NULL; }
	const VERTEX_FORMAT *backup() const
	{
		return m_vertexBufferBackup ? reinterpret_cast<const VERTEX_FORMAT *>(m_vertexBufferBackup[0]) : NULL;
	}
};
}

extern "C" void zh_probe_flat_terrain_geometry()
{
	const char *path = std::getenv("ZH_M22_FLAT_TERRAIN_MAP");
	require(path, "original flat terrain map path missing");
	CachedFileInputStream input;
	require(input.open(AsciiString(path)), "original flat terrain map unreadable");
	WorldHeightMap *map = NEW_REF(WorldHeightMap, (&input, FALSE));
	input.close();
	const Real saved_partition = TheWritableGlobalData->m_partitionCellSize;
	TheWritableGlobalData->m_partitionCellSize = MAP_XY_FACTOR;

	zh::renderer::RecordingGpuDevice device;
	{
		zh::original_runtime::OriginalGpuEdge edge(device);
		auto display = std::make_unique<W3DDisplay>();
		display->init();
		TestTerrain terrain;
		device.fail_next_buffer_create();
		bool injected = false;
		try { terrain.initHeightData(8, 8, map, NULL, TRUE); }
		catch (...) { injected = true; }
		require(injected && !terrain.getMap() && map->Num_Refs() == 1 &&
			device.resource_counts().buffers == 0,
			"original flat terrain failed init published state");

		require(terrain.initHeightData(8, 8, map, NULL, TRUE) == 0 &&
			terrain.getMap() == map && map->Num_Refs() == 2 && terrain.tile_count() == 1 &&
			terrain.index() && terrain.index()->Get_Index_Count() == 6144 &&
			terrain.vertices() && terrain.vertices()->Get_Vertex_Count() == 4096 &&
			device.resource_counts().buffers == 2,
			"original flat terrain geometry ownership changed");
		const UnsignedShort *indices = terrain.index()->Get_CPU_Index_Buffer();
		const VERTEX_FORMAT *vertices = terrain.backup();
		require(indices[0] == 0 && indices[1] == 2 && indices[2] == 3 &&
			indices[3] == 0 && indices[4] == 1 && indices[5] == 2 &&
			indices[42] == 28 && vertices[28].x == 0 && vertices[28].y == 0 &&
			vertices[28].z == 0 &&
			vertices[0].x == 0 && vertices[0].y == 0 && vertices[0].z == 0 &&
			vertices[2].x == MAP_XY_FACTOR && vertices[2].y == MAP_XY_FACTOR &&
			vertices[2].z == 9 * MAP_HEIGHT_SCALE && vertices[0].u1 == 0 &&
			vertices[0].v1 == 0 && vertices[0].u2 == 0 && vertices[0].v2 == 0,
			"original flat terrain source topology changed");
		map->setRawHeight(0, 0, 5);
		require(terrain.updateBlock(0, 0, 1, 1, map, NULL) == 0 &&
			terrain.backup()[0].z == 5 * MAP_HEIGHT_SCALE &&
			device.resource_counts().buffers == 2,
			"original flat terrain bounded update changed ownership");
		bool invalid_update = false;
		try { terrain.updateBlock(0, 0, 8, 8, map, NULL); }
		catch (...) { invalid_update = true; }
		require(invalid_update, "original flat terrain invalid update accepted");
		require(terrain.freeMapResources() == 0 && !terrain.getMap() && map->Num_Refs() == 1,
			"original flat terrain free retained map");
		edge.release_source_buffers();
		require(device.resource_counts().buffers == 0,
			"original flat terrain edge retained freed buffers");
		map->setRawHeight(0, 0, 0);
		require(terrain.initHeightData(8, 8, map, NULL, TRUE) == 0 &&
			terrain.backup()[0].z == 0 && device.resource_counts().buffers == 2,
			"original flat terrain re-entry failed");
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
		bool missing_frame_rejected = false;
		try { terrain.Render(render_info); }
		catch (const std::runtime_error&) { missing_frame_rejected = true; }
		require(missing_frame_rejected && device.snapshot().find("draw pipeline=") == std::string::npos,
			"original base terrain draw accepted no caller-owned frame");

		zh::renderer::TextureDesc target;
		target.width = target.height = 32;
		target.render_target = true;
		target.sampled = false;
		target.format = zh::renderer::TextureFormat::bgra8;
		const auto color = device.create_texture(target, "original flat terrain color");
		target.format = zh::renderer::TextureFormat::depth24_stencil8;
		const auto depth = device.create_texture(target, "original flat terrain depth");
		auto draw = [&]() {
			edge.bind_frame_targets(color, depth, 32, 32);
			edge.begin_source_frame(true, true, 0, 0, 0, 1);
			try {
				terrain.Render(render_info);
				edge.end_source_frame(false);
			} catch (...) {
				edge.abort_source_frame();
				throw;
			}
		};
		const auto count_draws = [](const std::string &trace) {
			unsigned count = 0;
			for (std::size_t at = trace.find("draw pipeline="); at != std::string::npos;
				at = trace.find("draw pipeline=", at + 1)) ++count;
			return count;
		};
		const auto require_aborted_draw = [&](unsigned successful_passes, const char *message) {
			device.fail_draw_after(successful_passes);
			const std::string before = device.snapshot();
			bool rejected = false;
			try { draw(); } catch (const std::runtime_error &) { rejected = true; }
			const std::string aborted = device.snapshot().substr(before.size());
			require(rejected && count_draws(aborted) == successful_passes &&
				device.resource_counts().samplers == 0,
				message);
		};
		require_aborted_draw(0, "original base terrain pass-zero abort retained state");
		require_aborted_draw(1, "original base terrain pass-one abort retained state");
		const std::string before_retry = device.snapshot();
		draw();
		const std::string retry = device.snapshot().substr(before_retry.size());
		const auto first_selection = retry.find("original TextureClass::Apply stage=0 selected");
		const auto first_draw = retry.find("draw pipeline=");
		const auto second_selection = first_selection == std::string::npos ? std::string::npos :
			retry.find("original TextureClass::Apply stage=0 selected", first_selection + 1);
		const auto second_draw = first_draw == std::string::npos ? std::string::npos :
			retry.find("draw pipeline=", first_draw + 1);
		const std::string range = "count=6144 point_size=0.000000 index_bits=16 first_index=0 base_vertex=0";
		const auto first_range = retry.find(range);
		require(first_selection != std::string::npos && first_selection < first_draw &&
			second_selection != std::string::npos && first_draw < second_selection && second_selection < second_draw &&
			count_draws(retry) == 2 && first_range != std::string::npos &&
			retry.find(range, first_range + 1) != std::string::npos &&
			edge.texture_handle(map->getTerrainTexture()) == edge.texture_handle(map->getAlphaTerrainTexture()) &&
			device.last_draw_index_bytes().size() == 6144U * sizeof(UnsignedShort),
			"original base terrain did not submit both native terrain passes");
		const auto require_no_draw = [&](const char *message, const auto &operation) {
			const std::string before = device.snapshot();
			edge.bind_frame_targets(color, depth, 32, 32);
			edge.begin_source_frame(true, true, 0, 0, 0, 1);
			bool rejected = false;
			try { operation(); } catch (const std::runtime_error &) { rejected = true; edge.abort_source_frame(); }
			if (!rejected) edge.end_source_frame(false);
			require(device.snapshot().substr(before.size()).find("draw pipeline=") == std::string::npos &&
				(rejected || terrain.Is_Hidden()), message);
		};
		terrain.Set_Hidden(true);
		require_no_draw("original hidden terrain submitted a draw", [&] { terrain.Render(render_info); });
		terrain.Set_Hidden(false);
		terrain.doTextures(FALSE);
		require_no_draw("original texture-disabled terrain submitted a draw", [&] { terrain.Render(render_info); });
		terrain.doTextures(TRUE);
		const Bool saved_cloud_map = TheWritableGlobalData->m_useCloudMap;
		TheWritableGlobalData->m_useCloudMap = TRUE;
		require_no_draw("original deferred cloud terrain submitted a draw", [&] { terrain.Render(render_info); });
		TheWritableGlobalData->m_useCloudMap = saved_cloud_map;
		const Bool saved_light_map = TheWritableGlobalData->m_useLightMap;
		TheWritableGlobalData->m_useLightMap = TRUE;
		require_no_draw("original deferred light-map terrain submitted a draw", [&] { terrain.Render(render_info); });
		TheWritableGlobalData->m_useLightMap = saved_light_map;
		W3DShaderManager::shutdown();
		DX8Wrapper::Set_Vertex_Buffer(NULL);
		DX8Wrapper::Set_Index_Buffer(NULL, 0);
		terrain.freeMapResources();
		require_no_draw("original released terrain submitted a draw", [&] { terrain.Render(render_info); });
		edge.release_source_buffers();
		device.destroy(depth);
		device.destroy(color);
		display.reset();
		map->Release_Ref();
		map = NULL;
	}
	require(device.resource_counts().total() == 0,
		"original flat terrain teardown retained resource");
	TheWritableGlobalData->m_partitionCellSize = saved_partition;
	std::puts("original flat terrain geometry: cells=7x7 vb=4096 ib=6144 draws=2");
}
