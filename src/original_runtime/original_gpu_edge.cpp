#include "original_gpu_edge.h"

#include "dx8vertexbuffer.h"
#include "dx8indexbuffer.h"
#include "dx8fvf.h"
#include "dx8wrapper.h"
#include "ww3dformat.h"
#include "texture.h"
#include "missingtexture.h"
#include "volume_stencil_contract.h"
#include "volume_buffer_provider.h"
#include "ww3d.h"
#include "assetmgr.h"

#include <algorithm>
#include <atomic>
#include <cmath>
#include <cstring>
#include <cstdio>
#include <limits>
#include <stdexcept>
#include <string>
#include <exception>
#include <vector>

namespace zh::original_runtime {
namespace {
thread_local OriginalGpuEdge* active_edge;
std::atomic<std::uint64_t> next_generation{1};

renderer::TextureFormat translate_format(WW3DFormat source)
{
    switch (source) {
    case WW3D_FORMAT_DXT1: return renderer::TextureFormat::bc1;
    case WW3D_FORMAT_DXT2:
    case WW3D_FORMAT_DXT3: return renderer::TextureFormat::bc2;
    case WW3D_FORMAT_DXT4:
    case WW3D_FORMAT_DXT5: return renderer::TextureFormat::bc3;
    case WW3D_FORMAT_A8R8G8B8:
    case WW3D_FORMAT_X8R8G8B8: return renderer::TextureFormat::bgra8;
    case WW3D_FORMAT_A1R5G5B5: return renderer::TextureFormat::bgr5a1;
    default: throw std::runtime_error("original texture format has no physical GPU mapping");
    }
}
}

struct OriginalGpuEdge::SourceStageAttempt {
    SourceStageToken token;
    std::array<TextureBaseClass*,8> selections{};
    std::shared_ptr<DX8Wrapper::SourceStageCheckpoint> checkpoint;
    std::array<PendingStage,8> pending{};
    std::array<PendingFilterValues,8> filters{};
    std::array<TextureBaseClass*,source_reference_capacity> providers{};
    std::array<unsigned,source_reference_capacity> access{};
    unsigned provider_count=0,stage_keys=0,transform_nodes=0;
    std::uint64_t revision=0;
    SourceReferenceToken refs;
    renderer::DeviceTransactionToken device;
    bool failed=false,applied=false;
};

struct OriginalGpuEdge::SourceFrameAttempt {
    struct TextureMetadata {
        TextureBaseClass* source=nullptr;
        unsigned access=0,inactivation=0,extended=0;
        bool initialized=false;
    };
    struct VertexBytes { const DX8VertexBufferClass* source=nullptr;std::vector<unsigned char> bytes; };
    struct IndexBytes { const DX8IndexBufferClass* source=nullptr;std::vector<unsigned short> bytes; };
    std::shared_ptr<DX8Wrapper::SourceFrameCheckpoint> source;
    std::shared_ptr<WW3D::SourceFrameCheckpoint> ww3d;
    std::shared_ptr<DynamicVBAccessClass::SourceFrameCheckpoint> dynamic_vertices;
    std::shared_ptr<DynamicIBAccessClass::SourceFrameCheckpoint> dynamic_indices;
    decltype(vertices_) vertices;
    decltype(indices_) indices;
    decltype(textures_) textures;
    decltype(texture_owner_refs_) texture_owners;
    decltype(pending_stages_) pending{};
    decltype(pending_filter_values_) filters{};
    decltype(physical_) physical;
    decltype(volume_stencil_) volume;
    decltype(source_viewport_) viewport;
    std::vector<TextureMetadata> metadata;
    std::vector<VertexBytes> vertex_bytes;
    std::vector<IndexBytes> index_bytes;
    std::uint64_t revision=0;
    renderer::DeviceTransactionToken device;
    bool buffer_pins=false,texture_pins=false;
    bool terrain_produced=false;
    ~SourceFrameAttempt()
    {
        if (buffer_pins) {
            for (const auto& entry:vertices) entry.first->Release_Ref();
            for (const auto& entry:indices) entry.first->Release_Ref();
        }
        if (texture_pins) for (const auto& entry:metadata) entry.source->Release_Ref();
    }
};

bool OriginalGpuEdge::tree_source_frame_pending() const noexcept
{ return source_frame_attempt_!=nullptr; }

void OriginalGpuEdge::mark_tree_source_terrain()
{
    if (!source_frame_attempt_) return;
    if (!source_frame_active_ || !device_.pass_active() || source_frame_attempt_->terrain_produced)
        throw std::runtime_error("original immutable tree terrain producer rejected");
    source_frame_attempt_->terrain_produced=true;
}

bool OriginalGpuEdge::begin_tree_source_frame()
{
    if (active_edge!=this || source_frame_attempt_ || device_transaction_ || source_stage_attempt_
        || source_frame_active_ || device_.pass_active() || source_reference_queued_
        || source_reference_token_.units || !bound_frame_
        || vertices_.size()+indices_.size()+textures_.size()>4096
        || !device_.supports_device_transactions(renderer::DeviceTransactionMode::frame_commands)) return false;
    try {
        auto attempt=std::make_unique<SourceFrameAttempt>();
        attempt->vertices=vertices_;attempt->indices=indices_;attempt->textures=textures_;
        attempt->texture_owners=texture_owner_refs_;attempt->pending=pending_stages_;
        attempt->filters=pending_filter_values_;attempt->physical=physical_;
        attempt->volume=volume_stencil_;attempt->revision=source_revision_;
        attempt->viewport=source_viewport_;
        attempt->metadata.reserve(textures_.size());attempt->vertex_bytes.reserve(vertices_.size());
        attempt->index_bytes.reserve(indices_.size());
        std::size_t bytes=0;
        const auto capture_metadata=[&](TextureBaseClass *texture,bool resident) {
            if (!texture || (resident && !texture->Is_Initialized()) || texture->Num_Refs()<=0
                || texture->Num_Refs()>std::numeric_limits<int>::max()-32)
                throw std::runtime_error("original tree frame texture metadata provider rejected");
            for (const auto &entry:attempt->metadata) if (entry.source==texture) return;
            if (attempt->metadata.size()==4096)
                throw std::runtime_error("original tree frame texture metadata capacity rejected");
            attempt->metadata.push_back({texture,texture->LastAccessed,
                texture->LastInactivationSyncTime,texture->ExtendedInactivationTime,texture->Initialized});
        };
        for (const auto& entry:textures_) capture_metadata(entry.first,true);
        // Begin_Render's native loader expiry walks the complete asset hash,
        // not just GPU-resident sources. Capture only its mutable metadata;
        // do not initialize/load any nonresident source while admitting it.
        if (WW3D::Get_Thumbnail_Enabled()) {
            auto *assets=WW3DAssetManager::Get_Instance();
            if (assets) {
                HashTemplateIterator<StringClass,TextureClass *> textures(assets->Texture_Hash());
                unsigned entries=0;
                for (textures.First();!textures.Is_Done();textures.Next()) {
                    if (entries++==4096) throw std::runtime_error("original tree frame asset texture capacity rejected");
                    capture_metadata(textures.Peek_Value(),false);
                }
            }
        }
        for (const auto& entry:vertices_) {
            const auto* vertex=static_cast<const DX8VertexBufferClass*>(entry.first);
            const std::size_t size=std::size_t(vertex->Get_Vertex_Count())*vertex->FVF_Info().Get_FVF_Size();
            if (!vertex->Get_CPU_Vertex_Buffer() || size>renderer::RendererLimits::maximum_upload_bytes-bytes) return false;
            bytes+=size;
            const auto* begin=static_cast<const unsigned char*>(vertex->Get_CPU_Vertex_Buffer());
            attempt->vertex_bytes.push_back({vertex,std::vector<unsigned char>(begin,begin+size)});
        }
        for (const auto& entry:indices_) {
            const auto* index=static_cast<const DX8IndexBufferClass*>(entry.first);
            const std::size_t size=std::size_t(index->Get_Index_Count())*sizeof(unsigned short);
            if (!index->Get_CPU_Index_Buffer() || size>renderer::RendererLimits::maximum_upload_bytes-bytes) return false;
            bytes+=size;
            const auto* begin=index->Get_CPU_Index_Buffer();
            attempt->index_bytes.push_back({index,std::vector<unsigned short>(begin,begin+index->Get_Index_Count())});
        }
        attempt->source=DX8Wrapper::Capture_Source_Frame();
        attempt->ww3d=WW3D::Capture_Source_Frame();
        attempt->dynamic_vertices=DynamicVBAccessClass::Capture_Source_Frame();
        attempt->dynamic_indices=DynamicIBAccessClass::Capture_Source_Frame();
        for (const auto& entry:attempt->vertices) entry.first->Add_Ref();
        for (const auto& entry:attempt->indices) entry.first->Add_Ref();
        attempt->buffer_pins=true;
        for (const auto& entry:attempt->metadata) entry.source->Add_Ref();
        attempt->texture_pins=true;
        const renderer::DeviceTransactionDesc desc{renderer::DeviceTransactionMode::frame_commands,
            frame_target_generation_,4096,4096,renderer::RendererLimits::maximum_upload_bytes,
            renderer::RendererLimits::ordered_views};
        if (!begin_device_transaction(desc,attempt->device)) return false;
        source_frame_attempt_=std::move(attempt);
        return true;
    } catch (...) { return false; }
}

bool OriginalGpuEdge::commit_tree_source_frame() noexcept
{
    if (!source_frame_attempt_ || source_frame_active_ || device_.pass_active()) return false;
    if (source_frame_commit_fault_) { source_frame_commit_fault_=false;return false; }
    if (!commit_device_transaction(source_frame_attempt_->device)) return false;
    source_frame_attempt_.reset();
    return true;
}

bool OriginalGpuEdge::abort_tree_source_frame() noexcept
{
    if (!source_frame_attempt_) return false;
    auto& attempt=*source_frame_attempt_;
    if (!abort_device_transaction(attempt.device)) return false;
    DX8Wrapper::Restore_Source_Frame(*attempt.source);
    WW3D::Restore_Source_Frame(*attempt.ww3d);
    DynamicVBAccessClass::Restore_Source_Frame(*attempt.dynamic_vertices);
    DynamicIBAccessClass::Restore_Source_Frame(*attempt.dynamic_indices);
    for (const auto& entry:vertices_) entry.first->Release_Ref();
    for (const auto& entry:indices_) entry.first->Release_Ref();
    vertices_.swap(attempt.vertices);indices_.swap(attempt.indices);
    attempt.buffer_pins=false; // Captured pins are now the restored map units.
    textures_.swap(attempt.textures);texture_owner_refs_.swap(attempt.texture_owners);
    pending_stages_=attempt.pending;pending_filter_values_=attempt.filters;
    physical_=attempt.physical;volume_stencil_=attempt.volume;source_revision_=attempt.revision;
    source_frame_active_=false;source_viewport_=attempt.viewport;
    for (const auto& entry:attempt.metadata) {
        entry.source->LastAccessed=entry.access;entry.source->LastInactivationSyncTime=entry.inactivation;
        entry.source->ExtendedInactivationTime=entry.extended;entry.source->Initialized=entry.initialized;
    }
    for (const auto& entry:attempt.vertex_bytes)
        std::memcpy(const_cast<unsigned char*>(entry.source->Get_CPU_Vertex_Buffer()),entry.bytes.data(),entry.bytes.size());
    for (const auto& entry:attempt.index_bytes)
        std::memcpy(const_cast<unsigned short*>(entry.source->Get_CPU_Index_Buffer()),entry.bytes.data(),entry.bytes.size()*sizeof(unsigned short));
    source_frame_attempt_.reset();
    return true;
}

void OriginalGpuEdge::draw_immutable_tree(const PreparedTreeProgram& program,
    const VertexBufferClass* vertex,const IndexBufferClass* index,
    unsigned vertex_count,unsigned index_count)
{
    guard_nonstage_mutation();
    const auto vb=vertices_.find(vertex);
    const auto ib=indices_.find(index);
    if (!source_frame_attempt_ || !source_frame_active_ || !device_.pass_active()
        || !source_frame_attempt_->terrain_produced || !immutable_tree_program_current(program) || vb==vertices_.end() || ib==indices_.end()
        || !vertex_count || !index_count || index_count%3
        || vertex_count!=vertex->Get_Vertex_Count() || index_count!=index->Get_Index_Count()
        || vertex->Type()!=BUFFER_TYPE_DX8 || index->Type()!=BUFFER_TYPE_DX8
        || program.source_fvf_!=DX8_FVF_XYZNDUV1 || vertex->FVF_Info().Get_FVF()!=DX8_FVF_XYZNDUV1)
        throw std::runtime_error("original immutable tree draw provider/range unavailable");
    const auto* source=static_cast<const DX8IndexBufferClass*>(index)->Get_CPU_Index_Buffer();
    if (!source) throw std::runtime_error("original immutable tree indices unavailable");
    for (unsigned i=0;i<index_count;++i)
        if (source[i]>=vertex_count)
            throw std::runtime_error("original immutable tree index escapes accepted range");
    renderer::DrawDesc draw;
    draw.pipeline=program.state_.pipeline;draw.vertex_buffer=vb->second;draw.index_buffer=ib->second;
    draw.vertex_or_index_count=index_count;draw.index_element_size=renderer::IndexElementSize::uint16;
    draw.vertex_bindings=program.state_.vertex_bindings;draw.fragment_bindings=program.state_.fragment_bindings;
    if (!device_.draw(draw)) throw std::runtime_error("original immutable tree submission rejected");
    record_source_state("original BaseHeightMap::renderTrees immutable indexed triangles");
}

void OriginalGpuEdge::draw_immutable_tree_decal(const PreparedTreeProgram& program,
    const VertexBufferClass* vertex,const IndexBufferClass* index,
    unsigned vertex_count,unsigned index_count)
{
    guard_nonstage_mutation();
    const auto vb=vertices_.find(vertex);const auto ib=indices_.find(index);
    if (!source_frame_attempt_ || !source_frame_active_ || !device_.pass_active()
        || !source_frame_attempt_->terrain_produced || !immutable_tree_program_current(program)
        || vb==vertices_.end() || ib==indices_.end() || !vertex_count || !index_count || index_count%3
        || vertex_count!=vertex->Get_Vertex_Count() || index_count!=index->Get_Index_Count()
        || vertex->Type()!=BUFFER_TYPE_DX8 || index->Type()!=BUFFER_TYPE_DX8
        || program.source_fvf_!=DX8_FVF_XYZDUV1 || vertex->FVF_Info().Get_FVF()!=DX8_FVF_XYZDUV1)
        throw std::runtime_error("original immutable tree decal provider/range unavailable");
    const auto* source=static_cast<const DX8IndexBufferClass*>(index)->Get_CPU_Index_Buffer();
    if (!source) throw std::runtime_error("original immutable tree decal indices unavailable");
    for (unsigned i=0;i<index_count;++i) if (source[i]>=vertex_count)
        throw std::runtime_error("original immutable tree decal index escapes accepted range");
    renderer::DrawDesc draw;
    draw.pipeline=program.state_.pipeline;draw.vertex_buffer=vb->second;draw.index_buffer=ib->second;
    draw.vertex_or_index_count=index_count;draw.index_element_size=renderer::IndexElementSize::uint16;
    draw.vertex_bindings=program.state_.vertex_bindings;draw.fragment_bindings=program.state_.fragment_bindings;
    if (!device_.draw(draw)) throw std::runtime_error("original immutable tree decal submission rejected");
    record_source_state("original W3DProjectedShadowManager::flushDecals multiplicative tree triangles");
}

renderer::ValidationResult OriginalGpuEdge::begin_source_stages(const SourceStageDesc& desc,SourceStageToken& token)
{
    if (active_edge!=this || source_stage_attempt_ || device_transaction_ || source_frame_active_ || source_reference_queued_
        || device_.pass_active() || desc.generation!=generation_ || !desc.selections || !desc.count
        || !device_.supports_device_transactions(renderer::DeviceTransactionMode::idle_preparation)
        || desc.count>8 || !desc.stage_keys || desc.stage_keys>12 || desc.transform_nodes>8 || desc.transform_nodes<desc.count
        || !desc.commands || desc.commands>4096 || !desc.resources || desc.resources>4096
        || !desc.bytes || desc.bytes>64U*1024U*1024U
        || source_revision_>std::numeric_limits<std::uint64_t>::max()-desc.commands-1
        || source_stage_sequence_==std::numeric_limits<std::uint64_t>::max())
        return {false,"original source stage transaction bounds or phase are invalid"};
    unsigned mask=0,transforms=0;
    std::array<TextureBaseClass*,source_reference_capacity> providers{};
    unsigned count=0;
    const auto admit=[&](TextureBaseClass* source,unsigned stage) {
        if (!source) return true;
        const auto owner=textures_.find(source);
        if (owner==textures_.end() || owner->second.generation!=generation_
            || !device_.describe_texture_format(owner->second.handle)) return false;
        if (!source->As_TextureClass() || !source->Is_Initialized()
            || source->Num_Refs()<=0 || source->Num_Refs()>std::numeric_limits<int>::max()-10
            || !source->As_TextureClass()->Get_Filter().Can_Apply(stage)) return false;
        for (unsigned i=0;i<count;++i) if (providers[i]==source) return true;
        if (count==providers.size()) return false;
        providers[count++]=source;return true;
    };
    for (unsigned i=0;i<desc.count;++i) {
        const auto selection=desc.selections[i];
        if (selection.stage>=8 || (mask&(1U<<selection.stage))
            || DX8Wrapper::Source_Stage_Key_Count(selection.stage)>desc.stage_keys)
            return {false,"original source stage selection is invalid"};
        mask|=1U<<selection.stage;
        if (DX8Wrapper::Source_Transform_Present(D3DTS_TEXTURE0+selection.stage)) ++transforms;
        if (!admit(selection.texture,selection.stage)
            || !admit(DX8Wrapper::Peek_Texture(selection.stage),selection.stage)
            || !admit(const_cast<TextureBaseClass*>(pending_stages_[selection.stage].source),selection.stage))
            return {false,"original source stage provider is not resident and ready"};
    }
    if (transforms>desc.transform_nodes || count>source_reference_capacity-source_reference_queued_)
        return {false,"original source stage checkpoint capacity is insufficient"};
    SourceReferenceToken acquired;
    renderer::DeviceTransactionToken admitted;
    try {
        source_stage_allocation_boundary();
        auto attempt=std::make_unique<SourceStageAttempt>();
        attempt->checkpoint=DX8Wrapper::Capture_Source_Stages(mask);
        attempt->stage_keys=desc.stage_keys;attempt->transform_nodes=desc.transform_nodes;
        attempt->revision=source_revision_;attempt->pending=pending_stages_;attempt->filters=pending_filter_values_;
        attempt->providers=providers;attempt->provider_count=count;
        for (unsigned i=0;i<count;++i) attempt->access[i]=providers[i]->LastAccessed;
        for (unsigned i=0;i<desc.count;++i) attempt->selections[desc.selections[i].stage]=desc.selections[i].texture;
        if (count) { auto result=begin_source_references(generation_,providers.data(),count,attempt->refs);if (!result) return result;acquired=attempt->refs; }
        const renderer::DeviceTransactionDesc device_desc{renderer::DeviceTransactionMode::idle_preparation,
            generation_,desc.commands,desc.resources,desc.bytes,0};
        auto result=begin_device_transaction(device_desc,attempt->device);
        if (!result) {
            if (count && (!cancel_source_references(attempt->refs) || !drain_source_references(generation_))) std::terminate();
            return result;
        }
        admitted=attempt->device;
        attempt->token={generation_,source_stage_sequence_+1,generation_,mask};
        token=attempt->token;++source_stage_sequence_;source_stage_attempt_=std::move(attempt);
        return {true,{}};
    } catch (...) {
        if (admitted.sequence && !abort_device_transaction(admitted)) std::terminate();
        if (acquired.units && source_reference_token_.units
            && (!cancel_source_references(acquired) || !drain_source_references(generation_))) std::terminate();
        return {false,"original source stage checkpoint allocation failed"};
    }
}
void OriginalGpuEdge::poison_source_stages() noexcept
{ if (source_stage_attempt_) source_stage_attempt_->failed=true; }
void OriginalGpuEdge::source_stage_allocation_boundary()
{
    if (!source_stage_allocation_fault_) return;
    if (*source_stage_allocation_fault_) { --*source_stage_allocation_fault_;return; }
    source_stage_allocation_fault_.reset();poison_source_stages();throw std::bad_alloc();
}
void OriginalGpuEdge::guard_nonstage_mutation()
{
    if (!source_stage_attempt_) return;
    poison_source_stages();throw std::runtime_error("original nonstage mutation is outside the selected stage owner");
}
void OriginalGpuEdge::guard_source_stage(unsigned stage,const TextureBaseClass* provider,bool selection)
{
    if (!source_stage_attempt_) return;
    auto& owner=*source_stage_attempt_;
    if (stage<8 && (owner.token.mask&(1U<<stage))
        && (!selection || owner.selections[stage]==provider) && !owner.failed) { owner.applied=false;return; }
    poison_source_stages();throw std::runtime_error("original source stage mutation is outside its declared selection");
}
void OriginalGpuEdge::guard_source_stage_key(unsigned stage,unsigned key)
{
    guard_source_stage(stage);
    if (!source_stage_attempt_) return;
    const bool known=key==D3DTSS_TEXCOORDINDEX || key==D3DTSS_TEXTURETRANSFORMFLAGS
        || key==D3DTSS_BUMPENVMAT00 || key==D3DTSS_BUMPENVMAT01 || key==D3DTSS_BUMPENVMAT10 || key==D3DTSS_BUMPENVMAT11
        || key==D3DTSS_COLOROP || key==D3DTSS_ALPHAOP || key==D3DTSS_COLORARG1 || key==D3DTSS_COLORARG2
        || key==D3DTSS_ALPHAARG1 || key==D3DTSS_ALPHAARG2;
    if (known && (DX8Wrapper::Source_Stage_Key_Present(stage,key)
        || DX8Wrapper::Source_Stage_Key_Count(stage)<source_stage_attempt_->stage_keys)) return;
    poison_source_stages();throw std::runtime_error("original source stage key capacity exceeded");
}
void OriginalGpuEdge::guard_source_filter(unsigned stage,const TextureFilterClass* filter)
{
    guard_source_stage(stage);
    if (!source_stage_attempt_) return;
    const auto* source=source_stage_attempt_->selections[stage];
    if (source && &const_cast<TextureBaseClass*>(source)->As_TextureClass()->Get_Filter()==filter) return;
    poison_source_stages();throw std::runtime_error("original filter is outside its declared resident provider");
}
void OriginalGpuEdge::guard_source_transform(int transform)
{
    if (!source_stage_attempt_) return;
    if (transform<D3DTS_TEXTURE0 || transform>=D3DTS_TEXTURE0+8) { guard_nonstage_mutation();return; }
    guard_source_stage(transform-D3DTS_TEXTURE0);
    unsigned count=0;
    for (unsigned stage=0;stage<8;++stage) if ((source_stage_attempt_->token.mask&(1U<<stage))
        && DX8Wrapper::Source_Transform_Present(D3DTS_TEXTURE0+stage)) ++count;
    if (DX8Wrapper::Source_Transform_Present(transform) || count<source_stage_attempt_->transform_nodes) return;
    poison_source_stages();throw std::runtime_error("original source transform checkpoint capacity exceeded");
}
static bool Same_Source_Stage_Token(const OriginalGpuEdge::SourceStageToken& left,const OriginalGpuEdge::SourceStageToken& right) noexcept
{ return left.owner==right.owner && left.sequence==right.sequence && left.generation==right.generation && left.mask==right.mask; }
void OriginalGpuEdge::apply_source_stages(const SourceStageToken& token)
{
    if (!source_stage_attempt_ || !Same_Source_Stage_Token(token,source_stage_attempt_->token))
        throw std::runtime_error("original source stage apply token is foreign");
    if (source_stage_attempt_->failed) throw std::runtime_error("original source stage attempt is poisoned");
    try {
        for (unsigned stage=0;stage<8;++stage) if (token.mask&(1U<<stage)) {
            if (DX8Wrapper::Peek_Texture(stage)!=source_stage_attempt_->selections[stage])
                { poison_source_stages();throw std::runtime_error("original source stage declared selection was not staged"); }
        }
        DX8Wrapper::Apply_Source_Stages(token.mask);source_stage_attempt_->applied=true;
    } catch (...) { poison_source_stages();throw; }
}
bool OriginalGpuEdge::commit_source_stages(const SourceStageToken& token) noexcept
{
    if (!source_stage_attempt_ || !Same_Source_Stage_Token(token,source_stage_attempt_->token)
        || source_stage_attempt_->failed || !source_stage_attempt_->applied) return false;
    if (source_stage_commit_fault_) { source_stage_commit_fault_=false;poison_source_stages();return false; }
    if (!commit_device_transaction(source_stage_attempt_->device)) { poison_source_stages();return false; }
    if (source_stage_attempt_->provider_count && !finish_source_references(source_stage_attempt_->refs)) std::terminate();
    source_stage_attempt_.reset();return true;
}
bool OriginalGpuEdge::abort_source_stages(const SourceStageToken& token) noexcept
{
    if (!source_stage_attempt_ || !Same_Source_Stage_Token(token,source_stage_attempt_->token)) return false;
    auto& owner=*source_stage_attempt_;
    if (!abort_device_transaction(owner.device)) return false;
    DX8Wrapper::Restore_Source_Stages(*owner.checkpoint);
    for (unsigned stage=0;stage<8;++stage) if (token.mask&(1U<<stage)) {
        pending_stages_[stage]=owner.pending[stage];pending_filter_values_[stage]=owner.filters[stage];
    }
    source_revision_=owner.revision;
    for (unsigned i=0;i<owner.provider_count;++i) owner.providers[i]->LastAccessed=owner.access[i];
    if (owner.provider_count && !cancel_source_references(owner.refs)) std::terminate();
    source_stage_attempt_.reset();return true;
}

void OriginalGpuEdge::cancel_source_stages_for_reset()
{
    if (!source_stage_attempt_) return;
    const auto token=source_stage_attempt_->token;
    if (!abort_source_stages(token) || !drain_source_references(generation_))
        throw std::runtime_error("original source stage reset cleanup unavailable");
}

OriginalGpuEdge::OriginalGpuEdge(renderer::GpuDevice& device)
    : device_(device), previous_(active_edge), generation_(next_generation.fetch_add(1))
{
    if (previous_) throw std::runtime_error("nested original GPU translation session");
    active_edge = this;
    DX8Wrapper::Reset_Source_State();
}

OriginalGpuEdge::~OriginalGpuEdge()
{
    if (source_frame_attempt_ && !abort_tree_source_frame()) std::terminate();
    if (source_stage_attempt_ && !abort_source_stages(source_stage_attempt_->token)) std::terminate();
    if (device_transaction_) (void)abort_device_transaction(*device_transaction_);
    abort_source_frame();
    if (!shutdown_source_references()) {
        std::fputs("original source-reference cleanup unavailable\n",stderr);
        std::terminate();
    }
    release_prepared_state();
    release_volume_stencil();
    DX8Wrapper::Reset_Source_State();
    for (auto& stage : pending_stages_) if (stage.sampler) device_.destroy(stage.sampler);
    while (!textures_.empty()) {
		const auto before = textures_.size();
        textures_.begin()->first->Invalidate();
		if (textures_.size() == before) std::terminate();
    }
    if (!texture_owner_refs_.empty()) std::terminate();
    release_source_buffers();
    if (missing_texture_) device_.destroy(missing_texture_);
    active_edge = previous_;
}

void OriginalGpuEdge::release_source_buffers()
{
    guard_nonstage_mutation();
    if (source_frame_active_)
        throw std::runtime_error("original source buffers cannot retire during a frame");
    for (auto& entry : indices_) { device_.destroy(entry.second); entry.first->Release_Ref(); }
    indices_.clear();
    for (auto& entry : vertices_) { device_.destroy(entry.second); entry.first->Release_Ref(); }
    vertices_.clear();
}

OriginalGpuEdge& OriginalGpuEdge::required()
{
    if (!active_edge) throw std::runtime_error("original physical call requires a GPU translation session");
    return *active_edge;
}

OriginalGpuEdge* OriginalGpuEdge::active() noexcept
{
    return active_edge;
}

bool OriginalGpuEdge::supports_device_transactions(renderer::DeviceTransactionMode mode) const noexcept
{ return device_.supports_device_transactions(mode); }

bool OriginalGpuEdge::idle_preparation_ready() const noexcept
{
    return active_edge==this && !device_transaction_ && !source_stage_attempt_
        && !source_frame_active_ && !source_reference_queued_ && !device_.pass_active()
        && device_.supports_device_transactions(renderer::DeviceTransactionMode::idle_preparation);
}
bool OriginalGpuEdge::resident_texture(const TextureBaseClass* source) const noexcept
{
    if (active_edge!=this || !source) return false;
    const auto owned=textures_.find(const_cast<TextureBaseClass*>(source));
    return owned!=textures_.end() && owned->second.generation==generation_
        && !owned->second.shared_missing && device_.describe_texture_format(owned->second.handle).has_value();
}

renderer::ValidationResult OriginalGpuEdge::begin_device_transaction(
    const renderer::DeviceTransactionDesc& desc,renderer::DeviceTransactionToken& token)
{
    if (device_transaction_ || source_frame_active_ || device_.pass_active() || !device_.supports_device_transactions(desc.mode)
        || (desc.mode == renderer::DeviceTransactionMode::frame_commands
            && (!bound_frame_ || desc.generation != frame_target_generation_))
        || (desc.mode == renderer::DeviceTransactionMode::idle_preparation && desc.generation != generation_))
        return {false,"original device transaction has an unsupported mode or mismatched owner generation"};
    const auto result=device_.begin_device_transaction(desc,token);
    if (result) device_transaction_=token;
    return result;
}

bool OriginalGpuEdge::commit_device_transaction(const renderer::DeviceTransactionToken& token) noexcept
{
    if (!device_transaction_ || source_frame_active_ || device_.pass_active()) return false;
    const auto& owner=*device_transaction_;
    if (token.device!=owner.device || token.sequence!=owner.sequence || token.generation!=owner.generation
        || token.mode!=owner.mode || !device_.commit_device_transaction(token)) return false;
    device_transaction_.reset();return true;
}

bool OriginalGpuEdge::abort_device_transaction(const renderer::DeviceTransactionToken& token) noexcept
{
    if (!device_transaction_) return false;
    const auto owner=*device_transaction_;
    if (token.device!=owner.device || token.sequence!=owner.sequence || token.generation!=owner.generation
        || token.mode!=owner.mode || !device_.abort_device_transaction(token)) return false;
    device_transaction_.reset();
    // The admitted device has restored its inactive-pass baseline. Do not
    // issue a fallible ordinary end_pass after aborting the journal.
    source_frame_active_=false; source_viewport_.reset();
    return true;
}

renderer::ValidationResult OriginalGpuEdge::begin_source_references(std::uint64_t generation,
    TextureBaseClass* const* sources,unsigned count,SourceReferenceToken& token)
{
    if (active_edge!=this || generation!=generation_ || !generation || !sources || !count
        || count>source_reference_capacity-source_reference_queued_ || source_reference_token_.units
        || source_reference_sequence_==std::numeric_limits<std::uint64_t>::max()
        || device_transaction_ || source_frame_active_ || device_.pass_active()
        || !device_.supports_device_transactions(renderer::DeviceTransactionMode::idle_preparation))
        return {false,"source reference admission unavailable"};
    // Membership precedes every provider dereference; no lazy source lookup.
    for (unsigned i=0;i<count;++i) {
        const auto found=textures_.find(sources[i]);
        if (!sources[i] || found==textures_.end() || found->second.generation!=generation_
            || !found->second.handle || !device_.describe_texture_format(found->second.handle)
            || !sources[i]->As_TextureClass() || !sources[i]->Is_Initialized()
            || sources[i]->Num_Refs()<=0)
            return {false,"source reference provider unavailable"};
        unsigned units=1;
        for (unsigned j=0;j<i;++j) if (sources[j]==sources[i]) ++units;
        if (static_cast<unsigned>(sources[i]->Num_Refs())>
            static_cast<unsigned>(std::numeric_limits<int>::max())-units)
            return {false,"source reference count exhausted"};
    }
    for (unsigned i=0;i<count;++i) { sources[i]->Add_Ref();source_reference_pins_[i]=sources[i]; }
    source_reference_token_={static_cast<std::uint64_t>(reinterpret_cast<std::uintptr_t>(this)),
        ++source_reference_sequence_,generation_,count};
    token=source_reference_token_;
    return {};
}

bool OriginalGpuEdge::finish_source_references(const SourceReferenceToken& token) noexcept
{
    const auto& owner=source_reference_token_;
    if (!owner.units || token.owner!=owner.owner || token.sequence!=owner.sequence
        || token.generation!=owner.generation || token.units!=owner.units || active_edge!=this)
        return false;
    for (unsigned i=0;i<owner.units;++i) {
        source_reference_queue_[source_reference_queued_++]=source_reference_pins_[i];
        source_reference_pins_[i]=nullptr;
    }
    source_reference_token_={};
    return true;
}
bool OriginalGpuEdge::cancel_source_references(const SourceReferenceToken& token) noexcept
{ return finish_source_references(token); }

bool OriginalGpuEdge::drain_source_references(std::uint64_t generation) noexcept
{
    if (active_edge!=this || generation!=generation_ || source_reference_token_.units
        || device_transaction_ || source_frame_active_ || device_.pass_active()) return false;
    if (!source_reference_queued_) return true;
    struct Source { TextureBaseClass* object=nullptr;unsigned units=0;bool terminal=false,owned=false; };
    struct Native { renderer::TextureHandle handle;unsigned owners=0; };
    std::array<Source,source_reference_capacity> sources{};
    std::array<Native,source_reference_capacity> natives{};
    unsigned source_count=0,native_count=0,terminal_owned=0;
    bool has_terminal=false;
    for (unsigned i=0;i<source_reference_queued_;++i) {
        auto* object=source_reference_queue_[i];
        unsigned j=0;while (j<source_count && sources[j].object!=object) ++j;
        if (j==source_count) { sources[j].object=object;++source_count; }
        ++sources[j].units;
    }
    for (unsigned i=0;i<source_count;++i) {
        auto& source=sources[i];
        if (!source.object || source.object->Num_Refs()<static_cast<int>(source.units)) return false;
        source.terminal=source.object->Num_Refs()==static_cast<int>(source.units);
        if (!source.terminal) continue;
        has_terminal=true;
        const auto found=textures_.find(source.object);
        if (found==textures_.end()) continue; // previously invalidated while still strongly pinned
        if (found->second.generation!=generation_ || !found->second.handle) return false;
        source.owned=true;++terminal_owned;
        if (found->second.shared_missing) continue;
        unsigned j=0;while (j<native_count && natives[j].handle!=found->second.handle) ++j;
        if (j==native_count) { natives[j].handle=found->second.handle;++native_count; }
        ++natives[j].owners;
    }
    if (terminal_owned>std::numeric_limits<std::uint64_t>::max()-source_revision_
        || source_reference_queued_>std::numeric_limits<std::uint64_t>::max()-source_reference_releases_)
        return false;
    const auto release_units=[&]() noexcept {
        const auto units=source_reference_queued_;
        source_reference_queued_=0;source_reference_releases_+=units;
        for (unsigned i=0;i<units;++i) {
            auto* object=source_reference_queue_[i];source_reference_queue_[i]=nullptr;
            object->Release_Ref();
        }
    };
    // Grouped ownership proves every decrement leaves a live external/source
    // reference. No destructor, device admission, callback or fault consumption
    // is possible; cancellation remains usable after persistent device rejection.
    if (!has_terminal) { release_units();return true; }
    for (unsigned i=0;i<native_count;++i) {
        const auto found=texture_owner_refs_.find(natives[i].handle.value());
        if (found==texture_owner_refs_.end() || found->second<natives[i].owners) return false;
    }
    renderer::DeviceTransactionToken device_token{};
    bool device_started=false;
    try {
        const renderer::DeviceTransactionDesc desc{renderer::DeviceTransactionMode::idle_preparation,
            generation_,4096,4096,64ULL*1024*1024,0};
        if (!begin_device_transaction(desc,device_token)) return false;
        device_started=true;
        for (unsigned stage=0;stage<pending_stages_.size();++stage) {
            for (unsigned i=0;i<source_count;++i)
                if (sources[i].terminal && sources[i].owned
                    && pending_stages_[stage].source==sources[i].object && pending_stages_[stage].sampler)
                    device_.destroy(pending_stages_[stage].sampler);
        }
        for (unsigned i=0;i<native_count;++i)
            if (texture_owner_refs_.find(natives[i].handle.value())->second==natives[i].owners)
                device_.destroy(natives[i].handle);
        if (source_reference_commit_fault_) { source_reference_commit_fault_=false;
            (void)abort_device_transaction(device_token);return false; }
        if (!commit_device_transaction(device_token)) {
            (void)abort_device_transaction(device_token);return false;
        }
    } catch (...) {
        if (device_started) (void)abort_device_transaction(device_token);
        return false;
    }
    // All fallible device cleanup is complete. Detach metadata before the
    // final reference can invoke its original pooled destructor callback.
    for (unsigned i=0;i<source_count;++i) if (sources[i].terminal && sources[i].owned) {
        for (unsigned stage=0;stage<pending_stages_.size();++stage)
            if (pending_stages_[stage].source==sources[i].object) {
                pending_stages_[stage]={};pending_filter_values_[stage]={};
            }
        textures_.erase(sources[i].object);
    }
    for (unsigned i=0;i<native_count;++i) {
        const auto found=texture_owner_refs_.find(natives[i].handle.value());
        found->second-=natives[i].owners;
        if (!found->second) texture_owner_refs_.erase(found);
    }
    source_revision_+=terminal_owned;
    release_units();
    return true;
}

bool OriginalGpuEdge::shutdown_source_references() noexcept
{
    if (source_reference_token_.units && !cancel_source_references(source_reference_token_)) return false;
    return drain_source_references(generation_);
}
bool OriginalGpuEdge::source_texture_references_pending(const TextureBaseClass* source) noexcept
{
    if (!active_edge) return false;
    for (unsigned i=0;i<active_edge->source_reference_token_.units;++i)
        if (active_edge->source_reference_pins_[i]==source) return true;
    for (unsigned i=0;i<active_edge->source_reference_queued_;++i)
        if (active_edge->source_reference_queue_[i]==source) return true;
    return false;
}
void OriginalGpuEdge::notify_source_texture_invalidation(TextureBaseClass* source)
{
    if (!source_texture_references_pending(source)) return;
    if (active_edge->source_stages_active()) active_edge->cancel_source_stages_for_reset();
    if (!active_edge->shutdown_source_references())
        throw std::runtime_error("source reference invalidation cleanup unavailable");
}

renderer::OriginalFvfLayout OriginalGpuEdge::layout_for_fvf(unsigned source_fvf)
{
    // The original FVFInfoClass computes the offsets/stride; these bits only
    // select which source-owned attributes exist and their dimensionality.
    const unsigned coordinates = source_fvf & 0x400eU;
    const unsigned count = (source_fvf >> 8U) & 0xfU;
    if (coordinates != 0x002U || count > 8 || (source_fvf & 0x1000U)
        || (source_fvf & 0x000020U))
        throw std::runtime_error("unsupported original FVF position or blend layout");
    constexpr unsigned known = 0x002U | 0x010U | 0x040U | 0x080U | 0xf00U;
    unsigned dimension_bits = 0;
    for (unsigned i = 0; i < count; ++i) dimension_bits |= 3U << (16U + 2U * i);
    if (source_fvf & ~(known | dimension_bits))
        throw std::runtime_error("unsupported original FVF element combination");
    const FVFInfoClass source(source_fvf);
    renderer::OriginalFvfLayout result;
    result.stride = source.Get_FVF_Size();
    const auto add = [&](unsigned location, renderer::VertexElementFormat format, unsigned offset) {
        if (result.attribute_count >= result.attributes.size())
            throw std::runtime_error("original FVF exceeds vertex attribute limit");
        result.attributes[result.attribute_count++] = {static_cast<renderer::UInt8>(location), format, offset};
    };
    add(0, renderer::VertexElementFormat::float3, source.Get_Location_Offset());
    if (source_fvf & 0x010U) add(1, renderer::VertexElementFormat::float3, source.Get_Normal_Offset());
    if (source_fvf & 0x040U) add(2, renderer::VertexElementFormat::ubyte4_norm, source.Get_Diffuse_Offset());
    if (source_fvf & 0x080U) add(3, renderer::VertexElementFormat::ubyte4_norm, source.Get_Specular_Offset());
    for (unsigned i = 0; i < count; ++i) {
        const unsigned size_code = (source_fvf >> (16U + 2U * i)) & 3U;
        const auto format = size_code == 3 ? renderer::VertexElementFormat::float1
            : size_code == 1 ? renderer::VertexElementFormat::float3
            : size_code == 2 ? renderer::VertexElementFormat::float4
            : renderer::VertexElementFormat::float2;
        add(4 + i, format, source.Get_Tex_Offset(i));
    }
    renderer::PipelineDesc probe;
    probe.vertex_shader = renderer::ShaderHandle(1);
    probe.fragment_shader = renderer::ShaderHandle(2);
    probe.vertex_layout = renderer::VertexLayout::original_fvf;
    probe.original_fvf = result;
    if (auto status = renderer::validate(probe); !status)
        throw std::runtime_error("unsupported original FVF device layout: " + status.error);
    return result;
}

OriginalGpuEdge::AppliedState OriginalGpuEdge::map_applied_state(unsigned source_fvf)
{
    return map_state(source_fvf,false);
}

OriginalGpuEdge::AppliedState OriginalGpuEdge::map_state(unsigned source_fvf,bool tree_program)
{
    (void)required();
    const auto source=DX8Wrapper::Snapshot_Source_State();
    if (!source.material_applied)
        throw std::runtime_error("original material has not issued its physical state");
    AppliedState result;
    result.pipeline.vertex_layout=renderer::VertexLayout::original_fvf;
    result.pipeline.original_fvf=layout_for_fvf(source_fvf);
    const auto render=[&](unsigned key) {
        auto found=source.render.find(key);
        if (found==source.render.end())
            throw std::runtime_error("original applied render state is absent: "+std::to_string(key));
        return found->second;
    };
    const auto boolean=[&](unsigned key) {
        const unsigned value=render(key);
        if (value>1) throw std::runtime_error("original applied boolean render state is invalid");
        return value!=0;
    };
    const auto compare=[](unsigned value) {
        switch (value) {
        case 1: return renderer::CompareOp::never;
        case 2: return renderer::CompareOp::less;
        case 3: return renderer::CompareOp::equal;
        case 4: return renderer::CompareOp::less_equal;
        case 5: return renderer::CompareOp::greater;
        case 6: return renderer::CompareOp::not_equal;
        case 7: return renderer::CompareOp::greater_equal;
        case 8: return renderer::CompareOp::always;
        default: throw std::runtime_error("original comparison mode is unsupported by public GPU");
        }
    };
    const auto blend=[](unsigned value) {
        switch (value) {
        case D3DBLEND_ZERO: return renderer::BlendFactor::zero;
        case D3DBLEND_ONE: return renderer::BlendFactor::one;
        case D3DBLEND_SRCCOLOR: return renderer::BlendFactor::src_color;
        case D3DBLEND_INVSRCCOLOR: return renderer::BlendFactor::inv_src_color;
        case D3DBLEND_SRCALPHA: return renderer::BlendFactor::src_alpha;
        case D3DBLEND_INVSRCALPHA: return renderer::BlendFactor::inv_src_alpha;
        case D3DBLEND_DESTCOLOR: return renderer::BlendFactor::dst_color;
        default: throw std::runtime_error("original blend factor is unsupported by public GPU");
        }
    };
    result.pipeline.blend.enabled=boolean(D3DRS_ALPHABLENDENABLE);
    if (result.pipeline.blend.enabled) {
        const auto src=blend(render(D3DRS_SRCBLEND));
        const auto dst=blend(render(D3DRS_DESTBLEND));
        result.pipeline.blend.source_color=result.pipeline.blend.source_alpha=src;
        result.pipeline.blend.destination_color=result.pipeline.blend.destination_alpha=dst;
    }
    result.pipeline.depth_stencil.depth_compare=compare(render(D3DRS_ZFUNC));
    result.pipeline.depth_stencil.depth_write=boolean(D3DRS_ZWRITEENABLE);
    const auto cull=render(D3DRS_CULLMODE);
    if (cull==D3DCULL_NONE) result.pipeline.raster.cull=renderer::CullMode::none;
    else if (cull==D3DCULL_CW) result.pipeline.raster.cull=renderer::CullMode::clockwise;
    else if (cull==D3DCULL_CCW) result.pipeline.raster.cull=renderer::CullMode::counter_clockwise;
    else throw std::runtime_error("original cull mode is unsupported by public GPU");
    // D3D8's positive integer ZBIAS pulls coplanar decals toward the viewer;
    // public SDL_GPU/Vulkan depth-bias constant factors have the opposite sign.
    // The canonical wrapper admits only source 0 and the decal renderer's 8.
    const auto zbias=source.render.find(D3DRS_ZBIAS);
    if (zbias!=source.render.end()) {
        if (zbias->second!=0U && zbias->second!=8U)
            throw std::runtime_error("original ZBIAS has no bounded public GPU mapping");
        result.pipeline.raster.depth_bias=zbias->second ? -static_cast<float>(zbias->second) : 0.0f;
    }
    result.lighting=boolean(D3DRS_LIGHTING);
    result.specular_enabled=boolean(D3DRS_SPECULARENABLE);
    result.color_vertex=boolean(D3DRS_COLORVERTEX);
    result.local_viewer=boolean(D3DRS_LOCALVIEWER);
    result.normalize_normals=boolean(D3DRS_NORMALIZENORMALS);
    result.ambient_source=render(D3DRS_AMBIENTMATERIALSOURCE);
    result.diffuse_source=render(D3DRS_DIFFUSEMATERIALSOURCE);
    result.specular_source=render(D3DRS_SPECULARMATERIALSOURCE);
    result.emissive_source=render(D3DRS_EMISSIVEMATERIALSOURCE);
    if (result.ambient_source>2 || result.diffuse_source>2 ||
        result.specular_source>2 || result.emissive_source>2)
        throw std::runtime_error("original material color selector is unsupported");
    if (result.lighting) {
        if (!(source_fvf&0x10U))
            throw std::runtime_error("original lighting requires source FVF normal");
        if (source.light_environment_selected) {
            const unsigned ambient=render(D3DRS_AMBIENT);
            result.global_ambient={static_cast<float>((ambient>>16)&255)/255.0f,
                static_cast<float>((ambient>>8)&255)/255.0f,
                static_cast<float>(ambient&255)/255.0f,1.0f};
        }
        result.lights=source.lights;
        result.light_enabled=source.light_enabled;
        result.light_environment_selected=source.light_environment_selected;
        const auto source_matrix=[&](int key,std::array<float,16>& output) {
            const auto found=source.transforms.find(key);
            if (found==source.transforms.end()) return false;
            for (unsigned row=0;row<4;++row) {
                const auto& r=found->second[row];
                const float components[]={r.X,r.Y,r.Z,r.W};
                for (unsigned col=0;col<4;++col) {
                    if (!std::isfinite(components[col]))
                        throw std::runtime_error("original lit source transform is not finite");
                    output[row*4+col]=components[col];
                }
            }
            return true;
        };
        result.source_world_set=source_matrix(D3DTS_WORLD,result.source_world);
        result.source_view_set=source_matrix(D3DTS_VIEW,result.source_view);
    }
    const auto material=[](const D3DCOLORVALUE& value) {
        std::array<float,4> components{value.r,value.g,value.b,value.a};
        for (float component:components) if (!std::isfinite(component))
            throw std::runtime_error("original material color is not finite");
        return components;
    };
    result.diffuse=material(source.material.Diffuse);
    result.ambient=material(source.material.Ambient);
    result.specular=material(source.material.Specular);
    result.emissive=material(source.material.Emissive);
    result.power=source.material.Power;
    if (!std::isfinite(result.power) || result.power<0)
        throw std::runtime_error("original material power is invalid");
    result.alpha_test=boolean(D3DRS_ALPHATESTENABLE);
    if (result.alpha_test) {
        result.alpha_compare=compare(render(D3DRS_ALPHAFUNC));
        const auto reference=render(D3DRS_ALPHAREF);
        if (reference>255) throw std::runtime_error("original alpha reference exceeds byte range");
        result.alpha_reference=static_cast<float>(reference)/255.0f;
    }
    result.pipeline.fog_enabled=boolean(D3DRS_FOGENABLE);
    if (result.pipeline.fog_enabled) {
        const auto color=render(D3DRS_FOGCOLOR);
        result.fog_color={static_cast<float>((color>>16)&255)/255.0f,
            static_cast<float>((color>>8)&255)/255.0f,
            static_cast<float>(color&255)/255.0f,
            static_cast<float>((color>>24)&255)/255.0f};
        const unsigned start_bits=render(D3DRS_FOGSTART),end_bits=render(D3DRS_FOGEND);
        std::memcpy(&result.fog_start,&start_bits,sizeof(float));
        std::memcpy(&result.fog_end,&end_bits,sizeof(float));
        if (!std::isfinite(result.fog_start) || !std::isfinite(result.fog_end)
            || result.fog_end<=result.fog_start)
            throw std::runtime_error("original fog range is invalid");
    }
    const auto operation=[](unsigned value) {
        switch (value) {
        case D3DTOP_DISABLE: return CombinerOp::disable;
        case D3DTOP_SELECTARG1: return CombinerOp::select_first;
        case D3DTOP_SELECTARG2: return CombinerOp::select_second;
        case D3DTOP_MODULATE: return CombinerOp::modulate;
        case D3DTOP_ADD: return CombinerOp::add;
        default: throw std::runtime_error("original applied combiner operation is unsupported");
        }
    };
    const auto argument=[](unsigned value) {
        switch (value) {
        case D3DTA_DIFFUSE: return CombinerArg::diffuse;
        case D3DTA_CURRENT: return CombinerArg::current;
        case D3DTA_TEXTURE: return CombinerArg::texture;
        default: throw std::runtime_error("original applied combiner argument is unsupported");
        }
    };
    for (unsigned stage=0; stage<result.stages.size(); ++stage) {
        const auto& states=source.stages[stage];
        const auto stage_value=[&](unsigned key) {
            auto found=states.find(key);
            if (found==states.end()) throw std::runtime_error(
                "original applied combiner/mapper stage is absent: "+std::to_string(stage)+":"+std::to_string(key));
            return found->second;
        };
        auto& out=result.stages[stage];
        const auto channel=[&](unsigned op,unsigned first,unsigned second) {
            CombinerChannel translated;
            translated.op=operation(stage_value(op));
            if (translated.op!=CombinerOp::disable) {
                translated.first=argument(stage_value(first));
                if (translated.op!=CombinerOp::select_first)
                    translated.second=argument(stage_value(second));
            }
            return translated;
        };
        out.color=channel(D3DTSS_COLOROP,D3DTSS_COLORARG1,D3DTSS_COLORARG2);
        out.alpha=channel(D3DTSS_ALPHAOP,D3DTSS_ALPHAARG1,D3DTSS_ALPHAARG2);
        if (stage && result.stages[stage-1].color.op==CombinerOp::disable
            && (out.color.op!=CombinerOp::disable || out.alpha.op!=CombinerOp::disable))
            throw std::runtime_error("original enabled stage follows disabled color stage");
        const auto needs_texture=[](const CombinerChannel& channel) {
            if (channel.op==CombinerOp::disable) return false;
            if (channel.op==CombinerOp::select_first) return channel.first==CombinerArg::texture;
            if (channel.op==CombinerOp::select_second) return channel.second==CombinerArg::texture;
            return channel.first==CombinerArg::texture || channel.second==CombinerArg::texture;
        };
        out.texture_required=needs_texture(out.color)||needs_texture(out.alpha);
        const auto uv=stage_value(D3DTSS_TEXCOORDINDEX);
        out.uv_source=uv&0xffffU;
        out.coordinate_mode=uv&0xffff0000U;
        if (out.uv_source>=8 || out.coordinate_mode>D3DTSS_TCI_CAMERASPACEREFLECTIONVECTOR)
            throw std::runtime_error("original mapper coordinate selection is unsupported");
        const unsigned source_uv_count=(source_fvf>>8U)&0xfU;
        if (out.texture_required && out.coordinate_mode==D3DTSS_TCI_PASSTHRU
            && out.uv_source>=source_uv_count && !tree_program)
            throw std::runtime_error("original textured stage requests absent source UV coordinates");
        out.transform_flags=stage_value(D3DTSS_TEXTURETRANSFORMFLAGS);
        if (tree_program && out.texture_required &&
            (out.coordinate_mode!=D3DTSS_TCI_PASSTHRU || out.uv_source!=stage ||
             out.transform_flags!=D3DTTFF_DISABLE))
            throw std::runtime_error("original tree stage requires exact program UV output without a second transform");
        if (out.transform_flags!=D3DTTFF_DISABLE && out.transform_flags!=D3DTTFF_COUNT2 &&
            out.transform_flags!=D3DTTFF_COUNT3 &&
            out.transform_flags!=(D3DTTFF_PROJECTED|D3DTTFF_COUNT3))
            throw std::runtime_error("original mapper transform flags are unsupported");
        auto transform=source.transforms.find(D3DTS_TEXTURE0+stage);
        if (out.transform_flags!=D3DTTFF_DISABLE && transform==source.transforms.end())
            throw std::runtime_error("original enabled mapper transform is missing");
        if (transform!=source.transforms.end()) {
            out.transform_set=true;
            for (unsigned row=0;row<4;++row) {
                const auto& r=transform->second[row];
                const float values[]={r.X,r.Y,r.Z,r.W};
                for (unsigned col=0;col<4;++col) {
                    if (!std::isfinite(values[col])) throw std::runtime_error("original mapper transform is not finite");
                    out.transform[row*4+col]=values[col];
                }
            }
        }
        constexpr unsigned bump_keys[]={D3DTSS_BUMPENVMAT00,D3DTSS_BUMPENVMAT01,
            D3DTSS_BUMPENVMAT10,D3DTSS_BUMPENVMAT11};
        for (unsigned i=0;i<4;++i) {
            auto found=states.find(bump_keys[i]);
            if (found!=states.end()) {
                std::memcpy(&out.bump[i],&found->second,sizeof(float));
                if (!std::isfinite(out.bump[i])) throw std::runtime_error("original bump matrix is not finite");
            }
        }
    }
    return result;
}

void OriginalGpuEdge::release_prepared_state() noexcept
{
    if (!physical_) return;
    device_.destroy(physical_->pipeline);
    device_.destroy(physical_->vertex_uniform);
    device_.destroy(physical_->fragment_uniform);
    device_.destroy(physical_->vertex_shader);
    device_.destroy(physical_->fragment_shader);
    physical_.reset();
}

void OriginalGpuEdge::release_volume_stencil() noexcept
{
    if (source_stages_active()) { poison_source_stages(); return; }
    if (!volume_stencil_) return;
    device_.destroy(volume_stencil_->composite);
    device_.destroy(volume_stencil_->decrement);
    device_.destroy(volume_stencil_->increment);
    device_.destroy(volume_stencil_->fragment_shader);
    device_.destroy(volume_stencil_->vertex_shader);
    volume_stencil_.reset();
}

OriginalGpuEdge::PhysicalState OriginalGpuEdge::prepare_applied_state(unsigned source_fvf,
    renderer::PrimitiveTopology topology)
{
    return prepare_state(source_fvf,topology,nullptr);
}

OriginalGpuEdge::PhysicalState OriginalGpuEdge::prepare_tree_state(
    const VertexBufferClass* source,const TreeVertexUniform& constants)
{
    guard_nonstage_mutation();
    const auto* vertex_source=dynamic_cast<const DX8VertexBufferClass*>(source);
    if (!source || source->FVF_Info().Get_FVF()!=DX8_FVF_XYZNDUV1 ||
        !vertex_source || !vertex_source->Get_CPU_Vertex_Buffer() || !source->Get_Vertex_Count())
        throw std::runtime_error("original tree program requires exact source vertex bytes");
    for (const auto& row:constants.composite)
        for (float value:row)
            if (!std::isfinite(value)) throw std::runtime_error("original tree composite is not finite");
    for (const auto& wave:constants.sway) {
        for (float value:wave)
            if (!std::isfinite(value)) throw std::runtime_error("original tree sway is not finite");
        if (wave[3]!=0) throw std::runtime_error("original tree sway changes homogeneous position");
    }
    if (constants.sway[0]!=std::array<float,4>{})
        throw std::runtime_error("original tree c8 is not zero");
    for (unsigned i=0;i<4;++i)
        if (!std::isfinite(constants.shroud_offset[i]) || !std::isfinite(constants.shroud_scale[i]))
            throw std::runtime_error("original tree shroud constants are not finite");
    if (constants.shroud_offset[2]!=0 || constants.shroud_offset[3]!=0 ||
        constants.shroud_scale[0]<=0 || constants.shroud_scale[1]<=0 ||
        constants.shroud_scale[2]!=1 || constants.shroud_scale[3]!=1)
        throw std::runtime_error("original tree shroud constants are not canonical");
    const auto* bytes=vertex_source->Get_CPU_Vertex_Buffer();
    const auto& layout=source->FVF_Info();
    for (unsigned i=0;i<source->Get_Vertex_Count();++i) {
        const auto* vertex=bytes+i*layout.Get_FVF_Size();
        float position[3],packed[3],uv[2];
        std::memcpy(position,vertex+layout.Get_Location_Offset(),sizeof(position));
        std::memcpy(packed,vertex+layout.Get_Normal_Offset(),sizeof(packed));
        std::memcpy(uv,vertex+layout.Get_Tex_Offset(0),sizeof(uv));
        for (float value:position) if (!std::isfinite(value))
            throw std::runtime_error("original tree position is not finite");
        for (float value:packed) if (!std::isfinite(value))
            throw std::runtime_error("original tree packed slot is not finite");
        for (float value:uv) if (!std::isfinite(value))
            throw std::runtime_error("original tree atlas UV is not finite");
        if (packed[0]<0 || packed[0]>10 || std::floor(packed[0])!=packed[0])
            throw std::runtime_error("original tree sway slot is outside c8–18");
    }
    return prepare_state(DX8_FVF_XYZNDUV1,renderer::PrimitiveTopology::triangle_list,&constants);
}

OriginalGpuEdge::PreparedTreeProgram::PreparedTreeProgram(OriginalGpuEdge& edge) noexcept
    : owner_(&edge),generation_(edge.generation_) {}

OriginalGpuEdge::PreparedTreeProgram::~PreparedTreeProgram()
{
    if (OriginalGpuEdge::active()==owner_ && owner_->generation_==generation_) {
        // The terrain phase owns completion/cancellation at an inactive boundary.
        if (!owner_->source_buffers_retirable() || owner_->device_.pass_active()) std::terminate();
        auto& device=owner_->device_;
        if (state_.pipeline) device.destroy(state_.pipeline);
        if (state_.vertex_bindings.uniforms[0].buffer) device.destroy(state_.vertex_bindings.uniforms[0].buffer);
        if (state_.fragment_bindings.uniforms[0].buffer) device.destroy(state_.fragment_bindings.uniforms[0].buffer);
        for (auto sampler:samplers_) if (sampler) device.destroy(sampler);
        if (fragment_shader_) device.destroy(fragment_shader_);
        if (vertex_shader_) device.destroy(vertex_shader_);
    }
    for (auto* source:sources_) if (source) source->Release_Ref();
}

bool OriginalGpuEdge::immutable_tree_program_current(const PreparedTreeProgram& program) const noexcept
{
    if (active_edge!=this || program.owner_!=this || program.generation_!=generation_
        || !program.state_.pipeline || program.state_.generation!=generation_) return false;
    for (unsigned stage=0;stage<program.state_.fragment_bindings.texture_count;++stage) {
        const auto found=textures_.find(program.sources_[stage]);
        if (found==textures_.end() || found->second.generation!=generation_
            || found->second.handle!=program.state_.fragment_bindings.textures[stage]
            || !device_.describe_texture_format(found->second.handle)) return false;
    }
    return true;
}

bool OriginalGpuEdge::probe_tree_frame_admission()
{
    if (active_edge!=this || device_transaction_ || source_frame_active_ || device_.pass_active()
        || !bound_frame_ || !device_.supports_device_transactions(renderer::DeviceTransactionMode::frame_commands))
        return false;
    const auto color=device_.describe_texture_format(bound_frame_->color);
    const auto depth=device_.describe_texture_format(bound_frame_->depth);
    if (!color || !depth || (*color!=renderer::TextureFormat::rgba8 && *color!=renderer::TextureFormat::bgra8)
        || (*depth!=renderer::TextureFormat::depth16 && *depth!=renderer::TextureFormat::depth24_stencil8
            && *depth!=renderer::TextureFormat::depth32)) return false;
    renderer::DeviceTransactionDesc desc;
    desc.mode=renderer::DeviceTransactionMode::frame_commands;
    desc.generation=frame_target_generation_;desc.commands=4096;desc.resources=4096;
    desc.bytes=renderer::RendererLimits::maximum_upload_bytes;desc.views=renderer::RendererLimits::ordered_views;
    renderer::DeviceTransactionToken token;
    if (!begin_device_transaction(desc,token)) return false;
    if (!abort_device_transaction(token)) std::terminate();
    return true;
}

std::unique_ptr<OriginalGpuEdge::PreparedTreeProgram> OriginalGpuEdge::prepare_immutable_tree_program(
    const VertexBufferClass* source,const ImmutableTreeSnapshot& snapshot)
{
    guard_nonstage_mutation();
    const auto* vertex=dynamic_cast<const DX8VertexBufferClass*>(source);
    if (active_edge!=this || !idle_preparation_ready() || !bound_frame_ || !vertex
        || source->FVF_Info().Get_FVF()!=DX8_FVF_XYZNDUV1 || !source->Get_Vertex_Count()
        || !vertex->Get_CPU_Vertex_Buffer() || physical_serial_==std::numeric_limits<std::uint64_t>::max())
        throw std::runtime_error("original immutable tree program owner unavailable");
    const auto& constants=snapshot.vertex;
    for (const auto& row:constants.composite) for (float value:row)
        if (!std::isfinite(value)) throw std::runtime_error("original immutable tree composite invalid");
    for (const auto& wave:constants.sway) {
        for (float value:wave) if (!std::isfinite(value)) throw std::runtime_error("original immutable tree sway invalid");
        if (wave[3]!=0) throw std::runtime_error("original immutable tree homogeneous sway invalid");
    }
    if (constants.sway[0]!=std::array<float,4>{} || constants.shroud_offset[2]!=0
        || constants.shroud_offset[3]!=0 || constants.shroud_scale[0]<=0 || constants.shroud_scale[1]<=0
        || constants.shroud_scale[2]!=1 || constants.shroud_scale[3]!=1)
        throw std::runtime_error("original immutable tree constants noncanonical");
    for (unsigned i=0;i<4;++i)
        if (!std::isfinite(constants.shroud_offset[i]) || !std::isfinite(constants.shroud_scale[i]))
            throw std::runtime_error("original immutable tree shroud invalid");
    const auto& layout=source->FVF_Info();
    for (unsigned index=0;index<source->Get_Vertex_Count();++index) {
        const auto* bytes=vertex->Get_CPU_Vertex_Buffer()+index*layout.Get_FVF_Size();
        float values[8];
        std::memcpy(values,bytes+layout.Get_Location_Offset(),3*sizeof(float));
        std::memcpy(values+3,bytes+layout.Get_Normal_Offset(),3*sizeof(float));
        std::memcpy(values+6,bytes+layout.Get_Tex_Offset(0),2*sizeof(float));
        for (float value:values) if (!std::isfinite(value))
            throw std::runtime_error("original immutable tree vertex invalid");
        if (values[3]<0 || values[3]>10 || std::floor(values[3])!=values[3])
            throw std::runtime_error("original immutable tree packed slot invalid");
    }
    for (unsigned stage=0;stage<2;++stage) {
        auto* texture=snapshot.textures[stage];
        if (!resident_texture(texture) || !texture->Is_Initialized() || texture->Num_Refs()<=0
            || texture->Num_Refs()>std::numeric_limits<int>::max()-2)
            throw std::runtime_error("original immutable tree texture pin unavailable");
        const auto& operations=snapshot.fragment.stage_ops[stage];
        const auto& arguments=snapshot.fragment.stage_args[stage];
        if (operations!=std::array<std::int32_t,4>{static_cast<int>(CombinerOp::modulate),
                static_cast<int>(stage?CombinerOp::select_second:CombinerOp::modulate),1,D3DTTFF_DISABLE}
            || arguments!=std::array<std::int32_t,4>{static_cast<int>(CombinerArg::texture),
                static_cast<int>(stage?CombinerArg::current:CombinerArg::diffuse),static_cast<int>(CombinerArg::texture),
                static_cast<int>(stage?CombinerArg::current:CombinerArg::diffuse)})
            throw std::runtime_error("original immutable tree stage snapshot invalid");
    }
    if (snapshot.pipeline.vertex_layout!=renderer::VertexLayout::original_fvf
        || !(snapshot.pipeline.original_fvf==layout_for_fvf(DX8_FVF_XYZNDUV1))
        || snapshot.pipeline.topology!=renderer::PrimitiveTopology::triangle_list
        || snapshot.pipeline.fog_enabled || snapshot.pipeline.blend.enabled
        || !snapshot.pipeline.depth_stencil.depth_test || snapshot.pipeline.depth_stencil.stencil_test
        || !snapshot.pipeline.depth_stencil.depth_write
        || snapshot.pipeline.depth_stencil.depth_compare!=renderer::CompareOp::less_equal
        || snapshot.pipeline.raster.cull!=renderer::CullMode::none)
        throw std::runtime_error("original immutable tree pipeline snapshot invalid");
    const auto color=device_.describe_texture_format(bound_frame_->color);
    const auto depth=device_.describe_texture_format(bound_frame_->depth);
    if (!color || !depth) throw std::runtime_error("original immutable tree targets unavailable");
    std::unique_ptr<PreparedTreeProgram> candidate(new PreparedTreeProgram(*this));
    candidate->source_fvf_=DX8_FVF_XYZNDUV1;
    for (unsigned stage=0;stage<2;++stage) {
        snapshot.textures[stage]->Add_Ref();candidate->sources_[stage]=snapshot.textures[stage];
        candidate->state_.fragment_bindings.textures[stage]=texture_handle(snapshot.textures[stage]);
        candidate->samplers_[stage]=device_.create_sampler(snapshot.samplers[stage],"immutable source tree sampler");
        if (!candidate->samplers_[stage]) throw std::runtime_error("original immutable tree sampler failed");
        candidate->state_.fragment_bindings.samplers[stage]=candidate->samplers_[stage];
    }
    candidate->state_.fragment_bindings.texture_count=2;
    candidate->vertex_shader_=device_.create_shader({renderer::ShaderStage::vertex,"renderer/original_tree.vert",1,0},"immutable source tree vertex");
    if (!candidate->vertex_shader_) throw std::runtime_error("original immutable tree vertex shader failed");
    candidate->fragment_shader_=device_.create_shader({renderer::ShaderStage::fragment,"renderer/original_applied_3.frag",1,2},"immutable source tree fragment");
    if (!candidate->fragment_shader_) throw std::runtime_error("original immutable tree fragment shader failed");
    auto pipeline=snapshot.pipeline;
    pipeline.color_format=*color;pipeline.depth_format=*depth;
    pipeline.vertex_shader=candidate->vertex_shader_;pipeline.fragment_shader=candidate->fragment_shader_;
    candidate->state_.pipeline=device_.create_pipeline(renderer::PipelineKey(pipeline),"immutable source tree pipeline");
    if (!candidate->state_.pipeline) throw std::runtime_error("original immutable tree pipeline failed");
    const auto upload=[&](renderer::StageBindings& bindings,const void* bytes,std::size_t size) {
        auto handle=device_.create_buffer({size,renderer::BufferUsage::uniform,true},"immutable source tree constants");
        if (!handle) throw std::runtime_error("original immutable tree constant buffer failed");
        bindings.uniform_count=1;bindings.uniforms[0]={handle,0,size};
        if (!device_.upload({handle,size,0,size},bytes)) throw std::runtime_error("original immutable tree constant upload failed");
    };
    upload(candidate->state_.vertex_bindings,&constants,sizeof(constants));
    upload(candidate->state_.fragment_bindings,&snapshot.fragment,sizeof(snapshot.fragment));
    candidate->state_.generation=generation_;
    candidate->state_.serial=physical_serial_+1;candidate->state_.texture_mask=3;
    // The caller publishes this already-owned resource bundle with C1 only
    // after the real begin/abort probe. Generic physical_ is never touched.
    ++physical_serial_;
    return candidate;
}

std::unique_ptr<OriginalGpuEdge::PreparedTreeProgram> OriginalGpuEdge::prepare_immutable_tree_decal_program(
    const VertexBufferClass* source,const ImmutableTreeDecalSnapshot& snapshot)
{
    guard_nonstage_mutation();
    const auto* vertex=dynamic_cast<const DX8VertexBufferClass*>(source);
    if (active_edge!=this || !idle_preparation_ready() || !bound_frame_ || !vertex
        || source->FVF_Info().Get_FVF()!=DX8_FVF_XYZDUV1 || !source->Get_Vertex_Count()
        || !vertex->Get_CPU_Vertex_Buffer() || physical_serial_==std::numeric_limits<std::uint64_t>::max()
        || !resident_texture(snapshot.texture) || !snapshot.texture->Is_Initialized()
        || snapshot.texture->Num_Refs()<=0 || snapshot.texture->Num_Refs()>std::numeric_limits<int>::max()-2)
        throw std::runtime_error("original immutable tree decal owner unavailable");
    for (const auto* matrix:{&snapshot.vertex.world,&snapshot.vertex.view,&snapshot.vertex.projection})
        for (float value:*matrix) if (!std::isfinite(value))
            throw std::runtime_error("original immutable tree decal matrix invalid");
    const auto& layout=source->FVF_Info();
    for (unsigned index=0;index<source->Get_Vertex_Count();++index) {
        const auto* bytes=vertex->Get_CPU_Vertex_Buffer()+index*layout.Get_FVF_Size();
        float position[3],uv[2];std::memcpy(position,bytes+layout.Get_Location_Offset(),sizeof(position));
        std::memcpy(uv,bytes+layout.Get_Tex_Offset(0),sizeof(uv));
        for (float value:position) if (!std::isfinite(value)) throw std::runtime_error("original tree decal position invalid");
        for (float value:uv) if (!std::isfinite(value)) throw std::runtime_error("original tree decal UV invalid");
    }
    const auto& pipeline=snapshot.pipeline;
    if (pipeline.vertex_layout!=renderer::VertexLayout::original_fvf
        || !(pipeline.original_fvf==layout_for_fvf(DX8_FVF_XYZDUV1))
        || pipeline.topology!=renderer::PrimitiveTopology::triangle_list || pipeline.fog_enabled
        || !pipeline.blend.enabled || pipeline.blend.source_color!=renderer::BlendFactor::zero
        || pipeline.blend.destination_color!=renderer::BlendFactor::src_color
        || pipeline.blend.source_alpha!=renderer::BlendFactor::zero
        || pipeline.blend.destination_alpha!=renderer::BlendFactor::src_color
        || !pipeline.depth_stencil.depth_test || pipeline.depth_stencil.depth_write
        || pipeline.depth_stencil.depth_compare!=renderer::CompareOp::less_equal
        || pipeline.depth_stencil.stencil_test || pipeline.raster.cull!=renderer::CullMode::clockwise
        || snapshot.fragment.alpha_parameters[0]!=0
        || snapshot.fragment.stage_ops[0]!=std::array<std::int32_t,4>{static_cast<int>(CombinerOp::modulate),static_cast<int>(CombinerOp::modulate),1,0}
        || snapshot.fragment.stage_args[0]!=std::array<std::int32_t,4>{static_cast<int>(CombinerArg::texture),static_cast<int>(CombinerArg::diffuse),static_cast<int>(CombinerArg::texture),static_cast<int>(CombinerArg::diffuse)}
        || snapshot.fragment.stage_ops[1][0]!=static_cast<int>(CombinerOp::disable)
        || snapshot.sampler.address_u!=renderer::AddressMode::clamp_edge
        || snapshot.sampler.address_v!=renderer::AddressMode::clamp_edge || snapshot.sampler.maximum_lod!=0)
        throw std::runtime_error("original immutable tree decal source state invalid");
    const auto color=device_.describe_texture_format(bound_frame_->color),depth=device_.describe_texture_format(bound_frame_->depth);
    if (!color || !depth) throw std::runtime_error("original immutable tree decal targets unavailable");
    std::unique_ptr<PreparedTreeProgram> candidate(new PreparedTreeProgram(*this));
    candidate->source_fvf_=DX8_FVF_XYZDUV1;
    snapshot.texture->Add_Ref();candidate->sources_[0]=snapshot.texture;
    candidate->state_.fragment_bindings.textures[0]=texture_handle(snapshot.texture);
    candidate->samplers_[0]=device_.create_sampler(snapshot.sampler,"immutable source tree decal sampler");
    if (!candidate->samplers_[0]) throw std::runtime_error("original immutable tree decal sampler failed");
    candidate->state_.fragment_bindings.samplers[0]=candidate->samplers_[0];candidate->state_.fragment_bindings.texture_count=1;
    candidate->vertex_shader_=device_.create_shader({renderer::ShaderStage::vertex,"renderer/original_applied_d1.vert",1,0},"immutable tree decal vertex");
    if (!candidate->vertex_shader_) throw std::runtime_error("original immutable tree decal vertex failed");
    candidate->fragment_shader_=device_.create_shader({renderer::ShaderStage::fragment,"renderer/original_applied_1.frag",1,1},"immutable tree decal fragment");
    if (!candidate->fragment_shader_) throw std::runtime_error("original immutable tree decal fragment failed");
    auto descriptor=pipeline;descriptor.color_format=*color;descriptor.depth_format=*depth;
    descriptor.vertex_shader=candidate->vertex_shader_;descriptor.fragment_shader=candidate->fragment_shader_;
    candidate->state_.pipeline=device_.create_pipeline(renderer::PipelineKey(descriptor),"immutable tree decal pipeline");
    if (!candidate->state_.pipeline) throw std::runtime_error("original immutable tree decal pipeline failed");
    const auto upload=[&](renderer::StageBindings& bindings,const void* bytes,std::size_t size) {
        const auto handle=device_.create_buffer({size,renderer::BufferUsage::uniform,true},"immutable tree decal constants");
        if (!handle) throw std::runtime_error("original immutable tree decal constants failed");
        bindings.uniform_count=1;bindings.uniforms[0]={handle,0,size};
        if (!device_.upload({handle,size,0,size},bytes)) throw std::runtime_error("original immutable tree decal upload failed");
    };
    upload(candidate->state_.vertex_bindings,&snapshot.vertex,sizeof(snapshot.vertex));
    upload(candidate->state_.fragment_bindings,&snapshot.fragment,sizeof(snapshot.fragment));
    candidate->state_.generation=generation_;candidate->state_.serial=++physical_serial_;candidate->state_.texture_mask=1;
    return candidate;
}

OriginalGpuEdge::PhysicalState OriginalGpuEdge::prepare_state(unsigned source_fvf,
    renderer::PrimitiveTopology topology,const TreeVertexUniform* tree)
{
    guard_nonstage_mutation();
    const auto source_state=DX8Wrapper::Snapshot_Source_State();
    const auto source_lighting=source_state.render.find(D3DRS_LIGHTING);
    std::optional<AppliedState> lit_state;
    if (source_lighting!=source_state.render.end() && source_lighting->second==1) {
        if (tree) throw std::runtime_error("original tree packed slots cannot use fixed-function lighting");
        lit_state=map_applied_state(source_fvf);
        if (!lit_state->light_environment_selected)
            throw std::runtime_error("original lighting requires selected source light environment");
        if (source_fvf!=DX8_FVF_XYZN && source_fvf!=DX8_FVF_XYZNUV1 &&
            source_fvf!=DX8_FVF_XYZNUV2 && source_fvf!=DX8_FVF_XYZNDUV2)
            throw std::runtime_error("original lit physical FVF has no exact shader input variant");
        if (topology!=renderer::PrimitiveTopology::triangle_list &&
            topology!=renderer::PrimitiveTopology::triangle_strip)
            throw std::runtime_error("original lit indexed primitive topology is unsupported");
        for (unsigned slot=0;slot<4;++slot)
            if (lit_state->light_enabled[slot] &&
                lit_state->lights[slot].Type!=D3DLIGHT_POINT &&
                lit_state->lights[slot].Type!=D3DLIGHT_DIRECTIONAL)
                throw std::runtime_error("original lit physical light type is unsupported");
    }
    if (!tree) release_prepared_state(); // Keep the established generic contract.
    const AppliedState mapped=lit_state ? *lit_state : map_state(source_fvf,tree!=nullptr);
    if (tree && (mapped.pipeline.fog_enabled || mapped.specular_enabled))
        throw std::runtime_error("original tree program requires source fog/specular disabled");
    if (topology!=renderer::PrimitiveTopology::triangle_list &&
        topology!=renderer::PrimitiveTopology::triangle_strip)
        throw std::runtime_error("original indexed primitive topology is unsupported");
    const char* vertex_variant=nullptr;
    if (tree) vertex_variant="renderer/original_tree.vert";
    else if (source_fvf==DX8_FVF_XYZDUV1) vertex_variant="renderer/original_applied_d1.vert";
    else if (source_fvf==DX8_FVF_XYZDUV2) vertex_variant="renderer/original_applied_d2.vert";
    else if (source_fvf==DX8_FVF_XYZN) vertex_variant=mapped.lighting?
        "renderer/original_applied_n0_lit.vert":"renderer/original_applied_n0.vert";
    else if (source_fvf==DX8_FVF_XYZNUV1) vertex_variant=mapped.lighting?
        "renderer/original_applied_n1_lit.vert":"renderer/original_applied_n1.vert";
    else if (source_fvf==DX8_FVF_XYZNUV2) vertex_variant=mapped.lighting?
        "renderer/original_applied_n2_lit.vert":"renderer/original_applied_n2.vert";
    else if (source_fvf==DX8_FVF_XYZNDUV2) vertex_variant=mapped.lighting?
        "renderer/original_applied_nd2_lit.vert":"renderer/original_applied_nd2.vert";
    else throw std::runtime_error("original physical FVF has no exact shader input variant: "+
        std::to_string(source_fvf));
    for (const auto& stage:mapped.stages)
        if ((stage.coordinate_mode==D3DTSS_TCI_CAMERASPACENORMAL ||
             stage.coordinate_mode==D3DTSS_TCI_CAMERASPACEREFLECTIONVECTOR) &&
            !(source_fvf&0x10U) && stage.texture_required)
            throw std::runtime_error("original camera-normal UV mapper requires source FVF normal");
    const auto source=DX8Wrapper::Snapshot_Source_State();
    VertexUniform vertex;
    const auto matrix=[&](int key,std::array<float,16>& output) {
        auto found=source.transforms.find(key);
        if (found==source.transforms.end())
            throw std::runtime_error("original physical state is missing a source-issued world/view/projection transform");
        for (unsigned row=0;row<4;++row) {
            const auto& r=found->second[row];
            const float values[]={r.X,r.Y,r.Z,r.W};
            for (unsigned col=0;col<4;++col) {
                if (!std::isfinite(values[col]))
                    throw std::runtime_error("original physical transform is not finite");
                output[row*4+col]=values[col];
            }
        }
    };
    matrix(D3DTS_WORLD,vertex.world);
    matrix(D3DTS_VIEW,vertex.view);
    matrix(D3DTS_PROJECTION,vertex.projection);
    if (mapped.lighting) {
        vertex.lit_diffuse=mapped.diffuse;
        vertex.lit_ambient=mapped.ambient;
        vertex.lit_specular=mapped.specular;
        vertex.lit_specular[3]=mapped.power;
        vertex.lit_emissive=mapped.emissive;
        vertex.lit_global_ambient=mapped.global_ambient;
        vertex.lit_switches={mapped.normalize_normals?1:0,mapped.local_viewer?1:0,
            mapped.specular_enabled?1:0,mapped.color_vertex?1:0};
        vertex.lit_material_sources={static_cast<std::int32_t>(mapped.ambient_source),
            static_cast<std::int32_t>(mapped.diffuse_source),
            static_cast<std::int32_t>(mapped.specular_source),
            static_cast<std::int32_t>(mapped.emissive_source)};
        for (unsigned slot=0;slot<4;++slot) {
            if (!mapped.light_enabled[slot]) continue;
            const auto& light=mapped.lights[slot];
            if (light.Type!=D3DLIGHT_POINT && light.Type!=D3DLIGHT_DIRECTIONAL)
                throw std::runtime_error("original lit physical light type is unsupported");
            vertex.light_position_range[slot]={light.Position.x,light.Position.y,
                light.Position.z,light.Range};
            vertex.light_direction_attenuation0[slot]={light.Direction.x,light.Direction.y,
                light.Direction.z,light.Attenuation0};
            vertex.light_diffuse_attenuation1[slot]={light.Diffuse.r,light.Diffuse.g,
                light.Diffuse.b,light.Attenuation1};
            vertex.light_ambient_attenuation2[slot]={light.Ambient.r,light.Ambient.g,
                light.Ambient.b,light.Attenuation2};
            vertex.light_specular_type[slot]={light.Specular.r,light.Specular.g,
                light.Specular.b,static_cast<float>(light.Type)};
        }
    }
    FragmentUniform fragment;
    fragment.diffuse=mapped.diffuse;
    fragment.ambient=mapped.ambient;
    fragment.specular=mapped.specular;
    fragment.emissive=mapped.emissive;
    fragment.fog_color=mapped.fog_color;
    fragment.fog_parameters={mapped.fog_start,mapped.fog_end,
        mapped.pipeline.fog_enabled?1.0f:0.0f,mapped.power};
    fragment.alpha_parameters={mapped.alpha_test?1.0f:0.0f,
        static_cast<float>(static_cast<unsigned>(mapped.alpha_compare)),
        mapped.alpha_reference,mapped.specular_enabled?1.0f:0.0f};
    fragment.material_sources={static_cast<std::int32_t>(mapped.ambient_source),
        static_cast<std::int32_t>(mapped.diffuse_source),
        static_cast<std::int32_t>(mapped.emissive_source),mapped.lighting?1:0};
    unsigned mask=0,slot=0;
    renderer::StageBindings fragment_bindings;
    std::array<const TextureBaseClass*,2> stage_sources{};
    for (unsigned stage=0;stage<2;++stage) {
        const auto& selected=mapped.stages[stage];
        vertex.texture_transform[stage]=selected.transform;
        vertex.coordinate_modes[stage]=static_cast<std::int32_t>(selected.coordinate_mode);
        vertex.uv_indices[stage]=static_cast<std::int32_t>(selected.uv_source);
        vertex.transform_flags[stage]=static_cast<std::int32_t>(selected.transform_flags);
        fragment.stage_ops[stage]={static_cast<std::int32_t>(selected.color.op),
            static_cast<std::int32_t>(selected.alpha.op),selected.texture_required?1:0,
            static_cast<std::int32_t>(selected.transform_flags)};
        fragment.stage_args[stage]={static_cast<std::int32_t>(selected.color.first),
            static_cast<std::int32_t>(selected.color.second),
            static_cast<std::int32_t>(selected.alpha.first),
            static_cast<std::int32_t>(selected.alpha.second)};
        fragment.bump[stage]=selected.bump;
        if (!selected.texture_required) continue;
        const auto pending=pending_stage(stage);
        if (!pending.source || !pending.texture || !pending.sampler ||
            pending.texture!=texture_handle(pending.source))
            throw std::runtime_error("original physical stage has no resident TextureClass/filter owner");
        mask |= 1U<<stage;
        fragment_bindings.textures[slot]=pending.texture;
        fragment_bindings.samplers[slot]=pending.sampler;
        stage_sources[stage]=pending.source;
        ++slot;
    }
    fragment_bindings.texture_count=slot;
    PhysicalResources next;
    next.vertex_uniform_size=tree?sizeof(TreeVertexUniform):sizeof(VertexUniform);
    try {
        next.vertex_shader=device_.create_shader(
            {renderer::ShaderStage::vertex,vertex_variant,1,0},"original applied vertex");
        if (!next.vertex_shader) throw std::runtime_error("original vertex shader creation failed: "+device_.last_error());
        const std::string fragment_name="renderer/original_applied_"+std::to_string(mask)+".frag";
        next.fragment_shader=device_.create_shader(
            {renderer::ShaderStage::fragment,fragment_name,1,slot},"original applied fragment");
        if (!next.fragment_shader) throw std::runtime_error("original fragment shader creation failed: "+device_.last_error());
        auto pipeline=mapped.pipeline;
        pipeline.topology=topology;
        if (source_frame_active_ && bound_frame_) {
            const auto color_format=device_.describe_texture_format(bound_frame_->color);
            const auto depth_format=device_.describe_texture_format(bound_frame_->depth);
            if (!color_format || !depth_format)
                throw std::runtime_error("original source pipeline target is stale");
            if (*color_format!=renderer::TextureFormat::rgba8 &&
                *color_format!=renderer::TextureFormat::bgra8)
                throw std::runtime_error("original source pipeline color target is unsupported");
            pipeline.color_format=*color_format;
            pipeline.depth_format=*depth_format;
        }
        pipeline.vertex_shader=next.vertex_shader;
        pipeline.fragment_shader=next.fragment_shader;
        next.pipeline=device_.create_pipeline(renderer::PipelineKey(pipeline),"original applied pipeline");
        if (!next.pipeline) throw std::runtime_error("original applied pipeline creation failed: "+device_.last_error());
        next.vertex_uniform=device_.create_buffer({next.vertex_uniform_size,renderer::BufferUsage::uniform,true},
            "original world/view/projection and UV state");
        if (!next.vertex_uniform) throw std::runtime_error("original vertex uniform creation failed: "+device_.last_error());
        next.fragment_uniform=device_.create_buffer({sizeof(fragment),renderer::BufferUsage::uniform,true},
            "original material/shader/fog state");
        if (!next.fragment_uniform) throw std::runtime_error("original fragment uniform creation failed: "+device_.last_error());
        const void* vertex_bytes=tree?static_cast<const void*>(tree):static_cast<const void*>(&vertex);
        if (auto result=device_.upload({next.vertex_uniform,next.vertex_uniform_size,0,next.vertex_uniform_size},vertex_bytes); !result)
            throw std::runtime_error("original vertex uniform upload failed: "+result.error);
        if (auto result=device_.upload({next.fragment_uniform,sizeof(fragment),0,sizeof(fragment)},&fragment); !result)
            throw std::runtime_error("original fragment uniform upload failed: "+result.error);
        next.state.pipeline=next.pipeline;
        next.state.vertex_bindings.uniforms[0]={next.vertex_uniform,0,next.vertex_uniform_size};
        next.state.vertex_bindings.uniform_count=1;
        next.state.fragment_bindings=fragment_bindings;
        next.state.fragment_bindings.uniforms[0]={next.fragment_uniform,0,sizeof(fragment)};
        next.state.fragment_bindings.uniform_count=1;
        next.state.generation=generation_;
        next.state.serial=++physical_serial_;
        next.state.source_revision=source_revision_;
        next.state.texture_mask=mask;
        next.sources=stage_sources;
        if (tree) release_prepared_state(); // Candidate is complete; publication cannot allocate.
        physical_=next;
        return next.state;
    } catch (...) {
        if (next.pipeline) device_.destroy(next.pipeline);
        if (next.vertex_uniform) device_.destroy(next.vertex_uniform);
        if (next.fragment_uniform) device_.destroy(next.fragment_uniform);
        if (next.vertex_shader) device_.destroy(next.vertex_shader);
        if (next.fragment_shader) device_.destroy(next.fragment_shader);
        throw;
    }
}

void OriginalGpuEdge::validate_prepared_state(const PhysicalState& state) const
{
    if (!physical_ || state.generation!=generation_ || state.serial!=physical_->state.serial ||
        state.source_revision!=source_revision_ || state.pipeline!=physical_->pipeline ||
        state.texture_mask!=physical_->state.texture_mask ||
        state.vertex_bindings.uniform_count!=1 ||
        state.vertex_bindings.uniforms[0].buffer!=physical_->vertex_uniform ||
        state.vertex_bindings.uniforms[0].offset!=0 ||
        state.vertex_bindings.uniforms[0].size!=physical_->vertex_uniform_size ||
        state.vertex_bindings.texture_count!=0 ||
        state.fragment_bindings.uniform_count!=1 ||
        state.fragment_bindings.uniforms[0].buffer!=physical_->fragment_uniform ||
        state.fragment_bindings.uniforms[0].offset!=0 ||
        state.fragment_bindings.uniforms[0].size!=sizeof(FragmentUniform) ||
        state.fragment_bindings.texture_count!=physical_->state.fragment_bindings.texture_count)
        throw std::runtime_error("original prepared physical state is stale or from another source/device generation");
    for (unsigned binding=0;binding<state.fragment_bindings.texture_count;++binding)
        if (state.fragment_bindings.textures[binding]!=physical_->state.fragment_bindings.textures[binding] ||
            state.fragment_bindings.samplers[binding]!=physical_->state.fragment_bindings.samplers[binding])
            throw std::runtime_error("original prepared physical texture bindings are stale");
    for (unsigned stage=0;stage<2;++stage) {
        if (!(state.texture_mask&(1U<<stage))) continue;
        const auto selected=pending_stage(stage);
        if (selected.source!=physical_->sources[stage] ||
            selected.texture!=texture_handle(selected.source) || !selected.sampler)
            throw std::runtime_error("original prepared TextureClass owner is stale");
    }
}

bool OriginalGpuEdge::supports_texture_format(WW3DFormat format) const noexcept
{
    try { return device_.supports_texture_format(translate_format(format),renderer::TextureDimension::texture_2d,true,false); }
    catch (...) { return false; }
}

renderer::TextureHandle OriginalGpuEdge::create_texture(WW3DFormat format, unsigned width,
    unsigned height, unsigned& mips)
{
    guard_nonstage_mutation();
    const auto translated=translate_format(format);
    if (!device_.supports_texture_format(translated,renderer::TextureDimension::texture_2d,true,false))
        throw std::runtime_error("original texture format is unsupported by physical device");
    if (!width || !height || width>16384 || height>16384)
        throw std::runtime_error("original texture extent is invalid at physical edge");
    if (!mips) {
        unsigned w=width, h=height;
        mips=1;
        while (w>1 || h>1) { w=std::max(1U,w/2); h=std::max(1U,h/2); ++mips; }
    }
    renderer::TextureDesc desc;
    desc.width=width; desc.height=height; desc.mip_levels=mips;
    desc.dimension=renderer::TextureDimension::texture_2d; desc.format=translated;
    auto handle=device_.create_texture(desc,"original TextureLoader");
    if (!handle) throw std::runtime_error("original texture creation failed: "+device_.last_error());
    return handle;
}

void OriginalGpuEdge::upload_texture(renderer::TextureHandle texture, unsigned level,
    unsigned width, unsigned height, unsigned pitch, const void* bytes, std::size_t size)
{
    guard_nonstage_mutation();
    renderer::TextureUploadDesc desc{texture,width,height,pitch,size,level};
    const auto result=device_.upload_texture(desc,bytes);
    if (!result) throw std::runtime_error("original texture mip upload failed: "+device_.last_error());
}

void OriginalGpuEdge::discard_texture(renderer::TextureHandle handle) noexcept
{
    if (source_stages_active()) {poison_source_stages();return;}
    if (handle) device_.destroy(handle);
}

void OriginalGpuEdge::publish_texture(TextureBaseClass* source, renderer::TextureHandle texture, bool shared_missing)
{
    guard_nonstage_mutation();
    if (!source || !texture || textures_.count(source))
        throw std::runtime_error("original texture physical publication is invalid");
    textures_.emplace(source,TextureOwnership{texture,generation_,shared_missing});
    try {
        if (!shared_missing) ++texture_owner_refs_[texture.value()];
    } catch (...) {
        textures_.erase(source);
        throw;
    }
}

void OriginalGpuEdge::publish_texture_alias(TextureBaseClass* source, const TextureBaseClass* owner)
{
    guard_nonstage_mutation();
    if (!source || !owner || source==owner || textures_.count(source))
        throw std::runtime_error("original texture alias publication is invalid");
    const auto found=textures_.find(const_cast<TextureBaseClass*>(owner));
    if (found==textures_.end() || found->second.generation!=generation_ || found->second.shared_missing)
        throw std::runtime_error("original texture alias owner is unavailable");
    textures_.emplace(source,TextureOwnership{found->second.handle,generation_,false});
    ++texture_owner_refs_[found->second.handle.value()];
}

bool OriginalGpuEdge::withdraw_candidate_texture(TextureBaseClass* source,renderer::TextureHandle texture) noexcept
{
    if (source_stage_attempt_) {poison_source_stages();return false;}
    if (active_edge!=this || !device_transaction_ || source_stage_attempt_
        || device_transaction_->mode!=renderer::DeviceTransactionMode::idle_preparation
        || device_transaction_->generation!=generation_) return false;
    const auto owned=textures_.find(source);
    const auto refs=texture_owner_refs_.find(texture.value());
    if (owned==textures_.end() || owned->second.generation!=generation_
        || owned->second.handle!=texture || owned->second.shared_missing
        || refs==texture_owner_refs_.end() || refs->second!=1) return false;
    for (const auto& stage:pending_stages_) if (stage.source==source) return false;
    texture_owner_refs_.erase(refs);textures_.erase(owned);
    return true;
}

renderer::TextureHandle OriginalGpuEdge::missing_texture()
{
    guard_nonstage_mutation();
    if (!missing_texture_) missing_texture_=MissingTexture::_Create_Gpu_Missing_Texture();
    return missing_texture_;
}

bool OriginalGpuEdge::is_missing_texture(const TextureBaseClass* source) noexcept
{
    if (!active_edge) return false;
    const auto it=active_edge->textures_.find(const_cast<TextureBaseClass*>(source));
    return it!=active_edge->textures_.end() && it->second.shared_missing;
}

renderer::TextureHandle OriginalGpuEdge::texture_handle(const TextureBaseClass* source) const
{
    const auto it=textures_.find(const_cast<TextureBaseClass*>(source));
    if (it==textures_.end() || it->second.generation!=generation_)
        throw std::runtime_error("original texture owner is not resident in active device generation");
    return it->second.handle;
}

void OriginalGpuEdge::release_texture_if_owned(TextureBaseClass* source) noexcept
{
    if (!active_edge) return;
    if (active_edge->source_stages_active()) {
        try { active_edge->cancel_source_stages_for_reset(); } catch (...) { std::terminate(); }
    }
    const auto it=active_edge->textures_.find(source);
    if (it==active_edge->textures_.end()) return;
    ++active_edge->source_revision_;
    for (unsigned index=0;index<active_edge->pending_stages_.size();++index) {
        auto& stage=active_edge->pending_stages_[index];
        if (stage.source==source) {
            if (stage.sampler) active_edge->device_.destroy(stage.sampler);
            stage={};
            active_edge->pending_filter_values_[index]={};
        }
    }
    if (!it->second.shared_missing) {
        auto ref=active_edge->texture_owner_refs_.find(it->second.handle.value());
        if (ref==active_edge->texture_owner_refs_.end() || !ref->second) std::terminate();
        if (--ref->second==0) {
            active_edge->device_.destroy(it->second.handle);
            active_edge->texture_owner_refs_.erase(ref);
        }
    }
    active_edge->textures_.erase(it);
}

void OriginalGpuEdge::select_texture(unsigned stage, const TextureBaseClass* source)
{
    SourceMutationGuard mutation{*this};
    guard_source_stage(stage);
    if (source_stage_attempt_ && source!=source_stage_attempt_->selections[stage]
        && (source || WW3D::Is_Texturing_Enabled())) guard_source_stage(stage,source,true);
    if (stage>=pending_stages_.size()) throw std::runtime_error("original texture stage exceeds DX8 stage count");
    auto handle=source ? texture_handle(source) : renderer::TextureHandle{};
    // An explicit null selection is also the source-equivalent release point
    // for a sampler configured on a disabled legacy stage.
    if (pending_stages_[stage].source!=source || !source) {
        if (pending_stages_[stage].sampler) device_.destroy(pending_stages_[stage].sampler);
        pending_stages_[stage]={};
        pending_filter_values_[stage]={};
    }
    pending_stages_[stage].texture=handle;
    pending_stages_[stage].generation=generation_;
    pending_stages_[stage].source=source;
    ++source_revision_;
    device_.record_marker("original TextureClass::Apply stage="+std::to_string(stage)+
        (source ? " selected" : " disabled"));
}

void OriginalGpuEdge::set_filter_stage_state(unsigned stage, FilterStageState state, unsigned value)
{
    SourceMutationGuard mutation{*this};
    guard_source_stage(stage);
    if (stage>=pending_stages_.size()) throw std::runtime_error("original filter stage exceeds DX8 stage count");
    if (value>(state==FilterStageState::min_filter || state==FilterStageState::mag_filter ||
        state==FilterStageState::mip_filter ? 2U : 1U))
        { poison_source_stages();throw std::runtime_error("original texture filter/address mode unsupported by physical device"); }
    auto& selected=pending_filter_values_[stage];
    switch (state) {
    case FilterStageState::min_filter: selected.min=value; break;
    case FilterStageState::mag_filter: selected.mag=value; break;
    case FilterStageState::mip_filter: selected.mip=value; break;
    case FilterStageState::address_u: selected.u=value; break;
    case FilterStageState::address_v: selected.v=value; break;
    }
    ++source_revision_;
    device_.record_marker("original TextureFilterClass stage="+std::to_string(stage)+
        " property="+std::to_string(static_cast<unsigned>(state))+" value="+std::to_string(value));
    if (state!=FilterStageState::address_v) return;
    if (selected.min<0 || selected.mag<0 || selected.mip<0 || selected.u<0 || selected.v<0)
        { poison_source_stages();throw std::runtime_error("original filter state sequence is incomplete"); }
    renderer::SamplerDesc desc;
    desc.min_filter=selected.min ? renderer::Filter::linear : renderer::Filter::nearest;
    desc.mag_filter=selected.mag ? renderer::Filter::linear : renderer::Filter::nearest;
    desc.mip_filter=selected.mip==2 ? renderer::Filter::linear : renderer::Filter::nearest;
    desc.maximum_anisotropy=selected.min==2 || selected.mag==2 ? 2 : 1;
    desc.maximum_lod=selected.mip ? 1000.0F : 0.0F;
    desc.address_u=selected.u ? renderer::AddressMode::clamp_edge : renderer::AddressMode::repeat;
    desc.address_v=selected.v ? renderer::AddressMode::clamp_edge : renderer::AddressMode::repeat;
    auto sampler=device_.create_sampler(desc,"original TextureFilterClass stage");
    if (!sampler) throw std::runtime_error("original texture sampler creation failed: "+device_.last_error());
    auto& pending=pending_stages_[stage];
    if (pending.sampler) device_.destroy(pending.sampler);
    pending.sampler=sampler;
    pending.generation=generation_;
}

OriginalGpuEdge::PendingStage OriginalGpuEdge::pending_stage(unsigned stage) const
{
    if (stage>=pending_stages_.size() || pending_stages_[stage].generation!=generation_)
        throw std::runtime_error("original texture stage is absent from active device generation");
    return pending_stages_[stage];
}

void OriginalGpuEdge::record_source_state(std::string_view label)
{
    ++source_revision_;
    try { device_.record_marker(label); } catch (...) { poison_source_stages();throw; }
}

void OriginalGpuEdge::bind_frame_targets(renderer::TextureHandle color,
    renderer::TextureHandle depth,unsigned width,unsigned height)
{
    guard_nonstage_mutation();
    if (device_transaction_ || source_frame_active_ || !color || !depth || !width || !height)
        throw std::runtime_error("original frame target binding requires idle, complete attachments");
    bound_frame_=BoundFrame{color,depth,width,height};
    ++frame_target_generation_;
    source_viewport_.reset();
}

std::pair<unsigned,unsigned> OriginalGpuEdge::bound_frame_extent() const
{
    if (!bound_frame_) throw std::runtime_error("original frame has no caller-owned render targets");
    return {bound_frame_->width,bound_frame_->height};
}

void OriginalGpuEdge::begin_source_frame(bool clear_color,bool clear_depth,
    float red,float green,float blue,float alpha)
{
    guard_nonstage_mutation();
    const auto [width,height]=bound_frame_extent();
    if (source_frame_active_) throw std::runtime_error("original frame pass already active");
    renderer::RenderPassDesc pass;
    pass.color_targets[0]=bound_frame_->color;
    pass.color_target_count=1;
    pass.depth_target=bound_frame_->depth;
    pass.width=width; pass.height=height;
    pass.color_load=clear_color ? renderer::AttachmentLoad::clear : renderer::AttachmentLoad::load;
    pass.depth_load=clear_depth ? renderer::AttachmentLoad::clear : renderer::AttachmentLoad::load;
    pass.clear_color={red,green,blue,alpha};
    pass.target_generation=frame_target_generation_;
    if (clear_depth && !pass.depth_target)
        throw std::runtime_error("original depth clear requires a caller-owned depth target");
    if (auto result=device_.begin_pass(pass,"WW3D::Begin_Render source frame"); !result)
        throw std::runtime_error("original source frame begin failed: "+result.error);
    source_frame_active_=true;
    source_viewport_.reset();
    if (clear_color || clear_depth) {
        renderer::ViewportDesc viewport{0,0,static_cast<float>(width),
            static_cast<float>(height),0,1};
        if (auto result=device_.set_viewport(viewport); !result) {
            abort_source_frame();
            throw std::runtime_error("original full-target frame viewport failed: "+result.error);
        }
        source_viewport_=viewport;
    }
}

void OriginalGpuEdge::end_source_frame(bool present)
{
    guard_nonstage_mutation();
    if (!source_frame_active_) throw std::runtime_error("original source frame is not active");
    if (auto result=device_.end_pass(); !result)
        throw std::runtime_error("original source frame end failed: "+result.error);
    source_frame_active_=false;
    source_viewport_.reset();
    if (present) {
        if (source_frame_attempt_ && source_frame_present_fault_) {
            source_frame_present_fault_=false;
            throw std::runtime_error("original generated source pre-present rejection");
        }
        if (auto result=device_.present(bound_frame_->color); !result)
            throw std::runtime_error("original source frame presentation failed: "+result.error);
    }
}

void OriginalGpuEdge::abort_source_frame() noexcept
{
    if (source_stages_active()) { poison_source_stages(); return; }
    if (!source_frame_active_) return;
    source_frame_active_=false;
    source_viewport_.reset();
    (void)device_.end_pass();
}

std::pair<unsigned,unsigned> OriginalGpuEdge::active_render_target_extent() const noexcept
{ return device_.active_pass_extent(); }

void OriginalGpuEdge::set_source_viewport(float x,float y,float width,float height,
    float min_depth,float max_depth)
{
    guard_nonstage_mutation();
    renderer::ViewportDesc viewport{x,y,width,height,min_depth,max_depth};
    if (auto result=device_.set_viewport(viewport); !result)
        throw std::runtime_error("original camera viewport GPU translation failed: "+result.error);
    source_viewport_=viewport;
    record_source_state("CameraClass::Apply viewport");
}

void OriginalGpuEdge::clear_source_viewport(bool color,bool depth,bool stencil,
    std::array<float,4> rgba,float z,unsigned stencil_value)
{
    guard_nonstage_mutation();
    if (!source_frame_active_ || !source_viewport_ || !bound_frame_)
        throw std::runtime_error("original camera clear requires an active source frame and viewport");
    const auto& vp=*source_viewport_;
    const auto integral=[](float value) { return std::isfinite(value) && std::trunc(value)==value; };
    if (!integral(vp.x) || !integral(vp.y) || !integral(vp.width) || !integral(vp.height) ||
        vp.x<0 || vp.y<0 || static_cast<double>(vp.x)>std::numeric_limits<renderer::Int32>::max() ||
        static_cast<double>(vp.y)>std::numeric_limits<renderer::Int32>::max() ||
        static_cast<double>(vp.width)>std::numeric_limits<renderer::UInt32>::max() ||
        static_cast<double>(vp.height)>std::numeric_limits<renderer::UInt32>::max())
        throw std::runtime_error("original camera clear requires an integral D3D viewport rectangle");
    if (stencil && stencil_value>255U)
        throw std::runtime_error("original camera clear stencil exceeds 8-bit attachment");
    renderer::ViewportClearDesc clear;
    clear.color_target=bound_frame_->color;
    clear.depth_target=bound_frame_->depth;
    clear.target_generation=frame_target_generation_;
    clear.x=static_cast<renderer::Int32>(vp.x);
    clear.y=static_cast<renderer::Int32>(vp.y);
    clear.width=static_cast<renderer::UInt32>(vp.width);
    clear.height=static_cast<renderer::UInt32>(vp.height);
    clear.color=color; clear.depth=depth; clear.stencil=stencil;
    clear.color_value=rgba; clear.depth_value=z;
    clear.stencil_value=static_cast<renderer::UInt8>(stencil_value);
    if (auto result=device_.clear_viewport(clear); !result)
        throw std::runtime_error("original camera viewport clear failed: "+result.error);
}

bool OriginalGpuEdge::source_depth_has_stencil() const
{
    if (!bound_frame_ || !source_frame_active_)
        throw std::runtime_error("original source depth-format query requires an active frame");
    const auto format=device_.describe_texture_format(bound_frame_->depth);
    if (!format)
        throw std::runtime_error("original source depth target is stale or lacks a format query");
    switch (*format) {
    case renderer::TextureFormat::depth24_stencil8: return true;
    case renderer::TextureFormat::depth16:
    case renderer::TextureFormat::depth32: return false;
    default: throw std::runtime_error("original source depth target format is unsupported");
    }
}

[[noreturn]] void OriginalGpuEdge::texture_creation_unavailable(WW3DFormat format,
    unsigned width, unsigned height, unsigned mips, unsigned reduction)
{
    guard_nonstage_mutation();
    device_.record_marker("original TextureLoader selected format=" + std::to_string(format) +
        " width=" + std::to_string(width) + " height=" + std::to_string(height) +
        " mips=" + std::to_string(mips) + " reduction=" + std::to_string(reduction));
    throw std::runtime_error("original texture creation requires a GPU device translation (M22 05B2B)");
}

renderer::BufferHandle OriginalGpuEdge::bind_vertex(const VertexBufferClass* source)
{
    guard_nonstage_mutation();
    if (!source || source->Type() != BUFFER_TYPE_DX8)
        throw std::runtime_error("original sorting/dynamic vertex physical route is not translated");
    const auto* original = static_cast<const DX8VertexBufferClass*>(source);
    const auto* bytes = original->Get_CPU_Vertex_Buffer();
    const renderer::UInt64 size = static_cast<renderer::UInt64>(source->Get_Vertex_Count()) *
        source->FVF_Info().Get_FVF_Size();
    if (!bytes || !size) throw std::runtime_error("original vertex buffer has no upload bytes");
    auto it = vertices_.find(source);
    if (it == vertices_.end()) {
        auto handle = device_.create_buffer({size, renderer::BufferUsage::vertex, true}, "original WW3D vertex buffer");
        if (!handle) throw std::runtime_error("original vertex buffer device creation failed");
        try { it = vertices_.emplace(source, handle).first; source->Add_Ref(); }
        catch (...) { device_.destroy(handle); throw; }
    }
    if (auto result = device_.upload({it->second, size, 0, size}, bytes); !result)
        throw std::runtime_error("original vertex upload failed: " + result.error);
    return it->second;
}

renderer::BufferHandle OriginalGpuEdge::bind_index(const IndexBufferClass* source)
{
    guard_nonstage_mutation();
    if (!source || source->Type() != BUFFER_TYPE_DX8)
        throw std::runtime_error("original sorting/dynamic index physical route is not translated");
    const auto* original = static_cast<const DX8IndexBufferClass*>(source);
    const auto* bytes = original->Get_CPU_Index_Buffer();
    const renderer::UInt64 size = static_cast<renderer::UInt64>(source->Get_Index_Count()) * sizeof(*bytes);
    if (!bytes || !size) throw std::runtime_error("original index buffer has no upload bytes");
    auto it = indices_.find(source);
    if (it == indices_.end()) {
        auto handle = device_.create_buffer({size, renderer::BufferUsage::index, true}, "original WW3D 16-bit index buffer");
        if (!handle) throw std::runtime_error("original index buffer device creation failed");
        try { it = indices_.emplace(source, handle).first; source->Add_Ref(); }
        catch (...) { device_.destroy(handle); throw; }
    }
    if (auto result = device_.upload({it->second, size, 0, size}, bytes); !result)
        throw std::runtime_error("original index upload failed: " + result.error);
    return it->second;
}

void OriginalGpuEdge::release_vertex(const VertexBufferClass* source)
{
    guard_nonstage_mutation();
    if (source_frame_active_)
        throw std::runtime_error("original source vertex buffer cannot retire during a frame");
    const auto it=vertices_.find(source);
    if (it==vertices_.end()) return;
    device_.destroy(it->second);
    it->first->Release_Ref();
    vertices_.erase(it);
    ++source_revision_;
}

void OriginalGpuEdge::release_index(const IndexBufferClass* source)
{
    guard_nonstage_mutation();
    if (source_frame_active_)
        throw std::runtime_error("original source index buffer cannot retire during a frame");
    const auto it=indices_.find(source);
    if (it==indices_.end()) return;
    device_.destroy(it->second);
    it->first->Release_Ref();
    indices_.erase(it);
    ++source_revision_;
}

void OriginalGpuEdge::draw_source_indexed(const VertexBufferClass* vertex,
    const IndexBufferClass* index,unsigned first_index,unsigned index_count,
    unsigned base_vertex,unsigned min_vertex,unsigned vertex_count,
    renderer::PrimitiveTopology topology)
{
    guard_nonstage_mutation();
    if (!device_.pass_active())
        throw std::runtime_error("original indexed draw requires a caller-owned active render pass");
    if (!vertex || !index || !index_count || !vertex_count)
        throw std::runtime_error("original indexed draw is missing a source buffer or range");
    if (first_index>index->Get_Index_Count() ||
        index_count>index->Get_Index_Count()-first_index ||
        base_vertex>vertex->Get_Vertex_Count() ||
        min_vertex>vertex->Get_Vertex_Count()-base_vertex ||
        vertex_count>vertex->Get_Vertex_Count()-base_vertex-min_vertex)
        throw std::runtime_error("original indexed draw source range exceeds its buffer");
    if (vertex->Type()!=BUFFER_TYPE_DX8 || index->Type()!=BUFFER_TYPE_DX8)
        throw std::runtime_error("original indexed draw cannot use sorting/dynamic source buffers yet");
    const auto* source_indices=static_cast<const DX8IndexBufferClass*>(index)->Get_CPU_Index_Buffer();
    if (!source_indices) throw std::runtime_error("original indexed draw is missing source indices");
    for (unsigned i=0;i<index_count;++i) {
        const unsigned element=source_indices[first_index+i];
        if (element<min_vertex || element-min_vertex>=vertex_count)
            throw std::runtime_error("original indexed draw source index escapes its declared vertex range");
    }
    auto bound_vertex=bind_vertex(vertex);
    auto bound_index=bind_index(index);
    const auto prepared=prepare_applied_state(vertex->FVF_Info().Get_FVF(),topology);
    validate_prepared_state(prepared);
    renderer::DrawDesc draw;
    draw.pipeline=prepared.pipeline;
    draw.vertex_buffer=bound_vertex;
    draw.index_buffer=bound_index;
    draw.vertex_or_index_count=index_count;
    draw.index_element_size=renderer::IndexElementSize::uint16;
    draw.first_index=first_index;
    draw.base_vertex=static_cast<renderer::Int32>(base_vertex);
    draw.vertex_bindings=prepared.vertex_bindings;
    draw.fragment_bindings=prepared.fragment_bindings;
    if (auto result=device_.draw(draw); !result) {
        release_prepared_state();
        throw std::runtime_error("original indexed draw GPU translation failed: "+result.error);
    }
    device_.record_marker("DX8Wrapper::Draw indexed first="+std::to_string(first_index)+
        " count="+std::to_string(index_count)+" base="+std::to_string(base_vertex));
}

void OriginalGpuEdge::draw_volume_stencil(const VertexBufferClass* vertex,
    const IndexBufferClass* index, unsigned first_index, unsigned index_count,
    unsigned base_vertex, unsigned vertex_count, renderer::UInt8 shadow_mask)
{
    guard_nonstage_mutation();
    if (!source_frame_active_ || !bound_frame_ || !device_.pass_active())
        throw std::runtime_error("original volume stencil requires an active caller-owned source frame");
    if (!vertex || !index || vertex->Type()!=BUFFER_TYPE_DX8 || index->Type()!=BUFFER_TYPE_DX8 ||
        vertex->FVF_Info().Get_FVF()!=DX8_FVF_XYZ || !index_count || !vertex_count)
        throw std::runtime_error("original volume stencil source layout or range is unsupported");
    if (!zh_w3d_volume_provider_owns(vertex,index))
        throw std::runtime_error("original volume stencil source buffer provider is foreign or absent");
    if (first_index>index->Get_Index_Count() || index_count>index->Get_Index_Count()-first_index ||
        base_vertex>vertex->Get_Vertex_Count() || vertex_count>vertex->Get_Vertex_Count()-base_vertex)
        throw std::runtime_error("original volume stencil source range exceeds its buffers");
    const auto* source_indices=static_cast<const DX8IndexBufferClass*>(index)->Get_CPU_Index_Buffer();
    if (!source_indices) throw std::runtime_error("original volume stencil is missing source indices");
    for (unsigned offset=0;offset<index_count;++offset)
        if (source_indices[first_index+offset]>=vertex_count)
            throw std::runtime_error("original volume stencil source index escapes its declared range");
    const auto color_format=device_.describe_texture_format(bound_frame_->color);
    const auto depth_format=device_.describe_texture_format(bound_frame_->depth);
    if (!color_format || !depth_format || *depth_format!=renderer::TextureFormat::depth24_stencil8 ||
        !device_.supports_texture_format(renderer::TextureFormat::depth24_stencil8,
            renderer::TextureDimension::texture_2d,false,true))
        throw std::runtime_error("original volume stencil requires a supported D24S8 source target");
    if (*color_format!=renderer::TextureFormat::rgba8 && *color_format!=renderer::TextureFormat::bgra8)
        throw std::runtime_error("original volume stencil source color target is unsupported");
    if (volume_stencil_ && volume_stencil_->color_format!=*color_format)
        throw std::runtime_error("original volume stencil target recreation is pending");
    if (!volume_stencil_) {
        VolumeStencilResources next;
        try {
            next.vertex_shader=device_.create_shader({renderer::ShaderStage::vertex,
                "renderer/original_volume.vert",0,0},"original volume XYZ vertex");
            next.fragment_shader=device_.create_shader({renderer::ShaderStage::fragment,
                "renderer/original_volume.frag",0,0},"original volume composite fragment");
            if (!next.vertex_shader || !next.fragment_shader)
                throw std::runtime_error("original volume stencil shader creation failed: "+device_.last_error());
            auto protocol=make_volume_stencil_protocol(next.vertex_shader,next.fragment_shader,*color_format,shadow_mask);
            for (auto *pass : {&protocol.increment,&protocol.decrement,&protocol.composite}) {
                pass->vertex_layout=renderer::VertexLayout::original_fvf;
                pass->original_fvf=layout_for_fvf(DX8_FVF_XYZ);
            }
            next.increment=device_.create_pipeline(renderer::PipelineKey(protocol.increment),"original volume stencil increment");
            next.decrement=device_.create_pipeline(renderer::PipelineKey(protocol.decrement),"original volume stencil decrement");
            next.composite=device_.create_pipeline(renderer::PipelineKey(protocol.composite),"original volume stencil composite");
            if (!next.increment || !next.decrement || !next.composite)
                throw std::runtime_error("original volume stencil pipeline creation failed: "+device_.last_error());
            next.color_format=*color_format;
            volume_stencil_=next;
        } catch (...) {
            device_.destroy(next.composite); device_.destroy(next.decrement); device_.destroy(next.increment);
            device_.destroy(next.fragment_shader); device_.destroy(next.vertex_shader);
            throw;
        }
    }
    const bool new_vertex=!vertices_.count(vertex), new_index=!indices_.count(index);
    try {
        renderer::DrawDesc draw;
        draw.vertex_buffer=bind_vertex(vertex); draw.index_buffer=bind_index(index);
        draw.vertex_or_index_count=index_count; draw.index_element_size=renderer::IndexElementSize::uint16;
        draw.first_index=first_index; draw.base_vertex=static_cast<renderer::Int32>(base_vertex);
        const renderer::PipelineHandle passes[]={volume_stencil_->increment,volume_stencil_->decrement,volume_stencil_->composite};
        const char* names[]={"increment","decrement","composite"};
        for (unsigned pass=0;pass<3;++pass) {
            draw.pipeline=passes[pass];
            if (auto result=device_.draw(draw); !result)
                throw std::runtime_error("original volume stencil "+std::string(names[pass])+" draw failed: "+result.error);
            device_.record_marker("OriginalGpuEdge::volume_stencil "+std::string(names[pass]));
        }
    } catch (...) {
        // A source frame owns its in-flight buffer lifetime. It is ended by
        // the caller before the normal retry/retirement boundary, so do not
        // violate the established no-retire-during-frame invariant here.
        if (!source_frame_active_) {
            if (new_index) release_index(index);
            if (new_vertex) release_vertex(vertex);
        }
        throw;
    }
}

} // namespace zh::original_runtime
