#include "PreRTS.h"

#include "Common/GlobalData.h"
#include "Common/MapReaderWriterInfo.h"
#include "W3DDevice/GameClient/HeightMap.h"
#include "W3DDevice/GameClient/W3DDisplay.h"
#include "original_gpu_edge.h"
#include "zh/renderer/recording_device.h"

#include <cstdlib>
#include <memory>
#include <stdexcept>

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
		terrain.freeMapResources();
		edge.release_source_buffers();
		display.reset();
	}
	require(device.resource_counts().total() == 0,
		"original flat terrain teardown retained resource");
	TheWritableGlobalData->m_partitionCellSize = saved_partition;
	map->Release_Ref();
	std::puts("original flat terrain geometry: cells=7x7 vb=4096 ib=6144 draws=0");
}
