#include "zh/renderer/dx8_state_cache.h"

#include <algorithm>
#include <string>

namespace zh::renderer {

Dx8StateCache::Dx8StateCache(GpuDevice& device, UInt32 width, UInt32 height)
    : device_(device), resize_(width, height)
{
}

ValidationResult Dx8StateCache::fail(std::string message)
{
    last_error_ = "DX8Wrapper: " + std::move(message);
    return {false, last_error_};
}

StageBindings& Dx8StateCache::bindings(ShaderStage stage)
{
    return stage == ShaderStage::vertex ? vertex_bindings_ : fragment_bindings_;
}

void Dx8StateCache::set_shaders(ShaderHandle vertex, ShaderHandle fragment)
{
    if (pipeline_state_.vertex_shader != vertex || pipeline_state_.fragment_shader != fragment) {
        pipeline_state_.vertex_shader = vertex;
        pipeline_state_.fragment_shader = fragment;
        pipeline_dirty_ = true;
    }
}

void Dx8StateCache::set_vertex_buffer(BufferHandle buffer) { vertex_buffer_ = buffer; }
void Dx8StateCache::set_index_buffer(BufferHandle buffer) { index_buffer_ = buffer; }

void Dx8StateCache::set_pipeline_state(const PipelineDesc& state)
{
    if (PipelineKey(pipeline_state_) != PipelineKey(state)) {
        pipeline_state_ = state;
        pipeline_dirty_ = true;
    }
}

ValidationResult Dx8StateCache::set_uniform(ShaderStage stage, UInt32 slot, UniformBinding binding)
{
    if (slot >= RendererLimits::uniform_buffers_per_stage) return fail("uniform slot exceeds limit 4");
    if (!binding.buffer || binding.size == 0) return fail("uniform binding is empty");
    auto& state = bindings(stage);
    state.uniforms[slot] = binding;
    state.uniform_count = std::max(state.uniform_count, slot + 1U);
    return {};
}

ValidationResult Dx8StateCache::set_texture(ShaderStage stage, UInt32 slot, TextureHandle texture, SamplerHandle sampler)
{
    if (slot >= RendererLimits::sampled_textures_per_stage) return fail("texture slot exceeds limit 16");
    if (!texture || !sampler) return fail("texture binding requires texture and sampler");
    auto& state = bindings(stage);
    state.textures[slot] = texture;
    state.samplers[slot] = sampler;
    state.texture_count = std::max(state.texture_count, slot + 1U);
    return {};
}

ValidationResult Dx8StateCache::begin_pass(const RenderPassDesc& pass, std::string_view label)
{
    if (pass_active_) return fail("begin_pass called while a pass is active");
    auto result = device_.begin_pass(pass, label);
    if (!result) return fail(result.error);
    pass_active_ = true;
    return {};
}

ValidationResult Dx8StateCache::draw(UInt32 vertex_or_index_count, float point_size)
{
    if (!pass_active_) return fail("draw requires an active pass");
    if (!pipeline_state_.vertex_shader || !pipeline_state_.fragment_shader) return fail("draw requires vertex and fragment shaders");
    if (!vertex_buffer_) return fail("draw requires a vertex buffer");
    if (pipeline_dirty_ || !pipeline_) {
        pipeline_ = device_.create_pipeline(PipelineKey(pipeline_state_), "DX8Wrapper cached pipeline");
        if (!pipeline_) return fail(device_.last_error());
        pipeline_dirty_ = false;
    }
    DrawDesc draw;
    draw.pipeline = pipeline_;
    draw.vertex_buffer = vertex_buffer_;
    draw.index_buffer = index_buffer_;
    draw.vertex_or_index_count = vertex_or_index_count;
    draw.point_size = point_size;
    draw.vertex_bindings = vertex_bindings_;
    draw.fragment_bindings = fragment_bindings_;
    auto result = device_.draw(draw);
    if (!result) return fail(result.error);
    return {};
}

ValidationResult Dx8StateCache::end_pass()
{
    if (!pass_active_) return fail("end_pass called without an active pass");
    auto result = device_.end_pass();
    if (!result) return fail(result.error);
    pass_active_ = false;
    return {};
}

ValidationResult Dx8StateCache::request_resize(UInt32 width, UInt32 height)
{
    auto result = resize_.request(width, height);
    if (!result) return fail(result.error);
    device_.record_marker(width == 0 || height == 0
        ? "resize suspended extent=" + std::to_string(width) + "x" + std::to_string(height)
        : "resize requested extent=" + std::to_string(width) + "x" + std::to_string(height));
    return {};
}

ValidationResult Dx8StateCache::begin_resize_recreation()
{
    auto result = resize_.begin_recreation();
    if (!result) return fail(result.error);
    device_.record_marker("resize recreating");
    return {};
}

ValidationResult Dx8StateCache::complete_resize_recreation(bool succeeded)
{
    auto result = resize_.complete_recreation(succeeded);
    if (!succeeded) {
        device_.record_marker("resize recreation failed pending");
        return fail(result.error);
    }
    if (!result) return fail(result.error);
    device_.record_marker("resize stable extent=" + std::to_string(resize_.width()) + "x" + std::to_string(resize_.height())
        + " generation=" + std::to_string(resize_.generation()));
    return {};
}

} // namespace zh::renderer
