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

WorldHeightMap *open_map(const char *environment)
{
	const char *path = std::getenv(environment);
	require(path, "original terrain map path missing");
	CachedFileInputStream input;
	require(input.open(AsciiString(path)), "original terrain map unreadable");
	WorldHeightMap *map = NEW_REF(WorldHeightMap, (&input, FALSE));
	input.close();
	return map;
}

class TestTerrain final : public HeightMapRenderObjClass
{
public:
	Int tile_count() const { return m_numVertexBufferTiles; }
	Int tile_columns() const { return m_numVBTilesX; }
	Int tile_rows() const { return m_numVBTilesY; }
	Int last_columns() const { return m_numBlockColumnsInLastVB; }
	Int last_rows() const { return m_numBlockRowsInLastVB; }
	DX8IndexBufferClass *index() const { return m_indexBuffer; }
	DX8VertexBufferClass *vertices(Int tile = 0) const
	{
		return m_vertexBufferTiles && tile >= 0 && tile < m_numVertexBufferTiles ?
			m_vertexBufferTiles[tile] : NULL;
	}
	DX8VertexBufferClass *detach_vertices(Int tile)
	{
		if (!m_vertexBufferTiles || tile < 0 || tile >= m_numVertexBufferTiles) return NULL;
		DX8VertexBufferClass *result = m_vertexBufferTiles[tile];
		m_vertexBufferTiles[tile] = NULL;
		return result;
	}
	void restore_vertices(Int tile, DX8VertexBufferClass *vertices)
	{
		require(m_vertexBufferTiles && tile >= 0 && tile < m_numVertexBufferTiles &&
			!m_vertexBufferTiles[tile], "original terrain tile restore rejected");
		m_vertexBufferTiles[tile] = vertices;
	}
	void set_last_tile_size(Int columns, Int rows)
	{
		m_numBlockColumnsInLastVB = columns;
		m_numBlockRowsInLastVB = rows;
	}
	Int extra_count() const { return m_numExtraBlendTiles; }
	Int extra_capacity() const { return m_extraBlendTilePositionsSize; }
	Int visible_extra_count() const { return m_numVisibleExtraBlendTiles; }
	Int extra_position(Int index) const
	{
		return index >= 0 && index < m_numExtraBlendTiles ? m_extraBlendTilePositions[index] : -1;
	}
	const VERTEX_FORMAT *backup(Int tile = 0) const
	{
		return m_vertexBufferBackup && tile >= 0 && tile < m_numVertexBufferTiles ?
			reinterpret_cast<const VERTEX_FORMAT *>(m_vertexBufferBackup[tile]) : NULL;
	}
};
}

extern "C" void zh_probe_flat_terrain_geometry()
{
	WorldHeightMap *map = open_map("ZH_M22_FLAT_TERRAIN_MAP");
	const Real saved_partition = TheWritableGlobalData->m_partitionCellSize;
	TheWritableGlobalData->m_partitionCellSize = MAP_XY_FACTOR;

	zh::renderer::RecordingGpuDevice device(256, 4096);
	{
		zh::original_runtime::OriginalGpuEdge edge(device);
		auto display = std::make_unique<W3DDisplay>();
		display->init();
		TestTerrain terrain;
		WorldHeightMap *inventory = open_map("ZH_M22_EXTRA_BLEND_TERRAIN_MAP");
		require(inventory->getTerrainTexture() && inventory->getAlphaTerrainTexture(),
			"original extra blend terrain atlas pair unavailable");
		device.fail_next_buffer_create();
		bool inventory_failure = false;
		try { terrain.initHeightData(8, 8, inventory, NULL, TRUE); }
		catch (...) { inventory_failure = true; }
		require(inventory_failure && !terrain.getMap() && inventory->Num_Refs() == 1 &&
			terrain.extra_count() == 0 && terrain.extra_capacity() == 0 &&
			device.resource_counts().buffers == 0,
			"original extra blend failed init published inventory");
		require(terrain.initHeightData(8, 8, inventory, NULL, TRUE) == 0 &&
			terrain.extra_count() == 8 && terrain.extra_capacity() == 8,
			"original extra blend inventory retry failed");
		for (Int index = 0; index != 8; ++index) {
			const Int expected = index < 7 ? (index | (2 << 16)) : (3 << 16);
			require(terrain.extra_position(index) == expected,
				"original extra blend inventory order changed");
		}
		float extra_u[4]{}, extra_v[4]{};
		UnsignedByte extra_alpha[4]{};
		Bool extra_flip = FALSE;
		Bool extra_cliff = FALSE;
		const Bool extra_present = inventory->getExtraAlphaUVData(0, 3, extra_u, extra_v, extra_alpha,
			&extra_flip, &extra_cliff);
		require(extra_present && inventory->isCliffMappedTexture(0, 3),
			"original cliff extra blend inventory entry changed");
		terrain.freeMapResources();
		require(terrain.extra_count() == 0 && terrain.extra_capacity() == 0,
			"original extra blend teardown retained inventory");
		edge.release_source_buffers();
		inventory->Release_Ref();
		inventory = NULL;

		const auto verify_shape = [&](const char *environment, Int columns, Int rows,
			Int last_columns, Int last_rows) {
			WorldHeightMap *shape = open_map(environment);
			require(terrain.initHeightData(shape->getDrawWidth(), shape->getDrawHeight(),
				shape, NULL, TRUE) == 0 && terrain.getMap() == shape &&
				terrain.tile_columns() == columns && terrain.tile_rows() == rows &&
				terrain.tile_count() == columns * rows &&
				terrain.last_columns() == last_columns && terrain.last_rows() == last_rows &&
				device.resource_counts().buffers == static_cast<std::size_t>(columns * rows + 1),
				"original multi-tile terrain partition changed");
			for (Int tile = 0; tile < terrain.tile_count(); ++tile)
				require(terrain.vertices(tile) && terrain.backup(tile),
					"original multi-tile terrain omitted owned storage");
			require(terrain.freeMapResources() == 0 && !terrain.getMap() && shape->Num_Refs() == 1,
				"original multi-tile shape teardown retained map");
			edge.release_source_buffers();
			require(device.resource_counts().buffers == 0,
				"original multi-tile shape teardown retained buffers");
			shape->Release_Ref();
		};
		verify_shape("ZH_M22_MULTI_TILE_X_MAP", 2, 1, 2, 7);
		verify_shape("ZH_M22_MULTI_TILE_Y_MAP", 1, 2, 7, 2);
		verify_shape("ZH_M22_MULTI_TILE_EXACT_MAP", 2, 2, 32, 32);

		WorldHeightMap *multi = open_map("ZH_M22_MULTI_TILE_TERRAIN_MAP");
		device.fail_buffer_upload_after(2);
		bool multi_failure = false;
		try {
			terrain.initHeightData(multi->getDrawWidth(), multi->getDrawHeight(), multi, NULL, TRUE);
		} catch (...) { multi_failure = true; }
		require(multi_failure && !terrain.getMap() && multi->Num_Refs() == 1 &&
			terrain.tile_count() == 0 && !terrain.index(),
			"original multi-tile upload failure published partial ownership");
		edge.release_source_buffers();
		require(device.resource_counts().buffers == 0,
			"original multi-tile upload failure retained buffers");
		require(terrain.initHeightData(35, 34, multi, NULL, TRUE) == 0 &&
			terrain.tile_columns() == 2 && terrain.tile_rows() == 2 &&
			terrain.tile_count() == 4 && terrain.last_columns() == 2 &&
			terrain.last_rows() == 1 && device.resource_counts().buffers == 5,
			"original multi-tile terrain retry failed");
		const VERTEX_FORMAT *tile_x = terrain.backup(1);
		const VERTEX_FORMAT *tile_y = terrain.backup(2);
		const VERTEX_FORMAT *tile_xy = terrain.backup(3);
		require(tile_x && tile_y && tile_xy &&
			tile_x[0].x == 32 * MAP_XY_FACTOR && tile_x[0].y == 0 &&
			tile_y[0].x == 0 && tile_y[0].y == 32 * MAP_XY_FACTOR &&
			tile_xy[0].x == 32 * MAP_XY_FACTOR && tile_xy[0].y == 32 * MAP_XY_FACTOR &&
			tile_xy[8].x == 0 && tile_xy[8].y == 0,
			"original multi-tile terrain seam or padding payload changed");
		multi->setRawHeight(33, 32, 201);
		require(terrain.updateBlock(33, 32, 34, 33, multi, NULL) == 0 &&
			terrain.backup(3)[4].z == 201 * MAP_HEIGHT_SCALE &&
			device.resource_counts().buffers == 5,
			"original multi-tile bounded update changed ownership");
		require(terrain.freeMapResources() == 0 && !terrain.getMap() && multi->Num_Refs() == 1,
			"original multi-tile free retained map");
		edge.release_source_buffers();
		require(device.resource_counts().buffers == 0,
			"original multi-tile edge retained freed buffers");
		multi->Release_Ref();
		multi = NULL;

		require(terrain.initHeightData(8, 8, map, NULL, TRUE) == 0 &&
			terrain.getMap() == map && map->Num_Refs() == 2 && terrain.tile_count() == 1 &&
			terrain.extra_count() == 0 && terrain.extra_capacity() == 0 &&
			terrain.index() && terrain.index()->Get_Index_Count() == 6144 &&
			terrain.vertices() && terrain.vertices()->Get_Vertex_Count() == 4096 &&
			device.resource_counts().buffers == 2,
			"original flat terrain geometry ownership changed");
		const UnsignedShort *indices = terrain.index()->Get_CPU_Index_Buffer();
		const VERTEX_FORMAT *vertices = terrain.backup();
		float flat_u[4]{}, flat_v[4]{}, flat_alpha_u[4]{}, flat_alpha_v[4]{};
		UnsignedByte flat_alpha[4]{};
		Bool flat_flip = FALSE;
		map->getUVData(0, 0, flat_u, flat_v, FALSE);
		map->getAlphaUVData(0, 0, flat_alpha_u, flat_alpha_v, flat_alpha, &flat_flip, FALSE);
		require(indices[0] == 0 && indices[1] == 2 && indices[2] == 3 &&
			indices[3] == 0 && indices[4] == 1 && indices[5] == 2 &&
			indices[42] == 28 && vertices[28].x == 0 && vertices[28].y == 0 &&
			vertices[28].z == 0 &&
			vertices[0].x == 0 && vertices[0].y == 0 && vertices[0].z == 0 &&
			vertices[2].x == MAP_XY_FACTOR && vertices[2].y == MAP_XY_FACTOR &&
			vertices[2].z == 9 * MAP_HEIGHT_SCALE &&
			vertices[0].u1 == flat_u[0] && vertices[0].v1 == flat_v[0] &&
			vertices[0].u2 == flat_alpha_u[0] && vertices[0].v2 == flat_alpha_v[0] &&
			(vertices[0].diffuse >> 24) == flat_alpha[0] && !flat_flip,
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
		const auto second_draw = second_selection == std::string::npos ? std::string::npos :
			retry.find("draw pipeline=", second_selection + 1);
		const std::string range = "count=42 point_size=0.000000 index_bits=16 first_index=0 base_vertex=0";
		const std::string last_row_range =
			"count=42 point_size=0.000000 index_bits=16 first_index=1152 base_vertex=0";
		const auto first_range = retry.find(range);
		require(first_selection != std::string::npos && first_selection < first_draw &&
			second_selection != std::string::npos && first_draw < second_selection && second_selection < second_draw &&
			count_draws(retry) == 14 && first_range != std::string::npos &&
			retry.find(range, first_range + 1) != std::string::npos &&
			retry.find(last_row_range) != std::string::npos &&
			edge.texture_handle(map->getTerrainTexture()) == edge.texture_handle(map->getAlphaTerrainTexture()) &&
			device.last_draw_index_bytes().size() == 42U * sizeof(UnsignedShort),
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
		DX8Wrapper::Set_Vertex_Buffer(NULL);
		DX8Wrapper::Set_Index_Buffer(NULL, 0);
		terrain.freeMapResources();
		require_no_draw("original released terrain submitted a draw", [&] { terrain.Render(render_info); });
		edge.release_source_buffers();
		map->Release_Ref();
		map = NULL;

		const auto render_shape = [&](const char *environment, unsigned expected_draws,
			const char *message) {
			WorldHeightMap *shape = open_map(environment);
			require(terrain.initHeightData(shape->getDrawWidth(), shape->getDrawHeight(),
				shape, NULL, TRUE) == 0, "original multi-tile render shape initialization failed");
			const std::string before = device.snapshot();
			draw();
			const std::string trace = device.snapshot().substr(before.size());
			require(count_draws(trace) == expected_draws, message);
			terrain.freeMapResources();
			edge.release_source_buffers();
			shape->Release_Ref();
		};
		render_shape("ZH_M22_MULTI_TILE_X_MAP", 16,
			"original X-edge terrain draw traversal changed");
		render_shape("ZH_M22_MULTI_TILE_Y_MAP", 68,
			"original Y-edge terrain draw traversal changed");
		render_shape("ZH_M22_MULTI_TILE_EXACT_MAP", 8,
			"original exact multi-tile terrain draw traversal changed");

		multi = open_map("ZH_M22_MULTI_TILE_TERRAIN_MAP");
		require(terrain.initHeightData(35, 34, multi, NULL, TRUE) == 0,
			"original partial multi-tile draw initialization failed");
		DX8VertexBufferClass *detached = terrain.detach_vertices(1);
		const std::string before_detached = device.snapshot();
		bool detached_rejected = false;
		try { draw(); } catch (const std::runtime_error &) { detached_rejected = true; }
		require(detached_rejected && count_draws(device.snapshot().substr(before_detached.size())) == 0,
			"original incomplete multi-tile owner mutated the frame");
		terrain.restore_vertices(1, detached);
		terrain.set_last_tile_size(0, 1);
		const std::string before_bad_edge = device.snapshot();
		bool bad_edge_rejected = false;
		try { draw(); } catch (const std::runtime_error &) { bad_edge_rejected = true; }
		require(bad_edge_rejected && count_draws(device.snapshot().substr(before_bad_edge.size())) == 0,
			"original invalid edge metadata mutated the frame");
		terrain.set_last_tile_size(2, 1);
		edge.release_vertex(terrain.vertices(0));
		device.fail_next_buffer_create();
		const std::string before_bind_failure = device.snapshot();
		bool bind_rejected = false;
		try { draw(); } catch (const std::runtime_error &) { bind_rejected = true; }
		require(bind_rejected &&
			count_draws(device.snapshot().substr(before_bind_failure.size())) == 0 &&
			device.resource_counts().samplers == 0,
			"original multi-tile bind failure retained shader or frame state");
		const std::string before_bind_recovery = device.snapshot();
		draw();
		require(count_draws(device.snapshot().substr(before_bind_recovery.size())) == 70,
			"original multi-tile bind failure did not recover");
		for (unsigned failure : {0U, 34U, 35U, 69U}) {
			require_aborted_draw(failure,
				"original multi-tile draw failure retained shader or frame state");
			const std::string before_recovery = device.snapshot();
			draw();
			const unsigned recovered = count_draws(device.snapshot().substr(before_recovery.size()));
			require(recovered == 70,
				"original multi-tile draw failure did not recover");
		}
		const std::string before_multi_draw = device.snapshot();
		draw();
		const std::string multi_draw = device.snapshot().substr(before_multi_draw.size());
		const std::string full_tile_range =
			"count=6144 point_size=0.000000 index_bits=16 first_index=0 base_vertex=0";
		const std::string full_width_edge_range =
			"count=192 point_size=0.000000 index_bits=16 first_index=0 base_vertex=0";
		const std::string partial_last_row_range =
			"count=12 point_size=0.000000 index_bits=16 first_index=5952 base_vertex=0";
		require(count_draws(multi_draw) == 70 &&
			multi_draw.find(full_tile_range) != std::string::npos &&
			multi_draw.find(full_width_edge_range) != std::string::npos &&
			multi_draw.find(partial_last_row_range) != std::string::npos &&
			device.last_draw_index_bytes().size() == 12U * sizeof(UnsignedShort),
			"original partial multi-tile ranges consumed padding");
		terrain.freeMapResources();
		edge.release_source_buffers();
		multi->Release_Ref();
		multi = NULL;

		WorldHeightMap *authored = open_map("ZH_M22_AUTHORED_TERRAIN_MAP");
		require(authored->getTerrainTexture() && authored->getAlphaTerrainTexture(),
			"original authored terrain atlas pair unavailable");
		require(terrain.initHeightData(8, 8, authored, NULL, TRUE) == 0,
			"original authored terrain geometry initialization failed");
		require(terrain.extra_count() == 1 && terrain.extra_capacity() == 1 &&
			terrain.extra_position(0) == 2,
			"original authored terrain extra blend inventory changed");
		float base_u[4]{}, base_v[4]{}, alpha_u[4]{}, alpha_v[4]{};
		UnsignedByte alpha_value[4]{};
		Bool flip = FALSE;
		authored->getUVData(0, 0, base_u, base_v, FALSE);
		authored->getAlphaUVData(0, 0, alpha_u, alpha_v, alpha_value, &flip, FALSE);
		const VERTEX_FORMAT *authored_vertices = terrain.backup();
		for (Int corner = 0; corner != 4; ++corner) {
			require(authored_vertices[corner].u1 == base_u[corner] &&
				authored_vertices[corner].v1 == base_v[corner] &&
				authored_vertices[corner].u2 == alpha_u[corner] &&
				authored_vertices[corner].v2 == alpha_v[corner] &&
				(authored_vertices[corner].diffuse >> 24) == alpha_value[corner] &&
				(authored_vertices[corner].diffuse & 0x00ffffffu) == 0x00ffffffu,
				"original authored terrain vertex payload changed");
		}
		const std::string before_authored_draw = device.snapshot();
		draw();
		require(count_draws(device.snapshot().substr(before_authored_draw.size())) == 15 &&
			terrain.visible_extra_count() == 1 &&
			device.last_draw_index_bytes().size() == 6U * sizeof(UnsignedShort),
			"original authored terrain did not submit its extra blend after the base passes");
		terrain.freeMapResources();
		require(terrain.extra_count() == 0 && terrain.extra_capacity() == 0,
			"original authored terrain free retained extra blend inventory");
		edge.release_source_buffers();
		authored->Release_Ref();
		authored = NULL;

		inventory = open_map("ZH_M22_EXTRA_BLEND_TERRAIN_MAP");
		require(inventory->getTerrainTexture() && inventory->getAlphaTerrainTexture(),
			"original extra blend retry atlas pair unavailable");
		require(terrain.initHeightData(8, 8, inventory, NULL, TRUE) == 0,
			"original extra blend submission initialization failed");
		const Bool saved_adjust_cliffs = TheWritableGlobalData->m_adjustCliffTextures;
		TheWritableGlobalData->m_adjustCliffTextures = TRUE;
		require_aborted_draw(14, "original extra blend draw abort retained state");
		require(terrain.visible_extra_count() == 0,
			"original failed extra blend draw published visibility");
		const std::string before_extra_retry = device.snapshot();
		draw();
		const std::string extra_retry = device.snapshot().substr(before_extra_retry.size());
		std::size_t last_base_draw = std::string::npos;
		std::size_t next_base_draw = 0;
		for (Int draw_index = 0; draw_index < 14; ++draw_index) {
			last_base_draw = extra_retry.find("draw pipeline=", next_base_draw);
			require(last_base_draw != std::string::npos,
				"original extra blend omitted a base terrain range");
			next_base_draw = last_base_draw + 1;
		}
		const auto extra_selection = extra_retry.find(
			"original TextureClass::Apply stage=0 selected", last_base_draw + 1);
		const auto extra_draw = extra_selection == std::string::npos ? std::string::npos :
			extra_retry.find("draw pipeline=", extra_selection + 1);
		const auto extra_indices = device.last_draw_index_bytes();
		const auto index_at = [&](std::size_t index) {
			return UnsignedShort(extra_indices[index * 2]) |
				(UnsignedShort(extra_indices[index * 2 + 1]) << 8);
		};
		require(count_draws(extra_retry) == 15 && terrain.visible_extra_count() == 8 &&
			extra_selection != std::string::npos && extra_selection < extra_draw &&
			extra_indices.size() == 48U * sizeof(UnsignedShort) &&
			index_at(0) == 0 && index_at(1) == 2 && index_at(2) == 3 &&
			index_at(6) == 5 && index_at(7) == 7 && index_at(8) == 4 &&
			index_at(42) == 29 && index_at(43) == 31 && index_at(44) == 28,
			"original extra blend source topology or cliff override changed");
		const Int saved_three_way = TheWritableGlobalData->m_use3WayTerrainBlends;
		TheWritableGlobalData->m_use3WayTerrainBlends = 0;
		const std::string before_disabled_extra = device.snapshot();
		draw();
		require(count_draws(device.snapshot().substr(before_disabled_extra.size())) == 14 &&
			terrain.visible_extra_count() == 0,
			"original disabled extra blend submitted a third pass");
		TheWritableGlobalData->m_use3WayTerrainBlends = 2;
		const std::string before_debug_extra = device.snapshot();
		bool debug_extra_rejected = false;
		try { terrain.renderExtraBlendTiles(); }
		catch (const std::runtime_error &) { debug_extra_rejected = true; }
		require(debug_extra_rejected &&
			device.snapshot().substr(before_debug_extra.size()).find("draw pipeline=") == std::string::npos,
			"original debug extra blend mode mutated submission state");
		TheWritableGlobalData->m_use3WayTerrainBlends = saved_three_way;
		TheWritableGlobalData->m_adjustCliffTextures = saved_adjust_cliffs;
		terrain.freeMapResources();
		edge.release_source_buffers();
		inventory->Release_Ref();
		inventory = NULL;
		device.destroy(depth);
		device.destroy(color);
		display.reset();
	}
	require(device.resource_counts().total() == 0,
		"original flat terrain teardown retained resource");
	TheWritableGlobalData->m_partitionCellSize = saved_partition;
	std::puts("original flat terrain geometry: cells=7x7 vb=4096 ib=6144 draws=14 multitile=70");
}
