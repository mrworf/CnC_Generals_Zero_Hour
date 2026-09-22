#include "zh/platform/bgfx_device.h"

#include <bgfx/bgfx.h>
#include <SDL3/SDL.h>

#include <algorithm>
#include <atomic>
#include <cmath>
#include <cstring>
#include <fstream>
#include <limits>
#include <map>
#include <regex>
#include <set>
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
    case TextureFormat::bgr5a1: return bgfx::TextureFormat::BGR5A1;
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
    if (format == TextureFormat::rgba8 || format == TextureFormat::bgra8 ||
        format == TextureFormat::bgr5a1)
        return static_cast<UInt64>(upload.height) * upload.row_pitch;
    return 0; // Compressed and depth uploads are admitted only with later format-specific proof.
}

bool vertex_layout_for(const PipelineDesc& desc, bgfx::VertexLayout& layout, UInt32& stride)
{
    layout.begin();
    switch (desc.vertex_layout) {
    case VertexLayout::position_color_uv:
        layout.add(bgfx::Attrib::Position, 2, bgfx::AttribType::Float)
            .add(bgfx::Attrib::Color0, 4, bgfx::AttribType::Uint8, true)
            .add(bgfx::Attrib::TexCoord0, 2, bgfx::AttribType::Float);
        stride = 20;
        break;
    case VertexLayout::point_sprite:
        layout.add(bgfx::Attrib::Position, 3, bgfx::AttribType::Float)
            .add(bgfx::Attrib::Color0, 4, bgfx::AttribType::Uint8, true);
        stride = 16;
        break;
    case VertexLayout::world_mesh:
    case VertexLayout::terrain:
    case VertexLayout::wwshade:
        layout.add(bgfx::Attrib::Position, 3, bgfx::AttribType::Float)
            .add(bgfx::Attrib::Normal, 3, bgfx::AttribType::Float)
            .add(bgfx::Attrib::TexCoord0, 2, bgfx::AttribType::Float);
        stride = 32;
        break;
    case VertexLayout::water:
        layout.add(bgfx::Attrib::Position, 3, bgfx::AttribType::Float)
            .add(bgfx::Attrib::TexCoord0, 2, bgfx::AttribType::Float);
        stride = 20;
        break;
    case VertexLayout::original_fvf:
    {
        const auto& source = desc.original_fvf;
        std::vector<VertexAttributeDesc> attributes(source.attributes.begin(),
            source.attributes.begin() + source.attribute_count);
        std::sort(attributes.begin(), attributes.end(), [](const auto& a, const auto& b) {
            return a.offset < b.offset;
        });
        UInt32 cursor = 0;
        for (const auto& attribute : attributes) {
            if (attribute.offset < cursor || attribute.offset - cursor > UINT8_MAX) return false;
            if (attribute.offset != cursor) layout.skip(static_cast<UInt8>(attribute.offset - cursor));
            bgfx::Attrib::Enum semantic;
            switch (attribute.location) {
            case 0: semantic = bgfx::Attrib::Position; break;
            case 1: semantic = bgfx::Attrib::Normal; break;
            case 2: semantic = bgfx::Attrib::Color0; break;
            case 3: semantic = bgfx::Attrib::Color1; break;
            case 4: semantic = bgfx::Attrib::TexCoord0; break;
            case 5: semantic = bgfx::Attrib::TexCoord1; break;
            default: return false;
            }
            const bool bytes = attribute.format == VertexElementFormat::ubyte4_norm;
            const UInt8 count = bytes ? 4 : static_cast<UInt8>(attribute.format) + 1;
            layout.add(semantic, count, bytes ? bgfx::AttribType::Uint8 : bgfx::AttribType::Float, bytes);
            cursor = attribute.offset + (bytes ? 4 : count * 4);
        }
        if (source.stride < cursor || source.stride - cursor > UINT8_MAX) return false;
        if (source.stride != cursor) layout.skip(static_cast<UInt8>(source.stride - cursor));
        stride = source.stride;
        break;
    }
    }
    layout.end();
    return layout.getStride() == stride;
}

bool range_written(const std::vector<UInt8>& written, UInt64 offset, UInt64 size)
{
    return offset <= written.size() && size <= written.size() - offset
        && std::all_of(written.begin() + static_cast<std::size_t>(offset),
            written.begin() + static_cast<std::size_t>(offset + size), [](UInt8 byte) { return byte != 0; });
}

UInt64 depth_compare_state(CompareOp op)
{
    switch (op) {
    case CompareOp::never: return BGFX_STATE_DEPTH_TEST_NEVER;
    case CompareOp::less: return BGFX_STATE_DEPTH_TEST_LESS;
    case CompareOp::equal: return BGFX_STATE_DEPTH_TEST_EQUAL;
    case CompareOp::less_equal: return BGFX_STATE_DEPTH_TEST_LEQUAL;
    case CompareOp::greater: return BGFX_STATE_DEPTH_TEST_GREATER;
    case CompareOp::not_equal: return BGFX_STATE_DEPTH_TEST_NOTEQUAL;
    case CompareOp::greater_equal: return BGFX_STATE_DEPTH_TEST_GEQUAL;
    case CompareOp::always: return BGFX_STATE_DEPTH_TEST_ALWAYS;
    }
    return BGFX_STATE_DEPTH_TEST_NEVER;
}

UInt32 stencil_compare_state(CompareOp op)
{
    switch (op) {
    case CompareOp::never: return BGFX_STENCIL_TEST_NEVER;
    case CompareOp::less: return BGFX_STENCIL_TEST_LESS;
    case CompareOp::equal: return BGFX_STENCIL_TEST_EQUAL;
    case CompareOp::less_equal: return BGFX_STENCIL_TEST_LEQUAL;
    case CompareOp::greater: return BGFX_STENCIL_TEST_GREATER;
    case CompareOp::not_equal: return BGFX_STENCIL_TEST_NOTEQUAL;
    case CompareOp::greater_equal: return BGFX_STENCIL_TEST_GEQUAL;
    case CompareOp::always: return BGFX_STENCIL_TEST_ALWAYS;
    }
    return BGFX_STENCIL_TEST_NEVER;
}

UInt32 stencil_op_code(StencilOp op)
{
    switch (op) {
    case StencilOp::zero: return 0;
    case StencilOp::keep: return 1;
    case StencilOp::replace: return 2;
    case StencilOp::increment_wrap: return 3;
    case StencilOp::increment_clamp: return 4;
    case StencilOp::decrement_wrap: return 5;
    case StencilOp::decrement_clamp: return 6;
    case StencilOp::invert: return 7;
    }
    return 1;
}

struct UniformMetadata { std::string name; UInt16 offset = 0; };

bool normalize_shader_uniform_identifiers(std::vector<char>& data, std::vector<UniformMetadata>& metadata)
{
    // M29's glslang reflection names include '.' for nested UBO
    // members. They are valid reflection paths but bgfx::createUniform's
    // public identifier grammar rejects them. The container header names are
    // CPU metadata; SPIR-V payload, offsets and manifest remain unchanged.
    if (data.size() < 22) return false;
    std::size_t position = 20;
    const UInt32 count = static_cast<UInt8>(data[position])
        | (static_cast<UInt32>(static_cast<UInt8>(data[position + 1])) << 8U);
    position += 2;
    if (count > 1024) return false;
    std::set<std::string> identifiers;
    const auto alpha = [](char c) { return (c >= 'A' && c <= 'Z') || (c >= 'a' && c <= 'z'); };
    const auto digit = [](char c) { return c >= '0' && c <= '9'; };
    for (UInt32 i = 0; i < count; ++i) {
        if (position >= data.size()) return false;
        const auto length = static_cast<UInt8>(data[position++]);
        if (!length || position + length + 10 > data.size()) return false;
        for (std::size_t j = 0; j < length; ++j) {
            char& c = data[position + j];
            if (j == 0 && !alpha(c) && c != '_') return false;
            if (j != 0 && c == '.') c = '_';
            else if (j != 0 && !alpha(c) && !digit(c) && c != '_') return false;
        }
        std::string name(data.data() + position, length);
        if (!identifiers.emplace(name).second) return false;
        const std::size_t header = position + length;
        const auto offset = static_cast<UInt16>(static_cast<UInt8>(data[header + 2])
            | (static_cast<UInt16>(static_cast<UInt8>(data[header + 3])) << 8U));
        metadata.push_back({std::move(name), offset});
        position += length + 10;
    }
    return position + 4 <= data.size();
}

struct ShaderManifest {
    ShaderStage stage = ShaderStage::vertex;
    std::map<std::string, UInt32> block_binding;
    std::map<std::string, std::pair<UInt32, UInt32>> texture_binding;
};

bool read_shader_manifest(const std::filesystem::path& path, ShaderManifest& manifest)
{
    std::ifstream input(path, std::ios::binary | std::ios::ate);
    if (!input || input.tellg() <= 0 || input.tellg() > 65536) return false;
    std::string contents(static_cast<std::size_t>(input.tellg()), '\0');
    input.seekg(0); input.read(contents.data(), static_cast<std::streamsize>(contents.size()));
    if (!input || contents.find("\"uniform_blocks\"") == std::string::npos
        || contents.find("\"textures\"") == std::string::npos) return false;
    if (contents.find("\"stage\":\"vertex\"") != std::string::npos) manifest.stage = ShaderStage::vertex;
    else if (contents.find("\"stage\":\"fragment\"") != std::string::npos) manifest.stage = ShaderStage::fragment;
    else return false;
    const std::regex block("\\\"instance\\\":\\\"([^\\\"]+)\\\",\\\"source_binding\\\":([0-3])");
    const std::regex texture("\\\"bgfx_stage\\\":([0-9]+),\\\"kind\\\":\\\"[^\\\"]+\\\",\\\"name\\\":\\\"([^\\\"]+)\\\",\\\"source_binding\\\":([0-9]+)");
    for (std::sregex_iterator it(contents.begin(), contents.end(), block), end; it != end; ++it)
        if (!manifest.block_binding.emplace((*it)[1], static_cast<UInt32>(std::stoul((*it)[2]))).second) return false;
    for (std::sregex_iterator it(contents.begin(), contents.end(), texture), end; it != end; ++it)
        if (!manifest.texture_binding.emplace((*it)[2], std::pair<UInt32, UInt32>{
                static_cast<UInt32>(std::stoul((*it)[3])), static_cast<UInt32>(std::stoul((*it)[1]))}).second)
            return false;
    return manifest.block_binding.size() <= RendererLimits::uniform_buffers_per_stage
        && manifest.texture_binding.size() <= RendererLimits::sampled_textures_per_stage;
}

UInt32 sampler_flags(const SamplerDesc& desc)
{
    const auto address = [](AddressMode mode, UInt32 mirror, UInt32 clamp, UInt32 border) {
        switch (mode) {
        case AddressMode::repeat: return UInt32{0};
        case AddressMode::mirrored_repeat: return mirror;
        case AddressMode::clamp_edge: return clamp;
        case AddressMode::clamp_border: return border;
        }
        return UInt32{0};
    };
    UInt32 flags = address(desc.address_u, BGFX_SAMPLER_U_MIRROR, BGFX_SAMPLER_U_CLAMP, BGFX_SAMPLER_U_BORDER)
        | address(desc.address_v, BGFX_SAMPLER_V_MIRROR, BGFX_SAMPLER_V_CLAMP, BGFX_SAMPLER_V_BORDER)
        | address(desc.address_w, BGFX_SAMPLER_W_MIRROR, BGFX_SAMPLER_W_CLAMP, BGFX_SAMPLER_W_BORDER);
    if (desc.maximum_anisotropy > 1) flags |= BGFX_SAMPLER_MIN_ANISOTROPIC | BGFX_SAMPLER_MAG_ANISOTROPIC;
    else {
        if (desc.min_filter == Filter::nearest) flags |= BGFX_SAMPLER_MIN_POINT;
        if (desc.mag_filter == Filter::nearest) flags |= BGFX_SAMPLER_MAG_POINT;
    }
    if (desc.mip_filter == Filter::nearest) flags |= BGFX_SAMPLER_MIP_POINT;
    return flags;
}

UInt64 blend_factor(BlendFactor factor)
{
    switch (factor) {
    case BlendFactor::zero: return BGFX_STATE_BLEND_ZERO;
    case BlendFactor::one: return BGFX_STATE_BLEND_ONE;
    case BlendFactor::src_color: return BGFX_STATE_BLEND_SRC_COLOR;
    case BlendFactor::inv_src_color: return BGFX_STATE_BLEND_INV_SRC_COLOR;
    case BlendFactor::src_alpha: return BGFX_STATE_BLEND_SRC_ALPHA;
    case BlendFactor::inv_src_alpha: return BGFX_STATE_BLEND_INV_SRC_ALPHA;
    case BlendFactor::dst_color: return BGFX_STATE_BLEND_DST_COLOR;
    case BlendFactor::inv_dst_color: return BGFX_STATE_BLEND_INV_DST_COLOR;
    case BlendFactor::dst_alpha: return BGFX_STATE_BLEND_DST_ALPHA;
    case BlendFactor::inv_dst_alpha: return BGFX_STATE_BLEND_INV_DST_ALPHA;
    case BlendFactor::src_alpha_saturate: return BGFX_STATE_BLEND_SRC_ALPHA_SAT;
    }
    return 0;
}

UInt64 blend_operation(BlendOp operation)
{
    switch (operation) {
    case BlendOp::add: return BGFX_STATE_BLEND_EQUATION_ADD;
    case BlendOp::subtract: return BGFX_STATE_BLEND_EQUATION_SUB;
    case BlendOp::reverse_subtract: return BGFX_STATE_BLEND_EQUATION_REVSUB;
    case BlendOp::minimum: return BGFX_STATE_BLEND_EQUATION_MIN;
    case BlendOp::maximum: return BGFX_STATE_BLEND_EQUATION_MAX;
    }
    return 0;
}

} // namespace

class BgfxGpuDevice::Impl {
public:
    struct BufferRecord { BufferDesc desc; std::vector<UInt8> shadow; std::vector<UInt8> written; };
    struct TextureRecord {
        TextureDesc desc;
        bgfx::TextureHandle native = BGFX_INVALID_HANDLE;
        bool color_initialized = false;
        bool depth_initialized = false;
        bool stencil_initialized = false;
    };
    struct SamplerRecord { SamplerDesc desc; };
    struct ShaderRecord {
        struct Field { bgfx::UniformHandle handle = BGFX_INVALID_HANDLE; UInt32 binding = 0, offset = 0, size = 0, count = 0; };
        struct TextureField { bgfx::UniformHandle handle = BGFX_INVALID_HANDLE; UInt32 binding = 0, stage = 0; };
        ShaderStage stage{};
        UInt32 uniforms = 0, samplers = 0;
        bgfx::ShaderHandle native = BGFX_INVALID_HANDLE;
        std::vector<Field> fields;
        std::vector<TextureField> textures;
    };
    struct PipelineRecord { PipelineRecord() : key(PipelineDesc{}) {} PipelineKey key; bgfx::ProgramHandle native = BGFX_INVALID_HANDLE; };

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
        if (const char* driver = SDL_GetCurrentVideoDriver(); driver && std::strcmp(driver, "wayland") == 0)
            init.platformData.type = bgfx::NativeWindowHandleType::Wayland;
        native_type = init.platformData.type;
        if (!bgfx::init(init)) {
            runtime_owned.store(false);
            throw std::runtime_error("public bgfx Vulkan initialization failed");
        }
    }

    ~Impl()
    {
        if (bgfx::isValid(framebuffer)) bgfx::destroy(framebuffer);
        if (bgfx::isValid(window_framebuffer)) bgfx::destroy(window_framebuffer);
        if (bgfx::isValid(present_program)) bgfx::destroy(present_program);
        for (auto& slot : pipelines)
            if (slot.alive && bgfx::isValid(slot.record.native)) bgfx::destroy(slot.record.native);
        for (auto& slot : shaders)
            if (slot.alive && bgfx::isValid(slot.record.native)) bgfx::destroy(slot.record.native);
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

    bool query_window_pixels(SDL_Window* window, int* width, int* height) const
    {
        return options.pixel_extent_query
            ? options.pixel_extent_query(window, width, height)
            : SDL_GetWindowSizeInPixels(window, width, height);
    }

    BgfxOptions options;
    std::string last_error;
    std::vector<Slot<BufferRecord>> buffers;
    std::vector<Slot<TextureRecord>> textures;
    std::vector<Slot<SamplerRecord>> samplers;
    std::vector<Slot<ShaderRecord>> shaders;
    std::vector<Slot<PipelineRecord>> pipelines;
    bgfx::FrameBufferHandle framebuffer = BGFX_INVALID_HANDLE;
    std::array<TextureHandle, RendererLimits::color_targets> colors{};
    UInt32 color_count = 0;
    TextureHandle depth;
    UInt32 width = 0, height = 0;
    UInt64 target_generation = 0;
    UInt32 next_view = 0;
    bool in_pass = false;
    SDL_Window* window = nullptr;
    bgfx::FrameBufferHandle window_framebuffer = BGFX_INVALID_HANDLE;
    bgfx::ProgramHandle present_program = BGFX_INVALID_HANDLE;
    ShaderHandle present_vertex;
    ShaderHandle present_fragment;
    bgfx::UniformHandle present_viewport = BGFX_INVALID_HANDLE;
    bgfx::UniformHandle present_texture = BGFX_INVALID_HANDLE;
    UInt32 window_width = 0, window_height = 0;
    void* window_nwh = nullptr;
    void* window_ndt = nullptr;
    bgfx::NativeWindowHandleType::Enum native_type = bgfx::NativeWindowHandleType::Default;
};

BgfxGpuDevice::BgfxGpuDevice(BgfxOptions options) : impl_(std::make_unique<Impl>(std::move(options))) {}
BgfxGpuDevice::~BgfxGpuDevice() = default;

bool BgfxGpuDevice::supports_texture_format(TextureFormat format, TextureDimension dimension,
    bool sampled, bool render_target) const noexcept
{
    if (format == TextureFormat::bgr5a1 && render_target) return false;
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
    auto handle = insert<BufferHandle>(impl_->buffers, {desc,
        std::vector<UInt8>(static_cast<std::size_t>(desc.size)),
        std::vector<UInt8>(static_cast<std::size_t>(desc.size))});
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

std::optional<TextureFormat> BgfxGpuDevice::describe_texture_format(TextureHandle handle) const noexcept
{
    const auto* slot=lookup(impl_->textures,handle);
    return slot ? std::optional<TextureFormat>(slot->record.desc.format) : std::nullopt;
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
    std::fill(slot->record.written.begin() + static_cast<std::size_t>(desc.offset),
        slot->record.written.begin() + static_cast<std::size_t>(desc.offset + desc.size), 1);
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
        || desc.row_pitch < static_cast<UInt64>(desc.width) *
            (texture.format == TextureFormat::bgr5a1 ? 2U : 4U) || desc.row_pitch > UINT16_MAX
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

ShaderHandle BgfxGpuDevice::create_shader(const ShaderDesc& desc, std::string_view label)
{
    if (auto result = validate(desc); !result) { impl_->fail("create_shader", result.error); return {}; }
    const std::filesystem::path relative(desc.name);
    if (label.empty() || relative.empty() || relative.is_absolute()) {
        impl_->fail("create_shader", "label and project-relative shader name are required"); return {};
    }
    for (const auto& part : relative)
        if (part == "..") { impl_->fail("create_shader", "shader path escapes owned root"); return {}; }
    const auto path = impl_->options.shader_root / (relative.string() + ".bin");
    std::ifstream input(path, std::ios::binary | std::ios::ate);
    if (!input || input.tellg() < 28 || input.tellg() > 4 * 1024 * 1024) {
        impl_->fail("create_shader", "missing, empty or oversized pinned bgfx shader binary"); return {};
    }
    std::vector<char> data(static_cast<std::size_t>(input.tellg()));
    input.seekg(0);
    input.read(data.data(), static_cast<std::streamsize>(data.size()));
    const char* expected = desc.stage == ShaderStage::vertex ? "VSH" : "FSH";
    if (!input || std::memcmp(data.data(), expected, 3) != 0 || static_cast<unsigned char>(data[3]) != 12) {
        impl_->fail("create_shader", "wrong bgfx shader stage/version envelope"); return {};
    }
    std::vector<UniformMetadata> metadata;
    if (!normalize_shader_uniform_identifiers(data, metadata)) {
        impl_->fail("create_shader", "invalid or colliding reflected shader uniform identifiers"); return {};
    }
    ShaderManifest manifest;
    if (!read_shader_manifest(impl_->options.shader_root / (relative.string() + ".json"), manifest)
        || manifest.stage != desc.stage) {
        impl_->fail("create_shader", "missing or invalid project-owned bgfx shader manifest"); return {};
    }
    auto native = bgfx::createShader(bgfx::copy(data.data(), static_cast<UInt32>(data.size())));
    if (!bgfx::isValid(native)) { impl_->fail("create_shader", "public bgfx shader creation failed"); return {}; }
    const auto uniform_count = bgfx::getShaderUniforms(native);
    std::vector<bgfx::UniformHandle> uniforms(uniform_count);
    bgfx::getShaderUniforms(native, uniforms.data(), uniform_count);
    for (const auto uniform : uniforms) {
        if (!bgfx::isValid(uniform)) {
            bgfx::destroy(native);
            impl_->fail("create_shader", "public bgfx rejected a reflected uniform identifier");
            return {};
        }
    }
    Impl::ShaderRecord record;
    record.stage = desc.stage; record.uniforms = desc.uniform_buffers;
    record.samplers = desc.samplers; record.native = native;
    std::map<std::string, bgfx::UniformHandle> by_name;
    for (const auto uniform : uniforms) {
        bgfx::UniformInfo info;
        bgfx::getUniformInfo(uniform, info);
        by_name.emplace(info.name, uniform);
    }
    for (const auto& item : metadata) {
        const auto found = by_name.find(item.name);
        if (found == by_name.end()) continue; // Optimized-out or non-public metadata.
        bgfx::UniformInfo info;
        bgfx::getUniformInfo(found->second, info);
        bool classified = false;
        for (const auto& block : manifest.block_binding) {
            const auto prefix = std::string("ZhStageUniforms_") + block.first + "_";
            if (item.name.rfind(prefix, 0) != 0) continue;
            UInt32 size = 0;
            switch (info.type) {
            case bgfx::UniformType::Vec4: size = 16; break;
            case bgfx::UniformType::Mat3: size = 48; break;
            case bgfx::UniformType::Mat4: size = 64; break;
            default: break;
            }
            if (!size || block.second >= desc.uniform_buffers) {
                bgfx::destroy(native); impl_->fail("create_shader", "reflected block type/binding mismatch"); return {};
            }
            record.fields.push_back({found->second, block.second, item.offset, size * info.num, info.num});
            classified = true;
            break;
        }
        if (classified) continue;
        for (const auto& texture : manifest.texture_binding) {
            if (item.name != texture.first + "_image") continue;
            if (info.type != bgfx::UniformType::Sampler || texture.second.first >= desc.samplers
                || texture.second.second >= RendererLimits::sampled_textures_per_stage) {
                bgfx::destroy(native); impl_->fail("create_shader", "reflected texture type/binding mismatch"); return {};
            }
            record.textures.push_back({found->second, texture.second.first, texture.second.second});
            classified = true;
            break;
        }
        if (!classified) {
            bgfx::destroy(native); impl_->fail("create_shader", "unclassified public bgfx uniform: " + item.name); return {};
        }
    }
    std::map<UInt32, UInt32> block_base;
    for (const auto& field : record.fields) {
        auto it = block_base.find(field.binding);
        if (it == block_base.end() || field.offset < it->second) block_base[field.binding] = field.offset;
    }
    for (auto& field : record.fields) field.offset -= block_base[field.binding];
    auto handle = insert<ShaderHandle>(impl_->shaders, std::move(record));
    if (!handle) { bgfx::destroy(native); impl_->fail("create_shader", "opaque shader slot budget exhausted"); }
    return handle;
}

PipelineHandle BgfxGpuDevice::create_pipeline(const PipelineKey& key, std::string_view label)
{
    const auto& desc = key.descriptor();
    if (auto result = validate(desc); !result) { impl_->fail("create_pipeline", result.error); return {}; }
    if (label.empty()) { impl_->fail("create_pipeline", "label must not be empty"); return {}; }
    for (std::size_t i = 0; i < impl_->pipelines.size(); ++i)
        if (impl_->pipelines[i].alive && impl_->pipelines[i].record.key == key)
            return encode<PipelineHandle>(i, impl_->pipelines[i].generation);
    auto* vertex = lookup(impl_->shaders, desc.vertex_shader);
    auto* fragment = lookup(impl_->shaders, desc.fragment_shader);
    if (!vertex || vertex->record.stage != ShaderStage::vertex
        || !fragment || fragment->record.stage != ShaderStage::fragment) {
        impl_->fail("create_pipeline", "stale or wrong-stage shader handles"); return {};
    }
    if (desc.topology == PrimitiveTopology::triangle_fan || desc.raster.fill == FillMode::point) {
        impl_->fail("create_pipeline", "unsupported primitive/fill state requires source-side expansion"); return {};
    }
    auto native = bgfx::createProgram(vertex->record.native, fragment->record.native, false);
    if (!bgfx::isValid(native)) { impl_->fail("create_pipeline", "public bgfx shader program creation failed"); return {}; }
    Impl::PipelineRecord record;
    record.key = key; record.native = native;
    auto handle = insert<PipelineHandle>(impl_->pipelines, std::move(record));
    if (!handle) { bgfx::destroy(native); impl_->fail("create_pipeline", "opaque pipeline slot budget exhausted"); }
    return handle;
}
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
    bgfx::setViewMode(view, bgfx::ViewMode::Sequential);
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
    bgfx::setViewMode(view, bgfx::ViewMode::Sequential);
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
    bgfx::setViewMode(view, bgfx::ViewMode::Sequential);
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
ValidationResult BgfxGpuDevice::draw(const DrawDesc& desc)
{
    if (!impl_->in_pass) return impl_->fail("draw", "draw requires an active pass");
    auto* pipeline = lookup(impl_->pipelines, desc.pipeline);
    if (!pipeline) return impl_->fail("draw", "stale or foreign pipeline handle");
    const auto& state = pipeline->record.key.descriptor();
    if (auto result = validate(desc, state); !result) return impl_->fail("draw", result.error);
    if (impl_->color_count != 1 || !lookup(impl_->textures, impl_->colors[0])
        || lookup(impl_->textures, impl_->colors[0])->record.desc.format != state.color_format
        || lookup(impl_->textures, impl_->depth)->record.desc.format != state.depth_format)
        return impl_->fail("draw", "pipeline and active target formats differ");
    if (state.raster.fill != FillMode::solid ||
        (state.raster.depth_bias != 0.0f && state.raster.depth_bias != -8.0f)
        || state.topology == PrimitiveTopology::triangle_fan)
        return impl_->fail("draw", "required pipeline or binding state is not yet mapped to public bgfx");
    if (state.topology == PrimitiveTopology::point_list
        && (desc.point_size > 15.0f || std::floor(desc.point_size) != desc.point_size))
        return impl_->fail("draw", "public bgfx point size must be an integer in 1..15");
    auto* vertex_shader = lookup(impl_->shaders, state.vertex_shader);
    auto* fragment_shader = lookup(impl_->shaders, state.fragment_shader);
    if (!vertex_shader || !fragment_shader)
        return impl_->fail("draw", "pipeline shader was destroyed");
    const auto validate_stage = [&](const Impl::ShaderRecord& shader, const StageBindings& bindings) -> ValidationResult {
        if (bindings.uniform_count != shader.uniforms || bindings.texture_count != shader.samplers)
            return impl_->fail("draw", "stage binding counts differ from declared shader resources");
        for (const auto& field : shader.fields) {
            if (field.binding >= bindings.uniform_count)
                return impl_->fail("draw", "reflected uniform binding exceeds stage inputs");
            const auto& binding = bindings.uniforms[field.binding];
            auto* buffer = lookup(impl_->buffers, binding.buffer);
            if (!buffer || buffer->record.desc.usage != BufferUsage::uniform
                || field.offset > binding.size || field.size > binding.size - field.offset
                || !range_written(buffer->record.written, binding.offset + field.offset, field.size))
                return impl_->fail("draw", "missing, stale or uninitialized reflected uniform field");
        }
        for (const auto& texture : shader.textures) {
            if (texture.binding >= bindings.texture_count)
                return impl_->fail("draw", "reflected texture binding exceeds stage inputs");
            auto* image = lookup(impl_->textures, bindings.textures[texture.binding]);
            auto* sampler = lookup(impl_->samplers, bindings.samplers[texture.binding]);
            if (!image || !image->record.desc.sampled || !image->record.color_initialized || !sampler
                || (sampler->record.desc.maximum_lod != 1000.0f
                    && !(sampler->record.desc.maximum_lod == 0.0f
                        && image->record.desc.mip_levels == 1)))
                return impl_->fail("draw", "missing/stale texture, sampler or unsupported sampler LOD");
        }
        return {};
    };
    if (auto result = validate_stage(vertex_shader->record, desc.vertex_bindings); !result) return result;
    if (auto result = validate_stage(fragment_shader->record, desc.fragment_bindings); !result) return result;
    auto* vertex = lookup(impl_->buffers, desc.vertex_buffer);
    if (!vertex || vertex->record.desc.usage != BufferUsage::vertex)
        return impl_->fail("draw", "stale or wrong-usage vertex buffer");
    bgfx::VertexLayout layout;
    UInt32 stride = 0;
    if (!vertex_layout_for(state, layout, stride) || !stride || vertex->record.shadow.size() % stride)
        return impl_->fail("draw", "unsupported or misaligned vertex layout");
    UInt64 start_vertex = 0;
    if (desc.base_vertex < 0) return impl_->fail("draw", "negative base vertex is not representable by public bgfx binding");
    start_vertex = static_cast<UInt32>(desc.base_vertex);
    const auto available_vertices = vertex->record.shadow.size() / stride;
    auto* index = desc.index_buffer ? lookup(impl_->buffers, desc.index_buffer) : nullptr;
    if (desc.index_buffer && (!index || index->record.desc.usage != BufferUsage::index))
        return impl_->fail("draw", "stale or wrong-usage index buffer");
    if (index) {
        const UInt64 width = static_cast<UInt8>(desc.index_element_size);
        const UInt64 offset = static_cast<UInt64>(desc.first_index) * width;
        const UInt64 bytes = static_cast<UInt64>(desc.vertex_or_index_count) * width;
        if (!range_written(index->record.written, offset, bytes))
            return impl_->fail("draw", "indexed range is out of bounds or uninitialized");
        for (UInt64 position = offset; position < offset + bytes; position += width) {
            UInt32 value = 0;
            std::memcpy(&value, index->record.shadow.data() + position, static_cast<std::size_t>(width));
            if (start_vertex + value >= available_vertices
                || !range_written(vertex->record.written, (start_vertex + value) * stride, stride))
                return impl_->fail("draw", "indexed vertex exceeds initialized source buffer");
        }
    } else if (start_vertex || desc.vertex_or_index_count > available_vertices
        || !range_written(vertex->record.written, 0, static_cast<UInt64>(desc.vertex_or_index_count) * stride))
        return impl_->fail("draw", "non-indexed vertex range is out of bounds or uninitialized");
    const auto native_vertex = bgfx::createVertexBuffer(
        bgfx::copy(vertex->record.shadow.data(), static_cast<UInt32>(vertex->record.shadow.size())), layout);
    if (!bgfx::isValid(native_vertex)) return impl_->fail("draw", "public bgfx vertex buffer creation failed");
    bgfx::IndexBufferHandle native_index = BGFX_INVALID_HANDLE;
    if (index) {
        const UInt16 flags = desc.index_element_size == IndexElementSize::uint32 ? BGFX_BUFFER_INDEX32 : BGFX_BUFFER_NONE;
        native_index = bgfx::createIndexBuffer(
            bgfx::copy(index->record.shadow.data(), static_cast<UInt32>(index->record.shadow.size())), flags);
        if (!bgfx::isValid(native_index)) {
            bgfx::destroy(native_vertex);
            return impl_->fail("draw", "public bgfx index buffer creation failed");
        }
    }
    bgfx::setVertexBuffer(0, native_vertex, static_cast<UInt32>(start_vertex), UINT32_MAX);
    if (index) bgfx::setIndexBuffer(native_index, desc.first_index, desc.vertex_or_index_count);
    const auto bind_stage = [&](const Impl::ShaderRecord& shader, const StageBindings& bindings) {
        for (const auto& field : shader.fields) {
            const auto& binding = bindings.uniforms[field.binding];
            const auto* buffer = lookup(impl_->buffers, binding.buffer);
            bgfx::setUniform(field.handle,
                buffer->record.shadow.data() + static_cast<std::size_t>(binding.offset + field.offset),
                static_cast<UInt16>(field.count));
        }
        for (const auto& texture : shader.textures) {
            const auto* image = lookup(impl_->textures, bindings.textures[texture.binding]);
            const auto* sampler = lookup(impl_->samplers, bindings.samplers[texture.binding]);
            bgfx::setTexture(static_cast<UInt8>(texture.stage), texture.handle, image->record.native,
                sampler_flags(sampler->record.desc));
        }
    };
    bind_stage(vertex_shader->record, desc.vertex_bindings);
    bind_stage(fragment_shader->record, desc.fragment_bindings);
    UInt64 flags = 0;
    if (state.blend.color_write_mask & 0x1) flags |= BGFX_STATE_WRITE_R;
    if (state.blend.color_write_mask & 0x2) flags |= BGFX_STATE_WRITE_G;
    if (state.blend.color_write_mask & 0x4) flags |= BGFX_STATE_WRITE_B;
    if (state.blend.color_write_mask & 0x8) flags |= BGFX_STATE_WRITE_A;
    if (state.depth_stencil.depth_test) flags |= depth_compare_state(state.depth_stencil.depth_compare);
    if (state.depth_stencil.depth_write) flags |= BGFX_STATE_WRITE_Z;
    if (state.raster.cull == CullMode::clockwise) flags |= BGFX_STATE_CULL_CW;
    else if (state.raster.cull == CullMode::counter_clockwise) flags |= BGFX_STATE_CULL_CCW;
    if (state.topology == PrimitiveTopology::triangle_strip) flags |= BGFX_STATE_PT_TRISTRIP;
    if (state.topology == PrimitiveTopology::point_list)
        flags |= BGFX_STATE_PT_POINTS | BGFX_STATE_POINT_SIZE(static_cast<UInt32>(desc.point_size));
    if (state.blend.enabled) {
        flags |= BGFX_STATE_BLEND_FUNC_SEPARATE(
            blend_factor(state.blend.source_color), blend_factor(state.blend.destination_color),
            blend_factor(state.blend.source_alpha), blend_factor(state.blend.destination_alpha));
        flags |= BGFX_STATE_BLEND_EQUATION_SEPARATE(
            blend_operation(state.blend.color_operation), blend_operation(state.blend.alpha_operation));
    }
    bgfx::setState(flags);
    // Pinned public bgfx provides per-draw depth control.  Reset it on every
    // draw so a source decal's D3D8 ZBIAS=8 cannot affect a later mesh.
    bgfx::setDepthControl(static_cast<Int32>(state.raster.depth_bias), 0.0f);
    if (state.depth_stencil.stencil_test) {
        if (state.depth_format != TextureFormat::depth24_stencil8) {
            if (index) bgfx::destroy(native_index);
            bgfx::destroy(native_vertex);
            return impl_->fail("draw", "stencil test requires D24S8 target");
        }
        const auto& stencil = state.depth_stencil;
        const UInt32 front = stencil_compare_state(stencil.stencil_compare)
            | BGFX_STENCIL_FUNC_REF(stencil.stencil_reference)
            | BGFX_STENCIL_FUNC_RMASK(stencil.stencil_read_mask)
            | (stencil_op_code(stencil.stencil_fail) << BGFX_STENCIL_OP_FAIL_S_SHIFT)
            | (stencil_op_code(stencil.depth_fail) << BGFX_STENCIL_OP_FAIL_Z_SHIFT)
            | (stencil_op_code(stencil.depth_pass) << BGFX_STENCIL_OP_PASS_Z_SHIFT);
        // The public second stencil word supplies the write mask to pinned
        // bgfx's Vulkan path while retaining the same front/back operations.
        const UInt32 back = (front & ~BGFX_STENCIL_FUNC_RMASK_MASK)
            | BGFX_STENCIL_FUNC_RMASK(stencil.stencil_write_mask);
        bgfx::setStencil(front, back);
    } else {
        bgfx::setStencil(BGFX_STENCIL_NONE);
    }
    bgfx::submit(static_cast<bgfx::ViewId>(impl_->next_view - 1), pipeline->record.native);
    if (index) bgfx::destroy(native_index);
    bgfx::destroy(native_vertex);
    return {};
}
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
ValidationResult BgfxGpuDevice::present(TextureHandle source)
{
    if (impl_->in_pass) return impl_->fail("present", "cannot present during an active pass");
    if (!impl_->window || !bgfx::isValid(impl_->window_framebuffer))
        return impl_->fail("present", "no SDL3 window is claimed");
    auto* image = lookup(impl_->textures, source);
    if (!image || !image->record.desc.render_target || !image->record.desc.sampled
        || !image->record.color_initialized || is_depth(image->record.desc.format))
        return impl_->fail("present", "stale, uninitialized or unsampleable color source");
    int width = 0, height = 0;
    if (!impl_->query_window_pixels(impl_->window, &width, &height) || width < 0 || height < 0)
        return impl_->fail("present", "SDL3 window pixel size is unavailable");
    if (width == 0 || height == 0) {
        bgfx::frame();
        impl_->next_view = 0;
        return {}; // Suspended/minimized surface; source resources remain live.
    }
    if (width > UINT16_MAX || height > UINT16_MAX || impl_->next_view >= RendererLimits::ordered_views)
        return impl_->fail("present", "window extent or ordered view budget exceeded");
    if (impl_->window_width != static_cast<UInt32>(width)
        || impl_->window_height != static_cast<UInt32>(height)) {
        bgfx::SwapChain swapchain;
        swapchain.nwh = impl_->window_nwh;
        swapchain.ndt = impl_->window_ndt;
        swapchain.width = static_cast<UInt32>(width);
        swapchain.height = static_cast<UInt32>(height);
        swapchain.formatColor = bgfx::TextureFormat::BGRA8;
        swapchain.formatDepthStencil = bgfx::TextureFormat::Count;
        bgfx::updateSwapChain(impl_->window_framebuffer, swapchain);
        impl_->window_width = static_cast<UInt32>(width);
        impl_->window_height = static_cast<UInt32>(height);
    }
    bgfx::VertexLayout layout;
    layout.begin().add(bgfx::Attrib::Position, 2, bgfx::AttribType::Float)
        .add(bgfx::Attrib::Color0, 4, bgfx::AttribType::Uint8, true)
        .add(bgfx::Attrib::TexCoord0, 2, bgfx::AttribType::Float).end();
    if (layout.getStride() != 20 || !bgfx::getAvailTransientVertexBuffer(3, layout))
        return impl_->fail("present", "public bgfx transient presentation vertices unavailable");
    bgfx::TransientVertexBuffer vertices;
    bgfx::allocTransientVertexBuffer(&vertices, 3, layout);
    struct Vertex { float x, y; UInt32 color; float u, v; };
    const std::array<Vertex,3> triangle{{
        {0,0,0xffffffffU,0,0},
        {2.0f * width,0,0xffffffffU,2,0},
        {0,2.0f * height,0xffffffffU,0,2},
    }};
    std::memcpy(vertices.data, triangle.data(), sizeof(triangle));
    const auto view = static_cast<bgfx::ViewId>(impl_->next_view++);
    bgfx::setViewFrameBuffer(view, impl_->window_framebuffer);
    bgfx::setViewMode(view, bgfx::ViewMode::Sequential);
    bgfx::setViewRect(view, 0, 0, static_cast<UInt16>(width), static_cast<UInt16>(height));
    bgfx::setViewClear(view, BGFX_CLEAR_COLOR, 0x000000ff);
    bgfx::setVertexBuffer(0, &vertices);
    const std::array<float,4> viewport{static_cast<float>(width),static_cast<float>(height),0,0};
    bgfx::setUniform(impl_->present_viewport, viewport.data());
    bgfx::setTexture(8, impl_->present_texture, image->record.native,
        BGFX_SAMPLER_U_CLAMP | BGFX_SAMPLER_V_CLAMP);
    bgfx::setState(BGFX_STATE_WRITE_RGB | BGFX_STATE_WRITE_A);
    bgfx::submit(view, impl_->present_program);
    bgfx::frame();
    impl_->next_view = 0;
    return {};
}
void BgfxGpuDevice::destroy(ShaderHandle handle)
{
    auto* slot = lookup(impl_->shaders, handle);
    if (!slot) return;
    for (const auto& pipeline : impl_->pipelines)
        if (pipeline.alive && (pipeline.record.key.descriptor().vertex_shader == handle
            || pipeline.record.key.descriptor().fragment_shader == handle)) {
            impl_->fail("destroy_shader", "shader is referenced by a live pipeline"); return;
        }
    bgfx::destroy(slot->record.native); retire(*slot);
}
void BgfxGpuDevice::destroy(PipelineHandle handle)
{
    if (auto* slot = lookup(impl_->pipelines, handle)) { bgfx::destroy(slot->record.native); retire(*slot); }
}
const std::string& BgfxGpuDevice::last_error() const noexcept { return impl_->last_error; }
bool BgfxGpuDevice::pass_active() const noexcept { return impl_->in_pass; }
void BgfxGpuDevice::record_marker(std::string_view) {}
ValidationResult BgfxGpuDevice::claim_window(SDL_Window* window)
{
    if (!window) return impl_->fail("claim_window", "SDL3 window is null");
    if (impl_->window) return impl_->fail("claim_window", "an SDL3 window is already claimed");
    if (impl_->in_pass) return impl_->fail("claim_window", "cannot claim during an active pass");
    if (!(bgfx::getCaps()->supported & BGFX_CAPS_SWAP_CHAIN))
        return impl_->fail("claim_window", "public bgfx Vulkan swap chains are unavailable");
    const auto properties = SDL_GetWindowProperties(window);
    if (!properties) return impl_->fail("claim_window", "invalid SDL3 window properties");
    void* nwh = nullptr;
    void* ndt = nullptr;
    bgfx::NativeWindowHandleType::Enum type = bgfx::NativeWindowHandleType::Default;
    const char* driver = SDL_GetCurrentVideoDriver();
    if (driver && std::strcmp(driver, "wayland") == 0) {
        nwh = SDL_GetPointerProperty(properties, SDL_PROP_WINDOW_WAYLAND_SURFACE_POINTER, nullptr);
        ndt = SDL_GetPointerProperty(properties, SDL_PROP_WINDOW_WAYLAND_DISPLAY_POINTER, nullptr);
        type = bgfx::NativeWindowHandleType::Wayland;
    } else if (driver && std::strcmp(driver, "x11") == 0) {
        nwh = reinterpret_cast<void*>(static_cast<std::uintptr_t>(
            SDL_GetNumberProperty(properties, SDL_PROP_WINDOW_X11_WINDOW_NUMBER, 0)));
        ndt = SDL_GetPointerProperty(properties, SDL_PROP_WINDOW_X11_DISPLAY_POINTER, nullptr);
    }
    if (!nwh || !ndt || type != impl_->native_type)
        return impl_->fail("claim_window", "SDL3 native window/display or video driver is unsupported");
    int width = 0, height = 0;
    if (!impl_->query_window_pixels(window, &width, &height) || width <= 0 || height <= 0
        || width > UINT16_MAX || height > UINT16_MAX)
        return impl_->fail("claim_window", "SDL3 window has unsupported pixel extent");
    auto vertex = create_shader({ShaderStage::vertex,"renderer/video.vert",1,0}, "presentation vertex");
    auto fragment = create_shader({ShaderStage::fragment,"renderer/video.frag",0,1}, "presentation fragment");
    if (!vertex || !fragment) {
        destroy(vertex); destroy(fragment);
        return impl_->fail("claim_window", "presentation shaders are unavailable");
    }
    auto* vertex_slot = lookup(impl_->shaders, vertex);
    auto* fragment_slot = lookup(impl_->shaders, fragment);
    if (vertex_slot->record.fields.size() != 1 || fragment_slot->record.textures.size() != 1) {
        destroy(vertex); destroy(fragment);
        return impl_->fail("claim_window", "presentation shader reflection differs");
    }
    auto program = bgfx::createProgram(vertex_slot->record.native, fragment_slot->record.native, false);
    if (!bgfx::isValid(program)) {
        destroy(vertex); destroy(fragment);
        return impl_->fail("claim_window", "public bgfx presentation program failed");
    }
    bgfx::SwapChain swapchain;
    swapchain.nwh = nwh; swapchain.ndt = ndt;
    swapchain.width = static_cast<UInt32>(width);
    swapchain.height = static_cast<UInt32>(height);
    swapchain.formatColor = bgfx::TextureFormat::BGRA8;
    swapchain.formatDepthStencil = bgfx::TextureFormat::Count;
    auto framebuffer = bgfx::createFrameBuffer(swapchain);
    if (!bgfx::isValid(framebuffer)) {
        bgfx::destroy(program); destroy(vertex); destroy(fragment);
        return impl_->fail("claim_window", "public bgfx swap-chain framebuffer failed");
    }
    impl_->window = window;
    impl_->window_framebuffer = framebuffer;
    impl_->present_program = program;
    impl_->present_vertex = vertex;
    impl_->present_fragment = fragment;
    impl_->present_viewport = vertex_slot->record.fields.front().handle;
    impl_->present_texture = fragment_slot->record.textures.front().handle;
    impl_->window_width = static_cast<UInt32>(width);
    impl_->window_height = static_cast<UInt32>(height);
    impl_->window_nwh = nwh; impl_->window_ndt = ndt;
    return {};
}
ValidationResult BgfxGpuDevice::wait_idle()
{
    if (impl_->in_pass) return impl_->fail("wait_idle", "pass remains active");
    bgfx::frame();
    impl_->next_view = 0;
    return {};
}

std::size_t BgfxGpuDevice::live_resource_count() const noexcept
{
    const auto live=[](const auto& slots) {
        return static_cast<std::size_t>(std::count_if(slots.begin(),slots.end(),
            [](const auto& slot) { return slot.alive; }));
    };
    return live(impl_->buffers)+live(impl_->textures)+live(impl_->samplers)+
        live(impl_->shaders)+live(impl_->pipelines);
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
    // bgfx::frame() returns the frame it just submitted after waiting for the
    // preceding render frame. The read command retains pixels.data(), so the
    // ready frame must finish before we inspect or release this vector.
    bgfx::frame();
    bgfx::destroy(target);
    impl_->next_view = 0;
    if (desc.format == TextureFormat::bgra8)
        for (std::size_t i = 0; i < pixels.size(); i += 4) std::swap(pixels[i], pixels[i + 2]);
    return pixels;
}
void BgfxGpuDevice::release_window() noexcept
{
    if (impl_->in_pass) return;
    if (bgfx::isValid(impl_->window_framebuffer)) bgfx::destroy(impl_->window_framebuffer);
    if (bgfx::isValid(impl_->present_program)) bgfx::destroy(impl_->present_program);
    destroy(impl_->present_vertex); destroy(impl_->present_fragment);
    impl_->window = nullptr;
    impl_->window_framebuffer = BGFX_INVALID_HANDLE;
    impl_->present_program = BGFX_INVALID_HANDLE;
    impl_->present_vertex = {}; impl_->present_fragment = {};
    impl_->present_viewport = BGFX_INVALID_HANDLE;
    impl_->present_texture = BGFX_INVALID_HANDLE;
    impl_->window_width = impl_->window_height = 0;
    impl_->window_nwh = impl_->window_ndt = nullptr;
}

} // namespace zh::renderer
