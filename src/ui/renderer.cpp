#include "zh/ui/renderer.h"

#include <algorithm>
#include <array>
#include <cmath>
#include <iomanip>
#include <limits>
#include <sstream>

namespace zh::ui {
namespace {

constexpr std::size_t maximum_elements = 4096;

struct Vertex {
    float x;
    float y;
    std::uint32_t color;
    float u;
    float v;
};

bool finite_rect(const Rect& rect)
{
    return std::isfinite(rect.x) && std::isfinite(rect.y) && std::isfinite(rect.width)
        && std::isfinite(rect.height);
}

bool positive_rect(const Rect& rect)
{
    return finite_rect(rect) && rect.width > 0.0F && rect.height > 0.0F;
}

Rect intersection(const Rect& left, const Rect& right)
{
    const float x1 = std::max(left.x, right.x);
    const float y1 = std::max(left.y, right.y);
    const float x2 = std::min(left.x + left.width, right.x + right.width);
    const float y2 = std::min(left.y + left.height, right.y + right.height);
    return {x1, y1, std::max(0.0F, x2 - x1), std::max(0.0F, y2 - y1)};
}

std::string number(float value)
{
    std::ostringstream stream;
    stream << std::fixed << std::setprecision(2) << value;
    return stream.str();
}

std::string transition_name(ScreenTransition transition)
{
    switch (transition) {
    case ScreenTransition::none: return "none";
    case ScreenTransition::menu_enter: return "menu-enter";
    case ScreenTransition::menu_leave: return "menu-leave";
    case ScreenTransition::loading_begin: return "loading-begin";
    case ScreenTransition::loading_complete: return "loading-complete";
    }
    return "unknown";
}

std::string layer_name(Layer layer)
{
    switch (layer) {
    case Layer::background: return "background";
    case Layer::content: return "content";
    case Layer::overlay: return "overlay";
    case Layer::cursor: return "cursor";
    }
    return "unknown";
}

std::string blend_name(BlendMode blend)
{
    switch (blend) {
    case BlendMode::opaque: return "opaque";
    case BlendMode::alpha: return "alpha";
    case BlendMode::additive: return "additive";
    }
    return "unknown";
}

} // namespace

UiRecorder::UiRecorder(renderer::RecordingGpuDevice& device) : device_(device) {}

UiRecorder::~UiRecorder()
{
    if (color_target_) device_.destroy(color_target_);
    if (depth_target_) device_.destroy(depth_target_);
    if (vertex_buffer_) device_.destroy(vertex_buffer_);
    if (opaque_pipeline_) device_.destroy(opaque_pipeline_);
    if (alpha_pipeline_) device_.destroy(alpha_pipeline_);
    if (additive_pipeline_) device_.destroy(additive_pipeline_);
    if (vertex_shader_) device_.destroy(vertex_shader_);
    if (fragment_shader_) device_.destroy(fragment_shader_);
}

renderer::ValidationResult UiRecorder::fail(std::string message)
{
    last_error_ = "ui recorder: " + std::move(message);
    return {false, last_error_};
}

bool UiRecorder::ensure_resources()
{
    if (vertex_buffer_) return true;
    vertex_shader_ = device_.create_shader({renderer::ShaderStage::vertex, "ui.vert", 0, 0}, "ui vertex");
    fragment_shader_ = device_.create_shader({renderer::ShaderStage::fragment, "ui.frag", 0, 0}, "ui fragment");
    vertex_buffer_ = device_.create_buffer({sizeof(Vertex) * 6, renderer::BufferUsage::vertex, true}, "ui quad vertices");
    if (!vertex_shader_ || !fragment_shader_ || !vertex_buffer_) return false;

    renderer::PipelineDesc desc;
    desc.vertex_shader = vertex_shader_;
    desc.fragment_shader = fragment_shader_;
    desc.vertex_layout = renderer::VertexLayout::position_color_uv;
    desc.depth_stencil.depth_test = false;
    desc.depth_stencil.depth_write = false;
    desc.raster.cull = renderer::CullMode::none;
    opaque_pipeline_ = device_.create_pipeline(renderer::PipelineKey(desc), "ui opaque");
    desc.blend.enabled = true;
    desc.blend.source_color = renderer::BlendFactor::one;
    desc.blend.destination_color = renderer::BlendFactor::inv_src_alpha;
    desc.blend.source_alpha = renderer::BlendFactor::one;
    desc.blend.destination_alpha = renderer::BlendFactor::inv_src_alpha;
    desc.premultiplied_alpha = true;
    alpha_pipeline_ = device_.create_pipeline(renderer::PipelineKey(desc), "ui premultiplied alpha");
    desc.blend.destination_color = renderer::BlendFactor::one;
    desc.blend.destination_alpha = renderer::BlendFactor::one;
    additive_pipeline_ = device_.create_pipeline(renderer::PipelineKey(desc), "ui additive");
    return opaque_pipeline_ && alpha_pipeline_ && additive_pipeline_;
}

bool UiRecorder::recreate_targets(int width, int height)
{
    if (width_ == width && height_ == height && color_target_ && depth_target_) return true;
    if (color_target_) device_.destroy(color_target_);
    if (depth_target_) device_.destroy(depth_target_);
    device_.record_marker("ui-resize " + std::to_string(width_) + "x" + std::to_string(height_)
        + " -> " + std::to_string(width) + "x" + std::to_string(height));
    renderer::TextureDesc color;
    color.width = static_cast<renderer::UInt32>(width);
    color.height = static_cast<renderer::UInt32>(height);
    color.render_target = true;
    color_target_ = device_.create_texture(color, "ui color target");
    auto depth = color;
    depth.format = renderer::TextureFormat::depth24_stencil8;
    depth.sampled = false;
    depth_target_ = device_.create_texture(depth, "ui depth target");
    if (!color_target_ || !depth_target_) return false;
    width_ = width;
    height_ = height;
    return true;
}

renderer::ValidationResult UiRecorder::record(const Scene& scene)
{
    last_error_.clear();
    if (scene.width <= 0 || scene.height <= 0 || scene.width > 16384 || scene.height > 16384)
        return fail("viewport width/height must be within 1..16384");
    if (scene.elements.size() > maximum_elements) return fail("element count exceeds 4096");
    for (const auto& element : scene.elements) {
        if (element.label.empty()) return fail("element label must not be empty");
        if (!positive_rect(element.bounds)) return fail("element '" + element.label + "' has invalid bounds");
        if (!positive_rect(element.clip)) return fail("element '" + element.label + "' has invalid clip");
    }
    if (!ensure_resources()) return fail("could not create UI resources: " + device_.last_error());
    if (!recreate_targets(scene.width, scene.height)) return fail("could not create UI targets: " + device_.last_error());

    std::vector<const UiElement*> ordered;
    ordered.reserve(scene.elements.size());
    for (const auto& element : scene.elements) ordered.push_back(&element);
    std::stable_sort(ordered.begin(), ordered.end(), [](const auto* left, const auto* right) {
        return static_cast<unsigned>(left->layer) < static_cast<unsigned>(right->layer);
    });

    device_.record_marker("ui-transition " + transition_name(scene.transition));
    renderer::RenderPassDesc pass;
    pass.color_targets[0] = color_target_;
    pass.color_target_count = 1;
    pass.depth_target = depth_target_;
    pass.width = static_cast<renderer::UInt32>(scene.width);
    pass.height = static_cast<renderer::UInt32>(scene.height);
    if (auto result = device_.begin_pass(pass, "ui pass"); !result) return fail(result.error);

    for (const auto* element : ordered) {
        const auto clipped = intersection(element->bounds, element->clip);
        if (clipped.width == 0.0F || clipped.height == 0.0F) {
            device_.record_marker("ui-clipped-out label=" + element->label);
            continue;
        }
        const float left = clipped.x + renderer::RendererConventions::ui_half_pixel_offset;
        const float top = clipped.y + renderer::RendererConventions::ui_half_pixel_offset;
        const float right = left + clipped.width;
        const float bottom = top + clipped.height;
        const float u0 = (clipped.x - element->bounds.x) / element->bounds.width;
        const float v0 = (clipped.y - element->bounds.y) / element->bounds.height;
        const float u1 = (clipped.x + clipped.width - element->bounds.x) / element->bounds.width;
        const float v1 = (clipped.y + clipped.height - element->bounds.y) / element->bounds.height;
        const std::array<Vertex, 6> vertices{{
            {left, top, element->argb, u0, v0}, {right, top, element->argb, u1, v0},
            {right, bottom, element->argb, u1, v1}, {left, top, element->argb, u0, v0},
            {right, bottom, element->argb, u1, v1}, {left, bottom, element->argb, u0, v1},
        }};
        const renderer::UploadDesc upload{vertex_buffer_, sizeof(vertices), 0, sizeof(vertices)};
        if (auto result = device_.upload(upload, vertices.data()); !result) {
            (void)device_.end_pass();
            return fail(result.error);
        }
        device_.record_marker("ui-element label=" + element->label + " layer=" + layer_name(element->layer)
            + " blend=" + blend_name(element->blend) + " clip=" + number(clipped.x) + ","
            + number(clipped.y) + "," + number(clipped.width) + "," + number(clipped.height)
            + " vertex=" + number(left) + "," + number(top));
        renderer::DrawDesc draw;
        draw.pipeline = element->blend == BlendMode::opaque ? opaque_pipeline_
            : element->blend == BlendMode::additive ? additive_pipeline_ : alpha_pipeline_;
        draw.vertex_buffer = vertex_buffer_;
        draw.vertex_or_index_count = 6;
        if (auto result = device_.draw(draw); !result) {
            (void)device_.end_pass();
            return fail(result.error);
        }
    }
    if (auto result = device_.end_pass(); !result) return fail(result.error);
    return {};
}

InternetActionResult UiRecorder::select_internet_action() const
{
    return {"Internet services are unavailable in this build", false, false};
}

} // namespace zh::ui
