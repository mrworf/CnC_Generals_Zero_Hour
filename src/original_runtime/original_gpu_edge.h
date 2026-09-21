#pragma once

#include "zh/renderer/contract.h"
#include "ww3dformat.h"

#include <unordered_map>
#include <array>
#include <cstdint>

class VertexBufferClass;
class IndexBufferClass;
class TextureBaseClass;

namespace zh::original_runtime {

// Device-only translation of the bytes owned and populated by original WW3D
// buffers. Never selects geometry, material, shader or pass order.
class OriginalGpuEdge final {
public:
    enum class CombinerOp : std::uint8_t { disable, select_first, select_second, modulate, add };
    enum class CombinerArg : std::uint8_t { diffuse, current, texture };
    struct CombinerChannel {
        CombinerOp op = CombinerOp::disable;
        CombinerArg first = CombinerArg::diffuse;
        CombinerArg second = CombinerArg::diffuse;
    };
    struct AppliedStage {
        CombinerChannel color;
        CombinerChannel alpha;
        unsigned uv_source=0;
        unsigned coordinate_mode=0;
        unsigned transform_flags=0;
        std::array<float,16> transform{};
        std::array<float,4> bump{};
        bool transform_set=false;
        bool texture_required=false;
    };
    struct AppliedState {
        renderer::PipelineDesc pipeline;
        std::array<AppliedStage,2> stages{};
        std::array<float,4> diffuse{};
        std::array<float,4> ambient{};
        std::array<float,4> specular{};
        std::array<float,4> emissive{};
        float power=0;
        bool lighting=false;
        bool specular_enabled=false;
        unsigned ambient_source=0;
        unsigned diffuse_source=0;
        unsigned emissive_source=0;
        bool alpha_test=false;
        renderer::CompareOp alpha_compare=renderer::CompareOp::always;
        float alpha_reference=0;
        std::array<float,4> fog_color{};
        float fog_start=0;
        float fog_end=0;
    };
    explicit OriginalGpuEdge(renderer::GpuDevice& device);
    ~OriginalGpuEdge();
    OriginalGpuEdge(const OriginalGpuEdge&) = delete;
    OriginalGpuEdge& operator=(const OriginalGpuEdge&) = delete;

    renderer::BufferHandle bind_vertex(const VertexBufferClass* source);
    renderer::BufferHandle bind_index(const IndexBufferClass* source);
    static renderer::OriginalFvfLayout layout_for_fvf(unsigned source_fvf);
    static AppliedState map_applied_state(unsigned source_fvf);
    bool supports_texture_format(WW3DFormat format) const noexcept;
    renderer::TextureHandle create_texture(WW3DFormat format, unsigned width, unsigned height, unsigned& mips);
    void upload_texture(renderer::TextureHandle texture, unsigned level, unsigned width,
        unsigned height, unsigned pitch, const void* bytes, std::size_t size);
    void discard_texture(renderer::TextureHandle texture) noexcept;
    void publish_texture(TextureBaseClass* source, renderer::TextureHandle texture, bool shared_missing = false);
    renderer::TextureHandle missing_texture();
    static bool is_missing_texture(const TextureBaseClass* source) noexcept;
    renderer::TextureHandle texture_handle(const TextureBaseClass* source) const;
    std::uint64_t generation() const noexcept { return generation_; }
    static void release_texture_if_owned(TextureBaseClass* source) noexcept;
    struct PendingStage {
        renderer::TextureHandle texture;
        renderer::SamplerHandle sampler;
        std::uint64_t generation = 0;
        const TextureBaseClass* source = nullptr;
    };
    void select_texture(unsigned stage, const TextureBaseClass* source);
    enum class FilterStageState { min_filter, mag_filter, mip_filter, address_u, address_v };
    void set_filter_stage_state(unsigned stage, FilterStageState state, unsigned value);
    PendingStage pending_stage(unsigned stage) const;
    void record_source_state(std::string_view label);
    [[noreturn]] void texture_creation_unavailable(WW3DFormat format, unsigned width,
        unsigned height, unsigned mips, unsigned reduction);
    static OriginalGpuEdge& required();

private:
    renderer::GpuDevice& device_;
    OriginalGpuEdge* previous_;
    std::unordered_map<const VertexBufferClass*, renderer::BufferHandle> vertices_;
    std::unordered_map<const IndexBufferClass*, renderer::BufferHandle> indices_;
    struct TextureOwnership { renderer::TextureHandle handle; std::uint64_t generation; bool shared_missing; };
    std::unordered_map<TextureBaseClass*, TextureOwnership> textures_;
    renderer::TextureHandle missing_texture_;
    std::array<PendingStage, 8> pending_stages_{};
    struct PendingFilterValues { int min=-1,mag=-1,mip=-1,u=-1,v=-1; };
    std::array<PendingFilterValues,8> pending_filter_values_{};
    std::uint64_t generation_;
};

} // namespace zh::original_runtime
