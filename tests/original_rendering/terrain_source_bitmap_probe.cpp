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
	TileData *source_tile() { return getSourceTile(0); }
};
}

extern "C" void zh_probe_terrain_source_bitmap()
{
	const char *path = std::getenv("ZH_M22_TERRAIN_BITMAP_MAP");
	require(path, "original terrain bitmap map path missing");
	const std::size_t baseline = zh::original_process::live_pool_allocations();
	for (Int generation = 0; generation != 2; ++generation) {
		CachedFileInputStream input;
		require(input.open(AsciiString(path)), "original terrain bitmap map unreadable");
		TestVisualMap *map = NEW_REF(TestVisualMap, (&input));
		input.close();
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
}
