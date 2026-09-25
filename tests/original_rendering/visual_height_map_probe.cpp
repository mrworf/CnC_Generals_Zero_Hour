#include "PreRTS.h"

#include "Common/MapReaderWriterInfo.h"
#include "W3DDevice/GameClient/WorldHeightMap.h"
#include "zh/original_process.h"

#include <cstdlib>
#include <stdexcept>

namespace {
class MetadataMap final : public WorldHeightMap
{
public:
	explicit MetadataMap(ChunkInputStream *input) : WorldHeightMap(input, FALSE) {}
	bool hasAuthoredMetadata() const
	{
		return m_numBitmapTiles == 5 && m_numBlendedTiles == 2 && m_numCliffInfo == 2 &&
			m_numTextureClasses == 2 && m_numEdgeTiles == 1 && m_numEdgeTextureClasses == 1 &&
			m_textureClasses[0].firstTile == 0 && m_textureClasses[0].numTiles == 4 &&
			m_textureClasses[0].width == 2 && m_textureClasses[1].firstTile == 4 &&
			m_edgeTextureClasses[0].firstTile == 0 && m_blendedTiles[1].blendNdx == 4 &&
			m_blendedTiles[1].horiz == 1 && m_blendedTiles[1].customBlendEdgeClass == 0 &&
			m_cliffInfo[1].tileIndex == 16 && m_cliffInfo[1].flip == TRUE;
	}
};

void require(bool value, const char *message)
{
	if (!value) throw std::runtime_error(message);
}

WorldHeightMap *open_map(const char *name, Bool logical = FALSE)
{
	const char *path = std::getenv(name);
	if (!path) throw std::runtime_error("original visual map input missing");
	CachedFileInputStream input;
	if (!input.open(AsciiString(path))) throw std::runtime_error("original visual map unreadable");
	WorldHeightMap *map = NEW_REF(WorldHeightMap, (&input, logical));
	input.close();
	return map;
}

bool rejects(const char *name)
{
	try {
		WorldHeightMap *map = open_map(name);
		map->Release_Ref();
		return false;
	} catch (...) {
		return true;
	}
}

MetadataMap *open_metadata(const char *name)
{
	const char *path = std::getenv(name);
	if (!path) throw std::runtime_error("original authored metadata input missing");
	CachedFileInputStream input;
	if (!input.open(AsciiString(path))) throw std::runtime_error("original authored metadata unreadable");
	MetadataMap *map = NEW_REF(MetadataMap, (&input));
	input.close();
	return map;
}
}

extern "C" void zh_probe_visual_height_map()
{
	for (const char *failure : {"ZH_M22_VISUAL_MAP_MISSING", "ZH_M22_VISUAL_MAP_DUPLICATE",
		"ZH_M22_VISUAL_MAP_TRUNCATED", "ZH_M22_VISUAL_MAP_SHAPE",
		"ZH_M22_VISUAL_MAP_ACTIVE_BLEND",
		"ZH_M22_VISUAL_MAP_OVERSIZE"}) {
		const std::size_t failure_baseline = zh::original_process::live_pool_allocations();
		require(rejects(failure), "original visual terrain negative input accepted");
		require(zh::original_process::live_pool_allocations() == failure_baseline,
			"original visual terrain rejected input retained allocation");
	}
	WorldHeightMap *cliff_bits = open_map("ZH_M22_VISUAL_MAP_ACTIVE_CLIFF");
	require(cliff_bits->getCliffState(0, 0), "original authored terrain cliff-state bit lost");
	cliff_bits->Release_Ref();
	for (const char *failure : {"ZH_M22_VISUAL_MAP_AUTHORED_BAD_SPAN",
		"ZH_M22_VISUAL_MAP_AUTHORED_OVERLAP", "ZH_M22_VISUAL_MAP_AUTHORED_BAD_REAL",
		"ZH_M22_VISUAL_MAP_AUTHORED_BAD_TILE", "ZH_M22_VISUAL_MAP_AUTHORED_BAD_BLEND",
		"ZH_M22_VISUAL_MAP_AUTHORED_BAD_CLIFF", "ZH_M22_VISUAL_MAP_AUTHORED_BAD_FLAG",
		"ZH_M22_VISUAL_MAP_AUTHORED_TAIL"}) {
		const std::size_t failure_baseline = zh::original_process::live_pool_allocations();
		require(rejects(failure), "original authored terrain metadata negative accepted");
		require(zh::original_process::live_pool_allocations() == failure_baseline,
			"original authored terrain metadata rejection retained allocation");
	}

	const std::size_t authored_baseline = zh::original_process::live_pool_allocations();
	for (Int generation = 0; generation != 2; ++generation) {
		MetadataMap *authored = open_metadata("ZH_M22_VISUAL_MAP_AUTHORED");
		require(authored->hasAuthoredMetadata() && authored->getTextureClass(0, 0, TRUE) == 0 &&
			authored->getTextureClassNoBlend(1, 0) == 1 && authored->getTextureClass(0, 0) == -1 &&
			authored->isCliffMappedTexture(4, 0) && authored->getCliffState(0, 0),
			"original authored terrain metadata changed");
		authored->Release_Ref();
		require(zh::original_process::live_pool_allocations() == authored_baseline,
			"original authored terrain metadata teardown retained allocation");
	}

	WorldHeightMap *logical = open_map("ZH_M22_VISUAL_MAP_INPUT", TRUE);
	require(logical->getXExtent() == 8 && logical->getHeight(7, 7) == 63,
		"original visual terrain changed logical-only contrast");
	logical->Release_Ref();

	const std::size_t baseline = zh::original_process::live_pool_allocations();
	for (Int generation = 0; generation != 2; ++generation) {
		WorldHeightMap *map = open_map("ZH_M22_VISUAL_MAP_INPUT");
		float u[4]{}, v[4]{};
		UnsignedByte alpha[4]{255, 255, 255, 255};
		Bool flip = TRUE;
		require(map->getXExtent() == 8 && map->getYExtent() == 8 &&
			map->getHeight(7, 7) == 63 && map->getTextureClass(0, 0) == 0 &&
			map->getTextureClassNoBlend(7, 7) == 0 &&
			!map->getFlipState(0, 0) && !map->getCliffState(7, 7) &&
			!map->isCliffMappedTexture(0, 0),
			"original visual terrain flat metadata changed");
		require(!map->getUVData(0, 0, u, v, FALSE),
			"original visual terrain claimed unloaded texture UV");
		map->getAlphaUVData(0, 0, u, v, alpha, &flip, FALSE);
		require(!flip && alpha[0] == 0 && alpha[1] == 0 && alpha[2] == 0 && alpha[3] == 0,
			"original visual terrain no-blend alpha changed");
		bool range = false;
		try { map->getAlphaUVData(7, 7, u, v, alpha, &flip, FALSE); }
		catch (...) { range = true; }
		bool texture = false;
		try { (void)map->getTerrainTexture(); }
		catch (...) { texture = true; }
		require(range && texture && map->getTextureClass(-1, 0) == -1,
			"original visual terrain unsupported query accepted");
		map->Release_Ref();
		require(zh::original_process::live_pool_allocations() == baseline,
			"original visual terrain teardown retained allocation");
	}
	std::puts("original visual WorldHeightMap metadata: map=8x8 flat=1 generations=2 resources=0");
}
