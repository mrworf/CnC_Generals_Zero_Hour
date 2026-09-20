#include "zh/world/recorder.h"

#include <algorithm>
#include <array>
#include <cstring>
#include <limits>
#include <set>
#include <string>
#include <tuple>

namespace zh::world {
namespace {

constexpr renderer::UInt64 vertex_stride = 32;
constexpr renderer::UInt64 index_stride = 4;

std::string state_marker(const WorldItem& item)
{
    const auto& material = item.material;
    return "world-state family=" + std::string(geometry_family_name(item.family))
        + " faction=" + std::string(faction_name(item.faction))
        + " transform=left-handed"
        + " lighting=" + (material.lighting.enabled ? "on" : "off")
        + " lights=" + std::to_string(material.lighting.directional_light_count)
        + " fog=" + (material.fog.enabled ? "view-space" : "off")
        + " alpha-test=" + (material.alpha_test.enabled ? "on" : "off")
        + " texture-stages=" + std::to_string(material.texture_stage_count)
        + " cull=" + std::to_string(static_cast<unsigned>(material.cull))
        + " depth-test=" + (material.depth_test ? "on" : "off")
        + " depth-write=" + (material.depth_write ? "on" : "off")
        + " color-mask=" + std::to_string(material.color_write_mask)
        + " animation-frame=" + std::to_string(item.animation_frame);
}

renderer::TextureDesc target_desc(UInt32 width, UInt32 height, renderer::TextureFormat format)
{
    renderer::TextureDesc desc;
    desc.width = width;
    desc.height = height;
    desc.format = format;
    desc.render_target = true;
    return desc;
}

} // namespace

WorldRecorder::WorldRecorder(renderer::GpuDevice& device) : device_(device) {}

renderer::ValidationResult WorldRecorder::fail(std::string message)
{
    last_error_ = "world recorder: " + std::move(message);
    device_.record_marker("world-error " + last_error_);
    return {false, last_error_};
}

renderer::ValidationResult WorldRecorder::load(WorldScene scene)
{
    if (loaded_) return fail("load requires an empty recorder; teardown the current map first");
    if (auto result = validate(scene); !result) return fail(result.error);
    std::sort(scene.items.begin(), scene.items.end(), [](const WorldItem& left, const WorldItem& right) {
        return std::tie(left.family, left.faction, left.label) < std::tie(right.family, right.faction, right.label);
    });
    scene_ = std::move(scene);
    if (auto result = create_resources(); !result) {
        destroy_resources();
        scene_ = {};
        return result;
    }
    loaded_ = true;
    frame_recorded_ = false;
    last_tick_ = 0;
    device_.record_marker("world-load map=" + scene_.map_name + " items=" + std::to_string(scene_.items.size()));
    device_.record_marker("world-camera handedness=left depth=0..1 winding=clockwise origin=top-left viewport="
        + std::to_string(scene_.camera.viewport_width) + "x" + std::to_string(scene_.camera.viewport_height));
    device_.record_marker("render-target dependency shadow-map -> main-world");
    return {};
}

renderer::ValidationResult WorldRecorder::create_resources()
{
    UInt32 maximum_vertices = 0;
    UInt32 maximum_indices = 0;
    for (const auto& item : scene_.items) {
        maximum_vertices = std::max(maximum_vertices, item.vertex_count);
        maximum_indices = std::max(maximum_indices, item.index_count);
    }
    vertex_shader_ = device_.create_shader({renderer::ShaderStage::vertex, "world.vert", 2, 0}, "world vertex transform");
    fragment_shader_ = device_.create_shader({renderer::ShaderStage::fragment, "world.frag", 1, 4}, "world fixed-function fragment");
    vertex_stream_size_ = vertex_stride * maximum_vertices;
    index_stream_size_ = index_stride * maximum_indices;
    vertex_stream_ = device_.create_buffer({vertex_stream_size_, renderer::BufferUsage::vertex, true}, "world vertex stream");
    index_stream_ = device_.create_buffer({index_stream_size_, renderer::BufferUsage::index, true}, "world index stream");
    frame_uniform_ = device_.create_buffer({256, renderer::BufferUsage::uniform, true}, "world frame camera/light/fog");
    object_uniform_ = device_.create_buffer({128, renderer::BufferUsage::uniform, true}, "world object transform/animation");
    material_uniform_ = device_.create_buffer({128, renderer::BufferUsage::uniform, true}, "world material/alpha stages");
    for (std::size_t index = 0; index < base_textures_.size(); ++index) {
        renderer::TextureDesc desc; desc.width = 1; desc.height = 1;
        base_textures_[index] = device_.create_texture(desc, "world texture stage " + std::to_string(index));
    }
    main_color_ = device_.create_texture(target_desc(scene_.camera.viewport_width, scene_.camera.viewport_height,
        renderer::TextureFormat::rgba8), "world main color");
    main_depth_ = device_.create_texture(target_desc(scene_.camera.viewport_width, scene_.camera.viewport_height,
        renderer::TextureFormat::depth24_stencil8), "world main depth");
    shadow_color_ = device_.create_texture(target_desc(256, 256, renderer::TextureFormat::rgba8), "world shadow map");
    shadow_depth_ = device_.create_texture(target_desc(256, 256, renderer::TextureFormat::depth24_stencil8), "world shadow depth");
    for (std::size_t index = 0; index < samplers_.size(); ++index) {
        renderer::SamplerDesc desc;
        desc.address_u = index == 3 ? renderer::AddressMode::clamp_border : renderer::AddressMode::repeat;
        desc.address_v = desc.address_u;
        samplers_[index] = device_.create_sampler(desc, "world sampler stage " + std::to_string(index));
    }
    if (!vertex_shader_ || !fragment_shader_ || !vertex_stream_ || !index_stream_ || !frame_uniform_
        || !object_uniform_ || !material_uniform_ || !main_color_ || !main_depth_ || !shadow_color_ || !shadow_depth_)
        return fail(device_.last_error());
    for (auto handle : base_textures_) if (!handle) return fail(device_.last_error());
    for (auto handle : samplers_) if (!handle) return fail(device_.last_error());

    for (const auto& item : scene_.items) {
        MaterialTranslation translated;
        if (auto result = translate_material(item.material, item.family, vertex_shader_, fragment_shader_, translated); !result)
            return fail(item.label + ": " + result.error);
        auto pipeline = device_.create_pipeline(renderer::PipelineKey(translated.pipeline), "world " + item.label);
        if (!pipeline) return fail(device_.last_error());
        if (std::find(pipelines_.begin(), pipelines_.end(), pipeline) == pipelines_.end()) pipelines_.push_back(pipeline);
    }
    return {};
}

renderer::ValidationResult WorldRecorder::record_frame(UInt32 tick)
{
    if (!loaded_) return fail("record_frame requires a loaded map");
    if (frame_recorded_ && tick < last_tick_) return fail("frame tick regressed from " + std::to_string(last_tick_) + " to " + std::to_string(tick));

    std::array<UInt32, 64> frame_data{};
    frame_data[0] = tick;
    frame_data[1] = scene_.camera.viewport_width;
    frame_data[2] = scene_.camera.viewport_height;
    if (auto result = device_.upload({frame_uniform_, sizeof(frame_data), 0, sizeof(frame_data)}, frame_data.data()); !result)
        return fail(result.error);

    renderer::RenderPassDesc shadow_pass;
    shadow_pass.color_targets[0] = shadow_color_;
    shadow_pass.color_target_count = 1;
    shadow_pass.depth_target = shadow_depth_;
    shadow_pass.width = 256;
    shadow_pass.height = 256;
    if (auto result = device_.begin_pass(shadow_pass, "world shadow dependency"); !result) return fail(result.error);
    for (const auto& item : scene_.items)
        if (item.family == GeometryFamily::shadow)
            if (auto result = draw_item(item, tick); !result) { device_.end_pass(); return result; }
    if (auto result = device_.end_pass(); !result) return fail(result.error);

    renderer::RenderPassDesc main_pass;
    main_pass.color_targets[0] = main_color_;
    main_pass.color_target_count = 1;
    main_pass.depth_target = main_depth_;
    main_pass.width = scene_.camera.viewport_width;
    main_pass.height = scene_.camera.viewport_height;
    if (auto result = device_.begin_pass(main_pass, "world main pass"); !result) return fail(result.error);
    for (const auto& item : scene_.items)
        if (item.family != GeometryFamily::shadow)
            if (auto result = draw_item(item, tick); !result) { device_.end_pass(); return result; }
    if (auto result = device_.end_pass(); !result) return fail(result.error);
    device_.record_marker("world-frame complete tick=" + std::to_string(tick));
    frame_recorded_ = true;
    last_tick_ = tick;
    return {};
}

renderer::ValidationResult WorldRecorder::draw_item(const WorldItem& item, UInt32 tick)
{
    std::vector<UInt8> vertices(static_cast<std::size_t>(vertex_stride * item.vertex_count));
    std::vector<UInt8> indices(static_cast<std::size_t>(index_stride * item.index_count));
    const UInt8 seed = static_cast<UInt8>((tick + item.animation_frame + static_cast<UInt32>(item.family) * 17U) & 0xffU);
    std::fill(vertices.begin(), vertices.end(), seed);
    for (std::size_t index = 0; index < indices.size(); ++index) indices[index] = static_cast<UInt8>((seed + index) & 0xffU);
    if (auto result = device_.upload({vertex_stream_, vertex_stream_size_, 0, vertex_stride * item.vertex_count}, vertices.data()); !result)
        return fail(item.label + ": " + result.error);
    if (auto result = device_.upload({index_stream_, index_stream_size_, 0, index_stride * item.index_count}, indices.data()); !result)
        return fail(item.label + ": " + result.error);

    std::array<UInt32, 32> object_data{};
    std::memcpy(object_data.data(), item.transform.elements.data(), sizeof(item.transform.elements));
    object_data[20] = tick;
    object_data[21] = item.animation_frame;
    std::array<UInt32, 32> material_data{};
    material_data[0] = item.material.lighting.ambient_argb;
    material_data[1] = item.material.fog.color_argb;
    material_data[2] = item.material.color_write_mask;
    material_data[3] = item.material.texture_stage_count;
    if (auto result = device_.upload({object_uniform_, sizeof(object_data), 0, sizeof(object_data)}, object_data.data()); !result)
        return fail(item.label + ": " + result.error);
    if (auto result = device_.upload({material_uniform_, sizeof(material_data), 0, sizeof(material_data)}, material_data.data()); !result)
        return fail(item.label + ": " + result.error);

    MaterialTranslation translated;
    if (auto result = translate_material(item.material, item.family, vertex_shader_, fragment_shader_, translated); !result)
        return fail(item.label + ": " + result.error);
    auto pipeline = device_.create_pipeline(renderer::PipelineKey(translated.pipeline), "world frame " + item.label);
    if (!pipeline) return fail(device_.last_error());

    device_.record_marker(state_marker(item));
    renderer::DrawDesc draw;
    draw.pipeline = pipeline;
    draw.vertex_buffer = vertex_stream_;
    draw.index_buffer = index_stream_;
    draw.vertex_or_index_count = item.index_count;
    draw.vertex_bindings.uniforms[0] = {frame_uniform_, 0, 256};
    draw.vertex_bindings.uniforms[1] = {object_uniform_, 0, 128};
    draw.vertex_bindings.uniform_count = 2;
    draw.fragment_bindings.uniforms[0] = {material_uniform_, 0, 128};
    draw.fragment_bindings.uniform_count = 1;
    for (UInt32 index = 0; index < 4; ++index) {
        draw.fragment_bindings.textures[index] = index == 3 ? shadow_color_ : base_textures_[index];
        draw.fragment_bindings.samplers[index] = samplers_[index];
    }
    draw.fragment_bindings.texture_count = 4;
    if (auto result = device_.draw(draw); !result) return fail(item.label + ": " + result.error);
    return {};
}

renderer::ValidationResult WorldRecorder::teardown()
{
    if (!loaded_) return fail("teardown requires a loaded map");
    if (device_.pass_active()) return fail("teardown is forbidden while a render pass is active");
    device_.record_marker("world-teardown map=" + scene_.map_name);
    destroy_resources();
    scene_ = {};
    loaded_ = false;
    frame_recorded_ = false;
    last_tick_ = 0;
    return {};
}

void WorldRecorder::destroy_resources()
{
    for (auto handle : pipelines_) if (handle) device_.destroy(handle);
    pipelines_.clear();
    for (auto handle : samplers_) if (handle) device_.destroy(handle);
    for (auto handle : base_textures_) if (handle) device_.destroy(handle);
    if (shadow_depth_) device_.destroy(shadow_depth_);
    if (shadow_color_) device_.destroy(shadow_color_);
    if (main_depth_) device_.destroy(main_depth_);
    if (main_color_) device_.destroy(main_color_);
    if (material_uniform_) device_.destroy(material_uniform_);
    if (object_uniform_) device_.destroy(object_uniform_);
    if (frame_uniform_) device_.destroy(frame_uniform_);
    if (index_stream_) device_.destroy(index_stream_);
    if (vertex_stream_) device_.destroy(vertex_stream_);
    if (fragment_shader_) device_.destroy(fragment_shader_);
    if (vertex_shader_) device_.destroy(vertex_shader_);
    vertex_shader_ = {}; fragment_shader_ = {}; vertex_stream_ = {}; index_stream_ = {};
    vertex_stream_size_ = 0; index_stream_size_ = 0;
    frame_uniform_ = {}; object_uniform_ = {}; material_uniform_ = {};
    base_textures_ = {}; samplers_ = {};
    main_color_ = {}; main_depth_ = {}; shadow_color_ = {}; shadow_depth_ = {};
}

} // namespace zh::world
