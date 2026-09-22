#include "PreRTS.h"

#include "Common/GlobalData.h"
#include "Common/MapReaderWriterInfo.h"
#include "W3DDevice/GameClient/W3DShroud.h"
#include "W3DDevice/GameClient/WorldHeightMap.h"
#include "WW3D2/camera.h"
#include "zh/original_process.h"

#include <algorithm>
#include <cstdlib>
#include <stdexcept>

namespace {
void require(bool value, const char *message)
{
	if (!value) throw std::runtime_error(message);
}
}

extern "C" void zh_probe_shroud_data()
{
	const char *path = std::getenv("ZH_M22_SHROUD_DATA_MAP");
	require(path, "original shroud map path missing");
	CachedFileInputStream input;
	require(input.open(AsciiString(path)), "original shroud map unreadable");
	WorldHeightMap *map = NEW_REF(WorldHeightMap, (&input, TRUE));
	input.close();
	require(map->getXExtent() == 8 && map->getYExtent() == 8 &&
		map->getBorderSizeInline() == 0, "original shroud map dimensions changed");
	const std::size_t baseline = zh::original_process::live_pool_allocations();
	{
		W3DShroud shroud;
		require(shroud.ReAcquireResources(),
			"original empty shroud resource reset changed");
		bool preinit_read = false;
		try { (void)shroud.getShroudLevel(0, 0); }
		catch (const std::runtime_error&) { preinit_read = true; }
		bool preinit_write = false;
		try { shroud.setShroudLevel(0, 0, 255); }
		catch (const std::runtime_error&) { preinit_write = true; }
		bool null_map = false;
		try { shroud.init(NULL, MAP_XY_FACTOR, MAP_XY_FACTOR); }
		catch (const std::runtime_error&) { null_map = true; }
		bool invalid_cells = false;
		try { shroud.init(map, 0, MAP_XY_FACTOR); }
		catch (const std::runtime_error&) { invalid_cells = true; }
		bool oversized = false;
		try { shroud.init(map, 0.001f, 0.001f); }
		catch (const std::runtime_error&) { oversized = true; }
		require(preinit_read && preinit_write && null_map && invalid_cells && oversized &&
			shroud.getNumShroudCellsX() == 0 && shroud.getNumShroudCellsY() == 0,
			"original shroud invalid initialization changed state");

		shroud.init(map, MAP_XY_FACTOR, MAP_XY_FACTOR);
		const W3DShroudLevel minimum = static_cast<W3DShroudLevel>(TheGlobalData->m_shroudAlpha);
		require(shroud.getNumShroudCellsX() == 7 && shroud.getNumShroudCellsY() == 7 &&
			shroud.getCellWidth() == MAP_XY_FACTOR && shroud.getCellHeight() == MAP_XY_FACTOR &&
			shroud.getShroudLevel(0, 0) == minimum,
			"original shroud map-derived grid changed");
		shroud.setBorderShroudLevel(17);
		require(shroud.getShroudLevel(-1, 0) == 17 && shroud.getShroudLevel(7, 7) == 17,
			"original shroud border level changed");
		shroud.setShroudLevel(2, 3, 220);
		const W3DShroudLevel cell = std::max<W3DShroudLevel>(minimum, 220);
		require(shroud.getShroudLevel(2, 3) == cell,
			"original shroud cell update changed");
		bool out_of_range = false;
		try { shroud.setShroudLevel(7, 0, 255); }
		catch (const std::runtime_error&) { out_of_range = true; }
		bool duplicate = false;
		try { shroud.init(map, MAP_XY_FACTOR, MAP_XY_FACTOR); }
		catch (const std::runtime_error&) { duplicate = true; }
		require(out_of_range && duplicate && shroud.getShroudLevel(2, 3) == cell,
			"original shroud rejected operation changed valid grid");
		shroud.fillShroudData(230);
		const W3DShroudLevel filled = std::max<W3DShroudLevel>(minimum, 230);
		require(shroud.getShroudLevel(0, 0) == filled &&
			shroud.getShroudLevel(6, 6) == filled,
			"original shroud fill changed");
		shroud.setShroudFilter(FALSE);
		shroud.setShroudFilter(TRUE);
		bool render_rejected = false;
		try { shroud.render(NULL); }
		catch (const std::runtime_error&) { render_rejected = true; }
		bool reacquire_rejected = false;
		try { (void)shroud.ReAcquireResources(); }
		catch (const std::runtime_error&) { reacquire_rejected = true; }
		W3DShroudMaterialPassClass material;
		bool material_rejected = false;
		try { material.Install_Materials(); }
		catch (const std::runtime_error&) { material_rejected = true; }
		require(render_rejected && reacquire_rejected && material_rejected,
			"original shroud active projection did not fail closed");

		shroud.reset();
		require(shroud.getNumShroudCellsX() == 0 && shroud.getNumShroudCellsY() == 0 &&
			zh::original_process::live_pool_allocations() == baseline,
			"original shroud reset retained CPU grid");
		shroud.init(map, MAP_XY_FACTOR * 2, MAP_XY_FACTOR * 2);
		require(shroud.getNumShroudCellsX() == 4 && shroud.getNumShroudCellsY() == 4,
			"original shroud re-entry dimensions changed");
		shroud.reset();
	}
	require(zh::original_process::live_pool_allocations() == baseline,
		"original shroud teardown retained allocation");
	map->Release_Ref();
	std::puts("original W3DShroud map data: grid=7x7 reentry=4x4 resources=0");
}
