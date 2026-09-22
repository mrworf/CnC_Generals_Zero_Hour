#pragma once

#include "zh/renderer/contract.h"
#include "ww3dformat.h"
#include "ww3d_cpu_boundary.h"

#include <unordered_map>
#include <array>
#include <cstdint>
#include <optional>

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
        bool color_vertex=true;
        bool local_viewer=true;
        bool normalize_normals=false;
        unsigned ambient_source=0;
        unsigned diffuse_source=0;
        unsigned specular_source=0;
        unsigned emissive_source=0;
        std::array<float,4> global_ambient{};
        std::array<D3DLIGHT8,4> lights{};
        std::array<bool,4> light_enabled{};
        bool light_environment_selected=false;
        bool source_world_set=false,source_view_set=false;
        std::array<float,16> source_world{}, source_view{};
        bool alpha_test=false;
        renderer::CompareOp alpha_compare=renderer::CompareOp::always;
        float alpha_reference=0;
        std::array<float,4> fog_color{};
        float fog_start=0;
        float fog_end=0;
    };
    // std140-compatible source-state ABI. Matrices retain original row order;
    // the B3B2B vertex shader performs the D3D row-vector composition.
    struct alignas(16) VertexUniform {
        std::array<float,16> world{}, view{}, projection{};
        std::array<std::array<float,16>,2> texture_transform{};
        std::array<std::int32_t,4> coordinate_modes{};
        std::array<std::int32_t,4> uv_indices{};
        std::array<std::int32_t,4> transform_flags{};
        // Only the original bounded light/material snapshot is translated here.
        // Appended fields preserve the existing unlit uniform prefix.
        std::array<float,4> lit_diffuse{},lit_ambient{},lit_specular{},lit_emissive{};
        std::array<float,4> lit_global_ambient{};
        std::array<std::int32_t,4> lit_switches{}; // normalize, local viewer, specular, color vertex
        std::array<std::array<float,4>,4> light_position_range{};
        std::array<std::array<float,4>,4> light_direction_attenuation0{};
        std::array<std::array<float,4>,4> light_diffuse_attenuation1{};
        std::array<std::array<float,4>,4> light_ambient_attenuation2{};
        std::array<std::array<float,4>,4> light_specular_type{}; // 0 inactive, 1 point, 3 directional
        std::array<std::int32_t,4> lit_material_sources{}; // ambient, diffuse, specular, emissive
    };
    struct alignas(16) FragmentUniform {
        std::array<float,4> diffuse{}, ambient{}, specular{}, emissive{};
        std::array<float,4> fog_color{};
        std::array<float,4> fog_parameters{}; // start, end, enabled, material power
        std::array<float,4> alpha_parameters{}; // enabled, compare, normalized ref, specular enabled
        std::array<std::int32_t,4> material_sources{}; // ambient, diffuse, emissive, lighting
        std::array<std::array<std::int32_t,4>,2> stage_ops{}; // color, alpha, texture needed, UV transform flags
        std::array<std::array<std::int32_t,4>,2> stage_args{}; // color 1/2, alpha 1/2
        std::array<std::array<float,4>,2> bump{};
    };
    struct PhysicalState {
        renderer::PipelineHandle pipeline;
        renderer::StageBindings vertex_bindings,fragment_bindings;
        std::uint64_t generation=0,serial=0,source_revision=0;
        unsigned texture_mask=0;
    };
    explicit OriginalGpuEdge(renderer::GpuDevice& device);
    ~OriginalGpuEdge();
    OriginalGpuEdge(const OriginalGpuEdge&) = delete;
    OriginalGpuEdge& operator=(const OriginalGpuEdge&) = delete;

    renderer::BufferHandle bind_vertex(const VertexBufferClass* source);
    renderer::BufferHandle bind_index(const IndexBufferClass* source);
    // WW3D shutdown retires source pools while this device session may remain alive.
    void release_source_buffers();
    static renderer::OriginalFvfLayout layout_for_fvf(unsigned source_fvf);
    static AppliedState map_applied_state(unsigned source_fvf);
    PhysicalState prepare_applied_state(unsigned source_fvf,
        renderer::PrimitiveTopology topology=renderer::PrimitiveTopology::triangle_list);
    void validate_prepared_state(const PhysicalState& state) const;
    void draw_source_indexed(const VertexBufferClass* vertex,const IndexBufferClass* index,
        unsigned first_index,unsigned index_count,unsigned base_vertex,
        unsigned min_vertex,unsigned vertex_count,renderer::PrimitiveTopology topology);
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
    // The caller owns both attachments. Only original WW3D::Begin/End selects
    // their clear/load and presentation; this edge translates that selection.
    void bind_frame_targets(renderer::TextureHandle color, renderer::TextureHandle depth,
        unsigned width,unsigned height);
    std::pair<unsigned,unsigned> bound_frame_extent() const;
    void begin_source_frame(bool clear_color,bool clear_depth,
        float red,float green,float blue,float alpha);
    void end_source_frame(bool present);
    void abort_source_frame() noexcept;
    std::pair<unsigned,unsigned> active_render_target_extent() const noexcept;
    void set_source_viewport(float x,float y,float width,float height,float min_depth,float max_depth);
    // Records the original DX8Wrapper::Clear after CameraClass::Apply. The
    // caller still owns whether/when the source invokes a scene clear.
    void clear_source_viewport(bool color,bool depth,bool stencil,
        std::array<float,4> rgba,float z,unsigned stencil_value);
    bool source_depth_has_stencil() const;
    [[noreturn]] void texture_creation_unavailable(WW3DFormat format, unsigned width,
        unsigned height, unsigned mips, unsigned reduction);
    static OriginalGpuEdge& required();
    static OriginalGpuEdge* active() noexcept;

private:
    void release_prepared_state() noexcept;
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
    std::uint64_t source_revision_=0,physical_serial_=0;
    struct PhysicalResources {
        renderer::ShaderHandle vertex_shader,fragment_shader;
        renderer::PipelineHandle pipeline;
        renderer::BufferHandle vertex_uniform,fragment_uniform;
        PhysicalState state;
        std::array<const TextureBaseClass*,2> sources{};
    };
    std::optional<PhysicalResources> physical_;
    struct BoundFrame { renderer::TextureHandle color,depth; unsigned width=0,height=0; };
    std::optional<BoundFrame> bound_frame_;
    bool source_frame_active_=false;
    std::optional<renderer::ViewportDesc> source_viewport_;
    std::uint64_t frame_target_generation_=0;
};

} // namespace zh::original_runtime
