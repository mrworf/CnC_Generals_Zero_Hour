// SPDX-License-Identifier: GPL-3.0-or-later
// Algorithms derived from EA WorldHeightMap.cpp and BaseHeightMap.cpp.
#include "GameLogic/NativeTerrainHeightMap.h"
#include "Common/Terrain.h"
#include <algorithm>
#include <array>
#include <bit>
#include <cmath>
#include <cstdint>
#include <limits>
#include <utility>

namespace {
struct HeightRead {
  NativeTerrainHeightMap& candidate;
  NativeTerrainHeightMap::Purpose purpose;
  bool seen = false;
};
struct TopologyRead {
  NativeTerrainHeightMap& candidate;
  Bool useThreeWayBlends;
  bool heightSeen = false, blendSeen = false;
};
Bool parseTopologyHeight(DataChunkInput& input, DataChunkInfo* info, void* raw) {
  auto& read = *static_cast<TopologyRead*>(raw);
  if (read.heightSeen || read.blendSeen) throw ERROR_CORRUPT_FILE_FORMAT;
  read.candidate.readHeightChunk(input, info->version,
                                NativeTerrainHeightMap::Purpose::HeightChunkBacking);
  read.heightSeen = true; return TRUE;
}
Bool parseTopologyBlend(DataChunkInput& input, DataChunkInfo* info, void* raw) {
  auto& read = *static_cast<TopologyRead*>(raw);
  if (!read.heightSeen || read.blendSeen) throw ERROR_CORRUPT_FILE_FORMAT;
  read.candidate.readBlendChunk(input, info->version, read.useThreeWayBlends);
  read.blendSeen = true; return TRUE;
}
void readShorts(DataChunkInput& input, std::vector<Short>& output, std::size_t count) {
  if (count > input.getChunkDataSizeLeft()/2) throw ERROR_CORRUPT_FILE_FORMAT;
  std::array<UnsignedByte,4096> block;
  while (count) {
    const auto units = std::min(count, block.size()/2);
    input.readArrayOfBytes(reinterpret_cast<char*>(block.data()), static_cast<Int>(units*2));
    for (std::size_t index = 0; index < units; ++index) {
      const auto value = static_cast<UnsignedShort>(block[index*2] | (UnsignedShort(block[index*2+1])<<8));
      output.push_back(std::bit_cast<Short>(value));
    }
    count -= units;
  }
}
Int tableCount(DataChunkInput& input, Int minimum, unsigned minimumRecordBytes) {
  const Int count = input.readInt();
  if (count < minimum || std::uint64_t(count-minimum)*minimumRecordBytes > input.getChunkDataSizeLeft())
    throw ERROR_CORRUPT_FILE_FORMAT;
  return count;
}
NativeTerrainHeightMap::TextureClass readTextureClass(DataChunkInput& input, bool legacyField) {
  NativeTerrainHeightMap::TextureClass entry;
  entry.firstTile = input.readInt(); entry.numTiles = input.readInt(); entry.width = input.readInt();
  if (legacyField) (void)input.readInt();
  entry.name = input.readAsciiString().str();
  // Source texture acquisition is intentionally separate; names/metadata are not
  // initialized image backing or success evidence for audiovisual output.
  return entry;
}
Bool parseHeight(DataChunkInput& input, DataChunkInfo* info, void* raw) {
  auto& read = *static_cast<HeightRead*>(raw);
  if (read.seen) throw ERROR_CORRUPT_FILE_FORMAT;
  read.candidate.readHeightChunk(input, info->version, read.purpose);
  read.seen = true;
  return TRUE;
}
std::int64_t gridCoordinate(Real value, Int border) {
  // Reserve traversal/subtraction headroom. No float-to-int UB for hostile queries.
  const auto floored = std::floor(value * (1.0f / MAP_XY_FACTOR));
  if (!std::isfinite(floored) || floored < std::numeric_limits<Int>::min() ||
      static_cast<double>(floored) > std::numeric_limits<Int>::max())
    throw ERROR_BAD_ARG;
  return static_cast<std::int64_t>(floored) + border;
}
void upNormal(Coord3D* normal) {
  if (normal) { normal->x = 0; normal->y = 0; normal->z = 1; }
}
}
void NativeTerrainHeightMap::swap(NativeTerrainHeightMap& candidate) noexcept {
  std::swap(m_width, candidate.m_width);
  std::swap(m_height, candidate.m_height);
  std::swap(m_border, candidate.m_border);
  m_boundaries.swap(candidate.m_boundaries);
  m_bytes.swap(candidate.m_bytes);
  std::swap(m_purpose, candidate.m_purpose);
  std::swap(m_topology, candidate.m_topology);
}
void NativeTerrainHeightMap::loadTerrainTopology(ChunkInputStream& stream, Bool useThreeWayBlends) {
  NativeTerrainHeightMap candidate;
  DataChunkInput input(&stream);
  TopologyRead read{candidate, useThreeWayBlends};
  input.registerParser(AsciiString("HeightMapData"), AsciiString::TheEmptyString, parseTopologyHeight);
  input.registerParser(AsciiString("BlendTileData"), AsciiString::TheEmptyString, parseTopologyBlend);
  if (!input.parse(&read) || !read.heightSeen || !read.blendSeen) throw ERROR_CORRUPT_FILE_FORMAT;
  swap(candidate);
}
void NativeTerrainHeightMap::loadHeightData(ChunkInputStream& stream, Purpose purpose) {
  NativeTerrainHeightMap candidate;
  DataChunkInput input(&stream);
  HeightRead read{candidate, purpose};
  input.registerParser(AsciiString("HeightMapData"), AsciiString::TheEmptyString,
                       parseHeight);
  if (!input.parse(&read) || !read.seen) throw ERROR_CORRUPT_FILE_FORMAT;
  swap(candidate);
}
void NativeTerrainHeightMap::readBlendChunk(DataChunkInput& input, DataChunkVersionType version,
                                          Bool useThreeWayBlends) {
  if (m_bytes.empty() || m_purpose != Purpose::HeightChunkBacking || topologyReady())
    throw ERROR_BAD_ARG;
  if (version < K_BLEND_TILE_VERSION_1 || version > K_BLEND_TILE_VERSION_8)
    throw ERROR_CORRUPT_FILE_FORMAT;
  const Int length = input.readInt();
  if (length <= 0 || std::size_t(length) != m_bytes.size()) throw ERROR_CORRUPT_FILE_FORMAT;
  Topology candidate; candidate.version = version;
  readShorts(input, candidate.tiles, length);
  readShorts(input, candidate.blends, length);
  if (version >= K_BLEND_TILE_VERSION_6) readShorts(input, candidate.extraBlends, length);
  else candidate.extraBlends.assign(length, 0);
  if (!useThreeWayBlends) std::fill(candidate.extraBlends.begin(), candidate.extraBlends.end(),0);
  if (version >= K_BLEND_TILE_VERSION_5) readShorts(input, candidate.cliffs, length);
  else candidate.cliffs.assign(length, 0);
  candidate.cliffStride = (std::size_t(m_width)+7)/8;
  candidate.cliffFlags.assign(candidate.cliffStride*std::size_t(m_height),0);
  if (version >= K_BLEND_TILE_VERSION_7) {
    const auto sourceStride = version == K_BLEND_TILE_VERSION_7 ? (std::size_t(m_width)+1)/8 : candidate.cliffStride;
    std::array<UnsignedByte,4096> block;
    for (Int y = 0; y < m_height; ++y) {
      for (std::size_t x = 0; x < sourceStride;) {
        const auto bytes = std::min(sourceStride-x,block.size());
        input.readArrayOfBytes(reinterpret_cast<char*>(block.data()),static_cast<Int>(bytes));
        std::copy_n(block.begin(),bytes,candidate.cliffFlags.begin()+std::size_t(y)*candidate.cliffStride+x);
        x += bytes;
      }
    }
  } else {
    // Match source pre-resize ordering for legacy height/blend version1.
    for (Int y = 0; y < m_height-1; ++y) for (Int x = 0; x < m_width-1; ++x) {
      const auto values = {rawHeight(x,y),rawHeight(x+1,y),rawHeight(x,y+1),rawHeight(x+1,y+1)};
      const auto [low,high] = std::minmax_element(values.begin(),values.end());
      if ((*high-*low)*MAP_HEIGHT_SCALE > 9.8f)
        candidate.cliffFlags[std::size_t(y)*candidate.cliffStride+std::size_t(x)/8] |= UnsignedByte(1u<<(x&7));
    }
  }
  candidate.bitmapTiles = input.readInt();
  if (candidate.bitmapTiles <= 0) throw ERROR_CORRUPT_FILE_FORMAT;
  const auto blendCount = tableCount(input,1,13);
  const auto cliffCount = version >= K_BLEND_TILE_VERSION_5 ? tableCount(input,1,38) : 1;
  const auto classes = tableCount(input,0,18);
  for (Int index = 0; index < classes; ++index)
    candidate.textureClasses.push_back(readTextureClass(input,true));
  if (version >= K_BLEND_TILE_VERSION_4) {
    candidate.edgeTiles = input.readInt();
    if (candidate.edgeTiles < 0) throw ERROR_CORRUPT_FILE_FORMAT;
    const auto edges = tableCount(input,0,14);
    for (Int index = 0; index < edges; ++index)
      candidate.edgeClasses.push_back(readTextureClass(input,false));
  }
  candidate.blendEntries.emplace_back(); // Source index0 is the no-blend sentinel.
  for (Int index = 1; index < blendCount; ++index) {
    BlendEntry entry; entry.tileIndex = input.readInt();
    for (unsigned flag = 0; flag < 5; ++flag) entry.flags[flag] = static_cast<UnsignedByte>(input.readByte());
    if (!useThreeWayBlends) entry.flags[4] &= ~UnsignedByte(2); // Source FLIPPED_MASK.
    if (version >= K_BLEND_TILE_VERSION_3) entry.flags[5] = static_cast<UnsignedByte>(input.readByte());
    if (version >= K_BLEND_TILE_VERSION_4) entry.customEdgeClass = input.readInt();
    if (input.readInt() != 0x7ada0000) throw ERROR_CORRUPT_FILE_FORMAT;
    candidate.blendEntries.push_back(entry);
  }
  candidate.cliffEntries.emplace_back();
  for (Int index = 1; index < cliffCount; ++index) {
    CliffEntry entry; entry.tileIndex = input.readInt();
    for (auto& value : entry.uv) value = input.readReal();
    for (auto& flag : entry.flags) flag = static_cast<UnsignedByte>(input.readByte());
    candidate.cliffEntries.push_back(entry);
  }
  if (!input.atEndOfChunk()) throw ERROR_CORRUPT_FILE_FORMAT;
  Int finalWidth = m_width, finalHeight = m_height;
  if (version == K_BLEND_TILE_VERSION_1) {
    finalWidth = m_width/2+m_width%2; finalHeight = m_height/2+m_height%2;
    for (Int y = 0; y < finalHeight; ++y) for (Int x = 0; x < finalWidth; ++x) {
      const auto destination = std::size_t(y)*finalWidth+x;
      candidate.tiles[destination] = candidate.tiles[std::size_t(2)*y*m_width+2*x];
      candidate.blends[destination] = candidate.extraBlends[destination] = candidate.cliffs[destination] = 0;
    }
    candidate.blendEntries.resize(1); candidate.cliffEntries.resize(1);
  }
  const auto active = std::size_t(finalWidth)*finalHeight;
  for (std::size_t index = 0; index < active; ++index) {
    if (candidate.blends[index] < 0 || std::size_t(candidate.blends[index]) >= candidate.blendEntries.size()) candidate.blends[index] = 0;
    if (candidate.extraBlends[index] < 0 || std::size_t(candidate.extraBlends[index]) >= candidate.blendEntries.size()) candidate.extraBlends[index] = 0;
    if (candidate.cliffs[index] < 0 || std::size_t(candidate.cliffs[index]) >= candidate.cliffEntries.size()) candidate.cliffs[index] = 0;
  }
  m_topology = std::move(candidate);
  m_width = finalWidth; m_height = finalHeight;
}
Bool NativeTerrainHeightMap::isCliffCell(Real x, Real y) const {
  const Real tx = std::trunc(x/MAP_XY_FACTOR), ty = std::trunc(y/MAP_XY_FACTOR);
  const auto representable = [](Real value) { return std::isfinite(value) &&
    static_cast<double>(value) >= std::numeric_limits<Int>::min() &&
    static_cast<double>(value) <= std::numeric_limits<Int>::max(); };
  if (!representable(tx) || !representable(ty)) throw ERROR_BAD_ARG;
  if (m_bytes.empty()) return FALSE;
  if (!topologyReady()) throw ERROR_BAD_ARG;
  if (m_width < 2 || m_height < 2) return FALSE;
  const auto ix = std::clamp<std::int64_t>(static_cast<std::int64_t>(tx)+m_border,0,m_width-2);
  const auto iy = std::clamp<std::int64_t>(static_cast<std::int64_t>(ty)+m_border,0,m_height-2);
  return (m_topology.cliffFlags[std::size_t(iy)*m_topology.cliffStride+std::size_t(ix)/8] & (1u<<(ix&7))) != 0;
}
void NativeTerrainHeightMap::readHeightChunk(DataChunkInput& input,
                                            DataChunkVersionType version,
                                            Purpose purpose) {
  if (version < K_HEIGHT_MAP_VERSION_1 || version > K_HEIGHT_MAP_VERSION_4)
    throw ERROR_CORRUPT_FILE_FORMAT;
  NativeTerrainHeightMap candidate;
  candidate.m_purpose = purpose;
  candidate.m_width = input.readInt();
  candidate.m_height = input.readInt();
  candidate.m_border = version >= K_HEIGHT_MAP_VERSION_3 ? input.readInt() : 0;
  const auto width = candidate.m_width, height = candidate.m_height;
  const std::int64_t innerWidth = std::int64_t(width) - 2LL * candidate.m_border;
  const std::int64_t innerHeight = std::int64_t(height) - 2LL * candidate.m_border;
  if (width <= 0 || height <= 0 || candidate.m_border < 0 ||
      innerWidth <= 0 || innerHeight <= 0)
    throw ERROR_CORRUPT_FILE_FORMAT;
  if (version >= K_HEIGHT_MAP_VERSION_4) {
    const Int count = input.readInt();
    // Every boundary needs two fixed-width integers, followed by byte count.
    const auto left = input.getChunkDataSizeLeft();
    if (count < 0 || left < 4 || std::uint64_t(count) * 8 > left - 4)
      throw ERROR_CORRUPT_FILE_FORMAT;
    for (Int index = 0; index < count; ++index) {
      ICoord2D boundary;
      boundary.x = input.readInt(); boundary.y = input.readInt();
      // Serialized boundary values are preserved. World admission must validate
      // the selected playable boundary; the editor supports dormant zero entries.
      candidate.m_boundaries.push_back(boundary);
    }
  } else {
    candidate.m_boundaries.push_back({static_cast<Int>(innerWidth),
                                      static_cast<Int>(innerHeight)});
  }
  const Int count = input.readInt();
  const std::int64_t product = std::int64_t(width) * height;
  if (count <= 0 || product != count ||
      static_cast<UnsignedInt>(count) != input.getChunkDataSizeLeft())
    throw ERROR_CORRUPT_FILE_FORMAT;
  // Declared chunk lengths are not proof of physical backing. A forged/truncated
  // envelope must not cause an enormous eager resize before its first read.
  std::array<UnsignedByte,4096> block;
  for (Int remaining = count; remaining > 0;) {
    const Int length = std::min<Int>(remaining, block.size());
    input.readArrayOfBytes(reinterpret_cast<char*>(block.data()), length);
    candidate.m_bytes.insert(candidate.m_bytes.end(), block.begin(), block.begin()+length);
    remaining -= length;
  }
  if (!input.atEndOfChunk()) throw ERROR_CORRUPT_FILE_FORMAT;
  if (version == K_HEIGHT_MAP_VERSION_1) {
    const Int newWidth = width / 2 + width % 2;
    const Int newHeight = height / 2 + height % 2;
    for (Int y = 0; y < newHeight; ++y)
      for (Int x = 0; x < newWidth; ++x)
        candidate.m_bytes[std::size_t(y) * newWidth + x] =
          candidate.m_bytes[std::size_t(2) * y * width + 2 * x];
    // Preserve the distinct original height-chunk contracts, including retained
    // tail bytes/boundaries. BlendTileData v1 completes full-reader dimensions;
    // height-only loading must not invent completion of that unread transition.
    if (purpose == Purpose::LogicalMetadata) {
      candidate.m_width = newWidth; candidate.m_height = newHeight;
    }
  }
  swap(candidate);
}
UnsignedByte NativeTerrainHeightMap::rawHeight(Int x, Int y) const {
  if (x < 0 || y < 0 || x >= m_width || y >= m_height || m_bytes.empty())
    throw ERROR_BAD_ARG;
  return m_bytes[std::size_t(y) * m_width + x];
}
Real NativeTerrainHeightMap::groundHeight(Real x, Real y, Coord3D* normal,
                                         const NativeTerrainHeightMap* displayGrid) const {
  const auto ix = gridCoordinate(x, m_border), iy = gridCoordinate(y, m_border);
  if (m_bytes.empty()) { upNormal(normal); return 0; }
  const auto& clipped = displayGrid ? *displayGrid : *this;
  if (ix < 1 || iy < 1 || ix > m_width - 3 || iy > m_height - 3) {
    upNormal(normal);
    if (clipped.m_bytes.empty()) return 0;
    return clipped.rawHeight(static_cast<Int>(std::clamp<std::int64_t>(ix, 0, clipped.m_width - 1)),
                             static_cast<Int>(std::clamp<std::int64_t>(iy, 0, clipped.m_height - 1))) * MAP_HEIGHT_SCALE;
  }
  const Real xd = x * (1.0f / MAP_XY_FACTOR), yd = y * (1.0f / MAP_XY_FACTOR);
  const Real fx = xd - std::floor(xd), fy = yd - std::floor(yd);
  const auto sample = [&](Int dx, Int dy) -> Real {
    return rawHeight(static_cast<Int>(ix) + dx, static_cast<Int>(iy) + dy);
  };
  const Real p0 = sample(0,0), p1 = sample(1,0), p2 = sample(1,1), p3 = sample(0,1);
  const Real result = fy > fx
    ? (p3 + (1.0f-fy)*(p0-p3) + fx*(p2-p3))*MAP_HEIGHT_SCALE
    : (p1 + fy*(p2-p1) + (1.0f-fx)*(p0-p1))*MAP_HEIGHT_SCALE;
  if (normal) {
    const Real dx0 = p1 - sample(-1,0), dx1 = sample(2,0) - p0;
    const Real dx2 = sample(2,1) - p3, dx3 = dx1;
    const Real dy0 = p3 - sample(0,-1), dy1 = p2 - sample(1,-1);
    const Real dy2 = sample(1,2) - p1, dy3 = sample(0,2) - p0;
    const Real dxLeft = dx0*(1.0f-fx)+fx*dx3, dxRight = dx1*(1.0f-fx)+fx*dx2;
    const Real dyLeft = dy0*(1.0f-fx)+fx*dy3, dyRight = dy1*(1.0f-fx)+fx*dy2;
    const Real dx = dxLeft*(1.0-fy)+fy*dxRight;
    const Real dy = dyLeft*(1.0-fy)+fy*dyRight;
    // Original Vector3 cross product and portable Inv_Sqrt representation.
    const Real span = 2*MAP_XY_FACTOR/MAP_HEIGHT_SCALE;
    const Real nx = 0.0f*dy-dx*span, ny = dx*0.0f-span*dy, nz = span*span-0.0f*0.0f;
    const Real inverse = 1.0f / static_cast<Real>(std::sqrt(static_cast<double>(nx*nx+ny*ny+nz*nz)));
    normal->x = nx*inverse; normal->y = ny*inverse; normal->z = nz*inverse;
  }
  return result;
}
Bool NativeTerrainHeightMap::clearLineOfSight(const Coord3D& from, const Coord3D& to,
                                            Real displayMaximumHeight) const {
  auto x = gridCoordinate(from.x, m_border), y = gridCoordinate(from.y, m_border);
  const auto endX = gridCoordinate(to.x, m_border), endY = gridCoordinate(to.y, m_border);
  if (!std::isfinite(from.z) || !std::isfinite(to.z) || !std::isfinite(displayMaximumHeight))
    throw ERROR_BAD_ARG;
  if (m_bytes.empty()) return FALSE;
  const auto dx = std::abs(endX - x), dy = std::abs(endY - y);
  const auto pixels = std::max(dx, dy);
  if (!pixels) return TRUE;
  const std::int64_t sx = endX >= x ? 1 : -1, sy = endY >= y ? 1 : -1;
  const std::int64_t x1 = dx >= dy ? 0 : sx, y1 = dx >= dy ? sy : 0;
  const std::int64_t x2 = dx >= dy ? sx : 0, y2 = dx >= dy ? 0 : sy;
  const auto add = std::min(dx, dy);
  auto numerator = pixels / 2;
  Real z = from.z;
  const Real zinc = (to.z - z) * (1.0f / static_cast<Real>(pixels));
  for (std::int64_t pixel = 0; pixel < pixels; ++pixel) {
    if (x < 0 || y < 0 || x >= m_width - 1 || y >= m_height - 1) break;
    const Int ix = static_cast<Int>(x), iy = static_cast<Int>(y);
    const Real height = std::max({rawHeight(ix,iy), rawHeight(ix+1,iy),
                                 rawHeight(ix,iy+1), rawHeight(ix+1,iy+1)}) * MAP_HEIGHT_SCALE;
    if (height > z + 0.5f) return FALSE;
    if (z >= displayMaximumHeight && zinc > 0.0f) break;
    z += zinc;
    numerator += add;
    if (numerator >= pixels) { numerator -= pixels; x += x1; y += y1; }
    x += x2; y += y2;
  }
  return TRUE;
}
