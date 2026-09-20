#include "zh/renderer/contract.h"

#include <cmath>
#include <iostream>
#include <stdexcept>
#include <string>
#include <unordered_set>

using namespace zh::renderer;

namespace {

void check(bool condition, const char* message)
{
    if (!condition) throw std::runtime_error(message);
}

PipelineDesc valid_pipeline()
{
    PipelineDesc desc;
    desc.vertex_shader = ShaderHandle(1);
    desc.fragment_shader = ShaderHandle(2);
    return desc;
}

void test_descriptors_and_limits()
{
    check(validate(BufferDesc{1024, BufferUsage::vertex, true}), "valid buffer rejected");
    check(!validate(BufferDesc{}), "zero buffer accepted");
    TextureDesc texture;
    texture.width = 64;
    texture.height = 64;
    check(validate(texture), "valid 2D texture rejected");
    TextureDesc cube = texture;
    cube.dimension = TextureDimension::cube;
    cube.depth_or_layers = 5;
    check(!validate(cube), "five-layer cube accepted");
    cube.depth_or_layers = 6;
    check(validate(cube), "six-layer cube rejected");
    check(requires_bc_fallback(TextureFormat::bc1, false), "BC fallback not requested");
    check(!requires_bc_fallback(TextureFormat::bc3, true), "supported BC format requested fallback");

    ShaderDesc shader{ShaderStage::vertex, "terrain", 4, 16};
    check(validate(shader), "limit-sized shader rejected");
    shader.uniform_buffers = 5;
    check(!validate(shader) && validate(shader).error.find("limit 4") != std::string::npos, "fifth uniform accepted");

    UploadDesc upload{BufferHandle(1), 32, 16, 16};
    check(validate(upload), "in-range upload rejected");
    upload.size = 17;
    check(!validate(upload), "overflowing upload accepted");
}

void test_pipeline_key()
{
    const auto desc = valid_pipeline();
    const PipelineKey first(desc);
    const PipelineKey same(desc);
    check(first == same && first.stable_hash() == same.stable_hash(), "equal pipelines differ");
    auto changed = desc;
    changed.fog_enabled = true;
    const PipelineKey other(changed);
    check(first != other && first.stable_hash() != other.stable_hash(), "pipeline feature missing from key");
    std::unordered_set<PipelineKey> keys{first, same, other};
    check(keys.size() == 2, "pipeline hash does not support bounded cache identity");

    auto points = desc;
    points.topology = PrimitiveTopology::point_list;
    check(!validate(points), "point pipeline without point-size behavior accepted");
    points.uses_point_size = true;
    check(validate(points), "point pipeline with explicit point-size behavior rejected");
}

void test_pass_draw_and_bindings()
{
    RenderPassDesc pass;
    pass.color_targets[0] = TextureHandle(1);
    pass.color_target_count = 1;
    pass.depth_target = TextureHandle(2);
    pass.width = 1280;
    pass.height = 720;
    check(validate(pass), "valid render pass rejected");
    pass.color_target_count = 5;
    check(!validate(pass), "excess render targets accepted");

    const auto pipeline = valid_pipeline();
    DrawDesc draw;
    draw.pipeline = PipelineHandle(1);
    draw.vertex_buffer = BufferHandle(2);
    draw.vertex_or_index_count = 3;
    draw.vertex_bindings.uniform_count = 4;
    for (UInt32 i = 0; i < 4; ++i) draw.vertex_bindings.uniforms[i] = {BufferHandle(i + 3), 0, 64};
    check(validate(draw, pipeline), "valid four-uniform draw rejected");
    draw.vertex_bindings.uniform_count = 5;
    check(!validate(draw, pipeline), "fifth draw uniform accepted");

    auto points = pipeline;
    points.topology = PrimitiveTopology::point_list;
    points.uses_point_size = true;
    draw.vertex_bindings.uniform_count = 0;
    check(!validate(draw, points), "point draw without positive point size accepted");
    draw.point_size = 2.0F;
    check(validate(draw, points), "valid point draw rejected");
}

void test_conventions_and_resize()
{
    check(RendererConventions::depth_minimum == 0.0F && RendererConventions::depth_maximum == 1.0F, "depth convention changed");
    check(RendererConventions::front_face == Winding::clockwise, "winding convention changed");
    check(RendererConventions::texture_origin == TextureOrigin::top_left, "texture origin changed");
    check(RendererConventions::ui_half_pixel_offset == -0.5F, "UI half-pixel convention changed");
    check(pack_argb8(0x12, 0x34, 0x56, 0x78) == 0x12345678U, "ARGB packing changed");

    ResizeState resize(800, 600);
    check(resize.phase() == ResizePhase::stable && resize.generation() == 1, "initial resize state");
    check(!resize.complete_recreation(true), "out-of-order resize completion accepted");
    check(resize.request(0, 0) && resize.phase() == ResizePhase::suspended, "zero extent did not suspend");
    check(!resize.begin_recreation(), "suspended resize began recreation");
    check(resize.request(1920, 1080), "valid resize request rejected");
    check(resize.begin_recreation(), "resize recreation did not begin");
    check(!resize.request(1024, 768), "request during recreation accepted");
    check(resize.complete_recreation(true), "resize recreation did not complete");
    check(resize.width() == 1920 && resize.height() == 1080 && resize.generation() == 2, "resize result incorrect");
}

} // namespace

int main()
{
    try {
        test_descriptors_and_limits();
        test_pipeline_key();
        test_pass_draw_and_bindings();
        test_conventions_and_resize();
        std::cout << "renderer contract tests: ok\n";
        return 0;
    } catch (const std::exception& error) {
        std::cerr << error.what() << '\n';
        return 1;
    }
}
