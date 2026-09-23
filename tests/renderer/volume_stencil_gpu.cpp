#include "volume_stencil_contract.h"
#include "zh/platform/bgfx_device.h"

#include <array>
#include <cstdint>
#include <stdexcept>
#include <string>

using namespace zh::renderer;

namespace {
void check(bool value, const std::string& message)
{
    if (!value) throw std::runtime_error(message);
}

struct Vertex { float x, y; std::uint32_t color; float u, v; };

void run_generation()
{
    BgfxOptions options;
    options.shader_root=ZH_BGFX_SHADER_DIR;
    BgfxGpuDevice device(options);
    check(device.supports_texture_format(TextureFormat::depth24_stencil8,TextureDimension::texture_2d,false,true),
        "public edge does not support D24S8 render targets");

    const auto vs=device.create_shader({ShaderStage::vertex,"renderer/acceptance.vert",0,0},"volume edge vertex");
    const auto fs=device.create_shader({ShaderStage::fragment,"renderer/acceptance.frag",0,0},"volume edge fragment");
    const auto protocol=zh::original_runtime::make_volume_stencil_protocol(vs,fs,TextureFormat::rgba8,0x80);
    const auto increment=device.create_pipeline(PipelineKey(protocol.increment),"volume edge increment");
    const auto decrement=device.create_pipeline(PipelineKey(protocol.decrement),"volume edge decrement");
    const auto composite=device.create_pipeline(PipelineKey(protocol.composite),"volume edge composite");
    const std::array<Vertex,3> vertices{{
        {-0.8F,-0.8F,0xffffffffU,0.0F,0.0F}, {0.8F,-0.8F,0xffffffffU,1.0F,0.0F},
        {0.0F,0.8F,0xffffffffU,0.5F,1.0F}}};
    const auto vb=device.create_buffer({sizeof(vertices),BufferUsage::vertex,true},"volume edge vertices");
    TextureDesc target; target.width=32; target.height=32; target.format=TextureFormat::rgba8; target.render_target=true;
    const auto color=device.create_texture(target,"volume edge color");
    target.format=TextureFormat::depth24_stencil8; target.sampled=false;
    const auto depth=device.create_texture(target,"volume edge depth");
    check(vs && fs && increment && decrement && composite && vb && color && depth &&
        device.upload({vb,sizeof(vertices),0,sizeof(vertices)},vertices.data()), device.last_error());
    RenderPassDesc pass; pass.color_targets[0]=color; pass.color_target_count=1; pass.depth_target=depth; pass.width=32; pass.height=32;
    DrawDesc draw; draw.vertex_buffer=vb; draw.vertex_or_index_count=3; draw.pipeline=increment;
    check(device.begin_pass(pass,"volume public stencil edge"),device.last_error());
    check(device.draw(draw),device.last_error());
    draw.pipeline=decrement; check(device.draw(draw),device.last_error());
    draw.pipeline=composite; check(device.draw(draw),device.last_error());
    check(device.end_pass(),device.last_error());
    device.destroy(depth); device.destroy(color); device.destroy(vb); device.destroy(composite); device.destroy(decrement);
    device.destroy(increment); device.destroy(fs); device.destroy(vs);
    check(device.wait_idle(),device.last_error());
}
}

int main()
{
    for (int generation=0; generation!=2; ++generation) run_generation();
}
