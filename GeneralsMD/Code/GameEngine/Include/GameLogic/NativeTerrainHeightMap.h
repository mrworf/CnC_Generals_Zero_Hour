// SPDX-License-Identifier: GPL-3.0-or-later
#pragma once
#include "Common/DataChunk.h"
#include "Common/GameCommon.h"
#include <vector>

// Game-owned extraction of WorldHeightMap height backing and BaseHeightMap's
// simulation queries. This is not a complete world/visual map loader.
class NativeTerrainHeightMap {
public:
  // HeightChunkBacking is the full reader's intermediate backing. For version1
  // the coupled BlendTileData reader still has to halve dimensions/dataSize.
  enum class Purpose { HeightChunkBacking, LogicalMetadata };
  void loadHeightData(ChunkInputStream& stream, Purpose purpose);
  void readHeightChunk(DataChunkInput& input, DataChunkVersionType version,
                       Purpose purpose);
  Int width() const noexcept { return m_width; }
  Int height() const noexcept { return m_height; }
  Int border() const noexcept { return m_border; }
  const std::vector<ICoord2D>& boundaries() const noexcept { return m_boundaries; }
  const std::vector<UnsignedByte>& bytes() const noexcept { return m_bytes; }
  UnsignedByte rawHeight(Int x, Int y) const;
  Real groundHeight(Real x, Real y, Coord3D* normal = nullptr,
                    const NativeTerrainHeightMap* displayGrid = nullptr) const;
  Bool clearLineOfSight(const Coord3D& from, const Coord3D& to,
                       Real displayMaximumHeight) const;
private:
  Int m_width = 0, m_height = 0, m_border = 0;
  std::vector<ICoord2D> m_boundaries;
  std::vector<UnsignedByte> m_bytes;
  void swap(NativeTerrainHeightMap& candidate) noexcept;
};
