#pragma once

#include "zh/renderer/recording_device.h"
#include "zh/world/scene.h"

#include <array>
#include <string>
#include <vector>

namespace zh::world {

class WorldRecorder {
public:
    explicit WorldRecorder(renderer::GpuDevice& device);

    renderer::ValidationResult load(WorldScene scene);
    renderer::ValidationResult record_frame(UInt32 tick);
    renderer::ValidationResult teardown();

    bool loaded() const noexcept { return loaded_; }
    const std::string& last_error() const noexcept { return last_error_; }
    std::size_t owned_pipeline_count() const noexcept { return pipelines_.size(); }

private:
    renderer::ValidationResult fail(std::string message);
    void destroy_resources();
    renderer::ValidationResult create_resources();
    renderer::ValidationResult draw_item(const WorldItem& item, UInt32 tick);

    renderer::GpuDevice& device_;
    WorldScene scene_;
    bool loaded_ = false;
    bool frame_recorded_ = false;
    UInt32 last_tick_ = 0;
    std::string last_error_;

    renderer::ShaderHandle vertex_shader_;
    renderer::ShaderHandle fragment_shader_;
    renderer::BufferHandle vertex_stream_;
    renderer::BufferHandle index_stream_;
    renderer::UInt64 vertex_stream_size_ = 0;
    renderer::UInt64 index_stream_size_ = 0;
    renderer::BufferHandle frame_uniform_;
    renderer::BufferHandle object_uniform_;
    renderer::BufferHandle material_uniform_;
    std::array<renderer::TextureHandle, 3> base_textures_{};
    renderer::TextureHandle main_color_;
    renderer::TextureHandle main_depth_;
    renderer::TextureHandle shadow_color_;
    renderer::TextureHandle shadow_depth_;
    std::array<renderer::SamplerHandle, 4> samplers_{};
    std::vector<renderer::PipelineHandle> pipelines_;
};

} // namespace zh::world
