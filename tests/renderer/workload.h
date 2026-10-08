#pragma once
#include <array>
#include <cstdint>

namespace workload {
// Source owners/formulas and explicitly chosen stress parameters are recorded
// in docs/renderer-workload-census.md. These are test sizes, never game limits.
constexpr uint32_t cells=16, extent=1025, tileSide=(extent+cells-2)/cells;
constexpr uint32_t tiles=tileSide*tileSide, gridVertices=(cells+1)*(cells+1), gridIndices=cells*cells*6;
constexpr uint16_t baseTexture=cells*8, highTexture=baseTexture*4;
constexpr uint32_t loadingBatch=32, casters=64, submeshes=160, shadowLights=1;
struct Group { const char* name; uint32_t draws; };
constexpr std::array groups={
    Group{"terrain",tiles*2}, Group{"models",casters*submeshes},
    Group{"volumes",casters*submeshes*shadowLights*2}, Group{"trees",64*2},
    Group{"bridges",200*2}, Group{"shorelines",4096/512}, Group{"particles",16},
    Group{"shroud",tiles}, Group{"markers",64}, Group{"water",8},
    Group{"distortion",2}, Group{"ui",128}, Group{"high-LOD",64}
};
constexpr uint32_t mixedDraws=[] { uint32_t n=0; for(auto group:groups)n+=group.draws; return n; }();
static_assert(mixedDraws==43826);
} // namespace workload
