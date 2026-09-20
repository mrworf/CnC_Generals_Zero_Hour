#include "zh/renderer/dx8_state_cache.h"

#include <iostream>
#include <stdexcept>
#include <string>

using namespace zh::renderer;

namespace {
void check(bool condition, const char* message) { if (!condition) throw std::runtime_error(message); }

struct Fixture {
    RecordingGpuDevice device;
    Dx8StateCache cache;
    ShaderHandle vs, fs;
    BufferHandle vertices, uniform;
    TextureHandle color, depth;

    explicit Fixture(std::size_t pipelines = 4) : device(pipelines), cache(device, 800, 600)
    {
        vs = device.create_shader({ShaderStage::vertex, "legacy.vert", 1, 0}, "legacy vertex");
        fs = device.create_shader({ShaderStage::fragment, "legacy.frag", 0, 0}, "legacy fragment");
        vertices = device.create_buffer({64, BufferUsage::vertex, true}, "stream vertices");
        uniform = device.create_buffer({64, BufferUsage::uniform, true}, "frame uniform");
        TextureDesc target; target.width = 32; target.height = 32; target.render_target = true;
        color = device.create_texture(target, "facade color");
        target.format = TextureFormat::depth24_stencil8;
        depth = device.create_texture(target, "facade depth");
        cache.set_shaders(vs, fs); cache.set_vertex_buffer(vertices);
        check(cache.set_uniform(ShaderStage::vertex, 0, {uniform, 0, 64}), "uniform setup failed");
    }

    RenderPassDesc pass() const
    {
        RenderPassDesc value; value.color_targets[0] = color; value.color_target_count = 1; value.depth_target = depth;
        value.width = 32; value.height = 32; return value;
    }
};

void test_cached_draw_order()
{
    Fixture fixture;
    check(fixture.cache.begin_pass(fixture.pass(), "legacy pass"), "begin pass failed");
    check(fixture.cache.draw(3), "first cached draw failed");
    check(fixture.cache.draw(6), "second cached draw failed");
    check(fixture.cache.end_pass(), "end pass failed");
    check(fixture.device.pipeline_count() == 1, "unchanged state created redundant pipelines");
    const auto before = fixture.device.snapshot();
    check(before.find("draw pipeline=P1") < before.rfind("draw pipeline=P1"), "draw ordering not retained");

    auto changed = PipelineDesc{}; changed.vertex_shader = fixture.vs; changed.fragment_shader = fixture.fs; changed.fog_enabled = true;
    fixture.cache.set_pipeline_state(changed);
    check(fixture.cache.begin_pass(fixture.pass(), "fog pass") && fixture.cache.draw(3) && fixture.cache.end_pass(), "changed state draw failed");
    check(fixture.device.pipeline_count() == 2, "changed state did not create immutable pipeline");
}

void test_facade_errors_and_destroyed_resources()
{
    Fixture fixture(1);
    check(!fixture.cache.draw(3) && fixture.cache.last_error().find("active pass") != std::string::npos, "draw without pass accepted");
    check(!fixture.cache.end_pass(), "end without pass accepted");
    check(!fixture.cache.set_uniform(ShaderStage::vertex, 4, {fixture.uniform, 0, 4}), "excess uniform slot accepted");
    check(fixture.cache.begin_pass(fixture.pass(), "failure pass"), "failure pass did not begin");
    fixture.device.destroy(fixture.vertices);
    check(!fixture.cache.draw(3) && fixture.cache.last_error().find("destroyed") != std::string::npos,
        "destroyed vertex buffer accepted or provenance lost");
    check(fixture.cache.end_pass(), "pass did not recover after rejected draw");

    fixture.cache.set_vertex_buffer(fixture.device.create_buffer({64, BufferUsage::vertex, true}, "replacement vertices"));
    check(fixture.cache.begin_pass(fixture.pass(), "overflow pass"), "overflow pass did not begin");
    check(fixture.cache.draw(3), "initial bounded pipeline failed");
    PipelineDesc changed; changed.vertex_shader = fixture.vs; changed.fragment_shader = fixture.fs; changed.fog_enabled = true;
    fixture.cache.set_pipeline_state(changed);
    check(!fixture.cache.draw(3) && fixture.cache.last_error().find("capacity exceeded") != std::string::npos,
        "pipeline cache explosion was accepted");
    check(fixture.cache.end_pass(), "overflow pass did not end");
}

void test_resize_command_generation()
{
    Fixture fixture;
    check(fixture.cache.request_resize(0, 0), "zero resize request rejected");
    check(fixture.cache.resize_state().phase() == ResizePhase::suspended, "zero resize did not suspend");
    check(!fixture.cache.begin_resize_recreation(), "suspended recreation accepted");
    check(fixture.cache.request_resize(1024, 768), "resize request failed");
    check(fixture.cache.begin_resize_recreation(), "resize begin failed");
    check(!fixture.cache.request_resize(640, 480), "resize request during recreation accepted");
    check(!fixture.cache.complete_resize_recreation(false), "failed recreation reported success");
    check(fixture.cache.resize_state().phase() == ResizePhase::requested, "failed recreation did not remain pending");
    check(fixture.cache.begin_resize_recreation() && fixture.cache.complete_resize_recreation(true), "resize retry did not recover");
    check(fixture.cache.resize_state().generation() == 2 && fixture.cache.resize_state().width() == 1024, "resize result is wrong");
    const auto snapshot = fixture.device.snapshot();
    const auto requested = snapshot.find("resize requested extent=1024x768");
    const auto recreating = snapshot.find("resize recreating", requested);
    const auto failed = snapshot.find("resize recreation failed pending", recreating);
    const auto stable = snapshot.find("resize stable extent=1024x768 generation=2", failed);
    check(requested < recreating && recreating < failed && failed < stable, "resize command ordering was not retained");
}
} // namespace

int main()
{
    try {
        test_cached_draw_order();
        test_facade_errors_and_destroyed_resources();
        test_resize_command_generation();
        std::cout << "DX8 state cache tests: ok\n";
        return 0;
    } catch (const std::exception& error) {
        std::cerr << error.what() << '\n';
        return 1;
    }
}
