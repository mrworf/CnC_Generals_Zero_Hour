#pragma once

#include "zh/renderer/recording_device.h"

#include <cstdint>
#include <string>
#include <string_view>
#include <vector>

namespace zh::ui {

struct Rect {
    float x = 0.0F;
    float y = 0.0F;
    float width = 0.0F;
    float height = 0.0F;
};

enum class BlendMode : std::uint8_t { opaque, alpha, additive };
enum class Layer : std::uint8_t { background, content, overlay, cursor };
enum class ScreenTransition : std::uint8_t { none, menu_enter, menu_leave, loading_begin, loading_complete };

struct UiElement {
    std::string label;
    Rect bounds;
    Rect clip;
    std::uint32_t argb = 0xffffffffU;
    BlendMode blend = BlendMode::alpha;
    Layer layer = Layer::content;
};

struct Scene {
    int width = 0;
    int height = 0;
    ScreenTransition transition = ScreenTransition::none;
    std::vector<UiElement> elements;
};

struct InternetActionResult {
    std::string explanation;
    bool network_attempted = false;
    bool waiting = false;
};

class UiRecorder {
public:
    explicit UiRecorder(renderer::GpuDevice& device);
    ~UiRecorder();

    UiRecorder(const UiRecorder&) = delete;
    UiRecorder& operator=(const UiRecorder&) = delete;

    renderer::ValidationResult record(const Scene& scene);
    const std::string& last_error() const noexcept { return last_error_; }
    InternetActionResult select_internet_action() const;

private:
    renderer::ValidationResult fail(std::string message);
    bool recreate_targets(int width, int height);
    bool ensure_resources();

    renderer::GpuDevice& device_;
    int width_ = 0;
    int height_ = 0;
    renderer::TextureHandle color_target_;
    renderer::TextureHandle depth_target_;
    renderer::BufferHandle vertex_buffer_;
    renderer::BufferHandle frame_uniform_;
    renderer::BufferHandle material_uniform_;
    renderer::TextureHandle white_texture_;
    renderer::SamplerHandle sampler_;
    renderer::ShaderHandle vertex_shader_;
    renderer::ShaderHandle fragment_shader_;
    renderer::PipelineHandle opaque_pipeline_;
    renderer::PipelineHandle alpha_pipeline_;
    renderer::PipelineHandle additive_pipeline_;
    std::string last_error_;
};

} // namespace zh::ui
