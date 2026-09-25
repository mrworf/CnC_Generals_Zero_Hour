#include "PreRTS.h"

#include "Common/MapReaderWriterInfo.h"
#include "W3DDevice/GameClient/TileData.h"
#include "W3DDevice/GameClient/WorldHeightMap.h"
#include "zh/original_process.h"

#include <cstdlib>
#include <stdexcept>

namespace {
void require(bool value, const char *message)
{
	if (!value) throw std::runtime_error(message);
}

class TestVisualMap final : public WorldHeightMap
{
public:
	explicit TestVisualMap(ChunkInputStream *input) : WorldHeightMap(input, FALSE) {}
	TileData *source_tile(UnsignedInt index = 0) { return getSourceTile(index); }
	TileData *edge_tile(UnsignedInt index = 0) { return getEdgeTile(index); }
};

TestVisualMap *open_map(const char *path)
{
	CachedFileInputStream input;
	require(input.open(AsciiString(path)), "original terrain bitmap map unreadable");
	TestVisualMap *map = NEW_REF(TestVisualMap, (&input));
	input.close();
	return map;
}
}

extern "C" void zh_probe_terrain_source_bitmap()
{
	const char *path = std::getenv("ZH_M22_TERRAIN_BITMAP_MAP");
	require(path, "original terrain bitmap map path missing");
	const std::size_t baseline = zh::original_process::live_pool_allocations();
	for (Int generation = 0; generation != 2; ++generation) {
		TestVisualMap *map = open_map(path);
		TileData *tile = map->source_tile();
		require(tile, "original terrain source tile omitted");
		for (Int width : {64, 32, 16, 8, 4, 2, 1}) {
			UnsignedByte *bytes = tile->getRGBDataForWidth(width);
			require(bytes && bytes[0] == 3 && bytes[1] == 2 && bytes[2] == 1 && bytes[3] == 4,
				"original terrain source tile mip changed");
		}
		map->Release_Ref();
		require(zh::original_process::live_pool_allocations() == baseline,
			"original terrain source tile teardown retained allocation");
	}
	std::puts("original terrain source bitmap: class=Flat tile=64 mips=7 resources=0");
	if (const char *authored_path = std::getenv("ZH_M22_TERRAIN_BITMAP_AUTHORED_MAP")) {
		for (Int generation = 0; generation != 2; ++generation) {
			TestVisualMap *map = open_map(authored_path);
			const UnsignedByte expected[6][4] = {
				{3, 2, 1, 4}, {7, 6, 5, 8}, {11, 10, 9, 12}, {15, 14, 13, 16},
				{23, 22, 21, 24}, {33, 32, 31, 255}};
			for (Int tile_index = 0; tile_index != 6; ++tile_index) {
				TileData *tile = tile_index == 5 ? map->edge_tile() : map->source_tile(tile_index);
				require(tile, "original authored terrain tile omitted");
				for (Int width : {64, 32, 16, 8, 4, 2, 1}) {
					UnsignedByte *bytes = tile->getRGBDataForWidth(width);
					require(bytes && bytes[0] == expected[tile_index][0] &&
						bytes[1] == expected[tile_index][1] && bytes[2] == expected[tile_index][2] &&
						bytes[3] == expected[tile_index][3],
						"original authored terrain tile order or mip changed");
				}
			}
			map->Release_Ref();
			require(zh::original_process::live_pool_allocations() == baseline,
				"original authored terrain tile teardown retained allocation");
		}
		std::puts("original terrain source tile sets: base=5 edge=1 mips=7 resources=0");
	}
}
