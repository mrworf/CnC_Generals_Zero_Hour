// SPDX-License-Identifier: GPL-3.0-or-later
// Algorithms derived from EA WorldHeightMap.cpp and BaseHeightMap.cpp.
#include "GameLogic/NativeTerrainHeightMap.h"
#include "Common/Terrain.h"
#include <algorithm>
#include <array>
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
void NativeTerrainHeightMap::readHeightChunk(DataChunkInput& input,
                                            DataChunkVersionType version,
                                            Purpose purpose) {
  if (version < K_HEIGHT_MAP_VERSION_1 || version > K_HEIGHT_MAP_VERSION_4)
    throw ERROR_CORRUPT_FILE_FORMAT;
  NativeTerrainHeightMap candidate;
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
