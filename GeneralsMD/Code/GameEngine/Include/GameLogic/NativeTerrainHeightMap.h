// SPDX-License-Identifier: GPL-3.0-or-later
#pragma once
#include "Common/DataChunk.h"
#include "Common/GameCommon.h"
#include <array>
#include <string>
#include <vector>

// Game-owned extraction of WorldHeightMap height backing and BaseHeightMap's
// simulation queries. This is not a complete world/visual map loader.
class NativeTerrainHeightMap {
public:
  // HeightChunkBacking is the full reader's intermediate backing. For version1
  // the coupled BlendTileData reader still has to halve dimensions/dataSize.
  enum class Purpose { HeightChunkBacking, LogicalMetadata };
  struct TextureClass {
    Int firstTile = 0, numTiles = 0, width = 0;
    std::string name;
  };
  struct BlendEntry {
    Int tileIndex = 0;
    std::array<UnsignedByte,6> flags{}; // horiz,vert,right,left,inverted,long
    Int customEdgeClass = -1;
  };
  struct CliffEntry {
    Int tileIndex = 0;
    std::array<Real,8> uv{};
    std::array<UnsignedByte,2> flags{}; // flip,mutant
  };
  struct Topology {
    DataChunkVersionType version = 0;
    Int bitmapTiles = 0, edgeTiles = 0;
    std::size_t cliffStride = 0;
    std::vector<Short> tiles, blends, extraBlends, cliffs;
    std::vector<UnsignedByte> cliffFlags;
    std::vector<TextureClass> textureClasses, edgeClasses;
    std::vector<BlendEntry> blendEntries;
    std::vector<CliffEntry> cliffEntries;
  };
  void loadHeightData(ChunkInputStream& stream, Purpose purpose);
  // Complete height/blend topology only: no world-object/lighting/asset readiness.
  void loadTerrainTopology(ChunkInputStream& stream, Bool useThreeWayBlends);
  void readHeightChunk(DataChunkInput& input, DataChunkVersionType version,
                       Purpose purpose);
  void readBlendChunk(DataChunkInput& input, DataChunkVersionType version,
                      Bool useThreeWayBlends);
  Bool topologyReady() const noexcept { return m_topology.version != 0; }
  const Topology& topology() const noexcept { return m_topology; }
  Bool isCliffCell(Real x, Real y) const;
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
  Purpose m_purpose = Purpose::HeightChunkBacking;
  Topology m_topology;
  void swap(NativeTerrainHeightMap& candidate) noexcept;
};
