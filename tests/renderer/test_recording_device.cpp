#include "zh/renderer/recording_device.h"

#include <array>
#include <iostream>
#include <stdexcept>
#include <string>

using namespace zh::renderer;

namespace {
void check(bool condition, const char* message) { if (!condition) throw std::runtime_error(message); }

struct Scene {
    RecordingGpuDevice device{2};
    ShaderHandle vertex;
    ShaderHandle fragment;
    PipelineHandle pipeline;
    BufferHandle vertices;
    TextureHandle color;
    TextureHandle depth;

    Scene()
    {
        vertex = device.create_shader({ShaderStage::vertex, "scene.vert", 1, 0}, "scene vertex");
        fragment = device.create_shader({ShaderStage::fragment, "scene.frag", 0, 0}, "scene fragment");
        PipelineDesc pipeline_desc;
        pipeline_desc.vertex_shader = vertex;
        pipeline_desc.fragment_shader = fragment;
        pipeline = device.create_pipeline(PipelineKey(pipeline_desc), "scene pipeline");
        vertices = device.create_buffer({12, BufferUsage::vertex, true}, "vertices");
        TextureDesc color_desc;
        color_desc.width = 16; color_desc.height = 16; color_desc.render_target = true;
        color = device.create_texture(color_desc, "color target");
        TextureDesc depth_desc = color_desc;
        depth_desc.format = TextureFormat::depth24_stencil8;
        depth = device.create_texture(depth_desc, "depth target");
    }

    RenderPassDesc pass() const
    {
        RenderPassDesc pass;
        pass.color_targets[0] = color; pass.color_target_count = 1; pass.depth_target = depth;
        pass.width = 16; pass.height = 16;
        return pass;
    }
};

void test_valid_stream_and_snapshot()
{
    Scene scene;
    const std::array<unsigned char, 4> bytes{1, 2, 3, 4};
    check(scene.device.upload({scene.vertices, 12, 4, bytes.size()}, bytes.data()), "valid upload failed");
    check(scene.device.buffer_bytes(scene.vertices)[6] == 3, "upload did not update CPU copy");
    const std::array<unsigned char, 16 * 16 * 4> pixels{};
    check(scene.device.upload_texture({scene.color, 16, 16, 64, pixels.size()}, pixels.data()),
        "valid RGBA texture upload failed");
    check(scene.device.begin_pass(scene.pass(), "main pass"), "valid pass failed");
    DrawDesc draw;
    draw.pipeline = scene.pipeline; draw.vertex_buffer = scene.vertices; draw.vertex_or_index_count = 3;
    auto uniforms = scene.device.create_buffer({64, BufferUsage::uniform, true}, "frame constants");
    draw.vertex_bindings.uniforms[0] = {uniforms, 0, 64}; draw.vertex_bindings.uniform_count = 1;
    check(scene.device.draw(draw), "valid draw failed");
    check(scene.device.end_pass(), "valid pass end failed");
    check(scene.device.present(scene.color), "valid presentation failed");
    const auto snapshot = scene.device.snapshot();
    check(snapshot.find("create_buffer B1 label=\"vertices\"") != std::string::npos, "buffer id was not normalized");
    check(snapshot.find("begin_pass") < snapshot.find("draw pipeline="), "draw order not retained");
    check(snapshot.find("draw pipeline=") < snapshot.find("end_pass"), "pass order not retained");
    check(snapshot.find("upload_texture") != std::string::npos && snapshot.find("present T") != std::string::npos,
        "texture upload or presentation was not recorded");
}

void test_lifetimes_bindings_and_ranges()
{
    Scene scene;
    const std::array<unsigned char, 4> bytes{};
    check(!scene.device.upload({scene.vertices, 11, 0, 4}, bytes.data()), "false destination size accepted");
    check(scene.device.last_error().find("vertices") != std::string::npos, "upload error lost label provenance");
    check(!scene.device.upload({scene.vertices, 12, 0, 4}, nullptr), "null upload accepted");
    const std::array<unsigned char, 16 * 16 * 4> pixels{};
    check(!scene.device.upload_texture({scene.color, 16, 16, 63, pixels.size()}, pixels.data()),
        "undersized texture row pitch accepted");
    scene.device.destroy(scene.vertices);
    check(!scene.device.upload({scene.vertices, 12, 0, 4}, bytes.data()), "destroyed buffer upload accepted");
    scene.device.destroy(scene.vertices);
    auto replacement = scene.device.create_buffer({12, BufferUsage::vertex, true}, "replacement");
    check(replacement && replacement != scene.vertices, "slot reuse did not change generation");

    check(scene.device.begin_pass(scene.pass(), "binding pass"), "pass failed");
    DrawDesc draw; draw.pipeline = scene.pipeline; draw.vertex_buffer = replacement; draw.vertex_or_index_count = 3;
    check(!scene.device.draw(draw), "missing required uniform accepted");
    check(scene.device.last_error().find("binding pass") != std::string::npos, "draw error lost pass provenance");
    auto uniform = scene.device.create_buffer({32, BufferUsage::uniform, true}, "uniform");
    draw.vertex_bindings.uniforms[0] = {uniform, 0, 32}; draw.vertex_bindings.uniform_count = 1;
    scene.device.destroy(uniform);
    check(!scene.device.draw(draw), "destroyed binding accepted");
    check(scene.device.end_pass(), "recovery end pass failed");
}

void test_pass_rules_and_unsupported_descriptors()
{
    Scene scene;
    check(!scene.device.create_buffer({}, "bad buffer"), "invalid descriptor accepted");
    check(scene.device.last_error().find("bad buffer") != std::string::npos, "create error lost label");
    check(!scene.device.end_pass(), "end without pass accepted");
    check(scene.device.begin_pass(scene.pass(), "outer"), "outer pass failed");
    check(!scene.device.begin_pass(scene.pass(), "nested"), "nested pass accepted");
    scene.device.destroy(scene.color);
    check(scene.device.last_error().find("attached to the active") != std::string::npos,
        "active render target destruction accepted");
    check(scene.device.end_pass(), "outer end failed");
    auto wrong = scene.pass(); wrong.width = 8;
    check(!scene.device.begin_pass(wrong, "wrong extent"), "attachment extent mismatch accepted");
    scene.device.destroy(scene.color);
    check(!scene.device.begin_pass(scene.pass(), "dead target"), "destroyed target accepted");
}

void test_pipeline_cache_is_immutable_and_bounded()
{
    Scene scene;
    PipelineDesc first;
    first.vertex_shader = scene.vertex; first.fragment_shader = scene.fragment;
    auto reused = scene.device.create_pipeline(PipelineKey(first), "same state");
    check(reused == scene.pipeline && scene.device.pipeline_count() == 1, "equal pipeline was not reused");
    auto second = first; second.fog_enabled = true;
    check(static_cast<bool>(scene.device.create_pipeline(PipelineKey(second), "fog state")), "second unique pipeline rejected");
    auto third = first; third.blend.enabled = true;
    check(!scene.device.create_pipeline(PipelineKey(third), "overflow state"), "pipeline cache explosion accepted");
    check(scene.device.pipeline_count() == 2, "failed cache insert mutated cache");
    scene.device.destroy(scene.vertex);
    auto fourth = first; fourth.depth_stencil.depth_write = false;
    check(!scene.device.create_pipeline(PipelineKey(fourth), "stale shader"), "stale shader accepted");
}

void test_original_16_bit_index_range_and_base_vertex()
{
    Scene scene;
    auto index = scene.device.create_buffer({12, BufferUsage::index, true}, "original 16-bit indices");
    auto uniform = scene.device.create_buffer({64, BufferUsage::uniform, true}, "world constants");
    check(scene.device.begin_pass(scene.pass(), "source mesh"), "source pass failed");
    DrawDesc draw;
    draw.pipeline = scene.pipeline;
    draw.vertex_buffer = scene.vertices;
    draw.index_buffer = index;
    draw.index_element_size = IndexElementSize::uint16;
    draw.first_index = 2;
    draw.base_vertex = 4;
    draw.vertex_or_index_count = 3;
    draw.vertex_bindings.uniforms[0] = {uniform, 0, 64};
    draw.vertex_bindings.uniform_count = 1;
    check(scene.device.draw(draw), "valid original indexed range rejected");
    check(scene.device.snapshot().find("index_bits=16 first_index=2 base_vertex=4") != std::string::npos,
        "original index format or offsets lost from recording");
    draw.first_index = 4;
    check(!scene.device.draw(draw), "16-bit range beyond index buffer accepted");
    draw.first_index = 0;
    draw.index_element_size = static_cast<IndexElementSize>(3);
    check(!scene.device.draw(draw), "unsupported index element width accepted");
    draw.index_buffer = {};
    draw.index_element_size = IndexElementSize::uint32;
    draw.base_vertex = 1;
    check(!scene.device.draw(draw), "non-indexed base vertex accepted");
    check(scene.device.end_pass(), "source pass end failed");
}
} // namespace

int main()
{
    try {
        test_valid_stream_and_snapshot();
        test_lifetimes_bindings_and_ranges();
        test_pass_rules_and_unsupported_descriptors();
        test_pipeline_cache_is_immutable_and_bounded();
        test_original_16_bit_index_range_and_base_vertex();
        std::cout << "recording device tests: ok\n";
        return 0;
    } catch (const std::exception& error) {
        std::cerr << error.what() << '\n';
        return 1;
    }
}
