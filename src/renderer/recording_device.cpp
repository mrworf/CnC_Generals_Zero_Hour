#include "zh/renderer/recording_device.h"
#include <stdexcept>

#include <algorithm>
#include <atomic>
#include <cstring>
#include <exception>
#include <limits>
#include <new>
#include <sstream>
#include <type_traits>
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
struct TextureRecord { TextureDesc desc; std::vector<std::vector<UInt8>> mips; bool initialized=false; };
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
HandleType allocate(std::vector<Slot<Record>>& slots, std::size_t& next_id, std::string_view label, Value&& value,
    bool append_only = false)
{
    for (std::size_t index = 0; index < slots.size(); ++index) {
        auto& slot = slots[index];
        if (!append_only && !slot.alive && slot.generation) {
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

bool is_depth(TextureFormat format)
{ return format==TextureFormat::depth16 || format==TextureFormat::depth24_stencil8 || format==TextureFormat::depth32; }

UInt64 bounded_sum(UInt64 a, UInt64 b) noexcept
{
    return a>RendererLimits::maximum_upload_bytes || b>RendererLimits::maximum_upload_bytes
        ? std::numeric_limits<UInt64>::max() : a+b;
}

} // namespace

class RecordingGpuDevice::Impl {
public:
    explicit Impl(std::size_t capacity, std::size_t views) : pipeline_capacity(capacity), view_capacity(views) {}

    struct Checkpoint {
        DeviceTransactionDesc desc;
        DeviceTransactionToken token;
        bool failed = false;
        UInt32 operations = 0, creates = 0, views = 0;
        UInt64 used_bytes = 0;
        std::size_t command_count = 0, view_count = 0;
        std::array<TextureHandle, RendererLimits::color_targets> active_colors{};
        std::vector<UInt8> last_draw_indices;
        std::vector<Slot<BufferRecord>> buffers;
        std::vector<Slot<TextureRecord>> textures;
        std::vector<Slot<SamplerRecord>> samplers;
        std::vector<Slot<ShaderRecord>> shaders;
        std::vector<Slot<PipelineRecord>> pipelines;
        std::size_t next_buffer=0, next_texture=0, next_sampler=0, next_shader=0, next_pipeline=0;
    };

    // Any exception after admission poisons commit, but leaves abort available.
    struct OperationGuard {
        Impl& owner;
        int exceptions = std::uncaught_exceptions();
        ~OperationGuard() noexcept {
            if (owner.checkpoint && std::uncaught_exceptions()>exceptions) owner.checkpoint->failed=true;
        }
    };

    bool admit(bool frame = false, UInt64 bytes = 0, bool create = false, UInt32 views = 0)
    {
        if (!checkpoint) return true;
        auto& c=*checkpoint;
        if (c.failed) return false;
        if ((frame && c.desc.mode!=DeviceTransactionMode::frame_commands) ||
            c.operations>=c.desc.commands || bytes>c.desc.bytes-c.used_bytes ||
            (create && c.creates>=c.desc.resources) || views>c.desc.views-c.views) {
            c.failed=true;
            last_error="transaction: mode or bounded journal capacity rejected";
            return false;
        }
        if (transaction_failure_countdown!=std::numeric_limits<unsigned>::max()) {
            if (!transaction_failure_countdown) {
                transaction_failure_countdown=std::numeric_limits<unsigned>::max();
                c.failed=true;
                last_error="transaction: injected operation failure";
                return false;
            }
            --transaction_failure_countdown;
        }
        ++c.operations;
        c.used_bytes+=bytes;
        c.creates+=create ? 1U : 0U;
        c.views+=views;
        return true;
    }

    bool matches(const DeviceTransactionToken& t) const noexcept {
        return checkpoint && t.device==device_identity && t.sequence==checkpoint->token.sequence &&
            t.generation==checkpoint->token.generation && t.mode==checkpoint->token.mode;
    }

    template<class Record>
    static void restore_slots(std::vector<Slot<Record>>& current, std::vector<Slot<Record>>& baseline) noexcept
    {
        static_assert(std::is_nothrow_move_constructible_v<Slot<Record>>);
        // Both tables were reserved at entry. Candidate slots never reused an
        // old slot. Keep their tombstones, advancing/retiring their generation,
        // so even a later ordinary allocation cannot resurrect an aborted ID.
        while (baseline.size()<current.size()) {
            Slot<Record> retired;
            const auto generation=current[baseline.size()].generation;
            retired.generation=generation>=index_mask ? 0 : generation+1;
            baseline.push_back(std::move(retired));
        }
        current.swap(baseline);
    }

    ValidationResult fail(std::string operation, std::string reason, std::string_view label = {})
    {
        if (checkpoint) {
            checkpoint->failed=true;
            if (reject_next_diagnostic_allocation) {
                reject_next_diagnostic_allocation=false;
                throw std::bad_alloc();
            }
        }
        last_error = std::move(operation) + ": " + std::move(reason);
        if (!label.empty()) last_error += " [label=" + std::string(label) + "]";
        if (!checkpoint) commands.push_back("error " + quoted(last_error));
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
        OperationGuard guard{*this};
        if (!admit()) return;
        auto* slot = lookup(slots, handle);
        if (!slot) {
            fail("destroy", "stale or destroyed " + std::string(kind) + " handle");
            return;
        }
        commands.push_back("destroy " + std::string(1, prefix) + std::to_string(slot->normalized_id) + " label=" + quoted(slot->label));
        slot->alive = false;
        slot->label.clear();
        if (++slot->generation > index_mask) slot->generation = checkpoint ? 0 : 1;
    }

    std::size_t pipeline_capacity;
    std::size_t view_capacity;
    std::size_t view_count = 0;
    std::array<bool,9> supported_texture_formats{true,true,true,true,true,true,true,true,true};
    unsigned texture_create_failure_countdown=std::numeric_limits<unsigned>::max();
    unsigned texture_upload_failure_countdown=std::numeric_limits<unsigned>::max();
    bool reject_next_sampler_create=false;
    bool reject_next_shader_create=false;
    bool reject_next_pipeline_create=false;
    unsigned buffer_create_failure_countdown=std::numeric_limits<unsigned>::max();
    unsigned buffer_upload_failure_countdown=std::numeric_limits<unsigned>::max();
    unsigned draw_failure_countdown=std::numeric_limits<unsigned>::max();
    bool in_pass = false;
    std::string active_pass_label;
    std::array<TextureHandle, RendererLimits::color_targets> active_colors{};
    UInt32 active_color_count = 0;
    TextureHandle active_depth;
    UInt32 active_width=0,active_height=0;
    UInt64 active_target_generation=0;
    std::string last_error;
    std::vector<std::string> commands;
    std::vector<UInt8> last_draw_indices;
    std::vector<Slot<BufferRecord>> buffers;
    std::vector<Slot<TextureRecord>> textures;
    std::vector<Slot<SamplerRecord>> samplers;
    std::vector<Slot<ShaderRecord>> shaders;
    std::vector<Slot<PipelineRecord>> pipelines;
    std::size_t next_buffer = 0, next_texture = 0, next_sampler = 0, next_shader = 0, next_pipeline = 0;
    inline static std::atomic<UInt64> next_device_identity{1};
    const UInt64 device_identity=next_device_identity.fetch_add(1,std::memory_order_relaxed);
    UInt64 next_transaction=0;
    bool reject_next_checkpoint=false;
    bool reject_next_diagnostic_allocation=false;
    unsigned transaction_failure_countdown=std::numeric_limits<unsigned>::max();
    std::unique_ptr<Checkpoint> checkpoint;
};

RecordingGpuDevice::RecordingGpuDevice(std::size_t pipeline_capacity, std::size_t view_capacity)
    : impl_(std::make_unique<Impl>(pipeline_capacity, view_capacity)) {}
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
    if (render_target && (format==TextureFormat::bgr5a1 || format==TextureFormat::bc1 ||
        format==TextureFormat::bc2 || format==TextureFormat::bc3)) return false;
    return true;
}

void RecordingGpuDevice::set_texture_format_supported(TextureFormat format, bool supported)
{
    const auto index=static_cast<std::size_t>(format);
    if (index>=impl_->supported_texture_formats.size()) throw std::invalid_argument("unsupported texture format enum");
    impl_->supported_texture_formats[index]=supported;
}

void RecordingGpuDevice::fail_next_texture_create() { impl_->texture_create_failure_countdown=0; }
void RecordingGpuDevice::fail_next_texture_upload() { impl_->texture_upload_failure_countdown=0; }
void RecordingGpuDevice::fail_texture_create_after(unsigned successful_creates)
{ impl_->texture_create_failure_countdown=successful_creates; }
void RecordingGpuDevice::fail_texture_upload_after(unsigned successful_uploads)
{ impl_->texture_upload_failure_countdown=successful_uploads; }
void RecordingGpuDevice::fail_next_sampler_create() { impl_->reject_next_sampler_create=true; }
void RecordingGpuDevice::fail_next_shader_create() { impl_->reject_next_shader_create=true; }
void RecordingGpuDevice::fail_next_pipeline_create() { impl_->reject_next_pipeline_create=true; }
void RecordingGpuDevice::fail_next_buffer_create() { impl_->buffer_create_failure_countdown=0; }
void RecordingGpuDevice::fail_buffer_create_after(unsigned successful_creates)
{ impl_->buffer_create_failure_countdown=successful_creates; }
void RecordingGpuDevice::fail_next_buffer_upload() { impl_->buffer_upload_failure_countdown=0; }
void RecordingGpuDevice::fail_buffer_upload_after(unsigned successful_uploads)
{ impl_->buffer_upload_failure_countdown=successful_uploads; }
void RecordingGpuDevice::fail_next_draw() { impl_->draw_failure_countdown=0; }
void RecordingGpuDevice::fail_draw_after(unsigned successful_draws)
{ impl_->draw_failure_countdown=successful_draws; }

void RecordingGpuDevice::fail_next_transaction_checkpoint() { impl_->reject_next_checkpoint=true; }
void RecordingGpuDevice::fail_next_transaction_diagnostic_allocation()
{ impl_->reject_next_diagnostic_allocation=true; }
void RecordingGpuDevice::fail_transaction_operation_after(unsigned successful_operations)
{ impl_->transaction_failure_countdown=successful_operations; }
bool RecordingGpuDevice::supports_device_transactions(DeviceTransactionMode mode) const noexcept
{ return mode==DeviceTransactionMode::idle_preparation || mode==DeviceTransactionMode::frame_commands; }

ValidationResult RecordingGpuDevice::begin_device_transaction(const DeviceTransactionDesc& desc,
    DeviceTransactionToken& token)
{
    if (impl_->checkpoint || impl_->in_pass || !supports_device_transactions(desc.mode) ||
        !desc.generation || !desc.commands || desc.commands>4096 || !desc.resources || desc.resources>4096 ||
        !desc.bytes || desc.bytes>RendererLimits::maximum_upload_bytes || desc.views>RendererLimits::ordered_views ||
        (desc.mode==DeviceTransactionMode::idle_preparation && desc.views) ||
        (desc.mode==DeviceTransactionMode::frame_commands && !desc.views) ||
        impl_->next_transaction==std::numeric_limits<UInt64>::max())
        return {false,"transaction: invalid, nested, or active-pass admission"};
    if (impl_->reject_next_checkpoint) {
        impl_->reject_next_checkpoint=false;
        return {false,"transaction: injected checkpoint failure"};
    }
    const auto resources=impl_->buffers.size()+impl_->textures.size()+impl_->samplers.size()+
        impl_->shaders.size()+impl_->pipelines.size();
    UInt64 bytes=impl_->last_draw_indices.size();
    const auto charge=[&](UInt64 amount) {
        if (amount>desc.bytes-bytes) return false;
        bytes+=amount; return true;
    };
    if (bytes>desc.bytes || resources>desc.resources) return {false,"transaction: baseline exceeds budget"};
    for (const auto& slot:impl_->buffers)
        if (!charge(slot.label.size()) || !charge(slot.value.bytes.size())) return {false,"transaction: baseline exceeds budget"};
    for (const auto& slot:impl_->textures) {
        if (!charge(slot.label.size())) return {false,"transaction: baseline exceeds budget"};
        for (const auto& mip:slot.value.mips) if (!charge(mip.size())) return {false,"transaction: baseline exceeds budget"};
    }
    for (const auto& slot:impl_->samplers) if (!charge(slot.label.size())) return {false,"transaction: baseline exceeds budget"};
    for (const auto& slot:impl_->shaders)
        if (!charge(slot.label.size()) || !charge(slot.value.name.size())) return {false,"transaction: baseline exceeds budget"};
    for (const auto& slot:impl_->pipelines) if (!charge(slot.label.size())) return {false,"transaction: baseline exceeds budget"};
    try {
        auto c=std::make_unique<Impl::Checkpoint>();
        c->desc=desc;
        c->creates=static_cast<UInt32>(resources);
        c->used_bytes=bytes;
        c->command_count=impl_->commands.size(); c->view_count=impl_->view_count;
        c->active_colors=impl_->active_colors; c->last_draw_indices=impl_->last_draw_indices;
        c->buffers=impl_->buffers; c->textures=impl_->textures; c->samplers=impl_->samplers;
        c->shaders=impl_->shaders; c->pipelines=impl_->pipelines;
        c->next_buffer=impl_->next_buffer; c->next_texture=impl_->next_texture;
        c->next_sampler=impl_->next_sampler; c->next_shader=impl_->next_shader; c->next_pipeline=impl_->next_pipeline;
        // Allocation failure here changes capacity only, never observable state.
        const auto reserve=[&](auto& current, auto& baseline) {
            if (current.size()+desc.resources>index_mask) throw std::length_error("transaction resource table bound");
            current.reserve(current.size()+desc.resources);
            baseline.reserve(current.size()+desc.resources);
        };
        reserve(impl_->buffers,c->buffers); reserve(impl_->textures,c->textures);
        reserve(impl_->samplers,c->samplers); reserve(impl_->shaders,c->shaders); reserve(impl_->pipelines,c->pipelines);
        impl_->commands.reserve(impl_->commands.size()+desc.commands);
        c->token={impl_->device_identity,impl_->next_transaction+1,desc.generation,desc.mode};
        token=c->token;
        ++impl_->next_transaction;
        impl_->checkpoint=std::move(c);
        return {};
    } catch (const std::exception&) {
        return {false,"transaction: bounded checkpoint allocation failed"};
    }
}

bool RecordingGpuDevice::commit_device_transaction(const DeviceTransactionToken& token) noexcept
{
    if (!impl_->matches(token) || impl_->checkpoint->failed || impl_->in_pass) return false;
    impl_->checkpoint.reset();
    return true;
}

bool RecordingGpuDevice::abort_device_transaction(const DeviceTransactionToken& token) noexcept
{
    if (!impl_->matches(token)) return false;
    auto& c=*impl_->checkpoint;
    Impl::restore_slots(impl_->buffers,c.buffers); Impl::restore_slots(impl_->textures,c.textures);
    Impl::restore_slots(impl_->samplers,c.samplers); Impl::restore_slots(impl_->shaders,c.shaders);
    Impl::restore_slots(impl_->pipelines,c.pipelines);
    impl_->commands.resize(c.command_count);
    impl_->last_draw_indices.swap(c.last_draw_indices);
    impl_->view_count=c.view_count; impl_->in_pass=false; impl_->active_pass_label.clear();
    impl_->active_colors=c.active_colors; impl_->active_color_count=0; impl_->active_depth={};
    impl_->active_width=impl_->active_height=0; impl_->active_target_generation=0;
    impl_->next_buffer=c.next_buffer; impl_->next_texture=c.next_texture;
    impl_->next_sampler=c.next_sampler; impl_->next_shader=c.next_shader; impl_->next_pipeline=c.next_pipeline;
    impl_->checkpoint.reset();
    return true;
}

BufferHandle RecordingGpuDevice::create_buffer(const BufferDesc& desc, std::string_view label)
{
    Impl::OperationGuard guard{*impl_};
    if (!impl_->admit(false,bounded_sum(desc.size,label.size()),true)) return {};
    if (impl_->buffer_create_failure_countdown!=std::numeric_limits<unsigned>::max()) {
        if (!impl_->buffer_create_failure_countdown) {
            impl_->buffer_create_failure_countdown=std::numeric_limits<unsigned>::max();
            impl_->fail("create_buffer", "injected buffer creation failure", label); return {};
        }
        --impl_->buffer_create_failure_countdown;
    }
    if (auto result = validate(desc); !result) { impl_->fail("create_buffer", result.error, label); return {}; }
    if (label.empty()) { impl_->fail("create_buffer", "label must not be empty"); return {}; }
    BufferRecord record{desc, std::vector<UInt8>(static_cast<std::size_t>(desc.size), 0)};
    auto handle = allocate<BufferHandle>(impl_->buffers, impl_->next_buffer, label, std::move(record),bool(impl_->checkpoint));
    if (!handle) { impl_->fail("create_buffer", "resource table exhausted", label); return {}; }
    impl_->commands.push_back("create_buffer " + impl_->name(impl_->buffers, handle, 'B') + " label=" + quoted(label)
        + " size=" + std::to_string(desc.size) + " usage=" + enum_value(desc.usage)
        + " dynamic=" + (desc.dynamic ? "true" : "false"));
    return handle;
}

TextureHandle RecordingGpuDevice::create_texture(const TextureDesc& desc, std::string_view label)
{
    Impl::OperationGuard guard{*impl_};
    if (!impl_->admit(false,label.size(),true)) return {};
    if (impl_->texture_create_failure_countdown!=std::numeric_limits<unsigned>::max()) {
        if (impl_->texture_create_failure_countdown) {
            --impl_->texture_create_failure_countdown;
        } else {
            impl_->texture_create_failure_countdown=std::numeric_limits<unsigned>::max();
            impl_->fail("create_texture", "injected texture creation failure", label); return {};
        }
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
    auto handle = allocate<TextureHandle>(impl_->textures, impl_->next_texture, label, TextureRecord{desc},bool(impl_->checkpoint));
    if (!handle) { impl_->fail("create_texture", "resource table exhausted", label); return {}; }
    impl_->commands.push_back("create_texture " + impl_->name(impl_->textures, handle, 'T') + " label=" + quoted(label)
        + " extent=" + std::to_string(desc.width) + "x" + std::to_string(desc.height)
        + " layers=" + std::to_string(desc.depth_or_layers) + " mips=" + std::to_string(desc.mip_levels)
        + " dimension=" + enum_value(desc.dimension) + " format=" + enum_value(desc.format)
        + " rt=" + (desc.render_target ? "true" : "false") + " sampled=" + (desc.sampled ? "true" : "false"));
    return handle;
}

std::optional<TextureFormat> RecordingGpuDevice::describe_texture_format(TextureHandle handle) const noexcept
{
    const auto* slot=lookup(impl_->textures,handle);
    return slot ? std::optional<TextureFormat>(slot->value.desc.format) : std::nullopt;
}

SamplerHandle RecordingGpuDevice::create_sampler(const SamplerDesc& desc, std::string_view label)
{
    Impl::OperationGuard guard{*impl_};
    if (!impl_->admit(false,label.size(),true)) return {};
    if (impl_->reject_next_sampler_create) {
        impl_->reject_next_sampler_create=false;
        impl_->fail("create_sampler", "injected sampler creation failure", label); return {};
    }
    if (auto result = validate(desc); !result) { impl_->fail("create_sampler", result.error, label); return {}; }
    if (label.empty()) { impl_->fail("create_sampler", "label must not be empty"); return {}; }
    auto handle = allocate<SamplerHandle>(impl_->samplers, impl_->next_sampler, label, SamplerRecord{desc},bool(impl_->checkpoint));
    if (!handle) { impl_->fail("create_sampler", "resource table exhausted", label); return {}; }
    impl_->commands.push_back("create_sampler " + impl_->name(impl_->samplers, handle, 'S') + " label=" + quoted(label)
        + " filter=" + enum_value(desc.min_filter) + "/" + enum_value(desc.mag_filter) + "/" + enum_value(desc.mip_filter)
        + " address=" + enum_value(desc.address_u) + "/" + enum_value(desc.address_v) + "/" + enum_value(desc.address_w)
        + " aniso=" + std::to_string(desc.maximum_anisotropy));
    return handle;
}

ShaderHandle RecordingGpuDevice::create_shader(const ShaderDesc& desc, std::string_view label)
{
    Impl::OperationGuard guard{*impl_};
    if (!impl_->admit(false,bounded_sum(label.size(),desc.name.size()),true)) return {};
    if (impl_->reject_next_shader_create) {
        impl_->reject_next_shader_create=false;
        impl_->fail("create_shader", "injected shader creation failure", label); return {};
    }
    if (auto result = validate(desc); !result) { impl_->fail("create_shader", result.error, label); return {}; }
    if (label.empty()) { impl_->fail("create_shader", "label must not be empty"); return {}; }
    ShaderRecord record{desc.stage, std::string(desc.name), desc.uniform_buffers, desc.samplers};
    auto handle = allocate<ShaderHandle>(impl_->shaders, impl_->next_shader, label, std::move(record),bool(impl_->checkpoint));
    if (!handle) { impl_->fail("create_shader", "resource table exhausted", label); return {}; }
    impl_->commands.push_back("create_shader " + impl_->name(impl_->shaders, handle, 'H') + " label=" + quoted(label)
        + " name=" + quoted(desc.name) + " stage=" + enum_value(desc.stage)
        + " uniforms=" + std::to_string(desc.uniform_buffers) + " samplers=" + std::to_string(desc.samplers));
    return handle;
}

PipelineHandle RecordingGpuDevice::create_pipeline(const PipelineKey& key, std::string_view label)
{
    Impl::OperationGuard guard{*impl_};
    if (!impl_->admit(false,label.size())) return {};
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
    if (impl_->checkpoint) {
        auto& c=*impl_->checkpoint;
        if (c.creates>=c.desc.resources) { impl_->fail("create_pipeline","transaction resource capacity exceeded");return {}; }
        ++c.creates;
    }
    auto handle = allocate<PipelineHandle>(impl_->pipelines, impl_->next_pipeline, label, PipelineRecord(key),bool(impl_->checkpoint));
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
    Impl::OperationGuard guard{*impl_};
    if (!impl_->admit(false,desc.size)) return {false,impl_->last_error};
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
    Impl::OperationGuard guard{*impl_};
    if (!impl_->admit(false,desc.size)) return {false,impl_->last_error};
    if (impl_->texture_upload_failure_countdown!=std::numeric_limits<unsigned>::max()) {
        if (!impl_->texture_upload_failure_countdown) {
            impl_->texture_upload_failure_countdown=std::numeric_limits<unsigned>::max();
            return impl_->fail("upload_texture", "injected texture upload failure");
        }
        --impl_->texture_upload_failure_countdown;
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
    else if (format==TextureFormat::bgr5a1) block_size=2;
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
    if (desc.mip_level==0 && texture->value.desc.render_target) texture->value.initialized=true;
    return {};
}

ValidationResult RecordingGpuDevice::begin_pass(const RenderPassDesc& desc, std::string_view label)
{
    Impl::OperationGuard guard{*impl_};
    if (!impl_->admit(true,label.size(),false,1)) return {false,impl_->last_error};
    if (impl_->in_pass) return impl_->fail("begin_pass", "render pass is already active", label);
    if (impl_->view_count >= impl_->view_capacity)
        return impl_->fail("begin_pass", "ordered view budget exhausted", label);
    if (auto result = validate(desc); !result) return impl_->fail("begin_pass", result.error, label);
    for (UInt32 index = 0; index < desc.color_target_count; ++index) {
        const auto* target = lookup(impl_->textures, desc.color_targets[index]);
        if (!target || !target->value.desc.render_target || is_depth(target->value.desc.format))
            return impl_->fail("begin_pass", "color target is stale, destroyed, or not renderable", label);
        if (target->value.desc.width != desc.width || target->value.desc.height != desc.height)
            return impl_->fail("begin_pass", "color target extent does not match pass", target->label);
        if (desc.color_load==AttachmentLoad::load && !target->value.initialized)
            return impl_->fail("begin_pass", "cannot load an uninitialized color target", target->label);
    }
    const auto* depth = lookup(impl_->textures, desc.depth_target);
    if (!depth || !depth->value.desc.render_target || !is_depth(depth->value.desc.format))
        return impl_->fail("begin_pass", "depth target is stale, destroyed, or not renderable", label);
    if (depth->value.desc.width != desc.width || depth->value.desc.height != desc.height)
        return impl_->fail("begin_pass", "depth target extent does not match pass", depth->label);
    if (desc.depth_load==AttachmentLoad::load && !depth->value.initialized)
        return impl_->fail("begin_pass", "cannot load an uninitialized depth target", depth->label);
    impl_->in_pass = true;
    impl_->active_pass_label.assign(label);
    impl_->active_colors = desc.color_targets;
    impl_->active_color_count = desc.color_target_count;
    impl_->active_depth = desc.depth_target;
    impl_->active_width=desc.width;
    impl_->active_height=desc.height;
    impl_->active_target_generation=desc.target_generation;
    ++impl_->view_count;
    std::string command = "begin_pass label=" + quoted(label) + " colors=";
    for (UInt32 index = 0; index < desc.color_target_count; ++index) {
        if (index != 0) command += ",";
        command += impl_->name(impl_->textures, desc.color_targets[index], 'T');
    }
    command += " depth=" + impl_->name(impl_->textures, desc.depth_target, 'T') + " extent="
        + std::to_string(desc.width) + "x" + std::to_string(desc.height);
    if (desc.color_load!=AttachmentLoad::clear || desc.depth_load!=AttachmentLoad::clear ||
        desc.clear_color!=std::array<float,4>{0.02F,0.02F,0.04F,1.0F} || desc.clear_depth!=1.0F)
        command += " color_load=" + std::to_string(static_cast<unsigned>(desc.color_load)) +
            " depth_load=" + std::to_string(static_cast<unsigned>(desc.depth_load)) +
            " clear=" + std::to_string(desc.clear_color[0])+","+std::to_string(desc.clear_color[1])+","+
            std::to_string(desc.clear_color[2])+","+std::to_string(desc.clear_color[3])+","+
            std::to_string(desc.clear_depth);
    impl_->commands.push_back(std::move(command));
    return {};
}

std::pair<UInt32,UInt32> RecordingGpuDevice::active_pass_extent() const noexcept
{ return impl_->in_pass ? std::pair{impl_->active_width,impl_->active_height} : std::pair<UInt32,UInt32>{0,0}; }

ValidationResult RecordingGpuDevice::set_viewport(const ViewportDesc& desc)
{
    Impl::OperationGuard guard{*impl_};
    if (!impl_->admit(true)) return {false,impl_->last_error};
    if (!impl_->in_pass) return impl_->fail("set_viewport","no render pass is active");
    if (auto result=validate(desc,impl_->active_width,impl_->active_height); !result)
        return impl_->fail("set_viewport",result.error,impl_->active_pass_label);
    impl_->commands.push_back("viewport="+std::to_string(desc.x)+","+std::to_string(desc.y)+
        ","+std::to_string(desc.width)+","+std::to_string(desc.height)+
        " depth="+std::to_string(desc.min_depth)+":"+std::to_string(desc.max_depth));
    return {};
}

ValidationResult RecordingGpuDevice::clear_viewport(const ViewportClearDesc& desc)
{
    Impl::OperationGuard guard{*impl_};
    if (!impl_->admit(true,0,false,1)) return {false,impl_->last_error};
    if (!impl_->in_pass) return impl_->fail("clear_viewport", "no render pass is active");
    if (impl_->view_count >= impl_->view_capacity)
        return impl_->fail("clear_viewport", "ordered view budget exhausted", impl_->active_pass_label);
    if (auto result=validate(desc,impl_->active_width,impl_->active_height); !result)
        return impl_->fail("clear_viewport",result.error,impl_->active_pass_label);
    if (desc.target_generation != impl_->active_target_generation)
        return impl_->fail("clear_viewport","target generation does not match active pass",impl_->active_pass_label);
    if (desc.color && (std::find(impl_->active_colors.begin(),
            impl_->active_colors.begin()+impl_->active_color_count,desc.color_target)==
            impl_->active_colors.begin()+impl_->active_color_count ||
            !lookup(impl_->textures,desc.color_target)))
        return impl_->fail("clear_viewport","color target is stale or not attached",impl_->active_pass_label);
    if ((desc.depth || desc.stencil) && (desc.depth_target!=impl_->active_depth ||
            !lookup(impl_->textures,desc.depth_target)))
        return impl_->fail("clear_viewport","depth target is stale or not attached",impl_->active_pass_label);
    if (desc.stencil && lookup(impl_->textures,desc.depth_target)->value.desc.format!=TextureFormat::depth24_stencil8)
        return impl_->fail("clear_viewport","stencil clear requires a stencil attachment",impl_->active_pass_label);
    const auto left=std::max<std::int64_t>(0,desc.x);
    const auto top=std::max<std::int64_t>(0,desc.y);
    const auto right=std::min<std::int64_t>(impl_->active_width,static_cast<std::int64_t>(desc.x)+desc.width);
    const auto bottom=std::min<std::int64_t>(impl_->active_height,static_cast<std::int64_t>(desc.y)+desc.height);
    std::string command="clear_viewport rect="+std::to_string(left)+","+std::to_string(top)+","+
        std::to_string(right-left)+","+std::to_string(bottom-top)+
        " generation="+std::to_string(desc.target_generation)+
        " flags="+(desc.color ? "C" : "-")+(desc.depth ? "D" : "-")+(desc.stencil ? "S" : "-");
    if (desc.color) command+=" color="+std::to_string(desc.color_value[0])+","+
        std::to_string(desc.color_value[1])+","+std::to_string(desc.color_value[2])+","+
        std::to_string(desc.color_value[3]);
    if (desc.depth) command+=" depth="+std::to_string(desc.depth_value);
    if (desc.stencil) command+=" stencil="+std::to_string(desc.stencil_value);
    impl_->commands.push_back(std::move(command));
    ++impl_->view_count;
    return {};
}

ValidationResult RecordingGpuDevice::draw(const DrawDesc& desc)
{
    Impl::OperationGuard guard{*impl_};
    const UInt64 index_bytes=desc.index_buffer ? static_cast<UInt64>(desc.vertex_or_index_count)*static_cast<UInt8>(desc.index_element_size) : 0;
    if (!impl_->admit(true,index_bytes,false,1)) return {false,impl_->last_error};
    if (!impl_->in_pass) return impl_->fail("draw", "draw requires an active render pass");
    if (impl_->view_count >= impl_->view_capacity)
        return impl_->fail("draw", "ordered view budget exhausted", impl_->active_pass_label);
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
    ++impl_->view_count;
    return {};
}

ValidationResult RecordingGpuDevice::end_pass()
{
    Impl::OperationGuard guard{*impl_};
    if (!impl_->admit(true)) return {false,impl_->last_error};
    if (!impl_->in_pass) return impl_->fail("end_pass", "no render pass is active");
    for (UInt32 index=0; index<impl_->active_color_count; ++index)
        lookup(impl_->textures,impl_->active_colors[index])->value.initialized=true;
    lookup(impl_->textures,impl_->active_depth)->value.initialized=true;
    impl_->commands.push_back("end_pass label=" + quoted(impl_->active_pass_label));
    impl_->in_pass = false;
    impl_->active_pass_label.clear();
    impl_->active_color_count = 0;
    impl_->active_depth = {};
    impl_->active_width=impl_->active_height=0;
    impl_->active_target_generation=0;
    return {};
}

ValidationResult RecordingGpuDevice::present(TextureHandle source)
{
    Impl::OperationGuard guard{*impl_};
    if (!impl_->admit(true)) return {false,impl_->last_error};
    if (impl_->in_pass) return impl_->fail("present", "cannot present while a render pass is active");
    const auto* texture = lookup(impl_->textures, source);
    if (!texture || !texture->value.desc.render_target || texture->value.desc.format != TextureFormat::rgba8)
        return impl_->fail("present", "source texture is stale or not a color render target");
    if (!texture->value.initialized) return impl_->fail("present", "source color target is not initialized");
    impl_->commands.push_back("present " + impl_->name(impl_->textures, source, 'T') + " label=" + quoted(texture->label));
    impl_->view_count=0;
    return {};
}

void RecordingGpuDevice::destroy(BufferHandle handle) { impl_->destroy_resource(impl_->buffers, handle, 'B', "buffer"); }
void RecordingGpuDevice::destroy(TextureHandle handle)
{
    Impl::OperationGuard guard{*impl_};
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
    const auto count=impl_->checkpoint ? impl_->checkpoint->command_count : impl_->commands.size();
    for (std::size_t i=0;i<count;++i) stream << impl_->commands[i] << '\n';
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

RecordingOperationCounts RecordingGpuDevice::operation_counts() const noexcept
{
    RecordingOperationCounts counts;
    counts.commands = impl_->checkpoint ? impl_->checkpoint->command_count : impl_->commands.size();
    for (std::size_t i=0;i<counts.commands;++i) {
        const auto& command=impl_->commands[i];
        const auto begins = [&command](const char* prefix) {
            const std::size_t length = std::char_traits<char>::length(prefix);
            return command.size() >= length && command.compare(0, length, prefix) == 0;
        };
        if (begins("create_")) ++counts.creates;
        else if (begins("upload ") || begins("upload_texture ")) ++counts.uploads;
        else if (begins("begin_pass ")) ++counts.passes;
        else if (begins("draw ")) ++counts.draws;
        else if (begins("present ")) ++counts.presents;
        else if (begins("error ")) ++counts.failures;
    }
    return counts;
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

TextureDesc RecordingGpuDevice::texture_descriptor(TextureHandle handle) const
{
    const auto* texture=lookup(impl_->textures,handle);
    if (!texture) throw std::runtime_error("recording texture handle is stale or destroyed");
    return texture->value.desc;
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
void RecordingGpuDevice::record_marker(std::string_view marker)
{
    Impl::OperationGuard guard{*impl_};
    if (!impl_->admit(false,marker.size())) throw std::runtime_error("transaction marker rejected");
    impl_->commands.push_back("marker " + quoted(marker));
}

} // namespace zh::renderer
