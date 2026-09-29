#include "zh/platform/bgfx_device.h"

#include <algorithm>
#include <array>
#include <iostream>
#include <stdexcept>
#include <string>
#include <vector>

namespace {

void check(bool condition, const char* message)
{
    if (!condition) throw std::runtime_error(message);
}

std::vector<zh::renderer::UInt8> pixels(unsigned width, unsigned height, unsigned bytes_per_pixel, unsigned seed)
{
    std::vector<zh::renderer::UInt8> result(width * height * bytes_per_pixel);
    for (unsigned index = 0; index < result.size(); ++index)
        result[index] = static_cast<zh::renderer::UInt8>(seed + index);
    return result;
}

void test_multi_mip_lifecycle()
{
    using namespace zh::renderer;
    BgfxOptions options;
    options.shader_root = ZH_BGFX_SHADER_DIR;

    for (unsigned generation = 0; generation != 2; ++generation) {
        BgfxGpuDevice device(options);
        for (const auto format : {TextureFormat::rgba8, TextureFormat::bgr5a1}) {
            const unsigned bytes_per_pixel = format == TextureFormat::bgr5a1 ? 2U : 4U;
            TextureDesc desc;
            desc.width = 8;
            desc.height = 4;
            desc.format = format;
            desc.mip_levels = 4;
            auto texture = device.create_texture(desc, "M22 multi-mip source atlas");
            check(bool(texture), "multi-mip sampled texture allocation failed");

            for (unsigned mip = 0; mip != desc.mip_levels; ++mip) {
                const unsigned width = std::max(1U, desc.width >> mip);
                const unsigned height = std::max(1U, desc.height >> mip);
                const auto bytes = pixels(width, height, bytes_per_pixel, generation * 17U + mip);
                check(device.upload_texture({texture, width, height, width * bytes_per_pixel, bytes.size(), mip}, bytes.data()),
                    "declared source atlas mip upload failed");
            }

            const auto level_zero = pixels(8, 4, bytes_per_pixel, 91);
            check(!device.upload_texture({texture, 8, 4, 8 * bytes_per_pixel, level_zero.size(), 4}, level_zero.data()),
                "out-of-range mip upload accepted");
            check(!device.upload_texture({texture, 4, 4, 4 * bytes_per_pixel, level_zero.size(), 0}, level_zero.data()),
                "wrong base mip extent accepted");
            check(!device.upload_texture({texture, 1, 1, bytes_per_pixel - 1U, bytes_per_pixel, 3}, level_zero.data()),
                "short mip row pitch accepted");
            check(!device.upload_texture({texture, 1, 1, bytes_per_pixel, bytes_per_pixel * 2U, 3}, level_zero.data()),
                "oversized mip payload accepted");

            TextureDesc over_mipped = desc;
            over_mipped.mip_levels = 5;
            check(!device.create_texture(over_mipped, "oversized mip chain"),
                "mip chain beyond source extent accepted");
            TextureDesc render_target = desc;
            render_target.render_target = true;
            check(!device.create_texture(render_target, "mipped render target"),
                "mipped render target accepted");

            device.destroy(texture);
            check(!device.upload_texture({texture, 8, 4, 8 * bytes_per_pixel, level_zero.size(), 0}, level_zero.data()),
                "retired multi-mip texture accepted an upload");
            auto replacement = device.create_texture(desc, "recreated M22 multi-mip source atlas");
            check(replacement && replacement != texture, "multi-mip texture did not recreate with new generation");
            device.destroy(replacement);
        }
        check(device.wait_idle() && device.live_resource_count() == 0,
            "multi-mip texture lifecycle leaked resources");
    }
}

// All declared levels are authored; the last is green, while level zero is
// red. Extreme minification must see green, never bgfx's undeclared tail.
void test_declared_sampled_range()
{
    using namespace zh::renderer;
    for (unsigned generation=0; generation<2; ++generation) {
        BgfxOptions options; options.shader_root=ZH_BGFX_SHADER_DIR;
        BgfxGpuDevice device(options);
        auto vs=device.create_shader({ShaderStage::vertex,"renderer/video.vert",1,0},"mip viewport");
        auto fs=device.create_shader({ShaderStage::fragment,"renderer/video.frag",0,1},"mip colors");
        PipelineDesc state; state.vertex_shader=vs; state.fragment_shader=fs;
        state.vertex_layout=VertexLayout::position_color_uv; state.color_format=TextureFormat::bgra8;
        state.raster.cull=CullMode::none;
        auto pipeline=device.create_pipeline(PipelineKey(state),"declared mip program");
        const std::array<float,4> viewport{32,32,0,0};
        struct Vertex { float x,y; UInt32 color; float u,v; };
        std::array<Vertex,3> triangle{{{0,0,0xffffffff,0,0},{64,0,0xffffffff,2048,0},{0,64,0xffffffff,0,2048}}};
        const std::array<UInt16,3> indices{0,1,2};
        auto uniform=device.create_buffer({sizeof(viewport),BufferUsage::uniform,true},"mip viewport bytes");
        auto vertices=device.create_buffer({sizeof(triangle),BufferUsage::vertex,true},"mip geometry");
        auto index=device.create_buffer({sizeof(indices),BufferUsage::index,true},"mip indices");
        check(vs && fs && pipeline && uniform && vertices && index
            && device.upload({uniform,sizeof(viewport),0,sizeof(viewport)},viewport.data())
            && device.upload({vertices,sizeof(triangle),0,sizeof(triangle)},triangle.data())
            && device.upload({index,sizeof(indices),0,sizeof(indices)},indices.data()),"mip draw setup");
        TextureDesc target; target.width=target.height=32; target.render_target=true; target.format=TextureFormat::bgra8;
        auto color=device.create_texture(target,"mip pixel target");
        target.format=TextureFormat::depth24_stencil8;
        auto depth=device.create_texture(target,"mip pixel depth");
        check(color && depth,"mip target setup");
        for (const auto format : {TextureFormat::rgba8,TextureFormat::bgra8,TextureFormat::bgr5a1})
        for (const auto extent : {std::array<unsigned,3>{8,8,2}, {16,4,3}, {1,8,2}, {8,1,2}, {8,4,4}, {1,1,1}})
        for (const auto filtering : {Filter::nearest,Filter::linear}) {
            TextureDesc desc; desc.width=extent[0];desc.height=extent[1];desc.mip_levels=extent[2];desc.format=format;
            auto sample=device.create_texture(desc,"exact partial or full source");
            SamplerDesc sampling; sampling.min_filter=sampling.mag_filter=Filter::nearest;sampling.mip_filter=filtering;
            auto sampler=device.create_sampler(sampling,"unchanged mip policy");
            check(sample && sampler,"sampled range setup");
            const unsigned bpp=format==TextureFormat::bgr5a1 ? 2 : 4;
            for (unsigned mip=0;mip<desc.mip_levels;++mip) {
                const auto width=std::max(1U,desc.width>>mip),height=std::max(1U,desc.height>>mip);
                std::vector<UInt8> bytes(width*height*bpp);
                const bool green=mip+1==desc.mip_levels;
                for (unsigned pixel=0;pixel<width*height;++pixel) {
                    if (bpp==2) { const UInt16 packed=green ? 0x83e0 : 0xfc00;
                        bytes[pixel*2]=static_cast<UInt8>(packed);bytes[pixel*2+1]=static_cast<UInt8>(packed>>8); }
                    else {bytes[pixel*4+(green ? 1 : format==TextureFormat::bgra8 ? 2 : 0)]=255;bytes[pixel*4+3]=255;}
                }
                if (mip+1==desc.mip_levels && mip) {
                    RenderPassDesc pass;pass.color_targets[0]=color;pass.color_target_count=1;pass.depth_target=depth;
                    pass.width=pass.height=32;
                    DrawDesc draw;draw.pipeline=pipeline;draw.vertex_buffer=vertices;draw.index_buffer=index;
                    draw.index_element_size=IndexElementSize::uint16;draw.vertex_or_index_count=3;
                    draw.vertex_bindings.uniform_count=1;draw.vertex_bindings.uniforms[0]={uniform,0,sizeof(viewport)};
                    draw.fragment_bindings.texture_count=1;draw.fragment_bindings.textures[0]=sample;draw.fragment_bindings.samplers[0]=sampler;
                    check(device.begin_pass(pass,"unknown mip rejection") && !device.draw(draw) && device.end_pass(),
                        "unknown exposed mip admitted");
                }
                check(device.upload_texture({sample,width,height,width*bpp,bytes.size(),mip},bytes.data()),"authored mip bytes");
            }
            RenderPassDesc pass;pass.color_targets[0]=color;pass.color_target_count=1;pass.depth_target=depth;
            pass.width=pass.height=32;
            DrawDesc draw;draw.pipeline=pipeline;draw.vertex_buffer=vertices;draw.index_buffer=index;
            draw.index_element_size=IndexElementSize::uint16;draw.vertex_or_index_count=3;
            draw.vertex_bindings.uniform_count=1;draw.vertex_bindings.uniforms[0]={uniform,0,sizeof(viewport)};
            draw.fragment_bindings.texture_count=1;draw.fragment_bindings.textures[0]=sample;draw.fragment_bindings.samplers[0]=sampler;
            check(device.begin_pass(pass,"declared range only") && device.draw(draw) && device.end_pass(),device.last_error().c_str());
            const auto result=device.readback_rgba(color);const auto center=(16U*32U+16U)*4U;
            check(result.size()==32*32*4 && result[center]==0 && result[center+1]==255
                && result[center+2]==0 && result[center+3]==255,"partial chain sampled undeclared native mip");
            device.destroy(sample);
            device.destroy(sampler);
        }
        device.destroy(color);device.destroy(depth);device.destroy(index);device.destroy(vertices);device.destroy(uniform);
        device.destroy(pipeline);device.destroy(vs);device.destroy(fs);
        check(device.wait_idle() && device.live_resource_count()==0,"declared range teardown residual");
    }
}

} // namespace

int main()
{
    try {
        test_multi_mip_lifecycle();
        test_declared_sampled_range();
        std::cout << "bgfx multi-mip texture contract: ok\n";
        return 0;
    } catch (const std::exception& error) {
        std::cerr << error.what() << '\n';
        return 1;
    }
}
