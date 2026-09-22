#include "PreRTS.h"

#include "Common/GlobalData.h"
#include "Common/MapReaderWriterInfo.h"
#include "W3DDevice/GameClient/BaseHeightMap.h"
#include "W3DDevice/GameClient/W3DDisplay.h"
#include "W3DDevice/GameClient/W3DShroud.h"
#include "W3DDevice/GameClient/WorldHeightMap.h"
#include "original_gpu_edge.h"
#include "zh/original_process.h"
#include "zh/renderer/recording_device.h"

#include <cstdlib>
#include <memory>
#include <stdexcept>

namespace {
void require(bool value, const char *message)
{
	if (!value) throw std::runtime_error(message);
}

class TestBaseTerrain final : public BaseHeightMapRenderObjClass
{
public:
	void Render(RenderInfoClass&) override
	{
		throw std::runtime_error("original derived terrain mesh pending");
	}
	void doPartialUpdate(const IRegion2D&, WorldHeightMap*, RefRenderObjListIterator*) override
	{
		throw std::runtime_error("original derived terrain partial update pending");
	}
	int updateBlock(Int, Int, Int, Int, WorldHeightMap*, RefRenderObjListIterator*) override
	{
		throw std::runtime_error("original derived terrain block update pending");
	}
};
}

extern "C" void zh_probe_base_terrain_map()
{
	bool missing_owner = false;
	try { TestBaseTerrain terrain; }
	catch (const std::runtime_error&) { missing_owner = true; }
	require(missing_owner, "original base terrain constructed without display edge");

	const char *path = std::getenv("ZH_M22_BASE_TERRAIN_MAP");
	require(path, "original base terrain map path missing");
	CachedFileInputStream input;
	require(input.open(AsciiString(path)), "original base terrain map unreadable");
	WorldHeightMap *map = NEW_REF(WorldHeightMap, (&input, TRUE));
	input.close();
	require(map->Num_Refs() == 1, "original base terrain map initial ref changed");
	const Real saved_partition_cell_size = TheWritableGlobalData->m_partitionCellSize;

	zh::renderer::RecordingGpuDevice device;
	{
		zh::original_runtime::OriginalGpuEdge edge(device);
		auto display = std::make_unique<W3DDisplay>();
		display->init();
		const std::size_t terrain_baseline = zh::original_process::live_pool_allocations();
		{
			TestBaseTerrain terrain;
			require(terrain.getShroud() && !terrain.getMap(),
				"original base terrain omitted source shroud owner");
			bool missing_partition = false;
			try { terrain.initHeightData(8, 8, map, NULL, FALSE); }
			catch (const std::runtime_error&) { missing_partition = true; }
			TheWritableGlobalData->m_partitionCellSize = MAP_XY_FACTOR;
			bool null_map = false;
			try { terrain.initHeightData(8, 8, NULL, NULL, FALSE); }
			catch (const std::runtime_error&) { null_map = true; }
			bool mismatched = false;
			try { terrain.initHeightData(7, 8, map, NULL, FALSE); }
			catch (const std::runtime_error&) { mismatched = true; }
			bool visual = false;
			try { terrain.initHeightData(8, 8, map, NULL, TRUE); }
			catch (const std::runtime_error&) { visual = true; }
			require(missing_partition && null_map && mismatched && visual && !terrain.getMap() &&
				map->Num_Refs() == 1 && terrain.getShroud()->getNumShroudCellsX() == 0,
				"original base terrain invalid bind changed owner state");

			require(terrain.initHeightData(8, 8, map, NULL, FALSE) == 0 &&
				terrain.getMap() == map && map->Num_Refs() == 2 &&
				terrain.getShroud()->getNumShroudCellsX() == 7 &&
				terrain.getShroud()->getNumShroudCellsY() == 7 &&
				terrain.getMinHeight() == 0 &&
				terrain.getMaxHeight() == 63 * MAP_HEIGHT_SCALE,
				"original base terrain logical bind changed");
			bool duplicate = false;
			try { terrain.initHeightData(8, 8, map, NULL, FALSE); }
			catch (const std::runtime_error&) { duplicate = true; }
			require(duplicate && terrain.getMap() == map && map->Num_Refs() == 2,
				"original base terrain duplicate bind changed ref");
			terrain.reset();
			require(terrain.getMap() == map && map->Num_Refs() == 2 &&
				terrain.getShroud()->getNumShroudCellsX() == 0,
				"original base terrain reset changed map ownership");
			require(terrain.freeMapResources() == 0 && !terrain.getMap() &&
				map->Num_Refs() == 1,
				"original base terrain free retained map ref");
			require(terrain.initHeightData(8, 8, map, NULL, FALSE) == 0 &&
				map->Num_Refs() == 2,
				"original base terrain re-entry failed");
		}
		require(map->Num_Refs() == 1,
			"original base terrain destructor retained map ref");
		require(zh::original_process::live_pool_allocations() == terrain_baseline,
			"original base terrain teardown retained owner allocation");
		display.reset();
	}
	require(device.resource_counts().total() == 0,
		"original base terrain emitted Recording resource");
	TheWritableGlobalData->m_partitionCellSize = saved_partition_cell_size;
	map->Release_Ref();
	std::puts("original base terrain map owner: map=8x8 shroud=7x7 resources=0");
}
