#pragma once

#include "zh/renderer/contract.h"
#include "ww3dformat.h"
#include "ww3d_cpu_boundary.h"
#include "tree_program_contract.h"

#include <unordered_map>
#include <array>
#include <cstdint>
#include <optional>
#include <memory>
#include <exception>

class VertexBufferClass;
class IndexBufferClass;
class TextureBaseClass;
class TextureFilterClass;
class W3DShroud;
class HeightMapRenderObjClass;
struct W3DFrameGeneratedProbeAccess;
struct W3DTerrainPropLifecycleProbeAccess;

namespace zh::original_runtime {

// Device-only translation of the bytes owned and populated by original WW3D
// buffers. Never selects geometry, material, shader or pass order.
class OriginalGpuEdge final {
	friend struct ::W3DTerrainPropLifecycleProbeAccess;
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
    // Complete source-issued tree state. It does not select global DX8 stages.
    struct ImmutableTreeSnapshot {
        TreeVertexUniform vertex;
        FragmentUniform fragment;
        renderer::PipelineDesc pipeline;
        std::array<TextureBaseClass*,2> textures{};
        std::array<renderer::SamplerDesc,2> samplers{};
    };
    struct ImmutableTreeDecalSnapshot {
        VertexUniform vertex;
        FragmentUniform fragment;
        renderer::PipelineDesc pipeline;
        TextureBaseClass* texture=nullptr;
        renderer::SamplerDesc sampler;
    };
    class PreparedTreeProgram final {
    public:
        ~PreparedTreeProgram();
        PreparedTreeProgram(const PreparedTreeProgram&)=delete;
        PreparedTreeProgram& operator=(const PreparedTreeProgram&)=delete;
        const PhysicalState& state() const noexcept { return state_; }
    private:
        friend class OriginalGpuEdge;
        explicit PreparedTreeProgram(OriginalGpuEdge& edge) noexcept;
        OriginalGpuEdge* owner_;
        std::uint64_t generation_;
        PhysicalState state_;
        renderer::ShaderHandle vertex_shader_,fragment_shader_;
        std::array<renderer::SamplerHandle,2> samplers_{};
        std::array<TextureBaseClass*,2> sources_{};
        unsigned source_fvf_=0;
    };
    explicit OriginalGpuEdge(renderer::GpuDevice& device);
    ~OriginalGpuEdge();
    OriginalGpuEdge(const OriginalGpuEdge&) = delete;
    OriginalGpuEdge& operator=(const OriginalGpuEdge&) = delete;

    renderer::BufferHandle bind_vertex(const VertexBufferClass* source);
    renderer::BufferHandle bind_index(const IndexBufferClass* source);
    // A source owner may retire one buffer while the device session remains
    // live; do not force unrelated source owners out of the same session.
    void release_vertex(const VertexBufferClass* source);
    void release_index(const IndexBufferClass* source);
    // WW3D shutdown retires source pools while this device session may remain alive.
    void release_source_buffers();
    static renderer::OriginalFvfLayout layout_for_fvf(unsigned source_fvf);
    static AppliedState map_applied_state(unsigned source_fvf);
    PhysicalState prepare_applied_state(unsigned source_fvf,
        renderer::PrimitiveTopology topology=renderer::PrimitiveTopology::triangle_list);
    PhysicalState prepare_tree_state(const VertexBufferClass* source,
        const TreeVertexUniform& constants);
    std::unique_ptr<PreparedTreeProgram> prepare_immutable_tree_program(
        const VertexBufferClass* source,const ImmutableTreeSnapshot& snapshot);
    std::unique_ptr<PreparedTreeProgram> prepare_immutable_tree_decal_program(
        const VertexBufferClass* source,const ImmutableTreeDecalSnapshot& snapshot);
    bool probe_tree_frame_admission();
    bool immutable_tree_program_current(const PreparedTreeProgram&) const noexcept;
    // Source-frame ownership composes the device journal with source caches.
    // Only a fully admitted immutable tree frame uses this boundary.
    bool begin_tree_source_frame();
    bool commit_tree_source_frame() noexcept;
    bool abort_tree_source_frame() noexcept;
    bool tree_source_frame_pending() const noexcept;
    bool admitted_source_frame_active() const noexcept
    { return source_frame_attempt_ && source_frame_active_ && device_.pass_active(); }
    void draw_immutable_tree(const PreparedTreeProgram&,const VertexBufferClass*,
        const IndexBufferClass*,unsigned vertex_count,unsigned index_count);
    void draw_immutable_tree_decal(const PreparedTreeProgram&,const VertexBufferClass*,
        const IndexBufferClass*,unsigned vertex_count,unsigned index_count);
    void validate_prepared_state(const PhysicalState& state) const;
    void draw_source_indexed(const VertexBufferClass* vertex,const IndexBufferClass* index,
        unsigned first_index,unsigned index_count,unsigned base_vertex,
        unsigned min_vertex,unsigned vertex_count,renderer::PrimitiveTopology topology);
    // B3C0's source-buffer bridge: it consumes only an original XYZ/16-bit
    // volume range inside a caller-owned D24S8 source frame. It creates no
    // Shadow owner, geometry, scheduling, target, or presentation path.
    void draw_volume_stencil(const VertexBufferClass* vertex, const IndexBufferClass* index,
        unsigned first_index, unsigned index_count, unsigned base_vertex, unsigned vertex_count,
        renderer::UInt8 shadow_mask);
    void release_volume_stencil() noexcept;
    bool supports_texture_format(WW3DFormat format) const noexcept;
    renderer::TextureHandle create_texture(WW3DFormat format, unsigned width, unsigned height, unsigned& mips);
    void upload_texture(renderer::TextureHandle texture, unsigned level, unsigned width,
        unsigned height, unsigned pitch, const void* bytes, std::size_t size);
    void discard_texture(renderer::TextureHandle texture) noexcept;
    void publish_texture(TextureBaseClass* source, renderer::TextureHandle texture, bool shared_missing = false);
    void publish_texture_alias(TextureBaseClass* source, const TextureBaseClass* owner);
    renderer::TextureHandle missing_texture();
    static bool is_missing_texture(const TextureBaseClass* source) noexcept;
    renderer::TextureHandle texture_handle(const TextureBaseClass* source) const;
    std::uint64_t generation() const noexcept { return generation_; }
    // Device capability only. Complete source-stage checkpointing is a
    // separate owner; this boundary does not snapshot refs/maps/revisions.
    bool supports_device_transactions(renderer::DeviceTransactionMode mode) const noexcept;
    bool idle_preparation_ready() const noexcept;
    bool resident_texture(const TextureBaseClass* source) const noexcept;
    renderer::ValidationResult begin_device_transaction(const renderer::DeviceTransactionDesc&,
        renderer::DeviceTransactionToken&);
    bool commit_device_transaction(const renderer::DeviceTransactionToken&) noexcept;
    bool abort_device_transaction(const renderer::DeviceTransactionToken&) noexcept;
    std::uint64_t frame_target_generation() const noexcept { return frame_target_generation_; }
    // Source references are distinct from native create/reference ownership.
    // Finish only transfers pin units; ordinary drain owns fallible cleanup.
    static constexpr unsigned source_reference_capacity=64;
    struct SourceReferenceToken {
        std::uint64_t owner=0,sequence=0,generation=0;
        unsigned units=0;
    };
    renderer::ValidationResult begin_source_references(std::uint64_t generation,
        TextureBaseClass* const* sources,unsigned count,SourceReferenceToken& token);
    bool finish_source_references(const SourceReferenceToken& token) noexcept;
    bool cancel_source_references(const SourceReferenceToken& token) noexcept;
    bool drain_source_references(std::uint64_t generation) noexcept;
    bool shutdown_source_references() noexcept;
    unsigned queued_source_reference_count() const noexcept { return source_reference_queued_; }
    unsigned reserved_source_reference_count() const noexcept { return source_reference_token_.units; }
    std::uint64_t source_reference_release_count() const noexcept { return source_reference_releases_; }
    std::uint64_t source_revision() const noexcept { return source_revision_; }
    struct SourceStageSelection { unsigned stage=0; TextureBaseClass* texture=nullptr; };
    struct SourceStageDesc {
        std::uint64_t generation=0;
        const SourceStageSelection* selections=nullptr;
        unsigned count=0,stage_keys=12,transform_nodes=8;
        unsigned commands=4096,resources=4096;
        std::size_t bytes=64U*1024U*1024U;
    };
    struct SourceStageToken { std::uint64_t owner=0,sequence=0,generation=0; unsigned mask=0; };
    renderer::ValidationResult begin_source_stages(const SourceStageDesc&,SourceStageToken&);
    void apply_source_stages(const SourceStageToken&);
    bool commit_source_stages(const SourceStageToken&) noexcept;
    bool abort_source_stages(const SourceStageToken&) noexcept;
    void cancel_source_stages_for_reset();
    void guard_source_stage(unsigned stage,const TextureBaseClass* provider=nullptr,bool selection=false);
    void guard_source_stage_key(unsigned stage,unsigned key);
    void guard_source_filter(unsigned stage,const TextureFilterClass* filter);
    void guard_source_transform(int transform);
    void guard_nonstage_mutation();
    void poison_source_stages() noexcept;
    struct SourceMutationGuard {
        OriginalGpuEdge* edge;
        int exceptions=std::uncaught_exceptions();
        explicit SourceMutationGuard(OriginalGpuEdge& owner):edge(&owner) {}
        explicit SourceMutationGuard(OriginalGpuEdge* owner):edge(owner) {}
        ~SourceMutationGuard() { if (edge && std::uncaught_exceptions()>exceptions) edge->poison_source_stages(); }
    };
    bool source_stages_active() const noexcept { return bool(source_stage_attempt_); }
    void fail_next_source_stage_commit() noexcept { source_stage_commit_fault_=true; }
    void fail_source_stage_allocation_after(unsigned successful) noexcept { source_stage_allocation_fault_=successful; }
    void source_stage_allocation_boundary();
    void fail_next_source_reference_commit() noexcept { source_reference_commit_fault_=true; }
    // Lifecycle notification borrows no stale provider. The caller pins its
    // receiver across notification and its ordinary mutation.
    static bool source_texture_references_pending(const TextureBaseClass* source) noexcept;
    static void notify_source_texture_invalidation(TextureBaseClass* source);
    bool source_buffers_retirable() const noexcept { return !source_frame_active_; }
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
    friend class ::W3DShroud;
    friend class ::HeightMapRenderObjClass;
    friend struct ::W3DFrameGeneratedProbeAccess;
    void mark_tree_source_terrain();
    // Only the shroud owner may withdraw its unpublished first candidate.
    // The device owns native rollback; this is not ordinary removal.
    bool withdraw_candidate_texture(TextureBaseClass* source,renderer::TextureHandle texture) noexcept;
    struct SourceStageAttempt;
    struct SourceFrameAttempt;
    std::unique_ptr<SourceFrameAttempt> source_frame_attempt_;
    // Only the generated friend can reach these once-only source boundaries.
    // No public setter, serialized state, environment or retail selector.
    bool source_frame_present_fault_=false,source_frame_commit_fault_=false;
    std::unique_ptr<SourceStageAttempt> source_stage_attempt_;
    std::uint64_t source_stage_sequence_=0;
    bool source_stage_commit_fault_=false;
    std::optional<unsigned> source_stage_allocation_fault_;
    static AppliedState map_state(unsigned source_fvf,bool tree_program);
    PhysicalState prepare_state(unsigned source_fvf,renderer::PrimitiveTopology topology,
        const TreeVertexUniform* tree);
    void release_prepared_state() noexcept;
    renderer::GpuDevice& device_;
    std::optional<renderer::DeviceTransactionToken> device_transaction_;
    std::array<TextureBaseClass*,source_reference_capacity> source_reference_pins_{};
    std::array<TextureBaseClass*,source_reference_capacity> source_reference_queue_{};
    SourceReferenceToken source_reference_token_{};
    std::uint64_t source_reference_sequence_=0,source_reference_releases_=0;
    unsigned source_reference_queued_=0;
    bool source_reference_commit_fault_=false;
    OriginalGpuEdge* previous_;
    std::unordered_map<const VertexBufferClass*, renderer::BufferHandle> vertices_;
    std::unordered_map<const IndexBufferClass*, renderer::BufferHandle> indices_;
    struct TextureOwnership { renderer::TextureHandle handle; std::uint64_t generation; bool shared_missing; };
    std::unordered_map<TextureBaseClass*, TextureOwnership> textures_;
    std::unordered_map<renderer::UInt32, unsigned> texture_owner_refs_;
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
        std::size_t vertex_uniform_size=sizeof(VertexUniform);
        PhysicalState state;
        std::array<const TextureBaseClass*,2> sources{};
    };
    std::optional<PhysicalResources> physical_;
    struct VolumeStencilResources {
        renderer::ShaderHandle vertex_shader,fragment_shader;
        renderer::PipelineHandle increment,decrement,composite;
        renderer::TextureFormat color_format=renderer::TextureFormat::rgba8;
    };
    std::optional<VolumeStencilResources> volume_stencil_;
    struct BoundFrame { renderer::TextureHandle color,depth; unsigned width=0,height=0; };
    std::optional<BoundFrame> bound_frame_;
    bool source_frame_active_=false;
    std::optional<renderer::ViewportDesc> source_viewport_;
    std::uint64_t frame_target_generation_=0;
};

} // namespace zh::original_runtime
