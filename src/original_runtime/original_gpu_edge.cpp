#include "original_gpu_edge.h"

#include "dx8vertexbuffer.h"
#include "dx8indexbuffer.h"
#include "dx8fvf.h"
#include "dx8wrapper.h"
#include "ww3dformat.h"
#include "texture.h"
#include "missingtexture.h"

#include <algorithm>
#include <atomic>
#include <cmath>
#include <cstring>
#include <stdexcept>
#include <string>

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
    default: throw std::runtime_error("original texture format has no physical GPU mapping");
    }
}
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
    release_prepared_state();
    DX8Wrapper::Reset_Source_State();
    for (auto& stage : pending_stages_) if (stage.sampler) device_.destroy(stage.sampler);
    while (!textures_.empty()) {
        auto it=textures_.begin();
        TextureBaseClass* source=it->first;
        if (!it->second.shared_missing) device_.destroy(it->second.handle);
        textures_.erase(it);
        source->Invalidate();
    }
    for (auto& entry : indices_) { device_.destroy(entry.second); entry.first->Release_Ref(); }
    for (auto& entry : vertices_) { device_.destroy(entry.second); entry.first->Release_Ref(); }
    if (missing_texture_) device_.destroy(missing_texture_);
    active_edge = previous_;
}

OriginalGpuEdge& OriginalGpuEdge::required()
{
    if (!active_edge) throw std::runtime_error("original physical call requires a GPU translation session");
    return *active_edge;
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
            && out.uv_source>=source_uv_count)
            throw std::runtime_error("original textured stage requests absent source UV coordinates");
        out.transform_flags=stage_value(D3DTSS_TEXTURETRANSFORMFLAGS);
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

OriginalGpuEdge::PhysicalState OriginalGpuEdge::prepare_applied_state(unsigned source_fvf,
    renderer::PrimitiveTopology topology)
{
    const auto source_state=DX8Wrapper::Snapshot_Source_State();
    const auto source_lighting=source_state.render.find(D3DRS_LIGHTING);
    if (source_lighting!=source_state.render.end() && source_lighting->second==1) {
        (void)map_applied_state(source_fvf);
        throw std::runtime_error("original lit physical state requires category-issued light environment (M22 06)");
    }
    release_prepared_state();
    const AppliedState mapped=map_applied_state(source_fvf);
    if (topology!=renderer::PrimitiveTopology::triangle_list &&
        topology!=renderer::PrimitiveTopology::triangle_strip)
        throw std::runtime_error("original indexed primitive topology is unsupported");
    const char* vertex_variant=nullptr;
    if (source_fvf==DX8_FVF_XYZDUV1) vertex_variant="renderer/original_applied_d1.vert";
    else if (source_fvf==DX8_FVF_XYZDUV2) vertex_variant="renderer/original_applied_d2.vert";
    else if (source_fvf==DX8_FVF_XYZN) vertex_variant="renderer/original_applied_n0.vert";
    else if (source_fvf==DX8_FVF_XYZNUV1) vertex_variant="renderer/original_applied_n1.vert";
    else if (source_fvf==DX8_FVF_XYZNUV2) vertex_variant="renderer/original_applied_n2.vert";
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
        pipeline.vertex_shader=next.vertex_shader;
        pipeline.fragment_shader=next.fragment_shader;
        next.pipeline=device_.create_pipeline(renderer::PipelineKey(pipeline),"original applied pipeline");
        if (!next.pipeline) throw std::runtime_error("original applied pipeline creation failed: "+device_.last_error());
        next.vertex_uniform=device_.create_buffer({sizeof(vertex),renderer::BufferUsage::uniform,true},
            "original world/view/projection and UV state");
        if (!next.vertex_uniform) throw std::runtime_error("original vertex uniform creation failed: "+device_.last_error());
        next.fragment_uniform=device_.create_buffer({sizeof(fragment),renderer::BufferUsage::uniform,true},
            "original material/shader/fog state");
        if (!next.fragment_uniform) throw std::runtime_error("original fragment uniform creation failed: "+device_.last_error());
        if (auto result=device_.upload({next.vertex_uniform,sizeof(vertex),0,sizeof(vertex)},&vertex); !result)
            throw std::runtime_error("original vertex uniform upload failed: "+result.error);
        if (auto result=device_.upload({next.fragment_uniform,sizeof(fragment),0,sizeof(fragment)},&fragment); !result)
            throw std::runtime_error("original fragment uniform upload failed: "+result.error);
        next.state.pipeline=next.pipeline;
        next.state.vertex_bindings.uniforms[0]={next.vertex_uniform,0,sizeof(vertex)};
        next.state.vertex_bindings.uniform_count=1;
        next.state.fragment_bindings=fragment_bindings;
        next.state.fragment_bindings.uniforms[0]={next.fragment_uniform,0,sizeof(fragment)};
        next.state.fragment_bindings.uniform_count=1;
        next.state.generation=generation_;
        next.state.serial=++physical_serial_;
        next.state.source_revision=source_revision_;
        next.state.texture_mask=mask;
        next.sources=stage_sources;
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
        state.vertex_bindings.uniforms[0].size!=sizeof(VertexUniform) ||
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
    renderer::TextureUploadDesc desc{texture,width,height,pitch,size,level};
    const auto result=device_.upload_texture(desc,bytes);
    if (!result) throw std::runtime_error("original texture mip upload failed: "+device_.last_error());
}

void OriginalGpuEdge::discard_texture(renderer::TextureHandle handle) noexcept
{
    if (handle) device_.destroy(handle);
}

void OriginalGpuEdge::publish_texture(TextureBaseClass* source, renderer::TextureHandle texture, bool shared_missing)
{
    if (!source || !texture || textures_.count(source))
        throw std::runtime_error("original texture physical publication is invalid");
    textures_.emplace(source,TextureOwnership{texture,generation_,shared_missing});
}

renderer::TextureHandle OriginalGpuEdge::missing_texture()
{
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
    if (!it->second.shared_missing) active_edge->device_.destroy(it->second.handle);
    active_edge->textures_.erase(it);
}

void OriginalGpuEdge::select_texture(unsigned stage, const TextureBaseClass* source)
{
    if (stage>=pending_stages_.size()) throw std::runtime_error("original texture stage exceeds DX8 stage count");
    auto handle=source ? texture_handle(source) : renderer::TextureHandle{};
    if (pending_stages_[stage].source!=source) {
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
    if (stage>=pending_stages_.size()) throw std::runtime_error("original filter stage exceeds DX8 stage count");
    if (value>(state==FilterStageState::min_filter || state==FilterStageState::mag_filter ||
        state==FilterStageState::mip_filter ? 2U : 1U))
        throw std::runtime_error("original texture filter/address mode unsupported by physical device");
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
        throw std::runtime_error("original filter state sequence is incomplete");
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
    device_.record_marker(label);
}

[[noreturn]] void OriginalGpuEdge::texture_creation_unavailable(WW3DFormat format,
    unsigned width, unsigned height, unsigned mips, unsigned reduction)
{
    device_.record_marker("original TextureLoader selected format=" + std::to_string(format) +
        " width=" + std::to_string(width) + " height=" + std::to_string(height) +
        " mips=" + std::to_string(mips) + " reduction=" + std::to_string(reduction));
    throw std::runtime_error("original texture creation requires a GPU device translation (M22 05B2B)");
}

renderer::BufferHandle OriginalGpuEdge::bind_vertex(const VertexBufferClass* source)
{
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

void OriginalGpuEdge::draw_source_indexed(const VertexBufferClass* vertex,
    const IndexBufferClass* index,unsigned first_index,unsigned index_count,
    unsigned base_vertex,unsigned min_vertex,unsigned vertex_count,
    renderer::PrimitiveTopology topology)
{
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

} // namespace zh::original_runtime
