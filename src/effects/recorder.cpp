#include "zh/effects/recorder.h"

#include <algorithm>
#include <array>
#include <cmath>
#include <set>
#include <string>
#include <tuple>
#include <utility>
#include <vector>

namespace zh::effects {
namespace {

constexpr renderer::UInt32 target_width = 640;
constexpr renderer::UInt32 target_height = 360;
constexpr renderer::UInt64 vertex_stride = 32;

renderer::TextureDesc target_desc(renderer::TextureFormat format)
{
    renderer::TextureDesc desc;
    desc.width = target_width;
    desc.height = target_height;
    desc.format = format;
    desc.render_target = true;
    return desc;
}

renderer::RenderPassDesc pass_desc(renderer::TextureHandle color, renderer::TextureHandle depth)
{
    renderer::RenderPassDesc pass;
    pass.color_targets[0] = color;
    pass.color_target_count = 1;
    pass.depth_target = depth;
    pass.width = target_width;
    pass.height = target_height;
    return pass;
}

} // namespace

EffectsRecorder::EffectsRecorder(renderer::GpuDevice& device) : device_(device) {}

renderer::ValidationResult EffectsRecorder::fail(std::string message)
{
    last_error_ = "effects recorder: " + std::move(message);
    device_.record_marker("effects-error " + last_error_);
    return {false, last_error_};
}

renderer::ValidationResult EffectsRecorder::load(std::vector<EffectRequest> requests)
{
    if (loaded_) return fail("load requires an empty recorder; teardown the current effects first");
    if (requests.empty()) return fail("load requires at least one effect request");

    std::vector<PreparedEffect> prepared;
    std::set<std::pair<std::string, std::string>> logical_materials;
    for (std::size_t index = 0; index < requests.size(); ++index) {
        auto& request = requests[index];
        const auto prefix = "effect request " + std::to_string(index) + ": ";
        if (request.logical_asset.empty()) return fail(prefix + "logical asset must not be empty");
        if (request.material.empty()) return fail(prefix + "material must not be empty");
        if (request.legacy_effect.empty()) return fail(prefix + "legacy effect must not be empty");
        if (request.vertex_count == 0 || request.vertex_count > 1'000'000)
            return fail(prefix + "vertex count must be in range 1..1000000");
        if (!std::isfinite(request.point_size) || request.point_size <= 0.0F || request.point_size > 4096.0F)
            return fail(prefix + "point size must be finite and in range (0,4096]");
        if (!logical_materials.emplace(request.logical_asset, request.material).second)
            return fail(prefix + "duplicates logical asset '" + request.logical_asset + "' material '" + request.material + "'");
        const EffectMapping* mapping = nullptr;
        if (auto result = find_effect(effect_registry(), request.legacy_effect, request.logical_asset, request.material, mapping); !result)
            return fail(result.error);
        prepared.push_back({std::move(request), mapping});
    }
    std::sort(prepared.begin(), prepared.end(), [](const PreparedEffect& left, const PreparedEffect& right) {
        return std::tie(left.mapping->kind, left.request.logical_asset, left.request.material)
            < std::tie(right.mapping->kind, right.request.logical_asset, right.request.material);
    });
    effects_ = std::move(prepared);
    if (auto result = create_resources(); !result) {
        destroy_resources();
        effects_.clear();
        return result;
    }
    loaded_ = true;
    frame_recorded_ = false;
    last_tick_ = 0;
    device_.record_marker("effects-load count=" + std::to_string(effects_.size()) + " wwshade-runtime=linked");
    device_.record_marker("effects-render-target dependency source -> main-effects -> post-output");
    return {};
}

renderer::ValidationResult EffectsRecorder::create_resources()
{
    renderer::UInt32 maximum_vertices = 0;
    for (const auto& effect : effects_) maximum_vertices = std::max(maximum_vertices, effect.request.vertex_count);
    vertex_buffer_size_ = static_cast<renderer::UInt64>(maximum_vertices) * vertex_stride;
    vertex_buffer_ = device_.create_buffer({vertex_buffer_size_, renderer::BufferUsage::vertex, true}, "effects vertex stream");
    for (std::size_t index = 0; index < uniforms_.size(); ++index)
        uniforms_[index] = device_.create_buffer({256, renderer::BufferUsage::uniform, true}, "effects uniform " + std::to_string(index));

    for (std::size_t index = 0; index < textures_.size(); ++index) {
        renderer::TextureDesc desc;
        desc.width = 1;
        desc.height = 1;
        textures_[index] = device_.create_texture(desc, "effects texture " + std::to_string(index));
        renderer::SamplerDesc sampler;
        sampler.address_u = index == 3 ? renderer::AddressMode::clamp_border : renderer::AddressMode::repeat;
        sampler.address_v = sampler.address_u;
        samplers_[index] = device_.create_sampler(sampler, "effects sampler " + std::to_string(index));
    }
    renderer::TextureDesc environment_desc;
    environment_desc.width = 1;
    environment_desc.height = 1;
    environment_desc.dimension = renderer::TextureDimension::cube;
    environment_desc.depth_or_layers = 6;
    environment_texture_ = device_.create_texture(environment_desc, "effects environment cube");
    source_color_ = device_.create_texture(target_desc(renderer::TextureFormat::rgba8), "effects source color");
    source_depth_ = device_.create_texture(target_desc(renderer::TextureFormat::depth24_stencil8), "effects source depth");
    main_color_ = device_.create_texture(target_desc(renderer::TextureFormat::rgba8), "effects main color");
    main_depth_ = device_.create_texture(target_desc(renderer::TextureFormat::depth24_stencil8), "effects main depth");
    output_color_ = device_.create_texture(target_desc(renderer::TextureFormat::rgba8), "effects output color");
    output_depth_ = device_.create_texture(target_desc(renderer::TextureFormat::depth24_stencil8), "effects output depth");
    if (!vertex_buffer_) return fail(device_.last_error());
    for (auto handle : uniforms_) if (!handle) return fail(device_.last_error());
    for (auto handle : textures_) if (!handle) return fail(device_.last_error());
    if (!environment_texture_) return fail(device_.last_error());
    for (auto handle : samplers_) if (!handle) return fail(device_.last_error());
    if (!source_color_ || !source_depth_ || !main_color_ || !main_depth_ || !output_color_ || !output_depth_)
        return fail(device_.last_error());

    for (auto& effect : effects_) {
        const auto& mapping = *effect.mapping;
        effect.vertex_shader = device_.create_shader(
            {renderer::ShaderStage::vertex, mapping.vertex_source, mapping.vertex_uniforms, 0},
            std::string(mapping.legacy_name) + " vertex");
        effect.fragment_shader = device_.create_shader(
            {renderer::ShaderStage::fragment, mapping.fragment_source, mapping.fragment_uniforms, mapping.samplers},
            std::string(mapping.legacy_name) + " fragment");
        if (!effect.vertex_shader || !effect.fragment_shader) return fail(device_.last_error());
        effect.first_pipeline = pipelines_.size();
        for (renderer::UInt32 pass = 0; pass < mapping.pass_count; ++pass) {
            renderer::PipelineDesc desc;
            desc.vertex_shader = effect.vertex_shader;
            desc.fragment_shader = effect.fragment_shader;
            desc.vertex_layout = mapping.vertex_layout;
            desc.topology = mapping.topology;
            desc.blend.enabled = mapping.blending;
            desc.blend.source_color = pass == 0 ? mapping.source_blend : renderer::BlendFactor::one;
            desc.blend.destination_color = pass == 0 ? mapping.destination_blend : renderer::BlendFactor::one;
            desc.blend.source_alpha = desc.blend.source_color;
            desc.blend.destination_alpha = desc.blend.destination_color;
            desc.depth_stencil.depth_test = mapping.depth_test;
            desc.depth_stencil.depth_write = pass == 0 && mapping.depth_write;
            desc.raster.depth_bias = mapping.depth_bias + static_cast<float>(pass) * 0.25F;
            desc.uses_point_size = mapping.point_size;
            desc.premultiplied_alpha = mapping.premultiplied_alpha;
            auto pipeline = device_.create_pipeline(renderer::PipelineKey(desc),
                std::string(mapping.legacy_name) + " pass " + std::to_string(pass + 1));
            if (!pipeline) return fail(device_.last_error());
            pipelines_.push_back(pipeline);
        }
    }
    return {};
}

renderer::ValidationResult EffectsRecorder::record_frame(renderer::UInt32 tick)
{
    if (!loaded_) return fail("record_frame requires loaded effects");
    if (frame_recorded_ && tick < last_tick_)
        return fail("frame tick regressed from " + std::to_string(last_tick_) + " to " + std::to_string(tick));

    std::array<renderer::UInt32, 64> uniform_data{};
    uniform_data[0] = tick;
    uniform_data[1] = target_width;
    uniform_data[2] = target_height;
    for (std::size_t index = 0; index < uniforms_.size(); ++index) {
        uniform_data[3] = static_cast<renderer::UInt32>(index);
        if (auto result = device_.upload({uniforms_[index], sizeof(uniform_data), 0, sizeof(uniform_data)}, uniform_data.data()); !result)
            return fail(result.error);
    }

    if (auto result = device_.begin_pass(pass_desc(source_color_, source_depth_), "effects source dependency"); !result)
        return fail(result.error);
    device_.record_marker("effects-source ready for water/stealth");
    if (auto result = device_.end_pass(); !result) return fail(result.error);

    if (auto result = device_.begin_pass(pass_desc(main_color_, main_depth_), "effects main pass"); !result)
        return fail(result.error);
    for (const auto& effect : effects_) {
        if (effect.mapping->kind == EffectKind::post_effect) continue;
        if (auto result = draw_effect(effect, tick); !result) { device_.end_pass(); return result; }
    }
    if (auto result = device_.end_pass(); !result) return fail(result.error);

    if (auto result = device_.begin_pass(pass_desc(output_color_, output_depth_), "effects post output"); !result)
        return fail(result.error);
    for (const auto& effect : effects_) {
        if (effect.mapping->kind != EffectKind::post_effect) continue;
        if (auto result = draw_effect(effect, tick); !result) { device_.end_pass(); return result; }
    }
    if (auto result = device_.end_pass(); !result) return fail(result.error);
    device_.record_marker("effects-frame complete tick=" + std::to_string(tick));
    frame_recorded_ = true;
    last_tick_ = tick;
    return {};
}

renderer::ValidationResult EffectsRecorder::draw_effect(const PreparedEffect& effect, renderer::UInt32 tick)
{
    const auto& mapping = *effect.mapping;
    std::vector<renderer::UInt8> vertices(static_cast<std::size_t>(effect.request.vertex_count) * vertex_stride);
    const auto seed = static_cast<renderer::UInt8>((tick + static_cast<renderer::UInt32>(mapping.kind) * 29U) & 0xffU);
    std::fill(vertices.begin(), vertices.end(), seed);
    if (auto result = device_.upload({vertex_buffer_, vertex_buffer_size_, 0, vertices.size()}, vertices.data()); !result)
        return fail(effect.request.logical_asset + ": " + result.error);

    for (renderer::UInt32 pass = 0; pass < mapping.pass_count; ++pass) {
        const float depth_bias = mapping.depth_bias + static_cast<float>(pass) * 0.25F;
        device_.record_marker("effect kind=" + std::string(effect_kind_name(mapping.kind))
            + " asset=" + effect.request.logical_asset + " material=" + effect.request.material
            + " legacy=" + std::string(mapping.legacy_name) + " pass=" + std::to_string(pass + 1)
            + "/" + std::to_string(mapping.pass_count) + " depth-bias=" + std::to_string(depth_bias)
            + " premultiplied=" + (mapping.premultiplied_alpha ? "true" : "false")
            + " render-target-input=" + (mapping.render_target_input ? "true" : "false"));
        renderer::DrawDesc draw;
        draw.pipeline = pipelines_[effect.first_pipeline + pass];
        draw.vertex_buffer = vertex_buffer_;
        draw.vertex_or_index_count = effect.request.vertex_count;
        draw.point_size = mapping.point_size ? effect.request.point_size : 0.0F;
        for (renderer::UInt32 slot = 0; slot < mapping.vertex_uniforms; ++slot)
            draw.vertex_bindings.uniforms[slot] = {uniforms_[slot], 0, 256};
        draw.vertex_bindings.uniform_count = mapping.vertex_uniforms;
        for (renderer::UInt32 slot = 0; slot < mapping.fragment_uniforms; ++slot)
            draw.fragment_bindings.uniforms[slot] = {uniforms_[slot], 0, 256};
        draw.fragment_bindings.uniform_count = mapping.fragment_uniforms;
        for (renderer::UInt32 slot = 0; slot < mapping.samplers; ++slot) {
            auto texture = textures_[slot % textures_.size()];
            if (mapping.kind == EffectKind::bump_environment && slot == 2)
                texture = environment_texture_;
            if (mapping.render_target_input && slot == 0)
                texture = mapping.kind == EffectKind::post_effect ? main_color_ : source_color_;
            draw.fragment_bindings.textures[slot] = texture;
            draw.fragment_bindings.samplers[slot] = samplers_[slot % samplers_.size()];
        }
        draw.fragment_bindings.texture_count = mapping.samplers;
        if (auto result = device_.draw(draw); !result)
            return fail(effect.request.logical_asset + "/" + effect.request.material + ": " + result.error);
    }
    return {};
}

renderer::ValidationResult EffectsRecorder::teardown()
{
    if (!loaded_) return fail("teardown requires loaded effects");
    if (device_.pass_active()) return fail("teardown is forbidden while a render pass is active");
    device_.record_marker("effects-teardown count=" + std::to_string(effects_.size()));
    destroy_resources();
    effects_.clear();
    loaded_ = false;
    frame_recorded_ = false;
    last_tick_ = 0;
    return {};
}

void EffectsRecorder::destroy_resources()
{
    for (auto handle : pipelines_) if (handle) device_.destroy(handle);
    pipelines_.clear();
    for (auto& effect : effects_) {
        if (effect.fragment_shader) device_.destroy(effect.fragment_shader);
        if (effect.vertex_shader) device_.destroy(effect.vertex_shader);
        effect.fragment_shader = {};
        effect.vertex_shader = {};
        effect.first_pipeline = 0;
    }
    if (output_depth_) device_.destroy(output_depth_);
    if (output_color_) device_.destroy(output_color_);
    if (main_depth_) device_.destroy(main_depth_);
    if (main_color_) device_.destroy(main_color_);
    if (source_depth_) device_.destroy(source_depth_);
    if (source_color_) device_.destroy(source_color_);
    for (auto handle : samplers_) if (handle) device_.destroy(handle);
    if (environment_texture_) device_.destroy(environment_texture_);
    for (auto handle : textures_) if (handle) device_.destroy(handle);
    for (auto handle : uniforms_) if (handle) device_.destroy(handle);
    if (vertex_buffer_) device_.destroy(vertex_buffer_);
    output_depth_ = {}; output_color_ = {}; main_depth_ = {}; main_color_ = {};
    source_depth_ = {}; source_color_ = {}; samplers_ = {}; environment_texture_ = {}; textures_ = {}; uniforms_ = {};
    vertex_buffer_ = {}; vertex_buffer_size_ = 0;
}

} // namespace zh::effects
