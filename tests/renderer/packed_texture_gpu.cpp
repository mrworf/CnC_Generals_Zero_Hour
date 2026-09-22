#include "zh/platform/bgfx_device.h"
#include "zh/platform/sdl_gpu_device.h"

#include <SDL3/SDL.h>
#include <array>
#include <cstdint>
#include <memory>
#include <stdexcept>
#include <string>

using namespace zh::renderer;
namespace {
void check(bool value,const std::string& message) { if (!value) throw std::runtime_error(message); }
struct Vertex { float x,y; std::uint32_t color; float u,v; };

template <class Device> void run(Device& device)
{
    check(device.supports_texture_format(TextureFormat::bgr5a1,TextureDimension::texture_2d,true,false),
        "packed sampled texture capability missing");
    check(!device.supports_texture_format(TextureFormat::bgr5a1,TextureDimension::texture_2d,true,true),
        "packed render-target usage accepted");
    TextureDesc source_desc{2,2,1,1,TextureDimension::texture_2d,TextureFormat::bgr5a1,false,true};
    auto invalid_target_desc=source_desc; invalid_target_desc.render_target=true;
    check(!device.create_texture(invalid_target_desc,"invalid packed target"),
        "packed render-target texture creation succeeded");
    auto source=device.create_texture(source_desc,"packed terrain source");
    const std::array<std::uint16_t,4> texels{{0xfc00,0x83e0,0x801f,0x0000}};
    check(source && device.upload_texture({source,2,2,4,sizeof(texels),0},texels.data()),device.last_error());
    check(!device.upload_texture({source,2,2,2,sizeof(texels),0},texels.data()),
        "short packed row accepted");
    auto sampler=device.create_sampler({Filter::nearest,Filter::nearest,Filter::nearest},"packed nearest");
    auto vs=device.create_shader({ShaderStage::vertex,"renderer/packed_texture_probe.vert",0,0},"packed vertex");
    auto fs=device.create_shader({ShaderStage::fragment,"renderer/packed_texture_probe.frag",0,1},"packed fragment");
    PipelineDesc pd; pd.vertex_shader=vs; pd.fragment_shader=fs; pd.vertex_layout=VertexLayout::position_color_uv;
    pd.raster.cull=CullMode::none; pd.depth_stencil.depth_test=false; pd.depth_stencil.depth_write=false;
    auto pipeline=device.create_pipeline(PipelineKey(pd),"packed sample pipeline");
    const std::array<Vertex,6> vertices{{
        {-1,-1,0xffffffff,0,1},{1,-1,0xffffffff,1,1},{1,1,0xffffffff,1,0},
        {-1,-1,0xffffffff,0,1},{1,1,0xffffffff,1,0},{-1,1,0xffffffff,0,0}}};
    auto vb=device.create_buffer({sizeof(vertices),BufferUsage::vertex,true},"packed quad");
    check(sampler && vs && fs && pipeline && vb && device.upload({vb,sizeof(vertices),0,sizeof(vertices)},vertices.data()),device.last_error());
    TextureDesc target_desc{8,8,1,1,TextureDimension::texture_2d,TextureFormat::rgba8,true,true};
    auto target=device.create_texture(target_desc,"packed sample target");
    auto depth_desc=target_desc; depth_desc.format=TextureFormat::depth24_stencil8; depth_desc.sampled=false;
    auto depth=device.create_texture(depth_desc,"packed sample depth");
    RenderPassDesc pass; pass.color_targets[0]=target; pass.color_target_count=1; pass.depth_target=depth; pass.width=8; pass.height=8;
    DrawDesc draw; draw.pipeline=pipeline; draw.vertex_buffer=vb; draw.vertex_or_index_count=6;
    draw.fragment_bindings.textures[0]=source; draw.fragment_bindings.samplers[0]=sampler; draw.fragment_bindings.texture_count=1;
    check(target && depth && device.begin_pass(pass,"packed sample") && device.draw(draw) && device.end_pass(),device.last_error());
    const auto pixels=device.readback_rgba(target); check(pixels.size()==8*8*4,device.last_error());
    unsigned red=0,green=0,blue=0,transparent=0;
    for (std::size_t i=0;i<pixels.size();i+=4) {
        red += pixels[i]>240 && pixels[i+1]<16 && pixels[i+2]<16 && pixels[i+3]>240;
        green += pixels[i]<16 && pixels[i+1]>240 && pixels[i+2]<16 && pixels[i+3]>240;
        blue += pixels[i]<16 && pixels[i+1]<16 && pixels[i+2]>240 && pixels[i+3]>240;
        transparent += pixels[i+3]<16;
    }
    check(red && green && blue && transparent,"packed channel/alpha shader sample changed");
    device.destroy(depth); device.destroy(target); device.destroy(vb); device.destroy(pipeline); device.destroy(fs); device.destroy(vs);
    device.destroy(sampler); device.destroy(source); check(device.wait_idle(),device.last_error());
}
}
int main(int argc,char** argv)
{
    check(argc==2,"backend argument required");
    if (std::string(argv[1])=="bgfx") {
        for (int i=0;i<2;++i) { BgfxOptions options; options.shader_root=ZH_BGFX_SHADER_DIR; BgfxGpuDevice device(options); run(device); }
    } else {
        check(SDL_Init(SDL_INIT_VIDEO),SDL_GetError());
        for (int i=0;i<2;++i) { SdlGpuOptions options; options.shader_root=ZH_GPU_SHADER_DIR; SdlGpuDevice device(options); run(device); }
        SDL_Quit();
    }
}
