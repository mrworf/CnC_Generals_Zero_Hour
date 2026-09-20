#pragma once

#include "zh/renderer/recording_device.h"

#include <string>

namespace zh::renderer {

// Transitional engine-facing facade for legacy DX8Wrapper call sites. It
// translates cached state into immutable backend-neutral pipeline descriptors;
// it intentionally exposes no platform graphics object.
class Dx8StateCache {
public:
    Dx8StateCache(GpuDevice& device, UInt32 width, UInt32 height);

    void set_shaders(ShaderHandle vertex, ShaderHandle fragment);
    void set_vertex_buffer(BufferHandle buffer);
    void set_index_buffer(BufferHandle buffer);
    void set_pipeline_state(const PipelineDesc& state);
    ValidationResult set_uniform(ShaderStage stage, UInt32 slot, UniformBinding binding);
    ValidationResult set_texture(ShaderStage stage, UInt32 slot, TextureHandle texture, SamplerHandle sampler);

    ValidationResult begin_pass(const RenderPassDesc& pass, std::string_view label);
    ValidationResult draw(UInt32 vertex_or_index_count, float point_size = 0.0F);
    ValidationResult end_pass();

    ValidationResult request_resize(UInt32 width, UInt32 height);
    ValidationResult begin_resize_recreation();
    ValidationResult complete_resize_recreation(bool succeeded);

    const ResizeState& resize_state() const noexcept { return resize_; }
    const std::string& last_error() const noexcept { return last_error_; }

private:
    ValidationResult fail(std::string message);
    StageBindings& bindings(ShaderStage stage);

    GpuDevice& device_;
    ResizeState resize_;
    PipelineDesc pipeline_state_;
    PipelineHandle pipeline_;
    BufferHandle vertex_buffer_;
    BufferHandle index_buffer_;
    StageBindings vertex_bindings_;
    StageBindings fragment_bindings_;
    bool pipeline_dirty_ = true;
    bool pass_active_ = false;
    std::string last_error_;
};

} // namespace zh::renderer
