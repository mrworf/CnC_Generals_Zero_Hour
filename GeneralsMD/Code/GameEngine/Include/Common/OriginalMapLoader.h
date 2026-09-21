#pragma once

#include "Lib/BaseType.h"
#include "Common/STLTypedefs.h"

#include <vector>

class ChunkInputStream;
class DataChunkInput;
struct DataChunkInfo;

// Device-independent extraction of the logical .map readers historically
// co-located with W3D's WorldHeightMap.  This is the canonical CPU path used
// by non-W3D devices; physical terrain resources remain device adapters.
class OriginalMapLoader
{
public:
	Bool load(ChunkInputStream *stream);
	void clear();

	Int width() const { return m_width; }
	Int height() const { return m_height; }
	Int borderSize() const { return m_borderSize; }
	const std::vector<ICoord2D>& boundaries() const { return m_boundaries; }
	const std::vector<UnsignedByte>& heights() const { return m_heights; }

private:
	static Bool parseHeightMap(DataChunkInput&, DataChunkInfo*, void*);
	static Bool parseWorldInfo(DataChunkInput&, DataChunkInfo*, void*);
	static Bool parseObjects(DataChunkInput&, DataChunkInfo*, void*);
	static Bool parseObject(DataChunkInput&, DataChunkInfo*, void*);

	Int m_width = 0;
	Int m_height = 0;
	Int m_borderSize = 0;
	std::vector<ICoord2D> m_boundaries;
	std::vector<UnsignedByte> m_heights;
};
