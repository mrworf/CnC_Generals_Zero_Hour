#include "zh/renderer/recording_device.h"
#include <stdexcept>

#include <algorithm>
#include <cstring>
#include <limits>
#include <sstream>
#include <utility>

namespace zh::renderer {
namespace {

constexpr UInt32 index_mask = 0xffffU;

template <typename HandleType>
HandleType make_handle(std::size_t index, UInt32 generation)
{
    if (index >= index_mask || generation == 0 || generation > index_mask) return {};
    return HandleType((generation << 16U) | static_cast<UInt32>(index + 1));
}

template <typename HandleType>
std::size_t handle_index(HandleType handle)
{
    const UInt32 encoded = handle.value() & index_mask;
    return encoded == 0 ? std::numeric_limits<std::size_t>::max() : encoded - 1U;
}

template <typename HandleType>
UInt32 handle_generation(HandleType handle)
{
    return handle.value() >> 16U;
}

template <typename Record>
struct Slot {
    UInt32 generation = 1;
    bool alive = false;
    std::size_t normalized_id = 0;
    std::string label;
    Record value{};
};

struct BufferRecord { BufferDesc desc; std::vector<UInt8> bytes; };
struct TextureRecord { TextureDesc desc; std::vector<std::vector<UInt8>> mips; };
struct SamplerRecord { SamplerDesc desc; };
struct ShaderRecord { ShaderStage stage{}; std::string name; UInt32 uniforms = 0; UInt32 samplers = 0; };
struct PipelineRecord {
    PipelineRecord() : key(PipelineDesc{}) {}
    explicit PipelineRecord(const PipelineKey& value) : key(value) {}
    PipelineKey key;
};

template <typename HandleType, typename Record>
const Slot<Record>* lookup(const std::vector<Slot<Record>>& slots, HandleType handle)
{
    const auto index = handle_index(handle);
    if (index >= slots.size()) return nullptr;
    const auto& slot = slots[index];
    return slot.alive && slot.generation == handle_generation(handle) ? &slot : nullptr;
}

template <typename HandleType, typename Record>
Slot<Record>* lookup(std::vector<Slot<Record>>& slots, HandleType handle)
{
    return const_cast<Slot<Record>*>(lookup(static_cast<const std::vector<Slot<Record>>&>(slots), handle));
}

template <typename HandleType, typename Record, typename Value>
HandleType allocate(std::vector<Slot<Record>>& slots, std::size_t& next_id, std::string_view label, Value&& value)
{
    for (std::size_t index = 0; index < slots.size(); ++index) {
        auto& slot = slots[index];
        if (!slot.alive) {
            slot.alive = true;
            slot.normalized_id = ++next_id;
            slot.label.assign(label);
            slot.value = std::forward<Value>(value);
            return make_handle<HandleType>(index, slot.generation);
        }
    }
    if (slots.size() >= index_mask) return {};
    Slot<Record> slot;
    slot.alive = true;
    slot.normalized_id = ++next_id;
    slot.label.assign(label);
    slot.value = std::forward<Value>(value);
    slots.push_back(std::move(slot));
    return make_handle<HandleType>(slots.size() - 1, slots.back().generation);
}

std::string quoted(std::string_view text)
{
    std::string result;
    result.reserve(text.size() + 2);
    result.push_back('"');
    for (char c : text) {
        if (c == '"' || c == '\\') result.push_back('\\');
        result.push_back(c == '\n' ? ' ' : c);
    }
    result.push_back('"');
    return result;
}

template <typename Enum>
std::string enum_value(Enum value) { return std::to_string(static_cast<unsigned>(value)); }

} // namespace

class RecordingGpuDevice::Impl {
public:
    explicit Impl(std::size_t capacity) : pipeline_capacity(capacity) {}

    ValidationResult fail(std::string operation, std::string reason, std::string_view label = {})
    {
        last_error = std::move(operation) + ": " + std::move(reason);
        if (!label.empty()) last_error += " [label=" + std::string(label) + "]";
        commands.push_back("error " + quoted(last_error));
        return {false, last_error};
    }

    template <typename HandleType, typename Record>
    std::string name(const std::vector<Slot<Record>>& slots, HandleType handle, char prefix) const
    {
        const auto* slot = lookup(slots, handle);
        return slot ? std::string(1, prefix) + std::to_string(slot->normalized_id) : std::string(1, prefix) + "?";
    }

    template <typename HandleType, typename Record>
    void destroy_resource(std::vector<Slot<Record>>& slots, HandleType handle, char prefix, std::string_view kind)
    {
        auto* slot = lookup(slots, handle);
        if (!slot) {
            fail("destroy", "stale or destroyed " + std::string(kind) + " handle");
            return;
        }
        commands.push_back("destroy " + std::string(1, prefix) + std::to_string(slot->normalized_id) + " label=" + quoted(slot->label));
        slot->alive = false;
        slot->label.clear();
        if (++slot->generation > index_mask) slot->generation = 1;
    }

    std::size_t pipeline_capacity;
    std::array<bool,8> supported_texture_formats{true,true,true,true,true,true,true,true};
    bool reject_next_texture_create=false;
    bool reject_next_texture_upload=false;
    bool reject_next_sampler_create=false;
    bool reject_next_shader_create=false;
    bool reject_next_pipeline_create=false;
    bool reject_next_buffer_create=false;
    unsigned buffer_upload_failure_countdown=std::numeric_limits<unsigned>::max();
    unsigned draw_failure_countdown=std::numeric_limits<unsigned>::max();
    bool in_pass = false;
    std::string active_pass_label;
    std::array<TextureHandle, RendererLimits::color_targets> active_colors{};
    UInt32 active_color_count = 0;
    TextureHandle active_depth;
    std::string last_error;
    std::vector<std::string> commands;
    std::vector<UInt8> last_draw_indices;
    std::vector<Slot<BufferRecord>> buffers;
    std::vector<Slot<TextureRecord>> textures;
    std::vector<Slot<SamplerRecord>> samplers;
    std::vector<Slot<ShaderRecord>> shaders;
    std::vector<Slot<PipelineRecord>> pipelines;
    std::size_t next_buffer = 0, next_texture = 0, next_sampler = 0, next_shader = 0, next_pipeline = 0;
};

RecordingGpuDevice::RecordingGpuDevice(std::size_t pipeline_capacity) : impl_(std::make_unique<Impl>(pipeline_capacity)) {}
RecordingGpuDevice::~RecordingGpuDevice() = default;
RecordingGpuDevice::RecordingGpuDevice(RecordingGpuDevice&&) noexcept = default;
RecordingGpuDevice& RecordingGpuDevice::operator=(RecordingGpuDevice&&) noexcept = default;

bool RecordingGpuDevice::supports_texture_format(TextureFormat format, TextureDimension dimension, bool sampled, bool render_target) const noexcept
{
    if (dimension!=TextureDimension::texture_2d && dimension!=TextureDimension::cube &&
        dimension!=TextureDimension::texture_3d) return false;
    const auto index=static_cast<std::size_t>(format);
    if (index>=impl_->supported_texture_formats.size() || !impl_->supported_texture_formats[index]) return false;
    if (!sampled && !render_target) return false;
    if (render_target && (format==TextureFormat::bc1 || format==TextureFormat::bc2 || format==TextureFormat::bc3)) return false;
    return true;
}

void RecordingGpuDevice::set_texture_format_supported(TextureFormat format, bool supported)
{
    const auto index=static_cast<std::size_t>(format);
    if (index>=impl_->supported_texture_formats.size()) throw std::invalid_argument("unsupported texture format enum");
    impl_->supported_texture_formats[index]=supported;
}

void RecordingGpuDevice::fail_next_texture_create() { impl_->reject_next_texture_create=true; }
void RecordingGpuDevice::fail_next_texture_upload() { impl_->reject_next_texture_upload=true; }
void RecordingGpuDevice::fail_next_sampler_create() { impl_->reject_next_sampler_create=true; }
void RecordingGpuDevice::fail_next_shader_create() { impl_->reject_next_shader_create=true; }
void RecordingGpuDevice::fail_next_pipeline_create() { impl_->reject_next_pipeline_create=true; }
void RecordingGpuDevice::fail_next_buffer_create() { impl_->reject_next_buffer_create=true; }
void RecordingGpuDevice::fail_next_buffer_upload() { impl_->buffer_upload_failure_countdown=0; }
void RecordingGpuDevice::fail_buffer_upload_after(unsigned successful_uploads)
{ impl_->buffer_upload_failure_countdown=successful_uploads; }
void RecordingGpuDevice::fail_next_draw() { impl_->draw_failure_countdown=0; }
void RecordingGpuDevice::fail_draw_after(unsigned successful_draws)
{ impl_->draw_failure_countdown=successful_draws; }

BufferHandle RecordingGpuDevice::create_buffer(const BufferDesc& desc, std::string_view label)
{
    if (impl_->reject_next_buffer_create) {
        impl_->reject_next_buffer_create=false;
        impl_->fail("create_buffer", "injected buffer creation failure", label); return {};
    }
    if (auto result = validate(desc); !result) { impl_->fail("create_buffer", result.error, label); return {}; }
    if (label.empty()) { impl_->fail("create_buffer", "label must not be empty"); return {}; }
    BufferRecord record{desc, std::vector<UInt8>(static_cast<std::size_t>(desc.size), 0)};
    auto handle = allocate<BufferHandle>(impl_->buffers, impl_->next_buffer, label, std::move(record));
    if (!handle) { impl_->fail("create_buffer", "resource table exhausted", label); return {}; }
    impl_->commands.push_back("create_buffer " + impl_->name(impl_->buffers, handle, 'B') + " label=" + quoted(label)
        + " size=" + std::to_string(desc.size) + " usage=" + enum_value(desc.usage)
        + " dynamic=" + (desc.dynamic ? "true" : "false"));
    return handle;
}

TextureHandle RecordingGpuDevice::create_texture(const TextureDesc& desc, std::string_view label)
{
    if (impl_->reject_next_texture_create) {
        impl_->reject_next_texture_create=false;
        impl_->fail("create_texture", "injected texture creation failure", label); return {};
    }
    if (auto result = validate(desc); !result) { impl_->fail("create_texture", result.error, label); return {}; }
    if (desc.width>16384 || desc.height>16384 || desc.depth_or_layers>16384 ||
        static_cast<std::uint64_t>(desc.width)*desc.height*desc.depth_or_layers*4>64ULL*1024*1024) {
        impl_->fail("create_texture", "texture exceeds bounded recording upload budget", label); return {};
    }
    if (!supports_texture_format(desc.format, desc.dimension, desc.sampled, desc.render_target)) {
        impl_->fail("create_texture", "format/dimension/usage unsupported by recording device", label); return {};
    }
    if (label.empty()) { impl_->fail("create_texture", "label must not be empty"); return {}; }
    auto handle = allocate<TextureHandle>(impl_->textures, impl_->next_texture, label, TextureRecord{desc});
    if (!handle) { impl_->fail("create_texture", "resource table exhausted", label); return {}; }
    impl_->commands.push_back("create_texture " + impl_->name(impl_->textures, handle, 'T') + " label=" + quoted(label)
        + " extent=" + std::to_string(desc.width) + "x" + std::to_string(desc.height)
        + " layers=" + std::to_string(desc.depth_or_layers) + " mips=" + std::to_string(desc.mip_levels)
        + " dimension=" + enum_value(desc.dimension) + " format=" + enum_value(desc.format)
        + " rt=" + (desc.render_target ? "true" : "false") + " sampled=" + (desc.sampled ? "true" : "false"));
    return handle;
}

SamplerHandle RecordingGpuDevice::create_sampler(const SamplerDesc& desc, std::string_view label)
{
    if (impl_->reject_next_sampler_create) {
        impl_->reject_next_sampler_create=false;
        impl_->fail("create_sampler", "injected sampler creation failure", label); return {};
    }
    if (auto result = validate(desc); !result) { impl_->fail("create_sampler", result.error, label); return {}; }
    if (label.empty()) { impl_->fail("create_sampler", "label must not be empty"); return {}; }
    auto handle = allocate<SamplerHandle>(impl_->samplers, impl_->next_sampler, label, SamplerRecord{desc});
    if (!handle) { impl_->fail("create_sampler", "resource table exhausted", label); return {}; }
    impl_->commands.push_back("create_sampler " + impl_->name(impl_->samplers, handle, 'S') + " label=" + quoted(label)
        + " filter=" + enum_value(desc.min_filter) + "/" + enum_value(desc.mag_filter) + "/" + enum_value(desc.mip_filter)
        + " address=" + enum_value(desc.address_u) + "/" + enum_value(desc.address_v) + "/" + enum_value(desc.address_w)
        + " aniso=" + std::to_string(desc.maximum_anisotropy));
    return handle;
}

ShaderHandle RecordingGpuDevice::create_shader(const ShaderDesc& desc, std::string_view label)
{
    if (impl_->reject_next_shader_create) {
        impl_->reject_next_shader_create=false;
        impl_->fail("create_shader", "injected shader creation failure", label); return {};
    }
    if (auto result = validate(desc); !result) { impl_->fail("create_shader", result.error, label); return {}; }
    if (label.empty()) { impl_->fail("create_shader", "label must not be empty"); return {}; }
    ShaderRecord record{desc.stage, std::string(desc.name), desc.uniform_buffers, desc.samplers};
    auto handle = allocate<ShaderHandle>(impl_->shaders, impl_->next_shader, label, std::move(record));
    if (!handle) { impl_->fail("create_shader", "resource table exhausted", label); return {}; }
    impl_->commands.push_back("create_shader " + impl_->name(impl_->shaders, handle, 'H') + " label=" + quoted(label)
        + " name=" + quoted(desc.name) + " stage=" + enum_value(desc.stage)
        + " uniforms=" + std::to_string(desc.uniform_buffers) + " samplers=" + std::to_string(desc.samplers));
    return handle;
}

PipelineHandle RecordingGpuDevice::create_pipeline(const PipelineKey& key, std::string_view label)
{
    if (impl_->reject_next_pipeline_create) {
        impl_->reject_next_pipeline_create=false;
        impl_->fail("create_pipeline", "injected pipeline creation failure", label); return {};
    }
    const auto& desc = key.descriptor();
    if (auto result = validate(desc); !result) { impl_->fail("create_pipeline", result.error, label); return {}; }
    const auto* vertex = lookup(impl_->shaders, desc.vertex_shader);
    const auto* fragment = lookup(impl_->shaders, desc.fragment_shader);
    if (!vertex || vertex->value.stage != ShaderStage::vertex) { impl_->fail("create_pipeline", "vertex shader handle is stale or has wrong stage", label); return {}; }
    if (!fragment || fragment->value.stage != ShaderStage::fragment) { impl_->fail("create_pipeline", "fragment shader handle is stale or has wrong stage", label); return {}; }
    for (std::size_t index = 0; index < impl_->pipelines.size(); ++index) {
        const auto& slot = impl_->pipelines[index];
        if (slot.alive && slot.value.key == key) {
            auto handle = make_handle<PipelineHandle>(index, slot.generation);
            impl_->commands.push_back("reuse_pipeline " + impl_->name(impl_->pipelines, handle, 'P') + " request=" + quoted(label));
            return handle;
        }
    }
    if (pipeline_count() >= impl_->pipeline_capacity) { impl_->fail("create_pipeline", "pipeline cache capacity exceeded", label); return {}; }
    auto handle = allocate<PipelineHandle>(impl_->pipelines, impl_->next_pipeline, label, PipelineRecord(key));
    if (!handle) { impl_->fail("create_pipeline", "resource table exhausted", label); return {}; }
    impl_->commands.push_back("create_pipeline " + impl_->name(impl_->pipelines, handle, 'P') + " label=" + quoted(label)
        + " key=" + std::to_string(key.stable_hash()) + " shaders=" + impl_->name(impl_->shaders, desc.vertex_shader, 'H')
        + "/" + impl_->name(impl_->shaders, desc.fragment_shader, 'H') + " layout=" + enum_value(desc.vertex_layout)
        + " topology=" + enum_value(desc.topology) + " color=" + enum_value(desc.color_format)
        + " depth=" + enum_value(desc.depth_format) + " blend=" + (desc.blend.enabled ? "true" : "false")
        + " point_size=" + (desc.uses_point_size ? "true" : "false")
        + " premultiplied=" + (desc.premultiplied_alpha ? "true" : "false")
        + " fog=" + (desc.fog_enabled ? "true" : "false"));
    return handle;
}

ValidationResult RecordingGpuDevice::upload(const UploadDesc& desc, const void* bytes)
{
    if (impl_->buffer_upload_failure_countdown!=std::numeric_limits<unsigned>::max()) {
        if (!impl_->buffer_upload_failure_countdown) {
            impl_->buffer_upload_failure_countdown=std::numeric_limits<unsigned>::max();
            return impl_->fail("upload", "injected buffer upload failure");
        }
        --impl_->buffer_upload_failure_countdown;
    }
    if (auto result = validate(desc); !result) return impl_->fail("upload", result.error);
    auto* buffer = lookup(impl_->buffers, desc.destination);
    if (!buffer) return impl_->fail("upload", "destination buffer handle is stale or destroyed");
    if (!buffer->value.desc.dynamic) return impl_->fail("upload", "destination buffer is not dynamic", buffer->label);
    if (desc.destination_size != buffer->value.desc.size) return impl_->fail("upload", "declared destination size does not match resource", buffer->label);
    if (desc.offset > buffer->value.desc.size || desc.size > buffer->value.desc.size - desc.offset)
        return impl_->fail("upload", "range exceeds actual destination buffer", buffer->label);
    if (!bytes) return impl_->fail("upload", "source bytes are null", buffer->label);
    std::memcpy(buffer->value.bytes.data() + static_cast<std::size_t>(desc.offset), bytes, static_cast<std::size_t>(desc.size));
    impl_->commands.push_back("upload " + impl_->name(impl_->buffers, desc.destination, 'B') + " offset="
        + std::to_string(desc.offset) + " size=" + std::to_string(desc.size));
    return {};
}

ValidationResult RecordingGpuDevice::upload_texture(const TextureUploadDesc& desc, const void* bytes)
{
    if (impl_->reject_next_texture_upload) {
        impl_->reject_next_texture_upload=false;
        return impl_->fail("upload_texture", "injected texture upload failure");
    }
    auto* texture = lookup(impl_->textures, desc.destination);
    if (!texture) return impl_->fail("upload_texture", "destination texture handle is stale or destroyed");
    if (!bytes) return impl_->fail("upload_texture", "source bytes are null", texture->label);
    if (texture->value.desc.dimension != TextureDimension::texture_2d ||
        desc.mip_level >= texture->value.desc.mip_levels)
        return impl_->fail("upload_texture", "unsupported dimension or mip level", texture->label);
    const auto format=texture->value.desc.format;
    UInt64 block_size=0;
    UInt32 block_extent=1;
    if (format==TextureFormat::rgba8 || format==TextureFormat::bgra8) block_size=4;
    else if (format==TextureFormat::bc1) { block_size=8; block_extent=4; }
    else if (format==TextureFormat::bc2 || format==TextureFormat::bc3) { block_size=16; block_extent=4; }
    else return impl_->fail("upload_texture", "texture format has no color upload path", texture->label);
    if (desc.width != std::max(1U,texture->value.desc.width >> desc.mip_level) ||
        desc.height != std::max(1U,texture->value.desc.height >> desc.mip_level))
        return impl_->fail("upload_texture", "extent does not match destination texture", texture->label);
    const UInt64 minimum_pitch = static_cast<UInt64>((desc.width+block_extent-1)/block_extent)*block_size;
    const auto rows=(desc.height+block_extent-1)/block_extent;
    if (desc.row_pitch < minimum_pitch || desc.row_pitch%block_size ||
        desc.size != static_cast<UInt64>(desc.row_pitch)*rows)
        return impl_->fail("upload_texture", "row pitch or byte count is invalid", texture->label);
    if (texture->value.mips.size()<texture->value.desc.mip_levels)
        texture->value.mips.resize(texture->value.desc.mip_levels);
    auto& payload=texture->value.mips[desc.mip_level];
    payload.resize(static_cast<std::size_t>(desc.size));
    std::memcpy(payload.data(),bytes,payload.size());
    impl_->commands.push_back("upload_texture " + impl_->name(impl_->textures, desc.destination, 'T') + " extent="
        + std::to_string(desc.width) + "x" + std::to_string(desc.height) + " bytes=" + std::to_string(desc.size)
        + (desc.mip_level ? " mip="+std::to_string(desc.mip_level) : ""));
    return {};
}

ValidationResult RecordingGpuDevice::begin_pass(const RenderPassDesc& desc, std::string_view label)
{
    if (impl_->in_pass) return impl_->fail("begin_pass", "render pass is already active", label);
    if (auto result = validate(desc); !result) return impl_->fail("begin_pass", result.error, label);
    for (UInt32 index = 0; index < desc.color_target_count; ++index) {
        const auto* target = lookup(impl_->textures, desc.color_targets[index]);
        if (!target || !target->value.desc.render_target) return impl_->fail("begin_pass", "color target is stale, destroyed, or not renderable", label);
        if (target->value.desc.width != desc.width || target->value.desc.height != desc.height)
            return impl_->fail("begin_pass", "color target extent does not match pass", target->label);
    }
    const auto* depth = lookup(impl_->textures, desc.depth_target);
    if (!depth || !depth->value.desc.render_target) return impl_->fail("begin_pass", "depth target is stale, destroyed, or not renderable", label);
    if (depth->value.desc.width != desc.width || depth->value.desc.height != desc.height)
        return impl_->fail("begin_pass", "depth target extent does not match pass", depth->label);
    impl_->in_pass = true;
    impl_->active_pass_label.assign(label);
    impl_->active_colors = desc.color_targets;
    impl_->active_color_count = desc.color_target_count;
    impl_->active_depth = desc.depth_target;
    std::string command = "begin_pass label=" + quoted(label) + " colors=";
    for (UInt32 index = 0; index < desc.color_target_count; ++index) {
        if (index != 0) command += ",";
        command += impl_->name(impl_->textures, desc.color_targets[index], 'T');
    }
    command += " depth=" + impl_->name(impl_->textures, desc.depth_target, 'T') + " extent="
        + std::to_string(desc.width) + "x" + std::to_string(desc.height);
    impl_->commands.push_back(std::move(command));
    return {};
}

ValidationResult RecordingGpuDevice::draw(const DrawDesc& desc)
{
    if (!impl_->in_pass) return impl_->fail("draw", "draw requires an active render pass");
    const auto* pipeline = lookup(impl_->pipelines, desc.pipeline);
    if (!pipeline) return impl_->fail("draw", "pipeline handle is stale or destroyed", impl_->active_pass_label);
    if (auto result = validate(desc, pipeline->value.key.descriptor()); !result) return impl_->fail("draw", result.error, impl_->active_pass_label);
    const auto* vertex = lookup(impl_->buffers, desc.vertex_buffer);
    if (!vertex || vertex->value.desc.usage != BufferUsage::vertex) return impl_->fail("draw", "vertex buffer is stale, destroyed, or has wrong usage", impl_->active_pass_label);
    if (desc.index_buffer) {
        const auto* index = lookup(impl_->buffers, desc.index_buffer);
        if (!index || index->value.desc.usage != BufferUsage::index) return impl_->fail("draw", "index buffer is stale, destroyed, or has wrong usage", impl_->active_pass_label);
        const UInt64 element_size = static_cast<UInt8>(desc.index_element_size);
        if (static_cast<UInt64>(desc.first_index) + desc.vertex_or_index_count > index->value.desc.size / element_size)
            return impl_->fail("draw", "indexed draw range exceeds index buffer", impl_->active_pass_label);
        if (auto result = validate_original_fvf_indexed_vertices(desc, pipeline->value.key.descriptor(),
                vertex->value.desc.size, index->value.bytes.data(), index->value.bytes.size()); !result)
            return impl_->fail("draw", result.error, impl_->active_pass_label);
    } else if (auto result = validate_original_fvf_indexed_vertices(desc, pipeline->value.key.descriptor(),
                   vertex->value.desc.size, nullptr, 0); !result) {
        return impl_->fail("draw", result.error, impl_->active_pass_label);
    }
    const auto& pipeline_desc = pipeline->value.key.descriptor();
    const auto* vertex_shader = lookup(impl_->shaders, pipeline_desc.vertex_shader);
    const auto* fragment_shader = lookup(impl_->shaders, pipeline_desc.fragment_shader);
    if (!vertex_shader || !fragment_shader) return impl_->fail("draw", "pipeline references a destroyed shader", impl_->active_pass_label);
    const auto check_stage = [&](const StageBindings& bindings, const ShaderRecord& shader, std::string_view stage) -> ValidationResult {
        if (bindings.uniform_count < shader.uniforms || bindings.texture_count < shader.samplers)
            return impl_->fail("draw", std::string(stage) + " bindings do not satisfy shader requirements", impl_->active_pass_label);
        for (UInt32 i = 0; i < bindings.uniform_count; ++i) {
            const auto* buffer = lookup(impl_->buffers, bindings.uniforms[i].buffer);
            if (!buffer) return impl_->fail("draw", std::string(stage) + " uniform buffer is stale or destroyed", impl_->active_pass_label);
            if (buffer->value.desc.usage != BufferUsage::uniform)
                return impl_->fail("draw", std::string(stage) + " binding does not reference a uniform buffer", buffer->label);
            if (bindings.uniforms[i].offset > buffer->value.desc.size
                || bindings.uniforms[i].size > buffer->value.desc.size - bindings.uniforms[i].offset)
                return impl_->fail("draw", std::string(stage) + " uniform range exceeds actual buffer", buffer->label);
        }
        for (UInt32 i = 0; i < bindings.texture_count; ++i)
            if (!lookup(impl_->textures, bindings.textures[i]) || !lookup(impl_->samplers, bindings.samplers[i]))
                return impl_->fail("draw", std::string(stage) + " texture or sampler is stale or destroyed", impl_->active_pass_label);
        return {};
    };
    if (auto result = check_stage(desc.vertex_bindings, vertex_shader->value, "vertex"); !result) return result;
    if (auto result = check_stage(desc.fragment_bindings, fragment_shader->value, "fragment"); !result) return result;
    if (impl_->draw_failure_countdown!=std::numeric_limits<unsigned>::max()) {
        if (!impl_->draw_failure_countdown) {
            impl_->draw_failure_countdown=std::numeric_limits<unsigned>::max();
            return impl_->fail("draw","injected physical draw failure",impl_->active_pass_label);
        }
        --impl_->draw_failure_countdown;
    }
    impl_->last_draw_indices.clear();
    if (desc.index_buffer) {
        const auto* index=lookup(impl_->buffers,desc.index_buffer);
        const auto element_size=static_cast<std::size_t>(static_cast<UInt8>(desc.index_element_size));
        const auto first=static_cast<std::size_t>(desc.first_index)*element_size;
        const auto last=first+static_cast<std::size_t>(desc.vertex_or_index_count)*element_size;
        impl_->last_draw_indices.assign(index->value.bytes.begin()+first,index->value.bytes.begin()+last);
    }
    std::string command = "draw pipeline=" + impl_->name(impl_->pipelines, desc.pipeline, 'P') + " vertex="
        + impl_->name(impl_->buffers, desc.vertex_buffer, 'B') + " index="
        + (desc.index_buffer ? impl_->name(impl_->buffers, desc.index_buffer, 'B') : "none")
        + " count=" + std::to_string(desc.vertex_or_index_count) + " point_size=" + std::to_string(desc.point_size);
    if (desc.index_buffer && (desc.index_element_size != IndexElementSize::uint32 || desc.first_index || desc.base_vertex))
        command += " index_bits=" + std::to_string(8U * static_cast<UInt8>(desc.index_element_size))
            + " first_index=" + std::to_string(desc.first_index) + " base_vertex=" + std::to_string(desc.base_vertex);
    const auto append_bindings = [&](const StageBindings& bindings, std::string_view stage) {
        command += " " + std::string(stage) + "_uniforms=";
        for (UInt32 i = 0; i < bindings.uniform_count; ++i) {
            if (i != 0) command += ",";
            command += impl_->name(impl_->buffers, bindings.uniforms[i].buffer, 'B') + "@"
                + std::to_string(bindings.uniforms[i].offset) + "+" + std::to_string(bindings.uniforms[i].size);
        }
        command += " " + std::string(stage) + "_textures=";
        for (UInt32 i = 0; i < bindings.texture_count; ++i) {
            if (i != 0) command += ",";
            command += impl_->name(impl_->textures, bindings.textures[i], 'T') + "/"
                + impl_->name(impl_->samplers, bindings.samplers[i], 'S');
        }
    };
    append_bindings(desc.vertex_bindings, "vertex");
    append_bindings(desc.fragment_bindings, "fragment");
    impl_->commands.push_back(std::move(command));
    return {};
}

ValidationResult RecordingGpuDevice::end_pass()
{
    if (!impl_->in_pass) return impl_->fail("end_pass", "no render pass is active");
    impl_->commands.push_back("end_pass label=" + quoted(impl_->active_pass_label));
    impl_->in_pass = false;
    impl_->active_pass_label.clear();
    impl_->active_color_count = 0;
    impl_->active_depth = {};
    return {};
}

ValidationResult RecordingGpuDevice::present(TextureHandle source)
{
    if (impl_->in_pass) return impl_->fail("present", "cannot present while a render pass is active");
    const auto* texture = lookup(impl_->textures, source);
    if (!texture || !texture->value.desc.render_target || texture->value.desc.format != TextureFormat::rgba8)
        return impl_->fail("present", "source texture is stale or not a color render target");
    impl_->commands.push_back("present " + impl_->name(impl_->textures, source, 'T') + " label=" + quoted(texture->label));
    return {};
}

void RecordingGpuDevice::destroy(BufferHandle handle) { impl_->destroy_resource(impl_->buffers, handle, 'B', "buffer"); }
void RecordingGpuDevice::destroy(TextureHandle handle)
{
    if (impl_->in_pass) {
        const bool active_color = std::find(impl_->active_colors.begin(), impl_->active_colors.begin() + impl_->active_color_count, handle)
            != impl_->active_colors.begin() + impl_->active_color_count;
        if (active_color || impl_->active_depth == handle) {
            const auto* target = lookup(impl_->textures, handle);
            impl_->fail("destroy", "texture is attached to the active render pass", target ? target->label : std::string_view{});
            return;
        }
    }
    impl_->destroy_resource(impl_->textures, handle, 'T', "texture");
}
void RecordingGpuDevice::destroy(SamplerHandle handle) { impl_->destroy_resource(impl_->samplers, handle, 'S', "sampler"); }
void RecordingGpuDevice::destroy(ShaderHandle handle) { impl_->destroy_resource(impl_->shaders, handle, 'H', "shader"); }
void RecordingGpuDevice::destroy(PipelineHandle handle) { impl_->destroy_resource(impl_->pipelines, handle, 'P', "pipeline"); }

const std::string& RecordingGpuDevice::last_error() const noexcept { return impl_->last_error; }
std::string RecordingGpuDevice::snapshot() const
{
    std::ostringstream stream;
    for (const auto& command : impl_->commands) stream << command << '\n';
    return stream.str();
}
std::size_t RecordingGpuDevice::pipeline_count() const noexcept
{
    return static_cast<std::size_t>(std::count_if(impl_->pipelines.begin(), impl_->pipelines.end(), [](const auto& slot) { return slot.alive; }));
}
ResourceCounts RecordingGpuDevice::resource_counts() const noexcept
{
    const auto count = [](const auto& slots) {
        return static_cast<std::size_t>(std::count_if(slots.begin(), slots.end(), [](const auto& slot) { return slot.alive; }));
    };
    return {count(impl_->buffers), count(impl_->textures), count(impl_->samplers),
        count(impl_->shaders), count(impl_->pipelines)};
}
bool RecordingGpuDevice::pass_active() const noexcept { return impl_->in_pass; }
std::vector<UInt8> RecordingGpuDevice::buffer_bytes(BufferHandle handle) const
{
    const auto* buffer = lookup(impl_->buffers, handle);
    return buffer ? buffer->value.bytes : std::vector<UInt8>{};
}
std::vector<UInt8> RecordingGpuDevice::last_draw_index_bytes() const
{ return impl_->last_draw_indices; }

std::vector<UInt8> RecordingGpuDevice::texture_bytes(TextureHandle handle, UInt32 mip_level) const
{
    const auto* texture=lookup(impl_->textures,handle);
    if (!texture || mip_level>=texture->value.mips.size()) return {};
    return texture->value.mips[mip_level];
}

SamplerDesc RecordingGpuDevice::sampler_descriptor(SamplerHandle handle) const
{
    const auto* sampler=lookup(impl_->samplers,handle);
    if (!sampler) throw std::runtime_error("recording sampler handle is stale or destroyed");
    return sampler->value.desc;
}
PipelineDesc RecordingGpuDevice::pipeline_descriptor(PipelineHandle handle) const
{
    const auto* pipeline=lookup(impl_->pipelines,handle);
    if (!pipeline) throw std::runtime_error("recording pipeline handle is stale or destroyed");
    return pipeline->value.key.descriptor();
}
void RecordingGpuDevice::record_marker(std::string_view marker) { impl_->commands.push_back("marker " + quoted(marker)); }

} // namespace zh::renderer
