#include "zh/platform/bgfx_device.h"

#include <bgfx/bgfx.h>

#include <algorithm>
#include <atomic>
#include <cmath>
#include <cstring>
#include <limits>
#include <stdexcept>
#include <vector>

namespace zh::renderer {
namespace {

std::atomic<bool> runtime_owned{false};
constexpr UInt32 slot_mask = 0xffffU;

template <class HandleType> HandleType encode(std::size_t index, UInt32 generation)
{
    if (index >= slot_mask || generation == 0 || generation > slot_mask) return {};
    return HandleType((generation << 16U) | static_cast<UInt32>(index + 1));
}

template <class Record> struct Slot {
    UInt32 generation = 1;
    bool alive = false;
    Record record{};
};

template <class HandleType, class Record>
Slot<Record>* lookup(std::vector<Slot<Record>>& slots, HandleType handle)
{
    const UInt32 low = handle.value() & slot_mask;
    if (!low || low > slots.size()) return nullptr;
    auto& slot = slots[low - 1];
    return slot.alive && slot.generation == handle.value() >> 16U ? &slot : nullptr;
}

template <class HandleType, class Record>
HandleType insert(std::vector<Slot<Record>>& slots, Record value)
{
    for (std::size_t i = 0; i < slots.size(); ++i) {
        if (!slots[i].alive) {
            slots[i].alive = true;
            slots[i].record = std::move(value);
            return encode<HandleType>(i, slots[i].generation);
        }
    }
    if (slots.size() >= slot_mask) return {};
    slots.push_back({1, true, std::move(value)});
    return encode<HandleType>(slots.size() - 1, 1);
}

template <class Record> void retire(Slot<Record>& slot)
{
    slot.alive = false;
    slot.record = {};
    if (++slot.generation > slot_mask) slot.generation = 1;
}

bgfx::TextureFormat::Enum physical_format(TextureFormat format)
{
    switch (format) {
    case TextureFormat::rgba8: return bgfx::TextureFormat::RGBA8;
    case TextureFormat::bgra8: return bgfx::TextureFormat::BGRA8;
    case TextureFormat::bc1: return bgfx::TextureFormat::BC1;
    case TextureFormat::bc2: return bgfx::TextureFormat::BC2;
    case TextureFormat::bc3: return bgfx::TextureFormat::BC3;
    case TextureFormat::depth16: return bgfx::TextureFormat::D16;
    case TextureFormat::depth24_stencil8: return bgfx::TextureFormat::D24S8;
    case TextureFormat::depth32: return bgfx::TextureFormat::D32F;
    }
    return bgfx::TextureFormat::Unknown;
}

bool is_depth(TextureFormat format)
{
    return format == TextureFormat::depth16 || format == TextureFormat::depth24_stencil8
        || format == TextureFormat::depth32;
}

UInt32 clear_rgba(const std::array<float,4>& value)
{
    const auto channel = [](float component) { return static_cast<UInt32>(std::lround(component * 255.0f)); };
    return (channel(value[0]) << 24U) | (channel(value[1]) << 16U)
        | (channel(value[2]) << 8U) | channel(value[3]);
}

UInt64 required_texture_bytes(const TextureUploadDesc& upload, TextureFormat format)
{
    if (format == TextureFormat::rgba8 || format == TextureFormat::bgra8)
        return static_cast<UInt64>(upload.height) * upload.row_pitch;
    return 0; // Compressed and depth uploads are admitted only with later format-specific proof.
}

} // namespace

class BgfxGpuDevice::Impl {
public:
    struct BufferRecord { BufferDesc desc; std::vector<UInt8> shadow; };
    struct TextureRecord {
        TextureDesc desc;
        bgfx::TextureHandle native = BGFX_INVALID_HANDLE;
        bool color_initialized = false;
        bool depth_initialized = false;
        bool stencil_initialized = false;
    };
    struct SamplerRecord { SamplerDesc desc; };

    explicit Impl(BgfxOptions value) : options(std::move(value))
    {
        if (options.driver != "vulkan") throw std::invalid_argument("bgfx driver must be 'vulkan' for the Linux port");
        if (options.shader_root.empty()) throw std::invalid_argument("bgfx shader root must not be empty");
        bool expected = false;
        if (!runtime_owned.compare_exchange_strong(expected, true))
            throw std::runtime_error("bgfx runtime is already owned by another device");
        bgfx::Init init;
        init.type = bgfx::RendererType::Vulkan;
        init.fallback = false;
        init.swapChain.width = 0;
        init.swapChain.height = 0;
        init.debug = options.debug;
        if (!bgfx::init(init)) {
            runtime_owned.store(false);
            throw std::runtime_error("public bgfx Vulkan initialization failed");
        }
    }

    ~Impl()
    {
        if (bgfx::isValid(framebuffer)) bgfx::destroy(framebuffer);
        for (auto& slot : textures)
            if (slot.alive && bgfx::isValid(slot.record.native)) bgfx::destroy(slot.record.native);
        bgfx::frame();
        bgfx::shutdown();
        runtime_owned.store(false);
    }

    ValidationResult fail(std::string_view operation, std::string_view reason)
    {
        last_error = std::string(operation) + ": " + std::string(reason);
        return {false, last_error};
    }

    BgfxOptions options;
    std::string last_error;
    std::vector<Slot<BufferRecord>> buffers;
    std::vector<Slot<TextureRecord>> textures;
    std::vector<Slot<SamplerRecord>> samplers;
    bgfx::FrameBufferHandle framebuffer = BGFX_INVALID_HANDLE;
    std::array<TextureHandle, RendererLimits::color_targets> colors{};
    UInt32 color_count = 0;
    TextureHandle depth;
    UInt32 width = 0, height = 0;
    UInt64 target_generation = 0;
    UInt32 next_view = 0;
    bool in_pass = false;
};

BgfxGpuDevice::BgfxGpuDevice(BgfxOptions options) : impl_(std::make_unique<Impl>(std::move(options))) {}
BgfxGpuDevice::~BgfxGpuDevice() = default;

bool BgfxGpuDevice::supports_texture_format(TextureFormat format, TextureDimension dimension,
    bool sampled, bool render_target) const noexcept
{
    if (dimension != TextureDimension::texture_2d || (!sampled && !render_target)) return false;
    const auto native = physical_format(format);
    if (native == bgfx::TextureFormat::Unknown) return false;
    const UInt64 flags = render_target ? BGFX_TEXTURE_RT : BGFX_TEXTURE_NONE;
    return bgfx::isTextureValid(1, false, 1, native, flags);
}

BufferHandle BgfxGpuDevice::create_buffer(const BufferDesc& desc, std::string_view)
{
    if (auto result = validate(desc); !result) { impl_->fail("create_buffer", result.error); return {}; }
    // Vertex layout and index width are supplied only by the later draw. Keep
    // a bounded upload shadow and materialize the correctly typed native buffer
    // at the draw boundary instead of guessing its layout here.
    auto handle = insert<BufferHandle>(impl_->buffers, {desc, std::vector<UInt8>(static_cast<std::size_t>(desc.size))});
    if (!handle) impl_->fail("create_buffer", "opaque resource slot budget exhausted");
    return handle;
}

TextureHandle BgfxGpuDevice::create_texture(const TextureDesc& desc, std::string_view)
{
    if (auto result = validate(desc); !result) { impl_->fail("create_texture", result.error); return {}; }
    if (desc.dimension != TextureDimension::texture_2d || desc.width > UINT16_MAX || desc.height > UINT16_MAX
        || desc.mip_levels != 1 || !supports_texture_format(desc.format, desc.dimension, desc.sampled, desc.render_target)) {
        impl_->fail("create_texture", "unsupported public bgfx texture dimension, extent, mip or format/usage");
        return {};
    }
    const auto flags = desc.render_target ? BGFX_TEXTURE_RT : BGFX_TEXTURE_NONE;
    auto native = bgfx::createTexture2D(static_cast<UInt16>(desc.width), static_cast<UInt16>(desc.height),
        false, 1, physical_format(desc.format), flags);
    if (!bgfx::isValid(native)) { impl_->fail("create_texture", "public bgfx texture allocation failed"); return {}; }
    auto handle = insert<TextureHandle>(impl_->textures, {desc, native, false, false, false});
    if (!handle) { bgfx::destroy(native); impl_->fail("create_texture", "opaque resource slot budget exhausted"); }
    return handle;
}

SamplerHandle BgfxGpuDevice::create_sampler(const SamplerDesc& desc, std::string_view)
{
    if (auto result = validate(desc); !result) { impl_->fail("create_sampler", result.error); return {}; }
    // bgfx encodes samplers as validated flags at the setTexture binding edge.
    auto handle = insert<SamplerHandle>(impl_->samplers, {desc});
    if (!handle) impl_->fail("create_sampler", "opaque resource slot budget exhausted");
    return handle;
}

ValidationResult BgfxGpuDevice::upload(const UploadDesc& desc, const void* bytes)
{
    if (auto result = validate(desc); !result) return impl_->fail("upload", result.error);
    auto* slot = lookup(impl_->buffers, desc.destination);
    if (!slot) return impl_->fail("upload", "stale or foreign buffer handle");
    if (!bytes || desc.destination_size != slot->record.desc.size)
        return impl_->fail("upload", "null bytes or mismatched destination size");
    std::memcpy(slot->record.shadow.data() + desc.offset, bytes, static_cast<std::size_t>(desc.size));
    return {};
}

ValidationResult BgfxGpuDevice::upload_texture(const TextureUploadDesc& desc, const void* bytes)
{
    auto* slot = lookup(impl_->textures, desc.destination);
    if (!slot) return impl_->fail("upload_texture", "stale or foreign texture handle");
    const auto& texture = slot->record.desc;
    const UInt64 required = required_texture_bytes(desc, texture.format);
    if (!bytes || desc.mip_level != 0 || !desc.width || !desc.height
        || desc.width != texture.width || desc.height != texture.height
        || desc.row_pitch < static_cast<UInt64>(desc.width) * 4U || desc.row_pitch > UINT16_MAX
        || !required || desc.size < required || desc.size > RendererLimits::maximum_upload_bytes)
        return impl_->fail("upload_texture", "unsupported or out-of-bounds texture upload");
    bgfx::updateTexture2D(slot->record.native, 0, 0, 0, 0,
        static_cast<UInt16>(desc.width), static_cast<UInt16>(desc.height),
        bgfx::copy(bytes, static_cast<UInt32>(desc.size)), static_cast<UInt16>(desc.row_pitch));
    slot->record.color_initialized = true;
    return {};
}

void BgfxGpuDevice::destroy(BufferHandle handle)
{
    if (auto* slot = lookup(impl_->buffers, handle)) retire(*slot);
}
void BgfxGpuDevice::destroy(TextureHandle handle)
{
    if (auto* slot = lookup(impl_->textures, handle)) {
        if (impl_->in_pass && (handle == impl_->depth
            || std::find(impl_->colors.begin(), impl_->colors.begin() + impl_->color_count, handle)
                != impl_->colors.begin() + impl_->color_count)) {
            impl_->fail("destroy_texture", "active render attachment cannot be destroyed");
            return;
        }
        bgfx::destroy(slot->record.native); retire(*slot);
    }
}
void BgfxGpuDevice::destroy(SamplerHandle handle)
{
    if (auto* slot = lookup(impl_->samplers, handle)) retire(*slot);
}

// Later M30 slices install physical pass and draw behavior. These explicit
// failures prevent accidental success while the resource slice is reviewable.
ShaderHandle BgfxGpuDevice::create_shader(const ShaderDesc&, std::string_view)
{ impl_->fail("create_shader", "shader submission is not installed"); return {}; }
PipelineHandle BgfxGpuDevice::create_pipeline(const PipelineKey&, std::string_view)
{ impl_->fail("create_pipeline", "pipeline submission is not installed"); return {}; }
ValidationResult BgfxGpuDevice::begin_pass(const RenderPassDesc& desc, std::string_view)
{
    if (impl_->in_pass) return impl_->fail("begin_pass", "a pass is already active");
    if (auto result = validate(desc); !result) return impl_->fail("begin_pass", result.error);
    if (desc.width > UINT16_MAX || desc.height > UINT16_MAX || impl_->next_view >= RendererLimits::ordered_views)
        return impl_->fail("begin_pass", "target extent or ordered view budget exceeded");
    std::array<bgfx::TextureHandle, RendererLimits::color_targets + 1> attachments{};
    for (UInt32 i = 0; i < desc.color_target_count; ++i) {
        auto* color = lookup(impl_->textures, desc.color_targets[i]);
        if (!color || !color->record.desc.render_target || is_depth(color->record.desc.format)
            || color->record.desc.width != desc.width || color->record.desc.height != desc.height)
            return impl_->fail("begin_pass", "invalid color attachment or extent");
        if (desc.color_load == AttachmentLoad::load && !color->record.color_initialized)
            return impl_->fail("begin_pass", "color LOAD requires prior completed write");
        attachments[i] = color->record.native;
    }
    auto* depth = lookup(impl_->textures, desc.depth_target);
    if (!depth || !depth->record.desc.render_target || !is_depth(depth->record.desc.format)
        || depth->record.desc.width != desc.width || depth->record.desc.height != desc.height)
        return impl_->fail("begin_pass", "invalid depth attachment or extent");
    if (desc.depth_load == AttachmentLoad::load && (!depth->record.depth_initialized
        || (depth->record.desc.format == TextureFormat::depth24_stencil8 && !depth->record.stencil_initialized)))
        return impl_->fail("begin_pass", "depth LOAD requires prior completed write");
    attachments[desc.color_target_count] = depth->record.native;
    auto framebuffer = bgfx::createFrameBuffer(static_cast<UInt8>(desc.color_target_count + 1), attachments.data(), false);
    if (!bgfx::isValid(framebuffer)) return impl_->fail("begin_pass", "public bgfx framebuffer creation failed");
    const auto view = static_cast<bgfx::ViewId>(impl_->next_view++);
    bgfx::setViewFrameBuffer(view, framebuffer);
    bgfx::setViewRect(view, 0, 0, static_cast<UInt16>(desc.width), static_cast<UInt16>(desc.height));
    const UInt16 flags = (desc.color_load == AttachmentLoad::clear ? BGFX_CLEAR_COLOR : 0)
        | (desc.depth_load == AttachmentLoad::clear ? BGFX_CLEAR_DEPTH | BGFX_CLEAR_STENCIL : 0);
    bgfx::setViewClear(view, flags, clear_rgba(desc.clear_color), desc.clear_depth, 0);
    bgfx::touch(view);
    impl_->framebuffer = framebuffer;
    impl_->colors = desc.color_targets;
    impl_->color_count = desc.color_target_count;
    impl_->depth = desc.depth_target;
    impl_->width = desc.width;
    impl_->height = desc.height;
    impl_->target_generation = desc.target_generation;
    impl_->in_pass = true;
    if (desc.color_load == AttachmentLoad::clear)
        for (UInt32 i = 0; i < desc.color_target_count; ++i)
            lookup(impl_->textures, desc.color_targets[i])->record.color_initialized = true;
    if (desc.depth_load == AttachmentLoad::clear) {
        depth->record.depth_initialized = true;
        if (depth->record.desc.format == TextureFormat::depth24_stencil8)
            depth->record.stencil_initialized = true;
    }
    return {};
}

std::pair<UInt32,UInt32> BgfxGpuDevice::active_pass_extent() const noexcept
{ return impl_->in_pass ? std::pair<UInt32,UInt32>{impl_->width, impl_->height} : std::pair<UInt32,UInt32>{0,0}; }

ValidationResult BgfxGpuDevice::set_viewport(const ViewportDesc& desc)
{
    if (!impl_->in_pass) return impl_->fail("set_viewport", "no active pass");
    if (auto result = validate(desc, impl_->width, impl_->height); !result)
        return impl_->fail("set_viewport", result.error);
    if (std::trunc(desc.x) != desc.x || std::trunc(desc.y) != desc.y
        || std::trunc(desc.width) != desc.width || std::trunc(desc.height) != desc.height
        || desc.min_depth != 0.0f || desc.max_depth != 1.0f)
        return impl_->fail("set_viewport", "public bgfx view requires integer rectangle and full depth range");
    if (impl_->next_view >= RendererLimits::ordered_views)
        return impl_->fail("set_viewport", "ordered view budget exhausted");
    // View state is frame-global, not an immediate command. Mutating the
    // previous view would retroactively clip its full-target clear/draws.
    const auto view = static_cast<bgfx::ViewId>(impl_->next_view++);
    bgfx::setViewFrameBuffer(view, impl_->framebuffer);
    bgfx::setViewRect(view,
        static_cast<UInt16>(desc.x), static_cast<UInt16>(desc.y),
        static_cast<UInt16>(desc.width), static_cast<UInt16>(desc.height));
    bgfx::setViewClear(view, BGFX_CLEAR_NONE);
    return {};
}

ValidationResult BgfxGpuDevice::clear_viewport(const ViewportClearDesc& desc)
{
    if (!impl_->in_pass) return impl_->fail("clear_viewport", "no active pass");
    if (auto result = validate(desc, impl_->width, impl_->height); !result)
        return impl_->fail("clear_viewport", result.error);
    if (desc.target_generation != impl_->target_generation
        || (desc.color && (impl_->color_count != 1 || desc.color_target != impl_->colors[0]))
        || ((desc.depth || desc.stencil) && desc.depth_target != impl_->depth))
        return impl_->fail("clear_viewport", "stale generation, attachment or unsupported MRT selection");
    if (desc.stencil && lookup(impl_->textures, impl_->depth)->record.desc.format != TextureFormat::depth24_stencil8)
        return impl_->fail("clear_viewport", "stencil clear requires D24S8");
    if (impl_->next_view >= RendererLimits::ordered_views)
        return impl_->fail("clear_viewport", "ordered view budget exhausted");
    const auto left = std::max<Int32>(0, desc.x);
    const auto top = std::max<Int32>(0, desc.y);
    const auto right = std::min<std::int64_t>(impl_->width, static_cast<std::int64_t>(desc.x) + desc.width);
    const auto bottom = std::min<std::int64_t>(impl_->height, static_cast<std::int64_t>(desc.y) + desc.height);
    const auto view = static_cast<bgfx::ViewId>(impl_->next_view++);
    bgfx::setViewFrameBuffer(view, impl_->framebuffer);
    bgfx::setViewRect(view, static_cast<UInt16>(left), static_cast<UInt16>(top),
        static_cast<UInt16>(right - left), static_cast<UInt16>(bottom - top));
    const UInt16 flags = (desc.color ? BGFX_CLEAR_COLOR : 0) | (desc.depth ? BGFX_CLEAR_DEPTH : 0)
        | (desc.stencil ? BGFX_CLEAR_STENCIL : 0);
    bgfx::setViewClear(view, flags, clear_rgba(desc.color_value), desc.depth_value, desc.stencil_value);
    bgfx::touch(view);
    if (desc.color) lookup(impl_->textures, impl_->colors[0])->record.color_initialized = true;
    if (desc.depth) lookup(impl_->textures, impl_->depth)->record.depth_initialized = true;
    if (desc.stencil) lookup(impl_->textures, impl_->depth)->record.stencil_initialized = true;
    return {};
}
ValidationResult BgfxGpuDevice::draw(const DrawDesc&)
{ return impl_->fail("draw", "shader draw submission is not installed"); }
ValidationResult BgfxGpuDevice::end_pass()
{
    if (!impl_->in_pass) return impl_->fail("end_pass", "no active pass");
    bgfx::destroy(impl_->framebuffer);
    impl_->framebuffer = BGFX_INVALID_HANDLE;
    impl_->in_pass = false;
    impl_->color_count = 0;
    impl_->colors = {};
    impl_->depth = {};
    impl_->width = impl_->height = 0;
    impl_->target_generation = 0;
    return {};
}
ValidationResult BgfxGpuDevice::present(TextureHandle)
{ return impl_->fail("present", "window presentation is not installed"); }
void BgfxGpuDevice::destroy(ShaderHandle) {}
void BgfxGpuDevice::destroy(PipelineHandle) {}
const std::string& BgfxGpuDevice::last_error() const noexcept { return impl_->last_error; }
bool BgfxGpuDevice::pass_active() const noexcept { return impl_->in_pass; }
void BgfxGpuDevice::record_marker(std::string_view) {}
ValidationResult BgfxGpuDevice::claim_window(SDL_Window*)
{ return impl_->fail("claim_window", "window presentation is not installed"); }
ValidationResult BgfxGpuDevice::wait_idle()
{
    if (impl_->in_pass) return impl_->fail("wait_idle", "pass remains active");
    bgfx::frame();
    impl_->next_view = 0;
    return {};
}

std::vector<UInt8> BgfxGpuDevice::readback_rgba(TextureHandle source)
{
    if (impl_->in_pass) { impl_->fail("readback_rgba", "pass remains active"); return {}; }
    auto* slot = lookup(impl_->textures, source);
    if (!slot || !slot->record.color_initialized || is_depth(slot->record.desc.format)
        || impl_->next_view >= RendererLimits::ordered_views) {
        impl_->fail("readback_rgba", "invalid/uninitialized color target or view budget exhausted");
        return {};
    }
    const auto& desc = slot->record.desc;
    if (static_cast<UInt64>(desc.width) * desc.height * 4U > RendererLimits::maximum_upload_bytes) {
        impl_->fail("readback_rgba", "diagnostic readback exceeds byte limit"); return {};
    }
    if (desc.format != TextureFormat::rgba8 && desc.format != TextureFormat::bgra8) {
        impl_->fail("readback_rgba", "unsupported color readback format"); return {};
    }
    auto target = bgfx::createTexture2D(static_cast<UInt16>(desc.width), static_cast<UInt16>(desc.height),
        false, 1, physical_format(desc.format), BGFX_TEXTURE_BLIT_DST | BGFX_TEXTURE_READ_BACK);
    if (!bgfx::isValid(target)) { impl_->fail("readback_rgba", "readback texture allocation failed"); return {}; }
    bgfx::TextureRegion destination{};
    destination.handle = target;
    bgfx::TextureRegion origin{};
    origin.handle = slot->record.native;
    bgfx::blit(static_cast<bgfx::ViewId>(impl_->next_view++), destination, origin);
    auto current = bgfx::frame();
    std::vector<UInt8> pixels(static_cast<std::size_t>(desc.width) * desc.height * 4U);
    const auto ready = bgfx::read(destination, pixels.data());
    while (current < ready) current = bgfx::frame();
    bgfx::destroy(target);
    impl_->next_view = 0;
    if (desc.format == TextureFormat::bgra8)
        for (std::size_t i = 0; i < pixels.size(); i += 4) std::swap(pixels[i], pixels[i + 2]);
    return pixels;
}
void BgfxGpuDevice::release_window() noexcept {}

} // namespace zh::renderer
