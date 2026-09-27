#pragma once

// Internal source-owned surface backing. It is not a SurfaceClass/device API.
#include "ww3dformat.h"
#include <cstddef>
#include <vector>
#include <algorithm>
#include <stdexcept>

namespace zh::original_runtime {
struct HouseColorTexturePixels {
    WW3DFormat format = WW3D_FORMAT_UNKNOWN;
    unsigned width = 0, height = 0;
    std::vector<unsigned> pitches;
    std::vector<std::vector<unsigned char>> mips;
};
inline constexpr std::size_t house_color_byte_limit = 64U * 1024U * 1024U;

// Explicit D3DX8-compatible BOX quantization (not a byte-exact D3DX8 claim).
inline void build_house_color_box_mips(HouseColorTexturePixels& pixels, unsigned requested)
{
    if ((pixels.format!=WW3D_FORMAT_A8R8G8B8 && pixels.format!=WW3D_FORMAT_X8R8G8B8) ||
        !pixels.width || !pixels.height || pixels.width>16384 || pixels.height>16384 ||
        (pixels.width&(pixels.width-1)) || (pixels.height&(pixels.height-1)) ||
        pixels.mips.empty() || pixels.pitches.empty() ||
        pixels.pitches[0]!=pixels.width*4 ||
        pixels.mips[0].size()!=std::size_t(pixels.width)*pixels.height*4)
        throw std::runtime_error("original color BOX surface is malformed");
    unsigned maximum=1;
    for (unsigned w=pixels.width,h=pixels.height;w>1 || h>1;
         w=std::max(1U,w/2),h=std::max(1U,h/2)) ++maximum;
    const unsigned count=requested ? requested : maximum;
    if (!count || count>maximum)
        throw std::runtime_error("original color BOX mip count is invalid");
    std::size_t total=0;
    for (unsigned level=0;level<count;++level) {
        const std::size_t bytes=std::size_t(std::max(1U,pixels.width>>level))*
            std::max(1U,pixels.height>>level)*4;
        if (bytes>house_color_byte_limit || total>house_color_byte_limit-bytes)
            throw std::runtime_error("original color BOX mip budget is exceeded");
        total+=bytes;
    }
    HouseColorTexturePixels candidate;
    candidate.format=pixels.format;candidate.width=pixels.width;candidate.height=pixels.height;
    candidate.mips.reserve(count);candidate.pitches.reserve(count);
    candidate.mips.push_back(pixels.mips[0]);candidate.pitches.push_back(pixels.pitches[0]);
    unsigned w=pixels.width,h=pixels.height;
    for (unsigned level=1;level<count;++level) {
        const unsigned next_w=std::max(1U,w/2),next_h=std::max(1U,h/2);
        const unsigned samples_x=w>1 ? 2 : 1,samples_y=h>1 ? 2 : 1;
        const unsigned samples=samples_x*samples_y;
        std::vector<unsigned char> mip(std::size_t(next_w)*next_h*4);
        const auto& prior=candidate.mips.back();
        for (unsigned y=0;y<next_h;++y) for (unsigned x=0;x<next_w;++x)
            for (unsigned channel=0;channel<4;++channel) {
                unsigned sum=0;
                for (unsigned sy=0;sy<samples_y;++sy) for (unsigned sx=0;sx<samples_x;++sx)
                    sum+=prior[(std::size_t(y*samples_y+sy)*w+x*samples_x+sx)*4+channel];
                mip[(std::size_t(y)*next_w+x)*4+channel]=
                    static_cast<unsigned char>((sum+samples/2)/samples);
            }
        candidate.mips.push_back(std::move(mip));candidate.pitches.push_back(next_w*4);
        w=next_w;h=next_h;
    }
    pixels=std::move(candidate);
}
}
