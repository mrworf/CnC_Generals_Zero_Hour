#include "PreRTS.h"

#include "Common/MapReaderWriterInfo.h"
#include "W3DDevice/GameClient/WorldHeightMap.h"
#include "zh/original_process.h"

#include <algorithm>
#include <cstdint>
#include <cstdlib>
#include <cstring>
#include <fstream>
#include <iterator>
#include <limits>
#include <stdexcept>
#include <vector>

namespace {
void require(bool value, const char *message)
{
	if (!value) throw std::runtime_error(message);
}

class MemoryChunkInput final : public ChunkInputStream
{
public:
	explicit MemoryChunkInput(const std::vector<std::uint8_t>& bytes, bool strict = false) :
		m_bytes(bytes), m_strict(strict) {}
	Int read(void *data, Int count) override
	{
		if (count < 0 || m_position > m_bytes.size()) return 0;
		const std::size_t available = m_bytes.size() - m_position;
		if (m_strict && available < static_cast<std::size_t>(count))
			throw std::runtime_error("truncated original map stream");
		const std::size_t size = std::min<std::size_t>(available,
			static_cast<std::size_t>(count));
		std::memcpy(data, m_bytes.data() + m_position, size);
		m_position += size;
		return static_cast<Int>(size);
	}
	UnsignedInt tell() override { return static_cast<UnsignedInt>(m_position); }
	Bool absoluteSeek(UnsignedInt position) override
	{
		if (position > m_bytes.size()) return FALSE;
		m_position = position;
		return TRUE;
	}
	Bool eof() override { return m_position == m_bytes.size(); }
private:
	const std::vector<std::uint8_t>& m_bytes;
	bool m_strict;
	std::size_t m_position = 0;
};

bool rejects(const std::vector<std::uint8_t>& bytes, Bool logical = TRUE, bool strict = false)
{
	try {
		MemoryChunkInput input(bytes, strict);
		WorldHeightMap *map = NEW_REF(WorldHeightMap, (&input, logical));
		map->Release_Ref();
		return false;
	} catch (const std::runtime_error&) {
		return true;
	} catch (...) {
		return true;
	}
}
}

extern "C" void zh_probe_world_height_map()
{
	const char *path = std::getenv("ZH_M22_HEIGHT_MAP_INPUT");
	require(path, "original logical height map path missing");
	std::ifstream file(path, std::ios::binary);
	require(file.good(), "original logical height map unreadable");
	std::vector<std::uint8_t> bytes(std::istreambuf_iterator<char>{file},
		std::istreambuf_iterator<char>{});
	require(bytes.size() > 64, "original logical height map fixture empty");
	auto malformed = bytes;
	malformed[0] ^= 0xff;
	auto truncated = bytes;
	auto optional_absent = bytes;
	auto invalid_bounds = bytes;
	const std::uint8_t header[] = {8, 0, 0, 0, 8, 0, 0, 0, 0, 0, 0, 0,
		1, 0, 0, 0, 8, 0, 0, 0, 8, 0, 0, 0, 64, 0, 0, 0};
	auto found = std::search(invalid_bounds.begin(), invalid_bounds.end(),
		std::begin(header), std::end(header));
	require(found != invalid_bounds.end(), "original logical height map fixture header missing");
	const std::size_t height_offset = static_cast<std::size_t>(found - invalid_bounds.begin());
	truncated.resize(height_offset + sizeof(header) + 16);
	optional_absent.resize(height_offset + sizeof(header) + 64);
	const std::uint32_t oversized = static_cast<std::uint32_t>(std::numeric_limits<Int>::max());
	std::memcpy(&*found, &oversized, sizeof(oversized));
	const bool malformed_rejected = rejects(malformed);
	const bool truncated_rejected = rejects(truncated, TRUE, true);
	const bool bounds_rejected = rejects(invalid_bounds);
	const bool visual_rejected = rejects(bytes, FALSE);
	require(malformed_rejected && truncated_rejected && bounds_rejected && visual_rejected &&
		!rejects(optional_absent),
		"original logical height map negative control accepted input");
	std::size_t parser_baseline = 0;
	for (Int generation = 0; generation != 2; ++generation) {
		MemoryChunkInput input(bytes);
		WorldHeightMap *map = NEW_REF(WorldHeightMap, (&input, TRUE));
		require(map->getXExtent() == 8 && map->getYExtent() == 8 &&
			map->getBorderSizeInline() == 0 && map->getDataPtr() &&
			map->getHeight(0, 0) == 0 && map->getHeight(7, 7) == 63 &&
			map->getAllBoundaries().size() == 1,
			"original logical height map metadata changed");
		map->setDrawWidth(1000);
		map->setDrawHeight(1000);
		require(map->getDrawWidth() == 8 && map->getDrawHeight() == 8,
			"original logical height map draw bounds changed");
		bool seismic = false;
		try { (void)map->getSeismicZVelocity(0, 0); }
		catch (const std::runtime_error&) { seismic = true; }
		require(seismic && map->getHeight(7, 7) == 63,
			"original logical height map unsupported state changed data");
		map->Release_Ref();
		const std::size_t current = zh::original_process::live_pool_allocations();
		if (generation == 0)
			parser_baseline = current;
		else
			require(current == parser_baseline,
				"original logical height map re-entry retained owner allocation");
	}
	std::puts("original WorldHeightMap logical owner: dimensions=8x8 generations=2 resources=0");
}
