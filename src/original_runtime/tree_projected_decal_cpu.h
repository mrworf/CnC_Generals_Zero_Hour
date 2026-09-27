#pragma once

// Owner-local translation of W3DProjectedShadowManager::queueDecal's object-less
// tree route. No object shadow, global streaming ring or Windows ABI is exposed.
#include "W3DDevice/GameClient/WorldHeightMap.h"
#include "WW3D2/dx8fvf.h"
#include <algorithm>
#include <cmath>
#include <cstdint>
#include <limits>
#include <vector>

class BaseHeightMapRenderObjClass;
class TextureClass;

namespace zh::original_runtime::detail {
struct TreeDecalIntent { DrawableID id{}; Coord3D position{0,0,0}; Real size=0; };
struct TreeDecalRectangle { Int startX=0,startY=0,endX=0,endY=0; };
struct TreeDecalBatch {
    std::vector<VertexFormatXYZDUV1> vertices;
    std::vector<UnsignedShort> indices;
};
struct TreeDecalQueue {
    TreeDecalRectangle rectangle;
    std::vector<TreeDecalIntent> intents;
    std::vector<TreeDecalBatch> batches;
};
// Read-only generated-owner witness; no runtime selector, registry mutation,
// serialized field or public engine-header surface.
struct TreeDecalGeneratedProbeAccess {
    static const TreeDecalQueue* peek(const BaseHeightMapRenderObjClass*) noexcept;
    static TextureClass* texture(const BaseHeightMapRenderObjClass*) noexcept;
    static bool objectless(const BaseHeightMapRenderObjClass*) noexcept;
};
inline bool build_tree_decal_queue(WorldHeightMap& map,
    const std::vector<TreeDecalIntent>& intents,TreeDecalQueue& result)
{
    if (intents.size()>4000 || map.getXExtent()<=0 || map.getYExtent()<=0
        || map.getDrawOrgX()<0 || map.getDrawOrgY()<0 || map.getDrawWidth()<=0
        || map.getDrawHeight()<=0 || !map.getDataPtr()) return false;
    TreeDecalQueue candidate;
    const auto end=[](Int origin,Int width,Int extent) {
        return static_cast<Int>(std::min<std::int64_t>(std::int64_t(origin)+width-1,extent-1));
    };
    candidate.rectangle={map.getDrawOrgX(),map.getDrawOrgY(),
        end(map.getDrawOrgX(),map.getDrawWidth(),map.getXExtent()),
        end(map.getDrawOrgY(),map.getDrawHeight(),map.getYExtent())};
    const auto& rectangle=candidate.rectangle;
    if (rectangle.startX>rectangle.endX || rectangle.startY>rectangle.endY) return false;
    const Int border=map.getBorderSizeInline();
    if (border<0 || border>=map.getXExtent() || border>=map.getYExtent()) return false;
    std::size_t totalBytes=0;
    for (const auto& intent:intents) {
        if (!std::isfinite(intent.size) || intent.size<=0 || !std::isfinite(intent.position.x)
            || !std::isfinite(intent.position.y) || !std::isfinite(intent.position.z)) return false;
        // Native size Y is negative; the -Y basis cancels that sign for UV V.
        const Real half=intent.size*0.5f;
        const auto coordinate=[&](Real position,Real offset,bool upper,Int low,Int high,Int& output) {
            const Real cell=(position+offset)*(1.0f/MAP_XY_FACTOR);
            if (!std::isfinite(cell)) return false;
            const double value=(upper?std::ceil(double(cell)):std::floor(double(cell)))+border;
            if (value<std::numeric_limits<Int>::min() || value>std::numeric_limits<Int>::max()) return false;
            output=std::clamp(static_cast<Int>(value),low,high);return true;
        };
        Int sx,sy,ex,ey;
        if (!coordinate(intent.position.x,-half,false,rectangle.startX,rectangle.endX,sx)
            || !coordinate(intent.position.x,half,true,rectangle.startX,rectangle.endX,ex)
            || !coordinate(intent.position.y,-half,false,rectangle.startY,rectangle.endY,sy)
            || !coordinate(intent.position.y,half,true,rectangle.startY,rectangle.endY,ey)) return false;
        const auto clip=[](Int& start,Int& finish) {
            const Int extra=finish-start+1-104;
            if (extra>0) { start+=extra/2;finish-=extra-extra/2; }
        };
        clip(sx,ex);clip(sy,ey);
        const Int rows=ey-sy+1,columns=ex-sx+1;
        candidate.intents.push_back(intent);
        if (rows<=1 || columns<=1) continue;
        const std::size_t vertexCount=std::size_t(rows)*columns;
        const std::size_t indexCount=std::size_t(rows-1)*(columns-1)*6;
        const std::size_t bytes=vertexCount*sizeof(VertexFormatXYZDUV1)+indexCount*sizeof(UnsignedShort);
        if (vertexCount>32768 || indexCount>65536 || bytes>64U*1024U*1024U-totalBytes) return false;
        totalBytes+=bytes;
        if (candidate.batches.empty() || candidate.batches.back().vertices.size()+vertexCount>32768
            || candidate.batches.back().indices.size()+indexCount>65535) {
            // Original DX8 CPU buffer counts are unsigned16; 65536 must split
            // rather than narrow to zero. Native ring capacity itself is unchanged.
            if (candidate.batches.size()==256) return false;
            candidate.batches.emplace_back();
        }
        auto& batch=candidate.batches.back();
        const auto base=static_cast<unsigned>(batch.vertices.size());
        const Real inverse=1.0f/intent.size;
        if (!std::isfinite(inverse)) return false;
        for (Int y=sy;y<=ey;++y) for (Int x=sx;x<=ex;++x) {
            VertexFormatXYZDUV1 vertex{};
            vertex.x=Real(x-border)*MAP_XY_FACTOR;vertex.y=Real(y-border)*MAP_XY_FACTOR;
            vertex.z=Real(map.getHeight(x,y))*MAP_HEIGHT_SCALE+0.01f*MAP_XY_FACTOR;
            vertex.diffuse=0xffffffffU;
            vertex.u1=(vertex.x-intent.position.x)*inverse+0.5f;
            vertex.v1=(vertex.y-intent.position.y)*inverse+0.5f;
            if (!std::isfinite(vertex.x) || !std::isfinite(vertex.y) || !std::isfinite(vertex.z)
                || !std::isfinite(vertex.u1) || !std::isfinite(vertex.v1)) return false;
            batch.vertices.push_back(vertex);
        }
        for (Int y=sy;y<ey;++y) for (Int x=sx;x<ex;++x) {
            const unsigned i=base+unsigned((y-sy)*columns+x-sx),row=unsigned(columns);
            if (map.getFlipState(x,y)) {
                for (const unsigned index:{i+1,i+row,i,i+1,i+1+row,i+row}) batch.indices.push_back(static_cast<UnsignedShort>(index));
            } else {
                for (const unsigned index:{i,i+1+row,i+row,i,i+1,i+1+row}) batch.indices.push_back(static_cast<UnsignedShort>(index));
            }
        }
    }
    result=std::move(candidate);return true;
}
}
