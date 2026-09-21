#include "zh/platform/sdl_gpu_device.h"
#include <SDL3/SDL.h>

#include <array>
#include <stdexcept>
#include <iostream>

namespace {
void check(bool condition,const char* message) {
    if (!condition) throw std::runtime_error(message);
}
}

int main() {
    try {
        check(SDL_Init(SDL_INIT_VIDEO),"SDL video initialization failed");
        {
        zh::renderer::SdlGpuOptions options;
        options.shader_root=ZH_GPU_SHADER_DIR;
        options.debug=true;
        zh::renderer::SdlGpuDevice device(options);
        using namespace zh::renderer;
        check(device.capabilities().backend=="vulkan","SDL_GPU selected non-Vulkan backend");
        TextureDesc desc;
        desc.width=8; desc.height=8; desc.mip_levels=2; desc.format=TextureFormat::bc1;
        check(device.supports_texture_format(desc.format,desc.dimension,true,false),
            "device does not support reached retail BC1 format");
        auto texture=device.create_texture(desc,"original owned BC1 mip upload");
        check(static_cast<bool>(texture),"BC1 texture creation failed");
        std::array<UInt8,32> first{};
        std::array<UInt8,8> second{};
        first[0]=17; second[0]=23;
        TextureUploadDesc upload{texture,8,8,16,first.size(),0};
        check(device.upload_texture(upload,first.data()),"BC1 top mip upload failed");
        upload={texture,4,4,8,second.size(),1};
        check(device.upload_texture(upload,second.data()),"BC1 reduced mip upload failed");
        device.destroy(texture);
        desc.width=2; desc.height=2; desc.mip_levels=1; desc.format=TextureFormat::bgra8;
        texture=device.create_texture(desc,"original owned BGRA upload");
        check(static_cast<bool>(texture),"BGRA texture creation failed");
        std::array<UInt8,16> pixels{0,1,2,255,3,4,5,255,6,7,8,255,9,10,11,255};
        check(device.upload_texture({texture,2,2,8,pixels.size(),0},pixels.data()),
            "BGRA upload failed");
        device.destroy(texture);
        SamplerDesc source_filter;
        source_filter.min_filter=Filter::linear;
        source_filter.mag_filter=Filter::linear;
        source_filter.mip_filter=Filter::nearest;
        source_filter.address_u=AddressMode::clamp_edge;
        source_filter.maximum_lod=0.0F;
        auto sampler=device.create_sampler(source_filter,"original source no-mip clamp sampler");
        check(static_cast<bool>(sampler),"source no-mip sampler creation failed");
        device.destroy(sampler);
        source_filter.maximum_lod=1000.0F;
        sampler=device.create_sampler(source_filter,"original source enabled-mip sampler");
        check(static_cast<bool>(sampler),"source enabled-mip sampler creation failed");
        device.destroy(sampler);
        source_filter.maximum_anisotropy=2;
        sampler=device.create_sampler(source_filter,"original source anisotropic stage-zero sampler");
        check(static_cast<bool>(sampler),"source anisotropic sampler creation failed");
        device.destroy(sampler);
        check(device.wait_idle(),"texture upload did not complete");
        std::cout << "original W3D texture GPU upload: vulkan BC1 BGRA mips ok\n";
        }
        {
        zh::renderer::SdlGpuOptions options;
        options.shader_root=ZH_GPU_SHADER_DIR;
        options.debug=true;
        zh::renderer::SdlGpuDevice recreated(options);
        zh::renderer::SamplerDesc source_filter;
        source_filter.maximum_lod=0.0F;
        auto sampler=recreated.create_sampler(source_filter,"original source recreated device sampler");
        check(static_cast<bool>(sampler),"recreated GPU sampler failed");
        recreated.destroy(sampler);
        check(recreated.wait_idle(),"recreated GPU sampler teardown failed");
        }
        SDL_Quit();
        return 0;
    } catch (const std::exception& error) {
        SDL_Quit();
        std::cerr << error.what() << '\n';
        return 1;
    }
}
