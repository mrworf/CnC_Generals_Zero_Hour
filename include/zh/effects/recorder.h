#pragma once

#include "zh/effects/registry.h"
#include "zh/renderer/recording_device.h"

#include <array>
#include <string>
#include <vector>

namespace zh::effects {

struct EffectRequest {
    std::string logical_asset;
    std::string material;
    std::string legacy_effect;
    renderer::UInt32 vertex_count = 3;
    float point_size = 16.0F;
};

class EffectsRecorder {
public:
    explicit EffectsRecorder(renderer::GpuDevice& device);

    renderer::ValidationResult load(std::vector<EffectRequest> requests);
    renderer::ValidationResult record_frame(renderer::UInt32 tick);
    renderer::ValidationResult teardown();

    bool loaded() const noexcept { return loaded_; }
    const std::string& last_error() const noexcept { return last_error_; }
    std::size_t owned_pipeline_count() const noexcept { return pipelines_.size(); }

private:
    struct PreparedEffect {
        EffectRequest request;
        const EffectMapping* mapping = nullptr;
        renderer::ShaderHandle vertex_shader;
        renderer::ShaderHandle fragment_shader;
        std::size_t first_pipeline = 0;
    };

    renderer::ValidationResult fail(std::string message);
    renderer::ValidationResult create_resources();
    renderer::ValidationResult draw_effect(const PreparedEffect& effect, renderer::UInt32 tick);
    void destroy_resources();

    renderer::GpuDevice& device_;
    std::vector<PreparedEffect> effects_;
    std::vector<renderer::PipelineHandle> pipelines_;
    bool loaded_ = false;
    bool frame_recorded_ = false;
    renderer::UInt32 last_tick_ = 0;
    std::string last_error_;

    renderer::BufferHandle vertex_buffer_;
    renderer::UInt64 vertex_buffer_size_ = 0;
    std::array<renderer::BufferHandle, renderer::RendererLimits::uniform_buffers_per_stage> uniforms_{};
    std::array<renderer::TextureHandle, 4> textures_{};
    std::array<renderer::SamplerHandle, 4> samplers_{};
    renderer::TextureHandle source_color_;
    renderer::TextureHandle source_depth_;
    renderer::TextureHandle main_color_;
    renderer::TextureHandle main_depth_;
    renderer::TextureHandle output_color_;
    renderer::TextureHandle output_depth_;
};

} // namespace zh::effects
