#include "../../src/renderer/bgfx_transaction_state.h"
#include <bgfx/bgfx.h>
#include <array>
#include <iostream>
#include <limits>
#include <stdexcept>

namespace {
void check(bool value,const char* message) { if (!value) throw std::runtime_error(message); }
void test()
{
    using namespace zh::renderer;
    using Limits=bgfx::BoundedSubmissionLimits;
    for (UInt32 levels=1;levels<=32;++levels) {
        for (UInt32 first=0;first<levels;++first) {
            check(Limits::acceptsMipRange(first,levels-first,levels)
                && Limits::acceptsMipRange(first,1,levels)
                && Limits::acceptsMipRange(first,255,levels),"exact native mip boundary rejected");
            check(!Limits::acceptsMipRange(first,0,levels)
                && !Limits::acceptsMipRange(first,levels-first+1,levels),"native empty/bound+1 admitted");
            check(detail::sampled_mip_range(first,levels-first,levels)
                && !detail::sampled_mip_range(first,levels-first+1,levels),"logical sampled boundary differs");
        }
        check(!Limits::acceptsMipRange(levels,1,levels)
            && !Limits::acceptsMipRange(std::numeric_limits<UInt32>::max(),1,levels)
            && !Limits::acceptsMipRange(0,std::numeric_limits<UInt32>::max(),levels),"overflow range admitted");
    }
    check(!Limits::acceptsMipRange(0,1,0) && !Limits::acceptsMipRange(0,1,256)
        && Limits::acceptsMipRange(254,1,255) && !Limits::acceptsMipRange(255,1,255),"native representation edge differs");
    const bgfx::BoundedSubmissionTexture legacy{};
    check(legacy.firstMip==0 && legacy.numMips==255,"legacy full-range default changed");
    const detail::BgfxCapturedSampler binding{8,7,6,5,4,0,3};
    check(detail::sampler_alias(binding,binding)==detail::BgfxSamplerAlias::identical,"equal range alias differs");
    for (unsigned field=0;field<2;++field) {
        auto other=binding;
        if (field==0) ++other.first_mip;else ++other.mip_count;
        check(detail::sampler_alias(binding,other)==detail::BgfxSamplerAlias::conflict,"different sampled range aliased");
        ++other.stage;++other.uniform;
        check(detail::sampler_alias(binding,other)==detail::BgfxSamplerAlias::distinct,"independent range stages coalesced");
    }
}
}
int main()
{
    try {test();std::cout<<"bgfx sampled mip range contract: ok\n";return 0;}
    catch (const std::exception& error) {std::cerr<<error.what()<<'\n';return 1;}
}
