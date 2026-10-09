// SPDX-License-Identifier: GPL-3.0-or-later
#pragma once
#include "Common/DataChunk.h"
#include "Common/GameCommon.h"
#include <array>
#include <optional>
#include <string>
#include <string_view>
#include <vector>
class GlobalData;

// Game-owned extraction of WorldHeightMap height backing and BaseHeightMap's
// simulation queries. This is not a complete world/visual map loader.
class NativeTerrainHeightMap {
public:
  explicit NativeTerrainHeightMap(std::optional<UnsignedInt> shadow = std::nullopt)
    : m_shadowColor(shadow) {}
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
  // Retained visual-owned CPU map: topology and source global lighting commit
  // together. Does not parse objects, sides or polygon-trigger owners.
  void loadVisualTerrain(ChunkInputStream& stream, Bool useThreeWayBlends,
                         GlobalData& configuration);
  std::optional<UnsignedInt> shadowColor() const noexcept { return m_shadowColor; }
  std::string_view terrainName(Real x, Real y) const;
  // Source construction adapter uses unbordered grid coordinates and only lowers.
  Bool lowerRawHeight(const ICoord2D& gridPosition, Int height);
  Int constructionHeight(const ICoord2D& gridPosition) const;
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
  Real maximumHeight() const noexcept { return m_maximumHeight; }
  Bool clearLineOfSight(const Coord3D& from, const Coord3D& to,
                       Real displayMaximumHeight) const;
  // Display mesh query, not logical height/LOS, water or bridge picking.
  // Misses leave output untouched; output is optional. No query allocation.
  Bool intersectSegment(const Coord3D& from, const Coord3D& to,
                        Coord3D* output = nullptr) const;
  void swap(NativeTerrainHeightMap& candidate) noexcept;
private:
  Int m_width = 0, m_height = 0, m_border = 0;
  std::vector<ICoord2D> m_boundaries;
  std::vector<UnsignedByte> m_bytes;
  Purpose m_purpose = Purpose::HeightChunkBacking;
  Topology m_topology;
  std::optional<UnsignedInt> m_shadowColor;
  Real m_maximumHeight = 0;
  void cacheMaximumHeight() noexcept;
};
